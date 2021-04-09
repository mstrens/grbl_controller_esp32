
#include "menu_file.h"

// Cette fonction affiche max N (=MAW_LCD_ROWS) noms de fichiers sélectionnés dans le directory courrant à partir du Xème fichier
// 
// utilise encoderTopLine = menu index du premier élément sur l'écran
//         encoderPosition = valeur de l'encodeur = index du fichier à afficher si possible
//    _lcdLineNr is the index of the LCD line (e.g., 0-3)
//   _menuLineNr is the menu item to draw and process

#include "Arduino.h"
//#include "SD.h"
#include "SdFat.h"
#include "config.h"
#include "language.h"
#include "draw.h"
#include "setupTxt.h"
#include "actions.h"
#include "cmd.h"


extern SdFat32 sd;
//extern File root ;
//extern File workDir ;
//extern File workDirParents[DIR_LEVEL_MAX] ;
//extern File aDir[DIR_LEVEL_MAX] ;
//extern File fileToRead ;
extern SdBaseFile aDir[DIR_LEVEL_MAX] ;
//extern SdBaseFile fileToRead ;

extern M_Button mButton[_MAX_BTN] ;
extern uint8_t statusPrinting ;
extern uint8_t prevPage , currentPage ;

extern uint16_t sdFileDirCnt ;
uint16_t sdFileRootCnt ; // count the nbr of files in root; used to check if SD card has been changed
extern char fileNames[4][23] ; // 22 car per line + "\0"
//extern char lastMsg[80] ;
extern uint16_t fileFocus ;
extern int16_t encoderTopLine ;

extern uint32_t sdFileSize ;
extern uint32_t sdNumberOfCharSent ;

extern volatile boolean waitOk ;
//extern File * sdFiles[DIR_LEVEL_MAX] ; // normally max 6 dir level
extern int8_t dirLevel ;

//uint8_t sdStatus =  SD_STATUS_NO_CARD ;
//SD sd1; // use SPI 1 hardware (stm32f103 has 2 spi)

extern uint16_t firstFileToDisplay ;   // 0 = first file in the directory

// data used to show sd file content
File32 fileToShow ;   // this contains the file being sent from SD
extern char sdShowBuffer[1000] ; // buffer containing the data to display
extern int16_t sdShowFirst  ;     // index in buffer of first char to display in the buffer
extern int16_t sdShowMax ;      // index in buffer of the first char that has not been written in the buffer = Number of char written in sdShowBuffer
uint32_t sdShowBeginPos  ;      // position in the SD file of the first char stored in the sdShowBuffer array   
uint32_t sdMillPos  ;           // position in the SD file of the next char to be sent to GRBL   
extern uint32_t sdShowNextPos ; // position in the SD file of the next char to be displayed if we press on Next btn

// sdInit()  // close all files, end of sd, set dirFiles to Null, set dirLevel et firstFileToDisplay = 0, execute sd begin; try to read sd, in case of error fill last message
// when entering SD menu
void p( char * text) {
  Serial.println(text);
}

