/*
    Companion app communication module

    Authors:
    Jakub Aldorf
*/

#ifndef BLUETOOTH_HEAD
#define BLUETOOTH_HEAD

#include <Arduino.h>

class Bluetooth
{
public:
    void init();
    void readPacket();
    void sendPacket(uint8_t packetType, uint8_t *data, size_t dataLength);

private:
    const size_t BUFFER_SIZE = 64;
    const char version[7] = "0.3.0";

    uint8_t receiveBuffer[64];
    size_t bufferIndex = 0;
    void handlePacket(uint8_t packetType, uint8_t *data, size_t dataLength);
    uint8_t calculateCRC(uint8_t *data, size_t length);
    void debugPrint(const char *message);
    void debugPrintln(const char *message);
    void debugPrintf(const char *format, ...);
    void printHex(uint8_t *data, size_t length);
};

#endif