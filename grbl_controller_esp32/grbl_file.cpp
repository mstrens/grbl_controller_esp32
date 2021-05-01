
#include "grbl_file.h"
#include "Arduino.h"
//#include "SD.h"
#include "SdFat.h"
#include "config.h"
#include "language.h"
#include "draw.h"
#include "setupTxt.h"
#include "actions.h"
#include "cmd.h"
#include "com.h" 

char grblDirFilter[100] = "/" ; // contains the name of the directory to be filtered; "/" for the root; last char must be "/"
char grblFileNames[GRBLFILEMAX][40]; // contain max n filename or directory name with max 40 char.
char grblFileNamesTft[4][40]; // contains only the 4 names to be displayed on TFT (needed because name is altered during btn drawing 
//int grblFileCount = 0 ; // number of entries in grblFileName
uint8_t firstGrblFileToDisplay ;   // 0 = first file in the directory
int8_t errorGrblFileReading ; // store the error while reading grbl files (0 = no error)
int grblFileIdx ; // index in the array where next file name being parse would be written
int grblTotalFilesCount ; // total number of files on grbl sd card; used to avoid reading when no files in current dir

extern uint8_t parseGrblFilesStatus ; 
//extern SdBaseFile aDir[DIR_LEVEL_MAX] ;

extern M_Button mButton[_MAX_BTN] ;
extern uint8_t statusPrinting ;

extern uint16_t sdFileDirCnt ;

extern boolean updateFullPage ;
extern boolean waitReleased ;

//uint16_t sdFileRootCnt ; // count the nbr of files in root; used to check if SD card has been changed
//extern char fileNames[4][23] ; // 22 car per line + "\0"
//extern int16_t encoderTopLine ;

//extern uint32_t sdFileSize ;
//extern uint32_t sdNumberOfCharSent ;

extern volatile boolean waitOk ;
//extern int8_t dirLevel ;


void grblReadFiles( uint8_t reason){ // request grbl to provide the list of file.
  errorGrblFileReading = 0; // reset the error
  grblFileIdx = 0 ; // reset the array of file
  grblTotalFilesCount = 0 ; // reset the total number of files
  parseGrblFilesStatus = PARSING_FILE_NAMES_RUNNING ; // allows com.cpp to get the [FILES:.... lines and to store them   
  toGrbl("$SD/List\r\n");
}

// Cette fonction affiche max N (=MAW_LCD_ROWS) noms de fichiers sélectionnés dans le directory courrant à partir du Xème fichier
// 
// utilise encoderTopLine = menu index du premier élément sur l'écran
//         encoderPosition = valeur de l'encodeur = index du fichier à afficher si possible
//        _lcdLineNr is the index of the LCD line (e.g., 0-3)
//        _menuLineNr is the menu item to draw and process
void updateGrblFilesBtn ( void ) {  // fill 4 buttons for file name
                                // this function assumes that 
                                // grblFileNames[GRBLFILEMAX] =  n filename or directory name with max 40 char.
                                // grblFileIdx =  number of entries in grblFileName = index where next file would be written
                                // firstGrblFileToDisplay  =  index of first file to display in grblFileNames starting from 0  
    fillMPage (_P_SD_GRBL , 0 , _NO_BUTTON , _NO_ACTION , fSdGrblFilePrint , 0 ) ; // deactive all buttons
    fillMPage (_P_SD_GRBL , 1 , _NO_BUTTON , _NO_ACTION , fSdGrblFilePrint , 0 ) ; 
    fillMPage (_P_SD_GRBL , 2 , _NO_BUTTON , _NO_ACTION , fSdGrblFilePrint , 0 ) ; 
    fillMPage (_P_SD_GRBL , 3 , _NO_BUTTON , _NO_ACTION , fSdGrblFilePrint , 0 ) ; 
    uint8_t currentFileIdx =  firstGrblFileToDisplay ;
    uint8_t i = 0 ;
    //Serial.print("update file Btn; new first name="); Serial.println( grblFileNames[firstGrblFileToDisplay] ) ;
    //Serial.print("mbutton file 0=") ; Serial.println(mButton[_FILE0_GRBL].pLabel ) ;
    if (grblFileIdx ) { // fill buttons only if there are names in the array
        //Serial.print("will begin while cnt=") ; Serial.println(cnt) ; Serial.print(" ,sdFileDirCnt=") ; Serial.println(sdFileDirCnt) ; Serial.print(" ,first+4=") ; Serial.println(firstFileToDisplay + 4) ;
        while ( (currentFileIdx  <  grblFileIdx ) && ( i< 4)  ) {
          //Serial.print("in while currentFileIdx= ") ; Serial.println( currentFileIdx );
          strcpy(grblFileNamesTft[i] , grblFileNames[firstGrblFileToDisplay+i] ) ; // copy the file name in the field used when drawing the btn
          fillMPage (_P_SD_GRBL , i , _FILE0_GRBL + i , _JUST_PRESSED , fSdGrblFilePrint , i) ; // activate the button; param contains the index of the file (0,...3); _FILE0 contains a pointer to grblFileNames[]
          i++ ;
          currentFileIdx++ ;
        }
    }
    //updatePartPage = true ;
    //waitReleased = true ;       
}  

