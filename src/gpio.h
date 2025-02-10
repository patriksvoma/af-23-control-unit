/*
    General purpose input/output module
    Used for the GPO, GPI pins
*/

namespace gpio
{
    void init();
    void write(uint8_t gpo, bool value);
    bool read(uint8_t gpi);
}