#include "config.h"
#include "language.h"
#include "draw.h"
#include "TFT_eSPI_ms/TFT_eSPI.h"
#include "FS.h"
#include "actions.h"
#include "menu_file.h"
#include "telnet.h"
#include "cmd.h"
#include "com.h"

// create for touchscreeen
extern TFT_eSPI tft ;
extern uint8_t prevPage  ;     
extern uint8_t currentPage  ;
extern boolean updateFullPage ;
extern boolean updatePartPage ;
extern boolean waitReleased ;
extern uint8_t statusPrinting ;
extern char machineStatus[9];           // Iddle, Run, Alarm, ...
//extern char lastMsg[80]  ;
extern char grblLastMessage[STR_GRBL_BUF_MAX_SIZE] ;
extern boolean grblLastMessageChanged;


extern uint16_t firstFileToDisplay ;
extern uint16_t sdFileDirCnt ;

extern char cmdName[7][17] ;     // store the names of the commands
extern uint8_t cmdToSend ;
extern M_Page mPages[_P_MAX_PAGES] ;
extern uint8_t currentBtn ;
extern uint8_t justPressedBtn;
extern uint8_t justReleasedBtn;
extern uint8_t longPressedBtn;
extern uint32_t beginChangeBtnMillis ;
extern volatile boolean waitOk ;

//extern uint32_t cntSameMove ;
extern uint8_t jog_status  ;
extern boolean jogCancelFlag ;
extern boolean jogCmdFlag  ; 
extern uint32_t startMoveMillis ;
extern int8_t jogDistX ;
extern int8_t jogDistY ;
extern int8_t jogDistZ ;
extern float moveMultiplier ;

uint32_t prevAutoMoveMillis ;

#define SOFT_RESET 0x18 

void fGoToPage(uint8_t param) {
//  Serial.print( "go to page : " ) ; Serial.println( param) ; // just for testing // to do
  prevPage = currentPage ;
  currentPage = param ;
  updateFullPage = true ; 
  waitReleased = true ;          // discard "pressed" until a release 
//  delay(10000);
}

void fGoToPageAndClearMsg(uint8_t param) {
  fillMsg(" ") ;
  grblLastMessage[0] = 0 ; // clear grbl last message ([....]
  grblLastMessageChanged = true ;
  prevPage = currentPage ;
  currentPage = param ;
  updateFullPage = true ; 
  waitReleased = true ;          // discard "pressed" until a release 
}


void fGoBack(uint8_t param) {
  //Serial.print( "go back : " ) ; Serial.println( prevPage) ; // just for testing // to do
  
  if (prevPage >= _P_INFO && prevPage < _P_MAX_PAGES ) {       // if there are sevral back, prevPage = 0 and we will go back to _P_INFO
    currentPage = prevPage ;
  } else {
    currentPage = _P_INFO ;
    prevPage = _P_INFO ;
  }
  updateFullPage = true ;               
  waitReleased = true ;          // discard "pressed" until a release 
}

void fHome(uint8_t param) {
  if( machineStatus[0] == 'I' || machineStatus[0] == 'A' ) {
#define HOME_CMD "$H"
    Serial2.println(HOME_CMD) ;  
  } else {
    fillMsg(__INVALID_BTN_HOME ) ;
  }
  waitReleased = true ;          // discard "pressed" until a release 
}

void fUnlock(uint8_t param) {
  if( machineStatus[0] == 'A') {  // if grbl is in alarm
    Serial2.println("$X") ;    // send command to unlock
    //Serial.println("$X has been sent");
  }
// Stay on current page
  waitReleased = true ;          // discard "pressed" until a release 
}

void fReset(uint8_t param) {
  Serial2.print( (char) SOFT_RESET) ;
  waitReleased = true ;          // discard "pressed" until a release
  fillMsg( " " );
  
}

