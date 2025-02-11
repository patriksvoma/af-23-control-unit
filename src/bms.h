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
        uint16_t totalVoltage = 0;
        uint16_t current;
        uint16_t remainingCapacity;
        uint16_t nominalCapacity;
        uint16_t cycles;
        uint16_t productionDate;
        uint16_t equilibrium;
        uint16_t equilibriumHigh;
        uint16_t protectionStatus;
        uint8_t softwareVersion;
        uint8_t rsoc;
        uint8_t fetControl;
        uint8_t batteryStringCount;
        uint8_t ntcCount;
        uint64_t ntcContent;

        public:

        void process(char* dataContent);
        void printOut();
    };
    
    extern BasicInfo basicInfo;
}