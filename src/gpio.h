/*
    General purpose input/output module
    Used for the GPO, GPI pins
*/

#include <Arduino.h>

namespace gpio
{
    void init();
    void write(uint8_t gpo, bool value);
    bool read(uint8_t gpi);
}