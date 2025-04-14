/*
    SPI flash memory storage module for the W25Q32BV chip

    IMPORTANT:  There is a bug in the "Adafruit_SPIFlash" library!
                On line 64 of "Adafruit_SPIFlashBase.cpp" the platform check needs to be disabled in order for it to work!
*/

namespace storage
{
    void init();
    uint8_t testRead();
    void saveTotalDistance(float distance_driven_total_m);
    void loadTotalDistance(float *distance_driven_total_m);
}