/*
  BTConfig.cpp -  Bluetooth functions class


  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#    include <BluetoothSerial.h>
#    include "bt.h"
#    include "com.h"
#include "config.h"
#include "draw.h"
#include "setupTxt.h"
    BluetoothSerial SerialBT;
#    ifdef __cplusplus
    extern "C" {
#    endif
    const uint8_t* esp_bt_dev_get_address(void);
#    ifdef __cplusplus
    }
#    endif

extern uint8_t grblLink ;
extern M_pLabel mText[_MAX_TEXT];

bool btConnected;
//uint32_t btLastUnsuccefullMillis = millis() ;



void my_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t* param) {
    switch (event) {
        case ESP_SPP_OPEN_EVT:   //Client connection open
            //char str[18];
            //str[17]       = '\0';
            //uint8_t* addr = param->srv_open.rem_bda;
            //sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
            //BT::_btclient = str;
            Serial.println("[MSG:BT Connected with GRBL reported by an event]");
            //btLastUnsuccefullMillis = millis(); 
            btConnected = true ; 
            break;
        case ESP_SPP_CLOSE_EVT:  //Client connection closed
            Serial.print("[MSG:BT Disconnected]\r\n");
            fillMsg(_BT_DISCONNECTED);
            btConnected = false; 
            //BTg::_btclient = "";
            break;
        default:
            break;
    }
}
    
void btGrblInit() {
    btConnected = false;
    grblLink = GRBL_LINK_BT ;
    //SerialBT.end();     //stop active services has a bug and force ESP32 to restart
    SerialBT.register_callback(&my_spp_cb);
    if (!SerialBT.begin("ESP32Ctrl", true)) {
      Serial.println("[MSG: An error occurred initializing Bluetooth]");
    }
    drawMsgOnTft(mText[_TRY_TO_CONNECT_WITH_BT].pLabel , mText[_WAIT_1_MIN].pLabel ); // affiche sur ligne 100 et 120
    SerialBT.connect(GRBL_BT_NAME);
    delay(100); // we wait for the caalback function to change the flag btConnected because SerialBT.connect() always return true even when not connected
    //if( SerialBT.connected(0)) {    // bug in this function: always return true
    if( btConnected ){
      drawMsgOnTft(mText[_BT_CONNECTED_WITH_GRBL].pLabel, " ");
      fillMsg(_BT_CONNECTED_WITH_GRBL , SCREEN_NORMAL_TEXT );
      Serial.println("[MSG:Successfully connected in BT]");
      //btLastUnsuccefullMillis = millis() ;  // not sure it is used anymore; foreseen to make auto restart of connexion
      btConnected = true; 
    } else {
      drawMsgOnTft(mText[_UNABLE_TO_CONNECT_IN_BT].pLabel, " ");
      Serial.print("[MSG:Unable to connect in BT to "); Serial.print(GRBL_BT_NAME); Serial.println("]");
      fillMsg(_UNABLE_TO_CONNECT_IN_BT);
      btConnected = false;
    }
}        

void btGrblStop() {
  //SerialBT.end();
  btConnected = false;
}

        
void toBt(char c) {
  if (btConnected){
    SerialBT.print(c);
  } /*else if ( millis() - btLastUnsuccefullMillis > 10000 ){
        SerialBT.end();                    //stop active services
        SerialBT.begin("ESP32Ctrl", true); 
        SerialBT.register_callback(&my_spp_cb);
        Serial.println("[MSG: trying to reconnect to GRBL using BT]");
        SerialBT.connect(GRBL_ESP32_BT_NAME);
        if( SerialBT.connected(0)) { // check if connected (no wait)
          btLastUnsuccefullMillis = 0;
          SerialBT.print(c);
        } else {
          Serial.print("[MSG:Unable to reconnect in BT]");
          btLastUnsuccefullMillis = millis();
        }
  }*/
}

void toBt(const char * data) {
  if (btConnected){
    SerialBT.print(data);
  } /*else if ( millis() - btLastUnsuccefullMillis > 10000 ){
        SerialBT.end();                    //stop active services
        SerialBT.begin("ESP32Ctrl", true); 
        SerialBT.register_callback(&my_spp_cb);
        Serial.println("[MSG: trying to reconnect to GRBL using BT]");
        SerialBT.connect(GRBL_ESP32_BT_NAME);
        if( SerialBT.connected(0)) {
          SerialBT.print(data);      
          btLastUnsuccefullMillis = 0;
        } else {
          Serial.print("[MSG:Unable to reconnect in BT]");
          btLastUnsuccefullMillis = millis();
        }
  }*/
}

