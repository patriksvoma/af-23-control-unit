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
    uint32_t getRPM();
    void hallInterrupt();

    const int HALL_PULSES_PER_ROTATION = 4;

    unsigned long hall_pulse_last_us;
    unsigned long hall_pulse_previous_us;

    void init()
    {
        // This module is not used currently because of the need to manually control low and high speed
        pinMode(MOTOR_BRAKE, OUTPUT);
        pinMode(MOTOR_HALL, INPUT);
        pinMode(MOTOR_THROTTLE, INPUT);

        attachInterrupt(MOTOR_HALL, hallInterrupt, RISING);

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
    
    uint32_t getRPM()
    {
        uint32_t rpm;

        // Calculate RPM
        if (hall_pulse_previous_us != 0 && hall_pulse_last_us != 0)
        {
            // Time it takes to complete one rotation
            unsigned long rotation_time_us = (hall_pulse_last_us - hall_pulse_previous_us) * HALL_PULSES_PER_ROTATION;

            // Minute / time per rotation
            rpm = 60000000 / rotation_time_us;
        }
        else rpm = 0;

        // If a hall interrupt hasn't been recieved for a second, set the RPM to zero
        // This will make sure the display will show 0 when stopped
        if (micros() - hall_pulse_last_us > 250000) rpm = 0;

        return rpm;
    }

    /// @brief Hall sensor 1 interrupt
    void hallInterrupt()
    {
        // Update the pulse timestamps
        hall_pulse_previous_us = hall_pulse_last_us;
        hall_pulse_last_us = micros();
    }
}