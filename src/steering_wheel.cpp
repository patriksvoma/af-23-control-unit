/*
 * Steering wheel module
 * Implements packet-based protocol communication with debug output
 */
#include <Arduino.h>
#include <motor.h>
#include <temperature.h>
#include <bms.h>
#include <shiftreg.h>


#define UART1_TX 8
#define UART1_RX 9
#define CON_PIN 15

// Packet protocol definitions
#define START_BYTE 0xAA
#define ACK_PACKET_TYPE 0x06
#define BATTERY_PACKET_TYPE 0x03
#define DASH_PACKET_TYPE 0x02
#define HELLO_PACKET_TYPE 0x00
#define SPEED_PACKET_TYPE 0x04
#define MODE_PACKET_TYPE 0x05
#define BREAK_PACKET_TYPE 0x07

// Debug control - set to false to disable debug messages
#define DEBUG_ENABLED true

namespace steeringWheel {
    // Buffer for receiving data
    const size_t BUFFER_SIZE = 64;
    uint8_t receiveBuffer[BUFFER_SIZE];
    size_t bufferIndex = 0;
    
    void init();
    bool testConnectionPin();
    bool testConnectionSerial();
    
    uint8_t calculateCRC(uint8_t* data, size_t length);
    void sendPacket(uint8_t packetType, uint8_t* data, size_t dataLength);
    bool readPacket();
    void handlePacket(uint8_t packetType, uint8_t* data, size_t dataLength, uint32_t timestamp);
    void sendAckPacket();
    void sendBatteryData();
    void sendDashboardData();
    void setSpeedMode(uint8_t mode);
    void setRideMode(uint8_t mode);
    void changeMotorBreak(uint8_t breakValue);


    void printHex(uint8_t* data, size_t length);
    
    // Debug print functions
    void debugPrint(const char* message) {
        if (DEBUG_ENABLED) {
            Serial.print("[SW] ");
            Serial.print(message);
        }
    }
    
    void debugPrintln(const char* message) {
        if (DEBUG_ENABLED) {
            Serial.print("[SW] ");
            Serial.println(message);
        }
    }
    
    void debugPrintf(const char* format, ...) {
        if (DEBUG_ENABLED) {
            char buffer[128];
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            
            Serial.print("[SW] ");
            Serial.print(buffer);
        }
    }
    
    /// @brief Initializes the steering wheel module
    void init()
    {
        pinMode(CON_PIN, INPUT);
        
        Serial2.setTX(UART1_TX);
        Serial2.setRX(UART1_RX);
        Serial2.setTimeout(50);
        Serial2.begin(115200);  // Changed to match Python example
        
        bufferIndex = 0;
        
        debugPrintln("Steering wheel module initialized");
    }
    
    /// @brief Checks if the steering wheel detection pin is LOW
    /// @return True if the wheel is connected (pin LOW)
    bool testConnectionPin()
    {
        bool connected = !digitalRead(CON_PIN);
        debugPrintf("Connection pin test: %s\n", connected ? "CONNECTED" : "DISCONNECTED");
        return connected;
    }
    
    /// @brief Sends a HELLO packet and waits for ACK response
    /// @return True if valid ACK is received
    bool testConnectionSerial()
    {
        debugPrintln("Testing serial connection with HELLO packet...");
        
        // Send a HELLO packet
        sendPacket(HELLO_PACKET_TYPE, NULL, 0);
        
        unsigned long startTime = millis();
        while (millis() - startTime < 200)  // 200ms timeout
        {
            if (readPacket())
            {
                debugPrintln("Serial connection test successful!");
                return true;
            }
            delay(1);
        }
        
        debugPrintln("Steering wheel connection check timed out!");
        return false;
    }
    
