#ifndef communication_h
#define communication_h

#include "Arduino.h"

#define STR_GRBL_BUF_MAX_SIZE 80         // size has been increased from 10 to 60 to support grbl [Msg:] Longest is [GC:....] message

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

#endif                                       

