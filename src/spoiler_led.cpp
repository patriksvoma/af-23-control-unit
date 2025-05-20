/*
    Spoiler LED module
    Interfaces a WS2812S LED strip

    Authors:
    Jakub Aldorf
    Patrik Švoma
*/

#include <Arduino.h>
#include <FastLED.h>
#include <brake_sensor.h>

#define LED_DATA 2
#define LED_COUNT 41
#define LED_COUNT_HALF LED_COUNT / 2

namespace spoilerLed
{
    void init();
    void setAnimation(uint8_t id);
    void anim_Driving();
    void anim_Braking();
    void anim_HardBraking();
    void anim_AnalogMiddle();
    void anim_Startup();
    void anim_SideToSide();
    void anim_MiddleToSides();
    void animBlinkerLeft();
    void animBlinkerRight();
    void animHazards();
    void animMiddleToSides();
    void animChristmasTree();

    void modifyStripColorBy(int colorDelta);
    void modifyStripColorBy(int redDelta, int greenDelta, int blueDelta);
    void modifyLedColorBy(int led, int colorDelta);
    void modifyLedColorBy(int led, int redDelta, int greenDelta, int blueDelta);

    CRGB leds[LED_COUNT];

    /// @brief The time it takes to switch to the idle animation when the brake is not pressed
    const int idleTimeout = 30 * 1000;
    /// @brief How much the brake needs to be pressed for it to be considered braking
    const uint brakingTreshold = 200;
    /// @brief How much the brake needs to be pressed for it to be considered hard braking
    const uint hardBrakingTreshold = 800;
    /// @brief The value the sensor reads when the brake pedal is fully released
    uint releasedVoltage;
    /// @brief The value the sensor reads when the brake pedal is fully pressed
    uint pressedVoltage = 1023;
    uint activeAnimation;

    long hardBraking_flashDelay = 200;
    long hardBraking_lastChangeMillis = 0;
    int hardBraking_stage = 0;

    const int startup_repeatCount = 2;
    const int startup_moveDelay = 50;
    const int startup_decayDelay = 40;
    long startup_lastMoveMillis = 0;
    long startup_lastDecayMillis = 0;
    bool startup_increasing = true;
    int startup_currentPixel = 0;
    int startup_timesCompleted = 0;

