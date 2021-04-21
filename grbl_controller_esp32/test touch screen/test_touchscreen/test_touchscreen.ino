
#include "SPI.h"

SPIClass spiTouch(VSPI) ;

#define BOARD 1 // select between board 1 or 2
/*
#if defined(BOARD) && (BOARD == 1)
  #define TFT_MISO 19
  #define TFT_MOSI 23
  #define TFT_SCLK 18
  #define TFT_CS   13 //15  // Chip select control pin
  #define TFT_DC   14// 2  // Data Command control pin  
  #define TFT_RST  12// 4  // Reset pin (could connect to RST pin)
  //#define TFT_RST  -1  // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST
  #define TOUCH_CS 27 //21     // Chip select pin (T_CS) of touch screen
  #define TFT_LED_PIN 25 
  #define TOUCH_MISO 19
  #define TOUCH_MOSI 23
  #define TOUCH_SCLK 18
  
#else // board = 2 = new model with splitted bus
*/
  #define TFT_MISO 12
  #define TFT_MOSI 13
  #define TFT_SCLK 14
  #define TFT_CS   32  // Chip select control pin
  #define TFT_DC   27  // Data Command control pin
  #define TFT_RST  33  // Reset pin (could connect to RST pin)
  #define TOUCH_CS  26
  //#define TFT_LED_PIN 25       // pin connected to led of lcd; pin has to be high to set led ON
  //#define SD_CHIPSELECT_PIN 5  // pin for SD card selection
  #define TOUCH_MISO 19
  #define TOUCH_MOSI 23
  #define TOUCH_SCLK 18  
//#endif




void setup() {
  Serial.begin(115200);
  Serial.println("\n\nStarting...");

  
  //pinMode(TFT_DC, OUTPUT);
  //digitalWrite(TFT_DC, HIGH); // Data/Command high = data mode

 //pinMode(TFT_RST, OUTPUT);
 //digitalWrite(TFT_RST, HIGH); // Set high, do not share pin with another SPI device
  
  //pinMode(TFT_LED_PIN , OUTPUT) ;
  //digitalWrite(TFT_LED_PIN , HIGH) ;
    
  
  //SPI.begin(TOUCH_SCLK, TOUCH_MISO, TOUCH_MOSI, -1);
  spiTouch.begin(TOUCH_SCLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  Serial.println("\n\nEnd of setup...");
  //SPI.endTransaction();  

  //tft.begin();

  //tft.setRotation(1);
  //tft.fillScreen(TFT_NAVY); // Clear screen to navy background

}

void readOneAxis(uint8_t axis){
  uint16_t data ;
  //Serial.print("1");
  
  spiTouch.endTransaction();
  spiTouch.beginTransaction(SPISettings(2500000, MSBFIRST, SPI_MODE0));
  //Serial.print("2");
  digitalWrite( TOUCH_CS, LOW) ; // select the device
  //pinMode(TFT_RST, INPUT  );
  spiTouch.transfer(axis);
  //Serial.print("3"); 
  data = spiTouch.transfer16(axis) ;
  //Serial.print("4");
  Serial.print(" "); Serial.print(data);
  digitalWrite( TOUCH_CS, HIGH) ; // select the device
  spiTouch.endTransaction();
  //Serial.print("5");
  
}
#define X_AXIS 0xD3
#define Y_AXIS 0x93
#define Z_AXIS 0xB1


void loop() {
  // put your main code here, to run repeatedly:
  readOneAxis(X_AXIS);
  readOneAxis(Y_AXIS);
  readOneAxis(Z_AXIS);
  Serial.println("");
  delay(200); 
}
