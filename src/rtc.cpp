/*
    DS1307 RTC module, also includes an EEPROM chip
    Communicates using I2C
    - RTC address: 0x68
    - EEPROM address: 0x50 (not used)

    Authors:
    Patrik Švoma
*/

#include <Arduino.h>
#include <uRTCLib.h>

#define I2C0_SDA 4
#define I2C0_SCL 5

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

    uRTCLib rtc(0x68);
    char daysOfTheWeek[7][12] = {"Nedele", "Pondeli", "Utery", "Streda", "Ctvrtek", "Patek", "Sobota"};

    /// @brief Initializes the RTC module
    void init()
    {
        URTCLIB_WIRE.setSCL(I2C0_SCL);
        URTCLIB_WIRE.setSDA(I2C0_SDA);
        URTCLIB_WIRE.begin();

        // Set the current time
        // rtc.set(0, 35, 20, URTCLIB_WEEKDAY_MONDAY, 27, 1, 25);
    }

    /// @brief Refreshes the RTC data
    void refresh()
    {
        rtc.refresh();
    }

    /// @brief Gets the current year
    /// @return Last two digits of the year
    uint8_t getYear()
    {
        return rtc.year();
    }

    /// @brief Gets the current month
    uint8_t getMonth()
    {
        return rtc.month();
    }

    /// @brief Gets the current day
    uint8_t getDay()
    {
        return rtc.day();
    }

    /// @brief Gets the current hour
    uint8_t getHour()
    {
        return rtc.hour();
    }

    /// @brief Gets the current minute
    uint8_t getMinute()
    {
        return rtc.minute();
    }

    /// @brief Gets the current second
    uint8_t getSecond()
    {
        return rtc.second();
    }

    /// @brief Gets the day of the week
    /// @return Index of the day of week, starting from sunday
    uint8_t getDayOfWeek()
    {
        return rtc.dayOfWeek();
    }

    // TODO: Remove
    /// @brief Prints out data from the RTC module
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