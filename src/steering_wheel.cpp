/*
 * Steering wheel module
 * Implements packet-based protocol communication with debug output
 */
#include <Arduino.h>
#include <motor.h>
#include <temperature.h>
#include <bms.h>
#include <shiftreg.h>
#include <rideStats.h>
#include "spoiler_led.h"

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
#define TEMP_PACKET_TYPE 0x08
#define RIDE_STATS_PACKET_TYPE 0x09
#define LED_SPOILER_PACKET_TYPE 0x10

// Debug control 
#define DEBUG_ENABLED true

extern RideStats rideStats;
namespace steeringWheel
{
    // Buffer for receiving data
    const size_t BUFFER_SIZE = 64;
    uint8_t receiveBuffer[BUFFER_SIZE];
    size_t bufferIndex = 0;

    void init();
    bool testConnectionPin();
    bool testConnectionSerial();

    uint8_t calculateCRC(uint8_t *data, size_t length);
    void sendPacket(uint8_t packetType, uint8_t *data, size_t dataLength);
    bool readPacket();
    void handlePacket(uint8_t packetType, uint8_t *data, size_t dataLength, uint32_t timestamp);
    void sendAckPacket();
    void sendBatteryData();
    void sendDashboardData();
    void sendRideStatsData();
    void sendTempData();
    void setSpeedMode(uint8_t mode);
    void setRideMode(uint8_t mode);
    void changeMotorBreak(uint8_t breakValue);

    void printHex(uint8_t *data, size_t length);

    // Debug print functions
    void debugPrint(const char *message)
    {
        if (DEBUG_ENABLED)
        {
            Serial.print("[SW] ");
            Serial.print(message);
        }
    }

    void debugPrintln(const char *message)
    {
        if (DEBUG_ENABLED)
        {
            Serial.print("[SW] ");
            Serial.println(message);
        }
    }