// look at sd card
boolean sdStart( void ) {  // this function is called when we enter the sd screen or when we navigate between files with left & right buttons
                           // if dirLevel is negative (sd card not yet read or error), it tries to open it and goes on root
                           // else, it try to check if sd card is still there and if OK, it just goes out
                           // ferme les fichiers ouverts, essaie de rouvrir et de se positionner sur le root, return false in case of issue
                           // keep currentDir open (so it can be used by other steps)
    if (dirLevel != -1) {
        if ( ! sd.exists( "/" ) ) { // check if root exist       // first check if root exists 
            fillMsg(_ROOT_NOT_FOUND ) ;
            dirLevel = -1; 
            return false;  
        }  
        if ( dirLevel == 0 && ( ! aDir[0].isDir() )  ) {
            fillMsg(_FIRST_DIR_IS_NOT_ROOT ) ;
            dirLevel = -1; 
            return false;  
        }
        if ( dirLevel > 0 && ( ! aDir[dirLevel].isDir()  ) ){
            fillMsg( _CURRENT_DIR_IS_NOT_A_SUB_DIR  ) ;
            dirLevel = -1; 
            return false;  
        }
        if ( sdFileRootCnt != fileCnt(0)) {  // if the number of files in root changed, the SD card has been perhaps removed and we go back to Root
            dirLevel = -1;  // force a close all and retry
            //Serial.println("nbr of files in root has changed") ;
        }
    }
    
    if ( dirLevel == -1) { // after an error or at startup, dirLevel = -1; this means that we have to close all files and we have to try again.
       closeAllFiles() ;
      //p("begin sdstart"); 
      //if ( ! SD.begin(SD_CHIPSELECT_PIN) ) {
      if ( ! sd.begin(SD_CHIPSELECT_PIN , SD_SCK_MHZ(5)) ) {  
          fillMsg(_CARD_MOUNT_FAILED  ) ;
          return false;       
      }
      //if ( ! SD.exists( "/" ) ) { // check if root exist
      if ( ! sd.exists( "/" ) ) { // check if root exist   
          fillMsg(_ROOT_NOT_FOUND  ) ;
          return false;  
      }
      if ( ! sd.chdir( "/" ) ) {
          fillMsg(_CHDIR_ERROR  ) ;
          return false;  
      }
      if ( ! aDir[0].open("/" ) ) { // open root 
          fillMsg(_FAILED_TO_OPEN_ROOT ) ;
          return false;  
      }
      //char nameTest[23] ;
      //if ( ! aDir[0].getName( nameTest , 22) ) p("name of root not found") ;
      //p(nameTest) ;
      dirLevel = 0 ;
      firstFileToDisplay = 0 ;
      sdFileRootCnt = fileCnt(dirLevel); // store the number of files in Root directory
      //p("end of sdStart to init" ) ;
      return true ;
    }
  // else if we already had some files opened; then we try to recover
    //Serial.println("verify that workDir is still ok") ;
  
  // here we assume that it is ok and so we can continue to use aDir[dirLevel], dirLevel and firstFileToDisplay
  // so, we wil (try to) update the name of the button
  return true ;
}

uint16_t fileCnt( uint8_t level ) {
  // Open next file in volume directory specified by level.  
  // Warning, openNext starts at the current position of sd.vwd() so a
  // rewind may be neccessary in your application.
  uint16_t cnt = 0;
  File32 file ; 
  aDir[level].rewind()  ;
  while ( file.openNext( &aDir[level] ) ) {
    cnt++ ;
 //   Serial.print("cnt= " ); Serial.print(cnt ); Serial.print(" , " ); Serial.println(file.name() ); 
    file.close();  
  }
  file.close();
  return cnt ;
}

