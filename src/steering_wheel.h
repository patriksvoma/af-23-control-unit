/*
    Steering wheel module
*/

namespace steeringWheel
{
  void init();
    bool testConnectionPin();
    bool testConnectionSerial();
    bool process();
    
    uint8_t calculateCRC(uint8_t* data, size_t length);
    void sendPacket(uint8_t packetType, uint8_t* data, size_t dataLength);
    bool readPacket();
    void handlePacket(uint8_t packetType, uint8_t* data, size_t dataLength, uint32_t timestamp);
    void sendAckPacket();
    void sendBatteryData();
    void sendDashboardData();
}