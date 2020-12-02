#pragma once
#include <Arduino.h>
#include <WiFi.h>

class GrblStream
{
public:
    virtual int available() = 0;
    virtual char read() = 0;
    virtual bool canSend(size_t n) = 0;
    virtual void waitSent() = 0;
    virtual size_t print(char c) = 0;
    virtual size_t print(const char* str) = 0;
    size_t print(float n) { return print(String(n).c_str()); }
    size_t print(uint32_t n) { return print(String(n).c_str()); }
    size_t println(const char* str) { size_t n = print(str); return n + print('\n'); } ;

    // You need a virtual destructor for 'delete' to work correctly:
    virtual ~GrblStream() {}
};

class SerialGrblStream : public GrblStream
{
    HardwareSerial _serial;
public:
SerialGrblStream(HardwareSerial serial) : _serial(serial) {}
    int available() override { return _serial.available(); }
    bool canSend(size_t n) override { return _serial.availableForWrite() > n; }
    void waitSent() override { while (_serial.availableForWrite() != 0x7F) ;  }
    char read() override { return _serial.read(); }
    size_t print(char c) override { return _serial.print(c); }
    size_t print(const char* str) override { return _serial.print(str); }
};

class TCPGrblStream : public GrblStream
{
    WiFiClient _netconn;
public:
    TCPGrblStream(const char *host, uint16_t port, int timeout) {
        if (_netconn.connect(host, port, timeout) == 0) {
            throw -2;
        }
        _netconn.setNoDelay(true);
    }
    ~TCPGrblStream() {
        _netconn.stop();
    };
    int available() override { return _netconn.available(); }
    char read() override { return _netconn.read(); }
    bool canSend(size_t n) override { return true; }
    void waitSent() override { }
    size_t print(char c) override { return _netconn.write((uint8_t)c); }
    size_t print(const char* str) override { return _netconn.write((uint8_t *)str, strlen(str)); }
};

extern GrblStream* grblStream;
