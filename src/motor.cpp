/*
    Motor controller module
    High level module for interfacing the motor controller

    Note: The output on the analog brake is inverted (255 will result in 0 V)
*/

#include <Arduino.h>

#define MOTOR_BRAKE 14

namespace motor
{
    void init();
    void setBrake(uint8_t value);

    void init()
    {
        // This module is not used currently because of the need to manually control low and high speed
        pinMode(MOTOR_BRAKE, OUTPUT);

        analogWrite(MOTOR_BRAKE, 255);
    }

    /// @brief Sets the analog brake value
    /// @param value Value (0-255)
    void setBrake(uint8_t value)
    {
        analogWrite(MOTOR_BRAKE, 255 - value);
    }
}