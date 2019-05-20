#include "config.h"
#include "draw.h"
#include "language.h"
#include "TFT_eSPI_ms/TFT_eSPI.h"
#include "FS.h"
#include "actions.h"
#include "menu_file.h"
#include "SdFat.h"
#include "browser.h"
#include "com.h"

#define LABELS9_FONT &FreeSans9pt7b    // Key label font 2
#define LABELS12_FONT &FreeSans12pt7b
#define LABELSB9_FONT &FreeSansBold9pt7b
#define LABELCB12_FONT &FreeSansBold12pt7b
#define LABELM9_FONT &FreeMono9pt7b    // Key label font 2
#define LABELM12_FONT &FreeMono12pt7b
#define LABELMB9_FONT &FreeMonoBold9pt7b
#define LABELMB12_FONT &FreeMonoBold12pt7b

//#define NORMAL_BORDER TFT_WHITE 
//#define SELECTED_BORDER TFT_RED 

// create for touchscreeen
TFT_eSPI tft = TFT_eSPI();

M_Button mButton[_MAX_BTN] ;
M_Page mPages[_P_MAX_PAGES];

extern uint8_t prevPage , currentPage ;
extern boolean updateFullPage ;
extern boolean updatePartPage ;
extern uint8_t justPressedBtn , justReleasedBtn, longPressedBtn , lastBtn ,currentBtn;  
extern boolean waitReleased ;
extern uint32_t beginChangeBtnMillis ;

//extern uint8_t sdStatus ;
extern int8_t dirLevel ;
extern uint16_t sdFileDirCnt ;
extern uint32_t sdFileSize ;
extern uint32_t sdNumberOfCharSent ;

extern char fileNames[4][23] ; // 22 car per line + "\0"
extern uint16_t firstFileToDisplay ;   // 0 = first file in the directory
extern SdBaseFile aDir[DIR_LEVEL_MAX] ;

extern char cmdName[7][17] ;          // contains the names of the commands

extern uint8_t statusPrinting ;
extern float wposXYZ[3] ;
extern float mposXYZ[3] ;
extern char machineStatus[9];
extern float feedSpindle[2] ;  

extern char lastMsg[80] ;        // last message to display
  

//char sdStatusText[][20]  = { "No Sd card" , "Sd card to check" , "Sd card OK" , "Sd card error"} ;  // pour affichage en clair sur le lcd;
char printingStatusText[][20] = { " " , "SD-->Grbl" , "Error SD-->Grbl" , "Pause SD-->Grbl" , "Usb-->Grbl" ,  "CMD" , "Telnet-->Grbl"} ; 

extern int8_t prevMoveX ;
extern int8_t prevMoveY ;
extern int8_t prevMoveZ ;
extern float moveMultiplier ;
// used by nunchuck
extern uint8_t jog_status  ;
extern boolean jogCancelFlag ;
extern boolean jogCmdFlag  ; 

extern boolean statusTelnetIsConnected ;

float wposMoveInitXYZ[3] ;

// previous values used to optimise redraw of INFO screen
uint8_t statusPrintingPrev;
uint8_t machineStatus0Prev;   // we keep only the first char for fast testing, so we can't use ir when equal to A or H
extern boolean lastMsgChanged ;
boolean statusTelnetIsConnectedPrev ;
boolean newInfoPage ;

extern char grblLastMessage[STR_GRBL_BUF_MAX_SIZE] ;
extern boolean grblLastMessageChanged;


// rempli le paramétrage des boutons de chaque page 
void fillMPage (uint8_t _page , uint8_t _btnPos , uint8_t _boutons, uint8_t _actions , void (*_pfNext)(uint8_t) , uint8_t _parameters ) {
  mPages[_page].boutons[_btnPos] =  _boutons ;
  mPages[_page].actions[_btnPos] =  _actions ;
  mPages[_page].pfNext[_btnPos] =  _pfNext ;
  mPages[_page].parameters[_btnPos] =  _parameters ;
}

