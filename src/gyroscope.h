/*
    Gyroscope module
*/

#include <Adafruit_MPU6050.h>

namespace gyroscope
{
    void init();
    sensors_event_t getAcceleration();
    sensors_event_t getRotation();
    sensors_event_t getTemperature();
    void loop();
}