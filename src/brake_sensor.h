/*
    Brake pressure sensor module

    Authors:
    Patrik Švoma
*/

namespace brakeSensor
{
    void init();
    int readPressureRaw();
    float readPressureBar();
    float readPressureMPa();
}