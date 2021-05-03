#include "FS.h"
#include "SPIFFS.h"
#include "draw.h"
#include "setupTxt.h"
#include "config.h"
#include "language.h"

/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED true

File createdFile ;
File cmdFileToRead ;
extern char cmdName[11][17] ;           // store the name of the button.
extern uint8_t cmdIcons[11][1300] ;    // store the icons of the commands buttons (if any) 1300 = an icon of 100X100
extern boolean cmdIconExist[11];       // store a flag to say if an icon exist or not for a cmd



boolean spiffsInit() {
  return SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED) ;
}

boolean cmdNameInit() {  // seach on SPIFFS the names of the commands
  uint8_t i = 0 ;
  const char * pchar;
  uint16_t nbrIconChar = 740; // for 3.2", there must be 740 bytes in an icon file
  uint32_t readIdx ;
  uint8_t cmdIdx ;
  boolean validIcon = true; // store if the content of an icon file is valid
  char hex[3] = "__" ; // keep 2 bytes from icon file for conversion Hex to bin
  char * p;            // detect when conversion hex to bin is wrong
              
  #if defined(TFT_SIZE) && (TFT_SIZE == 4)
  nbrIconChar = 1300 ;  // for 3.2", there must be 1300 bytes in an icon file
  #endif
  for (i = 0 ; i< 11 ; i++ ){
    cmdName[i][0] = 0 ; // clear fill name   
  }

  File root = SPIFFS.open("/");
    if(!root){
        root.close() ;
        return false ;
    }
    File file = root.openNextFile();
    File fIcon;
    char fIconName[] = "/icon_.txt"  ; // Name is supposed to be iconX.txt; / is added because required by spiffs
    while(file){           // process only Cmdx_yyyyyy.zzzzz files
        pchar = file.name() ;
        if(pchar[1] == 'C' && pchar[2] == 'm' && pchar[3] == 'd' && (( pchar[4] >= '1' && pchar[4] <= '9') || pchar[4] == 'A' || pchar[4] == 'B' ) && pchar[5] == '_' ) {
            if ( pchar[4] <= '9' ) {
              cmdIdx = pchar[4] - '1' ;
            } else {
              cmdIdx = pchar[4] - 'A' + 9 ;
            }
            strncpy( &cmdName[cmdIdx][0] , &pchar[6] , 16) ; // save the name part (so skip the "CmdX_ part"
            cmdName[cmdIdx][16] = 0 ;              // for safety fill a 0 in last position because strncpy do not do it when source is to long
            //Serial.println("Handling a cmd file at init") ;
            fIconName[5] = pchar[4] ; // replace the "_" by the digit/letter in the Icon name to search
            //Serial.print("File name on SPIFFS ="); Serial.println(fIconName);
            fIcon.close();
            fIcon = SPIFFS.open(fIconName ) ; // try to open the file
            if ( fIcon) { // when icon file exist, upload the data
                  //Serial.println("Icon file will be read from SPIFFS") ; 
                  readIdx = 0 ;
                  while(fIcon.available() ){
                        while (  fIcon.available()  && ( fIcon.read() != 'x') ) { // read up to a 'x'
                          //Serial.print("skip when readIdx=") ;Serial.println(readIdx) ;
                        }; 
                        if ( fIcon.available()) {
                          hex[0]= fIcon.read() ;
                        } else {
                          validIcon = false ;
                        }
                        if ( fIcon.available()) {
                          hex[1]= fIcon.read() ; 
                        } else {
                          validIcon = false ;
                        }
                        //Serial.print("hex =") ;Serial.println(hex) ;
                        long value = strtoul (hex, &p,16); 
                        if ( * p != 0 ) {  
                            validIcon = false ;
                        } else {
                          if (readIdx < nbrIconChar) {
                            cmdIcons[cmdIdx][readIdx] = (char) value;
                          }
                          readIdx++;
                        }  
                  }
                  //Serial.print("readIdx ="); Serial.println(readIdx); delay(2000);
                  if ( readIdx != nbrIconChar ) validIcon = false ;
                  cmdIconExist[cmdIdx] = validIcon ; // flag to say that an icon exist and is valid 
                  //Serial.println("Icon has been read from SPIFFS") ;
            }
            fIcon.close() ;
        } // end if (fIcon)
        file = root.openNextFile();
    }
    
  file.close() ;
  root.close() ;
  //Serial.println("end of cmdnameinit");
  return true ;
}


void deleteFileLike (const char * path) {       // path does not start with "/" but is a format CmdX_name.yyyy)
                                                // we have to delete also files having a name "iconX.yyyyy (does not take care of yyyy
    const char * pchar; 
    File root = SPIFFS.open("/");
    if(!root){
        fillMsg(_SPIFFS_FAIL_TO_OPEN );
        root.close() ;
        return;
    }
    File file = root.openNextFile();
    while(file){
        pchar = file.name() ;      // filename from SPIFFS starts with "/" so it is different from path
        if(pchar[1] == 'C' && pchar[2] == 'm' && pchar[3] == 'd' && pchar[4] == path[3] && pchar[5] == '_' ) {
            SPIFFS.remove(pchar );          // delete file having a name that begins with Cmdx_ (with same x)
        }
        if (pchar[1] == 'i' && pchar[2] == 'c' && pchar[3] == 'o' && pchar[4] == 'n' && pchar[5] == path[3] && pchar[6] == '.' ) {
            SPIFFS.remove(pchar );          // delete file having a name that begins with iconx. (with same x)
        }
        file = root.openNextFile();
    }
    file.close() ;
    root.close() ;
}

