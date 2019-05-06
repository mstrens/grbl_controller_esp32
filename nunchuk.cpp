/**
 * @license Nunchuk Arduino library v0.0.1 16/12/2016
 * http://www.xarg.org/2016/12/arduino-nunchuk-library/
 *
 * Copyright (c) 2016, Robert Eisele (robert@xarg.org)
 * Dual licensed under the MIT or GPL Version 2 licenses.
 **/

#include <Wire.h>
#include "config.h"
#include "language.h"
#include "com.h"
#include "nunchuk.h"
#include "draw.h"


// Calibration joystick values
#define NUNCHUK_JOYSTICK_X_ZERO 127
#define NUNCHUK_JOYSTICK_Y_ZERO 128

// The Nunchuk I2C address
#define NUNCHUK_ADDRESS 0x52

#if ARDUINO >= 100
#define I2C_READ() Wire.read()
#define I2C_WRITE(x) Wire.write(x)
#else
#define I2C_READ() Wire.receive()
#define I2C_WRITE(x) Wire.send(x)
#endif

#define I2C_START(x) Wire.beginTransmission(x)
#define I2C_STOP() Wire.endTransmission(true)

uint8_t nunchuk_data[6];

int8_t prevMoveX = 0;
int8_t prevMoveY = 0;
int8_t prevMoveZ = 0;
int8_t jogDistX = 0;
int8_t jogDistY = 0;
int8_t jogDistZ = 0;
float moveMultiplier ;
uint32_t cntSameMove = 0 ;
uint32_t startMoveMillis = 0;

uint8_t jog_status = JOG_NO ;
boolean jogCancelFlag = false ;
boolean jogCmdFlag = false ; 

boolean nunchukOK ;  // keep flag to detect a nunchuk at startup

extern char machineStatus[9];
//extern char lastMsg[80] ;

/**
 * Initializes the Nunchuk communication by sending a sequence of bytes
 */
void nunchuk_init() {
    Wire.begin();
    // Change TWI speed for nunchuk, which uses Fast-TWI (400kHz)
    // Normally this will be set in twi_init(), but this hack works without modifying the original source
    Wire.setClock(400000);
    delay(500);    // delay ajouté pour donner le temps à la nunchuk de s'initialiser

    Wire.beginTransmission(NUNCHUK_ADDRESS);
    if ( Wire.endTransmission()  ) {    // return 0 if the I2C device replies without error
      nunchukOK = false ;
      fillMsg( __NO_NUNCHUK  ) ;
    } else {
       nunchukOK = true ;
       disableEncription() ;
       nunchuk_read() ;  // read once to (perhaps) avoid error message at start up
       delay(20);
    }
    //disableEncription() ;
}

void disableEncription() {
    I2C_START(NUNCHUK_ADDRESS);  // disable encription with next 2 commands
    I2C_WRITE(0xF0);
    I2C_WRITE(0x55);
    I2C_STOP();
    I2C_START(NUNCHUK_ADDRESS);
    I2C_WRITE(0xFB);
    I2C_WRITE(0x00);
    I2C_STOP();
    }

/**
 * Central function to read a full chunk of data from Nunchuk
 *
 * @return A boolean if the data transfer was successful
 */
uint8_t nunchuk_read() {
    uint8_t i;
    uint8_t error = 0 ;
    Wire.requestFrom(NUNCHUK_ADDRESS, 6);
    //delayMicroseconds(10);
    for (i = 0; i < 6 && Wire.available(); i++) {
          nunchuk_data[i] = I2C_READ();
          if ( nunchuk_data[i] == 0 || nunchuk_data[i] == 0xFF) { 
            error |= (1 << i) ;
          }
    //      Serial.print( (uint8_t) nunchuk_data[i] , HEX) ;
    }
    //Serial.println(" ");
    error = ( error == 0b00111111 ); // error when all received bytes are 00 or FF.
    if ( error ) {     
      disableEncription() ;  // try to reactivate nunchunck in case of error.
    }  
    I2C_START(NUNCHUK_ADDRESS);
    I2C_WRITE(0x00);
    I2C_STOP();
    return !error ; // return true when there is no error
}

uint8_t nunchuk_buttonZ() {      // Checks the current state of button Z
    return (~nunchuk_data[5] >> 0) & 1;
}

uint8_t nunchuk_buttonC() {      // Checks the current state of button C
    return (~nunchuk_data[5] >> 1) & 1;
}


