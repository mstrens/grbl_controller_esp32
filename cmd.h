#ifndef cmd_h
#define cmd_h

void listSpiffsDir(const char * dirname, uint8_t levels) ;
//void readFile(const char * path) ;
//void writeFile(const char * path, const char * message) ;
//void appendFile(const char * path, const char * message) ;
//void renameFile(const char * path1, const char * path2) ;
//void deleteFile(const char * path) ;
//void testFileIO(const char * path) ;
void spiffsTest() ;
void deleteFileLike (const char * path) ;
boolean createFileCmd( const char * fileName) ;
boolean writeFileCmd( char ) ;      // write a char to an opened file
void closeFileCmd() ;
boolean spiffsInit() ;
boolean cmdNameInit() ; // true when error reading the cmd on SPIFFS
boolean spiffsOpenCmdFile( char * spiffsCmdName ) ;
int spiffsAvailableCmdFile() ;
char spiffsReadCmdFile() ;
#endif
