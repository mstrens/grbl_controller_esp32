#ifndef TELNET_H
#define TELNET_H


void telnetInit() ;

void checkTelnetConnection();

boolean telnetIsConnected() ;

void sendViaTelnet(char c) ;

void clearTelnetBuffer() ;
#endif

