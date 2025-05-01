/*
    DS18B20 temperature sensor module
*/

#define TEMP_MOTOR_CONTROLLER 0
#define TEMP_OUTSIDE 1
#define TEMP_MOTOR 2
#define TEMP_CONTROL_UNIT 3
#define TEMP_BRAKE 4

namespace temperature
{
    void init();
    void search();
    float readTemperature(uint8_t sensorId);
}