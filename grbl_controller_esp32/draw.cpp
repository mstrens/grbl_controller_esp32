#include "config.h"
#include "draw.h"
#include "setupTxt.h"
#include "language.h"
#include "TFT_eSPI_ms/TFT_eSPI.h"
#include "FS.h"
#include "actions.h"
#include "menu_file.h"
#include "SdFat.h"
#include "browser.h"
#include "com.h"
#include "log.h"
#include "touch.h"
#include "grbl_file.h"
#include "icons.h"


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

// create tft and touchscreeen
TFT_eSPI tft = TFT_eSPI();
TOUCH touchscreen =  TOUCH();
SPIClass spiTouch(VSPI);

extern M_Button mButton[_MAX_BTN] ;
extern M_Page mPages[_P_MAX_PAGES];
extern M_pLabel mText[_MAX_TEXT];
extern M_pLabel mGrblErrors[_MAX_GRBL_ERRORS] ;
extern M_pLabel mAlarms[_MAX_ALARMS]; 


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
extern SdFat32 sd;  // is created in ino file
extern SdBaseFile aDir[DIR_LEVEL_MAX] ;

extern char cmdName[11][17] ;          // contains the names of the commands

extern uint8_t statusPrinting ;
extern float wposXYZA[4] ;
extern float mposXYZA[3] ;
extern char machineStatus[9];
extern float feedSpindle[2] ;  
extern float overwritePercent[3] ; // first is for feedrate, second for rapid (G0...), third is for RPM



extern char lastMsg[80] ;        // last message to display
extern uint16_t lastMsgColor ;            // color of last Msg

extern uint8_t logBuffer[MAX_BUFFER_SIZE] ;
extern uint8_t * pNext ; // position of the next char to be written
extern uint8_t * pFirst ; // position of the first char
extern uint8_t * pGet ; // position of the last char to be read for display

  

//char sdStatusText[][20]  = { "No Sd card" , "Sd card to check" , "Sd card OK" , "Sd card error"} ;  // pour affichage en clair sur le lcd;
char printingStatusText[][20] = { " " , "SD-->Grbl" , "Error SD-->Grbl" , "Pause SD-->Grbl" , "Usb-->Grbl" ,  "CMD" , "Telnet-->Grbl" , "Set cmd", 
                                "SD in Grbl" , "Pause SD in Grbl" } ; 

//extern int8_t prevMoveX ;
//extern int8_t prevMoveY ;
//extern int8_t prevMoveZ ;
//extern int8_t prevMoveA ;
extern float moveMultiplier ;
// used by nunchuck
extern uint8_t jog_status  ;
extern boolean jogCancelFlag ;
//extern boolean jogCmdFlag  ; 

extern boolean statusTelnetIsConnected ;

extern uint8_t wifiType ; // can be NO_WIFI(= 0), ESP32_ACT_AS_STATION(= 1), ESP32_ACT_AS_AP(= 2)
extern uint8_t grblLink;
extern bool btConnected;

extern uint8_t logBuffer[MAX_BUFFER_SIZE] ;                              // log data
//extern uint8_t * pNext ; // position of the next char to be written
//extern uint8_t * pFirst ; // position of the first char
extern uint8_t * pGet ; // position of the last char to be read for display
uint8_t * pLastLogLine ; // pointer to the last Log line
boolean endOfLog = true ;
float wposMoveInitXYZA[4] ;

// previous values used to optimise redraw of INFO screen
uint8_t statusPrintingPrev;
uint8_t machineStatus0Prev;   // we keep only the first char for fast testing, so we can't use ir when equal to A or H
extern boolean lastMsgChanged ;
boolean statusTelnetIsConnectedPrev ;
boolean newInfoPage ;

char lineOfText[50] ; // Store 1 line of text for Log screen

extern char grblLastMessage[STR_GRBL_BUF_MAX_SIZE] ;
extern boolean grblLastMessageChanged;
extern uint16_t GrblFirstFileToDisplay ;   // 0 = first file in the directory
//extern boolean grblFileReadingBusy = false;
extern int grblFileIdx ; // index in the array where next file name being parse would be written = nbr of grbl file names
extern int grblTotalFilesCount ; // total number of files on grbl sd card; used to avoid reading when no files in current dir
extern int8_t errorGrblFileReading ; // store the error while reading grbl files (0 = no error)
extern char grblDirFilter[100] ; // contains the name of the directory to be filtered; "/" for the root; last char must be "/"
extern char grblFileNames[GRBLFILEMAX][40]; // contain max n filename or directory name with max 40 char.
extern char grblFileNamesTft[4][40]; // contains only the 4 names to be displayed on TFT (needed because name is altered during btn drawing 
extern uint8_t firstGrblFileToDisplay ;   // 0 = first file in the directory

extern uint8_t fileToExecuteIdx ; // save the idex (0...3) of the file to be executed after confimation; 0/3 = sd on tft, 10/3 = sd on GRBL card

extern float runningPercent ; // contains the percentage of char being sent to GRBL from SD card on GRBL_ESP32; to check if it is valid
extern boolean runningFromGrblSd  ; // indicator saying that we are running a job from the GRBL Sd card ; is set to true when status line contains SD:


//**************** normal screen definition.
#if defined( TFT_SIZE) and (TFT_SIZE == 3)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define BTN_SIZE 74
#define ICON_SIZE 74
#define BTN_H_MARGIN 3
#define BTN_V_MARGIN 3
#define BTN_FILE_HEIGHT 54
#define TFT_H_RATIO 1
#define TFT_V_RATIO 1
#define RADIUS_ROUND_RECTANGLE 4
#elif defined(TFT_SIZE) and (TFT_SIZE == 4)
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320
#define BTN_SIZE 100
#define ICON_SIZE 100
#define BTN_H_MARGIN 10
#define BTN_V_MARGIN 3
#define BTN_FILE_HEIGHT 74
#define TFT_H_RATIO 1.5
#define TFT_V_RATIO 1.33333
#define RADIUS_ROUND_RECTANGLE 6
#else
#error TFT_SIZE must be 3 or 4 (see config.h)
#endif



#define XB0 (BTN_H_MARGIN) // B is for begin, E is for end; it was B0 but B0 is already defined in some ESP32 library; so  I renamed B0 to XB0
#define XE0 (BTN_H_MARGIN + BTN_SIZE )   // 3+74
#define XB1 (3*BTN_H_MARGIN + BTN_SIZE ) // was B1 but B1 is already defined in some ESP32 library; so  I renamed B1 to But1
#define XE1 (3*BTN_H_MARGIN + 2*BTN_SIZE ) //3+80+74
#define XB2 (5*BTN_H_MARGIN + 2*BTN_SIZE ) // 3+160
#define XE2 (5*BTN_H_MARGIN + 3*BTN_SIZE ) //3+160+74
#define XB3 (7*BTN_H_MARGIN + 3*BTN_SIZE ) //3+240
#define XE3 (7*BTN_H_MARGIN + 4*BTN_SIZE ) // 3+240+74
#define YB0 (BTN_V_MARGIN) // B is for begin, E is for end; it was B0 but B0 is already defined in some ESP32 library; so  I renamed B0 to XB0
#define YE0 (BTN_V_MARGIN + BTN_SIZE )   // 3+74
#define YB1 (3*BTN_V_MARGIN + BTN_SIZE ) // was B1 but B1 is already defined in some ESP32 library; so  I renamed B1 to But1
#define YE1 (3*BTN_V_MARGIN + 2*BTN_SIZE ) //3+80+74
#define YB2 (5*BTN_V_MARGIN + 2*BTN_SIZE ) // 3+160
#define YE2 (5*BTN_V_MARGIN + 3*BTN_SIZE ) //3+160+74
#define YB3 (7*BTN_V_MARGIN + 3*BTN_SIZE ) //3+240
#define YE3 (7*BTN_V_MARGIN + 4*BTN_SIZE ) // 3+240+74

#define FXB (BTN_H_MARGIN) //3
#define FXE (3*BTN_H_MARGIN + 2*BTN_SIZE ) //3+80+74
#define FYB0 (BTN_V_MARGIN) //2
#define FYE0 (BTN_V_MARGIN + BTN_FILE_HEIGHT ) //2 + 56
#define FYB1 (3*BTN_V_MARGIN + BTN_FILE_HEIGHT ) //2 + 56 + 4
#define FYE1 (3*BTN_V_MARGIN + 2*BTN_FILE_HEIGHT )//2 + 56 + 4  + 56 
#define FYB2 (5*BTN_V_MARGIN + 2*BTN_FILE_HEIGHT ) //2 + 56 + 4  + 56 + 4 
#define FYE2 (5*BTN_V_MARGIN + 3*BTN_FILE_HEIGHT ) //2 + 56 + 4  + 56 + 4 + 56 
#define FYB3 (7*BTN_V_MARGIN + 3*BTN_FILE_HEIGHT ) //2 + 56 + 4  + 56 + 4 + 56 + 4
#define FYE3 (7*BTN_V_MARGIN + 4*BTN_FILE_HEIGHT )//2 + 56 + 4  + 56 + 4 + 56 + 4 + 56 
 

uint16_t btnDefNormal[12][4] = {{ XB0 , XE0 , YB0 , YE0 } ,  // each line contains the Xmin, Xmax, Ymin , Ymax of one button.
                                { XB1 , XE1 , YB0 , YE0 } ,
                                { XB2 , XE2 , YB0 , YE0 } ,
                                { XB3 , XE3 , YB0 , YE0 } ,
                                { XB0 , XE0 , YB1 , YE1 } ,
                                { XB1 , XE1 , YB1 , YE1 } ,
                                { XB2 , XE2 , YB1 , YE1 } ,
                                { XB3 , XE3 , YB1 , YE1 } ,
                                { XB0 , XE0 , YB2 , YE2 } ,
                                { XB1 , XE1 , YB2 , YE2 } ,
                                { XB2 , XE2 , YB2 , YE2 } ,
                                { XB3 , XE3 , YB2 , YE2 } } ;

uint16_t btnDefFiles[12][4] = {{ FXB , FXE , FYB0 , FYE0 } ,  // each line contains the Xmin, Xmax, Ymin , Ymax of one button for the menu File.
                                { FXB , FXE , FYB1 , FYE1 } , // currently they are only 10 btn on this type of screen
                                { FXB , FXE , FYB2 , FYE2 } ,
                                { FXB , FXE , FYB3 , FYE3 } ,
                                { XB2 , XE2 , YB0 , YE0 } ,
                                { XB3 , XE3 , YB0 , YE0 } ,
                                { XB2 , XE2 , YB1 , YE1 } ,
                                { XB3 , XE3 , YB1 , YE1 } ,
                                { XB2 , XE2 , YB2 , YE2 } ,
                                { XB3 , XE3 , YB2 , YE2 } ,
                                { 0 , 0 , 0 , 0 } ,
                                { 0 , 0 , 0 , 0 }    } ;

