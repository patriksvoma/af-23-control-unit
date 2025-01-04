/*
    Shift register module
    Not to be used directly, should be used by the motor and ?? modules
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
    void set_nc_1(bool newValue);
    void set_nc_2(bool newValue);
}