/*
    BMS communication module

    Communicates with JBD-DP24S002 using UART

    Protocol: https://cdn.shopifycdn.net/s/files/1/0606/5199/5298/files/JDB_RS485-RS232-UART-Bluetooth-Communication_Protocol.pdf?v=1682577935
*/

#include <Arduino.h>

#define UART0_TX 0
#define UART0_RX 1

namespace bms
{
    void init();
    void sendReceiveBasicInfo();
    uint16_t calculateChecksum(uint8_t commandCode, uint8_t dataLength, char* dataContent);
    uint16_t charToUInt16(char* buffer, uint8_t startIndex);

    struct BasicInfo
    {
        public:
        uint16_t totalVoltage; // Total voltage of the battery pack. Unit: 1 mV
        uint16_t current; // Current current Unit: 1 mA
        uint16_t remainingCapacity; // Remaining capacity of the battery pack. Unit: 1 mAh
        uint16_t nominalCapacity; // Nominal capacity of the battery pack. Unit: 1 mAh
        uint16_t cycles; // Number of completed cycles (1 cycle is a complete charge and discharge)
        uint8_t productionDateDay; // Day the BMS was produced
        uint8_t productionDateMonth; // Month the BMS was produced
        uint16_t productionDateYear; // Year the BMS was produced
        uint16_t equilibrium; // Each bit represents if a battery string is being balanced
        uint16_t equilibriumHigh; // Each bit represents if a battery string is being balanced
        uint16_t protectionStatus; // TODO
        uint8_t softwareVersion; // 0x10 -> v1.0
        uint8_t rsoc; // Percentage of remaining battery pack capacity
        uint8_t fetControl; // Status of the charge (bit 0) and discharge (bit 1) MOSFETs. 0 means off, 1 means on
        uint8_t batteryStringCount; // Number of batteries in the pack
        uint8_t temperatureSensorCount; // Number of temperature sensors in the battery pack
        uint8_t* temperatures; // Array with temperatures from each sensor. Unit: 0.1 °C

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

            uint16_t productionDate = charToUInt16(dataContent, currentByte);
            productionDateDay = productionDate & 0x1F;
            productionDateMonth = (productionDate >> 5) & 0x0F;
            productionDateYear = 2000 + (productionDate >> 9);
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

            temperatureSensorCount = (uint8_t)dataContent[currentByte];
            currentByte++;

            delete[] temperatures;
            temperatures = new uint8_t[temperatureSensorCount];
            for (int i = 0; i < temperatureSensorCount; i++)
            {
                uint16_t tempKelvin = charToUInt16(dataContent, currentByte);
                temperatures[i] = tempKelvin - 2731;
                currentByte += 2;
            }
        }

        void printOut()
        {
            Serial.println("Total voltage: " + String(totalVoltage) + " mV");
            Serial.println("Current: " + String(current) + " mA");
            Serial.println("Remaining capacity: " + String(remainingCapacity) + " mAh");
            Serial.println("Nominal capacity: " + String(nominalCapacity) + " mAh");
            Serial.println("Completed cycles: " + String(cycles));
            Serial.println("Production date: " + String(productionDateYear) + "/" + String(productionDateMonth) + "/" + String(productionDateDay));
            Serial.println("Equilibrium 0: " + String(equilibrium, BIN));
            Serial.println("Equilibrium 1: " + String(equilibriumHigh, BIN));
            Serial.println("Protection status: " + String(protectionStatus, BIN));
            Serial.println("Software version: " + String(softwareVersion, HEX));
            Serial.println("Relative state of charge: " + String(rsoc) + " %");
            Serial.println("MOSFET status: " + String(fetControl, BIN));
            Serial.println("Battery count: " + String(batteryStringCount));
            Serial.println("Temperature sensor count: " + String(temperatureSensorCount));
            
            for (int i = 0; i < temperatureSensorCount; i++)
            {
                Serial.println("Temperature " + String(i) + ": " + String(temperatures[i]) + " * 0.1°C");
            }
        }
    };

    BasicInfo basicInfo = BasicInfo();

    /// @brief Initializes the balancer module
    void init()
    {
        Serial1.setTX(UART0_TX);
        Serial1.setRX(UART0_RX);
        Serial1.setInvertTX(true);
        Serial1.setInvertRX(true);
        Serial1.setTimeout(50);
        Serial1.begin(9600, SERIAL_8N1);
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

        //
        //  Receiving
        //

        // Create a buffer for the received data. 34 bytes of data are expected for the whole message
        const uint16_t expectedDataLength = 34; // TODO: Change for the real data
        char* receivedData = new char[expectedDataLength];

        // Read all bytes into an array, prevent the serial buffer from overflowing when receiving more than 32 bytes
        // TODO: A timeout should be here
        for (int i = 0; i < expectedDataLength; i++)
        {
            while (Serial1.available() == 0) {};
            receivedData[i] = Serial1.read();
        }

        // Check if the start and stop bits are correct
        if (receivedData[0] != 0xDD || receivedData[expectedDataLength - 1] != 0x77)
        {
            Serial.println("Start or stop bit is not correct, BMS data invalid.");
            return;
        }

        // Separate the data into variables
        uint8_t recStartByte = receivedData[0];
        uint8_t recStateByte = receivedData[1];
        uint8_t recStatusByte = receivedData[2];
        uint8_t recDataLength = receivedData[3];
        char* recDataContent = new char[recDataLength];
        uint16_t recChecksum;
        uint8_t recStopByte = receivedData[33];

        memcpy(&recDataContent[0], &receivedData[4], expectedDataLength - 7);
        recChecksum = charToUInt16(receivedData, 31);

        delete[] receivedData;

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

        Serial.println("Processing data");
        basicInfo.process(recDataContent);
        delete[] recDataContent;

        basicInfo.printOut();
        Serial.println("Data processed\n\n\n");
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