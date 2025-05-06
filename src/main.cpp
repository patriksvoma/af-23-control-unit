#include <Arduino.h>
#include <SerialBT.h>

#include <shiftreg.h>
#include <motor.h>
#include <gpio.h>
#include <gyroscope.h>
#include <brake_sensor.h>
#include <rtc.h>
#include <bms.h>
#include <gpio.h>
#include <indicator_led.h>
#include <spoiler_led.h>
#include <temperature.h>
#include <steering_wheel.h>
#include <storage.h>
#include <bluetooth.h>
#include <rideStats.h>

#define FAN_CHECK_DELAY 1000
#define FAN_TEMPERATURE_THRESHOLD 30

Bluetooth bluetooth;
RideStats rideStats;

uint32_t lastFanCheck;

void setup()
{
    Serial.begin(9600);
    delay(2000);

    // TODO:    Check for a safe-mode entry into OTA mode
    //          Maybe when the brake is fully pressed and the steering wheel is disconnected

    // Initialize modules
    shiftreg::init();
    motor::init();
    gpio::init();
    gyroscope::init();
    brakeSensor::init();
    rtc::init();
    bms::init();
    gpio::init();
    indicatorLed::init();
    spoilerLed::init();
    temperature::init();
    steeringWheel::init();
    storage::init();

    rideStats.init();
    bluetooth.init();

    Serial.println("Module setup finished");

    shiftreg::set_motor_pwr(true);
    spoilerLed::setAnimation(0);
}

void loop()
{
    bluetooth.readPacket();
    rideStats.update();
    spoilerLed::updateAnimation();
    steeringWheel::process();

    // Turn on the motor controller fan if the temperature is more than the threshold
    // Only check the temperature every X milliseconds
    if (millis() - lastFanCheck > FAN_CHECK_DELAY)
    {
        if (temperature::readTemperature(TEMP_MOTOR_CONTROLLER) > FAN_TEMPERATURE_THRESHOLD)
        {
            shiftreg::set_motor_fan(true);
        }
        else
        {
            shiftreg::set_motor_fan(false);
        }

        lastFanCheck = millis();
    }

    // TODO MAYBE RIDE HISTORY
    //  GET THE DATA
    //  WRITE THEM INTO RIDE DATA INTO MEMORY EVERY 1s or so
    //  READ DATA REQUEST and send the response

    // Allow rebooting via USB serial
    if (Serial.available() > 0)
    {
        String command = Serial.readString();

        if (command == "reb\r\n")
        {
            Serial.println("Rebooting");
            rp2040.reboot();
        }
    }

    delay(50);
}