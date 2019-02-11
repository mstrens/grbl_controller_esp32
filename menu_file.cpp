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
#include "draw.h"
#include "actions.h"

extern SdFat sd;
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
extern char fileNames[4][23] ; // 22 car per line + "\0"
extern char lastMsg[23] ;
extern uint16_t fileFocus ;
extern int16_t encoderTopLine ;

extern uint32_t sdFileSize ;
extern uint32_t sdNumberOfCharSent ;

extern boolean waitOk ;
//extern File * sdFiles[DIR_LEVEL_MAX] ; // normally max 6 dir level
extern int8_t dirLevel ;

//uint8_t sdStatus =  SD_STATUS_NO_CARD ;
//SD sd1; // use SPI 1 hardware (stm32f103 has 2 spi)

extern uint16_t firstFileToDisplay ;   // 0 = first file in the directory

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
    if ( dirLevel == -1) { // after an error or at startup, dirLevel = -1; this means that we have to close all files and we have to try again.
       closeAllFiles() ;
      p("begin sdstart"); 
      //if ( ! SD.begin(SD_CHIPSELECT_PIN) ) {
      if ( ! sd.begin(SD_CHIPSELECT_PIN , SD_SCK_MHZ(5)) ) {  
          memccpy( lastMsg , "Card Mount Failed" , '\0' , 22) ;
          return false;       
      }
      //if ( ! SD.exists( "/" ) ) { // check if root exist
      if ( ! sd.exists( "/" ) ) { // check if root exist   
          memccpy( lastMsg , "Root not found" , '\0' , 22) ;
          return false;  
      }
      if ( ! sd.chdir( "/" ) ) {
          memccpy( lastMsg , "chdir error" , '\0' , 22) ;
          return false;  
      }
      if ( ! aDir[0].open("/" ) ) { // open root 
          memccpy( lastMsg , "Failed to open Root" , '\0' , 22) ;
          return false;  
      }
      char nameTest[23] ;
      if ( ! aDir[0].getName( nameTest , 22) ) p("name of root not found") ;
      p(nameTest) ;
      dirLevel = 0 ;
      firstFileToDisplay = 0 ;
      p("end of sdStart to init" ) ;
      return true ;
    }
  // else if we already had some files opened; then we try to recover
    Serial.println("verify that workDir is still ok") ;
  if ( ! sd.exists( "/" ) ) { // check if root exist       // first check if root exists 
      memccpy( lastMsg , "Root not found" , '\0' , 22) ;
      dirLevel = -1; 
        return false;  
  }  
  if ( dirLevel == 0 && ( ! aDir[0].isDir() )  ) {
      memccpy( lastMsg , "first dir is not Root" , '\0' , 22) ;
      dirLevel = -1; 
        return false;  
  }
  if ( dirLevel > 0 && ( ! aDir[dirLevel].isDir()  ) ){
      memccpy( lastMsg , "current dir is not a sub dir" , '\0' , 22) ;
      dirLevel = -1; 
        return false;  
  }
  // here we assume that it is ok and so we can continue to use aDir[dirLevel], dirLevel and firstFileToDisplay
  // so, we wiil (try to) update the name of the button
  Serial.println("aDir[dirLevel] is still ok") ;
  return true ;
}

uint16_t fileCnt( void ) {
  // Open next file in volume working directory.  
  // Warning, openNext starts at the current position of sd.vwd() so a
  // rewind may be neccessary in your application.
  // return -1 in case of error
  uint16_t cnt = 0;
  
  SdBaseFile file ; 
  aDir[dirLevel].rewind();
  while ( file.openNext( &aDir[dirLevel] ) ) {
    cnt++ ;
 //   Serial.print("cnt= " ); Serial.print(cnt ); Serial.print(" , " ); Serial.println(file.name() ); 
    file.close();  
  }
  Serial.print("nbr of file in dir= " ); Serial.println(cnt );
  return cnt ;
}

