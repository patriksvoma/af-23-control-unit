#include <Arduino.h>
#include <SerialBT.h>

#include <shiftreg.h>
#include <motor.h>
#include <gpio.h>

#include <gyroscope.h>
#include <storage.h>
#include <temperature.h>
#include <brake_sensor.h>
#include <steering_wheel.h>
#include <balancer.h>
#include <rtc.h>
#include <Wire.h>

#define BLUETOOTH_CONTROL

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
    
    //gyroscope::init();
    //storage::init();
    //temperature::init();
    //brakeSensor::init();
    //steeringWheel::init();
    //balancer::init();
    //rtc::init();

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
                motor::setBrake(commandValue);

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
#endif

    delay(10);
}