void initButtons() {  
mButton[_SETUP].pLabel = __SETUP  ;
mButton[_PRINT].pLabel = __PRINT  ;
mButton[_HOME].pLabel = __HOME  ;
mButton[_UNLOCK].pLabel = __UNLOCK  ;
mButton[_RESET].pLabel = __RESET  ;
mButton[_SD].pLabel = __SD  ;
mButton[_USB_GRBL].pLabel = __USB_GRBL  ;
mButton[_TELNET_GRBL].pLabel = __TELNET_GRBL  ;
mButton[_PAUSE].pLabel = __PAUSE  ;
mButton[_CANCEL].pLabel = __CANCEL  ;
mButton[_INFO].pLabel = __INFO  ;
mButton[_CMD].pLabel = __CMD  ;
mButton[_MOVE].pLabel = __MOVE  ;
mButton[_RESUME].pLabel = __RESUME  ;
mButton[_STOP_PC_GRBL].pLabel = __STOP_PC_GRBL ;
mButton[_XP].pLabel = "X+" ;
mButton[_XM].pLabel = "X-" ;
mButton[_YP].pLabel = "Y+" ;
mButton[_YM].pLabel = "Y-" ;
mButton[_ZP].pLabel = "Z+" ;
mButton[_ZM].pLabel = "Z-" ;
mButton[_D_AUTO].pLabel = __D_AUTO  ;
mButton[_D0_01].pLabel = "0.01" ;
mButton[_D0_1].pLabel = "0.1" ;
mButton[_D1].pLabel = "1" ;
mButton[_D10].pLabel = "10" ;
mButton[_SETX].pLabel = __SETX  ;
mButton[_SETY].pLabel = __SETY  ;
mButton[_SETZ].pLabel = __SETZ  ;
mButton[_SETXYZ].pLabel = __SETXYZ  ;
mButton[_BACK].pLabel = __BACK  ;
mButton[_LEFT].pLabel = __LEFT  ;
mButton[_RIGHT].pLabel = __RIGHT  ;
mButton[_UP].pLabel = __UP  ;
mButton[_CMD1].pLabel = &cmdName[0][0] ;
mButton[_CMD2].pLabel = &cmdName[1][0] ;
mButton[_CMD3].pLabel = &cmdName[2][0] ;
mButton[_CMD4].pLabel = &cmdName[3][0] ;
mButton[_CMD5].pLabel = &cmdName[4][0] ;
mButton[_CMD6].pLabel = &cmdName[5][0] ;
mButton[_CMD7].pLabel = &cmdName[6][0] ;
mButton[_MORE_PAUSE].pLabel = __MORE_PAUSE ;
mButton[_FILE0].pLabel = fileNames[0] ;  // labels are defined during execution in a table
mButton[_FILE1].pLabel = fileNames[1] ;
mButton[_FILE2].pLabel = fileNames[2] ;
mButton[_FILE3].pLabel = fileNames[3] ;

mPages[_P_INFO].titel = "Info" ;
mPages[_P_INFO].pfBase = fInfoBase ;
fillMPage (_P_INFO , 3 , _SETUP , _JUST_PRESSED , fGoToPage , _P_SETUP) ;   // those buttons are changed dynamically based on status (no print, ...)
fillMPage (_P_INFO , 7 , _PRINT , _JUST_PRESSED , fGoToPage , _P_PRINT) ;   // those buttons are changed dynamically based on status (no print, ...)

mPages[_P_SETUP].titel = "Setup" ;
mPages[_P_SETUP].pfBase = fSetupBase ;
fillMPage (_P_SETUP , 0 , _HOME , _JUST_PRESSED , fHome , 0) ;
fillMPage (_P_SETUP , 1 , _UNLOCK , _JUST_PRESSED , fUnlock , 0) ;
fillMPage (_P_SETUP , 2 , _RESET , _JUST_PRESSED , fReset , 0) ;
fillMPage (_P_SETUP , 3 , _PRINT , _JUST_PRESSED , fGoToPage , _P_PRINT ) ;
fillMPage (_P_SETUP , 4 , _MOVE , _JUST_PRESSED , fGoToPage , _P_MOVE ) ;
fillMPage (_P_SETUP , 5 , _SETXYZ , _JUST_PRESSED , fGoToPage , _P_SETXYZ ) ;
fillMPage (_P_SETUP , 6 , _CMD , _JUST_PRESSED , fGoToPage , _P_CMD ) ;
fillMPage (_P_SETUP , 7 , _INFO , _JUST_PRESSED , fGoToPageAndClearMsg ,  _P_INFO) ;

mPages[_P_PRINT].titel = "Print" ;
mPages[_P_PRINT].pfBase = fNoBase ;
fillMPage (_P_PRINT , 0 , _SD , _JUST_PRESSED , fGoToPage , _P_SD) ;
fillMPage (_P_PRINT , 1 , _USB_GRBL , _JUST_PRESSED , fStartUsb , 0) ;
fillMPage (_P_PRINT , 2 , _TELNET_GRBL , _JUST_PRESSED , fStartTelnet , 0) ;
fillMPage (_P_PRINT , 3 , _SETUP , _JUST_PRESSED , fGoToPage , _P_SETUP) ;
fillMPage (_P_PRINT , 6 , _CMD , _JUST_PRESSED , fGoToPage , _P_CMD ) ;
fillMPage (_P_PRINT , 7 , _INFO , _JUST_PRESSED , fGoToPage , _P_INFO) ;

mPages[_P_PAUSE].titel = "Paused" ;
mPages[_P_PAUSE].pfBase = fNoBase ;
fillMPage (_P_PAUSE , 0 , _CANCEL , _JUST_PRESSED , fCancel , 0) ;
fillMPage (_P_PAUSE , 1 , _RESUME , _JUST_PRESSED , fResume , 0) ;
fillMPage (_P_PAUSE , 2 , _MOVE , _JUST_PRESSED , fGoToPage , _P_MOVE) ;
fillMPage (_P_PAUSE , 7 , _INFO , _JUST_PRESSED , fGoToPage , _P_INFO) ;

mPages[_P_MOVE].titel = "Move" ;
mPages[_P_MOVE].pfBase = fMoveBase ;
fillMPage (_P_MOVE , 0 , _XP , _JUST_LONG_PRESSED_RELEASED , fMove , _XP) ;
fillMPage (_P_MOVE , 1 , _YP , _JUST_LONG_PRESSED_RELEASED , fMove , _YP) ;
fillMPage (_P_MOVE , 2 , _ZP , _JUST_LONG_PRESSED_RELEASED , fMove , _ZP) ;
fillMPage (_P_MOVE , 3 , _D_AUTO , _JUST_PRESSED , fDist , 0) ;
fillMPage (_P_MOVE , 4 , _XM , _JUST_LONG_PRESSED_RELEASED , fMove , _XM) ;
fillMPage (_P_MOVE , 5 , _YM , _JUST_LONG_PRESSED_RELEASED , fMove , _YM) ;
fillMPage (_P_MOVE , 6 , _ZM , _JUST_LONG_PRESSED_RELEASED , fMove , _ZM) ;
fillMPage (_P_MOVE , 7 , _BACK , _JUST_PRESSED , fGoBack , 0) ;

mPages[_P_SETXYZ].titel = "Set X, Y, Z to 0" ;
mPages[_P_SETXYZ].pfBase = fSetXYZBase ;
fillMPage (_P_SETXYZ , 0 , _SETX , _JUST_PRESSED , fSetXYZ , _SETX) ;
fillMPage (_P_SETXYZ , 1 , _SETY , _JUST_PRESSED , fSetXYZ , _SETY) ;
fillMPage (_P_SETXYZ , 2 , _SETZ, _JUST_PRESSED , fSetXYZ , _SETZ) ;
fillMPage (_P_SETXYZ , 3 , _SETXYZ , _JUST_PRESSED , fSetXYZ , _SETXYZ) ;
fillMPage (_P_SETXYZ , 5 , _SETUP , _JUST_PRESSED , fGoToPage , _P_SETUP ) ;
fillMPage (_P_SETXYZ , 7 , _INFO , _JUST_PRESSED , fGoToPage , _P_INFO ) ;

mPages[_P_SD].titel = "Select a file on Sd card" ;
mPages[_P_SD].pfBase = fSdBase ;   // cette fonction doit remplir les 4 premiers boutons en fonction des fichiers disponibles
fillMPage (_P_SD , 4 , _UP , _JUST_PRESSED , fSdMove , _UP) ;
fillMPage (_P_SD , 5 , _LEFT , _JUST_PRESSED , fSdMove , _LEFT) ;
fillMPage (_P_SD , 6 , _RIGHT , _JUST_PRESSED , fSdMove , _RIGHT) ;
fillMPage (_P_SD , 7 , _INFO , _JUST_PRESSED , fGoToPage , _P_INFO ) ;

mPages[_P_CMD].titel = "Select a command" ;
mPages[_P_CMD].pfBase = fCmdBase ; // 
if (cmdName[0][0] ) fillMPage (_P_CMD , 0 , _CMD1 , _JUST_PRESSED , fCmd , _CMD1) ; // le paramètre contient le n° du bouton
if (cmdName[1][0] ) fillMPage (_P_CMD , 1 , _CMD2 , _JUST_PRESSED , fCmd , _CMD2) ; // le paramètre contient le n° du bouton
if (cmdName[2][0] ) fillMPage (_P_CMD , 2 , _CMD3 , _JUST_PRESSED , fCmd , _CMD3) ; // le paramètre contient le n° du bouton
if (cmdName[3][0] ) fillMPage (_P_CMD , 3 , _CMD4 , _JUST_PRESSED , fCmd , _CMD4) ; // le paramètre contient le n° du bouton
if (cmdName[4][0] ) fillMPage (_P_CMD , 4 , _CMD5 , _JUST_PRESSED , fCmd , _CMD5) ; // le paramètre contient le n° du bouton
if (cmdName[5][0] ) fillMPage (_P_CMD , 5 , _CMD6 , _JUST_PRESSED , fCmd , _CMD6) ; // le paramètre contient le n° du bouton
if (cmdName[6][0] ) fillMPage (_P_CMD , 6 , _CMD7 , _JUST_PRESSED , fCmd , _CMD7) ; // le paramètre contient le n° du bouton
fillMPage (_P_CMD , 7 , _INFO , _JUST_PRESSED , fGoToPage , _P_INFO ) ;

}  // end of init