void fCancel(uint8_t param) {
  if( statusPrinting == PRINTING_FROM_SD || statusPrinting == PRINTING_PAUSED  ) {
    statusPrinting = PRINTING_STOPPED ;
    closeFileToRead() ;    
    Serial2.print( (char) SOFT_RESET) ;
  }  
  currentPage = _P_INFO ;  // go to page Info
  updateFullPage = true ;  // force a redraw even if current page does not change
  waitReleased = true ;          // discard "pressed" until a release 
}

void fPause(uint8_t param) {
  if( statusPrinting == PRINTING_FROM_SD  && ( machineStatus[0] == 'R' || machineStatus[0] == 'J' ) ) { // test on J added mainly for test purpose
  #define PAUSE_CMD "!" 
    Serial2.print(PAUSE_CMD) ;
    statusPrinting = PRINTING_PAUSED ;
    updateFullPage = true ;  // 
  }
  waitReleased = true ;          // discard "pressed" until a release   
}

void fResume(uint8_t param) {
  if( statusPrinting == PRINTING_PAUSED && machineStatus[0] == 'H') {
  #define RESUME_CMD "~" 
    Serial2.print(RESUME_CMD) ;
    statusPrinting = PRINTING_FROM_SD ;
    updateFullPage = true ;      // we have to redraw the buttons because Resume should become Pause
  }
  waitReleased = true ;          // discard "pressed" until a release 
}

void fDist( uint8_t param ) {
  uint8_t newDist =  mPages[_P_MOVE].boutons[3] ;       // convertit la position du bouton en type de bouton 
  //Serial.print("newDist=") ; Serial.println(newDist) ;
  if ( ++newDist > _D10 ) newDist = _D_AUTO ; // increase and reset to min value if to big
  mPages[_P_MOVE].boutons[3] = newDist ;   // update the button to display
  mButtonDraw( 4 , newDist ) ;  // draw a button at position (from 1 to 8)
  //updateFullPage = true ;                     // force a redraw of buttons
  waitReleased = true ;          // discard "pressed" until a release 
}  

void fMove( uint8_t param ) {
    float distance ;
    uint32_t moveMillis = millis() ;
    static uint32_t prevMoveMillis ;
    if ( mPages[_P_MOVE].boutons[3] == _D_AUTO ) {
      handleAutoMove(param) ; // process in a similar way as Nunchuk
    } else if (justPressedBtn) {                      // just pressed in non auto mode
      switch ( mPages[_P_MOVE].boutons[3] ) {         //  we suppose that the distance is defined by the 4th button on first line so idx = 3
      case _D0_01 :
        distance = 0.01;
        break ;
      case _D0_1 :
        distance = 0.1;
        break ;
      case _D1:
        distance = 1 ;
        break ;
      case _D10 :
        distance = 10 ;
        break ;
      }
      Serial2.println("") ; Serial2.print("$J=G91 G21 ") ;
      switch ( justPressedBtn ) {  // we convert the position of the button into the type of button
        case 1 :  Serial2.print("X")  ;  break ;
        case 5 :  Serial2.print("X-") ;  break ;
        case 2 :  Serial2.print("Y")  ;  break ;
        case 6 :  Serial2.print("Y-") ;  break ;
        case 3 :  Serial2.print("Z")  ;  break ;
        case 7 :  Serial2.print("Z-") ;  break ;
      }
      Serial2.print(distance) ; Serial2.println (" F100") ;
      //Serial.print("move for button") ; Serial.print(justPressedBtn) ;Serial.print(" ") ;  Serial.print(distance) ; Serial.println (" F100") ;
      
      updatePartPage = true ;                     // force a redraw of data
      waitReleased = true ;          // discard "pressed" until a release // todo change to allow repeated press on the same button
    }   
}

