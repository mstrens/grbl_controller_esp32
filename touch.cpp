/* Touchscreen library for XPT2046 Touch Controller Chip
 * Copyright (c) 2015, Paul Stoffregen, paul@pjrc.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
// modified by Bodmer to double sample, use Z value to improve detection reliability and to correct rotation handling
// modified by also by ms

#include "touch.h"
#define SPI_TOUCH_FREQUENCY  2500000

TOUCH::TOUCH(void){  
}

void TOUCH::begin(SPIClass &wspi ,uint8_t cspin){
  _pspi = &wspi;
  _pspi->begin();
  csPin = cspin;
  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, HIGH);
  //Serial.println("touchscreen begin done");
}

/***************************************************************************************
** Function name:           getTouch1Axis
** Description:             read one axis raw touch position as long as stable enough
***************************************************************************************/
uint16_t TOUCH::getTouch1Axis(uint8_t axis) {
  //Serial.println("entering getTouch1Axis");
  uint16_t data ;
  uint8_t i ;
  uint16_t dataMin ;
  uint16_t dataMax ;
  uint16_t dataSum ;
  _pspi->transfer(axis); 
  //i=0 ; for (i ; i<4 ; i++) data = SPI.transfer16(axis) ;
  do {
    i = 0 ;
    dataMin = 0xFF ;
    dataMax = 0 ;
    dataSum = 0 ;
    while (i<4) {
      data = _pspi->transfer16(axis) >> 3 ;
      if (data > dataMin) dataMin = data ;
      if (data < dataMax) dataMax = data ;
      dataSum += data ;
      i++;
    }   
  } while ( (dataMax - dataMin) > 30 ) ; // repeat if difference between min and max > tolerance  
  //Serial.print("ts 1 axe= "); Serial.println( dataSum>>2);
  return dataSum >> 2 ;
} 
/***************************************************************************************
** Function name:           getTouchRaw
** Description:             read raw touch position.; x,y,z are changed only if true (pressed) ; z is not really reliable
***************************************************************************************/
#define THRESEHOLD 40
#define X_AXIS 0xD3
#define Y_AXIS 0x93
#define Z1_AXIS 0xB1
boolean TOUCH::getTouchRaw(uint16_t *x, uint16_t *y, uint16_t *z)
{
//  uint32_t in = micros() ;
  uint16_t data ;
  bool has_touch = false;
  _pspi->beginTransaction(SPISettings(SPI_TOUCH_FREQUENCY, MSBFIRST, SPI_MODE0));
  digitalWrite(csPin, LOW);
  data = getTouch1Axis(Z1_AXIS) ;
  //Serial.println( " ") ; Serial.print (data) ; Serial.print( " , ") ;
  if ( data > THRESEHOLD ) {
    *z = data ;
    *x = getTouch1Axis(X_AXIS) ; 
    //Serial.print (*x) ; Serial.print( " , ") ;
    *y = getTouch1Axis(Y_AXIS) ;
    //Serial.print (*y) ; Serial.print( " , ") ;
    has_touch = true;
  }
  //SPI.transfer16(0x80); // set power down mode; not really needed, because touch touchscreen does not consume a lot and normally it is polled quite often 
  
  digitalWrite(csPin, HIGH);
  _pspi->endTransaction();
//  Serial.println( micros() - in ) ;
  return has_touch;
}

  
/***************************************************************************************
** Function name:           getTouch
** Description:             read callibrated position. Return false if not pressed. threshold is not used anymore in this code; kept just for compatibility with previous version
***************************************************************************************/
boolean TOUCH::getTouch(int16_t *x, int16_t *y, uint16_t threshold)
{
  uint16_t xt, yt ,zt ;
  int16_t x_press , y_press ; 
  if (!getTouchRaw(&xt, &yt, &zt) )  return false;
  //Serial.print("x y z =") ; Serial.print( xt) ; Serial.print( " , ") ;  Serial.print( yt) ; Serial.print( " , ") ; Serial.println( zt) ;
  x_press = xt ;  // convert to int16_t
  y_press = yt ;
  convertRawXY( &x_press ,  &y_press);
  //Serial.print("conv x y =") ; Serial.print( x_press) ; Serial.print( " , ") ;  Serial.print( y_press) ; Serial.println( " ") ;
  if (x_press >= _width || y_press >= _height || x_press <0 || y_press < 0 ) return false;
  *x = x_press ;
  *y = y_press ;
  return true ;
}



/***************************************************************************************
** Function name:           convertRawXY
** Description:             convert raw touch x,y values to screen coordinates 
***************************************************************************************/
void TOUCH::convertRawXY(int16_t *x, int16_t *y)
{
  int16_t x_tmp = *x, y_tmp = *y, xx, yy;

  if(!touchCalibration_rotate){
    xx=(x_tmp-touchCalibration_x0)*_width/touchCalibration_x1;
    yy=(y_tmp-touchCalibration_y0)*_height/touchCalibration_y1;
    if(touchCalibration_invert_x)
      xx = _width - xx;
    if(touchCalibration_invert_y)
      yy = _height - yy;
  } else {
    xx=(y_tmp-touchCalibration_x0)*_width/touchCalibration_x1;
    yy=(x_tmp-touchCalibration_y0)*_height/touchCalibration_y1;
    if(touchCalibration_invert_x)
      xx = _width - xx;
    if(touchCalibration_invert_y)
      yy = _height - yy;
  }
  *x = xx;
  *y = yy;
}

/***************************************************************************************
** Function name:           setTouch
** Description:             imports calibration parameters for touchscreen. 
***************************************************************************************/
void TOUCH::setTouch(uint16_t *parameters){
  touchCalibration_x0 = parameters[0];
  touchCalibration_x1 = parameters[1];
  touchCalibration_y0 = parameters[2];
  touchCalibration_y1 = parameters[3];

  if(touchCalibration_x0 == 0) touchCalibration_x0 = 1;
  if(touchCalibration_x1 == 0) touchCalibration_x1 = 1;
  if(touchCalibration_y0 == 0) touchCalibration_y0 = 1;
  if(touchCalibration_y1 == 0) touchCalibration_y1 = 1;

  touchCalibration_rotate = parameters[4] & 0x01;
  touchCalibration_invert_x = parameters[4] & 0x02;
  touchCalibration_invert_y = parameters[4] & 0x04; 
}

void   TOUCH::printCalibration( void) {
  Serial.print(touchCalibration_x0); Serial.print(" , " ) ;
  Serial.print(touchCalibration_x1); Serial.print(" , " ) ;
  Serial.print(touchCalibration_y0); Serial.print(" , " ) ;
  Serial.print(touchCalibration_y1); Serial.print(" , " ) ;

  Serial.print(touchCalibration_rotate); Serial.print(" , " ) ;
  Serial.print(touchCalibration_invert_x); Serial.print(" , " ) ;
  Serial.println(touchCalibration_invert_y); 
}