void tftInit() {
  tft.init() ; // Initialise l'écran avec les pins définies dans setup (par exemple)
                // #define TFT_MISO 19
                //#define TFT_MOSI 23
                //#define TFT_SCLK 18
                //#define TFT_CS   15  // Chip select control pin
                //#define TFT_DC    2  // Data Command control pin (to connect to RS pin of LDC probably)
                //#define TFT_RST   4  // Reset pin (could connect to RST pin)
                //#define TFT_RST  -1  // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST
                //Cette fonction met les pins en mode input/output; elle envoie aussi les commandes d'initialisation
  // Set the rotation before we calibrate
  tft.setRotation(1); // normally, this is already done in tft.int() but it is not clear how is rotation set (probably 0); so it can be usefull to change it here
  touch_calibrate(); // call screen calibration
  //tft.printCalibration() ;  // print calibration data (print on Serial port the calibration data ; only for debug
  tft.fillScreen(SCREEN_BACKGROUND);   // clear screen
}


boolean convertPosToXY( uint8_t pos , int32_t *_x, int32_t *_y ){
  if (pos > 0 && pos < 9 ) {                // accept only value from 1 to 8
    pos--;
    if (pos >= 4) { 
      pos -= 4 ;
      *_y = 160 ;  // to do, test if 150 is ok for second row of buttons
    } else {
      *_y = 80 ;
    }
    *_x = pos * 80 + 1 ; //   on suppose un bouton de 74 de large avec un bord de 3
    return true ;
  } else {
    return false ;
  }
} ;

void mButtonDraw(uint8_t pos , uint8_t btnIdx) {  // draw a button at position (from 1 to 8) 
//  si le texte a moins de 5 char, affiche 1 ligne en size 2
//                         9               1               1
//                         17              2               1 ; le . sert de séparateur pour répartir les noms de fichier sur 2 lignes si possible 
  int32_t _xl , _yl ;
  int32_t _w = 76 ;
  int32_t _h = 76 ;
  int32_t fill = BUTTON_BACKGROUND ;
  int32_t outline = BUTTON_BORDER_NOT_PRESSED ;
  int32_t text = BUTTON_TEXT ;
  char * pbtnLabel ; 
  pbtnLabel = mButton[btnIdx].pLabel ;
  if ( *mButton[btnIdx].pLabel == '/' ){    // when first char is "/", invert the color and skip the first char
    fill = BUTTON_BORDER_NOT_PRESSED ;
    text = BUTTON_BACKGROUND ;
    pbtnLabel++ ;
  }
//  Serial.print("pos="); Serial.print( pos ) ; Serial.print(" btnIdx="); Serial.print( btnIdx ) ;
//  Serial.print("first char of btnName ="); Serial.println( *pbtnLabel ) ;
  if ( convertPosToXY( pos , &_xl, &_yl ) ) {          //  Convert position index to colonne and line (top left corner) 
    tft.setTextColor(text);
    tft.setTextSize(1);
    //tft.setTextFont(2);
    uint8_t r = min(_w, _h) / 4; // Corner radius
    tft.fillRoundRect( _xl , _yl , _w, _h, r, fill);
    tft.drawRoundRect( _xl, _yl , _w, _h, r, outline);
    uint8_t tempdatum = tft.getTextDatum(); 
    tft.setTextDatum(MC_DATUM);
    int32_t y1 ;
    char tempText[9] ;              // keep max 8 char + /0
    char * pch;
    uint8_t numbChar = 8 ; 
    char tempLabel[50] ;
    memccpy( tempLabel , pbtnLabel , '\0' , 16); // copy max 16 char
    pch = strchr(tempLabel , '*') ; //check for a separator in the name in order to split on 2 lines   
    if  ( pch!=NULL ) {
      numbChar = pch - tempLabel ;
      if (numbChar <= 8 ) {
        tempLabel[numbChar] = 0 ;               // replace * by end of string
        tft.setFreeFont(LABELS9_FONT);
        //tft.setTextFont(2); 
        tft.drawString( &tempLabel[0] , _xl + (_w/2), _yl + (_h/3)); // affiche max 8 char sur la première ligne
        tempLabel[numbChar + 9] = 0 ;           // set a 0 to avoid that second part of name exceed 8 char 
        tft.drawString( &tempLabel[numbChar + 1]  , _xl + (_w/2), _yl + 2 * (_h/3)); // affiche la seconde ligne avec max 8 char
        return;
      }
    }
    // here it is asked to split in 2 lines (or the split was asked after the 8th char)
    // we can still split if there is more than 8 char; if there is less than 4, we use another font 
    tft.setFreeFont(LABELS9_FONT);
    uint8_t txtLength = strlen( pbtnLabel ) ;  // get the length of text to be displayed
    if ( txtLength <= 4 ) {                                               // imprime 1 ligne au milieu en grand
      tft.setFreeFont(LABELS12_FONT); 
      tft.drawString( pbtnLabel , _xl + (_w/2), _yl + (_h/2));  
    } else if ( txtLength <= 8 ) {                                        // imprime 1 ligne au milieu  
        tft.drawString( pbtnLabel , _xl + (_w/2), _yl + (_h/2));  
    } else { 
      if( memccpy( tempLabel , pbtnLabel , '\0' , 16) == NULL ) tempLabel[16] = 0 ; // copy the label in tmp
      pch = strchr(tempLabel , '.') ;
      if  ( pch!=NULL ) {
        numbChar = pch - tempLabel ;
        if (numbChar >= 8 ) numbChar = 8 ;
      }
      uint8_t tempChar =  tempLabel[numbChar] ; // sauvegarde le charactère '.' ou un autre
//      Serial.print("numChar="); Serial.print(numbChar) ;
//      Serial.print("txtLenth="); Serial.print(txtLength) ;
//      Serial.print("tempChar="); Serial.print(tempChar) ; Serial.print(" , "); Serial.print(tempChar , HEX) ;
      tempLabel[numbChar] = 0 ;            // put a 0 instead 
      tft.drawString( tempLabel , _xl + (_w/2), _yl + (_h/3)); // affiche max 7 char sur la première ligne (ou la partie avant .)
      tempLabel[numbChar] = tempChar ;    // restore the char
      if ( (txtLength - numbChar) >= 8 ) {
        tempLabel[numbChar + numbChar + 9 ] = 0 ;     // put a 0 to avoid a too long string 
      }  
      tft.drawString( &tempLabel[numbChar]  , _xl + (_w/2), _yl + 2 * (_h/3)); // affiche la seconde ligne avec max 8 char
    }   
    tft.setTextDatum(tempdatum);
  }  
}

