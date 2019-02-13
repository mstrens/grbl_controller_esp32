// original file
// The following touch screen support code by maxpautsch was merged 1/10/17
// https://github.com/maxpautsch

// Define TOUCH_CS is the user setup file to enable this code

// A demo is provided in examples Generic folder

// Additions by Bodmer to double sample, use Z value to improve detection reliability
// and to correct rotation handling

// See license in root directory.

// modified by ms

#define DEBUG_CALIBRATION // put the calibration parameters on serial port

/***************************************************************************************
** Function name:           getTouch1Axis
** Description:             read one axis raw touch position as long as stable enough
***************************************************************************************/
uint16_t TFT_eSPI::getTouch1Axis(uint8_t axis) {
	uint16_t data ;
	uint8_t i ;
	uint16_t dataMin ;
	uint16_t dataMax ;
	uint16_t dataSum ;
	SPI.transfer(axis); 
	//i=0 ; for (i ; i<4 ; i++) data = SPI.transfer16(axis) ;
	do {
		i = 0 ;
		dataMin = 0xFF ;
		dataMax = 0 ;
		dataSum = 0 ;
		for (i ; i<4 ; i++) {
			data = SPI.transfer16(axis) >> 3 ;
			if (data > dataMin) dataMin = data ;
			if (data < dataMax) dataMax = data ;
			dataSum += data ;
		}		
	} while ( (dataMax - dataMin) > 30 ) ; // repeat if difference between min and max > tolerance	
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
boolean TFT_eSPI::getTouchRaw(uint16_t *x, uint16_t *y, uint16_t *z)
{
//  uint32_t in = micros() ;
  uint16_t data ;
  bool has_touch = false;
  spi_begin_touch();
  T_CS_L;
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
  
  T_CS_H;
  spi_end_touch();
//	Serial.println( micros() - in ) ;
  return has_touch;
}

  
/***************************************************************************************
** Function name:           getTouch
** Description:             read callibrated position. Return false if not pressed. threshold is not used anymore in this code; kept just for compatibility with previous version
***************************************************************************************/
boolean TFT_eSPI::getTouch(int16_t *x, int16_t *y, uint16_t threshold)
{
  uint16_t xt, yt ,zt ;
  int16_t x_press , y_press ; 
  if (!getTouchRaw(&xt, &yt, &zt) )  return false;
 // Serial.print("x y z =") ; Serial.print( xt) ; Serial.print( " , ") ;  Serial.print( yt) ; Serial.print( " , ") ; Serial.println( zt) ;
  x_press = xt ;  // convert to int16_t
  y_press = yt ;
  convertRawXY( &x_press ,  &y_press);
   if (x_press >= _width || y_press >= _height || x_press <0 || y_press < 0 ) return false;
  *x = x_press ;
  *y = y_press ;
  return true ;
}



/***************************************************************************************
** Function name:           convertRawXY
** Description:             convert raw touch x,y values to screen coordinates 
***************************************************************************************/
void TFT_eSPI::convertRawXY(int16_t *x, int16_t *y)
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
** Function name:           calibrateTouch
** Description:             generates calibration parameters for touchscreen. 
***************************************************************************************/
//#define Z_THRESHOLD 600
void TFT_eSPI::calibrateTouch(uint16_t *parameters, uint32_t color_fg, uint32_t color_bg, uint8_t size){
  int16_t values[] = {0,0,0,0,0,0,0,0};
  uint16_t x_tmp, y_tmp , z_tmp ;



  for(uint8_t i = 0; i<4; i++){
    fillRect(0, 0, size+1, size+1, color_bg);
    fillRect(0, _height-size-1, size+1, size+1, color_bg);
    fillRect(_width-size-1, 0, size+1, size+1, color_bg);
    fillRect(_width-size-1, _height-size-1, size+1, size+1, color_bg);

    if (i == 5) break; // used to clear the arrows
    
    switch (i) {
      case 0: // up left
        drawLine(0, 0, 0, size, color_fg);
        drawLine(0, 0, size, 0, color_fg);
        drawLine(0, 0, size , size, color_fg);
        break;
      case 1: // bot left
        drawLine(0, _height-size-1, 0, _height-1, color_fg);
        drawLine(0, _height-1, size, _height-1, color_fg);
        drawLine(size, _height-size-1, 0, _height-1 , color_fg);
        break;
      case 2: // up right
        drawLine(_width-size-1, 0, _width-1, 0, color_fg);
        drawLine(_width-size-1, size, _width-1, 0, color_fg);
        drawLine(_width-1, size, _width-1, 0, color_fg);
        break;
      case 3: // bot right
        drawLine(_width-size-1, _height-size-1, _width-1, _height-1, color_fg);
        drawLine(_width-1, _height-1-size, _width-1, _height-1, color_fg);
        drawLine(_width-1-size, _height-1, _width-1, _height-1, color_fg);
        break;
      }

    // user has to get the chance to release
    if(i>0) delay(1000);
	while (getTouchRaw( &x_tmp, &y_tmp, &z_tmp )) ;  // wait that touch has been released

    for(uint8_t j= 0; j<8; j++){
      // Use a lower detect threshold as corners tend to be less sensitive
      //while( (!getTouchRaw( &x_tmp, &y_tmp, &z_tmp )) || z_tmp > Z_THRESHOLD);
	  while ( !getTouchRaw( &x_tmp, &y_tmp, &z_tmp )) ;
	 // Serial.print("cal x y for corner = ") ; Serial.print(i) ; Serial.print(" , ") ; Serial.print(x_tmp) ; Serial.print(" , ") ; Serial.println(y_tmp) ;  Serial.print(" , ") ; Serial.println(z_tmp) ;
      values[i*2  ] += x_tmp;
      values[i*2+1] += y_tmp;
      }
    values[i*2  ] /= 8;
    values[i*2+1] /= 8;
  }


  // from case 0 to case 1, the y value changed. 
  // If the measured delta of the touch x axis is bigger than the delta of the y axis, the touch and TFT axes are switched.
  touchCalibration_rotate = false;
  if(abs(values[0]-values[2]) > abs(values[1]-values[3])){
    touchCalibration_rotate = true;
    touchCalibration_x0 = (values[1] + values[3])/2; // calc min x
    touchCalibration_x1 = (values[5] + values[7])/2; // calc max x
    touchCalibration_y0 = (values[0] + values[4])/2; // calc min y
    touchCalibration_y1 = (values[2] + values[6])/2; // calc max y
  } else {
    touchCalibration_x0 = (values[0] + values[2])/2; // calc min x
    touchCalibration_x1 = (values[4] + values[6])/2; // calc max x
    touchCalibration_y0 = (values[1] + values[5])/2; // calc min y
    touchCalibration_y1 = (values[3] + values[7])/2; // calc max y
  }

  // in addition, the touch screen axis could be in the opposite direction of the TFT axis
  touchCalibration_invert_x = false;
  if(touchCalibration_x0 > touchCalibration_x1){
    values[0]=touchCalibration_x0;
    touchCalibration_x0 = touchCalibration_x1;
    touchCalibration_x1 = values[0];
    touchCalibration_invert_x = true;
  }
  touchCalibration_invert_y = false;
  if(touchCalibration_y0 > touchCalibration_y1){
    values[0]=touchCalibration_y0;
    touchCalibration_y0 = touchCalibration_y1;
    touchCalibration_y1 = values[0];
    touchCalibration_invert_y = true;
  }

  // pre calculate
  touchCalibration_x1 -= touchCalibration_x0;
  touchCalibration_y1 -= touchCalibration_y0;

  if(touchCalibration_x0 == 0) touchCalibration_x0 = 1;
  if(touchCalibration_x1 == 0) touchCalibration_x1 = 1;
  if(touchCalibration_y0 == 0) touchCalibration_y0 = 1;
  if(touchCalibration_y1 == 0) touchCalibration_y1 = 1;

  // export parameters, if pointer valid
  if(parameters != NULL){
    parameters[0] = touchCalibration_x0;
    parameters[1] = touchCalibration_x1;
    parameters[2] = touchCalibration_y0;
    parameters[3] = touchCalibration_y1;
    parameters[4] = touchCalibration_rotate | (touchCalibration_invert_x <<1) | (touchCalibration_invert_y <<2);
  }
#ifdef DEBUG_CALIBRATION
	Serial.println(  " =>  end of calibration lib" ) ;
#endif  
}


/***************************************************************************************
** Function name:           setTouch
** Description:             imports calibration parameters for touchscreen. 
***************************************************************************************/
void TFT_eSPI::setTouch(uint16_t *parameters){
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

void   TFT_eSPI::printCalibration( void) {
  Serial.print(touchCalibration_x0); Serial.print(" , " ) ;
  Serial.print(touchCalibration_x1); Serial.print(" , " ) ;
  Serial.print(touchCalibration_y0); Serial.print(" , " ) ;
  Serial.print(touchCalibration_y1); Serial.print(" , " ) ;

  Serial.print(touchCalibration_rotate); Serial.print(" , " ) ;
  Serial.print(touchCalibration_invert_x); Serial.print(" , " ) ;
  Serial.println(touchCalibration_invert_y); 
}