/*
    Balancer communication module

    Communicates with JBD-DP24S002 using UART

    Protocol: https://cdn.shopifycdn.net/s/files/1/0606/5199/5298/files/JDB_RS485-RS232-UART-Bluetooth-Communication_Protocol.pdf?v=1682577935
*/

#include <Arduino.h>

#define UART0_TX 0
#define UART0_RX 1

namespace balancer
{
    void init();
    void sendReceiveBasicInfo();
    uint16_t calculateChecksum(uint8_t commandCode, uint8_t dataLength, char* dataContent);

    struct BasicInfo
    {
        uint16_t totalVoltage;
        uint16_t current;
        uint16_t remainingCapacity;
        uint16_t nominalCapacity;
        uint16_t cycles;
        uint16_t productionDate;
        uint16_t equilibrium;
        uint16_t equilibriumHigh;
        uint16_t protectionStatus;
        uint8_t softwareVersion;
        uint8_t rsoc;
        uint8_t fetControl;
        uint8_t batterStringCount;
        uint8_t ntcCount; // Not sure how this works yet!
        uint64_t ntcContent; // Not sure how this works yet!
    };

    /// @brief Initializes the balancer module
    void init()
    {
        Serial1.setTX(UART0_TX);
        Serial1.setRX(UART0_RX);
        Serial1.setTimeout(50);
        Serial1.begin(9600, SERIAL_8N1);
   
        // TODO: Remove
        sendReceiveBasicInfo();
    }

    /// @brief Sends the "Basic info" command and receives the response
    void sendReceiveBasicInfo()
    {
        // Set all parts of the command
        uint8_t startByte = 0xDD;
        uint8_t stateByte = 0xA5;
        uint8_t commandCode = 0x03;
        uint8_t dataLength = 0;
        char* dataContent = new char[dataLength];
        uint16_t checksum;
        uint8_t stopByte = 0x77;

        // Calculate the checksum
        checksum = calculateChecksum(commandCode, dataLength, dataContent);

        // Write out the command
        Serial1.write(startByte);
        Serial1.write(stateByte);
        Serial1.write(commandCode);
        Serial1.write(dataLength);
        Serial1.write(dataContent, dataLength);
        Serial1.write(checksum >> 8);
        Serial1.write(checksum);
        Serial1.write(stopByte);
        Serial1.flush();

        // Remove the data content from the heap, since it will not be used anymore
        delete[] dataContent;

        while (Serial1.available() == 0) {};

        // Read the response
        if (Serial1.available() > 0)
        {
            // Create variables where parts of the received data will be stored
            uint8_t recStartByte;
            uint8_t recStateByte;
            uint8_t recStatusByte;
            uint8_t recDataLength;
            char* recDataContent;
            uint16_t recChecksum;
            uint8_t recStopByte;

            // Read the data
            recStartByte = Serial1.read();
            recStateByte = Serial1.read();
            recStatusByte = Serial1.read();
            recDataLength = Serial1.read();
            recDataContent = new char[recDataLength];
            Serial1.readBytes(recDataContent, recDataLength);

            char* tempChecksum = new char[2];
            Serial1.readBytes(tempChecksum, 2);
            recChecksum = ((uint16_t)(tempChecksum[0]) << 8) + (uint16_t)(tempChecksum[1]);
            delete[] tempChecksum;

            recStopByte = Serial1.read();

            // Print out the data

            Serial.println(recStartByte, HEX);
            Serial.println(recStateByte, HEX);
            Serial.println(recStatusByte, HEX);
            Serial.println(recDataLength, HEX);
            for (int i = 0; i < recDataLength; i++) Serial.print(recDataContent[i], HEX);
            Serial.println("");
            Serial.println(recChecksum, HEX);
            Serial.println(recStopByte, HEX);

            Serial.println("BMS data received");
            if (calculateChecksum(recStatusByte, recDataLength, recDataContent) == recChecksum)
            {
                Serial.println("Checksum matches");
            }
            else
            {
                Serial.println("Checksum does not match");
            }
            
        }
    }

    /// @brief Calculates the checksum of a command
    /// @param commandCode Command code / Status byte
    /// @param dataLength Number of data bytes
    /// @param dataContent Data buffer
    /// @return Checksum
    uint16_t calculateChecksum(uint8_t commandCode, uint8_t dataLength, char* dataContent)
    {
        uint16_t sum;
        for (int i = 0; i < dataLength; i++) sum += dataContent[i];
        sum += dataLength;
        sum += commandCode;

        uint16_t checksum = 0xFFFF - sum + 1;

        return checksum;
    }
}