#include <Arduino.h>
#include <shiftreg.h>
#include <gyroscope.h>
#include <memory.h>

void setup()
{
    Serial.begin();
    delay(6000);
    Serial.println("Starting");

    // Initialize modules
    //shiftreg::init();
    //gyroscope::init();
    memory::init();

    //pinMode(15, INPUT);

    //pinMode(10, OUTPUT);
    //analogWrite(10, 255);

    Serial.println("Setup finished");
}

void loop()
{
    //Serial.println(digitalRead(15));

    //gyroscope::loop();

    //shiftreg::set_motor_pwr(true);
    //shiftreg::set_motor_foot_sw(true);
    //shiftreg::set_motor_brake_sw(true);
    //shiftreg::set_motor_reverse(true);
    //shiftreg::set_motor_lo_speed(true);
    //shiftreg::set_motor_hi_speed(true);
    //shiftreg::set_nc_1(true);
    //shiftreg::set_nc_2(true);

    delay(200);
}