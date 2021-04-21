#include "config.h"
#include "language.h"
#include "draw.h"
#include "setupTxt.h"
#include "TFT_eSPI_ms/TFT_eSPI.h"
#include "FS.h"
#include "actions.h"
#include "menu_file.h"
#include "telnet.h"
#include "cmd.h"
#include "com.h"
#include "log.h"
#include "bt.h"

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

extern char cmdName[11][17] ;     // store the names of the commands
extern uint8_t cmdToSend ;
extern M_Page mPages[_P_MAX_PAGES] ;
extern uint8_t currentBtn ;
extern uint8_t justPressedBtn;
extern uint8_t justReleasedBtn;
extern uint8_t longPressedBtn;
extern uint32_t beginChangeBtnMillis ;
extern volatile boolean waitOk ;

extern uint8_t jog_status  ;
extern boolean jogCancelFlag ;
extern boolean jogCmdFlag  ; 
extern uint32_t startMoveMillis ;
extern int8_t jogDistX ;
extern int8_t jogDistY ;
extern int8_t jogDistZ ;
extern int8_t jogDistA ;


extern float moveMultiplier ;

extern uint8_t * pNext ; // position of the next char to be written
extern uint8_t * pFirst ; // position of the first char
extern uint8_t * pGet ; // position of the last char to be read for display

extern char printString[250] ;
extern char * pPrintString ;
extern uint8_t lastStringCmd ;

//extern uint8_t grblLink ; // identify if grbl is connected via Serial, BT or Telnet.

extern char grblDirFilter[100]  ; // contains the name of the directory to be filtered; "/" for the root; last char must be "/"
extern char grblFileNames[GRBLFILEMAX][40]; // contain max n filename or directory name with max 40 char.
extern uint8_t firstGrblFileToDisplay ;   // 0 = first file in the directory
extern int grblFileIdx ; // index in the array where next file name being parse would be written


uint8_t fileToExecuteIdx ; // save the idex (0...3) of the file to be executed after confimation; 0/3 = sd on tft, 10/3 = sd on GRBL card
uint32_t prevAutoMoveMillis ;

#define SOFT_RESET 0x18 

void fGoToPage(uint8_t param) {
//  Serial.print( "go to page : " ) ; Serial.println( param) ; // just for testing // to do
  prevPage = currentPage ;
  currentPage = param ;
  updateFullPage = true ; 
  waitReleased = true ;          // discard "pressed" until a release 
}

void fGoToPageAndClearMsg(uint8_t param) {
  clearMsg() ;
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
    toGrbl(HOME_CMD);toGrbl("\n\r");
    //Serial2.println(HOME_CMD) ;  
  } else {
    fillMsg(_INVALID_BTN_HOME ) ;
  }
  waitReleased = true ;          // discard "pressed" until a release 
}

void fUnlock(uint8_t param) {
  if( machineStatus[0] == 'A') {  // if grbl is in alarm
    toGrbl("$X\n\r") ;    // send command to unlock
    //Serial2.println("$X") ;    // send command to unlock
    //Serial.println("$X has been sent"); // to remove
  }
// Stay on current page
  waitReleased = true ;          // discard "pressed" until a release 
}

void fReset(uint8_t param) {
  toGrbl((char) SOFT_RESET) ;
  //Serial2.print( (char) SOFT_RESET) ;
  waitReleased = true ;          // discard "pressed" until a release
  clearMsg( );
  
}

void fCancel(uint8_t param) {
  if( statusPrinting == PRINTING_FROM_SD || statusPrinting == PRINTING_PAUSED   ) {
    statusPrinting = PRINTING_STOPPED ;
    closeFileToRead() ;    
    toGrbl( (char) SOFT_RESET) ;
    //Serial2.print( (char) SOFT_RESET) ;
  }  
  currentPage = _P_INFO ;  // go to page Info
  updateFullPage = true ;  // force a redraw even if current page does not change
  waitReleased = true ;          // discard "pressed" until a release 
}

void fCancelGrbl(uint8_t param) {
  if ( statusPrinting == PRINTING_FROM_GRBL || statusPrinting == PRINTING_FROM_GRBL_PAUSED) {
    statusPrinting = PRINTING_STOPPED ;
    toGrbl( (char) SOFT_RESET) ;
    //Serial2.print( (char) SOFT_RESET) ;
  }
  currentPage = _P_INFO ;  // go to page Info
  updateFullPage = true ;  // force a redraw even if current page does not change
  waitReleased = true ;          // discard "pressed" until a release 
}

