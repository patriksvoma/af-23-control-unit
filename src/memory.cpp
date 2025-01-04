/*
    SPI flash memory module for the W25Q32BV chip
*/

#include <Arduino.h>
#include <SPI.h>

#define SPI0_MOSI 3
#define SPI0_MISO 0
#define SPI0_SCK 2
#define SPI0_CS 1

// SPI commands for W25Q32BV
#define COMMAND_READ_JEDEC_ID 0x9F  // Command to read JEDEC ID
#define COMMAND_READ_UNIQUE_ID 0x4B // Command to read unique ID

namespace memory
{
    void enable_cs();
    void disable_cs();
    void init();
    void read_jedec_id();
    void read_unique_id();

    SPISettings spi_settings(10000, MSBFIRST, SPI_MODE0);

    /// @brief Enables the CS pin (by pulling it low)
    void enable_cs()
    {
        digitalWrite(SPI0_CS, LOW);
    }

    /// @brief Disables the CS pin (by pulling it high)
    void disable_cs()
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

        read_jedec_id();
        read_unique_id();
    }

    /// @brief Reads the JEDEC ID of the flash memory
    void read_jedec_id()
    {
        Serial.print("Flash JEDEC ID: ");

        // Begin the transaction
        SPI.beginTransaction(spi_settings);
        enable_cs();

        uint8_t out;

        // Send the command
        SPI.transfer(COMMAND_READ_JEDEC_ID);

        // Read the response
        out = SPI.transfer(0);
        Serial.print(out, HEX);

        out = SPI.transfer(0);
        Serial.print(out, HEX);

        out = SPI.transfer(0);
        Serial.println(out, HEX);

        // End the transaction
        disable_cs();
        SPI.endTransaction();
    }

    /// @brief Reads the 64-bit unique ID of the memory
    void read_unique_id()
    {
        Serial.print("Flash unique ID: ");

        // Begin the transaction
        SPI.beginTransaction(spi_settings);
        enable_cs();

        uint8_t out;

        // Send the command
        SPI.transfer(COMMAND_READ_UNIQUE_ID);

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
        disable_cs();
        SPI.endTransaction();
    }

    /// @brief Not to be used, this is how a command should look like, probably
    void command_template()
    {
        // Begin the transaction
        SPI.beginTransaction(spi_settings);
        enable_cs();

        uint8_t out;

        // Send the command
        SPI.transfer(COMMAND_READ_JEDEC_ID);

        // Send dummy bytes
        SPI.transfer(0);

        // Read the response
        out = SPI.transfer(0);
        Serial.println(out, HEX);
        
        // End the transaction
        disable_cs();
        SPI.endTransaction();
    }
}