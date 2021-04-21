#ifndef communication_h
#define communication_h

#include "Arduino.h"

#define STR_GRBL_BUF_MAX_SIZE 80         // size has been increased from 10 to 60 to support grbl [Msg:] Longest is [GC:....] message
#define GRBL_LINK_SERIAL 0
#define GRBL_LINK_BT 1
#define GRBL_LINK_TELNET 2

#define PARSING_FILE_NAMES_BLOCKED 0
#define PARSING_FILE_NAMES_RUNNING 1 
#define PARSING_FILE_NAMES_DONE   2


void getFromGrblAndForward2( void ) ;
void decodeGrblLine(char * line) ;
void parseMsgLine( char * line );
void parseFileLine(char * line );
void parseSatusLine(char * line);
void decodeFloat(char * pSection);
void parseErrorLine(const char * line);
void parseAlarmLine(const char * line);



void getFromGrblAndForward( void ) ;   //get char from GRBL, forward them is statusprinting = PRINTING_FROM_PC and decode the data (look for "OK", for <xxxxxx> sentence
                                       // fill machineStatus[] and mPosXYZ[]

void parseToLog(uint8_t c , uint8_t lastC) ;   // do not store in log the OK and the status message.
void handleLastNumericField(void) ;
void storeGrblState(void) ;

void sendToGrbl( void ) ;
void sendFromSd(void) ;
void sendFromCmd() ;
void sendFromString() ;
void sendJogCancelAndJog(void) ; 
boolean sendJogCmd(uint32_t startTime ) ; // parameter is the original time (millis) when we start a ; return true if cmd has been sent
void resetWaitOkWhenSdMillis() ;

void fillErrorMsg( const char * errorMsg ) ; 
void fillAlarmMsg( const char * alarmMsg ) ;
void fillStringExecuteMsg( uint8_t buttonMessageIdx ) ; 
void toGrbl(char c);
void toGrbl(const char * data);
void bufferise2Grbl(const char * data , char beginEnd = 'c');
int fromGrblAvailable();
int fromGrblRead();
void btInit() ;
void startGrblCom(uint8_t comMode);
#endif                                       

