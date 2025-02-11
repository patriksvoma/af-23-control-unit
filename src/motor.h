/*
    Motor controller module
    High level module for interfacing the motor controller

    Note: The output on the analog brake is inverted (255 will result in 0 V)
*/

namespace motor
{
    void init();
    void setMotorBrake(uint8_t value);
    bool getHallSignal();
    uint16_t getThrottle();
}