void mButtonBorder(uint8_t pos , uint16_t outline) {  // draw the border of a button at posiition 
  int32_t _xl , _yl ;
  int32_t _w = 76 ;
  int32_t _h = 76 ;
  convertPosToXY( pos , &_xl, &_yl ) ;
  uint8_t r = min(_w, _h) / 4; // Corner radius
  tft.drawRoundRect( _xl, _yl , _w, _h, r, outline);
}



uint8_t getButton( int16_t x, int16_t y ) {    // convert x y into a button if possible
                                                 // return 1 à 8 suivant le bouton; return 0 if no button
  if (y < 80 || y > 240 || x > 320 ) return 0 ;
  int16_t x1 , xReste;
  int16_t y1 , yReste;
  x1 = (x + 5) / 80;
  xReste = (x + 5) % 80;
  y1 = (y - 80) / 80  ;          // to do check if this is ok
  yReste = (y - 80) % 80  ; 
  if  ( (x1 >= 0 && x1 < 4) && (y1 >= 0 && y1 < 2) && ( xReste >= 5 && xReste <= 75 ) && ( yReste >= 5 && yReste <= 75 ) ) {
    return ( x1 + 1) + (y1 * 4 ) ;
  } else {
    return 0 ;
  }
}


//********************************************************************************
//                    look if touch screen has been pressed/released
//                    if pressed, convert the x,y position into a button number (from 1 ...8)
//                    compare to previous state; if previous state is the same, then take new state into account
//                    fill justPressedBtn, justReleasedBtn, currentBtn , longPressedBtn , beginChangeBtnMillis
//*********************************************************************************
#define WAIT_TIME_BETWEEN_TOUCH 100
void updateBtnState( void) {
  int16_t x , y ;
  static uint32_t nextMillis ;
  static uint8_t prevBt0 ; 
  uint32_t touchMillis = millis(); 
  uint8_t bt ;
  uint8_t bt0 = 0; // bt0 is the button nr based on the touched position (0 if not touched, or if touched is not at a valid position, otherwise 1...8)
  boolean touchPressed ;
  justPressedBtn = 0 ;
  justReleasedBtn = 0 ;  
  if ( touchMillis > nextMillis ) {    // s'il n'y a pas assez longtemp depuis la dernière lecture, on fait juste un reset des justPressedBtn et justReleasedBtn
    touchPressed = tft.getTouch( &x,  &y, 600);   // read the touch screen; // later perhaps exit immediately if IrqPin is HIGH (not touched)
                                                // false = key not pressed
    nextMillis = touchMillis + WAIT_TIME_BETWEEN_TOUCH ;
    if ( touchPressed)  {
      bt0 = getButton(x , y);  // convertit x, y en n° de bouton ; retourne 0 si en dehours de la zone des boutons; sinon retourne 1 à 9
//    Serial.print("x=") ; Serial.print(x) ; Serial.print( " ," ) ; Serial.print( y) ; Serial.print( " ," ) ; Serial.println(bt0) ;
    } else {
//      Serial.print("!") ; 
    }
    if ( prevBt0 != bt0 ) { // compare bt0 with previous,
        prevBt0 = bt0 ;     // if different, only save the new state but do not handel  
    } else {  // traite uniquement si 2 fois le même bouton consécutivement 
      if ( currentBtn != bt0 ) {    // en cas de changement, active justPressedBtn et justReleasedBtn et reset waitReleased
//      Serial.print(" x=") ; Serial.print(x) ; Serial.print("  y=") ; Serial.print(y) ; Serial.print("  bt=") ; Serial.println(bt0) ; 
         justPressedBtn = bt0 ;
         justReleasedBtn = currentBtn ;
         currentBtn = bt0 ;
         waitReleased = false ;
         beginChangeBtnMillis = touchMillis ;  // save the timestamp when bt has just changed
         longPressedBtn = 0 ;              // reset long press 
      } else {                  // same button pressed
        if ( ( touchMillis - beginChangeBtnMillis ) > 500 ) {
          longPressedBtn = currentBtn ;    // if the same button is pressed for a long time, update longPressedBtn (can be 0)   
        }  
      }
    }
  }
  if (justPressedBtn){
    //Serial.print( "just pressed") ;   Serial.println( justPressedBtn) ;    
  }
  if (justReleasedBtn){
    //Serial.print( "just released") ;   Serial.println( justReleasedBtn) ;    
  }
}

