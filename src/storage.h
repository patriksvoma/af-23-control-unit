/*
    SPI flash memory storage module for the W25Q32BV chip

    IMPORTANT:  There is a bug in the "Adafruit_SPIFlash" library as of version 5.0.1!
                On line 64 of "Adafruit_SPIFlashBase.cpp" the platform check needs to be disabled in order for it to work!

    Authors:
    Patrik Švoma
    Jakub Aldorf
*/

namespace storage
{
    void init();
    void saveTotalDistance(float distance_driven_total_m);
    void loadTotalDistance(float *distance_driven_total_m);
}