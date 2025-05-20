/*
    Main control unit program

    Authors:
    Jakub Aldorf
    Patrik Švoma
*/

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
#include <ota.h>

#define FAN_CHECK_DELAY 1000
#define FAN_TEMPERATURE_THRESHOLD 30

Bluetooth bluetooth;
RideStats rideStats;

uint32_t lastFanCheck;
uint32_t startupTime;

void setup()
{
    Serial.begin(9600);
    delay(2000);

    // TODO:    Check for a safe-mode entry into OTA mode
    //          Maybe when the brake is fully pressed and the steering wheel is disconnected

    // TODO: Replace this with a conditional safe-mode entry
    ota::enterSafe();

    // Make a delay in which OTA is available, but nothing else happens yet
    startupTime = millis();
    while (millis() - startupTime < 2000)
    {
        ota::handle();
        delay(5);
    }

    // If the brake pedal is fully pressed and the steering wheel disconnected, a 5 second delay is made
    // After the delay, conditions are checked again. If met, safe mode is entered.
/*
    const uint8_t brakeSensorPin = 26;
    const uint8_t steeringWheelConPin = 15;
    pinMode(brakeSensorPin, INPUT);
    pinMode(steeringWheelConPin, INPUT);

    if (analogRead(brakeSensorPin) == 255 && digitalRead(steeringWheelConPin) == HIGH)
    {
        Serial.println("Safe mode enter conditions met, checking again in 5 seconds");
        delay(5000);

        if (analogRead(brakeSensorPin) == 255 && digitalRead(steeringWheelConPin) == HIGH)
        {
            Serial.println("Entering safe mode.");
            ota::enterSafe();

            while (true)
            {
                ota::handle;
                delay(50);
            }
        }
        else
        {
            Serial.println("Safe mode conditions no longer met, continuing booting.");
        }
    }
*/
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

    // Handle the OTA if necessary
    ota::handle();

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

    delay(10);
}