void fPause(uint8_t param) {
  if( (statusPrinting == PRINTING_FROM_SD || statusPrinting == PRINTING_FROM_GRBL ) && ( machineStatus[0] == 'R' || machineStatus[0] == 'J' ) ) { // test on J added mainly for test purpose
  #define PAUSE_CMD "!" 
    toGrbl(  PAUSE_CMD) ;
    //Serial2.print(PAUSE_CMD) ;
    statusPrinting = PRINTING_PAUSED ;
    updateFullPage = true ;  // 
    //Serial.println("Pause is sent to grbl");
  }
  waitReleased = true ;          // discard "pressed" until a release   
}

#define RESUME_CMD "~"
void fResume(uint8_t param) {
  if(  statusPrinting == PRINTING_PAUSED && machineStatus[0] == 'H') {
    toGrbl( RESUME_CMD) ;
    resetWaitOkWhenSdMillis() ; // we reset the time we sent the last cmd otherwise, we can get a wrong warning saying that we are missing an OK (because it seems that GRBL suspends OK while in pause)
    statusPrinting = PRINTING_FROM_SD ;
    
    updateFullPage = true ;      // we have to redraw the buttons because Resume should become Pause
  }
  currentPage = _P_INFO ;  // go to page Info
  waitReleased = true ;          // discard "pressed" until a release 
}

void fResumeGrbl(uint8_t param) {
  if( statusPrinting == PRINTING_FROM_GRBL_PAUSED && machineStatus[0] == 'H') {
    toGrbl( RESUME_CMD) ;
    resetWaitOkWhenSdMillis() ; // we reset the time we sent the last cmd otherwise, we can get a wrong warning saying that we are missing an OK (because it seems that GRBL suspends OK while in pause)
    statusPrinting = PRINTING_FROM_GRBL ;
    updateFullPage = true ;      // we have to redraw the buttons because Resume should become Pause
  }
  currentPage = _P_INFO ;  // go to page Info
  waitReleased = true ;          // discard "pressed" until a release 
}


void fDist( uint8_t param ) {
  uint8_t newDist =  mPages[_P_MOVE].boutons[POS_OF_MOVE_D_AUTO] ;       // convertit la position du bouton en type de bouton 
  //Serial.print("newDist=") ; Serial.println(newDist) ;
  if ( ++newDist > _D0_01 ) newDist = _D_AUTO ; // increase and reset to min value if to big
  mPages[_P_MOVE].boutons[POS_OF_MOVE_D_AUTO ] = newDist ;   // update the button to display
  mButtonClear(POS_OF_MOVE_D_AUTO + 1 , newDist ) ;  // clear the current button
  mButtonDraw( POS_OF_MOVE_D_AUTO + 1 , newDist ) ;  // draw  at position (from 1 to 12) a button (newDist)
  //updateFullPage = true ;                     // force a redraw of buttons
  waitReleased = true ;          // discard "pressed" until a release 
}  

