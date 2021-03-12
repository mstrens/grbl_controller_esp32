
#include "SPI.h"

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   13 //15  // Chip select control pin
#define TFT_DC   14// 2  // Data Command control pin

#define TFT_RST  12// 4  // Reset pin (could connect to RST pin)

//#define TFT_RST  -1  // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST
#define TOUCH_CS 27 //21     // Chip select pin (T_CS) of touch screen

#define TFT_LED_PIN 25 



void setup() {
  Serial.begin(115200);
  Serial.println("\n\nStarting...");

  
  pinMode(TFT_DC, OUTPUT);
  digitalWrite(TFT_DC, HIGH); // Data/Command high = data mode

 pinMode(TFT_RST, OUTPUT);
 digitalWrite(TFT_RST, HIGH); // Set high, do not share pin with another SPI device
  
  pinMode(TFT_LED_PIN , OUTPUT) ;
  digitalWrite(TFT_LED_PIN , HIGH) ;
    
  
  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, -1);
  Serial.println("\n\nEnd of setup...");
  //SPI.endTransaction();  

  tft.begin();

  tft.setRotation(1);
  tft.fillScreen(TFT_NAVY); // Clear screen to navy background

}

void readOneAxis(uint8_t axis){
  uint16_t data ;
  //Serial.print("1");
  SPI.endTransaction();
  SPI.beginTransaction(SPISettings(2500000, MSBFIRST, SPI_MODE0));
  //Serial.print("2");
  digitalWrite( TOUCH_CS, LOW) ; // select the device
  //pinMode(TFT_RST, INPUT  );
  SPI.transfer(axis);
  //Serial.print("3"); 
  data = SPI.transfer16(axis) ;
  //Serial.print("4");
  Serial.print(" "); Serial.print(data);
  digitalWrite( TOUCH_CS, HIGH) ; // select the device
  SPI.endTransaction();
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
  //delay(500); 
}
