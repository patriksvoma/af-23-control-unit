/*
    General purpose input/output module
    Used for the GPO, GPI pins
*/

#include <Arduino.h>

namespace gpio
{
    void init();
    void writeDigital(uint8_t gpo, bool value);
    void writeAnalog(uint8_t gpo, uint8_t value);
    bool read(uint8_t gpi);
}