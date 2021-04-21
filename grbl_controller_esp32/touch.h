// original file Coded by Bodmer 10/2/18
// modified by ms in order to have a separate class for touchscreen and to use another SPI
#ifndef Touchscreen_h_
#define Touchscreen_h_

#include "Arduino.h"
#include <SPI.h>
#include "TFT_eSPI_ms/User_Setup_Select.h" // used to reuse the size of the TFT
class TOUCH {
  public:
  TOUCH() ;
            // initialise le Touchscreen avec le SPI utilisé
  void begin(SPIClass &wspi ,uint8_t cspin ) ; 
           // Get the value for one axis ( based on the average of 4 when min is nearly equal to max
  uint16_t getTouch1Axis(uint8_t axis) ;
       // Get raw x,y ADC values from touch controller
  boolean  getTouchRaw(uint16_t *x, uint16_t *y , uint16_t *z );
           // Convert raw x,y values to calibrated and correctly rotated screen coordinates
  void     convertRawXY(int16_t *x, int16_t *y); // originally it was uint16_t
           // Get the screen touch coordinates, returns true if screen has been touched
           // if the touch cordinates are off screen then x and y are not updated
  boolean  getTouch(int16_t *x, int16_t *y, uint16_t threshold = 600);  // the 2 first where originally uint16_t, return was uint8_t
           // Run screen calibration and test, report calibration values to the serial port
  void     calibrateTouch(uint16_t *data, uint32_t color_fg, uint32_t color_bg, uint8_t size);
           // Set the screen calibration values
  void     setTouch(uint16_t *data);
  void     printCalibration( void) ;
 
 private:
           // Initialise with example calibration values so processor does not crash if setTouch() not called in setup()
  uint16_t touchCalibration_x0 = 300;
  uint16_t touchCalibration_x1 = 3600;
  uint16_t touchCalibration_y0 = 300;
  uint16_t touchCalibration_y1 = 3600;
  uint8_t  touchCalibration_rotate = 1;
  uint8_t  touchCalibration_invert_x = 2;
  uint8_t  touchCalibration_invert_y = 0;
  SPIClass *_pspi = nullptr; // pointer to the SPI class being used
  uint8_t csPin ; // pin for chip select
  int16_t _width = TFT_HEIGHT ; //TFT_WIDTH; // width  defined in TFT_eSPI_ms/User_Setup_Select.h; here we invert width and height because we use rotation =1 
  int16_t _height = TFT_WIDTH ;//TFT_HEIGHT; // height defined in TFT_eSPI_ms/User_Setup_Select.h ; here we invert width and height because we use rotation =1 
};

#endif
