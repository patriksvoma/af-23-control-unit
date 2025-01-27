/*
    DS1307 RTC module, also includes an EEPROM chip
    Communicates using I2C
    - RTC address: 0x68
    - EEPROM address: 0x50 (not used)
*/

#include <Arduino.h>
#include <uRTCLib.h>

#define I2C0_SDA 4
#define I2C0_SCL 5

namespace rtc
{
    void init();

    uRTCLib rtc(0x68);
    char daysOfTheWeek[7][12] = {"Nedele", "Pondeli", "Utery", "Streda", "Ctvrtek", "Patek", "Sobota"};

    /// @brief Initializes the RTC module
    void init()
    {
        URTCLIB_WIRE.setSCL(I2C0_SCL);
        URTCLIB_WIRE.setSDA(I2C0_SDA);
        URTCLIB_WIRE.begin();

        // Set the current time
        //rtc.set(0, 35, 20, URTCLIB_WEEKDAY_MONDAY, 27, 1, 25);
    }

    void printOut()
    {
        // Read out the data
        rtc.refresh();
        Serial.println("Year: " + String(rtc.year()));
        Serial.println("Month: " + String(rtc.month()));
        Serial.println("Day: " + String(rtc.day()));
        Serial.println("Day of the week: " + String(daysOfTheWeek[rtc.dayOfWeek() - 1]));
        Serial.println("Hour: " + String(rtc.hour()));
        Serial.println("Minute: " + String(rtc.minute()));
        Serial.println("Second: " + String(rtc.second()));
    }
}