#include "com.h"
#include "config.h"
#include "language.h"
//#include "SD.h"
#include "SdFat.h"
#include "draw.h"
#include "nunchuk.h"
#include <WiFi.h> 
#include "telnet.h"
#include "cmd.h"
#include "log.h"
#include <Preferences.h>

// GRBL status are : Idle, Run, Hold, Jog, Alarm, Door, Check, Home, Sleep
// a message should look like (note : GRBL sent or WPOS or MPos depending on grbl parameter : to get WPos, we have to set "$10=0"
//  <Jog|WPos:1329.142,0.000,0.000|Bf:32,254|FS:2000,0|Ov:100,100,100|A:FM>
//  <Idle|WPos:0.000,0.000,0.000|FS:0.0,0> or e.g. <Idle|MPos:0.000,0.000,0.000|FS:0.0,0|WCO:0.000,0.000,0.000|OV:100,100,100>
//CLOSED
//   START
//        WPOS_HEADER
//            WPOS_DATA
//                               HEADER
//                                 F_DATA
//                                   F_DATA
//                                                                                      WCO_DATA
// note : status can also be with a ":" like Hold:1
// outside <...>, we can get "Ok" and "error:12" followed by an CR and/or LF
// we can also get messages: They are enclosed between [....] ; they are currently discarded. 
// used to decode the status and position sent by GRBL
#define GET_GRBL_STATUS_CLOSED 0
#define GET_GRBL_STATUS_START 1
#define GET_GRBL_STATUS_WPOS_HEADER 2
#define GET_GRBL_STATUS_WPOS_DATA 3
#define GET_GRBL_STATUS_HEADER 4
#define GET_GRBL_STATUS_F_DATA 5
#define GET_GRBL_STATUS_WCO_DATA 6
#define GET_GRBL_STATUS_ALARM 7
#define GET_GRBL_STATUS_BF_DATA 8
#define GET_GRBL_STATUS_MESSAGE 9
#define GET_GRBL_STATUS_OV_DATA 10
  
#define WAIT_OK_SD_TIMEOUT 120000

uint8_t wposIdx = 0 ;
uint8_t wcoIdx = 0 ;
uint8_t fsIdx = 0 ;
uint8_t bfIdx = 0 ;
uint8_t ovIdx = 0 ;
uint8_t getGrblPosState = GET_GRBL_STATUS_CLOSED ;
float feedSpindle[2] ;  // first is FeedRate, second is Speed
float bufferAvailable[2] ;  // first is number of blocks available in planner, second is number of chars available in serial buffer
float overwritePercent[3] ; // first is for feedrate, second for rapid (G0...), third is for RPM
float wcoXYZA[4] ;           // 4 because we can support 4 axis
float wposXYZA[4] ;
float mposXYZA[4] ;

extern Preferences preferences ; // object from ESP32 lib used to save/get data in flash 

char modalAbsRel[4] = {0}; // store the modal G20/G21 in a message received from grbl
char modalMmInch[4] = {0} ; // store the modal G90/G91 in a message received from grbl
char G30SavedX[12] = "0.0" ; // store the value of G30 X offset in a message received from grbl;
char G30SavedY[12] = "0.0" ; // store the value of G30 Y offset in a message received from grbl;

char printString[250] = {0} ;       // contains a command to be send from a string; wait for OK after each 0x13 char
char * pPrintString = printString ;
uint8_t lastStringCmd ;

char strGrblBuf[STR_GRBL_BUF_MAX_SIZE] ; // this buffer is used to store a few char received from GRBL before decoding them
uint8_t strGrblIdx ;
extern char grblLastMessage[STR_GRBL_BUF_MAX_SIZE] ;
extern boolean grblLastMessageChanged;

extern int8_t jogDistX ;
extern int8_t jogDistY ;
extern int8_t jogDistZ ;
extern int8_t jogDistA ;

extern float moveMultiplier ;
// used by nunchuck
extern uint8_t jog_status  ;
extern boolean jogCancelFlag ;
extern boolean jogCmdFlag  ; 
extern uint32_t startMoveMillis ;

extern volatile boolean waitOk ;
extern boolean newGrblStatusReceived ;
extern volatile uint8_t statusPrinting  ;
extern char machineStatus[9];
extern char lastMsg[80] ;        // last message to display
extern uint16_t lastMsgColor ;        // last message color

extern boolean updateFullPage ;

//        Pour sendToGrbl
//extern File fileToRead ;
extern SdBaseFile aDir[DIR_LEVEL_MAX] ; 
extern int8_t dirLevel ;
extern uint8_t cmdToSend ; // cmd to be send
extern uint32_t sdNumberOfCharSent ;

extern WiFiClient telnetClient;

uint8_t wposOrMpos ;
uint32_t waitOkWhenSdMillis ;  // timeout when waiting for OK while sending form SD