void fMove( uint8_t param ) { // param contains the touch being pressed or the released if no touch has been pressed
    float distance = 0.01 ;
    //uint32_t moveMillis = millis() ;
    //static uint32_t prevMoveMillis ;
    if ( mPages[_P_MOVE].boutons[POS_OF_MOVE_D_AUTO] == _D_AUTO ) {
      handleAutoMove(param) ; // process in a similar way as Nunchuk
    } else if (justPressedBtn) {                      // just pressed in non auto mode
      switch ( mPages[_P_MOVE].boutons[POS_OF_MOVE_D_AUTO] ) {        
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
      //bufferise2Grbl("\n\r",'b');
      bufferise2Grbl("$J=G91 G21 ", 'b') ;
      //Serial2.println("") ; Serial2.print("$J=G91 G21 ") ;
       
      //switch ( justPressedBtn ) {  // we convert the position of the button into the type of button
      //  case 7 :  Serial2.print("X")  ;  break ;
      //  case 5 :  Serial2.print("X-") ;  break ;
      //  case 2 :  Serial2.print("Y")  ;  break ;
      //  case 10 :  Serial2.print("Y-") ;  break ;
      //  case 4 :  Serial2.print("Z")  ;  break ;
      //  case 12 :  Serial2.print("Z-") ;  break ;
      // }
      uint8_t typeOfMove ;
      typeOfMove = convertBtnPosToBtnIdx( currentPage , justPressedBtn ) ;
      switch ( typeOfMove ) {  // we convert the position of the button into the type of button
        case _XP :  bufferise2Grbl("X")  ;  break ;
        case _XM :  bufferise2Grbl("X-") ;  break ;
        case _YP :  bufferise2Grbl("Y")  ;  break ;
        case _YM :  bufferise2Grbl("Y-") ;  break ;
        case _ZP :  bufferise2Grbl("Z")  ;  break ;
        case _ZM :  bufferise2Grbl("Z-") ;  break ;
        case _AP :  bufferise2Grbl("A")  ;  break ;
        case _AM :  bufferise2Grbl("A-") ;  break ;
      }
      char sdistance[20];
      sprintf(sdistance, "%.4f" , distance);
      bufferise2Grbl(sdistance) ; bufferise2Grbl(" F100\n", 's');
      //Serial2.print(distance) ; Serial2.println (" F100") ;
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
  //Serial.print("pressedBtn=");Serial.print(pressedBtn); Serial.print(" jogCmdFlag=");Serial.print(jogCmdFlag); Serial.println(" "); 
  if ( ( pressedBtn ) && (jogCmdFlag == false) ) {
    if (cntSameAutoMove == 0 ) { 
    //  moveMultiplier = 0.01 ; 
      startMoveMillis = millis();
    } 
    cntSameAutoMove++ ;
    jogDistX = 0 ;           // reset all deplacements
    jogDistY = 0 ;
    jogDistZ = 0 ;
    jogDistA = 0 ;
    //switch ( pressedBtn ) {  // fill one direction of move
    //  case 7 :  jogDistX = 1  ;  break ;
    //  case 5 :  jogDistX = -1 ;  break ;
    //  case 2 :  jogDistY = 1  ;  break ;
    //  case 10 :  jogDistY = -1 ;  break ;
    //  case 4 :  jogDistZ = 1 ;  break ;
    //  case 12 :  jogDistZ = -1 ;  break ;
    //}
    uint8_t typeOfMove;
    typeOfMove = convertBtnPosToBtnIdx( currentPage , pressedBtn ) ; // we convert the position of the button into the type of button
    switch ( typeOfMove ) {                                          // fill one direction of move
        case _XP :  jogDistX = 1  ;  break ;                  
        case _XM :  jogDistX = -1 ;  break ;
        case _YP :  jogDistY = 1  ;  break ;
        case _YM :  jogDistY = -1 ;  break ;
        case _ZP :  jogDistZ = 1  ;  break ;
        case _ZM :  jogDistZ = -1  ;  break ;
        case _AP :  jogDistA = 1  ;  break ;
        case _AM :  jogDistA = -1  ;  break ;
        
    }
    //Serial.print("type of move=");Serial.println(typeOfMove);
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
    fileToExecuteIdx = param ; // save index of file.
    prevPage = currentPage ;            // go to confirm page
    currentPage = _P_SD_CONFIRM ; 
    updateFullPage = true ;
    waitReleased = true ;          // discard "pressed" until a release
  }   
}

void fStartSdPrinting(uint8_t param){ // param is the index of the file button (0...3)
    waitOk = false ;
//    toGrbl(PAUSE_CMD) ;
    //Serial2.print(PAUSE_CMD) ;
//    delay(10);
//    toGrbl("?") ;
    //Serial2.print("?") ;
    //waitOk = false ; // do not wait for OK before sending char.
//    statusPrinting = PRINTING_PAUSED ; // initially it was PRINTING_FROM_SD ; // change the status, so char will be read and sent in main loop
    statusPrinting = PRINTING_FROM_SD ; // change the status, so char will be read and sent in main loop
//    prevPage = currentPage ;            // go to INFO page
    currentPage = _P_INFO ; 
    updateFullPage = true ;
    waitReleased = true ;          // discard "pressed" until a release
}

void fSdMove(uint8_t param) {     // param contient _PG_PREV ou _PG_NEXT
  if ( param == _PG_PREV ) {
    if ( firstFileToDisplay > 4 ) {
      firstFileToDisplay -= 4 ;
    } else {
      firstFileToDisplay = 1 ;
    } 
  } else if ( ( param == _PG_NEXT ) && ( (firstFileToDisplay + 4) <= sdFileDirCnt ) ) {
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
  case _SETX : memccpy ( printString , _SETX_STRING , '\0' , 249); break ;
  case _SETY : memccpy ( printString , _SETY_STRING , '\0' , 249); break ;
  case _SETZ :   memccpy ( printString , _SETZ_STRING , '\0' , 249); break ;
  case _SETA :   memccpy ( printString , _SETA_STRING , '\0' , 249); break ;
  case _SETXYZ : memccpy ( printString , _SETXYZ_STRING , '\0' , 249); break ;
  case _SETXYZA : memccpy ( printString , _SETXYZA_STRING , '\0' , 249); break ;
  case _SET_CHANGE : memccpy ( printString , _SET_CHANGE_STRING , '\0' , 249); break ;
  case _SET_PROBE :  memccpy ( printString , _SET_PROBE_STRING , '\0' , 249);  break ;  
  case _SET_CAL :    memccpy ( printString , _CAL_STRING , '\0' , 249);    break ;  
  case _GO_CHANGE :  memccpy ( printString , _GO_CHANGE_STRING , '\0' , 249); break ; 
  case _GO_PROBE :   memccpy ( printString , _GO_PROBE_STRING , '\0' , 249);  break ; 
  default : printString[0]= '\0' ; break ;  // null string for safety; normally can't happen
  }
  lastStringCmd = param ;           // save the last parameter in order to generate a text when the task is execute
  sendStringToGrbl() ;
  waitReleased = true ;          // discard "pressed" until a release
                                 // update will be done when we receive a new GRBL status message
}

void sendStringToGrbl() {
  //Serial.print("calibration = ") ; Serial.println( pStringToSend ) ;
  if ( statusPrinting != PRINTING_STOPPED ) {
    fillMsg(_NOT_IDLE ) ; 
  } else {
    //Serial.println("Start sending string") ; // to debug
    statusPrinting = PRINTING_STRING ;
    pPrintString = &printString[0] ;
    waitOk = false ;
  }
}

void fCmd(uint8_t param) {     // param contient le n° de la commande (valeur = _CMD1, ...)
  cmdToSend = param ;         // fill the index of the command to send; sending will be done in communication module
                              // create file name and try to open SPIFFS cmd file ; filename look like /Cmd
  char spiffsCmdName[21] = "/Cmd0_" ;               // begin with fix text
  if ( param <= _CMD9 ) {                           // fill the cmd number (from 1...9 or A or B ) ; A = 10 , B = 11
    spiffsCmdName[4] = param - _CMD1 + '1' ;          
  } else {
    spiffsCmdName[4] = param - _CMD10 + 'A' ;   
  }
  strcat( spiffsCmdName , cmdName[param - _CMD1]) ; // add the cmd name to the first part 
  if ( ! spiffsOpenCmdFile( spiffsCmdName ) ) {
      fillMsg(_CMD_NOT_RETRIEVED ) ;
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
      fillMsg(_NO_TELNET_CONNECTION  );   
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

void fLogPrev(uint8_t param) {
  fillMPage (_P_LOG , POS_OF_LOG_PG_NEXT , _PG_NEXT , _JUST_PRESSED , fLogNext , 0) ; //active the next button
  uint8_t count = 0 ;
  fillMPage (_P_LOG , POS_OF_LOG_PG_PREV , _PG_PREV , _JUST_PRESSED , fLogPrev , 0) ; // activate PREV btn
  while  (count < ( N_LOG_LINE_MAX * 2 ) ) {
    if ( getPrevLogLine()>=0 )  { 
      count++ ; // move back max 6 line before
    } else {
      count = N_LOG_LINE_MAX * 2 ; // force exit of while
      fillMPage (_P_LOG , POS_OF_LOG_PG_PREV , _NO_BUTTON , _NO_ACTION , fLogPrev , 0) ;  // deactivate PREV btn
    }
  }
  updateFullPage = true ; 
  waitReleased = true ;          // discard "pressed" until a release 
  //drawDataOnLogPage() ;
}

void fLogNext(uint8_t param) {
  uint8_t count = 0 ;
  fillMPage (_P_LOG , POS_OF_LOG_PG_PREV , _PG_PREV , _JUST_PRESSED , fLogPrev , 0) ; // activate PREV btn (even if not needed)
  fillMPage (_P_LOG , POS_OF_LOG_PG_NEXT , _PG_NEXT , _JUST_PRESSED , fLogNext , 0) ; //active the next button
  while ( count < N_LOG_LINE_MAX) { // move max line ahead (to be sure that we can display many lines when we are close to the end
    if ( getNextLogLine()>=0 ){
      count++ ; 
    } else {                       // When move is not possible, Pget point the last valid line; to display it we have to move back 1 line less 
      count = N_LOG_LINE_MAX ;
      fillMPage(_P_LOG , POS_OF_LOG_PG_NEXT , _NO_BUTTON , _NO_ACTION , fLogNext , 0) ; // deactivate the NEXT button
    }
  }
  count = 0 ;
  while  (count <  N_LOG_LINE_MAX ) {
    if ( getPrevLogLine()>=0 )  { 
      count++ ; // move back max 6 line before
    } else {
      count = N_LOG_LINE_MAX; // force exit of while
      fillMPage(_P_LOG , POS_OF_LOG_PG_PREV , _NO_BUTTON , _NO_ACTION , fLogPrev , 0) ;  // deactivate PREV btn
    }
  }
  //clearScreen() ;  // not needed because done when we execute the drawFullPage()
  //drawDataOnLogPage() ;
  updateFullPage = true ; 
  waitReleased = true ;          // discard "pressed" until a release 
}

void fSdShowPrev(uint8_t param) {  // to do
  setPrevShowBuffer() ;
  updateFullPage = true ; 
  waitReleased = true ;          // discard "pressed" until a release
}

void fSdShowNext(uint8_t param) {  // to do 
  setNextShowBuffer() ;
  updateFullPage = true ; 
  waitReleased = true ;          // discard "pressed" until a release
}

void fOverSwitch (uint8_t BtnParam) {
  if ( BtnParam == _OVER_SWITCH_TO_SPINDLE ) { // here we have to switch button and title in order to let modify RPM 
    fillMPage (_P_OVERWRITE , POS_OF_OVERWRITE_OVERWRITE , _OVER_SWITCH_TO_FEEDRATE , _JUST_PRESSED , fOverSwitch , _OVER_SWITCH_TO_FEEDRATE ) ;
  } else {                                     // here we have to switch button and title in order to let modify RPM 
    fillMPage (_P_OVERWRITE , POS_OF_OVERWRITE_OVERWRITE , _OVER_SWITCH_TO_SPINDLE , _JUST_PRESSED , fOverSwitch , _OVER_SWITCH_TO_SPINDLE ) ;
  } 
  updateFullPage = true ; // force a redraw because Btn has change
  waitReleased = true ;          // discard "pressed" until a release
}

void fOverModify (uint8_t BtnParam) {
  char grblOverwriteCode = BtnParam - _OVER_100 ;   // _OVER_100 is the first of the 5 codes; code have to be put in the right sequence in the enum 
  if ( mPages[_P_OVERWRITE].boutons[POS_OF_OVERWRITE_OVERWRITE] == _OVER_SWITCH_TO_SPINDLE ) {  // when button allows to change to spindle,it means we are currently changing Feedrate
    grblOverwriteCode += 0x90 ;    // 0x90 is the GRBL code for 100% feedrate
  //  Serial.print("We change Feedrate= ");  Serial.println( (uint8_t) grblOverwriteCode, HEX);  // to debug
  } else {                                                             // We change RPM
    grblOverwriteCode += 0x99 ;    // 0x99 is the GRBL code for 100% RPM
  //  Serial.println("We change RPM");   Serial.println(  (uint8_t) grblOverwriteCode, HEX);  // to debug
  }
  toGrbl((char) grblOverwriteCode ) ;
  //Serial2.print( (char) grblOverwriteCode ) ;  
  updatePartPage = true ;                     // force a redraw of data
  waitReleased = true ;          // discard "pressed" until a release
}

void fSerial(uint8_t param) { // activate GRBL over Serial2
  // to do ; unactivate other comm and change GrblLink
  startGrblCom(GRBL_LINK_SERIAL);
}
void fBluetooth(uint8_t param) { // activate GRBL over Bluetooth
  // to do ; unactivate other comm and change GrblLink
  startGrblCom(GRBL_LINK_BT);
}
void fTelnet(uint8_t param) { // activate GRBL over Telnet
  // to do ; unactivate other comm and change GrblLink
  startGrblCom(GRBL_LINK_TELNET);
}

void fSdGrblMove(uint8_t param) {     // param contient _PG_PREV ou _PG_NEXT
  if ( param == _PG_PREV ) {               // previous
    if ( firstGrblFileToDisplay >= 4 ) {
      firstGrblFileToDisplay -= 4 ;
    } else {
      firstGrblFileToDisplay = 0 ;
    } 
  } else if ( param == _PG_NEXT ){      // next
    if ( (firstGrblFileToDisplay + 4) < grblFileIdx )  {
      firstGrblFileToDisplay += 4 ;
      if ( ( firstGrblFileToDisplay + 4) >= grblFileIdx ) firstGrblFileToDisplay = grblFileIdx - 4 ;
    }  
  } else {             // move one level up
      if ( strlen(grblDirFilter)  > 2) { // change only if there are more than 1 car in the filter
          char * lastDirBeginAt ;
          lastDirBeginAt = grblDirFilter + strlen(grblDirFilter) - 2 ; // point to the car before the last one (last is a '/')
          while ( *lastDirBeginAt != '/' && lastDirBeginAt > grblDirFilter) lastDirBeginAt-- ; // search the last '/' before the last position
          *lastDirBeginAt++ ; //point to the first char after '/' 
          *lastDirBeginAt = '\0' ;  //replace this char by a string terminator
          currentPage = _P_SD_GRBL_WAIT ; // will go to the page that force to read again the file list 
        }    
  }
  //Serial.print("firstFileToDisplay=") ; Serial.println(firstFileToDisplay);
  updateFullPage = true ;
  waitReleased = true ;          // discard "pressed" until a release 
}


void fSdGrblFilePrint(uint8_t param ){   // lance l'impression d'un fichier; param contains l'index (0 à 3) du fichier à ouvrir; pour trouver le nom, il faut ajouter cela à firstGrblFileToDisplay
   if ( (*grblFileNames[param + firstGrblFileToDisplay])  == '/' ) { // if first char = "/", it is a directory
      if ( ( strlen(grblFileNames[param + firstGrblFileToDisplay]) + strlen(grblDirFilter))  > 99 ){ // when the new filter name would be to big, just discard
         // discard  
      } else { // change the filter (adding the dir name to existing filter and a "/" at the end
        char * pendgrblDirFilter ;
        pendgrblDirFilter = grblDirFilter + strlen(grblDirFilter) ; // point to the '\0' end of name
        strcpy(pendgrblDirFilter , grblFileNames[param + firstGrblFileToDisplay]+1 ) ; //add the dirname being selected but not the first char (='/'
        strcpy(grblDirFilter + strlen(grblDirFilter), "/") ; // add '/' at the end
      }
      currentPage = _P_SD_GRBL_WAIT ; // will go to the page that force to read again the file list 
      updateFullPage = true ;
      waitReleased = true ;          // discard "pressed" until a release       
      return ;
   } else {   // it is a file name; so send the command to Grbl (the command to send is "$SD/Run=/dir/dir/file.ext"
      fileToExecuteIdx = param +10 ; // save index of file + 10 (to make the difference with SD from TFT) (to be used in fConfirmedYes for calling fStartSdGrblPrinting
      prevPage = currentPage ;            // go to confirm page
      currentPage = _P_SD_CONFIRM ; 
      updateFullPage = true ;
      waitReleased = true ;          // discard "pressed" until a release
  }   
}

void fStartSdGrblPrinting(uint8_t param){ // param is the index of the file button (0...3)
                                          // we have to send a command with the name like "$SD/Run=/dir/dir/file.ext" 
      char fullFileName[200] ;
      strcpy(fullFileName , grblDirFilter ); // copy directory name
      strcpy(fullFileName + strlen(fullFileName) , grblFileNames[param + firstGrblFileToDisplay] )  ; // add the file name 
      toGrbl("$SD/run=");
      toGrbl(fullFileName);
      toGrbl("\r\n");
      currentPage = _P_INFO ; // go to info page 
      updateFullPage = true ;
      waitReleased = true ;          // discard "pressed" until a release
}

void fConfirmedYes(uint8_t param ) { // called when Yes btn is pressed; based on parameter call different function 
  //Serial.print("fileToExecuteIdx="); Serial.println(fileToExecuteIdx);
  if ( fileToExecuteIdx <=3) { // it is a file from sd card attached to tft
    fStartSdPrinting(fileToExecuteIdx) ;
  } else if ( fileToExecuteIdx >= 10 &&  fileToExecuteIdx <=13) {// it is a file from sd card attached to grbl
    fStartSdGrblPrinting(fileToExecuteIdx - 10);
  } else {
    fGoToPage(_P_INFO); // this should not occurs
  }
}
void fConfirmedNo(uint8_t param ) { // called when No btn is pressed; should do the same as back btn in principe
  fGoBack(0);
}


