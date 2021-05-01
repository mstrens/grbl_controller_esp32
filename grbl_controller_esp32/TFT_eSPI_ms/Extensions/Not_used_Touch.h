// original file
// Coded by Bodmer 10/2/18, see license in root directory.
 // This is part of the TFT_eSPI class and is associated with the Touch Screen handlers

 // modified by ms
 
 public:
           // Get the value for one axis ( based on the average of 4 when min is nearly equal to max
	uint16_t getTouch1Axis(uint8_t axis) ;
		   // Get raw x,y ADC values from touch controller
  boolean  getTouchRaw(uint16_t *x, uint16_t *y , uint16_t *z );
           // Get raw z (i.e. pressure) ADC value from touch controller
//  uint16_t getTouchRawZ(void);
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
           // Handlers for the SPI settings and clock speed change
  inline void spi_begin_touch() __attribute__((always_inline));
  inline void spi_end_touch()   __attribute__((always_inline));

           // Private function to validate a touch, allow settle time and reduce spurious coordinates
 // uint8_t  validTouch(uint16_t *x, uint16_t *y, uint16_t threshold = 600);

           // Initialise with example calibration values so processor does not crash if setTouch() not called in setup()
  uint16_t touchCalibration_x0 = 300, touchCalibration_x1 = 3600, touchCalibration_y0 = 300, touchCalibration_y1 = 3600;
  uint8_t  touchCalibration_rotate = 1, touchCalibration_invert_x = 2, touchCalibration_invert_y = 0;

//  uint32_t _pressTime;        // Press and hold time-out
//  uint16_t _pressX, _pressY;  // For future use (last sampled calibrated coordinates)