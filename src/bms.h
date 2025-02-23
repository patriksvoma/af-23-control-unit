/*
    BMS communication module

    Communicates with JBD-DP24S002 using UART
*/

#include <Arduino.h>

namespace bms
{
    void init();
    void sendReceiveBasicInfo();
    
    struct BasicInfo
    {
        public:
        uint16_t totalVoltage;
        uint16_t current;
        uint16_t remainingCapacity;
        uint16_t nominalCapacity;
        uint16_t cycles;
        uint8_t productionDateDay;
        uint8_t productionDateMonth;
        uint16_t productionDateYear;
        uint16_t equilibrium;
        uint16_t equilibriumHigh;
        uint16_t protectionStatus;
        uint8_t softwareVersion;
        uint8_t rsoc;
        uint8_t fetControl;
        uint8_t batteryStringCount;
        uint8_t temperatureSensorCount;
        uint8_t* temperatures;

        public:

        void process(char* dataContent);
        void printOut();
    };
    
    extern BasicInfo basicInfo;
}