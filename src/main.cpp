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

Bluetooth bluetooth;
RideStats rideStats;

void setup()
{
    Serial.begin(9600);
    delay(2000);

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
    steeringWheel::process();

    delay(50);
}