#define NUNCHUK_READ_DELAY 100 // read nunchuk every 100 msec (about)
//#define NUNCHUK_READ_DELAY_FIRST_CMD  5000 
// (if statusPrinting = PRINTING_STOPPED  and) if machineStatus= Idle or Jog, then if delay since previous read exceed 100msec, then read nunchuk data
//  and if buttonZ or buttonC is released while it was pressed bebore, then send a command to cancel JOG (= char 0x85 )
//  If button buttonZ or buttonC is pressed and if joysttick is moved, then send a jog command accordingly
//  sending grbl jogging commands is done in com.cpp because it has to wait for "ok" after sending a command.
// There are 2 type of grbl commands  : one for canceling and some for jogging

void handleNunchuk (void) {
  uint32_t nunchukMillis = millis() ;
  static uint32_t lastNunchukMillis = 0;
//  static uint8_t previousButtonC = 0 ;
//  static uint8_t previousButtonZ = 0 ;
//  int8_t moveDx ;
//  int8_t moveDy ;
  int8_t moveX = 0 ; //static int8 prevMoveX = 0;
  int8_t moveY = 0 ; //static int8 prevMoveY = 0;
  int8_t moveZ = 0 ; //static int8 prevMoveZ = 0;
  //float moveMultiplier ;
  //Serial.print("handle=");Serial.println( machineStatus[0] , HEX);
    if  ( ( nunchukMillis - lastNunchukMillis ) > NUNCHUK_READ_DELAY  )    {                // we can not read to fast
      lastNunchukMillis = nunchukMillis  ;
      if (nunchuk_read() ) {
          //Serial.print(nunchuk_data[0],HEX); Serial.print(",");Serial.print(nunchuk_data[1],HEX); Serial.print(",");Serial.println(nunchuk_data[5] & 0x03 , HEX);
          if ( nunchuk_buttonC() && nunchuk_buttonZ() == 0 ) {     // si le bouton C est enfoncé mais pas le bouton Z  
            //Serial.print( ( int )nunchuk_data[0] ) ; Serial.print(" ") ; Serial.println( ( int )nunchuk_data[1] ) ; 
            if (nunchuk_data[0] < 80 ) {
              moveX = - 1 ;
            } else if (nunchuk_data[0] > 170 ) {
              moveX =  1 ;
            }
            if (nunchuk_data[1] < 80 ) {
              moveY = - 1 ;
            } else if (nunchuk_data[1] > 170 ) {
              moveY =  1 ;
            }
          } else if ( nunchuk_buttonZ() && nunchuk_buttonC() == 0 ) {   // si le bouton Z est enfoncé mais pas le bouton C
             if (nunchuk_data[1] < 80 ) {
              moveZ = - 1 ;
            } else if (nunchuk_data[1] > 170 ) {
              moveZ =  1 ;
            } 
          }
          //if ( (machineStatus[0] == 'J' ) && ( ( prevMoveX != moveX) || ( prevMoveY != moveY)  || ( prevMoveZ != moveZ) ) ) { // cancel Jog if jogging and t least one direction change 
          if ( (machineStatus[0] == 'J' || machineStatus[0] == 'I' ) && ( ( prevMoveX != moveX) || ( prevMoveY != moveY)  || ( prevMoveZ != moveZ) ) ) { // cancel Jog if jogging and at least one direction change       
            jogCancelFlag = true ; 
            cntSameMove = 0 ;             // reset the counter
            //Serial.println("cancel jog") ;
          } else {
            //jogCancelFlag = false ;
          }
          if ( moveX || moveY || moveZ) {    // if at least one move is asked
            if (cntSameMove == 0 ) { 
            //  moveMultiplier = 0.01 ;
              startMoveMillis = millis() ; 
            } 
            //else if (cntSameMove < 5 ) {   // avoid to send to fast a new move
            //  moveMultiplier = 0.0 ;
            //} else if (cntSameMove < 10 ) {
            //  moveMultiplier = 0.01 ;
            //} else if (cntSameMove < 15 ) {
            //  moveMultiplier = 0.1 ;
            //} else if (cntSameMove < 20 ) {
            //  moveMultiplier = 1 ;
            //} else {
            //  moveMultiplier = 4;
            //} 
            cntSameMove++ ;
            jogCmdFlag = true ;
            jogDistX = moveX;
            jogDistY = moveY;
            jogDistZ = moveZ;
          } else {               // no move asked ( moveX || moveY || moveZ) 
            cntSameMove = 0 ;
          //  moveMultiplier = 0 ; // put the value on 0 to avoid an old move to be execute  ; let the flag to be reset by the com.cpp file after a OK being received
            //jogCmdFlag = false ;
          } // end if ( moveX || moveY || moveZ)
          prevMoveX = moveX ;
          prevMoveY = moveY ;
          prevMoveZ = moveZ ;
          //Serial.print("cnt= ") ; Serial.println( cntSameMove ) ;
      }  // end of nunchukRead is true
    }  //end of test on delay  
} // end handleNunchuk

