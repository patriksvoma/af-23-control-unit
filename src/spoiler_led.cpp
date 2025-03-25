/*
    Spoiler LED module
    Interfaces a WS2812S LED strip
*/

#include <Arduino.h>
#include <FastLED.h>

#define LED_DATA 2
#define LED_COUNT 42

namespace spoilerLed
{
    void init();
    void setAnimation(uint8_t id);

    CRGB leds[LED_COUNT];

    /// @brief Initializes the spoiler LED module
    void init()
    {
        pinMode(LED_DATA, OUTPUT);

        FastLED.addLeds<WS2812B, LED_DATA, GRB>(leds, LED_COUNT);
        FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);

        FastLED.clear();
    }

    /// @brief Sets the displayed animation
    /// @param id Animation ID
    void setAnimation(uint8_t id)
    {
        switch (id)
        {
        case 0x00:
            FastLED.showColor(CRGB::Black);
            break;
        case 0x01:
            FastLED.showColor(CRGB::White);
            break;
        case 0x02:
            FastLED.showColor(CRGB::Red);
            break;
        case 0x03:
            FastLED.showColor(CRGB::Green);
            break;
        case 0x04:
            FastLED.showColor(CRGB::Blue);
            break;
        case 0x05:
            FastLED.showColor(CRGB::Yellow);
            break;
        case 0x06:
            FastLED.showColor(CRGB::Purple);
            break;
        case 0x07:
            FastLED.showColor(CRGB::Orange);
            break;
            
        default:
            break;
        }
    }
}