uint16_t hCoord(uint16_t x){
  return (uint16_t) ( x * TFT_H_RATIO );
}

uint16_t vCoord(uint16_t y){
  return (uint16_t) ( y * TFT_V_RATIO );
}


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
  
  //spiTouch.begin( TOUCH_SCLK , TOUCH_MISO ,TOUCH_MOSI , TOUCH_CS_PIN );
  touchscreen.begin(spiTouch , TOUCH_CS_PIN ) ; // specify the SPI being used (we do not use "SPI" = default from Arduino = HVSPI) and the pin used for touchscreen Chip select 
  pinMode(TFT_LED_PIN , OUTPUT) ;
  digitalWrite(TFT_LED_PIN , HIGH) ;
  
  touch_calibrate(); // call screen calibration
  //tft.printCalibration() ;  // print calibration data (print on Serial port the calibration data ; only for debug
  clearScreen();   // clear screen
}


boolean convertPosToXY( uint8_t pos , int32_t *_x, int32_t *_y , uint16_t btnDef[12][4]){
  if (pos > 0 && pos < 13 ) {                // accept only value from 1 to 12
    *_x = btnDef[pos-1][0];
    *_y = btnDef[pos-1][2];
    return true ;
  } else {
    return false ;
  }

} ;

uint8_t convertBtnPosToBtnIdx( uint8_t page , uint8_t btn ) {  // retrieve the type of button based on the position on TFT (in range 1...12)
  return mPages[page].boutons[btn - 1] ;
}

void drawAllButtons(){
    uint8_t i = 0;
    uint8_t btnIdx;
    while ( i < 12 ) {           // pour chacun des 12 boutons possibles
      btnIdx = mPages[currentPage].boutons[i] ;
      //Serial.print("btnIdx="); Serial.println(btnIdx) ;
      //Serial.print("label de btnIdx="); Serial.println(mButton[btnIdx].pLabel[0]) ;
      
      //if ( btnIdx && btnIdx != _MASKED1 ) {  // si un n° de bouton est précisé, l'affiche sauf si c'est un bouton masqué (ex: _MASKED1)
      if ( btnIdx && mButton[btnIdx].pLabel[0] != 0 ) {  // si un n° de bouton est précisé, l'affiche sauf si c'est un bouton masqué (dont le label est vide)
        //Serial.print("va afficher le bouton ") ; Serial.println( i) ;
        //Serial.print("bouton code= " ) ; Serial.println( mPages[currentPage].boutons[i] ) ;
        mButtonDraw( i + 1 , btnIdx ) ; // affiche le bouton
      }
      i++ ;
    }
}