    /// @brief Helper function to print data in hex format
    /// @param data Pointer to data buffer
    /// @param length Length of data
    void printHex(uint8_t* data, size_t length) {
        if (!DEBUG_ENABLED) return;
        
        Serial.print("[SW] HEX: ");
        for (size_t i = 0; i < length; i++) {
            if (data[i] < 0x10) Serial.print("0");
            Serial.print(data[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
    
    /// @brief Calculate CRC8 (simple XOR-based CRC)
    /// @param data Pointer to data buffer
    /// @param length Length of data
    /// @return Calculated CRC value
    uint8_t calculateCRC(uint8_t* data, size_t length)
    {
        uint8_t crc = 0;
        for (size_t i = 0; i < length; i++)
        {
            crc ^= data[i];
        }
        debugPrintf("CRC calculated: 0x%02X\n", crc);
        return crc;
    }
    
    /// @brief Send a packet with specified type and data
    /// @param packetType Type of packet
    /// @param data Pointer to data buffer (can be NULL if dataLength is 0)
    /// @param dataLength Length of data
    void sendPacket(uint8_t packetType, uint8_t* data, size_t dataLength)
    {
        debugPrintf("Sending packet type: 0x%02X, data length: %d\n", packetType, dataLength);
        
        // Create packet: START_BYTE + packetType + timestamp(4 bytes) + dataLength + data + CRC
        uint8_t packet[BUFFER_SIZE];
        size_t packetIndex = 0;
        
        // Add start byte
        packet[packetIndex++] = START_BYTE;
        
        // Add packet type
        packet[packetIndex++] = packetType;
        
        // Add timestamp (4 bytes)
        uint32_t timestamp = millis();
        packet[packetIndex++] = (timestamp >> 24) & 0xFF;
        packet[packetIndex++] = (timestamp >> 16) & 0xFF;
        packet[packetIndex++] = (timestamp >> 8) & 0xFF;
        packet[packetIndex++] = timestamp & 0xFF;
        
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
        size_t bytesWritten = Serial2.write(packet, packetIndex);
        debugPrintf("Sent %d bytes\n", bytesWritten);
    }
    
    /// @brief Read and process incoming packets
    /// @return True if a valid packet was processed
    bool readPacket()
    {

        // Process any available bytes
        while (Serial2.available() > 0)
        {
            uint8_t byte = Serial2.read();
            
            // Debug incoming byte
            if (DEBUG_ENABLED && bufferIndex == 0) {
                debugPrintf("Received byte: 0x%02X\n", byte);
            }
            
            // If buffer is empty, check for start byte
            if (bufferIndex == 0 && byte != START_BYTE)
            {
                debugPrintf("Skipping non-start byte: 0x%02X\n", byte);
                continue;  // Skip until we find a start byte
            }
            
            // Add byte to buffer
            receiveBuffer[bufferIndex++] = byte;
            
            // Prevent buffer overflow
            if (bufferIndex >= BUFFER_SIZE)
            {
                debugPrintln("Buffer overflow! Resetting buffer.");
                bufferIndex = 0;
                return false;
            }
            
            // Minimum packet size: start(1) + type(1) + timestamp(4) + length(1) + crc(1) = 8 bytes
            if (bufferIndex < 8)
            {
                continue;  // Need more bytes for a complete packet
            }
            
            // Check if we have complete packet
            uint8_t dataLength = receiveBuffer[6];
            size_t expectedPacketLength = 8 + dataLength;
            
            if (bufferIndex < expectedPacketLength)
            {
                debugPrintf("Partial packet: %d/%d bytes\n", bufferIndex, expectedPacketLength);
                continue;  // Need more bytes for a complete packet
            }
            
            debugPrintln("Received complete packet:");
            printHex(receiveBuffer, bufferIndex);
            
            // Validate CRC
            uint8_t receivedCRC = receiveBuffer[bufferIndex - 1];
            uint8_t calculatedCRC = calculateCRC(receiveBuffer, bufferIndex - 1);
            
            // Extract timestamp
            uint32_t timestamp = ((uint32_t)receiveBuffer[2] << 24) |
                                ((uint32_t)receiveBuffer[3] << 16) |
                                ((uint32_t)receiveBuffer[4] << 8) |
                                (uint32_t)receiveBuffer[5];
            
            debugPrintf("Packet timestamp: %lu\n", timestamp);
            debugPrintf("CRC check: received=0x%02X, calculated=0x%02X\n", receivedCRC, calculatedCRC);
            
            // Reset buffer index
            bufferIndex = 0;
            
            if (receivedCRC == calculatedCRC)
            {
                // Extract packet components
                uint8_t packetType = receiveBuffer[1];
                uint8_t* data = &receiveBuffer[7];
                
                debugPrintf("Valid packet received, type: 0x%02X, data length: %d\n", packetType, dataLength);
                
                // Handle the packet
                handlePacket(packetType, data, dataLength, timestamp);
                return true;
            }
            else
            {
                debugPrintln("CRC mismatch, discarding packet.");
                return false;
            }
        }
        
        return false;
    }
    
    /// @brief Handle different packet types
    /// @param packetType Type of packet
    /// @param data Pointer to data buffer
    /// @param dataLength Length of data
    /// @param timestamp Timestamp from packet
    void handlePacket(uint8_t packetType, uint8_t* data, size_t dataLength, uint32_t timestamp)
    {
        switch (packetType)
        {
            case ACK_PACKET_TYPE:
                debugPrintln("Received ACK packet");
                break;
                
            case HELLO_PACKET_TYPE:
                debugPrintln("Received HELLO packet, sending ACK");
                sendAckPacket();
                break;
                
            case BATTERY_PACKET_TYPE:
                debugPrintln("Received request for battery data");
                sendBatteryData();
                break;
                
            case DASH_PACKET_TYPE:
                debugPrintln("Received request for dashboard data");
                sendDashboardData();
                break;
            case SPEED_PACKET_TYPE:
                debugPrintln("Received command for speed");
                setSpeedMode(data[0]);
                break;
            case MODE_PACKET_TYPE:
                debugPrintln("Received command for ride mode");
                setRideMode(data[0]);
                break;
            case BREAK_PACKET_TYPE:
                debugPrintln("Received command for changing motor break");
                changeMotorBreak(data[0]);
                break;
            default:
                debugPrintf("Unknown packet type: 0x%02X\n", packetType);
                break;
        }
    }

    void changeMotorBreak(uint8_t breakValue){
        if(breakValue == 10)//plus
        {
            if(motor::getMotorBrake()<= 223){
            motor::setMotorBrake(motor::getMotorBrake()+32);

            }
        }
        else if (breakValue ==20)//minus
        {
            if(motor::getMotorBrake()>= 32){
            motor::setMotorBrake(motor::getMotorBrake()-32);

            }
        }  
    }
    
    /// @brief Send an ACK response
    void sendAckPacket()
    {
        debugPrintln("Sending ACK packet");
        sendPacket(ACK_PACKET_TYPE, NULL, 0);
    }
    
    /// @brief Send battery data response
    void sendBatteryData()
    {
        debugPrintln("Sending battery data");
        
        // Example battery data (8 cells with 3 values each)
        uint8_t batteryData[24] = {
            80, 37, 40,  // Cell 1: voltage, temperature, status
            88, 37, 24,  // Cell 2
            87, 38, 24,  // Cell 3
            20, 35, 24,  // Cell 4
            5, 37, 24,   // Cell 5
            99, 40, 24,  // Cell 6
            100, 40, 24, // Cell 7
            45, 45, 24   // Cell 8
        };
        
        sendPacket(BATTERY_PACKET_TYPE, batteryData, sizeof(batteryData));
    }
      uint8_t encodeDigits(uint8_t high, uint8_t low) {
    return ((high & 0x0F) << 4) | (low & 0x0F);
}
    /// @brief Send dashboard data response
    void sendDashboardData()
    {
        debugPrintln("Sending dashboard data");
        
        uint8_t dashData[7];
        
        // RPM (2 bytes, big endian)
        uint32_t rpm = motor::getRPM();
        bms::sendReceiveBasicInfo();

        dashData[0] = (rpm >> 8) & 0xFF;
        dashData[1] = rpm & 0xFF;
        
        

        uint32_t temp = floor( (temperature::readTemperature(0)*100));
        dashData[2] = (temp >> 8) & 0xFF;;
        dashData[3] = temp & 0xFF;

        debugPrintf("STempeature:%d",temp );
    
        dashData[4] = bms::basicInfo.rsoc;
        
        // Status flags (1 byte)
        dashData[5] = encodeDigits(shiftreg::get_speed_mode(),shiftreg::get_ride_mode());
        dashData[6] = motor::getMotorBrake()/32;
        
        sendPacket(DASH_PACKET_TYPE, dashData, sizeof(dashData));
    }
  

    void setSpeedMode(uint8_t mode){
        debugPrintf("SpeedModeReceived:%d",mode );

        if(mode ==0){
            shiftreg::set_motor_hi_speed(false);
            shiftreg::set_motor_lo_speed(true);
        }
        else if(mode==1){
            shiftreg::set_motor_lo_speed(false);
            shiftreg::set_motor_hi_speed(false);

        }
        else if (mode ==2){
            shiftreg::set_motor_lo_speed(false);
            shiftreg::set_motor_hi_speed(true);
        }
    }

    void setRideMode(uint8_t mode){
        debugPrintf("SpeedModeReceived:%d",mode );

        if(mode ==0){
                            shiftreg::set_motor_pwr(true);
                            shiftreg::set_motor_reverse(false);

        }
        else if(mode==1){
            shiftreg::set_motor_pwr(false);


        }
        else if (mode ==2){
         shiftreg::set_motor_pwr(true);
                            shiftreg::set_motor_reverse(true);
        }
    }
    
    /// @brief Process steering wheel communications
    /// @return True if any packet was processed
    bool process()
    {
        return readPacket();
    }
}