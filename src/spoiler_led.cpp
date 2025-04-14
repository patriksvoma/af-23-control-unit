/*
    Spoiler LED module
    Interfaces a WS2812S LED strip
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
    if (red > 255) red = 250;
    if (green > 255) green = 250;
    if (blue > 255) blue = 250;

    if (red < 0) red = 0;
    if (green < 0) green = 0;
    if (blue < 0) blue = 0;

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
            if (startup_currentPixel == LED_COUNT) startup_increasing = false;
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
            if (sideToSide_currentPixel == LED_COUNT) sideToSide_increasing = false;
        }
        else
        {
            sideToSide_currentPixel--;
            if (sideToSide_currentPixel == -1) sideToSide_increasing = true;
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
}