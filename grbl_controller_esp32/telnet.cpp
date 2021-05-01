#include <WiFi.h>              // Built-in


#define MAX_SRV_CLIENTS 1
WiFiServer telnetServer(23);
WiFiClient telnetClient;


void telnetInit() {
  telnetServer.begin();
  telnetServer.setNoDelay(true) ;
}



void telnetStop() {
  telnetClient.stop() ;
  telnetServer.stop() ;
}

void checkTelnetConnection() {            // check if we can accept a new connection (only 1 connection can be active)
  if (telnetServer.hasClient()) {      
    if (telnetClient && telnetClient.connected()) {
      WiFiClient newClient;                    // Verify if the IP is same than actual conection
      newClient = telnetServer.available();
      String ip = newClient.remoteIP().toString();
      if (ip == telnetClient.remoteIP().toString()) {
        telnetClient.stop();                   // Reconnect
        telnetClient = newClient;
      } else {                                 // Disconnect (not allow more than one connection)
        newClient.stop();
        return;
      }
    } else {                                   // New TCP client
      telnetClient = telnetServer.available();
    }
    if (!telnetClient) {                       // No client yet ??? (MS : pas certain que ce soit possible))
      return;
    }
    telnetClient.flush(); // clear input buffer, else you get strange characters
    //delay(100);
    while (telnetClient.available()) {  // Empty buffer in
      telnetClient.read();
    }
  } // end of hasClient()
}

void sendViaTelnet( char c) {
  if (telnetClient && telnetClient.connected()) {
    telnetClient.print(c) ;
  }
}

boolean telnetIsConnected() {
  return telnetClient && telnetClient.connected() ;
}

void clearTelnetBuffer() {
  while (telnetClient.available()) {  // Empty buffer in
      telnetClient.read();
  }
}

