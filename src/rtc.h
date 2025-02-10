/*
    DS1307 RTC module, also includes an EEPROM chip
    Communicates using I2C
    - RTC address: 0x68
    - EEPROM address: 0x50
*/

namespace rtc
{
    void init();
    void printOut();
}