#include "com.h"
#include "config.h"
//#include "SD.h"
#include "SdFat.h"
#include "draw.h"
#include "nunchuk.h"

// GRBL status are : Idle, Run, Hold, Jog, Alarm, Door, Check, Home, Sleep
// a message should look like (note : GRBL sent or WPOS or MPos depending on grbl parameter : to get WPos, we have to set "$10=0"
//  <Idle|WPos:0.000,0.000,0.000|FS:0.0,0> or e.g. <Idle|MPos:0.000,0.000,0.000|FS:0.0,0|WCO:0.000,0.000,0.000>
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

uint8_t wposIdx = 0 ;
uint8_t wcoIdx = 0 ;
uint8_t fsIdx = 0 ;
uint8_t getGrblPosState = GET_GRBL_STATUS_CLOSED ;
float feedSpidle[2] ;
float wcoXYZ[3] ;
float wposXYZ[3] ; 
float mposXYZ[3] ;

#define STR_GRBL_BUF_MAX_SIZE 10 
char strGrblBuf[STR_GRBL_BUF_MAX_SIZE] ; // this buffer is used to store a few char received from GRBL before decoding them
uint8_t strGrblIdx ;

extern int8_t prevMoveX ;
extern int8_t prevMoveY ;
extern int8_t prevMoveZ ;
extern float moveMultiplier ;
// used by nunchuck
extern uint8_t jog_status  ;
extern boolean jogCancelFlag ;
extern boolean jogCmdFlag  ; 

extern volatile boolean waitOk ;
extern boolean newGrblStatusReceived ;
extern volatile uint8_t statusPrinting  ;
extern char machineStatus[9];
extern char lastMsg[23] ;        // last message to display

extern boolean updateFullPage ;

//        Pour sendToGrbl
//extern File fileToRead ;
extern SdBaseFile aDir[DIR_LEVEL_MAX] ; 
extern int8_t dirLevel ;
extern uint8_t cmdToSend ; // cmd to be send
extern uint32_t sdNumberOfCharSent ;