void drawUpdatedBtn( ) {   // update the color of the buttons on a page (based on currentPage, justPressedBtn , justReleasedBtn, longPressedBtn)
  if ( justReleasedBtn && mPages[currentPage].boutons[justReleasedBtn - 1]) {  // si justReleased contient un bouton et que ce bouton est affiché sur la page
    mButtonBorder( justReleasedBtn , BUTTON_BORDER_NOT_PRESSED ) ; // affiche le bord du bouton dans sa couleur normale
  }
  if ( justPressedBtn && mPages[currentPage].boutons[justPressedBtn - 1]) {  // si justPressed contient un bouton et que ce bouton est affiché sur la page
    mButtonBorder( justPressedBtn , BUTTON_BORDER_PRESSED ) ; // affiche le bord du bouton dans la couleur de sélection
  }
}

void executeMainActionBtn( ) {   // find and execute main action for ONE button (if any)
  uint8_t actionBtn = 0 ;
  if ( ( justPressedBtn ) && ( (mPages[currentPage].actions[justPressedBtn - 1] == _JUST_PRESSED ) \
                            || (mPages[currentPage].actions[justPressedBtn - 1 ] == _JUST_LONG_PRESSED ) \
                            || (mPages[currentPage].actions[justPressedBtn - 1 ] == _JUST_LONG_PRESSED_RELEASED )) ) {
    actionBtn = justPressedBtn  ;
  } else if ( ( longPressedBtn ) && ( (mPages[currentPage].actions[longPressedBtn - 1 ] == _LONG_PRESSED ) \
                                   || (mPages[currentPage].actions[longPressedBtn - 1 ] == _JUST_LONG_PRESSED ) \
                                   || (mPages[currentPage].actions[longPressedBtn - 1 ] == _JUST_LONG_PRESSED_RELEASED )) ) {
    actionBtn = longPressedBtn ;
  } else if ( ( justReleasedBtn ) && ( ( mPages[currentPage].actions[justReleasedBtn - 1 ] == _JUST_RELEASED ) \
                                    || ( mPages[currentPage].actions[justReleasedBtn - 1 ] == _JUST_LONG_PRESSED_RELEASED ) ) ) {
    actionBtn =  justReleasedBtn ;
  }
  if ( actionBtn) {
    (*mPages[currentPage].pfNext[actionBtn - 1 ])(mPages[currentPage].parameters[actionBtn - 1 ]) ;// execute the action with the predefined parameter
  }
}


// Basis functions
void blankTft(char * titel, uint16_t x , uint16_t y) {    // blank screen and display one titel
  tft.fillScreen( SCREEN_BACKGROUND ) ;
  // currently titel is not used so folowwing lines can be skipped; uncomment if titel would be used
  //tft.setTextFont( 2 ); // use Font2 = 16 pixel X 7 probably
  //tft.setTextColor(TFT_GREEN ,  TFT_BLACK) ; // when oly 1 parameter, background = fond);
  //tft.setTextSize(1) ;           // char is 2 X magnified => 
  //tft.setTextDatum( TL_DATUM ) ; // align rigth ( option la plus pratique pour les float ou le statut GRBL)
  //tft.drawString( titel , x , y ) ;     // affiche un texte
  //tft.setCursor( x , y + 30 , 2) ; // x, y, font
}

void printTft(char * text ) {     // print a text on screen
  tft.print( text ) ;
}

// affichage d'une nouvelle page = fonction drawPage(pageIdx)
//          remplir la page d'une couleur
//          exécuter le pointeur correspondant aux paramètres de la page
//          8 fois:
//                 si le n° du bouton existe
//                    afficher le bouton
void drawFullPage() {
  //uint32_t fullPageMillis = millis();
  tft.fillScreen( SCREEN_BACKGROUND ) ;
  mPages[currentPage].pfBase();   // exécute la fonction de base prévue pour la page
  //Serial.println("drawFullPage: fin de l'appel de la page de base") ;
  uint8_t i = 0;
  while ( i < 8 ) {           // pour chacun des 8 boutons possibles
    if ( mPages[currentPage].boutons[i] ) {  // si un n° de bouton est précisé, l'affiche)
//      Serial.print("va afficher le bouton ") ; Serial.println( i) ;
//      Serial.print("bouton code= " ) ; Serial.println( mPages[currentPage].boutons[i] ) ;
//      delay(3000) ;
      
      mButtonDraw( i + 1 , mPages[currentPage].boutons[i] ) ; // affiche le bouton
    }
    i++ ;
//    Serial.print(i); 
  }
  //Serial.print("drawFullPage takes");Serial.println(millis() - fullPageMillis); // most of time it takes about 100msec to redraw the full screen
}

void drawPartPage() {          // update only the data on screen (not the button)
// si la page courante est celle d'info, met à jour certaines données (positions, statut...)
// si la page courante est celle de Move, met à jour certaines données (positions, statut...)
  if ( currentPage == _P_INFO) {
    drawDataOnInfoPage() ;
  } else if ( currentPage == _P_MOVE) {
    drawWposOnMovePage() ;
  } else if (currentPage == _P_SETUP ){
    drawDataOnSetupPage() ;
  } else if (currentPage == _P_SETXYZ ){
    drawDataOnSetXYZPage() ;
  }
}

//********************************  Fonctions appelées quand on entre dans un écran (appelée via drawFullPage)
//                                  Elles changent seulement les paramètres des boutons.
//************************************************************************************************************                                  
void fInfoBase(void) { 
  updateButtonsInfoPage() ; // met à jour le set up de la page en fonction du statut d'impression
  statusPrintingPrev = 0xFF ;  // fill with a dummy value to force a redraw
  machineStatus0Prev = 0 ;  // fill with a dummy value to force a redraw
  lastMsgChanged = true ;   // force a redraw of Last Msg
  grblLastMessageChanged = true ;
  statusTelnetIsConnectedPrev = ! statusTelnetIsConnected ;
  newInfoPage = true ; 
  drawDataOnInfoPage() ;    // affiche les données sur la page info  
}

