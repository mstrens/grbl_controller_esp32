#ifndef log_h
#define log_h

#define MAX_BUFFER_SIZE 4000
#define BUFFER_EOL '#'

void logBufferInit() ;
void logBufferWrite(uint8_t c) ;
int16_t getPrevLogLine () ;
int16_t getNextLogLine () ;
void cpyLineToArray( char * dest , uint8_t * source , uint8_t nCar) ;

#endif
