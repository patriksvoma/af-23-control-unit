/*
    Motor controller module
    High level module for interfacing the motor controller

    Note: The output on the analog brake is inverted (255 will result in 0 V)

    Authors:
    Patrik Švoma
    Jakub Aldorf
*/

#include <Arduino.h>
#include <motor.h>
#include <shiftreg.h>

// Pin definitions
#define MOTOR_BRAKE 14
#define MOTOR_HALL 3
#define MOTOR_THROTTLE 27
#define ADC_RES 12 // ADC resolution, default is 10 bits (0-1023), maximum is 12 bits (0-4095)

namespace motor
{
    void init();
    void setRideMode(uint8_t rideMode);
    uint8_t getRideMode();
    void setRideSpeed(uint8_t rideSpeed);
    uint8_t getRideSpeed();
    void setMotorBrakeLevel(uint8_t brakeLevel);
    void increaseMotorBrakeLevel();
    void decreaseMotorBrakeLevel();
    uint8_t getMotorBrakeLevel();
    void setRawMotorBrake(uint8_t value);

    bool getRawHallSignal();
    uint16_t getThrottle();
    uint32_t getRPM();
    float getMPS();
    float getKMPH();

    void hallInterrupt();

    const int HALL_PULSES_PER_ROTATION = 4; // Number of pulses detected by all hall sensors on one rotation of the motor
    const float BELT_GEAR_RATIO = 0.5;      // Gear ratio of the motor belt - motor rotations : diff rotations
    const float DIFF_GEAR_RATIO = 0.25;     // Gear ratio of the differential - diff rotations : wheel rotations
    const int WHEEL_DIAMETER_mm = 500;      // Diameter of the back wheels in milimeters
    // DO NOT CHANGE
    const float WHEEL_CIRCUMFERENCE_mm = WHEEL_DIAMETER_mm * PI;

    unsigned long hall_pulse_last_us;
    unsigned long hall_pulse_previous_us;
    uint8_t currentRideMode;
    uint8_t currentRideSpeed;
    uint8_t currentMotorBrakeLevel;

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

    /// @brief Sets the ride mode
    /// @param rideMode 0 - Neutral, 1 - Forward, 2 - Reverse
    void setRideMode(uint8_t rideMode)
    {
        switch (rideMode)
        {
        case RIDE_MODE_FORWARD:
            shiftreg::set_motor_foot_sw(true);
            shiftreg::set_motor_brake_sw(false);
            shiftreg::set_motor_reverse(false);
            break;
        case RIDE_MODE_NEUTRAL:
            shiftreg::set_motor_foot_sw(false);
            shiftreg::set_motor_brake_sw(true);
            shiftreg::set_motor_reverse(false);
            break;
        case RIDE_MODE_REVERSE:
            shiftreg::set_motor_foot_sw(true);
            shiftreg::set_motor_brake_sw(false);
            shiftreg::set_motor_reverse(true);
            break;
        default:
            Serial.println("Trying to set invalid rideMode!");
            return;
        }

        currentRideMode = rideMode;
    }

    /// @brief Gets the current ride mode
    /// @return 0 - Neutral, 1 - Forward, 2 - Reverse
    uint8_t getRideMode()
    {
        return currentRideMode;
    }

    /// @brief Sets the ride speed
    /// @param rideSpeed 0 - Low, 1 - Medium, 2 - High
    void setRideSpeed(uint8_t rideSpeed)
    {
        switch (rideSpeed)
        {
        case RIDE_SPEED_LOW:
            shiftreg::set_motor_lo_speed(true);
            shiftreg::set_motor_hi_speed(false);
            break;
        case RIDE_SPEED_MEDIUM:
            shiftreg::set_motor_lo_speed(false);
            shiftreg::set_motor_hi_speed(false);
            break;
        case RIDE_SPEED_HIGH:
            shiftreg::set_motor_lo_speed(false);
            shiftreg::set_motor_hi_speed(true);
            break;
        default:
            Serial.println("Trying to set invalid rideSpeed!");
            return;
        }

        currentRideSpeed = rideSpeed;
    }

    /// @brief Gets the current ride speed
    /// @return 0 - Neutral, 1 - Forward, 2 - Reverse
    uint8_t getRideSpeed()
    {
        return currentRideSpeed;
    }

    /// @brief Sets the level of motor braking
    /// @param brakeLevel 0-3, with 0 being no braking
    void setMotorBrakeLevel(uint8_t brakeLevel)
    {
        if (brakeLevel > 3)
        {
            Serial.println("Trying to set invalid motor braking level");
            return;
        }

        setRawMotorBrake(64 * brakeLevel - 1);

        currentMotorBrakeLevel = brakeLevel;
    }

    /// @brief Increases the motor braking level by 1
    void increaseMotorBrakeLevel()
    {
        if (getMotorBrakeLevel() == 3)
            return;

        setMotorBrakeLevel(getMotorBrakeLevel() + 1);
    }

    /// @brief Decreases the motor braking level by 1
    void decreaseMotorBrakeLevel()
    {
        if (getMotorBrakeLevel() == 0)
            return;

        setMotorBrakeLevel(getMotorBrakeLevel() - 1);
    }

    /// @brief Gets the current motor braking level
    /// @return 0-3, with 0 being no braking
    uint8_t getMotorBrakeLevel()
    {
        return currentMotorBrakeLevel;
    }

    /// @brief Sets the analog brake value
    /// @param value Value (0-255)
    void setRawMotorBrake(uint8_t value)
    {
        analogWrite(MOTOR_BRAKE, 255 - value);
    }

    /// @brief Gets the current state of the hall sensor
    /// @return Bool
    bool getRawHallSignal()
    {
        return digitalRead(MOTOR_HALL);
    }

    /// @brief Gets the current throttle value
    /// @return 0-1023
    uint16_t getThrottle()
    {
        return analogRead(MOTOR_THROTTLE);
    }

    /// @brief Gets the current RPM of the motor
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
        else
        {
            rpm = 0;
        }

        // If a hall interrupt hasn't been recieved for a second, set the RPM to zero
        // This will make sure the display will show 0 when stopped
        if (micros() - hall_pulse_last_us > 250000)
            rpm = 0;

        return rpm;
    }

    /// @brief Gets the current speed in meters per second. Calculated from the motor RPM.
    float getMPS()
    {
        // 1. Get the RPM at the differential input
        float diff_rpm = (float)getRPM() * BELT_GEAR_RATIO;
        // 2. Convert RPM from the differential to the wheel axes
        float wheel_rpm = (float)diff_rpm * DIFF_GEAR_RATIO;
        // 3. Calculate m/s and km/h
        return wheel_rpm * (WHEEL_CIRCUMFERENCE_mm / 1000.0) / 60.0;
    }

    /// @brief Gets the current speed in kilometers per hour. Calculated from the motor RPM.
    float getKMPH()
    {
        return getMPS() * 3.6;
    }

    /// @brief Hall sensor 1 interrupt
    void hallInterrupt()
    {
        // Update the pulse timestamps
        hall_pulse_previous_us = hall_pulse_last_us;
        hall_pulse_last_us = micros();
    }
}