    void debugPrintf(const char *format, ...)
    {
        if (DEBUG_ENABLED)
        {
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
        Serial2.begin(115200); // Changed to match Python example

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
        while (millis() - startTime < 200) // 200ms timeout
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
    void printHex(uint8_t *data, size_t length)
    {
        if (!DEBUG_ENABLED)
            return;

        Serial.print("[SW] HEX: ");
        for (size_t i = 0; i < length; i++)
        {
            if (data[i] < 0x10)
                Serial.print("0");
            Serial.print(data[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }

    /// @brief Calculate CRC8 (simple XOR-based CRC)
    /// @param data Pointer to data buffer
    /// @param length Length of data
    /// @return Calculated CRC value
    uint8_t calculateCRC(uint8_t *data, size_t length)
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
    void sendPacket(uint8_t packetType, uint8_t *data, size_t dataLength)
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
                return false;
            }

            // Minimum packet size: start(1) + type(1) + timestamp(4) + length(1) + crc(1) = 8 bytes
            if (bufferIndex < 8)
            {
                continue; // Need more bytes for a complete packet
            }

            // Check if we have complete packet
            uint8_t dataLength = receiveBuffer[6];
            size_t expectedPacketLength = 8 + dataLength;

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
                uint8_t *data = &receiveBuffer[7];

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
    void handlePacket(uint8_t packetType, uint8_t *data, size_t dataLength, uint32_t timestamp)
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
        case TEMP_PACKET_TYPE:
        debugPrintln("Received request for temperature data");
            sendTempData();
            break;
        case RIDE_STATS_PACKET_TYPE:
        debugPrintln("Received request for ride stats data");
        sendRideStatsData();
        break;

        case LED_SPOILER_PACKET_TYPE:
            debugPrintln("Received command for spoiler animation");
            if(data[0]==10){
                //...
                spoilerLed::setAnimation(8);
            }
            else if(data[0]==20){
                spoilerLed::setAnimation(9);

            }
            else if(data[0]==30){
                spoilerLed::setAnimation(10);

            }
            break;
            
        default:
            debugPrintf("Unknown packet type: 0x%02X\n", packetType);
            break;
        }
    }

    void changeMotorBreak(uint8_t breakValue)
    {
        if (breakValue == 10) // plus
        {
            if (motor::getMotorBrake() <= 223)
            {
                motor::setMotorBrake(motor::getMotorBrake() + 32);
            }
        }
        else if (breakValue == 20) // minus
        {
            if (motor::getMotorBrake() >= 32)
            {
                motor::setMotorBrake(motor::getMotorBrake() - 32);
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
        bms::sendReceiveCellVoltage();
        bms::sendReceiveBasicInfo();

        bms::cellVoltages.voltages;
        bms::basicInfo.totalVoltage;
        bms::basicInfo.rsoc;
        bms::basicInfo.temperatures[1];
        bms::basicInfo.temperatures[2];

        // 16 cell voltages (2 bytes each) + total voltage (2 bytes) + rsoc (1 byte) + 2 temps (2 bytes each) = 39 bytes
        uint8_t batteryData[40];
        bms::populateBatteryData(batteryData);

        sendPacket(BATTERY_PACKET_TYPE, batteryData, sizeof(batteryData));
    }
    uint8_t encodeDigits(uint8_t high, uint8_t low)
    {
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

        uint32_t temp = floor((temperature::readTemperature(0) * 100));
        dashData[2] = (temp >> 8) & 0xFF;
        ;
        dashData[3] = temp & 0xFF;

        debugPrintf("STempeature:%d", temp);

        dashData[4] = bms::basicInfo.rsoc;

        // Status flags (1 byte)
        dashData[5] = encodeDigits(shiftreg::get_speed_mode(), shiftreg::get_ride_mode());
        dashData[6] = motor::getMotorBrake() / 32;

        sendPacket(DASH_PACKET_TYPE, dashData, sizeof(dashData));
    }

    void sendRideStatsData() {
        debugPrintln("Preparing to send ride stats data");
        
        // Debug print all values before sending
        debugPrintf("Current stats - Trip: %.2fm, Total: %.2fm, Max RPM: %d, Max Accel: %.2fm/s²\n",
                   rideStats.distance_driven_trip_m,
                   rideStats.distance_driven_total_m,
                   rideStats.max_rpm,
                   rideStats.max_accel_mps2);
        
        debugPrintln("Best acceleration times:");
        for (int i = 0; i < 5; i++) {
            debugPrintf("  #%d: %.2fs\n", i+1, rideStats.best_accel_times[i]);
        }
    
        uint8_t rideStatsData[36];
        uint8_t* ptr = rideStatsData;
        
        // Pack distance_driven_trip_m (4 bytes)
        debugPrintln("Packing trip distance...");
        float tripDistance = rideStats.distance_driven_trip_m;
        memcpy(ptr, &tripDistance, sizeof(float));
        debugPrintf("  Float value: %.2f -> Hex: %02X %02X %02X %02X\n",
                   tripDistance, ptr[0], ptr[1], ptr[2], ptr[3]);
        ptr += sizeof(float);
        
        // Pack distance_driven_total_m (4 bytes)
        debugPrintln("Packing total distance...");
        float totalDistance = rideStats.distance_driven_total_m;
        memcpy(ptr, &totalDistance, sizeof(float));
        debugPrintf("  Float value: %.2f -> Hex: %02X %02X %02X %02X\n",
                   totalDistance, ptr[0], ptr[1], ptr[2], ptr[3]);
        ptr += sizeof(float);
        
        // Pack max_rpm (4 bytes)
        debugPrintln("Packing max RPM...");
        uint32_t rpm = rideStats.max_rpm;
        *ptr++ = (rpm >> 24) & 0xFF;
        *ptr++ = (rpm >> 16) & 0xFF;
        *ptr++ = (rpm >> 8) & 0xFF;
        *ptr++ = rpm & 0xFF;
        debugPrintf("  RPM value: %d -> Hex: %02X %02X %02X %02X\n",
                   rpm, *(ptr-4), *(ptr-3), *(ptr-2), *(ptr-1));
        
        // Pack max_accel_mps2 (4 bytes)
        debugPrintln("Packing max acceleration...");
        float maxAccel = rideStats.max_accel_mps2;
        memcpy(ptr, &maxAccel, sizeof(float));
        debugPrintf("  Float value: %.2f -> Hex: %02X %02X %02X %02X\n",
                   maxAccel, ptr[0], ptr[1], ptr[2], ptr[3]);
        ptr += sizeof(float);
        
        // Pack best_accel_times[5] (20 bytes)
        debugPrintln("Packing acceleration times...");
        // Store the first 5 best times from any thresholds
        int times_stored = 0;
        for (int i = 0; i < 10 && times_stored < 5; i++) {
            float bestTime = rideStats.getAccelTime(i, 0);
            if (bestTime > 0) {
                memcpy(ptr, &bestTime, sizeof(float));
                debugPrintf("  Time #%d (0-%d km/h): %.2fs -> Hex: %02X %02X %02X %02X\n",
                           times_stored+1, rideStats.getThresholdSpeed(i), 
                           bestTime, ptr[0], ptr[1], ptr[2], ptr[3]);
                ptr += sizeof(float);
                times_stored++;
            }
        }
        
        // Fill remaining slots with zeros if we don't have 5 times
        while (times_stored < 5) {
            float zeroTime = 0.0f;
            memcpy(ptr, &zeroTime, sizeof(float));
            debugPrintf("  Time #%d: 0.00s (empty) -> Hex: %02X %02X %02X %02X\n",
                       times_stored+1, ptr[0], ptr[1], ptr[2], ptr[3]);
            ptr += sizeof(float);
            times_stored++;
        }
        // Verify total size
        if ((ptr - rideStatsData) != 36) {
            debugPrintf("ERROR: Incorrect packet size: %d bytes (expected 36)\n", (ptr - rideStatsData));
            return;
        }
    
        // Print full packet before sending
        debugPrintln("Complete packet data:");
        debugPrint("  HEX: ");
        for (int i = 0; i < 36; i++) {
            debugPrintf("%02X ", rideStatsData[i]);
        }
        debugPrintln("");
    
        debugPrintln("Sending ride stats packet...");
        sendPacket(RIDE_STATS_PACKET_TYPE, rideStatsData, sizeof(rideStatsData));
        debugPrintln("Ride stats data sent");
    }
    void sendTempData()
    {
        uint8_t tempData[26];
        int offset = 0;
        
        bms::sendReceiveBasicInfo();
    
        // Pack battery1 temperature (2 byte)
        uint16_t battery1 = bms::basicInfo.temperatures[1];
        tempData[offset++] = battery1 & 0xFF;
        tempData[offset++] = (battery1 >> 8) & 0xFF;
        
        // Pack battery2 temperature (2 bytes)
        uint16_t battery2 = bms::basicInfo.temperatures[2];
        tempData[offset++] = battery2 & 0xFF;
        tempData[offset++] = (battery2 >> 8) & 0xFF;
        
        // Pack outside temperature (float - 4 bytes)
        float outside = temperature::readTemperature(TEMP_OUTSIDE);
        memcpy(&tempData[offset], &outside, sizeof(float));
        offset += sizeof(float);
        
        // Pack BMS temperature (2 byte)
        uint16_t bms_temp = bms::basicInfo.temperatures[0];
        tempData[offset++] = bms_temp & 0xFF;
        tempData[offset++] = (bms_temp >> 8) & 0xFF;
        
        // Pack brake temperature (float - 4 bytes)
        float brake = temperature::readTemperature(TEMP_BRAKE);
        memcpy(&tempData[offset], &brake, sizeof(float));
        offset += sizeof(float);
        
        // Pack motor controller temperature (float - 4 bytes)
        float motorController = temperature::readTemperature(TEMP_MOTOR_CONTROLLER);
        memcpy(&tempData[offset], &motorController, sizeof(float));
        offset += sizeof(float);
        
        // Pack control unit temperature (float - 4 bytes)
        float controlUnit = temperature::readTemperature(TEMP_CONTROL_UNIT);
        memcpy(&tempData[offset], &controlUnit, sizeof(float));
        offset += sizeof(float);
        
        // Pack motor temperature (float - 4 bytes)
        float motor = temperature::readTemperature(TEMP_MOTOR);
        memcpy(&tempData[offset], &motor, sizeof(float));
        offset += sizeof(float);
        
        // Send the packet with all temperature data
        sendPacket(TEMP_PACKET_TYPE, tempData, sizeof(tempData));
    }

    void setSpeedMode(uint8_t mode)
    {
        debugPrintf("SpeedModeReceived:%d", mode);

        if (mode == 0)
        {
            shiftreg::set_motor_hi_speed(false);
            shiftreg::set_motor_lo_speed(true);
        }
        else if (mode == 1)
        {
            shiftreg::set_motor_lo_speed(false);
            shiftreg::set_motor_hi_speed(false);
        }
        else if (mode == 2)
        {
            shiftreg::set_motor_lo_speed(false);
            shiftreg::set_motor_hi_speed(true);
        }
    }

    void setRideMode(uint8_t mode)
    {
        debugPrintf("SpeedModeReceived:%d", mode);

        if (mode == 0)
        {
            shiftreg::set_motor_foot_sw(true);
            shiftreg::set_motor_reverse(false);
        }
        else if (mode == 1)
        {
            shiftreg::set_motor_foot_sw(false);
        }
        else if (mode == 2)
        {
            shiftreg::set_motor_foot_sw(true);

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