void handleAutoMove( uint8_t param) { // in Auto mode, we support long press to increase speed progressively, we cancel jog when released 
                                      // param contains the touch being pressed or the released if no touch has been pressed
#define AUTO_MOVE_REPEAT_DELAY 100
  uint8_t pressedBtn  = 0 ;
  static uint32_t cntSameAutoMove = 0 ;
  //float moveMultiplier ;
  uint32_t autoMoveMillis = millis() ;
  if ( justReleasedBtn )  {
      jogCancelFlag = true ;                                // cancel any previous move when a button is released (even before asking for another jog
      cntSameAutoMove = 0 ;             // reset the counter
       //Serial.println("cancel jog") ;
  } else {
      jogCancelFlag = false ;
  }
  if ( ( autoMoveMillis - prevAutoMoveMillis ) <  AUTO_MOVE_REPEAT_DELAY ) {
    return ;   
  }
  if ( justPressedBtn) {
    cntSameAutoMove = 0 ;                    // reset the counter when we just press the button
    pressedBtn = justPressedBtn ;
    prevAutoMoveMillis = autoMoveMillis ;
  } else if ( longPressedBtn  ) {
      pressedBtn = longPressedBtn ; 
      prevAutoMoveMillis = autoMoveMillis ;
    }
  if ( ( pressedBtn ) && (jogCmdFlag == false) ) {
    if (cntSameAutoMove == 0 ) { 
    //  moveMultiplier = 0.01 ; 
      startMoveMillis = millis();
    } 
    //else if (cntSameAutoMove < 5 ) {   // avoid to send to fast a new move
    //  moveMultiplier = 0.0 ;
    //} else if (cntSameAutoMove < 10 ) {
    //  moveMultiplier = 0.01 ;
    //} else if (cntSameAutoMove < 15 ) {
    //  moveMultiplier = 0.1 ;
    //} else if (cntSameAutoMove < 20 ) {
    //  moveMultiplier = 1 ;
    //} else {
    //  moveMultiplier = 2 ;
    //} 
    cntSameAutoMove++ ;
    jogDistX = 0 ;           // reset all deplacements
    jogDistY = 0 ;
    jogDistZ = 0 ;
    switch ( pressedBtn ) {  // fill one direction of move
      case 1 :  jogDistX = 1  ;  break ;
      case 5 :  jogDistX = -1 ;  break ;
      case 2 :  jogDistY = 1  ;  break ;
      case 6 :  jogDistY = -1 ;  break ;
      case 3 :  jogDistZ = 1 ;  break ;
      case 7 :  jogDistZ = -1 ;  break ;
    }
    jogCmdFlag = true ;                 // the flag will inform the send module that there is a command to be sent based on moveMultiplier and preMove. 
  }
}

void fSdFilePrint(uint8_t param ){   // lance l'impression d'un fichier; param contains l'index (0 à 3) du fichier à ouvrir
  //Serial.println("enter fsFilePrint") ;
  if ( ! setFileToRead( param ) ) {         // try to open the file to be printed ; in case of error, go back to Info page (lastMsg is already filled)
      //Serial.println("SetFileToRead is false") ;
      currentPage = _P_INFO ;
      updateFullPage = true ;
      waitReleased = true ;          // discard "pressed" until a release
      return ;
  }
  if ( fileToReadIsDir () ) {   // if user press a directory, then change the directory
    //Serial.println("FileToRead is dir") ;
    if ( ! changeDirectory() ) {
      //Serial.println("changeDirectory is false") ;
      currentPage = _P_INFO ;        // in case of error, goes to info page
      updateFullPage = true ;
      waitReleased = true ;          // discard "pressed" until a release
      return ;
    }                                // else = change dir is ok, button have been updated, screen must be reloaded
    updateFullPage = true ;
    waitReleased = true ;
    return ;
  } else if ( fileIsCmd() ) { //  when a "command" file is selected, it will not be executed it but will be saved in SPIFFS
                              // note : this function also fill LastMsg (e.g. with the file name when it is a file to be printed   
    updateFullPage = true ;
    waitReleased = true ;
    return ;
  } else {                    // file can be printed
    waitOk = false ;
    Serial2.print(PAUSE_CMD) ;
    delay(10);
    Serial2.print("?") ;
    //waitOk = false ; // do not wait for OK before sending char.
    statusPrinting = PRINTING_PAUSED ; // initially it was PRINTING_FROM_SD ; // change the status, so char will be read and sent in main loop
    prevPage = currentPage ;            // go to INFO page
    currentPage = _P_INFO ; 
    updateFullPage = true ;
    waitReleased = true ;          // discard "pressed" until a release
  }   
}

