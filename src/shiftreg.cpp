/*
    Shift register module
    Not to be used directly, should be used by high level modules

    THIS MODULE SHALL BE INITIALIZED BEFORE ALL OTHERS!

    The module automatically inverts the motor outputs as needed. A check is also made to make sure "motor_lo_speed" and "motor_hi_speed" is not enabled at the same time

    Authors:
    Patrik Švoma
*/

#include <Arduino.h>

#define SIPO_DATA 7
#define SIPO_CLOCK 13
#define SIPO_LATCH 12
#define SIPO_OE 21

namespace shiftreg
{
    void init();
    void set_motor_pwr(bool newValue);
    void set_motor_foot_sw(bool newValue);
    void set_motor_brake_sw(bool newValue);
    void set_motor_reverse(bool newValue);
    void set_motor_lo_speed(bool newValue);
    void set_motor_hi_speed(bool newValue);
    void set_motor_fan(bool newValue);
    void set_gpo_0(bool newValue);
    void write();

    // Variables for storing the current state of the outputs
    // Every time the shift register is written to, the output state needs to be known
    bool motor_pwr;
    bool motor_foot_sw;
    bool motor_brake_sw;
    bool motor_reverse;
    bool motor_lo_speed;
    bool motor_hi_speed;
    bool motor_fan;
    bool gpo_0;

    /// @brief Initializes the shift register module
    void init()
    {
        pinMode(SIPO_DATA, OUTPUT);
        pinMode(SIPO_CLOCK, OUTPUT);
        pinMode(SIPO_LATCH, OUTPUT);
        pinMode(SIPO_OE, OUTPUT);

        digitalWrite(SIPO_OE, HIGH);
        digitalWrite(SIPO_DATA, LOW);
        digitalWrite(SIPO_CLOCK, LOW);
        digitalWrite(SIPO_LATCH, LOW);

        write();

        // Enable the output of the register
        digitalWrite(SIPO_OE, LOW);
    }

    void set_motor_pwr(bool newValue)
    {
        motor_pwr = newValue;
        write();
    }

    void set_motor_foot_sw(bool newValue)
    {
        motor_foot_sw = newValue;
        write();
    }

    void set_motor_brake_sw(bool newValue)
    {
        motor_brake_sw = newValue;
        write();
    }

    void set_motor_reverse(bool newValue)
    {
        motor_reverse = newValue;
        write();
    }

    void set_motor_lo_speed(bool newValue)
    {
        motor_lo_speed = newValue;
        write();
    }

    void set_motor_hi_speed(bool newValue)
    {
        motor_hi_speed = newValue;
        write();
    }

    void set_motor_fan(bool newValue)
    {
        motor_fan = newValue;
        write();
    }

    void set_gpo_0(bool newValue)
    {
        gpo_0 = newValue;
        write();
    }

    void write()
    {
        uint8_t mask = 0;

        bitWrite(mask, 0, motor_pwr);
        bitWrite(mask, 1, !motor_foot_sw);
        bitWrite(mask, 2, !motor_brake_sw);
        bitWrite(mask, 3, !motor_reverse);
        bitWrite(mask, 4, !motor_lo_speed);
        bitWrite(mask, 5, !motor_hi_speed);
        bitWrite(mask, 6, motor_fan);
        bitWrite(mask, 7, gpo_0);

        digitalWrite(SIPO_LATCH, LOW);

        shiftOut(SIPO_DATA, SIPO_CLOCK, MSBFIRST, mask);

        digitalWrite(SIPO_LATCH, HIGH);
    }
}