/*
    Steering wheel module
*/

#include <Arduino.h>

#define WHEEL_TX 8
#define WHEEL_RX 9

namespace steeringWheel
{
    void init();
    bool testConnection();

    /// @brief Initializes the steering wheel module
    void init()
    {
        Serial2.setTX(WHEEL_TX);
        Serial2.setRX(WHEEL_RX);
        Serial2.setTimeout(50);
        Serial2.begin(9600);
    }

    /// @brief Sends a test message to the steering wheel and checks for a response
    /// @return True if the response is valid
    bool testConnection()
    {
        Serial2.println("connectionCheck");
        String response = Serial2.readString();

        if (response == "connectionValid\r\n")
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}