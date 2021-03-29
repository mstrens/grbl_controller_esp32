#ifndef grbl_file_h
#define grbl_file_h

#include "Arduino.h"

#define GRBL_FIRST_READ 0 // code says why we are reading the filenames from grbl; it is handle in the callback function
void grblReadFiles( uint8_t reason); // request grbl to provide the list of file.
void updateGrblFilesBtn() ; // update the button for the files being displayed

/*
void fillFileMenu ( void ) ;
boolean sdStart( void ) ;  // close all find, check sd card and open currentDir on root.
uint16_t fileCnt( uint8_t level ) ; // count number of files in directory specified by level
boolean updateFilesBtn() ;    // update the buttons to be displayed; return false in case of error
boolean startPrintFile( ) ;   // open the file to be read, fileIdx gives the index of the fileName; return false if file can't be opened and fill a lastMessage
void closeFileToRead(void ) ; // close fileToRead (file used to send the data)
boolean changeDirectory() ;      // change the directory when file selected is a directory
boolean fileToReadIsDir() ;
void closeAllFiles() ;
boolean setFileToRead ( uint8_t fileIdx ) ;
boolean sdMoveUp() ;
boolean fileIsCmd() ;           // check if the file is a cmd and if so, copy it into the SPIFFS, return true if it is cmd file
boolean createFile(char * fileName) ;
void setShowBuffer() ; // fill the show buffer and some variables when we enter the sd show screen
void setNextShowBuffer() ; // this is call when we press the next btn on sd show
void setPrevShowBuffer() ; // this is call when we press the prev btn on sd show
*/

#endif

