/*
    Indicator LED module
*/

#include <Arduino.h>

#define LED_PIN 20

namespace indicatorLed
{
    void init();
    void writeDigital(bool value);

    void init()
    {
        pinMode(LED_PIN, OUTPUT);
    }

    /// @brief Writes a digital value to the indicator LED
    void writeDigital(bool value)
    {
        digitalWrite(LED_PIN, value);
    }
}