boolean createFileCmd( const char * fileName){  // fileName does not begin with "/"
                                                // this function does not create the file to save the icon; it is done in another function
    char fileNamePlus[32] ;
    strcpy( fileNamePlus , "/") ; // copy "/" into fileNamePlus
    strncat (fileNamePlus , fileName , 30) ; // add max 30 char of fileName to fileNamePlus.
    createdFile.close() ;
    createdFile = SPIFFS.open( fileNamePlus, FILE_WRITE);
    if(!createdFile){
        fillMsg(_FAILED_TO_CREATE_CMD );
        createdFile.close() ;
        return false;
    }
    return true ;
}

boolean writeFileCmd( char sdChar) {      // write a char to an opened file
  return createdFile.print( sdChar) ; 
}

void closeFileCmd() {
  createdFile.close() ;    
}

boolean createFileIcon( const char * fileName){  // fileName does not begin with "/"                                           
    char fileNamePlus[32] ;
    strcpy( fileNamePlus , "/") ; // copy "/" into fileNamePlus
    strncat (fileNamePlus , fileName , 30) ; // add max 30 char of fileName to fileNamePlus.
    createdFile.close() ;
    createdFile = SPIFFS.open( fileNamePlus, FILE_WRITE);
    if(!createdFile){
        fillMsg(_FAILED_TO_CREATE_CMD );
        createdFile.close() ;
        return false;
    }
    return true ;
}

boolean writeFileIcon( char sdChar) {      // write a char to an opened file
  return createdFile.print( sdChar) ; 
}

void closeFileIcon() {
  createdFile.close() ;    
}



boolean spiffsOpenCmdFile( char * spiffsCmdName ) {      // open a cmd file to be send to GRBL
  cmdFileToRead.close() ;
  cmdFileToRead = SPIFFS.open(spiffsCmdName ) ;
  if (! cmdFileToRead ) {
    return false ;
  }
  return true ;
}

int spiffsAvailableCmdFile() {            //
  int cmdNumberChar =  cmdFileToRead.available() ;
  if ( cmdNumberChar == 0 ) cmdFileToRead.close() ;
  return  cmdNumberChar ;
}

char spiffsReadCmdFile() {              // return the code being read
    return cmdFileToRead.read() ;      
}
  
void listSpiffsDir(const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = SPIFFS.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listSpiffsDir(file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}


///////////////////////////////////////// functions here under are normally not used (copied from example)
void readFile(const char * path){
    Serial.printf("Reading file: %s\r\n", path);

    File file = SPIFFS.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return;
    }

    Serial.println("- read from file:");
    while(file.available()){
        Serial.write(file.read());
    }
}

void writeFile( const char * path, const char * message){
    Serial.printf("Writing file: %s\r\n", path);

    File file = SPIFFS.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- frite failed");
    }
}

void appendFile(const char * path, const char * message){
    Serial.printf("Appending to file: %s\r\n", path);

    File file = SPIFFS.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("- message appended");
    } else {
        Serial.println("- append failed");
    }
}

void renameFile(const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\r\n", path1, path2);
    if (SPIFFS.rename(path1, path2)) {
        Serial.println("- file renamed");
    } else {
        Serial.println("- rename failed");
    }
}

void deleteFile( const char * path){
    Serial.printf("Deleting file: %s\r\n", path);
    if(SPIFFS.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
}

void testFileIO( const char * path){
    Serial.printf("Testing file I/O with %s\r\n", path);

    static uint8_t buf[512];
    size_t len = 0;
    File file = SPIFFS.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }

    size_t i;
    Serial.print("- writing" );
    uint32_t start = millis();
    for(i=0; i<2048; i++){
        if ((i & 0x001F) == 0x001F){
          Serial.print(".");
        }
        file.write(buf, 512);
    }
    Serial.println("");
    uint32_t end = millis() - start;
    Serial.printf(" - %u bytes written in %u ms\r\n", 2048 * 512, end);
    file.close();

    file = SPIFFS.open(path);
    start = millis();
    end = start;
    i = 0;
    if(file && !file.isDirectory()){
        len = file.size();
        size_t flen = len;
        start = millis();
        Serial.print("- reading" );
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            if ((i++ & 0x001F) == 0x001F){
              Serial.print(".");
            }
            len -= toRead;
        }
        Serial.println("");
        end = millis() - start;
        Serial.printf("- %u bytes read in %u ms\r\n", flen, end);
        file.close();
    } else {
        Serial.println("- failed to open file for reading");
    }
}

void spiffsTest(){
    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    
    listSpiffsDir( "/", 0);
    writeFile( "/hello.txt", "Hello ");
    appendFile( "/hello.txt", "World!\r\n");
    readFile( "/hello.txt");
    renameFile( "/hello.txt", "/foo.txt");
    readFile( "/foo.txt");
    deleteFile( "/foo.txt");
    testFileIO( "/test.txt");
    deleteFile( "/test.txt");
    Serial.println( "Test complete" );
}


