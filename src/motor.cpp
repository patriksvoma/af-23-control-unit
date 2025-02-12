/*
    Motor controller module
    High level module for interfacing the motor controller

    Note: The output on the analog brake is inverted (255 will result in 0 V)
*/

#include <Arduino.h>

#define MOTOR_BRAKE 14
#define MOTOR_HALL 3
#define MOTOR_THROTTLE 27
#define ADC_RES 12 // ADC resolution, default is 10 bits (0-1023), maximum is 12 bits (0-4095)

namespace motor
{
    void init();
    void setMotorBrake(uint8_t value);
    bool getHallSignal();
    uint16_t getThrottle();

    void init()
    {
        // This module is not used currently because of the need to manually control low and high speed
        pinMode(MOTOR_BRAKE, OUTPUT);
        pinMode(MOTOR_HALL, INPUT);
        pinMode(MOTOR_THROTTLE, INPUT);
        
        analogReadResolution(ADC_RES);

        analogWrite(MOTOR_BRAKE, 255);
    }

    /// @brief Sets the analog brake value
    /// @param value Value (0-255)
    void setMotorBrake(uint8_t value)
    {
        analogWrite(MOTOR_BRAKE, 255 - value);
    }

    /// @brief Gets the current state of the hall sensor
    /// @return Bool
    bool getHallSignal()
    {
        return digitalRead(MOTOR_HALL);
    }

    /// @brief Gets the current throttle value
    /// @return 0-1023
    uint16_t getThrottle()
    {
        return analogRead(MOTOR_THROTTLE);
    }
}