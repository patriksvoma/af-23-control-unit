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
    uint16_t charToUInt16(char* buffer, uint8_t startIndex);

    struct BasicInfo
    {
        public:
        uint16_t totalVoltage = 0; // Total voltage of the battery pack. Unit: 1 mV
        uint16_t current; // Current current?? Unit: 1 mA
        uint16_t remainingCapacity; // Remaining capacity of the battery pack. Unit: 1 mAh
        uint16_t nominalCapacity; // Nominal capacity of the battery pack. Unit: 1 mAh
        uint16_t cycles; // Number of completed cycles (1 cycle is a complete charge and discharge)
        uint16_t productionDate; // TODO
        uint16_t equilibrium; // TODO
        uint16_t equilibriumHigh; // TODO
        uint16_t protectionStatus; // TODO
        uint8_t softwareVersion; // TODO
        uint8_t rsoc; // Percentage of remaining battery pack capacity
        uint8_t fetControl; // Status of the charge (bit 0) and discharge (bit 1) MOSFETs. 0 means off, 1 means on
        uint8_t batteryStringCount; // Number of batteries in the pack
        uint8_t ntcCount; // TODO: Number of temperature sensors in the pack
        uint64_t ntcContent; // Not sure how this works yet!

        public:

        /// @brief Processes raw data content received from the BMS. Converts the units of certain values.
        /// @param dataContent Pointer to the data
        void process(char* dataContent)
        {
            uint8_t currentByte = 0;

            totalVoltage = charToUInt16(dataContent, currentByte) * 10;
            currentByte += 2;

            current = charToUInt16(dataContent, currentByte) * 10;
            currentByte += 2;

            remainingCapacity = charToUInt16(dataContent, currentByte) * 10;
            currentByte += 2;

            nominalCapacity = charToUInt16(dataContent, currentByte) * 10;
            currentByte += 2;

            cycles = charToUInt16(dataContent, currentByte);
            currentByte += 2;

            productionDate = charToUInt16(dataContent, currentByte);
            currentByte += 2;

            equilibrium = charToUInt16(dataContent, currentByte);
            currentByte += 2;

            equilibriumHigh = charToUInt16(dataContent, currentByte);
            currentByte += 2;

            protectionStatus = charToUInt16(dataContent, currentByte);
            currentByte += 2;

            softwareVersion = (uint8_t)dataContent[currentByte];
            currentByte++;

            rsoc = (uint8_t)dataContent[currentByte];
            currentByte++;

            fetControl = (uint8_t)dataContent[currentByte];
            currentByte++;

            batteryStringCount = (uint8_t)dataContent[currentByte];
            currentByte++;

            ntcCount = (uint8_t)dataContent[currentByte];
            currentByte++;
        }

        void printOut()
        {
            Serial.println("Total voltage: " + String(totalVoltage) + " mV");
            Serial.println("Current: " + String(current) + " mA");
            Serial.println("Remaining capacity: " + String(remainingCapacity) + " mAh");
            Serial.println("Nominal capacity: " + String(nominalCapacity) + " mAh");
            Serial.println("Completed cycles: " + String(cycles));
            Serial.println("Production date: " + String(productionDate, HEX));
            Serial.println("Equilibrium 0: " + String(equilibrium, BIN));
            Serial.println("Equilibrium 1: " + String(equilibriumHigh, BIN));
            Serial.println("Protection status: " + String(protectionStatus, BIN));
            Serial.println("Software version: " + String(softwareVersion, HEX));
            Serial.println("Relative state of charge: " + String(rsoc) + " %");
            Serial.println("MOSFET status: " + String(fetControl, BIN));
            Serial.println("Battery count: " + String(batteryStringCount));
            Serial.println("Temperature sensor count: " + String(ntcCount));
            // TODO: ntc content
        }
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

            // The checksum needs to be read as an array and then converted to a uint16_t
            char* tempChecksum = new char[2];
            Serial1.readBytes(tempChecksum, 2);
            recChecksum = charToUInt16(tempChecksum, 0);
            delete[] tempChecksum;

            recStopByte = Serial1.read();

            // Check if the checksum matches
            if (calculateChecksum(recStatusByte, recDataLength, recDataContent) == recChecksum)
            {
                Serial.println("Checksum matches");
            }
            else
            {
                Serial.println("Checksum does not match");
                return;
            }

            // Print out the data
            // TODO: Remove
            Serial.println(recStartByte, HEX);
            Serial.println(recStateByte, HEX);
            Serial.println(recStatusByte, HEX);
            Serial.println(recDataLength, HEX);
            for (int i = 0; i < recDataLength; i++) Serial.print(recDataContent[i], HEX);
            Serial.println("");
            Serial.println(recChecksum, HEX);
            Serial.println(recStopByte, HEX);

            Serial.println("BMS data received");
            
            // Read the values from the data content
            BasicInfo basicInfo;
            basicInfo.process(recDataContent);
            basicInfo.printOut();

            delete[] recDataContent;
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

    /// @brief Reads a uint16_t from a buffer
    /// @param buffer Pointer to a char array
    /// @param startIndex Index of the first byte
    /// @return 16-bit unsigned int
    uint16_t charToUInt16(char* buffer, uint8_t startIndex)
    {
        return ((uint16_t)(buffer[startIndex]) << 8) + (uint16_t)(buffer[startIndex + 1]);
    }
}