/*
    Shift register module
    Not to be used directly, should be used by high level modules

    THIS MODULE SHALL BE INITIALIZED BEFORE ALL OTHERS!

    The module automatically inverts the motor outputs as needed. A check is also made to make sure "motor_lo_speed" and "motor_hi_speed" is not enabled at the same time
*/

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
}