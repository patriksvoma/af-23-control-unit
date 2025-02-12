/*
    General purpose input/output module
    Used for the GPO, GPI pins

    GPO_0 is toggled by the shift register
*/

#include <Arduino.h>
#include <shiftreg.h>

#define GPO_1 10
#define GPO_2 11
#define GPI_0 21
#define GPI_1 22
#define GPI_2 28

namespace gpio
{
    void init();
    void writeDigital(uint8_t gpo, bool value);
    void writeAnalog(uint8_t gpo, uint8_t value);
    bool read(uint8_t gpi);

    /// @brief Initializes the GPIO module
    void init()
    {
        pinMode(GPO_1, OUTPUT);
        pinMode(GPO_2, OUTPUT);
        pinMode(GPI_0, INPUT);
        pinMode(GPI_1, INPUT);
        pinMode(GPI_2, INPUT);

        digitalWrite(GPO_1, LOW);
        digitalWrite(GPO_2, LOW);
    }

    /// @brief Writes to a GPO pin
    /// @param gpo GPO pin number (1-2), if an invalid pin is requested, nothing happens
    /// @param value Digital value
    void writeDigital(uint8_t gpo, bool value)
    {
        if (gpo == 1) digitalWrite(GPO_1, value);
        else if (gpo == 2) digitalWrite(GPO_2, value);
        else return;
    }

    /// @brief Writes to a GPO pin
    /// @param gpo GPO pin number (1-2), if an invalid pin is requested, nothing happens
    /// @param value Analog value (0-255)
    void writeAnalog(uint8_t gpo, uint8_t value)
    {
        if (gpo == 1) analogWrite(GPO_1, value);
        else if (gpo == 2) analogWrite(GPO_2, value);
        else return;
    }

    /// @brief Reads a GPI pin
    /// @param gpi GPI pin number (0-2)
    /// @return Bool, if an invalid pin is requested, returns false
    bool read(uint8_t gpi)
    {
        bool val;

        if (gpi == 0) val = digitalRead(GPI_0);
        else if (gpi == 1) val = digitalRead(GPI_1);
        else if (gpi == 2) val = digitalRead(GPI_2);
        else return false;

        return val;
    }
}