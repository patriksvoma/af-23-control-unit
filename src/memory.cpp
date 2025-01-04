/*
    SPI flash memory module for the W25Q32BV chip
*/

#include <Arduino.h>
#include <SPI.h>

#define SPI0_MOSI 3
#define SPI0_MISO 0
#define SPI0_SCK 2
#define SPI0_CS 1

namespace memory
{
    void enableCs();
    void disableCs();
    void init();
    void readJedecId();
    void readUniqueId();
    void enableWrite();
    void writeData(uint32_t address, uint8_t data);
    uint8_t readData(uint32_t address);

    SPISettings spi_settings(10000, MSBFIRST, SPI_MODE0);

    /// @brief Enables the CS pin (by pulling it low)
    void enableCs()
    {
        digitalWrite(SPI0_CS, LOW);
    }

    /// @brief Disables the CS pin (by pulling it high)
    void disableCs()
    {
        digitalWrite(SPI0_CS, HIGH);
    }

    /// @brief Initializes the memory module
    void init()
    {
        Serial.println("Initializing SPI Flash memory.");

        // Set the correct SPI pins
        SPI.setRX(SPI0_MISO);
        SPI.setCS(SPI0_CS); // This is probably not needed, because the software CS pin is used
        SPI.setSCK(SPI0_SCK);
        SPI.setTX(SPI0_MOSI);
        pinMode(SPI0_CS, OUTPUT);

        // Start SPI
        SPI.begin();

        readJedecId();
        readUniqueId();

        Serial.println("Writing \"0xBA\" on address 0x0");

        writeData(0x0, 0xBA);

        Serial.println("Reading address 0x0...");

        Serial.println(readData(0x0));
    }

    /// @brief Reads the JEDEC ID of the flash memory
    void readJedecId()
    {
        uint8_t command = 0x9F;

        Serial.print("Flash JEDEC ID: ");

        // Begin the transaction
        SPI.beginTransaction(spi_settings);
        enableCs();

        uint8_t out;

        // Send the command
        SPI.transfer(command);

        // Read the response
        out = SPI.transfer(0);
        Serial.print(out, HEX);

        out = SPI.transfer(0);
        Serial.print(out, HEX);

        out = SPI.transfer(0);
        Serial.println(out, HEX);

        // End the transaction
        disableCs();
        SPI.endTransaction();
    }

    /// @brief Reads the 64-bit unique ID of the memory
    void readUniqueId()
    {
        uint8_t command = 0x4B;

        Serial.print("Flash unique ID: ");

        // Begin the transaction
        SPI.beginTransaction(spi_settings);
        enableCs();

        uint8_t out;

        // Send the command
        SPI.transfer(command);

        // Send dummy bytes
        for (int i = 0; i < 4; i++)
        {
            SPI.transfer(0);
        }

        // Read the response
        for (int i = 0; i < 8; i++)
        {
            out = SPI.transfer(0);
            Serial.print(out, HEX);
        }

        // End the line
        Serial.println();

        // End the transaction
        disableCs();
        SPI.endTransaction();
    }

    /// @brief Disables the write protection for the next write
    void enableWrite()
    {
        uint8_t command = 0x06;

        // Begin the transaction
        SPI.beginTransaction(spi_settings);
        enableCs();

        // Send the command
        SPI.transfer(command);
        
        // End the transaction
        disableCs();
        SPI.endTransaction();
    }

    /// @brief Writes a byte of data to the flash
    /// @param address 24-bit address of the data
    /// @param data Data byte
    void writeData(uint32_t address, uint8_t data)
    {
        // Disable the write protection
        enableWrite();

        uint8_t command = 0x02;

        // Begin the transaction
        SPI.beginTransaction(spi_settings);
        enableCs();

        // Send the command
        SPI.transfer(command);
        
        // Send the address from which the read operation should start
        SPI.transfer((uint8_t)(address >> 16));
        SPI.transfer((uint8_t)(address >> 8));
        SPI.transfer((uint8_t)(address));

        // Send the data
        SPI.transfer(data);
        
        // End the transaction
        disableCs();
        SPI.endTransaction();   
    }

    /// @brief Reads a byte of data
    /// @param address 24-bit address of the data byte
    /// @return Data byte
    uint8_t readData(uint32_t address)
    {
        uint8_t command = 0x03;
        uint8_t ret;

        // Begin the transaction
        SPI.beginTransaction(spi_settings);
        enableCs();

        uint8_t out;

        // Send the command
        SPI.transfer(command);

        // Send the address from which the read operation should start
        SPI.transfer((uint8_t)(address >> 16));
        SPI.transfer((uint8_t)(address >> 8));
        SPI.transfer((uint8_t)(address));

        // Read the response
        ret = SPI.transfer(0);

        // End the transaction
        disableCs();
        SPI.endTransaction();

        return ret;
    }

    /// @brief Not to be used, this is how a command should look like, probably
    void commandTemplate()
    {
        uint8_t command = 0x00;

        // Begin the transaction
        SPI.beginTransaction(spi_settings);
        enableCs();

        uint8_t out;

        // Send the command
        SPI.transfer(command);

        // Send dummy bytes
        SPI.transfer(0);

        // Read the response
        out = SPI.transfer(0);
        Serial.println(out, HEX);
        
        // End the transaction
        disableCs();
        SPI.endTransaction();
    }
}