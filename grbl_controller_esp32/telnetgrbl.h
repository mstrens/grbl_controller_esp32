#ifndef TELNETGRBL_H
#define TELNETGRBL_H

bool telnetGrblInit() ;
void telnetGrblStop() ;

void toTelnet(char c);
void toTelnet(const char * data) ;

bool fromTelnetAvailable();
int fromTelnetRead();

//void wifiGrblSend();

//void wifiGrblRead() ;

//void testWifiGrbl() ;

#endif

