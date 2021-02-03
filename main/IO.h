#ifndef RC_IO_H
#define RC_IO_H

#include <Adafruit_HX8357.h>
#include <Adafruit_STMPE610.h>
#include <Arduino.h>

#include "Config.h"

// RGB565 convert 24bit RGB(8,8,8) [0-255, 0-255, 0-255] to 16bit RGB(5,6,5) [0-31, 0-63, 0-31].
#define RGB565(r, g, b) (((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3))

#define ANALOG_MIN 0
#define ANALOG_MID 2047
#define ANALOG_MAX 4095

typedef enum {
    // analog
    eVoltage = VOLTAGE_PIN,
    eLeft_JoyX = L_JOY_X_PIN,
    eLeft_JoyY = L_JOY_Y_PIN,
    eRight_JoyX = R_JOY_X_PIN,
    eRight_JoyY = R_JOY_Y_PIN,
    eLeft_Trigger = L_TRIG_PIN,
    eRight_Trigger = R_TRIG_PIN,
} eGamepadAnalog;

typedef enum {
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
} eGamepadDigital;

typedef struct {
    const eGamepadDigital inputPin;
    bool value;
} DigitalMap;

typedef struct {
    const eGamepadAnalog inputPin;
    uint16_t value;
    uint16_t base;
    uint16_t min;
    uint16_t max;
    bool inverted;
} AnalogMap;

typedef struct {
    DigitalMap* digital;
} GamepadEvent;

typedef struct {
    uint8_t typeVersion;
} Payload;

extern Adafruit_HX8357 gfx;

// initIO initializes all default or eeprom stored input/output devices and values.
extern void initIO();

#endif