// ----------------- fonctions pour lire de GRBL -----------------------------
void getFromGrblAndForward( void ) {   //get char from GRBL, forward them if statusprinting = PRINTING_FROM_PC and decode the data (look for "OK", for <xxxxxx> sentence
                                       // fill machineStatus[] and wposXYZA[]
  static uint8_t c;
  static uint8_t lastC = 0 ;
  static uint32_t millisLastGetGBL = 0 ;
  //uint8_t i = 0 ;
  static int cntOk = 0 ;
  while (Serial2.available() ) {
#ifdef DEBUG_TO_PC
    //Serial.print(F("s=")); Serial.print( getGrblPosState ); Serial.println() ;
#endif    
    c=Serial2.read() ;
//#define DEBUG_RECEIVED_CHAR
#ifdef DEBUG_RECEIVED_CHAR      
      Serial.print( (char) c) ;
      //if  (c == 0x0A || c == 0x0C ) Serial.println(millis()) ;
#endif      
    if ( statusPrinting == PRINTING_FROM_USB  ) {
      Serial.print( (char) c) ;                         // forward characters from GRBL to PC when PRINTING_FROM_USB
    }
    sendViaTelnet((char) c) ;                   // forward characters from GRBL to telnet when PRINTING_FROM_TELNET
    switch (c) {                                // parse char received from grbl
    case 'k' :
      if ( lastC == 'o' ) {
        waitOk = false ;
        cntOk++;
        getGrblPosState = GET_GRBL_STATUS_CLOSED ;
        break ; 
      }
      //break ;   // avoid break here because 'k' can be part of another message

    case '\r' : // CR is sent after "error:xx"
      if( getGrblPosState == GET_GRBL_STATUS_CLOSED ) {
        if (  strGrblBuf[0] == 'e' && strGrblBuf[1] == 'r' )   {  // we got an error message or an ALARM message
          fillErrorMsg( strGrblBuf );           // save the error or ALARM
        } else if  ( strGrblBuf[0] == 'A' && strGrblBuf[1] == 'L' )  {
          fillAlarmMsg( strGrblBuf );  
        }
        
      }
      getGrblPosState = GET_GRBL_STATUS_CLOSED ;
      strGrblIdx = 0 ;                                        // reset the buffer
      strGrblBuf[strGrblIdx] = 0 ;
      break ;
 
    case '<' :
      getGrblPosState = GET_GRBL_STATUS_START ;
      strGrblIdx = 0 ;
      strGrblBuf[strGrblIdx] = 0 ;
      wposIdx = 0 ;
      millisLastGetGBL = millis();
      //Serial.print("LG< =") ; Serial.println( millisLastGetGBL ) ;
#ifdef DEBUG_TO_PC
      //Serial.print(" <") ; 
#endif      
      break ;
      
    case '>' :                     // end of grbl status 
      handleLastNumericField() ;  //wposXYZA[wposIdx] = atof (&strGrblBuf[0]) ;
      getGrblPosState = GET_GRBL_STATUS_CLOSED ;
      strGrblIdx = 0 ;
      strGrblBuf[strGrblIdx] = 0 ;
      newGrblStatusReceived = true;
#ifdef DEBUG_TO_PC
      //Serial.print("X=") ; Serial.print(wposXYZA[0]) ; Serial.print(" Y=") ; Serial.print(wposXYZA[1]) ;Serial.print(" Z=") ; Serial.println(wposXYZA[2]) ; 
      //Serial.println(">");
#endif      
      break ;
    case '|' :
      if ( getGrblPosState == GET_GRBL_STATUS_START ) {
         getGrblPosState = GET_GRBL_STATUS_WPOS_HEADER ; 
         memccpy( machineStatus , strGrblBuf , '\0', 9);
         strGrblIdx = 0;
         strGrblBuf[strGrblIdx] = 0 ;        
#ifdef DEBUG_TO_PC
         //Serial.print( "ms= " ) ; Serial.println( machineStatus ) ;
#endif          
      } else if (  getGrblPosState == GET_GRBL_STATUS_MESSAGE ) {
        if ( strGrblIdx < (STR_GRBL_BUF_MAX_SIZE - 1) 
        ) {
          strGrblBuf[strGrblIdx++] = c ;
          strGrblBuf[strGrblIdx] = 0 ;
        } 
      } else { 
        handleLastNumericField() ;
        getGrblPosState = GET_GRBL_STATUS_HEADER ;
        strGrblIdx = 0 ;  
      }
      break ;
    case ':' :
      if ( getGrblPosState == GET_GRBL_STATUS_WPOS_HEADER ) { // separateur entre field name et value 
         getGrblPosState = GET_GRBL_STATUS_WPOS_DATA ; 
         wposOrMpos = strGrblBuf[0] ;       // save the first char of the string that should be MPos or WPos
         strGrblIdx = 0 ;
         strGrblBuf[strGrblIdx] = 0 ;
         wposIdx = 0 ;
      } else if ( (getGrblPosState == GET_GRBL_STATUS_START || getGrblPosState == GET_GRBL_STATUS_CLOSED || getGrblPosState == GET_GRBL_STATUS_MESSAGE )
                && strGrblIdx < (STR_GRBL_BUF_MAX_SIZE - 1) ) {    // save the : as part of the text (for error 
         strGrblBuf[strGrblIdx++] = c ; 
         strGrblBuf[strGrblIdx] = 0 ;
      } else if ( getGrblPosState == GET_GRBL_STATUS_HEADER ) {                     // for other Header, check the type of field
        if ( strGrblBuf[0] == 'F' ) {               // start F or FS data
          getGrblPosState = GET_GRBL_STATUS_F_DATA ; 
          strGrblIdx = 0 ;
          fsIdx = 0 ;
        } else if ( strGrblBuf[0] == 'W' ) {     // start WCO data
          getGrblPosState = GET_GRBL_STATUS_WCO_DATA ; 
          strGrblIdx = 0 ;
          wcoIdx = 0 ;
        } else if ( strGrblBuf[0] == 'B' ) {     // start Bf data
          getGrblPosState = GET_GRBL_STATUS_BF_DATA ; 
          strGrblIdx = 0 ;
          bfIdx = 0 ;
        } else if ( strGrblBuf[0] == 'O' ) {     // start OV data
          getGrblPosState = GET_GRBL_STATUS_OV_DATA ; 
          strGrblIdx = 0 ;
          ovIdx = 0 ;
        }   
      }
      break ;
    case ',' :                                              // séparateur entre 2 chiffres
      if ( getGrblPosState != GET_GRBL_STATUS_MESSAGE ) {
        handleLastNumericField() ;                            // check that we are in data to be processed; if processed, reset strGrblIdx = 0 ; 
      } else if ( strGrblIdx < (STR_GRBL_BUF_MAX_SIZE - 1) ) {
        strGrblBuf[strGrblIdx++] = c ;
        strGrblBuf[strGrblIdx] = 0 ;
      }
      
      break ;  
    case '[' :                                              // annonce un message
      if( getGrblPosState == GET_GRBL_STATUS_CLOSED ) {
        getGrblPosState = GET_GRBL_STATUS_MESSAGE ;                            // check that we are in data to be processed; if processed, reset strGrblIdx = 0 ; 
        strGrblIdx = 0 ;                                        // reset the buffer
        strGrblBuf[strGrblIdx] = '[' ;
        strGrblIdx++ ;
      }
      break ;  
    case ' ' :                                              // 
      if( ( getGrblPosState == GET_GRBL_STATUS_MESSAGE ) && strGrblIdx < (STR_GRBL_BUF_MAX_SIZE - 1) ){
        strGrblBuf[strGrblIdx++] = ' ' ;
      }
      break ;  
    
    case ']' :                                              // ferme le message
      if( getGrblPosState == GET_GRBL_STATUS_MESSAGE ) {
        getGrblPosState = GET_GRBL_STATUS_CLOSED ;                            // check that we are in data to be processed; if processed, reset strGrblIdx = 0 ; 
        strGrblBuf[strGrblIdx++] = ']' ;
        strGrblBuf[strGrblIdx] = 0 ;
        memccpy( grblLastMessage , strGrblBuf , '\0', STR_GRBL_BUF_MAX_SIZE - 1);
        storeGrblState() ;                             // to do Store part of message in memory (G20, G21, ...)
        strGrblIdx = 0 ;                                        // reset the buffer
        strGrblBuf[strGrblIdx] = 0 ;
        grblLastMessageChanged = true ;
      }
      break ;  
    default :
      if (  strGrblIdx < ( STR_GRBL_BUF_MAX_SIZE - 1)) {
        if ( ( c =='-' || (c>='0' && c<='9' ) || c == '.' ) ) { 
          if ( getGrblPosState == GET_GRBL_STATUS_WPOS_DATA || getGrblPosState == GET_GRBL_STATUS_F_DATA || getGrblPosState == GET_GRBL_STATUS_BF_DATA || 
                  getGrblPosState == GET_GRBL_STATUS_WCO_DATA || getGrblPosState == GET_GRBL_STATUS_START || getGrblPosState == GET_GRBL_STATUS_CLOSED ||
                  getGrblPosState == GET_GRBL_STATUS_MESSAGE || getGrblPosState == GET_GRBL_STATUS_OV_DATA ){
            strGrblBuf[strGrblIdx++] = c ;
            strGrblBuf[strGrblIdx] = 0 ;
          }
        } else if ( ( (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ) && ( getGrblPosState == GET_GRBL_STATUS_START || getGrblPosState == GET_GRBL_STATUS_HEADER ||
                  getGrblPosState == GET_GRBL_STATUS_CLOSED || getGrblPosState == GET_GRBL_STATUS_MESSAGE  || getGrblPosState == GET_GRBL_STATUS_WPOS_HEADER ) ) { 
           strGrblBuf[strGrblIdx++] = c ;
           strGrblBuf[strGrblIdx] = 0 ;
        }
      }
    } // end switch 
    parseToLog(c , lastC ) ;
    lastC = c ;
  } // end while
  
  if ( (millis() - millisLastGetGBL ) > 2500 ) {           // if we did not get a GRBL status since 2500 ms, status become "?"
    machineStatus[0] = '?' ; machineStatus[1] = '?' ; machineStatus[2] = 0 ; 
    //Serial.print( "force reset ") ; Serial.print( millis()) ; Serial.print( " LG> = ") ; Serial.print( millis()) ;
    millisLastGetGBL = millis() ;
    newGrblStatusReceived = true ;                            // force a redraw if on info screen
    
    //Serial2.println( (char) 0x18) ;                             // force a soft reset of grbl
    
  }
}

void parseToLog(uint8_t c , uint8_t lastC) {   // do not store in log the OK and the status message.
  //Serial.print("To Parse "); Serial.print( (char) c , HEX) ; Serial.print(" , "); Serial.println( (char) c) ;
  if ( getGrblPosState == GET_GRBL_STATUS_CLOSED ) {
    if (c == 'o' && lastC != 'o') {
      return ; // skip 'o' because next char can be K (for an OK) 
    }
    if ( lastC == 'o' ) {
      if ( c == 'k' ) {
        return ; // skip k as part of OK
      } else {
        logBufferWrite(lastC) ; // save previous 'o' when not part of OK
      }  
    } 
    if ( c == '>' ) {
      return ; // skip '>' as begin of status
    }
    if ( c == '\r' ) {
      if ( lastC == '>' || lastC == 'k' ) {
        // skip return after a status line or a OK
      } else {
        logBufferWrite( BUFFER_EOL) ; // replace "new line " by 0 for string handling
      }  
    } else {
      logBufferWrite( c) ;
    } 
  } else if ( getGrblPosState == GET_GRBL_STATUS_MESSAGE ) {
    logBufferWrite( c) ;
  } 
}

void handleLastNumericField(void) { // decode last numeric field
  float temp = atof (&strGrblBuf[0]) ;
  if (  getGrblPosState == GET_GRBL_STATUS_WPOS_DATA && wposIdx < 4) {
          if ( wposOrMpos == 'W') {                  // we got a WPos
            wposXYZA[wposIdx] = temp ;
            mposXYZA[wposIdx] = wposXYZA[wposIdx] + wcoXYZA[wposIdx] ;
          } else {                                   // we got a MPos
            mposXYZA[wposIdx] = temp ;
            wposXYZA[wposIdx] = mposXYZA[wposIdx] - wcoXYZA[wposIdx] ;
          }
          wposIdx++ ;
          strGrblIdx = 0 ; 
  } else if (  getGrblPosState == GET_GRBL_STATUS_F_DATA && fsIdx < 2) {
          feedSpindle[fsIdx++] = temp ;
          strGrblIdx = 0 ;
  } else if (  getGrblPosState == GET_GRBL_STATUS_BF_DATA && bfIdx < 2) {   // save number of available block or char in GRBL buffer
          bufferAvailable[bfIdx++] = temp ;
          strGrblIdx = 0 ; 
  } else if (  getGrblPosState == GET_GRBL_STATUS_WCO_DATA && wcoIdx < 4) {
          wcoXYZA[wcoIdx] = temp ;
          if ( wposOrMpos == 'W') {                  // we previously had a WPos so we update MPos
            mposXYZA[wcoIdx] = wposXYZA[wcoIdx] + wcoXYZA[wcoIdx] ;  
          } else {                                   // we previously had a MPos so we update WPos
            wposXYZA[wcoIdx] = mposXYZA[wcoIdx] - wcoXYZA[wcoIdx] ;
          }
          
          wcoIdx++ ;
          strGrblIdx = 0 ;
  } else if (  getGrblPosState == GET_GRBL_STATUS_OV_DATA && ovIdx < 3) {   // save number of available block or char in GRBL buffer
          overwritePercent[ovIdx++] = temp ;
          strGrblIdx = 0 ; 
  } 
}

void storeGrblState(void) { // search for some char in message
  char * pBuf =  strGrblBuf + 5 ;
  char * pSearch ;
  char * pEndNumber1 ; // point to the first ','
  char * pEndNumber2 ; // point to the second ','
  if ( strGrblBuf[1] == 'G' && strGrblBuf[2] == 'C' && strGrblBuf[3] == ':'  ) { // GRBL reply to a $G with [GC:G20 ....]
    pSearch = strstr( pBuf , "G2" ) ; // can be G20 or G21
    if (pSearch != NULL ) memcpy( modalAbsRel , pSearch , 3) ;
    pSearch = strstr( pBuf , "G90" ) ;
    if (pSearch != NULL ) memcpy( modalMmInch , pSearch , 3) ;
    pSearch = strstr( pBuf , "G91" ) ;
    if (pSearch != NULL ) memcpy( modalMmInch , pSearch , 3) ;
    //Serial.print("AbsRel= " ) ; Serial.println( modalAbsRel) ;
    //Serial.print("MmInch= " ) ; Serial.println( modalMmInch) ;  
  } else if (strGrblBuf[1] == 'G' && strGrblBuf[2] == '3' && strGrblBuf[3] == '0'  ) {  // GRBL reply to $# with e.g. [G28:1.000,2.000,0.000] or [G30:4.000,6.000,0.000]
    pEndNumber1 = strstr(pBuf , ",") ; // find the position of the first ','
    if (pEndNumber1 != NULL ) { 
      memcpy( G30SavedX , pBuf , pEndNumber1 - pBuf) ;
      G30SavedX[pEndNumber1 - pBuf] = 0 ;
      pEndNumber1++ ; // point to the first char of second number
      pEndNumber2 = strstr(pEndNumber1 , ",") ; // find the position of the second ','
      if (pEndNumber2 != NULL ) {
        memcpy( G30SavedY , pEndNumber1 , pEndNumber2 - pEndNumber1) ;
        G30SavedY[pEndNumber2 - pEndNumber1] = 0 ;
      }
    }  
  }
}

void resetWaitOkWhenSdMillis() {    // after a resume (after a pause), we reset this time to avoid wrong warning
  waitOkWhenSdMillis = millis()  + WAIT_OK_SD_TIMEOUT ;  // wait for 2 min before generating the message again
}

//-----------------------------  Send ------------------------------------------------------
void sendToGrbl( void ) {   
                                    // set statusPrinting to PRINTING_STOPPED if eof; exit if we wait an OK from GRBL; 
                                    // if statusPrinting = PRINTING_FROM_USB or PRINTING_FRM_TELNET, then get char from USB or Telnet and forward it to GRBL.
                                    // if statusPrinting = PRINTING_FROM_SD, PRINTING_CMD or PRINTING_STRING, get the char and send
                                    // if statusprinting is NOT PRINTING_FROM_USB or TELNET, then send "?" to grbl to ask for status and position every x millisec
  int sdChar ;
  static uint32_t nextSendMillis = 0 ;
  uint32_t currSendMillis  ;
  if ( waitOk && ( statusPrinting == PRINTING_FROM_SD || statusPrinting == PRINTING_CMD || statusPrinting == PRINTING_STRING ) ) {
      if ( millis() > waitOkWhenSdMillis ) {
        fillMsg(__MISSING_OK_WHEN_SENDING_FROM_SD ) ;   // give an error if we have to wait to much to get an OK from grbl
        waitOkWhenSdMillis = millis()  + WAIT_OK_SD_TIMEOUT ;  // wait for 2 min before generating the message again
      }
  } else {
      switch ( statusPrinting ) {
          case PRINTING_FROM_SD :
            sendFromSd() ;
            break; 
          case PRINTING_FROM_USB :
            while ( Serial.available() && statusPrinting == PRINTING_FROM_USB ) {
              sdChar = Serial.read() ;
              Serial2.print( (char) sdChar ) ;
            } // end while 
            break ;
          case PRINTING_FROM_TELNET :
            while ( telnetClient.available() && statusPrinting == PRINTING_FROM_TELNET ) {
              sdChar = telnetClient.read() ;
              Serial2.print( (char) sdChar ) ;
            } // end while       
            break ;
          case PRINTING_CMD :
            sendFromCmd() ;
            break ;
          case PRINTING_STRING :
            sendFromString() ;
            break ;
      } // end switch
  } // end else if  
  if ( statusPrinting == PRINTING_STOPPED || statusPrinting == PRINTING_PAUSED ) {   // process nunchuk cancel and commands
     sendJogCancelAndJog() ;
  }  // end of nunchuk process
  if ( statusPrinting != PRINTING_FROM_USB && statusPrinting != PRINTING_FROM_TELNET) {     // when PC is master, it is the PC that asks for GRBL status
    currSendMillis = millis() ;                   // ask GRBL current status every X millis sec. GRBL replies with a message with status and position
    if ( currSendMillis > nextSendMillis) {
       nextSendMillis = currSendMillis + 300 ;
       Serial2.print("?") ; 
    }
  }
  if( statusPrinting != PRINTING_FROM_TELNET ) {               // clear the telnet buffer when not in use
    while ( telnetClient.available() && statusPrinting != PRINTING_FROM_TELNET ) {
      sdChar = telnetClient.read() ;
    } // end while  
  }
}  

void sendFromSd() {        // send next char from SD; close file at the end
      int sdChar ;
      waitOkWhenSdMillis = millis()  + WAIT_OK_SD_TIMEOUT ;  // set time out on 
      while ( aDir[dirLevel+1].available() > 0 && (! waitOk) && statusPrinting == PRINTING_FROM_SD && Serial2.availableForWrite() > 2 ) {
          sdChar = aDir[dirLevel+1].read() ;
          if ( sdChar < 0 ) {
            statusPrinting = PRINTING_ERROR  ;
            updateFullPage = true ;           // force to redraw the whole page because the buttons haved changed
          } else {
            sdNumberOfCharSent++ ;
            if( sdChar != 13 && sdChar != ' ' ){             // 13 = carriage return; do not send the space.
                                                             // to do : skip the comments
              Serial2.print( (char) sdChar ) ;
            }
            if ( sdChar == '\n' ) {        // n= new line = line feed = 10 decimal
               waitOk = true ;
            }
          }
      } // end while
      if ( aDir[dirLevel+1].available() == 0 ) { 
        aDir[dirLevel+1].close() ; // close the file when all bytes have been sent.
        statusPrinting = PRINTING_STOPPED  ; 
        updateFullPage = true ;           // force to redraw the whole page because the buttons haved changed
        //Serial2.print( (char) 0x18 ) ; //0x85) ;   // cancel jog (just for testing); must be removed
        Serial2.print( (char) 10 ) ; // sent a new line to be sure that Grbl handle last line.
      }
} 

void sendFromCmd() {
    int sdChar ;
    waitOkWhenSdMillis = millis()  + WAIT_OK_SD_TIMEOUT ;  // set time out on 
    while ( spiffsAvailableCmdFile() > 0 && (! waitOk) && statusPrinting == PRINTING_CMD && Serial2.availableForWrite() > 2 ) {
      sdChar = (int) spiffsReadCmdFile() ;
      if( sdChar != 13){
          Serial2.print( (char) sdChar ) ;
        }
      if ( sdChar == '\n' ) {
           waitOk = true ;
        }
    } // end while
    if ( spiffsAvailableCmdFile() == 0 ) { 
      statusPrinting = PRINTING_STOPPED  ; 
      updateFullPage = true ;           // force to redraw the whole page because the buttons haved changed
      Serial2.print( (char) 0x0A ) ; // sent a new line to be sure that Grbl handle last line.
    }      
}

void sendFromString(){
    char strChar ;   
    float savedWposXYZA[4] ; 
    waitOkWhenSdMillis = millis()  + WAIT_OK_SD_TIMEOUT ;  // set time out on 
    while ( *pPrintString != 0 && (! waitOk) && statusPrinting == PRINTING_STRING  ) {
      strChar = *pPrintString ++;
      if ( strChar == '%' ) {
         strChar = *pPrintString ++;
         switch (strChar) { 
         case 'z' : // save Z WCO
            //savedWposXYZA[2] = wposXYZA[2] ;
            preferences.putFloat("wposZ" , wposXYZA[2] ) ;
            //Serial.print( "wpos Z is saved with value = ") ; Serial.println( wposXYZA[2] ) ; // to debug
            break;
         case 'X' : // Put the G30 X offset
            //Serial.print("G30SavedX"); Serial.println(G30SavedX);
            Serial2.print(G30SavedX) ;
            break;
         case 'Y' : // Put the G30 Y offset
            Serial2.print(G30SavedY) ;
            break;
         case 'Z' : // Put some char in the flow
            savedWposXYZA[2] = preferences.getFloat("wposZ" , 0 ) ; // if wposZ does not exist in preferences, the function returns 0
            char floatToString[20] ;
            gcvt(savedWposXYZA[2], 3, floatToString); // convert float to string
            Serial2.print(floatToString) ;
            ///Serial.print( "wpos Z is retrieved with value = ") ; Serial.println( floatToString ) ; // to debug
            break;
         case 'M' : // Restore modal G20/G21/G90/G91
            Serial2.print( modalAbsRel) ;
            //Serial.print( modalAbsRel) ; // to debug
            Serial2.print( modalMmInch) ;
            //Serial.print( modalMmInch) ; // to debug
            break;
         }   
      } else {
        if( strChar != 13){                  // add here handling of special character for real time process; we skip \r char
            Serial2.print( strChar ) ;
            //Serial.print (strChar) ;  // to debug
          }
        if ( strChar == '\n' ) {
             waitOk = true ;
          }
      }    
    } // end while
    //Serial.println("End while"); // to debug
    //Serial.println( *printString , HEX );
    if ( *pPrintString == 0 ) { // we reached the end of the string 
      statusPrinting = PRINTING_STOPPED  ; 
      fillStringExecuteMsg( lastStringCmd );   // fill with a message saying the command has been executed
      updateFullPage = true ;           // force to redraw the whole page because the buttons haved changed
      Serial2.print( (char) 0x0A ) ; // sent a new line to be sure that Grbl handle last line.
      //Serial.println("last char has been sent") ;
    }      
}

void sendJogCancelAndJog(void) {
    static uint32_t exitMillis ;
    if ( jogCancelFlag ) {
      if ( jog_status == JOG_NO ) {
        //Serial.println("send a jog cancel");
        Serial2.print( (char) 0x85) ; Serial2.print("G4P0") ; Serial2.print( (char) 0x0A) ;    // to be execute after a cancel jog in order to get an OK that says that grbl is Idle.
        while (Serial2.availableForWrite() != 0x7F ) ;                        // wait that all char are sent 
        //Serial2.flush() ;             // wait that all outgoing char are really sent.!!! in ESP32 it also clear the RX buffer what is not expected in arduino logic
        
        waitOk = true ;
        jog_status = JOG_WAIT_END_CANCEL ;
        exitMillis = millis() + 500 ; //expect a OK before 500 msec
        //Serial.println(" send cancel code");     
      } else if ( jog_status == JOG_WAIT_END_CANCEL  ) {
        if ( !waitOk ) {
          jog_status = JOG_NO ;
          jogCancelFlag = false ;
           
        } else {
          if ( millis() >  exitMillis ) {  // si on ne reçoit pas le OK dans le délai maximum prévu
            jog_status = JOG_NO ; // reset all parameters related to jog .
            jogCancelFlag = false ;
            jogCmdFlag = false ;
            //if(lastMsg[0] || (lastMsg[0] == 32) ) fillMsg( __CAN_JOG_MISSING_OK  ) ; // put a message if there was no message (e.g. alarm:)
          }
        }
      } 
    } // end of jogCancelFlag
    
    if ( jogCmdFlag ) {
      if ( jog_status == JOG_NO ) {
        //Serial.println( bufferAvailable[0] ) ;
        if (bufferAvailable[0] > 15) {    // tests shows that GRBL gives errors when we fill to much the block buffer
          if ( sendJogCmd(startMoveMillis) ) { // if command has been sent
            waitOk = true ;
            jog_status = JOG_WAIT_END_CMD ;
            exitMillis = millis() + 500 ; //expect a OK before 500 msec
          }
        }  
              
      } else if ( jog_status == JOG_WAIT_END_CMD  ) {
        if ( !waitOk ) {
          jog_status = JOG_NO ;
          jogCmdFlag = false ;    // remove the flag because cmd has been processed
        } else {
          if ( millis() >  exitMillis ) {  // si on ne reçoit pas le OK dans le délai maximum prévu
            jog_status = JOG_NO ; // reset all parameters related to jog .
            jogCancelFlag = false ;
            jogCmdFlag = false ;
            if(lastMsg[0] || (lastMsg[0] == 32) ) fillMsg( __CMD_JOG_MISSING_OK  ) ; // put a message if there was no message (e.g. alarm:)
          }
        }
      } 
    }
} // end of function    





boolean sendJogCmd(uint32_t startTime) {
#define MINDIST 0.01    // mm
#define MINSPEED 10     // mm
#define MAXSPEEDXY MAX_XY_SPEED_FOR_JOGGING // mm/sec
#define MAXSPEEDZ 400   // mm/sec
#define DELAY_BEFORE_REPEAT_MOVE 500 //msec
#define DELAY_BETWEEN_MOVE 100       //msec
#define DELAY_TO_REACH_MAX_SPEED 2000 // msec
        float distanceMove ;
        uint32_t speedMove ;
        int32_t counter = millis() - startTime ;
        if ( counter < 10 ) {
          distanceMove = MINDIST ;
          speedMove = MINSPEED ;
        } else {
          counter = counter - DELAY_BEFORE_REPEAT_MOVE ;
          if (counter < 0) {
            //Serial.println("counter neg");
            return false ;              // do not send a move; // false means that cmd has not been sent
          }
          if ( counter > (  DELAY_TO_REACH_MAX_SPEED - DELAY_BEFORE_REPEAT_MOVE) ) {
            counter = DELAY_TO_REACH_MAX_SPEED - DELAY_BEFORE_REPEAT_MOVE ;
          }
          if (jogDistZ ) {
            speedMove = MAXSPEEDZ ;
          } else {
            speedMove = MAXSPEEDXY ;
          } 
          speedMove = speedMove * counter / ( DELAY_TO_REACH_MAX_SPEED  - DELAY_BEFORE_REPEAT_MOVE ) ;
          if (speedMove < MINSPEED) {
            speedMove = MINSPEED;
          }
          distanceMove = speedMove * DELAY_BETWEEN_MOVE / 60000.0 * 1.2;   // speed is in mm/min and time in millisec.  1.2 is to increase a little the distance to be sure buffer is filled 
        }
        //
        //Serial.println("send a jog") ;  
        Serial2.print("$J=G91 G21") ;
        if (jogDistX > 0) {
          Serial2.print(" X") ;
        } else if (jogDistX ) {
          Serial2.print(" X-") ;
        }
        if (jogDistX ) {
          Serial2.print(distanceMove) ;
        }  
        if (jogDistY > 0) {
          Serial2.print(" Y") ;
        } else if (jogDistY ) {
          Serial2.print(" Y-") ;
        }
        if (jogDistY ) {
          //Serial2.print(moveMultiplier) ;
          Serial2.print(distanceMove) ;
        }
        if (jogDistZ > 0) {
          Serial2.print(" Z") ;
        } else if (jogDistZ ) {
          Serial2.print(" Z-") ;
        }
        if (jogDistZ ) {
          Serial2.print(distanceMove) ;
        }
        if (jogDistA > 0) {
          Serial2.print(" A") ;
        } else if (jogDistA ) {
          Serial2.print(" A-") ;
        }
        if (jogDistA ) {
          Serial2.print(distanceMove) ;
        }
        //Serial2.print(" F2000");  Serial2.print( (char) 0x0A) ;
        Serial2.print(" F"); Serial2.print(speedMove); Serial2.print( (char) 0x0A) ;
        while (Serial2.availableForWrite() != 0x7F ) ;                        // wait that all char are sent 
        //Serial2.flush() ;       // wait that all char are really sent
        
        //Serial.print("Send cmd jog " ); Serial.print(distanceMove) ; Serial.print(" " ); Serial.print(speedMove) ;Serial.print(" " ); Serial.println(millis() - startTime );
        //Serial.print(prevMoveX) ; Serial.print(" " ); Serial.print(prevMoveY) ; Serial.print(" " ); Serial.print(prevMoveZ) ;Serial.print(" ") ; Serial.println(millis()) ;
        return true ; // true means that cmd has been sent
}

const char * errorArrayMsg[] = { __UNKNOWN_ERROR  , 
              __EXPECTED_CMD_LETTER ,
              __BAD_NUMBER_FORMAT ,
              __INVALID_$_SYSTEM_CMD ,
              __NEGATIVE_VALUE ,
              __HOMING_NOT_ENABLED ,
              __STEP_PULSE_LESS_3_USEC ,
              __EEPROM_READ_FAIL ,
              __$_WHILE_NOT_IDLE ,
              __LOCKED_ALARM_OR_JOG ,
              __SOFT_LIMIT_NO_HOMING ,
              __LINE_OVERFLOW ,
              __STEP_RATE_TO_HIGH ,
              __SAFETY_DOOR_DETECTED ,
              __LINE_LENGHT_EXCEEDED ,
              __JOG_TRAVEL_EXCEEDED ,
              __INVALID_JOG_COMMANF ,
              __LASER_REQUIRES_PWM ,
              __UNKNOWN_ERROR  ,
              __UNKNOWN_ERROR ,
              __UNSUPPORTED_COMMAND ,
              __MODAL_GROUP_VIOLATION ,
              __UNDEF_FEED_RATE ,
              __CMD_REQUIRES_INTEGER ,
              __SEVERAL_AXIS_GCODE ,
              __REPEATED_GCODE ,
              __AXIS_MISSING_IN_GCODE ,
              __INVALID_LINE_NUMBER ,
              __VALUE_MISSING_IN_GCODE ,
              __G59_WCS_NOT_SUPPORTED ,
              __G53_WITHOUT_G01_AND_G1 ,
              __AXIS_NOT_ALLOWED ,
              __G2_G3_REQUIRE_A_PLANE ,
              __INVALID_MOTION_TARGET ,
              __INVALID_ARC_RADIUS ,
              __G2_G3_REQUIRE_OFFSET ,
              __UNSUSED_VALUE ,
              __G431_TOOL_LENGTH ,
              __TOOL_NUMBER_EXCEED_MAX 
};

const char * alarmArrayMsg[] = { __UNKNOWN_ALARM  , 
              __HARD_LIMIT_REACHED ,
              __MOTION_EXCEED_CNC ,
              __RESET_IN_MOTION ,
              __PROBE_INIT_FAIL ,
              __PROBE_TRAVEL_FAIL  ,
              __RESET_DURING_HOMING ,
              __DOOR_OPEN_HOMING ,
              __LIMIT_ON_HOMING ,
              __LIMIT_MISSING_HOMING 
}; 

const char * stringExecuteMsg[] = { __SETX_EXECUTED  ,  // messages must be defined here in the same sequence as the buttons used to call the string commands 
                              __SETY_EXECUTED  ,
                              __SETZ_EXECUTED  ,
                              __SETA_EXECUTED  ,
                              __SETXYZ_EXECUTED ,
                              __SETXYZA_EXECUTED ,
                              __SET_CHANGE_EXECUTED ,
                              __SET_PROBE_EXECUTED  ,
                              __SET_CAL_EXECUTED    ,
                              __GO_CHANGE_EXECUTED  ,
                              __GO_PROBE_EXECUTED ,
                              __UNKNOWN_BTN_EXECUTED
                              
};

void fillErrorMsg( const char * errorMsg ) {   // errorMsg contains "Error:xx"
   int errorNum = atoi( &errorMsg[6]) ;
   if (errorNum < 1 || errorNum > 38 ) errorNum = 0 ;
   fillMsg( errorArrayMsg[errorNum]  ) ;
}
void fillAlarmMsg( const char * alarmMsg ) {   //alarmMsg contains "ALARM:xx"
  int alarmNum = atoi( &alarmMsg[6]) ;
   if (alarmNum < 1 || alarmNum > 9 ) alarmNum = 0 ;
   fillMsg( alarmArrayMsg[alarmNum] ) ;
}

void fillStringExecuteMsg( uint8_t buttonMessageIdx ) {   // buttonMessageIdx contains the number of the button
   //Serial.print("param= ") ; Serial.println(buttonMessageIdx ) ;  // to debug
   if ( buttonMessageIdx >= _SETX || buttonMessageIdx <= _GO_PROBE) {
      buttonMessageIdx -= _SETX ;
      fillMsg( stringExecuteMsg[buttonMessageIdx] , BUTTON_TEXT ) ;
   } else {
      buttonMessageIdx = _GO_PROBE - _SETX + 1 ;
      fillMsg( stringExecuteMsg[buttonMessageIdx] ) ;
   }
   //Serial.print("index of table= ") ; Serial.println(buttonMessageIdx ) ;  // to debug
   //Serial.print("message= ") ; Serial.println( stringExecuteMsg[buttonMessageIdx] ) ; // to debug
}