// ----------------- fonctions pour lire de GRBL -----------------------------
void getFromGrblAndForward( void ) {   //get char from GRBL, forward them if statusprinting = PRINTING_FROM_PC and decode the data (look for "OK", for <xxxxxx> sentence
                                       // fill machineStatus[] and wposXYZ[]
  static uint8_t c;
  static uint8_t lastC = 0 ;
  static uint32_t millisLastGetGBL = 0 ;
  uint8_t i = 0 ;
  static int cntOk = 0 ; 
  while (Serial2.available() ) {
#ifdef DEBUG_TO_PC
    //Serial.print(F("s=")); Serial.print( getGrblPosState ); Serial.println() ;
#endif    
    c=Serial2.read() ;
#define DEBUG_RECEIVED_CHAR
#ifdef DEBUG_RECEIVED_CHAR      
      Serial.print( (char) c) ; 
      //if  (c == 0x0A || c == 0x0C ) Serial.println(millis()) ;
#endif      
    if ( statusPrinting == PRINTING_FROM_PC ) {
      Serial.print( (char) c) ;                         // forward characters from GRBL to PC when PRINTING_FROM_PC
    }
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
        if ( ( strGrblBuf[0] == 'e' && strGrblBuf[1] == 'r' ) || ( strGrblBuf[0] == 'A' && strGrblBuf[1] == 'L' ) ) {  // we got an error message or an ALARM message
          memccpy( lastMsg , strGrblBuf , '\0', 22);           // save the error or ALARM
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
      Serial.print(" <") ; 
#endif      
      break ;
      
    case '>' :                     // end of grbl status 
      handleLastNumericField() ;  //wposXYZ[wposIdx] = atof (&strGrblBuf[0]) ;
      getGrblPosState = GET_GRBL_STATUS_CLOSED ;
      strGrblIdx = 0 ;
      strGrblBuf[strGrblIdx] = 0 ;
      newGrblStatusReceived = true;
#ifdef DEBUG_TO_PC
      //Serial.print("X=") ; Serial.print(wposXYZ[0]) ; Serial.print(" Y=") ; Serial.print(wposXYZ[1]) ;Serial.print(" Z=") ; Serial.println(wposXYZ[2]) ; 
      Serial.println(">");
#endif      
      break ;
    case '|' :
      if ( getGrblPosState == GET_GRBL_STATUS_START ) {
         getGrblPosState = GET_GRBL_STATUS_WPOS_HEADER ; 
         memccpy( machineStatus , strGrblBuf , '\0', 9);        
#ifdef DEBUG_TO_PC
         //Serial.print( "ms= " ) ; Serial.println( machineStatus ) ;
#endif          
      } else { 
        handleLastNumericField() ;
        getGrblPosState = GET_GRBL_STATUS_HEADER ;
        strGrblIdx = 0 ;  
      }
      break ;
    case ':' :
      if ( getGrblPosState == GET_GRBL_STATUS_WPOS_HEADER ) { // separateur entre field name et value 
         getGrblPosState = GET_GRBL_STATUS_WPOS_DATA ; 
         strGrblIdx = 0 ;
         strGrblBuf[strGrblIdx] = 0 ;
         wposIdx = 0 ;
      } else if ( (getGrblPosState == GET_GRBL_STATUS_START || getGrblPosState == GET_GRBL_STATUS_CLOSED )&& strGrblIdx < (STR_GRBL_BUF_MAX_SIZE - 1) ) {    // save the : as part of the text (for error 
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
        }  
      }
      break ;
    case ',' :                                              // séparateur entre 2 chiffres
      handleLastNumericField() ;                            // check that we are in data to be processed; if processed, reset strGrblIdx = 0 ; 
      break ;  
    default :
      if (  strGrblIdx < ( STR_GRBL_BUF_MAX_SIZE - 1)) {
        if ( ( c =='-' || (c>='0' && c<='9' ) || c == '.' ) ) { 
          if ( getGrblPosState == GET_GRBL_STATUS_WPOS_DATA || getGrblPosState == GET_GRBL_STATUS_F_DATA || getGrblPosState == GET_GRBL_STATUS_WCO_DATA || getGrblPosState == GET_GRBL_STATUS_START || getGrblPosState == GET_GRBL_STATUS_CLOSED ){
            strGrblBuf[strGrblIdx++] = c ;
            strGrblBuf[strGrblIdx] = 0 ;
          }
        } else if ( ( (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ) && ( getGrblPosState == GET_GRBL_STATUS_START || getGrblPosState == GET_GRBL_STATUS_HEADER || getGrblPosState == GET_GRBL_STATUS_CLOSED ) ) { 
           strGrblBuf[strGrblIdx++] = c ;
           strGrblBuf[strGrblIdx] = 0 ;
        }
      }
    } // end switch 
    lastC = c ;
  } // end while
  
  if ( (millis() - millisLastGetGBL ) > 5500 ) {           // if we did not get a GRBL status since 5500 ms, status become "?"
    machineStatus[0] = '?' ; machineStatus[1] = '?' ; machineStatus[2] = 0 ; 
    //Serial.print( "force reset ") ; Serial.print( millis()) ; Serial.print( " LG> = ") ; Serial.print( millis()) ;
    millisLastGetGBL = millis() ;
    newGrblStatusReceived = true ;                            // force a redraw if on info screen
    
    //Serial2.println( (char) 0x18) ;                             // force a soft reset of grbl
    
  }
}


