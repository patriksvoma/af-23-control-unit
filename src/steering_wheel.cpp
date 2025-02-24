/*
    Steering wheel module
*/

#include <Arduino.h>

#define UART1_TX 8
#define UART1_RX 9
#define CON_PIN 15

namespace steeringWheel
{
    void init();
    bool testConnectionPin();
    bool testConnectionSerial();

    /// @brief Initializes the steering wheel module
    void init()
    {
        pinMode(CON_PIN, INPUT);

        Serial2.setTX(UART1_TX);
        Serial2.setRX(UART1_RX);
        Serial2.setTimeout(50);
        Serial2.begin(9600);
    }

    /// @brief Checks if the steering wheel detection pin is LOW
    /// @return True if the wheel is connected (pin LOW)
    bool testConnectionPin()
    {
        return !digitalRead(CON_PIN);
    }

    /// @brief Sends a test message to the steering wheel and checks for a response
    /// @return True if the response is valid
    bool testConnectionSerial()
    {
        Serial2.write(0xAA);
        long checkStartTime = millis();

        while (Serial2.available() == 0)
        {
            delay(1);

            if (millis() > checkStartTime + 200)
            {
                Serial.println("Steering wheel connection check timed out!");
                return false;
            }
        }

        uint8_t response = Serial2.read();

        if (response == 0xBB)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}