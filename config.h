#ifndef _config_h
#define _config_h

#include "TFT_eSPI_ms/TFT_eSPI.h"

// decide if you will use Wifi or not (and how)
#define ESP32_ACT_AS_STATION               // select between NO_WIFI, ESP32_ACT_AS_STATION, ESP32_ACT_AS_AP 

// If wifi is used, set the parameter to identify the access point (= the router when ESP32_ACT_AS_STATION or the ESP32 when ESP32_ACT_AS_AP)
#define MY_SSID "bbox2-58c4"       // replace by the name of your access point (when act as station) or the name you assign to your device (when act as acces point)

// Only for ESP32_ACT_AS_STATION , set the password to get access to your access point (router)
#define MY_PASSWORD "yourpassword" // replace by the password of your access point (when act as station) or the password you want to use to protect your ESP32 (when act as acces point)
                                      // in this last case, password can remains empty 
// select your language between EN, FR, DE
#define LANGUAGE EN


// Here some pins (GPIO) being used
//************************************
// pin (GPIO) for touch screen are defined in tft_espi User_Setup.h file
//#define TS_CS_PIN  15

// Note: SPI is currently hardcoded for using following pins
// MOSI=13, MISO=12, SCK=14
// Those are used for the display, the touch panel and the SD card reader.

#define TFT_LED_PIN 25       // pin connected to led of lcd; pin has to be high to set led ON

// other TFT pins are defined in tft_espi User_Setup.h file

#define SD_CHIPSELECT_PIN 26  //5  // pin for SD card selection // to change probably

// pins for Serial to GRBL (it uses Serial2 UART)
#define SERIAL2_RXPIN 16
#define SERIAL2_TXPIN 17

// pin for Nunchuk are currently the defalult I2C pin so pins 21, 22

// This is the file name used to store the touch coordinate in the SPIFFS from ESP32 (in es32 flash memory)
// calibration data. Cahnge the name to start a new calibration.
#define CALIBRATION_FILE "/TouchCalData3"

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false


// note: this project allows to define up to 7 GRBL set of commands (makros) that can be called from setup screen.
// Those are defined by the user on a sd card and loaded on request into the ESP32 flash memory system (SPIFFS)
// So, once uploaded, they can be called without SD card.
// To be recognise as command, file name must be like Cmd3_xxxxxxxx.yyyy where
//       Cmd _ is fixed text (take care to the case)
//       3 is a digit from 1 up to 7; it defines the position of the button
//       xxxxxxxx will be the name of the button; first char must be a letter; min 1, max 16 characters, no space
//       yyy is the file name extension; it will be discarded
// The files to be used for upload must be placed in the root directory of SD card.
// To upload one of them, use the "Print" + "SD->GRBL" menus and then select the file.
// After reset, there will be a button named xxxxxxxxx in the "Setup" + "CMD" menu
// If you upload a file having the same digit (= button position) as an existing button, the new file will replace the button name and content of the previous button. 

// select color between (or define your own) 
// TFT_BLACK       0x0000      /*   0,   0,   0 */
// TFT_NAVY        0x000F      /*   0,   0, 128 */
// TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
// TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
// TFT_MAROON      0x7800      /* 128,   0,   0 */
// TFT_PURPLE      0x780F      /* 128,   0, 128 */
// TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
// TFT_LIGHTGREY   0xC618      /* 192, 192, 192 */
// TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
// TFT_BLUE        0x001F      /*   0,   0, 255 */
// TFT_GREEN       0x07E0      /*   0, 255,   0 */
// TFT_CYAN        0x07FF      /*   0, 255, 255 */
// TFT_RED         0xF800      /* 255,   0,   0 */
// TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
// TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
// TFT_WHITE       0xFFFF      /* 255, 255, 255 */
// TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
// TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
// TFT_PINK        0xFC9F 

//#define BUTTON_BORDER_NOT_PRESSED TFT_WHITE
//#define BUTTON_BORDER_PRESSED     TFT_RED
//#define BUTTON_BACKGROUND         TFT_BLUE
//#define BUTTON_TEXT               TFT_WHITE
//#define SCREEN_BACKGROUND         TFT_BLACK
//#define SCREEN_NORMAL_TEXT        TFT_GREEN
//#define SCREEN_ALERT_TEXT         TFT_RED
//#define SCREEN_HEADER_TEXT        TFT_WHITE

#define BUTTON_BORDER_NOT_PRESSED TFT_BLACK
#define BUTTON_BORDER_PRESSED TFT_RED
#define BUTTON_BACKGROUND TFT_GREEN
#define BUTTON_TEXT TFT_BLACK
#define SCREEN_BACKGROUND TFT_DARKGREY
#define SCREEN_NORMAL_TEXT TFT_GREEN
#define SCREEN_ALERT_TEXT TFT_RED
#define SCREEN_HEADER_TEXT TFT_WHITE

// *************************************     normally do not change here below ****************************
//       debugging
//#define DEBUG_TO_PC
#ifdef DEBUG_TO_PC
  #define COPY_GRBL_TO_PC
  #define DEBUG_TO_PC_MENU
#endif


#define DIR_LEVEL_MAX 6

#define MAX_FILES 4

//        commands available in menu ; this part is normally not used anymore because cmd are defined in SPIFFS
//#define CMD1_GRBL_CODE "*X"
//#define CMD2_GRBL_CODE "G01 X02"
//#define CMD3_GRBL_CODE "G01 X03"
//#define CMD4_GRBL_CODE "G01 X04"
//#define CMD5_GRBL_CODE "G01 X05"
//#define CMD6_GRBL_CODE "G01 X06"
//#define CMD7_GRBL_CODE "G01 X07"

//#define CMD1_NAME " Unlock GRBL"
//#define CMD2_NAME "G01 X02"
//#define CMD3_NAME "G01 X03"
//#define CMD4_NAME "G01 X04"
//#define CMD5_NAME "G01 X05"
//#define CMD6_NAME "G01 X06"
//#define CMD7_NAME "G01 X07"


#endif


