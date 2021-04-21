#include <stdint.h>
#include "log.h"
#include "config.h"
 

uint8_t logBuffer[MAX_BUFFER_SIZE] ;
uint8_t * pNext ; // position of the next char to be written
uint8_t * pFirst ; // position of the first char
uint8_t * pGet ; // position of the last char to be read for display

void logBufferInit() {  // all pointers set to the beginning of the buffer
  pNext = logBuffer ;
  pFirst = logBuffer ;
  pGet = logBuffer; 
  logBufferWrite(BUFFER_EOL) ; // fill once EOL, so when we search back the first line we find it.
}

void logBufferWrite(uint8_t c) {
  if ( c == '\n' ) return ; 
  //Serial.print("save ");Serial.print( (char) c , HEX) ; Serial.print(" , "); Serial.println((char) c);
  //if ( c == BUFFER_EOL ) {  // to debug
  //  Serial.println( " " );
  //} else {
  //  Serial.print((char) c); 
  //}
  *pNext = c ; // save the car
  pNext++ ;
  if ( ( pNext - logBuffer) >=  MAX_BUFFER_SIZE ) pNext = logBuffer ;
  if ( pNext == pFirst) { // If new next reach first, then increase first
    pFirst++;
    if ( ( pFirst - logBuffer) >=  MAX_BUFFER_SIZE ) pFirst = logBuffer ; 
  }
  if ( pNext == pGet) { // If new next reach pGet, then increase pGet
    pGet++;
    if ( ( pGet - logBuffer) >=  MAX_BUFFER_SIZE ) pGet = logBuffer ; 
  }
}

void logBufferWriteLine(char * line){
  int lineLen = strlen(line);
  int i = 0;
  for ( i ; i < lineLen ; i++ ) {
    logBufferWrite( line[i] ) ;    
  }
  logBufferWrite( BUFFER_EOL ) ; // special car to identify the end of the line
}

int16_t getPrevLogLine () {  // get previous line if any
                                // we first search an EOL in order to only display full lines
                                // we then search for another previous EOL and count the number of char
                                // pGet is updated only if we find one line (begining and ending with EOL, pGet points on the first char (not the first EOL) 
  uint8_t * pBegin ;
  uint8_t * pEnd ;
  int16_t nCar = 0 ;
  
  pEnd = pGet ;
  while ( ( pEnd != pFirst) && (*pEnd != BUFFER_EOL) ) {
    if (pEnd == logBuffer) pEnd = logBuffer + MAX_BUFFER_SIZE  ;
    pEnd--;
  }  
  //Serial.print("pEnd after first search ") ; Serial.println( pEnd - logBuffer) ; 
  if ( pEnd == pFirst) return -1 ; // no string has been found
  pBegin = pEnd ;
  if (pBegin == logBuffer) pBegin = logBuffer + MAX_BUFFER_SIZE ;
  pBegin--;
  while ( ( pBegin != pFirst) && (*pBegin != BUFFER_EOL) ) {
    if (pBegin == logBuffer) pBegin = logBuffer + MAX_BUFFER_SIZE ;
    pBegin--;
    nCar++ ;
  }
  if (*pBegin != BUFFER_EOL)  return -1 ; // no complete string has been found
  if ( nCar ) {
    pBegin++;
    if ( ( pBegin - logBuffer) >=  MAX_BUFFER_SIZE )  pBegin = logBuffer ;
  }
  pGet = pBegin ;
  //Serial.print("pBegin ") ; Serial.print( pBegin - logBuffer) ; Serial.print(" First char= ") ; Serial.println( (char) *pGet) ;
  //Serial.print("ncar=") ;  Serial.println( nCar ) ; 
  return nCar ;  
}

int16_t getNextLogLine () {  // Get next line (change pGet) 
                              // return -1 if next line is not complete
                                // first search for a next EOL (or an EOL just on pGet); this is end of current line
                                // then  move one char ahead and search for a next EOL
                                // if a next EOL is found, then return the number of Char (can be 0 when there is 2 consecutive EOL)  
                                // 
  uint8_t * pBegin ;
  uint8_t * pEnd ;
  int16_t nChar = 0 ;
  pBegin = pGet ;
  while ( ( pBegin != pNext) && (*pBegin != BUFFER_EOL) ) {   // look for next EOL (or current)
    pBegin++;
    if ( (pBegin -logBuffer) >= MAX_BUFFER_SIZE) pBegin = logBuffer ;
  }
  //Serial.print("pBegin after first search ") ; Serial.println( pBegin - logBuffer) ;   
  if ( pBegin == pNext ) return -1 ; // we can not increase pEnd because we reached pNext
  pBegin++;
  if ( (pBegin -logBuffer) >= MAX_BUFFER_SIZE) pBegin = logBuffer ;
  
  //if ( pBegin == pNext ) return -1 ; // we can not increase pEnd because we reached pNext
  pEnd = pBegin ; 
  while ( ( pEnd != pNext) && (*pEnd != BUFFER_EOL) ) { // Look for next EOL (or current)
    pEnd++;
    if ( (pEnd -logBuffer) >= MAX_BUFFER_SIZE ) pEnd = logBuffer ;
    nChar++ ;
  }
  if ( pEnd == pNext) {
    //Serial.println("No EOL before pNext, pGet point to last EOL +1" ); // to debug
    pGet = pBegin ; // Set Pget on first char after EOL
    return -1 ; // if we did not got a next EOL before the end
     
  }
  pGet = pBegin ;
  //Serial.print("new pget ") ; Serial.println( pGet - logBuffer) ; // to debug  
  return nChar ;  
}

void cpyLineToArray( char * dest , uint8_t * source , uint8_t nCar) {
  char * _dest = dest;
  uint8_t * _source = source;
  if ( nCar == 0 ) {
    *_dest = 0 ;
    return ; 
  }
  while (nCar ) {
    if ( *_source == BUFFER_EOL ) {
      *_dest = 0 ;
      return ;
    }
    *_dest = *_source ;
    _dest++;
    _source++;
    if ( ( _source - logBuffer) >=  MAX_BUFFER_SIZE ) _source = logBuffer ;
    nCar--;
  }
  *_dest = 0 ;
}


