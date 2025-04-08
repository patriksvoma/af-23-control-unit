/*
    BMS communication module

    Communicates with JBD-DP24S002 using UART

    Protocol: https://cdn.shopifycdn.net/s/files/1/0606/5199/5298/files/JDB_RS485-RS232-UART-Bluetooth-Communication_Protocol.pdf?v=1682577935
*/





#include "bluetooth.h"
      #include "bms.h"
#include "ota.h"
#include <SerialBT.h>


#define DEBUG_ENABLED true

#define START_BYTE 0xAA
#define BATTERY_PACKET 0x03
#define OTA_PACKET 0x04





    
    void Bluetooth::init(){

   Serial.println("Starting bluetooth");

    SerialBT.setName("AF-23");
    SerialBT.setTimeout(10);
    SerialBT.begin();

    Serial.println("Bluetooth started, waiting for connection. Starting modules.");
    }

    void Bluetooth::debugPrint(const char *message)
    {
        if (DEBUG_ENABLED)
        {
            Serial.print("[BT] ");
            Serial.print(message);
        }
    }

    void Bluetooth::debugPrintln(const char *message)
    {
        if (DEBUG_ENABLED)
        {
            Serial.print("[BT] ");
            Serial.println(message);
        }
    }

     void Bluetooth::debugPrintf(const char* format, ...) {
        if (DEBUG_ENABLED) {
            char buffer[128];
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            
            Serial.print("[BT] ");
            Serial.print(buffer);
        }
    }

    /// @brief Helper function to print data in hex format
    /// @param data Pointer to data buffer
    /// @param length Length of data
    void Bluetooth::printHex(uint8_t *data, size_t length)
    {
        if (!DEBUG_ENABLED)
            return;

        Serial.print("[BT] HEX: ");
        for (size_t i = 0; i < length; i++)
        {
            if (data[i] < 0x10)
                Serial.print("0");
            Serial.print(data[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }

      uint8_t Bluetooth::calculateCRC(uint8_t* data, size_t length)
    {
        uint8_t crc = 0;
        for (size_t i = 0; i < length; i++)
        {
            crc ^= data[i];
        }
        debugPrintf("CRC calculated: 0x%02X\n", crc);
        return crc;
    }


      void Bluetooth::handlePacket(uint8_t packetType, uint8_t* data, size_t dataLength)
    {
        switch (packetType)
        {
            case 0x02:
                debugPrintln("Received ACK packet");
                break;
            case BATTERY_PACKET:
                debugPrintln("Received request for battery");
                sendPacket(BATTERY_PACKET,&bms::basicInfo.rsoc,1);
                break;
            case OTA_PACKET:
                ota::enter();
                break;
            default:
                debugPrintf("Unknown packet type: 0x%02X\n", packetType);
                break;
        }
    }

     /// @brief Send a packet with specified type and data
    /// @param packetType Type of packet
    /// @param data Pointer to data buffer (can be NULL if dataLength is 0)
    /// @param dataLength Length of data
    void Bluetooth::sendPacket(uint8_t packetType, uint8_t* data, size_t dataLength)
    {
        debugPrintf("Sending packet type: 0x%02X, data length: %d\n", packetType, dataLength);
        
        // Create packet: START_BYTE + packetType + dataLength + data + CRC
        uint8_t packet[BUFFER_SIZE];
        size_t packetIndex = 0;
        
        // Add start byte
        packet[packetIndex++] = START_BYTE;
        
        // Add packet type
        packet[packetIndex++] = packetType;
        
        
        
        // Add data length
        packet[packetIndex++] = dataLength;
        
        // Add data
        if (data != NULL && dataLength > 0)
        {
            for (size_t i = 0; i < dataLength; i++)
            {
                packet[packetIndex++] = data[i];
            }
        }
        
        // Calculate CRC for all bytes except CRC itself
        uint8_t crc = calculateCRC(packet, packetIndex);
        
        // Add CRC
        packet[packetIndex++] = crc;
        
        // Print packet in hex
        debugPrintln("Packet data:");
        printHex(packet, packetIndex);
        
        // Send packet
        size_t bytesWritten = SerialBT.write(packet, packetIndex);
        debugPrintf("Sent %d bytes\n", bytesWritten);
    }


    void Bluetooth::readPacket()
    {
        if (SerialBT.availableForWrite())
        {
            while (SerialBT.available() > 0)
            {
                uint8_t byte = SerialBT.read();

                // Debug incoming byte
                if (DEBUG_ENABLED && bufferIndex == 0)
                {
                    debugPrintf("Received byte: 0x%02X\n", byte);
                }

                // If buffer is empty, check for start byte
                if (bufferIndex == 0 && byte != START_BYTE)
                {
                    debugPrintf("Skipping non-start byte: 0x%02X\n", byte);
                    continue; // Skip until we find a start byte
                }

                // Add byte to buffer
                receiveBuffer[bufferIndex++] = byte;

                // Prevent buffer overflow
                if (bufferIndex >= BUFFER_SIZE)
                {
                    debugPrintln("Buffer overflow! Resetting buffer.");
                    bufferIndex = 0;
                    return;
                }

                // Minimum packet size: start(1) + type(1)  + length(1) + crc(1) = 8 bytes
                if (bufferIndex < 4)
                {
                    continue; // Need more bytes for a complete packet
                }

                // Check if we have complete packet
                uint8_t dataLength = receiveBuffer[2];
                size_t expectedPacketLength = 4 + dataLength;

                if (bufferIndex < expectedPacketLength)
                {
                    debugPrintf("Partial packet: %d/%d bytes\n", bufferIndex, expectedPacketLength);
                    continue; // Need more bytes for a complete packet
                }

                debugPrintln("Received complete packet:");
                printHex(receiveBuffer, bufferIndex);

                // Validate CRC
                uint8_t receivedCRC = receiveBuffer[bufferIndex - 1];
                uint8_t calculatedCRC = calculateCRC(receiveBuffer, bufferIndex - 1);

            

   
                debugPrintf("CRC check: received=0x%02X, calculated=0x%02X\n", receivedCRC, calculatedCRC);

                // Reset buffer index
                bufferIndex = 0;

                if (receivedCRC == calculatedCRC)
                {
                    // Extract packet components
                    uint8_t packetType = receiveBuffer[1];
                    uint8_t *data = &receiveBuffer[3];

                    debugPrintf("Valid packet received, type: 0x%02X, data length: %d\n", packetType, dataLength);

                    // Handle the packet
                    handlePacket(packetType, data, dataLength);
                    return ;
                }
                else
                {
                    debugPrintln("CRC mismatch, discarding packet.");
                    return ;
                }
            }
        }
    }