void updateButtonsInfoPage (void) { // met à jour le set up de la page en fonction du statut d'impression
  switch ( statusPrinting ) {
    case PRINTING_STOPPED :
      fillMPage (_P_INFO , 3 , _SETUP , _JUST_PRESSED , fGoToPage , _P_SETUP) ;
      fillMPage (_P_INFO , 7 , _PRINT , _JUST_PRESSED , fGoToPage , _P_PRINT) ;
      break ;
    case PRINTING_FROM_SD :
      fillMPage (_P_INFO , 3 , _PAUSE , _JUST_PRESSED , fPause , 0 ) ;
      fillMPage (_P_INFO , 7 , _CANCEL , _JUST_PRESSED , fCancel , 0 ) ;
      break ;
    case PRINTING_ERROR :                                              // to do; not clear what we should do
      fillMPage (_P_INFO , 3 , _SETUP , _JUST_PRESSED , fGoToPage , _P_SETUP) ;
      fillMPage (_P_INFO , 7 , _PRINT , _JUST_PRESSED , fGoToPage , _P_PRINT) ;
      break ;
    case PRINTING_PAUSED :
      fillMPage (_P_INFO , 3 , _RESUME , _JUST_PRESSED , fResume , 0 ) ;
      fillMPage (_P_INFO , 7 , _MORE_PAUSE , _JUST_PRESSED , fGoToPage , _P_PAUSE) ;
      break ;
    case PRINTING_FROM_USB :
      fillMPage (_P_INFO , 3 , _STOP_PC_GRBL , _JUST_PRESSED , fStopPc , 0 ) ;
      fillMPage (_P_INFO , 7 , _NO_BUTTON , _JUST_PRESSED , fGoToPage , _P_INFO ) ;
      break ;
    case PRINTING_FROM_TELNET :
      fillMPage (_P_INFO , 3 , _STOP_PC_GRBL , _JUST_PRESSED , fStopPc , 0 ) ;
      fillMPage (_P_INFO , 7 ,_NO_BUTTON , _JUST_PRESSED , fGoToPage, _P_INFO ) ;
      break ;
    case PRINTING_CMD :
      fillMPage (_P_INFO , 3 , _PAUSE , _JUST_PRESSED , fPause , 0 ) ;
      fillMPage (_P_INFO , 7 , _CANCEL , _JUST_PRESSED , fCancel , 0 ) ;
      break ;
  }
}


void drawDataOnInfoPage() { // to do : affiche les données sur la page d'info
//            USB<-->Grbl                  Idle                (or Run, Alarm, ... grbl status)  
//                                                 So, printing status (blanco, ,SD-->Grbl  xxx%, USB<-->Grbl , Pause,  Cmd)  = printing status
//                                                 and GRBL status (or Run, Alarm, ... grbl status)
//            Last message                   (ex : card inserted, card removed, card error, Error: 2 
//            Wifi icon
//              Wpos          Mpos
//            X xxxxxpos      xxxxxpos                 
//            Y yyyyypos      yyyyypos       
//            Z zzzzzpos      zzzzzpos  
//            F 100           S 10000
   
  if ( statusPrintingPrev != statusPrinting ) {
    //tft.setTextFont( 2 ); // use Font2 = 16 pixel X 7 probably
    //tft.setTextSize(2) ;           // char is 2 X magnified => 
    tft.setFreeFont (LABELS12_FONT) ;
    tft.setTextSize(1) ;           // char is 2 X magnified => 
    tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ; // when only 1 parameter, background = fond);
    
    tft.setTextDatum( TL_DATUM ) ; // align Left
    tft.setTextPadding (200) ; 
    tft.drawString ( &printingStatusText[statusPrinting][0] , 5 , 0 )  ;
    statusPrintingPrev = statusPrinting ;
  }
  
  if ( statusPrinting == PRINTING_FROM_SD ) {
      //tft.setTextFont( 2 ); // use Font2 = 16 pixel X 7 probably
      //tft.setTextSize(2) ;           // char is 2 X magnified =>
      tft.setFreeFont (LABELS12_FONT) ;
      tft.setTextSize(1) ;           // char is 2 X magnified => 
      tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ; // when only 1 parameter, background = fond);
      tft.setTextDatum( TR_DATUM ) ;
      tft.setTextPadding (40) ;
      tft.drawNumber( ( (100 * sdNumberOfCharSent) / sdFileSize), 170 , 0 ) ;
      tft.setTextPadding (1) ;
      tft.drawString( "%" , 190 , 0 ) ;
      Serial.print( sdNumberOfCharSent ); Serial.print(  " / ") ; Serial.println( sdFileSize ); 
  }

  if ( machineStatus0Prev != machineStatus[0] || machineStatus[0] == 'A' || machineStatus[0] == 'H') {
      //tft.setTextFont( 2 ); // use Font2 = 16 pixel X 7 probably
      //tft.setTextSize(2) ;           // char is 2 X magnified =>
      tft.setFreeFont (LABELS12_FONT) ;
      tft.setTextSize(1) ;           // char is 2 X magnified => 
      tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND) ; // when only 1 parameter, background = fond);
      tft.setTextDatum( TR_DATUM ) ;
      tft.setTextPadding (120) ;      // expect to clear 70 pixel when drawing text or 
      tft.drawString( &machineStatus[0] , 315  , 0 ) ; // affiche le status GRBL (Idle,....)
      machineStatus0Prev = machineStatus[0] ;
  }

  if ( lastMsgChanged ) {    
      tft.setTextSize(1) ;
      tft.setTextColor(SCREEN_ALERT_TEXT ,  SCREEN_BACKGROUND ) ;
      tft.setTextDatum( TL_DATUM ) ; // align Left
      tft.setTextPadding (320) ; 
      tft.setFreeFont(LABELS9_FONT); 
      if ( strlen( lastMsg) > 30 ) {
        tft.drawString ( " " , 2 , 32) ; // print space first in the larger font to clear the pixels
        tft.setTextFont( 1 );           
      }
      tft.drawString ( &lastMsg[0] , 2 , 32) ;
      lastMsgChanged = false ;
  }

  if ( statusTelnetIsConnectedPrev != statusTelnetIsConnected) {    
      tft.setTextFont( 4);                              // font 4has been modified to have a wifi Icon for 0X7F
      tft.setTextPadding (10) ;
      tft.setTextSize(1) ;  
      if ( statusTelnetIsConnected ) {
        tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ;    // display in green when connected
      } else {
        tft.setTextColor(SCREEN_ALERT_TEXT ,  SCREEN_BACKGROUND ) ;      // display in red when not connected
      } 
      tft.setTextDatum( TL_DATUM ) ; 
      tft.drawChar (  0x7F , 2 , 62 ) ;  // char 0x7E in font 4 has been redesigned to get the Wifi logo
      statusTelnetIsConnectedPrev = statusTelnetIsConnected ;
  }    

  if ( grblLastMessageChanged ) {    
     
      tft.setTextSize(1) ;
      tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ;
      tft.setTextDatum( TL_DATUM ) ; // align Left
      tft.setTextPadding (290) ; 
      tft.setFreeFont(LABELS9_FONT);
      if ( strlen( grblLastMessage) > 30 ) {
        tft.drawString ( " " , 30 , 62) ; // print space first in the larger font to clear the pixels
        tft.setTextFont( 1 ); 
      }
      tft.drawString ( &grblLastMessage[0] , 30 , 62) ;
      grblLastMessageChanged = false ;
  }
  
  tft.setTextDatum( TR_DATUM ) ; 
  uint16_t line = 90 ;
  if (newInfoPage ) {
    tft.setTextColor( SCREEN_HEADER_TEXT ,  SCREEN_BACKGROUND ) ; 
    tft.setTextFont( 2 );
    tft.setTextSize(1) ;           
    tft.setTextPadding (0) ;      // expect to clear 70 pixel when drawing text or 
    tft.drawString( __WPOS , 70 , line ) ;     // affiche un texte
    tft.drawString( __MPOS , 190 , line ) ;     // affiche un texte
    newInfoPage = false ;
  }
  //tft.setTextSize(2) ;
  //tft.setTextFont( 2 );
  tft.setFreeFont (LABELS12_FONT) ;
  tft.setTextSize(1) ;           // char is 2 X magnified => 
  tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ;
  tft.setTextPadding (120) ;      // expect to clear 100 pixel when drawing text or float
  uint8_t c1 = 120, c2 = c1 + 120 ;
  line += 20 ; tft.drawFloat( wposXYZ[0] , 2 , c1 , line ); tft.drawFloat( mposXYZ[0] , 2 , c2 , line ); 
  line += 32 ; tft.drawFloat( wposXYZ[1] , 2 , c1 , line ); tft.drawFloat( mposXYZ[1] , 2 , c2 , line );
  line += 32 ; tft.drawFloat( wposXYZ[2] , 2 , c1 , line ); tft.drawFloat( mposXYZ[2] , 2 , c2 , line  ); 
  
  tft.setTextFont( 2 );
  tft.setTextSize(1) ;
  tft.setTextPadding (0) ;
  tft.setTextDatum( TL_DATUM ) ;
  tft.setTextColor(SCREEN_HEADER_TEXT ,  SCREEN_BACKGROUND) ; 
  line += 32 ; tft.drawString(__FEED , 90 , line)  ; tft.drawString(__RPM , 205 , line) ;
  tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND) ;
  tft.setTextPadding (80) ;
  tft.setTextDatum( TR_DATUM ) ;
  tft.drawNumber( (long) feedSpindle[0] , 80 , line) ; // here we could add the Feed rate and the spindle rpm
  tft.drawNumber( (long) feedSpindle[1] , 200 , line) ; // here we could add the Feed rate and the spindle rpm

}

