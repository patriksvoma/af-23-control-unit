/*
    BMS communication module

    Communicates with JBD-DP24S002 using UART

    Protocol: https://cdn.shopifycdn.net/s/files/1/0606/5199/5298/files/JDB_RS485-RS232-UART-Bluetooth-Communication_Protocol.pdf?v=1682577935

    Authors:
    Patrik Švoma
    Jakub Aldorf
*/

#include <Arduino.h>

namespace bms
{
    void init();
    void sendReceiveBasicInfo();
    void sendReceiveCellVoltage();
    void populateBatteryData(uint8_t *batteryData);

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
        uint16_t *temperatures;

    public:
        void process(char *dataContent);
        void printOut();
    };
    struct CellVoltages
    {
    public:
        uint8_t cellCount;
        uint16_t *voltages;

    public:
        void process(char *dataContent, uint8_t dataLength);
        void printOut();
    };

    extern BasicInfo basicInfo;
    extern CellVoltages cellVoltages;
}