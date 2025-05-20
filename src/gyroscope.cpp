/*
    Gyroscope module

    Authors:
    Patrik Švoma
*/

#include <Wire.h>
#include <Adafruit_MPU6050.h>

#define I2C0_SDA 4
#define I2C0_SCL 5

namespace gyroscope
{
    const int MPU_address = 0x69;
    Adafruit_MPU6050 mpu;

    void init();
    sensors_event_t getAcceleration();
    sensors_event_t getRotation();
    sensors_event_t getTemperature();

    /// @brief Initializes the gyroscope module
    void init()
    {
        Wire.setSDA(I2C0_SDA);
        Wire.setSCL(I2C0_SCL);

        mpu.begin(MPU_address);
        mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
        mpu.setGyroRange(MPU6050_RANGE_500_DEG);
        mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
    }

    /// @brief Gets the current acceleration of the sensor
    /// @return Sensor event
    sensors_event_t getAcceleration()
    {
        sensors_event_t acceleration, rotation, temp;

        mpu.getEvent(&acceleration, &rotation, &temp);

        return acceleration;
    }

    /// @brief Gets the current rotation of the sensor
    /// @return Sensor event
    sensors_event_t getRotation()
    {
        sensors_event_t acceleration, rotation, temp;

        mpu.getEvent(&acceleration, &rotation, &temp);

        return rotation;
    }

    /// @brief Gets the current temperature of the sensor
    /// @return Sensor event
    sensors_event_t getTemperature()
    {
        sensors_event_t acceleration, rotation, temp;

        mpu.getEvent(&acceleration, &rotation, &temp);

        return temp;
    }
}