boolean updateFilesBtn ( void ) {  // fill an array with max 4 files names and update the table used to display file name button ; 
                                // this function assumes that aDir[dirLevel] is defined and is the current dir
                                // it assumes also that sdFileDirCnt has been calculated and that
                                // firstFileToDisplay contains the index (starting at 1) of the first file name to display in the workDir
// Pour afficher l'écran SD, on utilise
//     un compteur du nbre de fichiers ( sdFileDirCnt )
//     un index du premier fichier affiché ( firstFileToDisplay  (1 = premier fichier; 0 = pas de fichier) )
//     il faut remplir les 4 premiers boutons (max) avec les noms des 4 fichiers à partir de l'index
//     S'il y a moins de 4 fichiers, on ne crée pas les dernier boutons
  
  fillMPage (_P_SD , 0 , _NO_BUTTON , _NO_ACTION , fSdFilePrint , 0 ) ; // deactive all buttons
  fillMPage (_P_SD , 1, _NO_BUTTON , _NO_ACTION , fSdFilePrint , 0 ) ; 
  fillMPage (_P_SD , 2 , _NO_BUTTON , _NO_ACTION , fSdFilePrint , 0 ) ; 
  fillMPage (_P_SD , 3 , _NO_BUTTON , _NO_ACTION , fSdFilePrint , 0 ) ; 
  aDir[dirLevel].rewind();
  uint16_t cnt = 1 ;
  File32 file ;
  
  while ( ( cnt ) < firstFileToDisplay ) {
    if ( ! file.openNext( &aDir[dirLevel] ) ) {       // ouvre le prochain fichier dans le répertoire courant ; en cas d'erreur, retour à la page info avec un message d'erreur 
      fillMsg(_FILES_MISSING  ) ;
      file.close() ;
      return false ; 
    }
    cnt++ ;
    file.close() ;
  }
                                                       //ici on est prêt à chercher le nom du premier fichier à afficher
  uint8_t i = 0 ;
  char * pfileNames ;
  //Serial.print("will begin while cnt=") ; Serial.println(cnt) ; Serial.print(" ,sdFileDirCnt=") ; Serial.println(sdFileDirCnt) ; Serial.print(" ,first+4=") ; Serial.println(firstFileToDisplay + 4) ;
  while ( (cnt <=  sdFileDirCnt ) && (cnt < ( firstFileToDisplay + 4) ) ) {
    //Serial.println("in while") ;
    if ( ! file.openNext( &aDir[dirLevel] ) ) {
      fillMsg(_FAILED_TO_OPEN_A_FILE ) ;
      file.close() ;
      return false ;  
    }
//    const char * pFileName = file.name() ;       // keep the pointer to the name
    pfileNames = fileNames[i] ;                    // put a "/" as first char when it is a directory; this will be used when button is draw to reverse the colors
    if ( file.isDir() ) { 
      *pfileNames = '/' ;
      pfileNames++ ;  
    }
    if ( ! file.getName( pfileNames , 21 ) ) {   // Rempli fileNames avec le nom du fichier
      fillMsg(_NO_FILE_NAME  ) ;
      file.close() ;
      return false ;  
    }
    //Serial.print("i= " ) ; Serial.print(i) ; Serial.print(" cnt= " ) ; Serial.println(cnt) ;    
    //Serial.println( fileNames[i] );
    fillMPage (_P_SD , i , _FILE0 + i , _JUST_PRESSED , fSdFilePrint , i) ; // activate the button; param contains the index of the file (0,...3)
    i++ ;
    cnt++ ;
    file.close() ;
  }
  return true ;
}      

boolean startPrintFile( ) {                         
  sdFileSize = aDir[dirLevel+1].fileSize() ;
  sdNumberOfCharSent = 0 ;
  statusPrinting = PRINTING_FROM_SD ;
  waitOk = false ; // do not wait for OK before sending char.
  return true ;
} 

void closeFileToRead() {
  aDir[dirLevel+1].close() ;
}