void handleLastNumericField(void) { // decode last numeric field
  float temp = atof (&strGrblBuf[0]) ;
  if (  getGrblPosState == GET_GRBL_STATUS_WPOS_DATA && wposIdx < 3) {
          wposXYZ[wposIdx] = temp ;
          mposXYZ[wposIdx] = wposXYZ[wposIdx] + wcoXYZ[wposIdx] ;
          wposIdx++ ;
          strGrblIdx = 0 ; 
  } else if (  getGrblPosState == GET_GRBL_STATUS_F_DATA && fsIdx < 2) {
          feedSpidle[fsIdx++] = temp ;
          strGrblIdx = 0 ; 
  } else if (  getGrblPosState == GET_GRBL_STATUS_WCO_DATA && wcoIdx < 3) {
          wcoXYZ[wcoIdx] = temp ;
          mposXYZ[wcoIdx] = wposXYZ[wcoIdx] + wcoXYZ[wcoIdx] ; //modifié pour mettre à jour en fonction de wcoIdx au lieu de wposIdx
          wcoIdx++ ;
          strGrblIdx = 0 ;
  } 
}

//-----------------------------  Send ------------------------------------------------------
void sendToGrbl( void ) {   
                                    // if statusprinting = PRINTING, then set statusPrinting to PRINTING_STOPPED if eof; exit if we wait an OK from GRBL; 
                                    // if statusprinting = PRINTING_FROM_PC, then get char from PC and forward it to GRBL.
                                    // if statusprinting is NOT PRINTING_FROM_PC, then send "?" to grbl to ask for status and position every x millisec
  int sdChar ;
  static uint32_t nextSendMillis = 0 ;
  uint32_t currSendMillis  ;
  static uint32_t exitMillis ;
 
  if ( statusPrinting == PRINTING_FROM_SD) {
    while ( aDir[dirLevel+1].available() > 0 && (! waitOk) && statusPrinting == PRINTING_FROM_SD && Serial2.availableForWrite() > 2 ) {
      sdChar = aDir[dirLevel+1].read() ;
      if ( sdChar < 0 ) {
        statusPrinting = PRINTING_STOPPED  ;
        updateFullPage = true ;           // force to redraw the whole page because the buttons haved changed
      } else {
        sdNumberOfCharSent++ ;
        if( sdChar != 13){
          Serial2.print( (char) sdChar ) ;
        }
        if ( sdChar == '\n' ) {
           waitOk = true ;
        }
      }
    } // end while
    if ( aDir[dirLevel+1].available() == 0 ) { 
      statusPrinting = PRINTING_STOPPED  ; 
      updateFullPage = true ;           // force to redraw the whole page because the buttons haved changed
      Serial2.print( (char) 0x10 ) ; // sent a new line to be sure that Grbl handle last line.
    }
  } else if ( statusPrinting == PRINTING_FROM_PC ) {
    while ( Serial.available() && statusPrinting == PRINTING_FROM_PC ) {
      sdChar = Serial.read() ;
      Serial2.print( (char) sdChar ) ;
    } // end while       
  } else if ( statusPrinting == PRINTING_CMD ) {
    if ( cmdToSend >= 1 && cmdToSend <= 7 ) {
      switch ( cmdToSend ) {
      case 1 :
        Serial2.print( CMD1_GRBL_CODE ) ;
        break ;
      case 2 :
        Serial2.print( CMD2_GRBL_CODE ) ;
        break ;
      case 3 :
        Serial2.print( CMD3_GRBL_CODE ) ;
        break ;
      case 4 :
        Serial2.print( CMD4_GRBL_CODE ) ;
        break ;
      case 5 :
        Serial2.print( CMD5_GRBL_CODE ) ;
        break ;
      case 6 :
        Serial2.print( CMD6_GRBL_CODE ) ;
        break ;
      case 7 :
        Serial2.print( CMD7_GRBL_CODE ) ;
        break ;
      } // end switch
      cmdToSend = 0 ;
      Serial2.print( '\n' ) ; // send end of line
      Serial2.flush() ;       // wait that all char are really sent
      delay (30) ; // wait for GRBL having sent "OK"  
    } // end if
    statusPrinting = PRINTING_STOPPED  ;
    //updateFullPage = true ; No need to redraw the page because we stay on the same page which is CMD page
  } // end else if  
  if ( statusPrinting == PRINTING_STOPPED || statusPrinting == PRINTING_PAUSED ) {   // process nunchuk cancel and commands
    if ( jogCancelFlag ) {
      if ( jog_status == JOG_NO ) {
        Serial2.print( (char) 0x85) ;  Serial2.print("G4P0") ; Serial2.print( (char) 0x0A) ;    // to be execute after a cancel jog in order to get an OK that says that grbl is Idle.
        Serial2.flush() ;             // wait that all outgoing char are really sent.
        waitOk = true ;
        jog_status = JOG_WAIT_END_CANCEL ;
        exitMillis = millis() + 500 ; //expect a OK before 500 msec
        Serial.println(" send cancel code");     
      } else if ( jog_status == JOG_WAIT_END_CANCEL  ) {
        if ( !waitOk ) {
          jog_status = JOG_NO ;
          jogCancelFlag = false ;
           
         } else {
          if ( millis() >  exitMillis ) {  // si on ne reçoit pas le OK dans le délai maximum prévu
            jog_status = JOG_NO ; // reset all parameters related to jog .
            jogCancelFlag = false ;
            jogCmdFlag = false ;
            if(lastMsg[0] ) memccpy( lastMsg , "Can.JOG:missing Ok" , '\0' , 22) ; // put a message if there was no message (e.g. alarm:)
          }
        }
      } 
    }
    if ( jogCmdFlag ) {
      if ( jog_status == JOG_NO ) {
        sendJogCmd() ;                
        waitOk = true ;
        jog_status = JOG_WAIT_END_CMD ;
        exitMillis = millis() + 500 ; //expect a OK before 500 msec      
      } else if ( jog_status == JOG_WAIT_END_CMD  ) {
        if ( !waitOk ) {
          jog_status = JOG_NO ;
          jogCmdFlag = false ;    // remove the flag because cmd has been processed
        } else {
          if ( millis() >  exitMillis ) {  // si on ne reçoit pas le OK dans le délai maximum prévu
            jog_status = JOG_NO ; // reset all parameters related to jog .
            jogCancelFlag = false ;
            jogCmdFlag = false ;
            if(lastMsg[0] ) memccpy( lastMsg , "Cmd JOG:missing Ok" , '\0' , 22) ; // put a message if there was no message (e.g. alarm:)
          }
        }
      } 
    }
     
  }  // end of nunchuk process
  
  if ( statusPrinting != PRINTING_FROM_PC ) {     // when PC is master, it is the PC that asks for GRBL status
    currSendMillis = millis() ;                   // ask GRBL current status every X millis sec. GRBL replies with a message with status and position
    if ( currSendMillis > nextSendMillis) {
       nextSendMillis = currSendMillis + 300 ;
       Serial2.print("?") ; 
    }
  }
}  

