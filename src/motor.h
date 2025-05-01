/*
    Motor controller module
    High level module for interfacing the motor controller

    Note: The output on the analog brake is inverted (255 will result in 0 V)
*/

// Ride mode and speed definitions
#define RIDE_MODE_FORWARD 1
#define RIDE_MODE_NEUTRAL 0
#define RIDE_MODE_REVERSE 2

#define RIDE_SPEED_LOW 0
#define RIDE_SPEED_MEDIUM 1
#define RIDE_SPEED_HIGH 2

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

    uint16_t getThrottle();
    uint32_t getRPM();
}