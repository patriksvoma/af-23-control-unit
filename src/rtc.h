/*
    DS1307 RTC module, also includes an EEPROM chip
    Communicates using I2C
    - RTC address: 0x68
    - EEPROM address: 0x50
*/

#include <Arduino.h>

namespace rtc
{
    void init();
    void refresh();
    uint8_t getYear();
    uint8_t getMonth();
    uint8_t getDay();
    uint8_t getHour();
    uint8_t getMinute();
    uint8_t getSecond();
    uint8_t getDayOfWeek();
    void printOut();
}