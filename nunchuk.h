/**
 * @license Nunchuk Arduino library v0.0.1 16/12/2016
 * http://www.xarg.org/2016/12/arduino-nunchuk-library/
 *
 * Copyright (c) 2016, Robert Eisele (robert@xarg.org)
 * Dual licensed under the MIT or GPL Version 2 licenses.
 **/

#ifndef NUNCHUK_H
#define NUNCHUK_H

#include <Wire.h>
             
void nunchuk_init() ;
uint8_t nunchuk_read() ;

uint8_t nunchuk_buttonZ() ;

uint8_t nunchuk_buttonC() ;
void handleNunchuk (void) ;

void disableEncription() ;

// list of status used also when sending
enum { JOG_NO = 0 , JOG_WAIT_END_CANCEL , JOG_WAIT_END_CMD } ;

#endif

