#ifndef communication_h
#define communication_h

#include "Arduino.h"

#define STR_GRBL_BUF_MAX_SIZE 50         // size has been increased from 10 to 50 to support grbl [Msg:] 

void getFromGrblAndForward( void ) ;   //get char from GRBL, forward them is statusprinting = PRINTING_FROM_PC and decode the data (look for "OK", for <xxxxxx> sentence
                                       // fill machineStatus[] and mPosXYZ[]
void handleLastNumericField(void) ;

void sendToGrbl( void ) ;
//int8_t waitForOK() ;
boolean sendJogCmd(uint32_t startTime ) ; // parameter is the original time (millis) when we start a ; return true if cmd has been sent
void fillErrorMsg( char * errorMsg ) ; 
void fillAlarmMsg( char * alarmMsg ) ;

#endif                                       

