#ifndef RC_IO_H
#define RC_IO_H

#include <Adafruit_STMPE610.h>
#include <Arduino.h>

#include "Config.h"

enum class Gamepad : uint8_t {
    // analog
    L_Joy_X   = L_JOY_X_PIN,
    L_Joy_Y   = L_JOY_Y_PIN,
    R_Joy_X   = R_JOY_X_PIN,
    R_Joy_Y   = R_JOY_Y_PIN,
    L_Trigger = L_TRIG_PIN,
    R_Trigger = R_TRIG_PIN,
    // digital
    L_DPad_Up    = L_DPAD_UP_PIN,
    L_DPad_Down  = L_DPAD_DN_PIN,
    L_DPad_Right = L_DPAD_RT_PIN,
    L_DPad_Left  = L_DPAD_LT_PIN,
    R_DPad_Up    = R_DPAD_UP_PIN,
    R_DPad_Down  = R_DPAD_DN_PIN,
    R_DPad_Right = R_DPAD_RT_PIN,
    R_DPad_Left  = R_DPAD_LT_PIN,
    L_Aux        = L_AUX_BTN_PIN,
    R_Aux        = R_AUX_BTN_PIN,
};

typedef enum {
    buttonDown,
    buttonUp,
    joystick,
    trigger,
} EventIO;

typedef struct {
    Gamepad input;
    EventIO type;
    union {
        bool button;
        int8_t joystick;
        uint8_t trigger;
    };
} GamepadEvent;

typedef struct {
    
} Payload;

void ioInit();

#endif
