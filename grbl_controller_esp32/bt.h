#ifndef bt_h
#define bt_h

#include <BluetoothSerial.h>

void btGrblInit();
void btGrblStop() ;
void my_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t* param) ;
void toBt(const char * data) ;
void toBt(char c) ;


#endif
