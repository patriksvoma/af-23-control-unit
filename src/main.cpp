#include <Arduino.h>
#include <SerialBT.h>

#include <shiftreg.h>
#include <motor.h>
#include <gpio.h>
#include <gyroscope.h>
#include <brake_sensor.h>
#include <rtc.h>
#include <bms.h>
#include <gpio.h>

#include <storage.h>
#include <temperature.h>
#include <steering_wheel.h>
#include <Wire.h>

#define BLUETOOTH_CONTROL

#ifdef BLUETOOTH_CONTROL

#define FAKE_BMS_REPORT
ulong lastBTSendTimestamp;
ulong btSendDelay = 100;
#endif

void setup()
{
    Serial.begin();
    delay(5000);
    Serial.println("Starting bluetooth");

    SerialBT.setName("AF-23");
    SerialBT.setTimeout(10);
    SerialBT.begin();

    Serial.println("Bluetooth started, waiting for connection. Starting modules.");

    // Initialize modules
    shiftreg::init();
    motor::init();
    gpio::init();
    gyroscope::init();
    brakeSensor::init();
    rtc::init();
    bms::init();
    gpio::init();

    //storage::init();
    //temperature::init();
    //steeringWheel::init();

    Serial.println("Module setup finished");
}

void loop()
{
    // Allow rebooting via USB serial
    if (Serial.available() > 0)
    {
        String command = Serial.readString();

        if (command == "reb\r\n")
        {
            Serial.println("Rebooting");
            rp2040.reboot();
        }
    }

#ifdef BLUETOOTH_CONTROL
    if (SerialBT.availableForWrite())
    {
        if (SerialBT.available() > 0)
        {
            // Read the first received byte (command code)
            uint8_t commandCode = SerialBT.read();
            uint8_t commandValue;
        
            switch (commandCode)
            {
            case 0x01:
                Serial.println("Motor PWR");

                commandValue = SerialBT.read();
                Serial.println(commandValue);

                if (commandValue == 0) shiftreg::set_motor_pwr(false);
                else if (commandValue == 1) shiftreg::set_motor_pwr(true);
                else Serial.println("Invalid command value");

                break;
            
            case 0x02:
                Serial.println("Foot switch");
                
                commandValue = SerialBT.read();
                Serial.println(commandValue);

                if (commandValue == 0) shiftreg::set_motor_foot_sw(false);
                else if (commandValue == 1) shiftreg::set_motor_foot_sw(true);
                else Serial.println("Invalid command value");

                break;
            
            case 0x03:
                Serial.println("Brake switch");
                
                commandValue = SerialBT.read();
                Serial.println(commandValue);

                if (commandValue == 0) shiftreg::set_motor_brake_sw(false);
                else if (commandValue == 1) shiftreg::set_motor_brake_sw(true);
                else Serial.println("Invalid command value");

                break;
            
            case 0x04:
                Serial.println("Low speed");
                
                commandValue = SerialBT.read();
                Serial.println(commandValue);

                if (commandValue == 0) shiftreg::set_motor_lo_speed(false);
                else if (commandValue == 1) shiftreg::set_motor_lo_speed(true);
                else Serial.println("Invalid command value");

                break;
            
            case 0x05:
                Serial.println("High speed");
                
                commandValue = SerialBT.read();
                Serial.println(commandValue);

                if (commandValue == 0) shiftreg::set_motor_hi_speed(false);
                else if (commandValue == 1) shiftreg::set_motor_hi_speed(true);
                else Serial.println("Invalid command value");

                break;
            
            case 0x06:
                Serial.println("Reverse");
                
                commandValue = SerialBT.read();
                Serial.println(commandValue);

                if (commandValue == 0) shiftreg::set_motor_reverse(false);
                else if (commandValue == 1) shiftreg::set_motor_reverse(true);
                else Serial.println("Invalid command value");

                break;
            
            case 0x07:
                Serial.println("Motor fan");
                
                commandValue = SerialBT.read();
                Serial.println(commandValue);

                if (commandValue == 0) shiftreg::set_motor_fan(false);
                else if (commandValue == 1) shiftreg::set_motor_fan(true);
                else Serial.println("Invalid command value");

                break;
            
            case 0x08:
                Serial.println("GPO 0");
                
                commandValue = SerialBT.read();
                Serial.println(commandValue);

                if (commandValue == 0) shiftreg::set_gpo_0(false);
                else if (commandValue == 1) shiftreg::set_gpo_0(true);
                else Serial.println("Invalid command value");

                break;
            
            case 0x09:
                Serial.println("Analog brake");
                
                commandValue = SerialBT.read();
                Serial.println(commandValue);

                // The value will always be a valid number
                motor::setMotorBrake(commandValue);

                break;
            
            case 0x10:
                Serial.println("GPO 1");
                
                commandValue = SerialBT.read();
                Serial.println(commandValue);

                if (commandValue == 0) gpio::write(1, false);
                else if (commandValue == 1) gpio::write(1, true);
                else Serial.println("Invalid command value");

                break;
            
            case 0x11:
                Serial.println("GPO 2");
                
                commandValue = SerialBT.read();
                Serial.println(commandValue);

                if (commandValue == 0) gpio::write(2, false);
                else if (commandValue == 1) gpio::write(2, true);
                else Serial.println("Invalid command value");

                break;
            
            default:
                Serial.println("Unknown command code");
                break;
            }

            // All bytes should have already been read at this time. If not, something is not correct
            if (SerialBT.available() > 0) Serial.println("Not all bytes received via bluetooth have been handled!");
        }
    }
    else
    {
        Serial.println("Not connected!");
    }

    if (millis() > lastBTSendTimestamp + btSendDelay && SerialBT.availableForWrite())
    {
        Serial.println("Sending data..");

        // Start byte
        SerialBT.write(0xDD);

        // Gyroscope data (28)
        sensors_event_t gyroAcceleration, gyroRotation, gyroTemp;
        gyroAcceleration = gyroscope::getAcceleration();
        gyroRotation = gyroscope::getRotation();
        gyroTemp = gyroscope::getTemperature();

        SerialBT.write((uint8_t*)&gyroAcceleration.acceleration.x, 4);
        SerialBT.write((uint8_t*)&gyroAcceleration.acceleration.y, 4);
        SerialBT.write((uint8_t*)&gyroAcceleration.acceleration.z, 4);

        SerialBT.write((uint8_t*)&gyroRotation.gyro.x, 4);
        SerialBT.write((uint8_t*)&gyroRotation.gyro.y, 4);
        SerialBT.write((uint8_t*)&gyroRotation.gyro.z, 4);

        SerialBT.write((uint8_t*)&gyroTemp.temperature, 4);

        // Motor data (3)
        SerialBT.write((uint8_t)motor::getHallSignal());

        uint16_t throttleValue = motor::getThrottle();
        SerialBT.write((uint8_t)(throttleValue >> 8));
        SerialBT.write((uint8_t)(throttleValue & 0xFF));

        // Brake pressure (2)
        uint16_t brakePressure = brakeSensor::readPressureRaw();
        SerialBT.write((uint8_t)(brakePressure >> 8));
        SerialBT.write((uint8_t)(brakePressure & 0xFF));

        // Temperatures (8)
        SerialBT.write(0xFF);
        SerialBT.write(0xFF);
        SerialBT.write(0xFF);
        SerialBT.write(0xFF);
        SerialBT.write(0xFF);
        SerialBT.write(0xFF);
        SerialBT.write(0xFF);
        SerialBT.write(0xFF);

        // Flash memory test (1)
        SerialBT.write(0xFF);

        // Steering wheel connected (1)
        SerialBT.write(0xFF);

        // RTC data (7)
        rtc::refresh();
        SerialBT.write(rtc::getYear());
        SerialBT.write(rtc::getMonth());
        SerialBT.write(rtc::getDay());
        SerialBT.write(rtc::getHour());
        SerialBT.write(rtc::getMinute());
        SerialBT.write(rtc::getSecond());
        SerialBT.write(rtc::getDayOfWeek());

        // BMS data (8)
#ifdef FAKE_BMS_REPORT
        for (int i = 0; i < 8; i++) SerialBT.write(0xFF);
#else
        bms::sendReceiveBasicInfo();
        SerialBT.write((uint8_t)(bms::basicInfo.totalVoltage >> 8));
        SerialBT.write((uint8_t)(bms::basicInfo.totalVoltage & 0xFF));

        SerialBT.write((uint8_t)(bms::basicInfo.current >> 8));
        SerialBT.write((uint8_t)(bms::basicInfo.current & 0xFF));

        SerialBT.write((uint8_t)(bms::basicInfo.nominalCapacity >> 8));
        SerialBT.write((uint8_t)(bms::basicInfo.nominalCapacity & 0xFF));

        SerialBT.write((uint8_t)(bms::basicInfo.remainingCapacity >> 8));
        SerialBT.write((uint8_t)(bms::basicInfo.remainingCapacity & 0xFF));
#endif

        // GPI (3)
        SerialBT.write((uint8_t)gpio::read(0));
        SerialBT.write((uint8_t)gpio::read(1));
        SerialBT.write((uint8_t)gpio::read(2));

        Serial.println("Data sent");
        lastBTSendTimestamp = millis();
    }

#endif

    delay(10);
}