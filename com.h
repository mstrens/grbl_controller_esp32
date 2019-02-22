#ifndef communication_h
#define communication_h

#include "Arduino.h"


void getFromGrblAndForward( void ) ;   //get char from GRBL, forward them is statusprinting = PRINTING_FROM_PC and decode the data (look for "OK", for <xxxxxx> sentence
                                       // fill machineStatus[] and mPosXYZ[]
void handleLastNumericField(void) ;

void sendToGrbl( void ) ;
//int8_t waitForOK() ;
void sendJogCmd(void ) ;
void fillErrorMsg( char * errorMsg ) ; 
void fillAlarmMsg( char * alarmMsg ) ;

#endif                                       