void mButtonDraw(uint8_t pos , uint8_t btnIdx) {  // draw a button at position (from 1 to 12) ; btnIdx = n° du bouton à afficher
//  si le texte a moins de 5 char, affiche 1 ligne en size 2
//                         9               1               1
//                         17              2               1 ; le . sert de séparateur pour répartir les noms de fichier sur 2 lignes si possible 
// si le btnIdx est un bouton masqué (ex _MASKED1), alors on n'affiche rien
  int32_t _xl , _yl ;
  int32_t _w = BTN_SIZE ;  // dimensions are changed here below when it is a button for a file name
  int32_t _h = BTN_SIZE ;
  int32_t fill = BUTTON_BACKGROUND ;
  int32_t outline = BUTTON_BORDER_NOT_PRESSED ;
  int32_t text = BUTTON_TEXT ;
  const char * pbtnLabel ;
  const uint8_t * pbtnIcon ;  
  boolean isFileName = false ;
  //int32_t y1 ;
  //char tempText[9] ;              // keep max 8 char + /0
  char * pExtensionChar;
  char * pch;
  uint8_t numbChar = 8 ; 
  char tempLabel[50] ;  
  char fileExtension[14] = "." ;
  char fileNameReduced[21] = "0123456789012345..." ;
  boolean convertPosIsTrue ;
    
  pbtnLabel = mButton[btnIdx].pLabel ;
  pbtnIcon = mButton[btnIdx].pIcon ; // 0 means that there is no icon.
  if (  (currentPage == _P_SD && btnIdx >= _FILE0 && btnIdx <= _FILE3  ) ||
        ( currentPage == _P_SD_GRBL  && btnIdx >= _FILE0_GRBL && btnIdx <= _FILE3_GRBL)  ){ // if it is a button for a file name
    _w = 2*BTN_H_MARGIN + 2*BTN_SIZE ;
    _h = BTN_FILE_HEIGHT ;
    isFileName = true ;
    if ( *mButton[btnIdx].pLabel == '/' ) {    // when first char is "/", invert the color and skip the first char
      fill = BUTTON_BORDER_NOT_PRESSED ;
      text = BUTTON_BACKGROUND ;
      pbtnLabel++ ;
    } 
    
    // remove extension if any and put it in fileExtension[]
    pExtensionChar = strrchr( pbtnLabel , '.' ); // search for first . from the right
    if ( pExtensionChar ) {
      *pExtensionChar = 0 ; // if any, replace . by 0 to truncate the string
      memccpy( fileExtension +1 , pExtensionChar + 1 , '\0' , 12) ; // copy max 12 char
      fileExtension[12] = 0 ; // put a 0 as last Char for security      
    } else {
      fileExtension[0] = 0 ; // reduce size of extension to 0
    }
    memccpy( fileNameReduced , pbtnLabel , '\0' , 16) ; // copy max 16 char
      
  }  
//  Serial.print("pos="); Serial.print( pos ) ; Serial.print(" btnIdx="); Serial.print( btnIdx ) ;
//  Serial.print("first char of btnName ="); Serial.println( *pbtnLabel ) ;
  if (  currentPage == _P_SD || currentPage == _P_SD_GRBL) {
    convertPosIsTrue =  convertPosToXY( pos , &_xl, &_yl , btnDefFiles ) ;          //  Convert position index to colonne and line (top left corner) 
  } else {
    convertPosIsTrue =  convertPosToXY( pos , &_xl, &_yl , btnDefNormal ) ;          //  Convert position index to colonne and line (top left corner) 
  }
  if ( convertPosIsTrue ) {
    if (pbtnIcon) {   // when it is an icon, draw it
      tft.drawBitmap( _xl, _yl , pbtnIcon , ICON_SIZE , ICON_SIZE , fill ); 
    } else { // when it is a text, draw the button, the border and the text (can be splitted on 2 lines)
        tft.setTextColor(text);
        tft.setTextSize(1);
        //tft.setTextFont(2);
        //uint8_t r = min(_w, _h) / 4; // Corner radius
        //uint8_t r = RADIUS_ROUND_RECTANGLE;
        tft.fillRoundRect( _xl , _yl , _w, _h, RADIUS_ROUND_RECTANGLE, fill);
        //tft.drawRoundRect( _xl, _yl , _w, _h, RADIUS_ROUND_RECTANGLE, outline);
        uint8_t tempdatum = tft.getTextDatum(); 
        tft.setTextDatum(MC_DATUM);
        
        if (isFileName ) {                // print filename in a special way (larger button)
          tft.setTextDatum(BC_DATUM);
          tft.setFreeFont(LABELS9_FONT);
          tft.drawString( fileNameReduced , _xl + (_w/2), _yl + (_h/2));
          tft.setTextDatum(TC_DATUM);
          tft.drawString( fileExtension , _xl + (_w/2), _yl + (_h/2)); 
          tft.setTextDatum(tempdatum);
          return ;
        }
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
}

void mButtonBorder(uint8_t pos , uint16_t outline) {  // draw the border of a button at position (from 1 to 12)
  int32_t _xl , _yl ;
  int32_t _w = BTN_SIZE ;
  int32_t _h = BTN_SIZE ;
  if (currentPage == _P_SD || currentPage == _P_SD_GRBL ) {
    convertPosToXY( pos , &_xl, &_yl , btnDefFiles) ;
    if ( pos <= 4) { // if it is a button for a file name
      _w =2*BTN_H_MARGIN + 2*BTN_SIZE ;// 74+6+80 ;
      _h = BTN_FILE_HEIGHT ;
    }  
  } else {
    convertPosToXY( pos , &_xl, &_yl , btnDefNormal) ;
  }
  //uint8_t r = min(_w, _h) / 4; // Corner radius
  //uint8_t r = RADIUS_ROUND_RECTANGLE ;
  //tft.drawRoundRect( _xl, _yl , _w, _h, RADIUS_ROUND_RECTANGLE, outline);
  tft.drawRoundRect( _xl-1, _yl-1 , _w+2, _h+2, RADIUS_ROUND_RECTANGLE+1, outline);
  tft.drawRoundRect( _xl-2, _yl-2 , _w+4, _h+4, RADIUS_ROUND_RECTANGLE+2, outline);
  tft.drawRoundRect( _xl-3, _yl-3 , _w+6, _h+6, RADIUS_ROUND_RECTANGLE+3, outline);
}

void mButtonClear(uint8_t pos , uint8_t btnIdx) {  // clear one button at position (from 1 to 12)
  int32_t _xl , _yl ;
  int32_t _w = BTN_SIZE ;  // dimensions are changed here below when it is a button for a file name
  int32_t _h = BTN_SIZE ;
  boolean isFileName = false ;
  boolean convertPosIsTrue ;
  if (  (currentPage == _P_SD && btnIdx >= _FILE0 && btnIdx <= _FILE3  ) ||
        ( currentPage == _P_SD_GRBL  && btnIdx >= _FILE0_GRBL && btnIdx <= _FILE3_GRBL)  ){ // if it is a button for a file name
    _w = 2*BTN_H_MARGIN + 2*BTN_SIZE ; //74+6+80 ;
    _h = BTN_FILE_HEIGHT ; //56 ;
    isFileName = true ;      
  }  
  if (  currentPage == _P_SD || currentPage == _P_SD_GRBL) {
    convertPosIsTrue =  convertPosToXY( pos , &_xl, &_yl , btnDefFiles ) ;          //  Convert position index to colonne and line (top left corner) 
  } else {
    convertPosIsTrue =  convertPosToXY( pos , &_xl, &_yl , btnDefNormal ) ;          //  Convert position index to colonne and line (top left corner) 
  }
  if ( convertPosIsTrue ) {
    tft.fillRoundRect( _xl , _yl , _w, _h, RADIUS_ROUND_RECTANGLE, SCREEN_BACKGROUND);
  }
}
 
uint8_t getButton( int16_t x, int16_t y  , uint16_t btnDef[12][4]) {    // convert x y into a button if possible
                                                 // return 1 à 12 suivant le bouton; return 0 if no button
  if ( y > SCREEN_HEIGHT || x > SCREEN_WIDTH ) return 0 ;
  uint8_t i = 12 ;
  while ( i ) {
    i--;
    if ( x > btnDef[i][0] && x < btnDef[i][1] && y > btnDef[i][2] && y < btnDef[i][3] ) return i+1 ; 
  } 
  return 0 ;
}


//********************************************************************************
//                    look if touch screen has been pressed/released
//                    if pressed, convert the x,y position into a button number (from 1 ...12)
//                    compare to previous state; if previous state is the same, then take new state into account
//                    fill justPressedBtn, justReleasedBtn, currentBtn , longPressedBtn , beginChangeBtnMillis
//*********************************************************************************
#define WAIT_TIME_BETWEEN_TOUCH 50
void updateBtnState( void) {
  int16_t x , y ;
  static uint32_t nextMillis ;
  static uint8_t prevBt0 ; 
  uint32_t touchMillis = millis(); 
  //uint8_t bt ;
  uint8_t bt0 = 0; // bt0 is the button nr based on the touched position (0 if not touched, or if touched is not at a valid position, otherwise 1...12)
  boolean touchPressed ;
  justPressedBtn = 0 ;
  justReleasedBtn = 0 ;  
  if ( touchMillis > nextMillis ) {    // s'il n'y a pas assez longtemps depuis la dernière lecture, on fait juste un reset des justPressedBtn et justReleasedBtn
    touchPressed = touchscreen.getTouch( &x,  &y, 600);   // read the touch screen; // later perhaps exit immediately if IrqPin is HIGH (not touched)
                                                // false = key not pressed
    nextMillis = touchMillis + WAIT_TIME_BETWEEN_TOUCH ;
    if ( touchPressed)  {
      if ( currentPage == _P_SD || currentPage == _P_SD_GRBL ) {             // conversion depend on current screen.
        bt0 = getButton(x , y , btnDefFiles) ;  // convertit x, y en n° de bouton ; retourne 0 si en dehors de la zone des boutons; sinon retourne 1 à 12
      } else {
        bt0 = getButton(x , y , btnDefNormal) ;  // convertit x, y en n° de bouton ; retourne 0 si en dehors de la zone des boutons; sinon retourne 1 à 12
        //Serial.print("btn="); Serial.println(bt0);
      }  
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
  //if (justPressedBtn){
  //  Serial.print( "just pressed") ;   Serial.println( justPressedBtn) ;    
  //}
  //if (justReleasedBtn){
  //  Serial.print( "just released") ;   Serial.println( justReleasedBtn) ;    
  //}
}

void drawUpdatedBtn( ) {   // update the color of the buttons on a page (based on currentPage, justPressedBtn , justReleasedBtn, longPressedBtn)
  if ( justReleasedBtn && mPages[currentPage].boutons[justReleasedBtn - 1] && 
                      (  mButton[mPages[currentPage].boutons[justReleasedBtn - 1]].pLabel[0] != 0 ) ) {  // si justReleased contient un bouton et que ce bouton est affiché sur la page
    mButtonBorder( justReleasedBtn , SCREEN_BACKGROUND ) ; // affiche le bord du bouton dans sa couleur normale // first it was BUTTON_BORDER_NOT_PRESSED
  }
  if ( justPressedBtn && mPages[currentPage].boutons[justPressedBtn - 1] && ( mButton[mPages[currentPage].boutons[justPressedBtn - 1]].pLabel[0] != 0 ) ) {  // si justPressed contient un bouton et que ce bouton est affiché sur la page
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
void blankTft( const char * titel, uint16_t x , uint16_t y) {    // blank screen and display one titel
  clearScreen() ;
  if ( strlen(titel) > 0) {
    //tft.fillScreen( SCREEN_BACKGROUND ) ;
    //currently titel is not used so folowwing lines can be skipped; uncomment if titel would be used
    tft.setTextFont( 2 ); // use Font2 = 16 pixel X 7 probably
    tft.setTextColor(TFT_GREEN ,  TFT_BLACK) ; // when oly 1 parameter, background = fond);
    tft.setTextSize(1) ;           // char is 2 X magnified => 
    tft.setTextDatum( TL_DATUM ) ; // align rigth ( option la plus pratique pour les float ou le statut GRBL)
    tft.drawString( titel , x , y ) ;     // affiche un texte
    tft.setCursor( x , y + vCoord(30) , 2 ) ; // x, y, font
  }
}


void clearScreen() {
  tft.fillScreen( SCREEN_BACKGROUND ) ;
}

void drawLogo() {
  tft.drawBitmap(( hCoord(320)-130) /2 , vCoord(20), logoIcon , 130, 118 , BUTTON_BACKGROUND );
  tft.setTextFont( 2 ); // use Font2 = 16 pixel X 7 probably
  tft.setTextColor(TFT_GREEN ,  TFT_BLACK) ; // when oly 1 parameter, background = fond);
  tft.setTextSize(1) ;           // char is 2 X magnified => 
  tft.setTextDatum( TC_DATUM ) ; // align center
  tft.drawString( "Developped by mstrens & htheatre for MakerFr" , hCoord(160) , vCoord(180) ) ;     // affiche un texte
  tft.drawString( VERSION_TEXT , hCoord(160) , vCoord(200) ) ; 
}

void drawLineText( char * text, uint16_t x, uint16_t y, uint8_t font , uint8_t fontSize, uint16_t color) { // texte, x, y , font, size, color
  tft.setTextFont( font ); // use Font2 = 16 pixel X 7 probably
  tft.setTextColor(color ,  TFT_BLACK) ; // when oly 1 parameter, background = fond);
  tft.setTextSize(fontSize) ;           // char is 2 X magnified => 
  tft.setTextDatum( TC_DATUM ) ; // center align 
  tft.drawString( text , x , y ) ;     // affiche un texte 
}

void clearLine( uint16_t y , uint8_t font , uint8_t fontSize , uint16_t color) { // clear a line based on font and font Size.
  tft.fillRect( 0, y, 320, tft.fontHeight(font)* fontSize , color);
}

void printTft(const char * text ) {     // print a text on screen
  tft.print( text ) ;
}

// affichage d'une nouvelle page = fonction drawPage(pageIdx)
//          remplir la page d'une couleur
//          exécuter le pointeur correspondant aux paramètres de la page
//          12 fois:
//                 si le n° du bouton existe
//                    afficher le bouton
void drawFullPage() {
  //uint32_t fullPageMillis = millis();
  clearScreen() ;
  mPages[currentPage].pfBase();   // exécute la fonction de base prévue pour la page
  //Serial.println("drawFullPage: fin de l'appel de la page de base") ;
  //Serial.print("drawFullPage takes");Serial.println(millis() - fullPageMillis); // most of time it takes about 100msec to redraw the full screen
  drawAllButtons();
}

void drawPartPage() {          // update only the data on screen (not the button)
// si la page courante est celle d'info, met à jour certaines données (positions, statut...)
// si la page courante est celle de Move, met à jour certaines données (positions, statut...)
// etc...
  if ( currentPage == _P_INFO) {
    drawDataOnInfoPage() ;
  } else if ( currentPage == _P_MOVE) {
    drawWposOnMovePage() ;
  } else if (currentPage == _P_SETUP ){
    drawDataOnSetupPage() ;
  } else if (currentPage == _P_SETXYZ ){
    drawDataOnSetXYZPage() ;
  } else if (currentPage == _P_TOOL ){
    drawDataOnToolPage() ;
  } else if (currentPage == _P_OVERWRITE ){
    drawDataOnOverwritePage() ;
  } else if (currentPage == _P_COMMUNICATION ){
    drawDataOnCommunicationPage() ;
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
      fillMPage (_P_INFO , 7 , _SETUP , _JUST_PRESSED , fGoToPage , _P_SETUP) ;
      fillMPage (_P_INFO , 11 , _PRINT , _JUST_PRESSED , fGoToPage , _P_PRINT) ;
      break ;
    case PRINTING_FROM_SD :
      fillMPage (_P_INFO , 7 , _PAUSE , _JUST_PRESSED , fPause , 0 ) ;
      fillMPage (_P_INFO , 11 , _OVERWRITE , _JUST_PRESSED , fGoToPage , _P_OVERWRITE ) ; 
      //fillMPage (_P_INFO , 11 , _CANCEL , _JUST_PRESSED , fCancel , 0 ) ;
      break ;
    case PRINTING_ERROR :                                              // to do; not clear what we should do
      fillMPage (_P_INFO , 7 , _SETUP , _JUST_PRESSED , fGoToPage , _P_SETUP) ;
      fillMPage (_P_INFO , 11 , _PRINT , _JUST_PRESSED , fGoToPage , _P_PRINT) ;
      break ;
    case PRINTING_PAUSED :
      fillMPage (_P_INFO , 7 , _RESUME , _JUST_PRESSED , fResume , 0 ) ;
      fillMPage (_P_INFO , 11 , _MORE_PAUSE , _JUST_PRESSED , fGoToPage , _P_PAUSE) ;
      break ;
    case PRINTING_FROM_USB :
      fillMPage (_P_INFO , 7 , _STOP_PC_GRBL , _JUST_PRESSED , fStopPc , 0 ) ;
      fillMPage (_P_INFO , 11 , _NO_BUTTON , _NO_ACTION , fGoToPage , _P_INFO ) ;
      break ;
    case PRINTING_FROM_TELNET :
      fillMPage (_P_INFO , 7 , _STOP_PC_GRBL , _JUST_PRESSED , fStopPc , 0 ) ;
      fillMPage (_P_INFO , 11 ,_NO_BUTTON , _NO_ACTION , fGoToPage, _P_INFO ) ;
      break ;
    case PRINTING_CMD :
      fillMPage (_P_INFO , 7 , _PAUSE , _JUST_PRESSED , fPause , 0 ) ;
      fillMPage (_P_INFO , 11 , _CANCEL , _JUST_PRESSED , fCancel , 0 ) ;
      break ;
    case PRINTING_STRING :
      fillMPage (_P_INFO , 7 , _NO_BUTTON , _NO_ACTION , fPause , 0 ) ;
      fillMPage (_P_INFO , 11 , _CANCEL , _JUST_PRESSED , fCancel , 0 ) ;
      break ;  
    case PRINTING_FROM_GRBL :
      fillMPage (_P_INFO , 7 , _PAUSE , _JUST_PRESSED , fPause , 0 ) ;
      fillMPage (_P_INFO , 11 , _OVERWRITE , _JUST_PRESSED , fGoToPage , _P_OVERWRITE ) ; 
      //fillMPage (_P_INFO , 11 , _CANCEL , _JUST_PRESSED , fCancel , 0 ) ;
      break ;
    case PRINTING_FROM_GRBL_PAUSED :
      fillMPage (_P_INFO , 7 , _RESUME , _JUST_PRESSED , fResume , 0 ) ;
      fillMPage (_P_INFO , 11 , _MORE_PAUSE , _JUST_PRESSED , fGoToPage , _P_PAUSE) ;
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
    tft.setFreeFont (LABELS12_FONT) ;
    tft.setTextSize(1) ;           // char is 2 X magnified => 
    tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ; // when only 1 parameter, background = fond);
    
    tft.setTextDatum( TL_DATUM ) ; // align Left
    tft.setTextPadding (200) ; 
    tft.drawString ( &printingStatusText[statusPrinting][0] , hCoord(5) , 0 )  ;
    statusPrintingPrev = statusPrinting ;
  }
  
  if ( statusPrinting == PRINTING_FROM_SD ) {
      tft.setFreeFont (LABELS12_FONT) ;
      tft.setTextSize(1) ;           // char is 2 X magnified => 
      tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ; // when only 1 parameter, background = fond);
      tft.setTextDatum( TR_DATUM ) ;
      tft.setTextPadding (40) ;
      tft.drawNumber( ( (100 * sdNumberOfCharSent) / sdFileSize), hCoord(170) , 0 ) ;
      tft.setTextPadding (1) ;
      tft.drawString( "%" , hCoord(170) + 20 , 0 ) ;
      // Serial.print( sdNumberOfCharSent ); Serial.print(  " / ") ; Serial.println( sdFileSize ); 
  }

  if ( statusPrinting == PRINTING_FROM_GRBL ) {
      tft.setFreeFont (LABELS12_FONT) ;
      tft.setTextSize(1) ;           // char is 2 X magnified => 
      tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ; // when only 1 parameter, background = fond);
      tft.setTextDatum( TR_DATUM ) ;
      tft.setTextPadding (40) ;
      tft.drawNumber( runningPercent, hCoord(170) , 0 ) ;
      tft.setTextPadding (1) ;
      tft.drawString( "%" , hCoord(170) + 20 , 0 ) ;
      // Serial.print( sdNumberOfCharSent ); Serial.print(  " / ") ; Serial.println( sdFileSize ); 
  }


  if ( machineStatus0Prev != machineStatus[0] || machineStatus[0] == 'A' || machineStatus[0] == 'H') {
      drawMachineStatus() ;
      machineStatus0Prev = machineStatus[0] ;
  }

  if ( lastMsgChanged ) {  
      drawLastMsg() ;  
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
      tft.drawChar (  0x7F , hCoord(2) , vCoord(62) ) ;  // char 0x7E in font 4 has been redesigned to get the Wifi logo
      statusTelnetIsConnectedPrev = statusTelnetIsConnected ;
  }    

  if ( grblLastMessageChanged ) {    
     
      tft.setTextSize(1) ;
      tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ;
      tft.setTextDatum( TL_DATUM ) ; // align Left
      tft.setTextPadding (290) ; 
      tft.setFreeFont(LABELS9_FONT);
      if ( strlen( grblLastMessage) > 30 ) {
        tft.drawString ( " " , hCoord(30) , vCoord(62)) ; // print space first in the larger font to clear the pixels
        tft.setTextFont( 1 ); 
      }
      tft.drawString ( &grblLastMessage[0] , hCoord(30) , vCoord(62) ) ;
      grblLastMessageChanged = false ;
  }
  
  tft.setTextDatum( TR_DATUM ) ; 
  uint16_t line = vCoord(90) ;
  if (newInfoPage ) {
    tft.setTextColor( SCREEN_HEADER_TEXT ,  SCREEN_BACKGROUND ) ; 
    tft.setTextFont( 2 );
    tft.setTextSize(1) ;           
    tft.setTextPadding (0) ;      // expect to clear 70 pixel when drawing text or 
    tft.drawString( mText[_WPOS].pLabel , hCoord(70) , line ) ;     // affiche un texte
    tft.drawString( mText[_MPOS].pLabel , hCoord(190) , line ) ;     // affiche un texte
    newInfoPage = false ;
  }
  //tft.setTextSize(2) ;
  //tft.setTextFont( 2 );
  tft.setTextDatum( TR_DATUM ) ;
  tft.setFreeFont (LABELS12_FONT) ;
  tft.setTextSize(1) ;           // char is 2 X magnified => 
  tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ;
  tft.setTextPadding (120) ;      // expect to clear 120 pixel when drawing text or float
  uint16_t c1 = hCoord(120), c2 =  c1 + hCoord(120) ;
#ifdef AA_AXIS
  uint16_t lineSpacing1 = vCoord(24) ;
#else 
  uint16_t lineSpacing1 = vCoord(34) ; // we have more space for 3 axis
#endif   
  line += vCoord(20)           ; tft.drawFloat( wposXYZA[0] , 2 , c1 , line ); tft.drawFloat( mposXYZA[0] , 2 , c2 , line ); 
  line += lineSpacing1 ; tft.drawFloat( wposXYZA[1] , 2 , c1 , line ); tft.drawFloat( mposXYZA[1] , 2 , c2 , line );
  line += lineSpacing1 ; tft.drawFloat( wposXYZA[2] , 2 , c1 , line ); tft.drawFloat( mposXYZA[2] , 2 , c2 , line  ); 
#ifdef AA_AXIS
  line += lineSpacing1 ; tft.drawFloat( wposXYZA[3] , 2 , c1 , line ); tft.drawFloat( mposXYZA[3] , 2 , c2 , line  );
#endif  
  tft.setTextFont( 2 );
  tft.setTextSize(1) ;
  tft.setTextPadding (0) ;
  tft.setTextDatum( TL_DATUM ) ;
  tft.setTextColor(SCREEN_HEADER_TEXT ,  SCREEN_BACKGROUND) ; 
  line += lineSpacing1 ; tft.drawString(mText[_FEED].pLabel , hCoord(90) , line)  ; tft.drawString(mText[_RPM].pLabel , hCoord(205) , line) ;
  tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND) ;
  tft.setTextPadding (80) ;
  tft.setTextDatum( TR_DATUM ) ;
  tft.drawNumber( (long) feedSpindle[0] , hCoord(80) , line) ; // here we could add the Feed rate and the spindle rpm
  tft.drawNumber( (long) feedSpindle[1] , hCoord(200) , line) ; // here we could add the Feed rate and the spindle rpm
  //tft.drawBitmap(100, 100, resetIcon , 74, 74, SCREEN_HEADER_TEXT );
}

void fNoBase(void) {
}

void fSetupBase(void) {
  tft.setFreeFont (LABELS9_FONT) ;
  tft.setTextSize(1) ;           // char is 2 X magnified => 
  tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ; // when oly 1 parameter, background = fond);
  tft.setTextDatum( TL_DATUM ) ; // align rigth ( option la plus pratique pour les float ou le statut GRBL)
  uint16_t line = vCoord(2) ;
  //uint8_t col = 1 ;
  //char ipBuffer[20] ;
  //if ( getWifiIp( ipBuffer ) ) { 
  //  tft.drawString( "IP:" , col , line );
  //  tft.drawString( ipBuffer , col + 20 , line ); // affiche la valeur avec 3 décimales 
  //} else {
  //  tft.drawString( "No WiFi" , col , line ); 
  //}
  tft.drawString( ESP32_VERSION,  hCoord(85) , line ) ;
}

void drawDataOnSetupPage() {  
  drawMachineStatus() ;       // draw machine status in the upper right corner
  drawLastMsgAt( 85 , 32 ) ; // Coord are for 3.2 TFT; conversion is done inside the function.
}

void drawLastMsgAt( uint16_t col , uint16_t line ) {
  tft.setTextSize(1) ;
  tft.setTextColor(lastMsgColor ,  SCREEN_BACKGROUND ) ; // color is defined in lastMsgColor ; previously SCREEN_ALERT_TEXT
  tft.setTextDatum( TL_DATUM ) ; // align Left
  tft.setTextPadding (320-col) ; 
  tft.setFreeFont(LABELS9_FONT); 
  if ( strlen( lastMsg) > 30 ) {
     tft.drawString ( " " , hCoord(col) , vCoord(line) ) ; // print space first in the larger font to clear the pixels
     tft.setTextFont( 1 );           
  }
  tft.drawString ( &lastMsg[0] , hCoord(col) , vCoord(line) ) ;
}

void fMoveBase(void) {
  fillMPage (_P_MOVE , POS_OF_MOVE_D_AUTO , _D_AUTO , _JUST_LONG_PRESSED , fDist , _D_AUTO) ;  // reset the button for autochange of speed
  wposMoveInitXYZA[0] = wposXYZA[0];             // save the position when entering (so we calculate distance between current pos and init pos on this screen)
  wposMoveInitXYZA[1] = wposXYZA[1];
  wposMoveInitXYZA[2] = wposXYZA[2];
  wposMoveInitXYZA[3] = wposXYZA[3];
  drawWposOnMovePage() ;
  // multiplier = 0.01 ; // à voir si cela sera encore utilisé (si on met à jour le bouton distance au fur et à mesure, on peut l'utiliser pour calculer 
  // movePosition = 0 ;  à utiliser si on l'affiche 
  // afficher la position des axes
}


void fSetXYZBase(void) {                 
  drawWposOnSetXYZPage() ;
}

void drawMachineStatus() {
      tft.setFreeFont (LABELS12_FONT) ;
      tft.setTextSize(1) ;           // char is 2 X magnified => 
      tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND) ; // when only 1 parameter, background = fond);
      tft.setTextDatum( TR_DATUM ) ;
      tft.setTextPadding (120) ;      // expect to clear 70 pixel when drawing text or 
      tft.drawString( &machineStatus[0] , hCoord(315)  , 0 ) ; // affiche le status GRBL (Idle,....)
}

void drawLastMsg() {  // affiche le lastMsg en ligne 32  
      tft.setTextSize(1) ;
      tft.setTextColor(lastMsgColor ,  SCREEN_BACKGROUND ) ; // color is defined in lastMsgColor ; previously SCREEN_ALERT_TEXT
      tft.setTextDatum( TL_DATUM ) ; // align Left
      tft.setTextPadding (320) ; 
      tft.setFreeFont(LABELS9_FONT); 
      if ( strlen( lastMsg) > 30 ) {
        tft.drawString ( " " , hCoord(2) , vCoord(32) ) ; // print space first in the larger font to clear the pixels
        tft.setTextFont( 1 );           
      }
      tft.drawString ( &lastMsg[0] , hCoord(2) , vCoord(32) ) ;
}
      
void drawDataOnSetXYZPage() {
  drawWposOnSetXYZPage() ;
}

void drawWposOnSetXYZPage() {
  drawMachineStatus() ;
  drawLastMsg() ;
  tft.setTextFont( 2 ); // use Font2 = 16 pixel X 7 probably
  tft.setTextSize(1) ;           // char is 2 X magnified => 
  tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ; // when oly 1 parameter, background = fond);
  tft.setTextDatum( TR_DATUM ) ; // align rigth ( option la plus pratique pour les float ou le statut GRBL)
  tft.setTextPadding (80) ;      // expect to clear 70 pixel when drawing text or   
  uint16_t line = vCoord(60) ; // was 60 ;
  uint16_t col = hCoord(60) ;
  tft.drawString( mText[_WPOS].pLabel , col , line);
  tft.drawFloat( wposXYZA[0] , 2 , col , line ); // affiche la valeur avec 3 décimales 
  tft.drawFloat( wposXYZA[1] , 2 , col + hCoord(80) , line );
  tft.drawFloat( wposXYZA[2] , 2 , col + hCoord(160) , line );
#ifdef AA_AXIS  
  tft.drawFloat( wposXYZA[3] , 2 , col + hCoord(240) , line );
#endif  
}


void fSdBase(void) {                // cette fonction doit vérifier que la carte est accessible et actualiser les noms des fichiers disponibles sur la carte sd
  if ( ! sdStart() ) {           // try to read the SD card; en cas d'erreur, rempli last message et retourne false
    currentPage = _P_INFO ;      // in case of error, go back to Info
    dirLevel = -1 ;              // force a reload of sd data next time we goes to SD card menu
    fInfoBase() ;               // prepare info page (affiche les data et redétermine les boutons à afficher)
  } else {           // if SD seems OK
    
    sdFileDirCnt = fileCnt(dirLevel) ;     // count the number of files in working dir
    if (sdFileDirCnt == 0) {
      firstFileToDisplay = 0 ;
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
    tft.setFreeFont (LABELS9_FONT) ;
    tft.setTextSize(1) ;
    tft.setTextDatum( TL_DATUM ) ;
    tft.setCursor(hCoord(180) , vCoord(20) ) ; // x, y, font
    tft.setTextColor(SCREEN_NORMAL_TEXT)
    tft.print( firstFileToDisplay ) ;
    tft.print( " / " ) ;
    tft.print( sdFileDirCnt ) ;  
    tft.setTextDatum( TR_DATUM ) ;
    char dirName[23] ;
    if ( ! aDir[dirLevel].getName( dirName , 16 ) ) { 
      fillMsg (_DIR_NAME_NOT_FOUND );
      currentPage = _P_INFO ;      // in case of error, go back to Info
      dirLevel = -1 ;              // force a reload of sd data next time we goes to SD card menu
      fInfoBase () ;               // fill info page with basis data (affiche les data et redétermine les boutons à afficher sans les afficher)
      return ;
    }
    tft.drawString( dirName , hCoord(310) , vCoord(40) );
    if ( ! updateFilesBtn() ) {             // met à jour les boutons à afficher; conserve le premier fichier affiché si possible ; retourne false en cas d'erreur
      //Serial.println( "updateFilesBtn retuns false"); 
      fillMsg(_BUG_UPDATE_FILE_BTN  );
      currentPage = _P_INFO ;      // in case of error, go back to Info
      dirLevel = -1 ;              // force a reload of sd data next time we goes to SD card menu
      fInfoBase () ;               // fill info page with basis data (affiche les data et redétermine les boutons à afficher sans les afficher)
    }
  }
}

void fCmdBase(void) {            //  En principe il n'y a rien à faire; 
}

void fLogBase(void) { // fonction pour l'affichage de l'écran Log 
  //Serial.print("justPressedBtn= ") ; Serial.println( justPressedBtn ) ; // to debug
  if (justPressedBtn == 1 ) { // when we go on this function directly from a previous screen using the MASKED button (btn 1 of 12)
                              // then we have to set up pGet in order to view the last lines
                              // this test is needed because the function can be called also when we have the flag updateFullPage = true
    uint8_t count = 0;
    pGet = pNext ;
    if (pGet != pFirst) {
      if (pGet == logBuffer) pGet = logBuffer + MAX_BUFFER_SIZE ;
      pGet--;         // Set Pget on last written Car
    }
    getPrevLogLine();
    pLastLogLine = pGet ;
    endOfLog = true ;
    if (pGet != pFirst) {
      fillMPage (_P_LOG , POS_OF_LOG_PG_PREV , _PG_PREV , _JUST_PRESSED , fLogPrev , 0) ; // activate PREV btn 
    } else {
      fillMPage (_P_LOG , POS_OF_LOG_PG_PREV , _NO_BUTTON , _NO_ACTION , fLogPrev , 0) ;  // deactivate PREV btn
    }
    fillMPage (_P_LOG , POS_OF_LOG_PG_NEXT , _NO_BUTTON , _NO_ACTION , fLogNext , 0) ; // deactivate the NEXT button
    //Serial.println("begin fLogBase");
    //Serial.print("pFirst at begin ") ; Serial.println( pFirst - logBuffer) ; // to debug
    //Serial.print("pget at begin ") ; Serial.println( pGet - logBuffer) ; // to debug
    //Serial.print("pNext at begin ") ; Serial.println( pNext - logBuffer) ; // to debug
    //Serial.print("pNext char (hex) = ") ;  Serial.println( *pNext, HEX ) ; // to debug
    while ( (count < ( N_LOG_LINE_MAX - 1) ) && ( getPrevLogLine()>=0 ) ) count++ ; // move back max 6 line before
    //Serial.print("nbr line to display"); Serial.println(count) ; // to debug
  }
  drawDataOnLogPage() ; // display from current position  
}

void fToolBase(void) {                 //  En principe il n'y a rien à faire;
  //drawWposOnSetXYZPage() ;
}

void fSdShowBase(void) { // fonction pour l'affichage de l'écran Sd show 
  //Serial.print("justPressedBtn= ") ; Serial.println( justPressedBtn ) ; // to debug
  if (justPressedBtn == 7 ) { // when we go on this function directly from a previous screen using a (masked) button (btn 1 of 12)
                              // then we have to set up some data in order to view the file lines
                              // Otherwise this set up is done in the function fSdShowPrev and fSdShowNext
            setShowBuffer() ;
  }
  drawDataOnSdShowPage() ; // display from current position  
}



void fOverBase(void) {                 //  En principe il n'y a rien à faire;
  drawDataOnOverwritePage() ;
}

void drawWposOnMovePage() {
#ifdef AA_AXIS
  //  X-    Y+      X+    Z+
  //  X-    Y+      X+    Z+
  //  X-    Y+      X+    Z+
  //  A-    Y-      A+    Z-
  //  A-    Y-      A+    Z-
  //  A-    Y-      A+    Z-
  //W 0,00 MO,00    Auto Back
  //  0,00  O,00    Auto Back
  //  0,00  O,00    Auto Back
  //  0,00  O,00    Auto Back

  tft.setTextFont( 2 ); // use Font2 = 16 pixel X 7 probably
  tft.setTextSize(1) ;           // char is 2 X magnified => 
  tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ; // when only 1 parameter, background = fond);
  tft.setTextDatum( TR_DATUM ) ; // align rigth ( option la plus pratique pour les float ou le statut GRBL)
  tft.setTextPadding (75) ;      // expect to clear 70 pixel when drawing text or 
  
  uint16_t line = vCoord(160) ;
  uint16_t col1 = hCoord(75) ;
  uint16_t col2 = h(Coord(75 + 80) ;
  tft.drawString( mText[_WPOS].pLabel , col1  , line );
  tft.drawString( mText[_MOVE].pLabel , col2  , line  );
  line += vCoord(16) ;
  //tft.drawString( "  X  " , col  , line + 80 );
  tft.drawFloat( wposXYZA[0] , 2 , col1 , line ); // affiche la valeur avec 2 décimales 
  tft.drawFloat( wposXYZA[0] - wposMoveInitXYZA[0] , 2 , col2 , line  ); // affiche la valeur avec 2 décimales 
  line +=vCoord(16) ;
  //tft.drawString( "  Y  " , col + 160  , line + 80  );
  tft.drawFloat( wposXYZA[1] , 2 , col1 , line );
  tft.drawFloat( wposXYZA[1] - wposMoveInitXYZA[1] , 2 , col2 , line );
  line +=vCoord(16) ;
  //tft.drawString( "  Z  " , col + 160  , line   );
  tft.drawFloat( wposXYZA[2] , 2 , col1 , line );
  tft.drawFloat( wposXYZA[2] - wposMoveInitXYZA[2] , 2 , col2 , line ) ;
  line +=vCoord(16) ;
  tft.drawFloat( wposXYZA[3] , 2 , col1 , line );
  tft.drawFloat( wposXYZA[3] - wposMoveInitXYZA[3] , 2 , col2 , line ) ;
  
#else
  //        Y+      x     Z+
  // Wpos   Y+      0.0   Z+
  // Move   Y+      0.0   Z+
  //   x    Y-      y     Z-
  //   0.0  Y-      0.0   Z-
  //   0.0  Y-      0.0   Z-
  //   X-  Auto     X+    Back
  //   X-  Auto     X+    Back
  //   X-  Auto     X+    Back
  tft.setTextFont( 2 ); // use Font2 = 16 pixel X 7 probably
  tft.setTextSize(1) ;           // char is 2 X magnified => 
  tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ; // when only 1 parameter, background = fond);
  tft.setTextDatum( TR_DATUM ) ; // align rigth ( option la plus pratique pour les float ou le statut GRBL)
  tft.setTextPadding (65) ;      // expect to clear 70 pixel when drawing text or 
  
  uint16_t line = vCoord(10) ;
  uint16_t col = hCoord(70) ;
  tft.drawString( mText[_WPOS].pLabel , col  , line + vCoord(20));
  tft.drawString( mButton[_MOVE].pLabel , col  , line + vCoord(40) );
  
  tft.drawString( "  X  " , col  , line + vCoord(80) );
  tft.drawFloat( wposXYZA[0] , 2 , col , line + vCoord(100) ); // affiche la valeur avec 2 décimales 
  tft.drawFloat( wposXYZA[0] - wposMoveInitXYZA[0] , 2 , col , line + vCoord(120) ); // affiche la valeur avec 2 décimales 

  tft.drawString( "  Y  " , col + hCoord(160)  , line + vCoord(80)  );
  tft.drawFloat( wposXYZA[1] , 2 , col + hCoord(160) , line + vCoord(100) );
  tft.drawFloat( wposXYZA[1] - wposMoveInitXYZA[1] , 2 , col + hCoord(160) , line + vCoord(120) );

  tft.drawString( "  Z  " , col + hCoord(160)  , line   );
  tft.drawFloat( wposXYZA[2] , 2 , col + hCoord(160) , line + vCoord(20) );
  tft.drawFloat( wposXYZA[2] - wposMoveInitXYZA[2] , 2 , col + hCoord(160) , line + vCoord(40) ) ;

#endif
}


void drawDataOnLogPage() {
  // tft has already been cleared before we call this function 
  //Serial.println("begin drawing a page");
  //Serial.print("pFirst at begin ") ; Serial.println( pFirst - logBuffer) ; // to debug
  //Serial.print("pget at begin ") ; Serial.println( pGet - logBuffer) ; // to debug
  //Serial.print("pNext at begin ") ; Serial.println( pNext - logBuffer) ; // to debug
  //tft.setFreeFont (LABELS9_FONT) ;
  tft.setTextFont ( 1 ) ;
  tft.setTextSize(1) ;           // char is 2 X magnified => 
  tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ; // when only 1 parameter, background = fond);
  tft.setTextDatum( TL_DATUM ) ; // align rigth ( option la plus pratique pour les float ou le statut GRBL)
  tft.setTextPadding (0) ;
  
  uint16_t line = vCoord(2) ;
  uint16_t col = 0 ;
  uint8_t count = N_LOG_LINE_MAX ;
  //int16_t nCar = 0 ;
  while (count ) {
    printOneLogLine(col , line );
    count--;
    line += vCoord(10) ; // goes to next line on screen (was20 if we use setTextFont(1)
    if (getNextLogLine() < 0 ) count = 0 ; // stop if next line is not complete
  }
}

void printOneLogLine(uint16_t col , uint16_t line ) {
  lineOfText[49] = 0 ; //make sure there is an 0 at the end
  cpyLineToArray( lineOfText , pGet , 48) ; // copy one line
  tft.drawString( lineOfText , col , line) ;
}

void drawDataOnToolPage() {
  drawMachineStatus() ;       // draw machine status in the upper right corner
  drawLastMsg() ;  
}

char sdShowBuffer[1000] = "1234567890\nA1234567890\nB1234567890\nC1234567890\nD1234567890\nE1234567890\nF12345\nG1234567890\nH1234567890\nI1234567890\n"  ; // buffer containing the data to display
int16_t sdShowFirst = 11 ;     // index in buffer of first char to display in the buffer
int16_t sdShowMax = 11;       // index in buffer of the first char that has not been written in the buffer = Number of char written in sdShowBuffer

extern uint32_t sdShowBeginPos ;      // position in the SD file of the first char stored in the sdShowBuffer array   
extern uint32_t sdMillPos  ;      // position in the SD file of the next char to be sent to GRBL   
uint32_t sdShowNextPos ;         // End of the the data having been displayed ( in fact first char that is not displayed); used by Next
boolean sdShowPrevVisible ;      // true when the prev button must be displayed
boolean sdShowNextVisible ;      // true when the next button must be displayed

int16_t bufferLineLength(int16_t firstCharIdx , int16_t &nCharInFile ) {   //return the number of char to be displayed on TFT       
                                                                //-1=end of buffer, 0=only \n , >1 means there are some char; \n is not counted
                                                                // update nCharInFile with the number of char found in the buffer
  int16_t i = firstCharIdx ;
  int16_t nCharOnTft = 0;
  char c ; 
  nCharInFile = 0 ;
  if ( firstCharIdx >= sdShowMax ) return -1 ; // we are alredy at the end of the buffer 
  while ( i < sdShowMax ) {
    c = sdShowBuffer[i++] ;
    if (  c == '\n' ) { 
      break ;
    } else if ( c!= '\r') nCharOnTft++ ; // 
  }
  nCharInFile = i - firstCharIdx ;
  //Serial.print("For line beginning at ") ; Serial.println(firstCharIdx ); // to debug
  //Serial.print("nChar= ") ; Serial.println(i - firstCharIdx ); // to debug
  return  nCharOnTft ;
}

int16_t prevBufferLineLength( int16_t beginSearchAt ,  int16_t &nCharInFile ) {   //return the number of char to be displayed on TFT in the previous line      
                                                                //-1=No previous line, 0=only \n , >1 means there are some char; \n is not counted
                                                                // update nCharInFile with the number of char found in the buffer
    int16_t i = beginSearchAt ;
    int16_t nCharOnTft = 0;
    char c ;
    nCharInFile = 0 ;
    if (beginSearchAt == 0 ) return -1 ; // we are already at the beginning
    do {
      c = sdShowBuffer[--i] ;
      if ( c == '\n' ) break ; // exit first do on first \n
    } while (  i >= 0 ) ;
    if ( i == 0 ) return -1 ; // we are already at the beginning
    do {
      c = sdShowBuffer[--i] ;
      if ( c == '\n' ) {
        i++ ;             // go one char ahead to point on the first char
        break ; // exit because we found a \n
      }
      if ( c != '\r' ) nCharOnTft++ ; // count the number of char on Tft (skip \r)
      } while (  i > 0 ) ;
    nCharInFile = beginSearchAt - i ;
    return nCharOnTft ;
}
    
#define NCHAR_PER_LINE 23  
uint8_t convertNCharToNLines ( int16_t nChar ) { // return the number of lines needed on Tft to display one line in the show buffer
  if ( nChar < 0) return 0 ;
  return (nChar / NCHAR_PER_LINE ) + 1 ;
}

void drawDataOnSdShowPage() { // this function assume that sdShowBuffer contains the data to be displayed
                              // and that sdShowFirst, sdShowMax, sdShowBeginPos , sdMillPos have been filled
                              // after drawing, it fill the tabel to make PREV and NEXT visible or not
                              // it also save the position in file of the first char not being displayed (so for a Next we can save from there in the buffer)
  tft.setFreeFont (LABELS9_FONT) ;
  tft.setTextSize(1) ;           // char is 2 X magnified => 
  tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ; // when only 1 parameter, background = fond);
  tft.setTextDatum( TL_DATUM ) ; 
  tft.setTextPadding (0) ;
  
  uint16_t line = vCoord(20) ;
  //uint8_t col = 0 ;
  int16_t sdShowCurrent  = sdShowFirst ;   // next char to display (during the loop)
  int16_t nChar ; // number of char in buffer for the current line
  uint8_t nLines ; // number of lines on TFT needed to display the current line from buffer
  int16_t nCharInFile ; //number of char on one line in the SD file
  if ( sdShowFirst >= sdShowMax ) return ; //we are at the end of file, so nothing to display 
  do {
    //if ( sdShowCurrent >= (sdShowFirst + sdShowMax) ) break ;   // exit if we reach the end of the buffer
    //Serial.print(">> do : sdShowCurrent = ") ; Serial.println(sdShowCurrent); // to debug
    nChar = bufferLineLength( sdShowCurrent , nCharInFile ) ; // détermine le nbr de char de la ligne à afficher (sans le \n)
    if (nChar < 0 ) break;                      // exit if we reach the end of the buffer
    nLines = convertNCharToNLines ( nChar ) ;   // calcule le nbr de lignes du TFT pour afficher une ligne du buffer
    if ( ( line + (nLines * vCoord(20)) ) > vCoord(240) ) break ; // exit if we reach the end of the tft or if there are no enough free tft lines for next SD line
    // when it is possible to print the full line on TFT, do it (even on several tft lines
    tft.setCursor( 0 , line ) ;
    if ( (sdShowBeginPos  + sdShowCurrent) >= sdMillPos) tft.setTextColor(SCREEN_TO_SEND_TEXT ,  SCREEN_BACKGROUND ) ;
    tft.print(">") ; // display > on each new line
    //Serial.println( "put >" ); // to debug
    tft.setCursor( hCoord(15) , line ) ;
    uint16_t i = 0 ;
    uint8_t nCharOnTftLine = 0 ;
    while ( i <= nChar ) {                     // display each tft line
       if ( nCharOnTftLine < NCHAR_PER_LINE  ) {
        tft.print(sdShowBuffer[sdShowCurrent + i]) ;
        //Serial.print( sdShowBuffer[sdShowCurrent + i] ); // to debug
        nCharOnTftLine++ ;
        i++;                                  // move to next char
       } else {
        tft.setCursor( hCoord(230) , line ) ;
        tft.print( "+" ) ; // put a + at the end of the line on tft when sd line continue
        line += vCoord(20) ;                          // move to next line
        nCharOnTftLine = 0 ;                       // reset counter of char per line
        tft.setCursor( hCoord(15) , line ) ;
       }
    } // end while
    sdShowCurrent += nCharInFile   ;            
    line += vCoord(20) ;
    //Serial.print("line = ") ; Serial.println(line); // to debug
       
  } while (true ) ;  // end do
  if ( (sdShowBeginPos + sdShowFirst) > 0 ) { // adjust the setup for Prev btn
    fillMPage (_P_SD_SHOW , POS_OF_SD_SHOW_PG_PREV , _PG_PREV , _JUST_PRESSED , fSdShowPrev , 0) ;
  } else {
    fillMPage (_P_SD_SHOW , POS_OF_SD_SHOW_PG_PREV , _NO_BUTTON , _NO_ACTION , fSdShowPrev , 0) ;
  }
  if ( (sdShowBeginPos + sdShowCurrent ) < sdFileSize ) { // adjust the setup for Next btn
    fillMPage (_P_SD_SHOW , POS_OF_SD_SHOW_PG_NEXT , _PG_NEXT , _JUST_PRESSED , fSdShowNext , 0) ;
  } else {
    fillMPage (_P_SD_SHOW , POS_OF_SD_SHOW_PG_NEXT , _NO_BUTTON , _NO_ACTION  , fSdShowNext , 0) ;
  }
  //Serial.print("sdShowBeginPos + sdShowCurrent=") ; Serial.println(sdShowBeginPos + sdShowCurrent); // to debug
  //Serial.print("sdFileSize=") ; Serial.println(sdFileSize); // to debug
  sdShowNextPos = sdShowBeginPos + sdShowCurrent ;              // adjust position in SD file of next char to be displayed if we press Next
  //Serial.print("sdShowNextPos = ") ; Serial.println(sdShowNextPos); // to debug
}

void drawDataOnOverwritePage() {                                // to do : text has still to be coded here
  tft.setFreeFont (LABELS12_FONT) ;
  tft.setTextSize(1) ;           // char is 2 X magnified => 
  tft.setTextColor( SCREEN_HEADER_TEXT ,  SCREEN_BACKGROUND ) ; // when only 1 parameter, background = fond);
  tft.setTextDatum( TL_DATUM ) ; // align left 
  tft.setTextPadding (239) ;      // expect to clear 230 pixel when drawing text or 
  uint16_t line = vCoord(15) ;
  uint16_t col = hCoord(2) ;
  if ( mPages[_P_OVERWRITE].boutons[POS_OF_OVERWRITE_OVERWRITE] == _OVER_SWITCH_TO_SPINDLE ) {
    tft.drawString( mText[_CHANGING_FEEDRATE1].pLabel , col  , line );
    tft.setTextDatum( TR_DATUM ) ; // align rigth 
    tft.drawString( mText[_CHANGING_FEEDRATE2].pLabel , col  + hCoord(230) , line + vCoord(30) );  
  } else {
    tft.drawString( mText[_CHANGING_SPINDLE1].pLabel , col  , line );
    tft.setTextDatum( TR_DATUM ) ; // align rigth 
    tft.drawString( mText[_CHANGING_SPINDLE2].pLabel , col + hCoord(230) , line + vCoord(30) );
  }

  tft.setTextFont( 2 );
  tft.setTextSize(1) ;
  tft.setTextPadding (0) ;
  tft.setTextDatum( TL_DATUM ) ;
  tft.setTextColor(SCREEN_HEADER_TEXT ,  SCREEN_BACKGROUND) ; 
  line = vCoord(180) ;
  tft.drawString(mText[_FEED].pLabel , hCoord(5) , line )  ;
  tft.drawString(mText[_RPM].pLabel , hCoord(5) , line + vCoord(20) ) ;
  tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND) ;
  tft.setTextPadding (70) ;
  tft.setTextDatum( TR_DATUM ) ;
  tft.drawNumber( (long) feedSpindle[0] , hCoord(110) , line) ; 
  tft.drawNumber( (long) feedSpindle[1] , hCoord(110) , line + vCoord(20) ) ; 
  tft.setTextPadding (30) ;
  tft.drawNumber( (long) overwritePercent[0] , hCoord(140) , line) ;
  tft.drawNumber( (long) overwritePercent[2] , hCoord(140) , line + vCoord(20)  ) ;
  tft.setTextPadding (0) ;
  tft.drawString(  "%" , hCoord(155) , line )  ;
  tft.drawString(  "%" , hCoord(155) , line + vCoord(20) )  ;
}

// Fill the first part of the Communication page.
void fCommunicationBase(void) { // fonction pour l'affichage de l'écran communication
  // display the wifi status mode and Ip adress on first line  
  tft.setFreeFont (LABELS9_FONT) ;
  tft.setTextSize(1) ;           // char is 2 X magnified => 
  tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ; // when oly 1 parameter, background = fond);
  tft.setTextDatum( TL_DATUM ) ; // align rigth ( option la plus pratique pour les float ou le statut GRBL)
  uint16_t line = vCoord(2) ;
  uint16_t col = hCoord(100) ;
  char ipBuffer[20] ;
  if (wifiType == NO_WIFI ) {
    tft.drawString( "No WiFi" , col , line );
  } else {
    if (wifiType == ESP32_ACT_AS_STATION ) {
      tft.drawString( "STA: " , col , line );
    } else {
      tft.drawString( "AP: " , col , line );
    }  
    if ( getWifiIp( ipBuffer ) ) { 
      tft.drawString( "IP=" , col + hCoord(50) , line );
      tft.drawString( ipBuffer , col + hCoord(80) , line ); // affiche la valeur avec 3 décimales 
    } else {
      tft.drawString( mText[_NO_IP_ASSIGNED].pLabel , col + hCoord(50)  , line ); 
    }
  }  
}

void drawDataOnCommunicationPage() {
  // Show:  the IP adress (already done in Sbase); wifi mode (no wifi, Station, access point)
  //  
  drawMachineStatus() ;       // draw machine status in the upper right corner
  drawLastMsgAt( 85 , 32 ) ; // Coord are for 3.2 TFT; conversion is done inside the function.
  tft.setFreeFont (LABELS9_FONT) ;
  tft.setTextSize(1) ;           // char is 2 X magnified => 
  tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ; // when oly 1 parameter, background = fond);
  tft.setTextDatum( TL_DATUM ) ; // align rigth ( option la plus pratique pour les float ou le statut GRBL)
  uint16_t line = vCoord(100) ;
  uint16_t col = hCoord(1) ;
  if ( grblLink == GRBL_LINK_SERIAL) {
    tft.drawString( mText[_GRBL_SERIAL_CONNECTED].pLabel , col , line );
  } else if ( grblLink == GRBL_LINK_BT) {
      if (btConnected) {
        tft.drawString( mText[_USES_BT_TO_GRBL].pLabel , col , line );
      } else {
        tft.drawString(mText[_UNABLE_TO_CONNECT_IN_BT].pLabel  , col , line );
      }
  } else if ( grblLink == GRBL_LINK_TELNET) {
    tft.drawString( mText[_USES_TELNET_TO_GRBL].pLabel , col , line );
  }
  tft.drawString( " " , col , line+ vCoord(20) ); // clear the second line used to say "please wait"  
}

void drawMsgOnTft(const char * msg1 , const char * msg2){
  tft.setFreeFont (LABELS9_FONT) ;
  tft.setTextSize(1) ;           // char is 2 X magnified => 
  tft.setTextColor(SCREEN_NORMAL_TEXT ,  SCREEN_BACKGROUND ) ; // when oly 1 parameter, background = fond);
  tft.setTextDatum( TL_DATUM ) ; // align left
  tft.setTextPadding (hCoord(239)) ;
  uint16_t line = vCoord(100) ;
  uint16_t col = hCoord(1) ;
  tft.drawString( msg1 , col , line );
  tft.drawString( msg2 , col , line+ vCoord(20) );  
}

/*
Exemple
[FILE:/System Volume Information/WPSettings.dat|SIZE:12]
[FILE:/System Volume Information/IndexerVolumeGuid|SIZE:76]
[FILE:/pcbtft_110320.pdf|SIZE:39253]
[FILE:/GRBL32_V2_110320.pdf|SIZE:76953]
[FILE:/TestRoot1.nc|SIZE:5]
[FILE:/TestRoot2.nc|SIZE:5]
[FILE:/TestRoot3.nc|SIZE:5]
[FILE:/dir1/TestDir3.nc|SIZE:5]
[FILE:/dir1/TestDir1.nc|SIZE:5]
[FILE:/dir1/TestDir2.nc|SIZE:5]
[SD Free:119.66 MB Used:288.00 KB Total:119.94 MB]

and here list of SD errors
"60","SD failed to mount"
"61","SD card failed to open file for reading"
"62","SD card failed to open directory"
"63","SD Card directory not found"
"64","SD Card file empty"
*/

void fSdGrblWaitBase(void) {                // cette fonction doit vérifier que la carte est accessible et actualiser les noms des fichiers disponibles sur la carte sd attahée à grbl
// sauvegarder le fichier courant (au moins son index)
// Appeler une fonction qui:
//    demande à GRBL la liste des fichiers présents dans le répertoire courant
//    afficher un écran disant de patienter et indiquant le dir en cours de lecture
//    Enregister pourquoi on fait appel à cette fonction    
//    activer un flag qui sera lu dans Loop et provoquera l'appel d'une fonction callback quand les données auront été lue (ou après un timeout ou une erreur) 

// Dans la fonction callback:
// S'il y a une erreur, générer un Msg et retour à l'écran Info 
// Si le fichier courrant existe encore, affiche la liste à partir de ce fichier.
// Si pas et s'il n'y a pas d'erreur, changer le répertoire courant en "/" et relancer la lecture
//
// Si on descend d'un niveau dans les directory, changer le filtre et reconstruire la liste
// Si on remonte d'un niveau, idem
// Avoir un array (ou un string) avec les noms des directory du filtre
// Avoir un array avec les x fichiers ou directory
// Pour lancer une recherche, vider l'array de 50, remplir le filtre et lancer la recherche
// La fonction qui lit les caractères venant de GRBL doit reconnaitre et extraire les messages commençant par [FILE:/, et sauter ceux qui commence par System Volume Information
// Elle doit accumuler les caractères dans un char array jusqu'à la fin de ligne
// Quand elle voit une ligne qui commence par [SD Free, elle active le flag de fin.
//  Quand on reçoit une erreur de Grbl dans la série 60, on mémorise cette erreur 
    grblReadFiles( GRBL_FIRST_READ); 
    tft.setFreeFont (LABELS9_FONT) ;
    tft.setTextSize(1) ;
    tft.setTextDatum( TL_DATUM ) ;
    tft.setCursor(0 , vCoord(20) ) ; // x, y, font
    tft.setTextColor(SCREEN_NORMAL_TEXT) ;
    tft.println( mText[_CURRENT_GRBL_DIR].pLabel );
    tft.println(grblDirFilter) ;
    tft.println(" ") ;
    tft.println(mText[_READING_FILES_ON_GRBL].pLabel);
    tft.println(mText[_PLEASE_WAIT].pLabel);
}

void executeGrblEndOfFileReading(){
  // check the reason for reading and act
  if ( errorGrblFileReading ) {
    fGoToPage(_P_INFO) ; // go to info in case of error; the error is in the Msg
  //} else if ( grblFileIdx == 0)  { // No file found with current dir
    //if ( grblTotalFilesCount == 0 ) { // go to info in case there are no file at all   
    //  fillMsg("Grbl SD card is empty") ;
    //  fGoToPage(_P_INFO) ; 
    //} else {
    //  fillMsg("Still to do with dir = /") ;
    //  fGoToPage(_P_INFO) ; 
    //}
  } else {
    fGoToPage(_P_SD_GRBL) ; // will display the file list 
  }  
}

void fSdGrblBase(void) {                // cette fonction est appelée une fois la liste de fichier chargée en mémoire (donc par exemple fsdWaitGrblBase doit avoir rempli les noms)
    if (  grblFileIdx == 0)  { // No file in the array 
      firstGrblFileToDisplay = 0;
    } else if  ( firstGrblFileToDisplay == 0) {
      firstGrblFileToDisplay = 0 ;
    } else if ( (firstGrblFileToDisplay + 4) > grblFileIdx ) {
        if (  grblFileIdx <= 4 ) {
          firstGrblFileToDisplay = 0 ;     // reset firstFileToDisplay 0 if less than 5 files
        } else {
          firstGrblFileToDisplay = grblFileIdx - 4 ;  // keep always 4 files on screen
        }  
    }
    //tft.setTextSize(2) ;
    tft.setFreeFont (LABELS9_FONT) ;
    tft.setTextSize(1) ;
    tft.setTextDatum( TL_DATUM ) ;
    tft.setCursor(hCoord(180) , vCoord(20) ) ; // x, y, font
    tft.setTextColor(SCREEN_NORMAL_TEXT) ;
    tft.print( firstGrblFileToDisplay + 1 ) ; // we add 1 because first file has an index of 0
    tft.print( " / " ) ;
    tft.print( grblFileIdx ) ;  
    tft.setTextDatum( TR_DATUM ) ;
    if ( strlen(grblDirFilter) <= 1 ) {
      tft.drawString( "/" , hCoord(310) , vCoord(40) );
    } else {  // search the current 'last' dir in the full filtering name 
      char * plastDirFilter ; // pointer to the begin of current dir (based on current filter 
      plastDirFilter = grblDirFilter + strlen(grblDirFilter) - 2;
      while ( ((*plastDirFilter) != '/')  && (plastDirFilter >= grblDirFilter) ) plastDirFilter--; 
      tft.drawString( plastDirFilter , hCoord(310) , vCoord(40) );
    }
    updateGrblFilesBtn() ;              // met à jour les boutons à afficher;
}

void fConfirmYesNoBase() { // should display the name of the file to be printed just before execution
  char * pfileName ;
  if ( fileToExecuteIdx <=3 ){ //we are printing from Sd card on TFT
    pfileName = fileNames[fileToExecuteIdx] ;
  } else if ( fileToExecuteIdx >=10 && fileToExecuteIdx <=13 ) {
    pfileName = grblFileNames[fileToExecuteIdx-10] ;
  } else {
    *pfileName = NULL ;
  }
  tft.setFreeFont (LABELS9_FONT) ;
  tft.setTextSize(1) ;
  tft.setTextDatum( TL_DATUM ) ;
  tft.setCursor(hCoord(20) , vCoord(100) ) ; // x, y, font
  tft.setTextColor(SCREEN_NORMAL_TEXT)
  tft.print( mText[_CONFIRM_SD_FILE].pLabel ) ; 
  tft.setCursor(hCoord(20) , vCoord(140) ) ; // x, y, font
  if (pfileName ) tft.print( pfileName ) ;
}

// ********************************************************************************************
// ******************************** touch calibrate ********************************************
//#define DEBUG_CALIBRATION
void touch_calibrate() {
  uint16_t calData[5]; // contains calibration parameters 
  uint8_t calDataOK = 0;
  bool repeatCal = REPEAT_CAL;  // parameter in the config.h file (true when calibration is requested)
  if (checkCalibrateOnSD()) { // to do todo MS to remove
    repeatCal = true ; // force a recalibration if a file "calibrate.txt" exist on SD card
  }
  // check file system exists
  if (!SPIFFS.begin()) {
    #ifdef DEBUG_CALIBRATION
    Serial.println("Formating file system");
    #endif
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (repeatCal)
    {
      // Delete if we want to re-calibrate
      #ifdef DEBUG_CALIBRATION
      Serial.println("Remove file system");
      #endif
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    { 
      fs::File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        #ifdef DEBUG_CALIBRATION
        Serial.println("File system opened for reading");
        #endif
        if (f.readBytes((char *)calData, 14) == 14)
          #ifdef DEBUG_CALIBRATION
          Serial.println("Cal Data has 14 bytes");
          #endif
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !repeatCal) {
    // calibration data valid
    #ifdef DEBUG_CALIBRATION
    Serial.println("Use calData");
    #endif
    touchscreen.setTouch(calData);
  } else {
    // data not valid so recalibrate
    #ifdef DEBUG_CALIBRATION
    Serial.println("Perform calibration");
    #endif
    clearScreen();
    tft.setCursor(hCoord(20), 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(SCREEN_HEADER_TEXT ,  SCREEN_BACKGROUND);
    tft.println(mText[_TOUCH_CORNER].pLabel );
    tft.setTextFont(1);
    tft.println();

    if (repeatCal) {
      tft.setTextColor(SCREEN_ALERT_TEXT ,  SCREEN_BACKGROUND);
      tft.println(mText[_SET_REPEAT_CAL].pLabel );
    }

    perform_calibration(calData, SCREEN_ALERT_TEXT, SCREEN_BACKGROUND , 15 , TFT_HEIGHT, TFT_WIDTH);  // we invert height and width because we use rotation 1 (= landscape) 
    //tft.getTouchCalibration(calData , 15) ; // added by ms for touch_ms_V1 instead of previous
    //tft.setTouch(calData);  // added by ms ; not needed with original because data are already loaded with original version
    tft.setTextColor(SCREEN_NORMAL_TEXT , SCREEN_BACKGROUND );
    tft.println(mText[_CAL_COMPLETED].pLabel );

    // store data
    fs::File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      #ifdef DEBUG_CALIBRATION
      Serial.println("Write calibration in file system");
      #endif
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

/***************************************************************************************
** Function name:           calibrateTouch
** Description:             generates calibration parameters for touchscreen. 
***************************************************************************************/
void perform_calibration(uint16_t *parameters, uint32_t color_fg, uint32_t color_bg, uint8_t size, int16_t _width, int16_t _height){
  int16_t values[] = {0,0,0,0,0,0,0,0};
  uint16_t x_tmp, y_tmp , z_tmp ;
  uint16_t touchCalibration_x0 ;
  uint16_t touchCalibration_x1 ;
  uint16_t touchCalibration_y0 ;
  uint16_t touchCalibration_y1;
  uint8_t  touchCalibration_rotate;
  uint8_t  touchCalibration_invert_x;
  uint8_t  touchCalibration_invert_y;
  
  for(uint8_t i = 0; i<4; i++){
    tft.fillRect(0, 0, size+1, size+1, color_bg);
    tft.fillRect(0, _height-size-1, size+1, size+1, color_bg);
    tft.fillRect(_width-size-1, 0, size+1, size+1, color_bg);
    tft.fillRect(_width-size-1, _height-size-1, size+1, size+1, color_bg);

    if (i == 5) break; // used to clear the arrows
    
    switch (i) {
      case 0: // up left
        tft.drawLine(0, 0, 0, size, color_fg);
        tft.drawLine(0, 0, size, 0, color_fg);
        tft.drawLine(0, 0, size , size, color_fg);
        break;
      case 1: // bot left
        tft.drawLine(0, _height-size-1, 0, _height-1, color_fg);
        tft.drawLine(0, _height-1, size, _height-1, color_fg);
        tft.drawLine(size, _height-size-1, 0, _height-1 , color_fg);
        break;
      case 2: // up right
        tft.drawLine(_width-size-1, 0, _width-1, 0, color_fg);
        tft.drawLine(_width-size-1, size, _width-1, 0, color_fg);
        tft.drawLine(_width-1, size, _width-1, 0, color_fg);
        break;
      case 3: // bot right
        tft.drawLine(_width-size-1, _height-size-1, _width-1, _height-1, color_fg);
        tft.drawLine(_width-1, _height-1-size, _width-1, _height-1, color_fg);
        tft.drawLine(_width-1-size, _height-1, _width-1, _height-1, color_fg);
        break;
      }

    // user has to get the chance to release
    if(i>0) delay(1000);
    while (touchscreen.getTouchRaw( &x_tmp, &y_tmp, &z_tmp )) ;  // wait that touch has been released
    for(uint8_t j= 0; j<8; j++){
      while ( !touchscreen.getTouchRaw( &x_tmp, &y_tmp, &z_tmp )) ;
      // Serial.print("cal x y for corner = ") ; Serial.print(i) ; Serial.print(" , ") ; Serial.print(x_tmp) ; Serial.print(" , ") ; Serial.println(y_tmp) ;  Serial.print(" , ") ; Serial.println(z_tmp) ;
      values[i*2  ] += x_tmp;
      values[i*2+1] += y_tmp;
    } // end for (8X)
    values[i*2  ] /= 8;
    values[i*2+1] /= 8;
  } // end for (4X) 


  // from case 0 to case 1, the y value changed. 
  // If the measured delta of the touch x axis is bigger than the delta of the y axis, the touch and TFT axes are switched.
  touchCalibration_rotate = false;
  if(abs(values[0]-values[2]) > abs(values[1]-values[3])){
    touchCalibration_rotate = true;
    touchCalibration_x0 = (values[1] + values[3])/2; // calc min x
    touchCalibration_x1 = (values[5] + values[7])/2; // calc max x
    touchCalibration_y0 = (values[0] + values[4])/2; // calc min y
    touchCalibration_y1 = (values[2] + values[6])/2; // calc max y
  } else {
    touchCalibration_x0 = (values[0] + values[2])/2; // calc min x
    touchCalibration_x1 = (values[4] + values[6])/2; // calc max x
    touchCalibration_y0 = (values[1] + values[5])/2; // calc min y
    touchCalibration_y1 = (values[3] + values[7])/2; // calc max y
  }

  // in addition, the touch screen axis could be in the opposite direction of the TFT axis
  touchCalibration_invert_x = false;
  if(touchCalibration_x0 > touchCalibration_x1){
    values[0]=touchCalibration_x0;
    touchCalibration_x0 = touchCalibration_x1;
    touchCalibration_x1 = values[0];
    touchCalibration_invert_x = true;
  }
  touchCalibration_invert_y = false;
  if(touchCalibration_y0 > touchCalibration_y1){
    values[0]=touchCalibration_y0;
    touchCalibration_y0 = touchCalibration_y1;
    touchCalibration_y1 = values[0];
    touchCalibration_invert_y = true;
  }

  // pre calculate
  touchCalibration_x1 -= touchCalibration_x0;
  touchCalibration_y1 -= touchCalibration_y0;

  if(touchCalibration_x0 == 0) touchCalibration_x0 = 1;
  if(touchCalibration_x1 == 0) touchCalibration_x1 = 1;
  if(touchCalibration_y0 == 0) touchCalibration_y0 = 1;
  if(touchCalibration_y1 == 0) touchCalibration_y1 = 1;

  // export parameters, if pointer valid
  if(parameters != NULL){
    parameters[0] = touchCalibration_x0;
    parameters[1] = touchCalibration_x1;
    parameters[2] = touchCalibration_y0;
    parameters[3] = touchCalibration_y1;
    parameters[4] = touchCalibration_rotate | (touchCalibration_invert_x <<1) | (touchCalibration_invert_y <<2);
    touchscreen.setTouch(parameters) ; // save parameters in the touchscreen class.
  }
#ifdef DEBUG_CALIBRATION
  Serial.println(  " =>  end of calibration lib" ) ;
#endif  
}


boolean checkCalibrateOnSD(void){
      // return true if a file calibrate.txt exist on the SD card; if so it means that new calibration is requested
      SdBaseFile calibrateFile ;  
      if ( ! sd.begin(SD_CHIPSELECT_PIN , SD_SCK_MHZ(5)) ) {  
          Serial.print("[MSG:");Serial.print( mText[_CARD_MOUNT_FAILED].pLabel  ) ;Serial.println("]");
          return false;       
      }
      //if ( ! SD.exists( "/" ) ) { // check if root exist
      if ( ! sd.exists( "/" ) ) { // check if root exist   
          Serial.print("[MSG:");Serial.println( mText[_ROOT_NOT_FOUND].pLabel  ) ;Serial.println("]");
          return false;  
      }
      if ( ! sd.chdir( "/" ) ) {
          Serial.print("[MSG:");Serial.println( mText[_CHDIR_ERROR].pLabel  ) ;Serial.println("]");
          return false;  
      }
      if ( ! calibrateFile.open("/calibrate.txt" ) ) { // try to open calibrate.txt 
          Serial.println("[MSG:failed to open calibrate.txt]" ) ;
          return false;  
      }
      Serial.println("calibrate.txt exist on SD" ) ;      
      calibrateFile.close() ;
      return true ;
}




void fillMsg(uint8_t msgIdx , uint16_t color) {
  memccpy ( lastMsg , mText[msgIdx].pLabel , '\0' , 79);
  lastMsgColor = color ;
  lastMsgChanged = true ;
}

void clearMsg(uint16_t color ){
  memccpy ( lastMsg , " " , '\0' , 79);
  lastMsgColor = color ;
  lastMsgChanged = true ;
}

void fillStringMsg( char * msg, uint16_t color) {
  memccpy ( lastMsg , msg , '\0' , 79);
  lastMsgColor = color ;
  lastMsgChanged = true ;
}