    const CRGB sideToSide_color = CRGB::Red;
    const int sideToSide_moveDelay = 100;
    const int sideToSide_decayDelay = 20;
    long sideToSide_lastMoveMillis = 0;
    long sideToSide_lastDecayMillis = 0;
    bool sideToSide_increasing = true;
    int sideToSide_currentPixel = 0;
    int current_anim = 0;

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
        current_anim = id;
    }

    void updateAnimation()
    {
        if (brakeSensor::readPressureRaw() > 800)
        {
            current_anim = 1;
            if (brakeSensor::readPressureRaw() > 2400)
            {
                current_anim = 2;
            }
        }
        else if ((current_anim == 2 || current_anim == 1))
        {
            current_anim = 0;
        }

        switch (current_anim)
        {
        case 0x00:
            anim_Driving();
            break;
        case 0x01:
            anim_Braking();
            break;
        case 0x02:
            anim_HardBraking();
            break;
        case 0x03:
            anim_AnalogMiddle();
            break;
        case 0x04:
            anim_SideToSide();
            break;
        case 0x05:
            anim_Startup();
            break;
        case 0x06: // App braking
            FastLED.showColor(CRGB::Red);
            break;
        case 0x07: // App Hard braking
            FastLED.showColor(CRGB::Orange);
            break;
        case 0x08:
            animBlinkerLeft();
            break;
        case 9:
            animBlinkerRight();
            break;
        case 10:
            animHazards();
            break;
        case 11:
            animMiddleToSides();
            break;
        case 12:
            animChristmasTree();
            break;
        default:
            break;
        }
    }

    void modifyStripColorBy(int colorDelta)
    {
        modifyStripColorBy(colorDelta, colorDelta, colorDelta);
    }

    /// @brief Modifies the color of all LEDs on the strip
    /// @param redDelta The red value of each LED will be modified by this value
    /// @param greenDelta The green value of each LED will be modified by this value
    /// @param blueDelta The blue value of each LED will be modified by this value
    void modifyStripColorBy(int redDelta, int greenDelta, int blueDelta)
    {
        for (int i = 0; i < LED_COUNT; i++)
        {
            modifyLedColorBy(i, redDelta, greenDelta, blueDelta);
        }
    }
    /// @brief Modifies the colors of a single LED
    /// @param led Index of the LED to modify
    /// @param colorDelta All of the colors will be modified by this value
    void modifyLedColorBy(int led, int colorDelta)
    {
        modifyLedColorBy(led, colorDelta, colorDelta, colorDelta);
    }

    /// @brief Modifies the colors of a single LED
    /// @param led Index of the LED to modify
    /// @param redDelta The red value of the LED will be modified by this value
    /// @param greenDelta The green value of the LED will be modified by this value
    /// @param blueDelta The blue value of the LED will be modified by this value
    void modifyLedColorBy(int led, int redDelta, int greenDelta, int blueDelta)
    {
        // Copy the LED color values
        int red = leds[led].red;
        int green = leds[led].green;
        int blue = leds[led].blue;

        // Modify the values
        red += redDelta;
        green += greenDelta;
        blue += blueDelta;

        // Clamp the values
        if (red > 255)
            red = 250;
        if (green > 255)
            green = 250;
        if (blue > 255)
            blue = 250;

        if (red < 0)
            red = 0;
        if (green < 0)
            green = 0;
        if (blue < 0)
            blue = 0;

        // Save the values onto the LED
        leds[led].red = red;
        leds[led].green = green;
        leds[led].blue = blue;
    }

    /// @brief Shows red stripes on sides
    void anim_Driving()
    {
        // Clear the LEDs
        FastLED.clear();

        // Set the LEDs on the ends to red
        for (int i = 0; i < 5; i++)
        {
            leds[i] = CRGB::Red;
        }

        for (int i = LED_COUNT - 1; i > LED_COUNT - 6; i--)
        {
            leds[i] = CRGB::Red;
        }

        FastLED.show();
    }

    /// @brief Shows full red
    void anim_Braking()
    {
        FastLED.showColor(CRGB::Red);
    }

    /// @brief Flashes between full red and stripes on sides
    void anim_HardBraking()
    {
        if (millis() - hardBraking_flashDelay > hardBraking_lastChangeMillis)
        {
            if (hardBraking_stage == 0)
            {
                FastLED.showColor(CRGB::Red);

                hardBraking_stage = 1;
            }
            else if (hardBraking_stage == 1)
            {
                FastLED.clear();

                // Set the LEDs on the ends to red
                for (int i = 0; i < 5; i++)
                {
                    leds[i] = CRGB::Red;
                }

                for (int i = LED_COUNT - 1; i > LED_COUNT - 6; i--)
                {
                    leds[i] = CRGB::Red;
                }

                FastLED.show();

                hardBraking_stage = 0;
            }

            hardBraking_lastChangeMillis = millis();
        }
    }

    /// @brief Shows a red bar that increases from the middle with the brake pedal being pressed
    void anim_AnalogMiddle()
    {
        FastLED.clear();

        for (int i = 0; i < map(long(brakeSensor::readPressureMPa), 0, 1000, 0, LED_COUNT_HALF); i++)
        {
            leds[LED_COUNT_HALF + i] = CRGB::Red;
            leds[LED_COUNT_HALF - i + 1] = CRGB::Red;
        }

        FastLED.show();
    }

    /// @brief Shows a green side-to-side animation three times, then automatically switches to the driving animation
    void anim_Startup()
    {
        // Moving the current pixel is detached from the updating and decay to make the animation smoother
        if (millis() - startup_lastMoveMillis > startup_moveDelay)
        {
            if (startup_increasing)
            {
                startup_currentPixel++;
                if (startup_currentPixel == LED_COUNT)
                    startup_increasing = false;
            }
            else
            {
                startup_currentPixel--;
                if (startup_currentPixel == -1)
                {
                    startup_increasing = true;
                    startup_timesCompleted++;
                    if (startup_timesCompleted == startup_repeatCount)
                    {
                        startup_timesCompleted = 0;
                        // TODO: Change to driving animation automatically
                        setAnimation(0);
                    }
                }
            }

            startup_lastMoveMillis = millis();
        }

        if (millis() - startup_lastMoveMillis > startup_decayDelay)
        {
            modifyStripColorBy(-5);
            leds[startup_currentPixel] = CRGB::Green;
            FastLED.show();

            startup_lastDecayMillis = millis();
        }
    }

    /// @brief Shows a red side-to-side animation
    void anim_SideToSide()
    {
        if (millis() - sideToSide_lastMoveMillis > sideToSide_moveDelay)
        {
            if (sideToSide_increasing)
            {
                sideToSide_currentPixel++;
                if (sideToSide_currentPixel == LED_COUNT)
                    sideToSide_increasing = false;
            }
            else
            {
                sideToSide_currentPixel--;
                if (sideToSide_currentPixel == -1)
                    sideToSide_increasing = true;
            }

            sideToSide_lastMoveMillis = millis();
        }

        if (millis() - sideToSide_lastDecayMillis > sideToSide_decayDelay)
        {
            modifyStripColorBy(-5);
            leds[sideToSide_currentPixel] = sideToSide_color;
            FastLED.show();

            sideToSide_lastDecayMillis = millis();
        }
    }
    u32_t middleToSides_lastMoveMillis = 0;
    u32_t middleToSides_lastDecayMillis = 0;
    int middleToSides_moveDelay = 50;
    int middleToSides_decayDelay = 20;
    int middleToSides_leftPixel;
    int middleToSides_rightPixel;
    bool middleToSides_expanding = true;
    CRGB middleToSides_color = CRGB::Blue;

    /// @brief Shows a blue middle-to-sides animation
    void animMiddleToSides()
    {
        if (middleToSides_lastMoveMillis == 0)
        {
            middleToSides_leftPixel = LED_COUNT / 2;
            middleToSides_rightPixel = LED_COUNT / 2;
            middleToSides_expanding = true;
        }

        if (millis() - middleToSides_lastMoveMillis > middleToSides_moveDelay)
        {
            if (middleToSides_expanding)
            {
                middleToSides_leftPixel--;
                middleToSides_rightPixel++;

                if (middleToSides_leftPixel < 0 || middleToSides_rightPixel >= LED_COUNT)
                {
                    middleToSides_expanding = false;
                    middleToSides_leftPixel = max(0, middleToSides_leftPixel);
                    middleToSides_rightPixel = min(LED_COUNT - 1, middleToSides_rightPixel);
                }
            }
            else
            {
                middleToSides_leftPixel++;
                middleToSides_rightPixel--;

                if (middleToSides_leftPixel >= middleToSides_rightPixel)
                {
                    middleToSides_expanding = true;
                    middleToSides_leftPixel = LED_COUNT / 2;
                    middleToSides_rightPixel = LED_COUNT / 2;
                }
            }

            middleToSides_lastMoveMillis = millis();
        }

        if (millis() - middleToSides_lastDecayMillis > middleToSides_decayDelay)
        {
            modifyStripColorBy(-5);

            leds[middleToSides_leftPixel] = middleToSides_color;
            leds[middleToSides_rightPixel] = middleToSides_color;

            FastLED.show();

            middleToSides_lastDecayMillis = millis();
        }
    }
    u32_t blinker_started;
    u32_t blinker_change;
    bool blinker_state = true;
    const int firstLed = LED_COUNT - 5;
    const int lastLed = 4;

    void animBlinkerLeft()
    {
        if (blinker_started == 0)
        {
            blinker_started = millis();
            blinker_change = millis();
        }
        if (millis() - blinker_change > 500)
        {
            blinker_change = millis();
            blinker_state = !blinker_state;
            for (int i = firstLed; i < LED_COUNT; i++)
            {
                leds[i] = blinker_state ? CRGB::Orange : CRGB::Black;
            }
            FastLED.show();
        }

        if (millis() - blinker_started > 6250)
        {
            blinker_started = 0;
            blinker_change = 0;
            blinker_state = true;

            setAnimation(0);
        }
    }

    void animBlinkerRight()
    {

        if (blinker_started == 0)
        {
            blinker_started = millis();
            blinker_change = millis();
        }
        if (millis() - blinker_change > 500)
        {
            blinker_change = millis();
            blinker_state = !blinker_state;
            for (int i = 0; i <= lastLed; i++)
            {
                leds[i] = blinker_state ? CRGB::Orange : CRGB::Black;
            }
            FastLED.show();
        }

        if (millis() - blinker_started > 6250)
        {
            blinker_started = 0;
            blinker_change = 0;
            blinker_state = true;

            setAnimation(0);
        }
    }

    void animHazards()
    {

        if (blinker_started == 0)
        {
            blinker_started = millis();
            blinker_change = millis();
        }
        if (millis() - blinker_change > 500)
        {
            blinker_change = millis();
            blinker_state = !blinker_state;
            for (int i = 0; i <= lastLed; i++)
            {
                leds[i] = blinker_state ? CRGB::Orange : CRGB::Black;
            }
            for (int i = firstLed; i < LED_COUNT; i++)
            {
                leds[i] = blinker_state ? CRGB::Orange : CRGB::Black;
            }
            FastLED.show();
        }

        if (millis() - blinker_started > 6250)
        {
            blinker_started = 0;
            blinker_change = 0;
            blinker_state = true;

            setAnimation(0);
        }
    }

    u32_t christmasTree_lastChangeMillis = 0;
    u32_t christmasTree_lastPatternMillis = 0;
    int christmasTree_changeDelay = 100;
    int christmasTree_patternDelay = 5000;
    uint8_t christmasTree_pattern = 0;
    uint8_t christmasTree_hue = 0;
    bool christmasTree_twinkle = false;

    /// @brief Shows a festive Christmas tree-like animation with full color range
    void animChristmasTree()
    {
        if (christmasTree_lastChangeMillis == 0)
        {
            christmasTree_lastChangeMillis = millis();
            christmasTree_lastPatternMillis = millis();
            christmasTree_pattern = 0;
            christmasTree_hue = 0;
        }

        if (millis() - christmasTree_lastPatternMillis > christmasTree_patternDelay)
        {
            christmasTree_pattern = (christmasTree_pattern + 1) % 4;
            christmasTree_lastPatternMillis = millis();
        }

        if (millis() - christmasTree_lastChangeMillis > christmasTree_changeDelay)
        {
            christmasTree_hue += 5;
            christmasTree_twinkle = !christmasTree_twinkle;

            switch (christmasTree_pattern)
            {
            case 0:
                for (int i = 0; i < LED_COUNT; i++)
                {
                    if (i % 2 == 0)
                        leds[i] = CRGB::Red;
                    else
                        leds[i] = CRGB::Green;

                    if (christmasTree_twinkle && random8() < 20)
                        leds[i] = CRGB::White;
                }
                break;

            case 1:
                for (int i = 0; i < LED_COUNT; i++)
                {
                    leds[i] = CHSV(christmasTree_hue + (i * 255 / LED_COUNT), 240, 255);
                }
                break;

            case 2:
            {
                CRGB colors[] = {CRGB::Red, CRGB::Green, CRGB::Gold, CRGB::Blue, CRGB::Purple};
                int colorCount = sizeof(colors) / sizeof(colors[0]);
                int segmentLength = max(1, LED_COUNT / (colorCount * 2));

                for (int i = 0; i < LED_COUNT; i++)
                {
                    int colorIndex = (i / segmentLength) % colorCount;
                    leds[i] = colors[colorIndex];
                }

                if (christmasTree_twinkle)
                {
                    for (int i = 0; i < max(1, LED_COUNT / 10); i++)
                    {
                        int pixelToTwinkle = random16(LED_COUNT);
                        leds[pixelToTwinkle] = CRGB::White;
                    }
                }
            }
            break;

            case 3:
                for (int i = 0; i < LED_COUNT; i++)
                {
                    leds[i] = CRGB(0, 0, 64);

                    if (random8() < 15)
                    {
                        int brightness = random8(100, 255);
                        if (random8() < 80)
                            leds[i] = CRGB(brightness, brightness, brightness);
                        else
                            leds[i] = CHSV(random8(), 180, brightness);
                    }
                }
                break;
            }

            FastLED.show();
            christmasTree_lastChangeMillis = millis();
        }
    }

    void resetChristmasTree()
    {
        christmasTree_lastChangeMillis = 0;
        christmasTree_lastPatternMillis = 0;
        christmasTree_pattern = 0;
        christmasTree_hue = 0;
    }

}