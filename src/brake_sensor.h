/*
    Brake pressure sensor module
*/

namespace brakeSensor
{
    void init();
    int readPressureRaw();
    float readPressureBar();
    float readPressureMPa();
}