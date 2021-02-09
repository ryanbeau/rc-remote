#ifndef RC_IO_H
#define RC_IO_H

#include <Adafruit_HX8357.h>
#include <Arduino.h>

#include "Config.h"

// RGB565 convert 24bit RGB(8,8,8) [0-255, 0-255, 0-255] to 16bit RGB(5,6,5) [0-31, 0-63, 0-31].
#define RGB565(r, g, b) (((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3))

enum eGamepadAnalog {
    // analog
    eVoltage = VOLTAGE_PIN,
    eLeft_JoyX = L_JOY_X_PIN,
    eLeft_JoyY = L_JOY_Y_PIN,
    eRight_JoyX = R_JOY_X_PIN,
    eRight_JoyY = R_JOY_Y_PIN,
    eLeft_Trigger = L_TRIG_PIN,
    eRight_Trigger = R_TRIG_PIN,
};

enum eGamepadDigital {
    eLeft_Aux = L_AUX_BTN_PIN,
    eRight_Aux = R_AUX_BTN_PIN,
    eLeft_Bumper = L_BUMPER_BTN_MCP_PIN,
    eLeft_DpadUp = L_DPAD_UP_MCP_PIN,
    eLeft_DpadDown = L_DPAD_DN_MCP_PIN,
    eLeft_DpadRight = L_DPAD_RT_MCP_PIN,
    eLeft_DpadLeft = L_DPAD_LT_MCP_PIN,
    eRight_Bumper = R_BUMPER_BTN_MCP_PIN,
    eRight_DpadUp = R_DPAD_UP_MCP_PIN,
    eRight_DpadDown = R_DPAD_DN_MCP_PIN,
    eRight_DpadRight = R_DPAD_RT_MCP_PIN,
    eRight_DpadLeft = R_DPAD_LT_MCP_PIN,
};

struct DigitalMap {
    const eGamepadDigital inputPin;
    bool value;
};

struct AnalogMap {
    const eGamepadAnalog inputPin;
    uint16_t value;
    uint16_t home;
    uint16_t min;
    uint16_t max;
    bool inverted;

    // setHomeMinMax sets the home, min, max with value.
    void setHomeMinMax(uint16_t value);

    // reclampMinMax clamps the min and max with value if below min or above max.
    void reclampMinMax();

    // reclampMax clamps the max with value if above max.
    void reclampMax();

    // getMapValue gets the mapped value from base, min, max.
    int16_t getMapValue(int16_t toMin, int16_t toMax);

    // getPercentValue gets the percent value -100 to 100 if joystick, 0 to 100 if trigger or voltage.
    int8_t getPercentValue();
};

struct GamepadEvent {
    DigitalMap* digital;
};

struct TouchPoint {
    uint16_t x;
    uint16_t y;
};

struct Payload {
    uint8_t typeVersion;
};

extern Adafruit_HX8357 gfx;
extern AnalogMap leftJoyX;
extern AnalogMap leftJoyY;
extern AnalogMap rightJoyX;
extern AnalogMap rightJoyY;
extern AnalogMap leftTrigger;
extern AnalogMap rightTrigger;

// initIO initializes all default or eeprom stored input/output devices and values.
extern void initIO();

// getAnalogMap returns the specific AnalogMap.
extern AnalogMap* getAnalogMap(eGamepadAnalog gamepadAnalog);

// isAnalogCalibrated returns whether the analog inputs are calibrated.
extern bool isAnalogCalibrated();

#endif
