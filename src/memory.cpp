/*
    SPI flash memory module for the W25Q32BV chip

    IMPORTANT:  There is a bug in the "Adafruit_SPIFlash" library!
                On line 64 of "Adafruit_SPIFlashBase.cpp" the platform check needs to be disabled in order for it to work!
*/

#include <Arduino.h>

#include <Adafruit_SPIFlash.h>
#include "flash_config.h"

#define SPI0_MOSI 3
#define SPI0_MISO 0
#define SPI0_SCK 2
#define SPI0_CS 1

namespace memory
{
    void init();

    Adafruit_SPIFlash flash(&flashTransport);
    static const SPIFlash_Device_t my_flash_devices[] = {
        W25Q32BV,
    };
    const int flashDevices = 1;

    /// @brief Initializes the memory module
    void init()
    {
        Serial.println("Initializing SPI Flash memory.");

        // Set the correct SPI pins
        SPI.setRX(SPI0_MISO);
        SPI.setSCK(SPI0_SCK);
        SPI.setTX(SPI0_MOSI);
        
        if (!flash.begin(my_flash_devices, flashDevices)) {
            Serial.println("Failed to initialize external SPI flash!");
            while (1);
        }

        uint32_t jedec_id = flash.getJEDECID();
        Serial.print("JEDEC ID: 0x");
        Serial.println(jedec_id, HEX);
        Serial.print("Flash size (usable): ");
        Serial.print(flash.size() / 1024);
        Serial.println(" KB");

        Serial.println("numPages");
        Serial.println(flash.numPages());
        Serial.println("pageSize");
        Serial.println(flash.pageSize());
        Serial.println("sectorCount");
        Serial.println(flash.sectorCount());

        Serial.println("Reading 8");
        Serial.println(flash.read8(0x0), HEX);

        Serial.println("Erasing sector 0");
        if (!flash.eraseSector(0x0)) Serial.println("Erasing sector failed!");
        flash.waitUntilReady();
        Serial.println("Reading 8");
        Serial.println(flash.read8(0x0), HEX);

        uint8_t writeBuf1 = 0xB2;

        Serial.println("Writing 0x0");
        flash.writeBuffer(0x0, &writeBuf1, sizeof(writeBuf1));
        flash.waitUntilReady();
        Serial.println("Reading 8");
        Serial.println(flash.read8(0x0), HEX);
        
        Serial.println("Complete");
    }
}