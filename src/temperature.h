/*
    DS18B20 temperature sensor module
*/

namespace temperature
{
    void init();
    void search();
    float readTemperature(uint8_t sensorId);
}