/*
    Gyroscope module
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
    void loop();

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

    // TODO: Remove
    void loop()
    {
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        /* Print out the values */
        Serial.print("Acceleration X: ");
        Serial.print(a.acceleration.x);
        Serial.print(", Y: ");
        Serial.print(a.acceleration.y);
        Serial.print(", Z: ");
        Serial.print(a.acceleration.z);
        Serial.println(" m/s^2");

        Serial.print("Rotation X: ");
        Serial.print(g.gyro.x);
        Serial.print(", Y: ");
        Serial.print(g.gyro.y);
        Serial.print(", Z: ");
        Serial.print(g.gyro.z);
        Serial.println(" rad/s");

        Serial.print("Temperature: ");
        Serial.print(temp.temperature);
        Serial.println(" degC");

        Serial.println("");
    }
}