void fNoBase(void) {
}

void fSetupBase(void) {
  //tft.setTextFont( 1 ); // use Font2 = 16 pixel X 7 probably
  //tft.setTextSize(2) ;           // char is 2 X magnified => 
  tft.setFreeFont (LABELS12_FONT) ;
  tft.setTextSize(1) ;           // char is 2 X magnified => 
  tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ; // when oly 1 parameter, background = fond);
  tft.setTextDatum( TL_DATUM ) ; // align rigth ( option la plus pratique pour les float ou le statut GRBL)
  //tft.setTextPadding (240) ;      // expect to clear 70 pixel when drawing text or 
  uint8_t line = 2 ;
  uint8_t col = 10 ;
  char ipBuffer[20] ;
  if ( getWifiIp( ipBuffer ) ) { 
    tft.drawString( "IP=" , col , line );
    tft.drawString( ipBuffer , col + 40 , line ); // affiche la valeur avec 3 décimales 
  } else {
    tft.drawString( "No WiFi" , col , line ); 
  }
}

void drawDataOnSetupPage() {  
  //tft.setTextFont( 2 ); // use Font2 = 16 pixel X 7 probably
  //tft.setTextSize(2) ;           // char is 2 X magnified => 
  tft.setFreeFont (LABELS12_FONT) ;
  tft.setTextSize(1) ;           // char is 2 X magnified => 
  tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND) ; // when oly 1 parameter, background = fond);
  tft.setTextDatum( TR_DATUM ) ;
  tft.setTextPadding (120) ;      // expect to clear 70 pixel when drawing text or 
  tft.drawString( &machineStatus[0] , 315  , 0 ) ; // affiche le status GRBL (Idle,....)
 
  
  tft.setTextSize(1) ;
  tft.setTextColor(SCREEN_ALERT_TEXT ,  SCREEN_BACKGROUND ) ;
  tft.setTextDatum( TL_DATUM ) ; // align Left
  tft.setTextPadding (320) ;  
  tft.setFreeFont(LABELS9_FONT);    
  if ( strlen( lastMsg) > 30 ) {
      tft.drawString ( " " , 2 , 32) ;
      tft.setTextFont( 1 ); 
  }
  tft.drawString ( &lastMsg[0] , 2 , 32) ;
}

void fMoveBase(void) {
  
  fillMPage (_P_MOVE , 3 , _D_AUTO , _JUST_LONG_PRESSED , fDist , 0) ;  // reset the button for autochange of speed
  wposMoveInitXYZ[0] = wposXYZ[0];             // save the position when entering (so we calculate distance between current pos and init pos on this screen)
  wposMoveInitXYZ[1] = wposXYZ[1];
  wposMoveInitXYZ[2] = wposXYZ[2];
  drawWposOnMovePage() ;
  // multiplier = 0.01 ; // à voir si cela sera encore utilisé (si on met à jour le bouton distance au fur et à mesure, on peut l'utiliser pour calculer 
  // movePosition = 0 ;  à utiliser si on l'affiche 
  // afficher la position des axes
}


void fSetXYZBase(void) {                 //  En principe il n'y a rien à faire;
  drawWposOnSetXYZPage() ;
}

void drawDataOnSetXYZPage() {
  drawWposOnSetXYZPage() ;
}

