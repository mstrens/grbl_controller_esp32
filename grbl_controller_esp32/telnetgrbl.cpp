// test a telnet connection to grbl 
#include <WiFi.h>              // Built-in
#include "telnetgrbl.h"
#include "com.h"  // to knwo the GRBL_LINK_SERIAL ...
#include "draw.h"
#include "setupTxt.h"

WiFiClient grblClient;
extern uint8_t grblLink ;
extern IPAddress grbl_Telnet_IP; // create IP adress with 4 values = 0; will be filled in init
extern M_pLabel mText[_MAX_TEXT];


//IPAddress grblIp(192,168,1,5);
//char grblName[]  = "grblesp.local";

uint32_t wifiGrblMillis = millis() ;
bool telnetConnected = false ;

/*
    btConnected = false;
    grblLink = GRBL_LINK_BT ;
    SerialBT.end();                    //stop active services
    SerialBT.begin("ESP32Ctrl", true);
    SerialBT.register_callback(&my_spp_cb);
    //Serial.println("[MSG: trying to connect to GRBL using BT]");
    drawMsgOnCom("Trying to connect to GRBL using BT", "It can take 1 min; please wait" );
    SerialBT.connect(GRBL_BT_NAME);
    if( SerialBT.connected(0)) {
      drawMsgOnCom("Successfully connected in BT", " ");
      fillMsg("BT Connected with GRBL");
      Serial.println("[MSG:Successfully connected in BT]");
      btLastUnsuccefullMillis = millis() ;
      btConnected = true; 
    } else {
      drawMsgOnCom("Unable to connect in BT", " ");
      Serial.print("[MSG:Unable to connect in BT to "); Serial.print(GRBL_BT_NAME); Serial.println("]");
      fillMsg("Unable to connect to GRBL in BT");
      btConnected = false;
    }
*/



bool telnetGrblInit(){
  grblLink = GRBL_LINK_TELNET ;
  grblClient.stop();
  while (grblClient.available() ) grblClient.read() ;
  telnetConnected = false ;
  if (WiFi.status() != WL_CONNECTED) {
        telnetConnected = false ;
        drawMsgOnTft(mText[_UNABLE_TO_CONNECT_TO_GRBL_TELNET].pLabel, mText[_WIFI_NOT_CONNECTED].pLabel  );
        fillMsg(_UNABLE_TO_CONNECT_TO_GRBL_TELNET);
        //Serial.println("[MSG: fail to connect to GRBL Telnet; WiFi not connected]");
    };
  drawMsgOnTft( mText[_TRY_TO_CONNECT_WITH_TELNET].pLabel , mText[_WAIT_1_MIN].pLabel );
  if (grblClient.connect("192.168.1.11", 23,1000)) {
    grblClient.setNoDelay(true);
    telnetConnected = true ;
    drawMsgOnTft(mText[_TELNET_CONNECTED_WITH_GRBL].pLabel, " " );
    fillMsg(_TELNET_CONNECTED_WITH_GRBL, SCREEN_NORMAL_TEXT);
    //Serial.println("[MSG: Connected to grbl using Telnet]");
  } else {
    telnetConnected = false ;
    drawMsgOnTft(mText[_UNABLE_TO_CONNECT_TO_GRBL_TELNET].pLabel , mText[_WIFI_CONNECTED].pLabel );
    //Serial.println("Unable to connect to GRBL Telnet; Wifi is connected");
    fillMsg(_UNABLE_TO_CONNECT_TO_GRBL_TELNET);
  } 
  return telnetConnected ;
}

void telnetGrblStop(){
  grblClient.stop();
  while (grblClient.available() ) grblClient.read() ;
  //grblLink = GRBL_LINK_SERIAL;
  telnetConnected = false ;
}

void toTelnet(char c) {
  grblClient.print(c);
  /*
  if ( (millis() - wifiGrblMillis) > 2000){
    Serial.print("connection lost 1 char, trying to reconnect");;Serial.println(millis());
    grblClient.stop();
    while (grblClient.available() ) grblClient.read() ;
    delay(10);
    if (grblClient.connect(grbl_Telnet_IP, 23,2000)) {
      Serial.print("reconnected to grbl telnet server");;Serial.println(millis());
      grblClient.setNoDelay(true);
      wifiGrblMillis = millis();
    } else {
      Serial.print("unable to reconnect:  char not send="); Serial.println(c);
    }
  }  
  if (c != '?'){
    Serial.print("telnet send char="); Serial.println(c);
  }
  grblClient.print(c);
  */
}

void toTelnet(const char * data) {
  grblClient.print(data);
  /*
  if ( (millis() - wifiGrblMillis) > 2000){
    Serial.print("connection lost, trying to reconnect at ");Serial.println(millis());
    grblClient.stop();
    while (grblClient.available() ) grblClient.read() ;
    delay(10);
    if (grblClient.connect(grbl_Telnet_IP, 23,2000)) {
      Serial.print("reconnected to grbl telnet server at ");;Serial.println(millis());
      grblClient.setNoDelay(true);
      wifiGrblMillis = millis();
    } else {
      Serial.print("unable to reconnect: data not send="); Serial.println(data);
    }
  }  
  Serial.print("telnet send char="); Serial.println(data);
  grblClient.print(data);
  */
}


bool fromTelnetAvailable( ){
  return grblClient.available();  
}  

int fromTelnetRead( ){
  wifiGrblMillis = millis() ;
  int val = grblClient.read();
  return val;
  //return grblClient.read();  
}  


/*
void wifiGrblSend(){
   //if (WiFi.status() != WL_CONNECTED){
   // Serial.println("WiFi satus is not connected");
   //}
  if ( (millis() - wifiGrblMillis) > 2000){
    Serial.println("connection lost, trying to reconnect");
    grblClient.stop();
    delay(10);
    if (grblClient.connect(grblIp, 23)) {
      Serial.println("reconnected to grbl telnet server");
      wifiGrblMillis = millis();
    } else {
      Serial.println("unable to reconnect");
    }
  }  
  Serial.println("Sending ? to grbl");
  grblClient.print("?");
}

void wifiGrblRead() {
   // Read all the lines of the reply from server and print them to Serial
    while(grblClient.available()) {
        String line = grblClient.readStringUntil('\r');
        Serial.println(line);
        wifiGrblMillis = millis() ;
    }
}

void testWifiGrbl(){
  wifiGrblMillis = millis();
  if (wifiGrblInit() ){
    int i = 0 ;
    for (i ; i<300 ; i++){
      Serial.print("loop= "); Serial.println(i);
      wifiGrblSend();
      wifiGrblRead();
      delay(200);
    }
    grblClient.stop();
    Serial.println("disconnecting after 100 send");
  } 
  if (wifiGrblInit() ){
    int i = 301 ;
    for (i ; i<600 ; i++){
      Serial.print("loop= "); Serial.println(i);
      wifiGrblSend();
      delay(100);
      wifiGrblRead();
      delay(100);
    }
    grblClient.stop();
  } 
}
*/
