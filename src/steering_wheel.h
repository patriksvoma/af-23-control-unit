/*
    Steering wheel communication module
    Implements packet-based protocol communication with debug output

    Authors:
    Jakub Aldorf
*/

namespace steeringWheel
{
    void init();
    bool testConnectionPin();
    bool testConnectionSerial();
    bool process();
}