boolean setFileToRead ( uint8_t fileIdx ) { // fileIdx is a number from 0...3 related to the button being pressed; return false when error after filling a message,
                                            // firstFileToDisplay starts from 1 (or is 0 when there is no file)
                                            // if OK, return is true and aDir[dirLevel+1] point to the file and sdFileSize = size of file
  char fileToShowName[50] ;             // save the name of the file being opened
  uint16_t cntIdx =  fileIdx + firstFileToDisplay ; 
  uint16_t cnt = 0;
  aDir[dirLevel].rewind();
  //aDir[dirLevel+1].close()  ;
//  Serial.print("firstFileToDisplay= ") ; Serial.println(firstFileToDisplay) ;
//  Serial.print("fileIdx= ") ; Serial.println(fileIdx) ;
//  Serial.print(" , cntIdx= ") ; Serial.println(cntIdx) ; 
//  Serial.print("diLevel is dir= ") ; Serial.println(aDir[dirLevel].isDir() );
//  char dirName[32] = "error" ;
//  boolean dirNameOk ;
//  dirNameOk = aDir[dirLevel].getName(dirName, 31) ;
//  Serial.print("dir OK=") ; Serial.println(dirNameOk);
//  Serial.print("dirName=") ; Serial.println(dirName );
  while ( cnt < cntIdx ) {
//      Serial.print("cnt= ") ; Serial.println(cnt) ; 
      aDir[dirLevel+1].close() ;
      if (  ! aDir[dirLevel+1].openNext( &aDir[dirLevel] ) ) {       
        fillMsg(_SELECTED_FILE_MISSING  ) ;
        aDir[dirLevel+1].close() ;
        dirLevel = -1 ;                         // in case of error, force a reset of SD card
        return false ; 
      }
      cnt++ ;
                                               // keep aDir[dirLevel+1] open when found
  }
//  dirNameOk = aDir[dirLevel+1].getName(dirName, 31) ;
//  Serial.print("file OK=") ; Serial.println(dirNameOk);
//  Serial.print("fileName=") ; Serial.println(dirName );
  
  // here fileToread is filled
  sdFileSize = aDir[dirLevel+1].fileSize() ;
  sdNumberOfCharSent = 0 ;
  aDir[dirLevel+1].getName ( fileToShowName , 49 ) ; // get the name of current file 
  fileToShow.close() ;
  fileToShow.open( &aDir[dirLevel] , fileToShowName, O_READ ) ; // open the current file for browsing
  char fileToShowLine[100] ;
  fileToShow.fgets( fileToShowLine , 99 ) ; // get the first line
  //Serial.print("first line is ") ; Serial.println( fileToShowLine ); // to debug
  return true ;
}

boolean fileToReadIsDir( ) {
  return aDir[dirLevel+1].isDir() ;
}

boolean fileIsCmd() {           // check if the file in aDir[dirLevel+1] is a cmd and if so, copy it into the SPIFFS, return true if it is cmd file
                                // command file names look like Cmd5_name.xxx where
                                //             Cmd and _ are fixed
                                //             5 is the digit of the Cmd (must be between 1 and 9, A or B)
                                //             name is the name given to the button (must be less than 16 char and begin with a letter a...z or A...Z)
                                //             xxx is the extension and is discarded
  char fileName[32] ;
  char * pchar ;
  int sdChar = 0 ;
  if (! aDir[dirLevel+1].getName(fileName , 31) ) {   // fill fileName
    fillMsg(_FILE_NAME_NOT_FOUND  ); 
    return false ;
  }
  if ( ! ( strlen(fileName) >= 6 && fileName[0] == 'C' && fileName[1] == 'm' && fileName[2] == 'd' &&  
            ( ( fileName[3] >= '1' && fileName[3] <= '9' ) || fileName[3] == 'A' || fileName[3] == 'B' ) &&  
            fileName[4] == '_' && isalpha(fileName[5]) ) ){
  //  if ( strlen(fileName) < 6 || fileName[0] != 'C' || fileName[1] != 'm' || fileName[2] != 'd' || fileName[3] < '1' || fileName[3] > '7' || fileName[4] != '_' || (!isalpha(fileName[5]) )  ){
    
    fillStringMsg( fileName , BUTTON_TEXT );
    return false ;
  }
  // here we assume that the file name identifies a command; so we can store it on SPIFFS
  // first we remove the extension
  pchar = strchr(fileName , '.' );   // replace the first '.' by 0 (= skip the file extension)
  if ( pchar ) {  // if pchar is not NULL
    *pchar = 0 ;
  }
  deleteFileLike( fileName ) ;    // then look in SPIFFS for files beginning by Cmdx_ and if found, delete them
  if ( strlen(fileName) == 11 && fileName[5] == 'd' && fileName[6] == 'e' && fileName[7] == 'l' && fileName[8] == 'e' && fileName[9] == 't' && fileName[10] == 'e' ){
      fillMsg(_CMD_DELETED , SCREEN_NORMAL_TEXT ) ;
      aDir[dirLevel+1].close() ; // close the file from SD card
      return true ;
  }  
  if ( ! createFileCmd(fileName ) ) {      // then create the new file name
       fillMsg(_CMD_NOT_CREATED ) ;
       aDir[dirLevel+1].close() ; // close the file from SD card
       return true ;
  }
  fillMsg(_CMD_CREATED , SCREEN_NORMAL_TEXT ) ;
  while ( aDir[dirLevel+1].available() > 0 ) {
      sdChar = aDir[dirLevel+1].read() ;
      if ( sdChar < 0 ) {
        fillMsg(_CMD_PART_NOT_READ ) ;
        break ;
      } else if ( ! writeFileCmd( (char) sdChar) ) {  // write the char in SPIFFS; on error, break
        fillMsg(_CMD_COULD_NOT_SAVE ) ;
        break ; 
      }
  } // end while
  aDir[dirLevel+1].close() ; // close the file from SD card
  closeFileCmd() ;
  return true ;  
}