void sendJogCmd() {
        Serial2.print("$J=G91 G21") ;
        if (prevMoveX > 0) {
          Serial2.print(" X") ;
        } else if (prevMoveX ) {
          Serial2.print(" X-") ;
        }
        if (prevMoveX ) {
          Serial2.print(moveMultiplier) ;
        }  
        if (prevMoveY > 0) {
          Serial2.print(" Y") ;
        } else if (prevMoveY ) {
          Serial2.print(" Y-") ;
        }
        if (prevMoveY ) {
          Serial2.print(moveMultiplier) ;
        }
        if (prevMoveZ > 0) {
          Serial2.print(" Z") ;
        } else if (prevMoveZ ) {
          Serial2.print(" Z-") ;
        }
        if (prevMoveZ ) {
          Serial2.print(moveMultiplier) ;
        }
        Serial2.print(" F2000");  Serial2.print( (char) 0x0A) ;
        Serial2.flush() ;       // wait that all char are really sent
        
        Serial.print("Send cmd jog " ); Serial.print(moveMultiplier) ; Serial.print(" " ); 
        Serial.print(prevMoveX) ; Serial.print(" " ); Serial.print(prevMoveY) ; Serial.print(" " ); Serial.print(prevMoveZ) ;Serial.print(" ") ; Serial.println(millis()) ;
}

