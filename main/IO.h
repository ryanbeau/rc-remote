#ifndef RC_IO_H
#define RC_IO_H

#include <Adafruit_HX8357.h>
#include <Adafruit_STMPE610.h>
#include <Arduino.h>

#include "Config.h"

typedef enum {
    // analog
    eVoltage       = VOLTAGE_PIN,
    eLeft_JoyX     = L_JOY_X_PIN,
    eLeft_JoyY     = L_JOY_Y_PIN,
    eRight_JoyX    = R_JOY_X_PIN,
    eRight_JoyY    = R_JOY_Y_PIN,
    eLeft_Trigger  = L_TRIG_PIN,
    eRight_Trigger = R_TRIG_PIN,
    // digital on MCU
    eLeft_Aux        = L_AUX_BTN_PIN,
    eRight_Aux       = R_AUX_BTN_PIN,
    // digital on MCP23017 port expander
    eLeft_DpadUp     = L_DPAD_UP_PIN,
    eLeft_DpadDown   = L_DPAD_DN_PIN,
    eLeft_DpadRight  = L_DPAD_RT_PIN,
    eLeft_DpadLeft   = L_DPAD_LT_PIN,
    eRight_DpadUp    = R_DPAD_UP_PIN,
    eRight_DpadDown  = R_DPAD_DN_PIN,
    eRight_DpadRight = R_DPAD_RT_PIN,
    eRight_DpadLeft  = R_DPAD_LT_PIN,
    eLeft_Bumper     = L_BUMPER_BTN_PIN,
    eRight_Bumper    = R_BUMPER_BTN_PIN,
} eGamepad;

typedef struct {
    const eGamepad inputPin;
    bool value;
} DigitalMap;

typedef struct {
    const eGamepad inputPin;
    uint16_t value;
    uint16_t base;
    uint16_t min;
    uint16_t max;
    bool inverted;
} AnalogMap;

typedef enum {
    eEventButton,
    eEventJoystick,
    eEventTrigger,
} eGamepadEventType;

typedef struct {
    eGamepadEventType type;
    union {
        DigitalMap* digital;
        AnalogMap* analog;
    };
} GamepadEvent;

typedef struct {
    uint8_t typeVersion;
} Payload;

extern Adafruit_HX8357 gfx;

// initIO initializes all default or eeprom stored input/output devices and values.
extern void initIO();

#endif
