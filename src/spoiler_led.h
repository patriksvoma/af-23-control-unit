/*
    Spoiler LED module
    Interfaces a WS2812S LED strip
*/

#include <Arduino.h>

namespace spoilerLed
{
    void init();
    void setAnimation(uint8_t id);
    void updateAnimation();

}