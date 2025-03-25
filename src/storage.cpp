/*
    SPI flash memory storage module for the W25Q32BV chip

    IMPORTANT:  There is a bug in the "Adafruit_SPIFlash" library!
                On line 64 of "Adafruit_SPIFlashBase.cpp" the platform check needs to be disabled in order for it to work!
*/

#include <Arduino.h>

#include <LittleFS.h>
#include <Adafruit_SPIFlash.h>
#include "flash_config.h"

#define SPI0_MOSI 19
#define SPI0_MISO 16
#define SPI0_SCK 18
#define SPI0_CS 17

#define BLOCK_SIZE 4096 // Block size in bytes
#define FLASH_SIZE (4 * 1024 * 1024) // Flash size in bytes

namespace storage
{
    // TODO: Remove
    bool testReadSuccessful = false;

    void init();
    uint8_t testRead();
    int eraseChip();
    int lfs_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset, void *buffer, lfs_size_t size);
    int lfs_write(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset, const void *buffer, lfs_size_t size);
    int lfs_erase(const struct lfs_config *c, lfs_block_t block);
    int lfs_sync(const struct lfs_config *c);

    // SPIFlash config
    Adafruit_SPIFlash flash(&flashTransport);
    static const SPIFlash_Device_t my_flash_devices[] = {
        W25Q32BV,
    };
    const int flashDevices = 1;

    // LittleFS
    lfs_t lfs;
    // LittleFS config
    struct lfs_config lfs_cfg = {
        .read = lfs_read,
        .prog = lfs_write,
        .erase = lfs_erase,
        .sync = lfs_sync,
        .read_size = 1,
        .prog_size = 1,
        .block_size = BLOCK_SIZE,
        .block_count = FLASH_SIZE / BLOCK_SIZE,
        .block_cycles = 400,
        .cache_size = 256,
        .lookahead_size = 16,
    };

    /// @brief Initializes the storage module
    void init()
    {
        Serial.println("Initializing SPI Flash memory.");

        // Set the correct SPI pins
        SPI.setRX(SPI0_MISO);
        SPI.setSCK(SPI0_SCK);
        SPI.setTX(SPI0_MOSI);
        
        if (!flash.begin(my_flash_devices, flashDevices)) {
            Serial.println("Failed to initialize external SPI flash!");
            return;
        }
        Serial.println("Flash memory initialized.");

        // Print out the JEBED id of the connected flash
        Serial.print("Flash JEBEC ID: 0x");
        Serial.print(flash.getJEDECID(), HEX);
        Serial.println(", expected value: 0xEF4016.");

        // Mount LittleFS
        Serial.println("Initializing LittleFS.");
        Serial.println("Mounting");
        if (lfs_mount(&lfs, &lfs_cfg) != 0)
        {
            // If mounting fails, format and try to mount again
            Serial.println("Mounting LittleFS failed, formatting and mounting again.");
            Serial.println(lfs_format(&lfs, &lfs_cfg));
            flash.waitUntilReady();
            if (lfs_mount(&lfs, &lfs_cfg)!= 0)
            {
                Serial.println("Mounting LittleFS failed again. Stopping execution!");
                while (true);
            }
        }
        Serial.println("LittleFS mounted successfully.");

        //lfs_remove(&lfs, "/test.txt");

        // Write to a file
        Serial.println("Testing file write.");
        const char* writeBuf = "CAU CAU CAU ALDYGAMES";

        lfs_file_t file;
        lfs_file_open(&lfs, &file, "/test2.txt", LFS_O_RDWR | LFS_O_CREAT);

        //lfs_file_write(&lfs, &file, writeBuf, strlen(writeBuf));

        lfs_file_close(&lfs, &file);
        Serial.println("File closed.");

        // Test file read
        // test.txt, test1.txt, test2.txt should be readable
        char readBuf[22];

        Serial.println("Testing file read.");
        lfs_file_open(&lfs, &file, "/test.txt", LFS_O_RDWR | LFS_O_CREAT);

        lfs_file_read(&lfs, &file, &readBuf, sizeof(readBuf));

        lfs_file_close(&lfs, &file);

        Serial.print("File closed, read: ");
        Serial.println(readBuf);

        if (readBuf[0] == 'T')
        {
            Serial.println("File read first letter matches.");
            testReadSuccessful = true;
        }

        // Unmount LittleFS
        lfs_unmount(&lfs);
    }

    // Test reads a single character
    uint8_t testRead()
    {
        return testReadSuccessful;

        lfs_file_t file;
        char readBuf[1];

        Serial.println("Testing file read");
        lfs_file_open(&lfs, &file, "/test.txt", LFS_O_RDWR | LFS_O_CREAT);

        lfs_file_read(&lfs, &file, &readBuf, sizeof(readBuf));

        lfs_file_close(&lfs, &file);

        return readBuf[0];
    }
    
    /// @brief Erases the whole chip
    int eraseChip()
    {
        Serial.println("Erasing the whole chip.");
        flash.eraseChip();
        flash.waitUntilReady();
        Serial.println("Chip erased");

        return 0;
    }

    /// @brief LittleFS uses this function to read data from the flash
    /// @param c LittleFS config
    /// @param block Index of the memory block
    /// @param offset Offset of the memory address in the block
    /// @param buffer Buffer to which the read data should be written
    /// @param size Size of the data to read
    /// @return Error code 0
    int lfs_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset, void *buffer, lfs_size_t size)
    {
        // Get the address from which to start reading
        uint32_t addr = (block * BLOCK_SIZE) + offset;

        // Read the buffer
        flash.readBuffer(addr, (uint8_t*)buffer, size);

        return 0;
    }

    /// @brief LittleFS uses this function to write data to the flash
    /// @param c LittleFS config
    /// @param block Index of the memory block
    /// @param offset Offset of the memory address in the block
    /// @param buffer Buffer where the data is stored
    /// @param size Size of the buffer
    /// @return Error code 0
    int lfs_write(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset, const void *buffer, lfs_size_t size)
    {
        // Get the address from which to start reading
        uint32_t addr = (block * BLOCK_SIZE) + offset;

        // Write the data
        //for (lfs_size_t i = 0; i < size; i++)
        //{
        //    uint8_t writeBuf = ((const uint8_t*)buffer)[i];
        //    flash.writeBuffer(addr + i, &writeBuf, sizeof(writeBuf));
        //}

        // Write the data
        flash.writeBuffer(addr, (const uint8_t*) buffer, size);
        
        return 0;
    }

    /// @brief LittleFS uses this function to erase blocks of flash memory
    /// @param c LittleFS config
    /// @param block Index of the memory block
    /// @return Error code 0
    int lfs_erase(const struct lfs_config *c, lfs_block_t block)
    {
        flash.eraseBlock(block);

        return 0;
    }

    /// @brief This function is called by LittleFS to sync the state of the flash memory. This doesn't need to be done here, but the function has to exist and return code 0
    /// @param c LittleFS config
    /// @return Error code 0
    int lfs_sync(const struct lfs_config *c)
    {
        return 0;
    }
}