void drawWposOnSetXYZPage() {
  tft.setTextFont( 2 ); // use Font2 = 16 pixel X 7 probably
  tft.setTextSize(1) ;           // char is 2 X magnified => 
  tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ; // when oly 1 parameter, background = fond);
  
  tft.setTextDatum( TR_DATUM ) ; // align rigth ( option la plus pratique pour les float ou le statut GRBL)
  tft.setTextPadding (80) ;      // expect to clear 70 pixel when drawing text or   
  uint8_t line = 40 ;
  uint8_t col = 60 ;
  tft.drawFloat( wposXYZ[0] , 2 , col , line ); // affiche la valeur avec 3 décimales 
  tft.drawFloat( wposXYZ[1] , 2 , col + 80 , line );
  tft.drawFloat( wposXYZ[2] , 2 , col + 160 , line );
  tft.drawString( __WPOS , col + 240 , line);
}


void fSdBase(void) {                // cette fonction doit vérifier que la carte est accessible et actualiser les noms des fichiers disponibles sur la carte sd
  if ( ! sdStart() ) {           // try to read the SD card; en cas d'erreur, rempli last message et retourne false
    currentPage = _P_INFO ;      // in case of error, go back to Info
    dirLevel = -1 ;              // force a reload of sd data next time we goes to SD card menu
    fInfoBase () ;               // prepare info page (affiche les data et redétermine les boutons à afficher)
  } else {           // if SD seems OK
    //Serial.println("fSdBase is ok") ;
    sdFileDirCnt = fileCnt() ;     // count the number of files in working dir
    if (sdFileDirCnt == 0) {
      firstFileToDisplay == 0 ;
    } else if ( firstFileToDisplay == 0 ) {
        firstFileToDisplay = 1 ;
    } else if ( (firstFileToDisplay + 4) > sdFileDirCnt ) {
      if (  sdFileDirCnt <= 4 ) {
        firstFileToDisplay = 1 ;     // reset firstFileToDisplay 0 if less than 5 files
      } else {
        firstFileToDisplay = sdFileDirCnt - 3 ;  // keep always 4 files on screen
      }  
    }
    //Serial.print("Nbr de fichiers") ; Serial.println(sdFileDirCnt) ;
    //Serial.print("firstFileToDisplay") ; Serial.println(firstFileToDisplay) ;
    //tft.setTextSize(2) ;
    tft.setFreeFont (LABELS12_FONT) ;
    tft.setTextSize(1) ;
    tft.setTextDatum( TL_DATUM ) ;
    tft.setCursor(20 , 20 ) ; // x, y, font
    tft.print( firstFileToDisplay ) ;
    tft.print( " / " ) ;
    tft.print( sdFileDirCnt ) ;  
    tft.setTextDatum( TR_DATUM ) ;
    char dirName[23] ;
    if ( ! aDir[dirLevel].getName( dirName , 22 ) ) { 
      fillMsg (__DIR_NAME_NOT_FOUND );
      currentPage = _P_INFO ;      // in case of error, go back to Info
      dirLevel = -1 ;              // force a reload of sd data next time we goes to SD card menu
      fInfoBase () ;               // fill info page with basis data (affiche les data et redétermine les boutons à afficher sans les afficher)
      return ;
    }
    tft.drawString( dirName , 319 , 40 );
    if ( ! updateFilesBtn() ) {             // met à jour les boutons à afficher; conserve le premier fichier affiché si possible ; retourne false en cas d'erreur
      //Serial.println( "updateFilesBtn retuns false"); 
      fillMsg( __BUG_UPDATE_FILE_BTN  );
      currentPage = _P_INFO ;      // in case of error, go back to Info
      dirLevel = -1 ;              // force a reload of sd data next time we goes to SD card menu
      fInfoBase () ;               // fill info page with basis data (affiche les data et redétermine les boutons à afficher sans les afficher)
    }
  }
}

void fCmdBase(void) {            //  En principe il n'y a rien à faire; 
}

void drawWposOnMovePage() {
  tft.setTextFont( 2 ); // use Font2 = 16 pixel X 7 probably
  tft.setTextSize(1) ;           // char is 2 X magnified => 
  tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ; // when only 1 parameter, background = fond);
  tft.setTextDatum( TR_DATUM ) ; // align rigth ( option la plus pratique pour les float ou le statut GRBL)
  tft.setTextPadding (80) ;      // expect to clear 70 pixel when drawing text or 
  
  uint8_t line = 40 ;
  uint8_t col = 60 ;
  tft.drawFloat( wposXYZ[0] , 2 , col , line ); // affiche la valeur avec 3 décimales 
  tft.drawFloat( wposXYZ[1] , 2 , col + 80 , line );
  tft.drawFloat( wposXYZ[2] , 2 , col + 160 , line );
  tft.drawString( __WPOS , col + 240 , line);
  line += 15 ;
  tft.drawFloat( wposXYZ[0] - wposMoveInitXYZ[0] , 2 , col , line ); // affiche la valeur avec 3 décimales 
  tft.drawFloat( wposXYZ[1] - wposMoveInitXYZ[1] , 2 , col + 80 , line );
  tft.drawFloat( wposXYZ[2] - wposMoveInitXYZ[2] , 2 , col + 160 , line ) ;
  tft.drawString( __MOVE , col + 240 , line);
}





void touch_calibrate() {
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    //Serial.println("Formating file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    { 
      fs::File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen( SCREEN_BACKGROUND);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(SCREEN_HEADER_TEXT ,  SCREEN_BACKGROUND);

    tft.println(__TOUCH_CORNER );

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(SCREEN_ALERT_TEXT ,  SCREEN_BACKGROUND);
      tft.println(__SET_REPEAT_CAL );
    }

    tft.calibrateTouch(calData, SCREEN_ALERT_TEXT, SCREEN_BACKGROUND , 15);  
    //tft.getTouchCalibration(calData , 15) ; // added by ms for touch_ms_V1 instead of previous
    //tft.setTouch(calData);  // added by ms ; not needed with original because data are already loaded with original version
    tft.setTextColor(SCREEN_NORMAL_TEXT , SCREEN_BACKGROUND );
    tft.println(__CAL_COMPLETED );

    // store data
    fs::File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

void fillMsg( char * msg) {
  memccpy ( lastMsg , msg , '\0' , 79);
  lastMsgChanged = true ;
}

