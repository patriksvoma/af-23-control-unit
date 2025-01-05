/*
    DS18B20 temperature sensor module
    Currently only supports 1 sensor
*/

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 16

namespace temperature
{
    OneWire oneWire(ONE_WIRE_BUS);
    DallasTemperature sensors(&oneWire);

    void init();
    void loop();

    void init()
    {
        sensors.begin();
    }

    void loop()
    {
        Serial.print("Requesting temperatures...");
        sensors.requestTemperatures(); // Send the command to get temperatures
        Serial.println("DONE");
        // After we got the temperatures, we can print them here.
        // We use the function ByIndex, and as an example get the temperature from the first sensor only.
        float tempC = sensors.getTempCByIndex(0);

        // Check if reading was successful
        if (tempC != DEVICE_DISCONNECTED_C)
        {
            Serial.print("Temperature for the device 1 (index 0) is: ");
            Serial.println(tempC);
        }
        else
        {
            Serial.println("Error: Could not read temperature data");
        }
    }
}