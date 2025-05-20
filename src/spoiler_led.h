/*
    Spoiler LED module
    Interfaces a WS2812S LED strip

    Authors:
    Jakub Aldorf
    Patrik Švoma
*/

#include <Arduino.h>

namespace spoilerLed
{
    void init();
    void setAnimation(uint8_t id);
    void updateAnimation();
}