void fSdMove(uint8_t param) {     // param contient _LEFT ou _RIGTH
  if ( param == _LEFT ) {
    if ( firstFileToDisplay > 4 ) {
      firstFileToDisplay -= 4 ;
    } else {
      firstFileToDisplay = 1 ;
    } 
  } else if ( ( param == _RIGHT ) && ( (firstFileToDisplay + 4) <= sdFileDirCnt ) ) {
    firstFileToDisplay += 4 ;
    if ( ( firstFileToDisplay + 4) > sdFileDirCnt ) firstFileToDisplay = sdFileDirCnt - 3 ;
  } else {             // move one level up
    sdMoveUp() ;
  }
  // look in the directory for another file and upload the list of button.
  //Serial.print("firstFileToDisplay=") ; Serial.println(firstFileToDisplay);
  updateFullPage = true ;
  waitReleased = true ;          // discard "pressed" until a release 
}

void fSetXYZ(uint8_t param) {     // param contient le n° de la commande
  switch (param) {
  case _SETX : Serial2.println("G10 L20 P1 X0") ;  break ;
  case _SETY : Serial2.println("G10 L20 P1 Y0") ;  break ;
  case _SETZ : Serial2.println("G10 L20 P1 Z0") ;  break ;
  case _SETXYZ : Serial2.println("G10 L20 P1 X0 Y0 Z0") ;  break ;
  }
  waitReleased = true ;          // discard "pressed" until a release
                                 // update will be done when we receive a new GRBL status message
}

void fCmd(uint8_t param) {     // param contient le n° de la commande (valeur = _CMD1, ...)
  cmdToSend = param ;         // fill the index of the command to send; sending will be done in communication module
                              // create file name and try to open SPIFFS cmd file
  char spiffsCmdName[21] = "/Cmd0_" ;               // begin with fix text
  spiffsCmdName[4] = param - _CMD1 + '1' ;          // fill the cmd number (from 1...7)
  strcat( spiffsCmdName , cmdName[param - _CMD1]) ; // add the cmd name to the first part 
  if ( ! spiffsOpenCmdFile( spiffsCmdName ) ) {
      fillMsg(__CMD_NOT_RETRIEVED ) ;
      currentPage = _P_INFO ;
      updateFullPage = true ;
      waitReleased = true ;          // discard "pressed" until a release
      return ;
  }
  waitOk = false ; // do not wait for OK before sending char.
  statusPrinting = PRINTING_CMD ;
  currentPage = _P_INFO ;
  updateFullPage = true ;
  waitReleased = true ;          // discard "pressed" until a release 
}

void fStartUsb(uint8_t param){
  if( statusPrinting == PRINTING_STOPPED ) {
    while ( Serial.available() ) {      // clear the incomming buffer 
      Serial.read() ;
    }
    statusPrinting = PRINTING_FROM_USB ;
  }
  currentPage = _P_INFO ;  // go to page Info
  updateFullPage = true ;
  waitReleased = true ;
}  

void fStartTelnet(uint8_t param){
  if( statusPrinting == PRINTING_STOPPED ) {
    checkTelnetConnection();
    if ( telnetIsConnected() ) {
      clearTelnetBuffer() ;
      statusPrinting = PRINTING_FROM_TELNET ;
      //fillMsg( "Connected to telnet" );
    } else { 
      fillMsg( __NO_TELNET_CONNECTION  );   
    }
  }
  currentPage = _P_INFO ;  // go to page Info
  updateFullPage = true ;
  waitReleased = true ;
}  



void fStopPc(uint8_t param){
  statusPrinting = PRINTING_STOPPED ;
  updateFullPage = true ;  // force a redraw even if current page does not change
  waitReleased = true ;          // discard "pressed" until a release 
  
}