boolean changeDirectory() {      // change the directory when selected file is a directory; aDir[dirLevel+1] is the directory
  /*char dirName[23] ;
  const char * pFileName = fileToRead.name() ;
  //fileToRead.getName( dirName , 22) ;
  Serial.println("begin change directory" ); delay(1000);
  if ( ( (*pFileName) == '.' ) && ( (* (pFileName +1)) == '.' ) ){ // if dir name is '..' then goes one level up
    Serial.println("dir is ..") ;
    fileToRead.close() ; // close current directory
    if (dirLevel > 0 ) dirLevel-- ;
    //workDir = workDirParents[dirLevel] ;
    sdFileDirCnt = fileCnt(dirLevel) ;
    firstFileToDisplay = 0 ; // todo restore previous value for this upper dir
    return updateFilesBtn() ;
  } else
  */
    if (dirLevel < (DIR_LEVEL_MAX - 1) ) {  // Goes one level lower 
    //Serial.println("goes one level dir down") ;
    dirLevel++ ;
    sdFileDirCnt = fileCnt(dirLevel) ;
    //Serial.print("After file cnt =") ;Serial.println(sdFileDirCnt) ;
    firstFileToDisplay = 0 ; 
    return updateFilesBtn() ;
  }                       // else keep current directory and do not go one level more
  return false ;
}

boolean sdMoveUp() {
  if (dirLevel == 0) return false ; // if we are already on root, just discard
  //Serial.println("goes one level dir up") ;
  aDir[dirLevel + 1].close() ;
  aDir[dirLevel].close() ;
  dirLevel-- ;
  sdFileDirCnt = fileCnt(dirLevel) ;
  //Serial.print("After file cnt =") ;Serial.println(sdFileDirCnt) ;
    firstFileToDisplay = 0 ; 
    return updateFilesBtn() ;
}


void closeAllFiles() {
  for (uint8_t i = 0; i < DIR_LEVEL_MAX ; i++ ) {
    aDir[i].close() ;
  }
  //memset(workDirParents , 0 , sizeof(workDirParents)) ;
  //workDir.close() ;
  //root.close() ;
  dirLevel = -1 ;
  firstFileToDisplay = 0 ; 
}

void setShowBuffer() { // this is call when we enter call sd show
  sdMillPos = aDir[dirLevel+1].curPosition() ;     // get the current position in the file being sent
  sdShowBeginPos = sdMillPos ; 
  fileToShow.seek( sdMillPos ) ; // set the same position for the file used for display
  //char delim[2] = {0xFF , 0x00}  ;         // use a dummy delimiter in order to read up to the end
  //sdShowMax = fileToShow.fgets( sdShowBuffer , 999 , delim ) ; // -1 means an error, 0= EOF immediately
  sdShowMax = fileToShow.read( sdShowBuffer , 999 ) ; // -1 means an error, 0= EOF immediately
  //Serial.print("sdMillPos=") ; Serial.println(sdMillPos ) ; // to debug
  //Serial.print("sdShowMax=") ; Serial.println(sdShowMax ) ; // to debug
  sdShowFirst = 0 ;     // first char to display in the buffer
}

