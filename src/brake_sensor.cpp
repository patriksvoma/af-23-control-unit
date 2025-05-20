/*
    Brake pressure sensor module

    Authors:
    Patrik Švoma
*/

#include <Arduino.h>

#define SENSOR_PIN 26
#define ADC_RES 12 // ADC resolution, default is 10 bits (0-1023), maximum is 12 bits (0-4095)

namespace brakeSensor
{
    void init();
    uint16_t readPressureRaw();
    float readPressureBar();
    float readPressureMPa();

    /// @brief Initializes the brake sensor module
    void init()
    {
        pinMode(SENSOR_PIN, INPUT);
        analogReadResolution(ADC_RES);
    }

    /// @brief Reads the current pressure and returns the raw reading
    /// @return ADC reading, always scaled to 12 bits!
    uint16_t readPressureRaw()
    {
        uint16_t pressure = map(analogRead(SENSOR_PIN), 0, (1 << ADC_RES) - 1, 0, 4095);

        return pressure;
    }

    /// @brief Reads the current pressure and converts it to bar
    /// @return Pressure in bars
    float readPressureBar()
    {
        return map(readPressureRaw(), 0, 4095, 0, 12000) / 1000;
    }

    /// @brief Reads the current pressure and converts it to MPa
    /// @return Pressure in MPa
    float readPressureMPa()
    {
        return readPressureBar() / 10;
    }
}