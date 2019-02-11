#ifndef _config_h
#define _config_h

// pins for touch screen are defined in tft_espi User_Setup.h file
//#define TS_CS_PIN  15

// Note: SPI is currently hardcoded for using following pins
// MOSI=13, MISO=12, SCK=14
// Those are used for the display, the touch panel and the SD card reader.

#define TFT_LED_PIN 25       // pin connected to led of lcd; pin has to be high to set led ON

#define SD_CHIPSELECT_PIN 26  //5  // pin for SD card selection // to change probably

// pins for Serial to GRBL
#define SERIAL2_RXPIN 16
#define SERIAL2_TXPIN 17

// pin for Nunchuk are currently the defalult I2C pin so pins 21, 22

// This is the file name used to store the touch coordinate
// calibration data. Cahnge the name to start a new calibration.
#define CALIBRATION_FILE "/TouchCalData3"

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false

// Set the parameter to connect to an access point (router) for the local network
#define MY_SSID "bbox2-58c4"       // replace by the name of your access point
#define MY_PASSWORD "alineloriejulien"   // replace by the password of your access point

#define ESP32_ACT_AS_STATION               // select between NO_WIFI, ESP32_ACT_AS_STATION, ESP32_ACT_AS_AP 

//        commands available in menu
#define CMD1_GRBL_CODE "*X"
#define CMD2_GRBL_CODE "G01 X02"
#define CMD3_GRBL_CODE "G01 X03"
#define CMD4_GRBL_CODE "G01 X04"
#define CMD5_GRBL_CODE "G01 X05"
#define CMD6_GRBL_CODE "G01 X06"
#define CMD7_GRBL_CODE "G01 X07"

#define CMD1_NAME " Unlock GRBL"
#define CMD2_NAME "G01 X02"
#define CMD3_NAME "G01 X03"
#define CMD4_NAME "G01 X04"
#define CMD5_NAME "G01 X05"
#define CMD6_NAME "G01 X06"
#define CMD7_NAME "G01 X07"


//                                    normally do not change here below
//       debugging
//#define DEBUG_TO_PC
#ifdef DEBUG_TO_PC
  #define COPY_GRBL_TO_PC
  #define DEBUG_TO_PC_MENU
#endif


#define DIR_LEVEL_MAX 6

#define MAX_FILES 4

#endif


