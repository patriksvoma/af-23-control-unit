/*
    DS18B20 temperature sensor module
*/

#include <temperature.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 6
#define RESOLUTION 10

namespace temperature
{
    void init();
    void search();
    float readTemperature(uint8_t sensorId);
    
    OneWire oneWire(ONE_WIRE_BUS);
    DallasTemperature sensors(&oneWire);
    DeviceAddress sensor0 = {0x28, 0x88, 0x88, 0x45, 0xD4, 0x02, 0x65, 0x8D};   // MOTOR CONTROLLER
    DeviceAddress sensor1 = {0x28, 0x02, 0x8B, 0x45, 0xD4, 0x15, 0x22, 0xA1};   // OUTSIDE
    DeviceAddress sensor2 = {0x28, 0xA2, 0xF3, 0x45, 0xD4, 0xE0, 0x66, 0xEF};   // MOTOR
    DeviceAddress sensor3 = {0x28, 0xDA, 0xCB, 0x1B, 0x10, 0x00, 0x00, 0xA1};   // CONTROL UNIT
    DeviceAddress sensor4 = {0x28, 0x31, 0x6C, 0x45, 0xD4, 0xA3, 0x6D, 0x31};   // BRAKE

    /// @brief Initializes the temperature module
    void init()
    {
        pinMode(ONE_WIRE_BUS, INPUT);

        sensors.begin();
        sensors.setResolution(RESOLUTION);
    }

    /// @brief Searches for all temperature sensors and prints their adresses to USB serial
    void search()
    {
        bool searching = true;

        oneWire.reset_search();

        while (searching)
        {
            uint8_t foundAdress[8];
            if (oneWire.search(foundAdress))
            {
                Serial.print("Found adress: ");
                for (int i = 0; i < 8; i++)
                {
                    Serial.print("0x");
                    Serial.print(foundAdress[i], HEX);
                    if (i < 7) Serial.print(", ");
                }
                Serial.println();
            }
            else
            {
                Serial.println("Search complete");
                searching = false;
            }
        }
    }

    /// @brief Reads the temperature of a sensor. Automatically requests a temperature reading, unless specified.
    /// @param sensorId ID of the sensor (0 or 1)
    /// @return Temperature in celsius
    float readTemperature(uint8_t sensorId)
    {
        DeviceAddress* selectedDevice;
        
        switch (sensorId)
        {
        case TEMP_MOTOR_CONTROLLER:
            selectedDevice = &sensor0;
            break;
        case TEMP_OUTSIDE:
            selectedDevice = &sensor1;
            break;
        case TEMP_MOTOR:
            selectedDevice = &sensor2;
            break;
        case TEMP_CONTROL_UNIT:
            selectedDevice = &sensor3;
            break;
        case TEMP_BRAKE:
            selectedDevice = &sensor4;
            break;
        default:
            return -127;
        }

        // Request temperature and read it
        sensors.requestTemperaturesByAddress(*selectedDevice);
        float tempC = sensors.getTempC(*selectedDevice);

        // If the temperature equals this, the sensor is not connected
        if (tempC == DEVICE_DISCONNECTED_C)
        {
            Serial.println("Reading temperature not successfull, device disconnected");
        }

        return tempC;
    }
}