void setSdShowFirstForPrev(int16_t sdShowSearch) {
  uint8_t lines = 0 ;  // cumulative number of lines to displayed
  int16_t sdShowCurrent  = sdShowSearch ;   // first char to display (during the loop)
  int16_t nChar ; // number of char in buffer for the current line
  int16_t nCharInFile ; //number of char on one line in the SD file
  //Serial.print("begin to search from =") ; Serial.println(sdShowSearch ) ; // to debug
  if ( sdShowCurrent == 0) {   // if we search already from the begining then we will display starting from begining of buffer
    sdShowFirst = 0 ;
    return; 
  }
  do{ 
    nChar = prevBufferLineLength( sdShowCurrent , nCharInFile ) ; // Recherche le début de la ligne précédente et détermine le nbr de char de la ligne à afficher (sans le \n)
    if ( nChar < 0 ) {           // it means that we reached the begining of buffer
      sdShowFirst = 0 ; 
      return ;
    }
    //Serial.print("prev line found; searched from sdShowCurrent=") ; Serial.println(sdShowCurrent) ; // to debug
    //Serial.print("nChar of prev line =") ; Serial.println(nChar ) ; // to debug
    //Serial.print("nChar in file  =") ; Serial.println(nCharInFile ) ; // to debug
    sdShowCurrent -= nCharInFile ;              // move the cursor back
    //Serial.print("new sdShowCurrent =") ; Serial.println(sdShowCurrent ) ; // to debug
    lines += convertNCharToNLines ( nChar ) ;   // cumule le nbr de lignes du TFT
    //Serial.print("lines =") ; Serial.println( lines ) ; // to debug
  }  while ( lines <= 11) ;    // end do
  sdShowFirst = sdShowCurrent ;             // we can display from the current position 
}

void setPrevShowBuffer() { // this is call when we press the prev btn on sd show
  uint32_t actualFirstCharPos = sdShowBeginPos + sdShowFirst ;
  int16_t sdShowSearch ;
  if ( actualFirstCharPos > 999 ) {
    sdShowBeginPos = actualFirstCharPos - 999 ; // first char in new buffer has to be 999 before the pos of first having been displayed
  } else { 
    sdShowBeginPos = 0 ; // first char in new buffer has to be the first in the file
  }
  //Serial.print("new sdShowBeginPos=") ; Serial.println(sdShowBeginPos ) ; // to debug
  fileToShow.seek( sdShowBeginPos ) ; // set this position for the file used for display
  sdShowMax = fileToShow.read( sdShowBuffer , 999 ) ; // -1 means an error, 0= EOF immediately
  sdShowSearch = actualFirstCharPos - sdShowBeginPos ;    // we will search backward from this counter in 
  setSdShowFirstForPrev(sdShowSearch) ;      // set the new value of the idx of first char to display in the buffer starting looking from sdShowSearch
  //Serial.print("sdShowMax=") ; Serial.println(sdShowMax ) ; // to debug
  //Serial.print("SdShowFirst =") ; Serial.println(sdShowFirst ) ; // to debug
}

void setNextShowBuffer() { // this is call when we press the next btn on sd show
  //sdMillPos = aDir[dirLevel+1].curPosition() ;     // get the current position in the file being sent
  sdShowBeginPos = sdShowNextPos ; // first char in new buffer has to be the last char displayed  
  fileToShow.seek( sdShowBeginPos ) ; // set this position for the file used for display
  //char delim[2] = {0xFF , 0x00} ;         // use a dummy delimiter in order to read up to the end
  //sdShowMax = fileToShow.fgets( sdShowBuffer , 999  , delim ) ; // -1 means an error, 0= EOF immediately
  sdShowMax = fileToShow.read( sdShowBuffer , 999 ) ; // -1 means an error, 0= EOF immediately
  //Serial.print("sdMillPos=") ; Serial.println(sdMillPos ) ; // to debug
  //Serial.print("sdShowMax=") ; Serial.println(sdShowMax ) ; // to debug
  sdShowFirst = 0 ;     // the first char in display is the first to display in the buffer
}


