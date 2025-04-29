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
    const char* TOTAL_DISTANCE_FILE = "/total_distance.dat";


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
        int mountResult = lfs_mount(&lfs, &lfs_cfg);
        if (mountResult != 0)
        {
            // If mounting fails, format and try to mount again
            Serial.println("Mounting LittleFS failed, formatting and mounting again.");
            int formatResult = lfs_format(&lfs, &lfs_cfg);
            Serial.print("Format result: ");
            Serial.println(formatResult);
            if (formatResult != 0) {
                Serial.println("Formatting failed. Stopping execution!");
                while (true);
            }
            flash.waitUntilReady();
            mountResult = lfs_mount(&lfs, &lfs_cfg);
            if (mountResult != 0)
            {
                Serial.print("Mounting LittleFS failed again with error: ");
                Serial.println(mountResult);
                Serial.println("Stopping execution!");
            }
        }
        Serial.println("LittleFS mounted successfully.");
    
         
    
       
       
    
       
    
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

    void loadTotalDistance(float *distance_driven_total_m) {
        lfs_file_t file;
        
        // Debug print
        Serial.println("Loading total distance...");
        
        // Mount LittleFS
        if (lfs_mount(&storage::lfs, &storage::lfs_cfg) != 0) {
            Serial.println("Failed to mount LittleFS for reading total distance");
            *distance_driven_total_m = 0; // Set to 0 if we can't read
            return;
        }
        
        // Try to open the file
        int open_result = lfs_file_open(&storage::lfs, &file, TOTAL_DISTANCE_FILE, LFS_O_RDONLY);
        if (open_result == 0) {
            // Read the distance value
            int read_result = lfs_file_read(&storage::lfs, &file, distance_driven_total_m, sizeof(float));
            if (read_result != sizeof(float)) {
                Serial.printf("Error reading total distance file. Read %d bytes, expected %d bytes\n", 
                              read_result, (int)sizeof(float));
                *distance_driven_total_m = 0;
            } else {
                Serial.printf("Loaded total distance: %.2f meters\n", *distance_driven_total_m);
            }
            lfs_file_close(&storage::lfs, &file);
        } else {
            // File doesn't exist yet, initialize to 0
            Serial.printf("File doesn't exist (error code: %d). Initializing to 0\n", open_result);
            *distance_driven_total_m = 0;
        }
        
        // Unmount LittleFS
        lfs_unmount(&storage::lfs);
        
        // Additional sanity check for loaded value
        if (*distance_driven_total_m < 0 || *distance_driven_total_m > 1000000) {
            Serial.printf("Warning: Loaded distance value looks invalid: %.2f. Resetting to 0\n", 
                         *distance_driven_total_m);
            *distance_driven_total_m = 0;
        }
    }
    
    void saveTotalDistance(float distance_driven_total_m) {
        lfs_file_t file;
        
        // Debug print
        Serial.printf("Saving total distance: %.2f meters\n", distance_driven_total_m);
        
        // Mount LittleFS
        if (lfs_mount(&storage::lfs, &storage::lfs_cfg) != 0) {
            Serial.println("Failed to mount LittleFS for saving total distance");
            return;
        }
        
        // Open or create the file
        int open_result = lfs_file_open(&storage::lfs, &file, TOTAL_DISTANCE_FILE, 
                                        LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
        if (open_result == 0) {
            // Write the distance value
            int write_result = lfs_file_write(&storage::lfs, &file, &distance_driven_total_m, sizeof(float));
            if (write_result != sizeof(float)) {
                Serial.printf("Error writing total distance file. Wrote %d bytes, expected %d bytes\n", 
                             write_result, (int)sizeof(float));
            } else {
                Serial.println("Total distance saved successfully");
            }
            lfs_file_close(&storage::lfs, &file);
        } else {
            Serial.printf("Failed to open total distance file for writing (error code: %d)\n", open_result);
        }
        
        // Unmount LittleFS - this was missing in your original code!
        lfs_unmount(&storage::lfs);
    }
}