boolean updateFilesBtn ( void ) {  // fill an array with max 4 files names and update the table used to display file name button ; 
                                // this function assumes that aDir[dirLevel] is defined and is the current dir
                                // it assumes also that sdFileDirCnt has been calculated and that
                                // firstFileToDisplay contains the index (starting at 0) of the first file name to display in the workDir
// Pour afficher l'écran SD, on utilise
//     un compteur du nbre de fichiers ( sdFileDirCnt )
//     un index du premier fichier affiché ( firstFileToDisplay )
//     il faut remplir les 4 premiers boutons (max) avec les noms des 4 fichiers à partir de l'index
//     S'il y a moins de 4 fichiers, on ne crée pas les dernier boutons
  Serial.print("Nbr de fichiers") ; Serial.println(sdFileDirCnt) ;
  Serial.print("firstFileToDisplay") ; Serial.println(firstFileToDisplay) ;
  //Serial.println(( ((int16_t) sdFileDirCnt) - 4)) ;
  if (  (firstFileToDisplay + 4) > sdFileDirCnt ) {
    if (  sdFileDirCnt <= 4 ) {
      firstFileToDisplay = 0 ;     // reset firstFileToDisplay 0 if less than 5 files
    } else {
      firstFileToDisplay = sdFileDirCnt - 4 ;  // keep always 4 files on screen
    }    
  }
  fillMPage (_P_SD , 0 , 0 , _JUST_PRESSED , fSdFilePrint , 0 ) ; // deactive all buttons
  fillMPage (_P_SD , 1 , 0 , _JUST_PRESSED , fSdFilePrint , 0 ) ; 
  fillMPage (_P_SD , 2 , 0 , _JUST_PRESSED , fSdFilePrint , 0 ) ; 
  fillMPage (_P_SD , 3 , 0 , _JUST_PRESSED , fSdFilePrint , 0 ) ; 
  aDir[dirLevel].rewind();
  uint16_t cnt = 0;
  SdBaseFile file ;
  
  while ( cnt < firstFileToDisplay ) {
    if ( ! file.openNext( &aDir[dirLevel] ) ) {       // ouvre le prochain fichier dans le répertoire courant ; en cas d'erreur, retour à la page info avec un message d'erreur 
      memccpy( lastMsg , "files missing" , '\0' , 22) ;
      file.close() ;
      return false ; 
    }
    cnt++ ;
    file.close() ;
  }
                                                       //ici on est prêt à chercher le nom du premier fichier à afficher
  uint8_t i = 0 ;
  char * pfileNames ;
  Serial.print("will begin while cnt=") ; Serial.println(cnt) ; Serial.print(" ,sdFileDirCnt=") ; Serial.println(sdFileDirCnt) ; Serial.print(" ,first+4=") ; Serial.println(firstFileToDisplay + 4) ;
  while ( (cnt < sdFileDirCnt) && (cnt < ( firstFileToDisplay + 4) ) ) {
    Serial.println("in while") ;
    if ( ! file.openNext( &aDir[dirLevel] ) ) {
      memccpy( lastMsg , "Failed to open a file" , '\0' , 22) ;
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
      memccpy( lastMsg , "No file name" , '\0' , 22) ;
      file.close() ;
      return false ;  
    }
    Serial.print("i= " ) ; Serial.print(i) ; Serial.print(" cnt= " ) ; Serial.println(cnt) ;    
    Serial.println( fileNames[i] );
    fillMPage (_P_SD , i , _FILE0 + i , _JUST_PRESSED , fSdFilePrint , i) ; // activate the button; param contains the index of the file (0,...3)
    i++ ;
    cnt++ ;
    file.close() ;
  }
  return true ;
}      

boolean startPrintFile( ) {                        // open the file given fy ffileToRead ; return false when error after filling a message, 
  sdFileSize = aDir[dirLevel+1].fileSize() ;
  sdNumberOfCharSent = 0 ;
  //Serial.print("file size=") ; Serial.println(sdFileSize) ;
  statusPrinting = PRINTING_FROM_SD ;
  waitOk = false ; // do not wait for OK before sending char.
  return true ;
} 

void closeFileToRead() {
  aDir[dirLevel+1].close() ;
}

boolean setFileToRead ( uint8_t fileIdx ) { // fileIdx is a number from 0...3 related to the button being pressed
  uint16_t cntIdx = fileIdx + firstFileToDisplay ; 
  uint16_t cnt = 0;
  aDir[dirLevel].rewind();
  aDir[dirLevel+1].close()  ;
  while ( cnt <= cntIdx ) {
//    Serial.print("cnt= ") ; Serial.print(cnt) ; Serial.print(" , Idx= ") ; Serial.println(cntIdx) ; 
//    Serial.print("workDir is dir= ") ; Serial.println(workDir.isDirectory() );
//    Serial.print("dirName=") ; Serial.println(workDir.name() );
      if (  ! aDir[dirLevel+1].openNext( &aDir[dirLevel] ) ) {       
        memccpy( lastMsg , "selected file missing" , '\0' , 22) ;
        aDir[dirLevel+1].close() ;
        dirLevel = -1 ;                         // in case of error, force a reset of SD card
        return false ; 
    }
    cnt++ ;
  }
  // here fileToread is filled
  sdFileSize = aDir[dirLevel+1].fileSize() ;
  sdNumberOfCharSent = 0 ;
  return true ;
}

boolean fileToReadIsDir( ) {
  return aDir[dirLevel+1].isDir() ;
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
    sdFileDirCnt = fileCnt() ;
    firstFileToDisplay = 0 ; // todo restore previous value for this upper dir
    return updateFilesBtn() ;
  } else
  */
    if (dirLevel < (DIR_LEVEL_MAX - 1) ) {  // Goes one level lower 
    Serial.println("goes one level dir down") ;
    dirLevel++ ;
    sdFileDirCnt = fileCnt() ;
    Serial.print("After file cnt =") ;Serial.println(sdFileDirCnt) ;
    firstFileToDisplay = 0 ; 
    return updateFilesBtn() ;
  }                       // else keep current directory and do not go one level more
  return false ;
}

boolean sdMoveUp() {
  if (dirLevel == 0) return false ; // if we are already on root, just discard
  Serial.println("goes one level dir up") ;
  aDir[dirLevel + 1].close() ;
  aDir[dirLevel].close() ;
  dirLevel-- ;
  sdFileDirCnt = fileCnt() ;
  Serial.print("After file cnt =") ;Serial.println(sdFileDirCnt) ;
    firstFileToDisplay = 0 ; 
    return updateFilesBtn() ;
}


void closeAllFiles() {
  uint8_t i = 0 ;
  for (i ; i < DIR_LEVEL_MAX ; i++ ) {
    aDir[i].close() ;
  }
  //memset(workDirParents , 0 , sizeof(workDirParents)) ;
  //workDir.close() ;
  //root.close() ;
  dirLevel = -1 ;
  firstFileToDisplay = 0 ; 
}

