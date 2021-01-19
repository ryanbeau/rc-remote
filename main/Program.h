#ifndef RC_PROGRAM_H
#define RC_PROGRAM_H

#include <Adafruit_GFX.h>
#include <Adafruit_STMPE610.h>

#include "Screen.h"

using namespace io;

namespace program {

class Application {
   public:
    Application(Adafruit_GFX& gfx, Adafruit_STMPE610& touch);

    void run(Screen* screen);
    void update();
    void setDigitalState(DigitalInputs button, bool value);
    void setAnalogState(AnalogInputs input, uint16_t value);

   private:
    Adafruit_GFX& _graphics;
    Adafruit_STMPE610& _touch;
    Screen* _screen;
    unsigned long _prevMS;
    TouchState _touchState;
    volatile ButtonState _buttonStates[BTN_COUNT];
    AnalogState _analogStates[ANALOG_COUNT];

    uint8_t updateMS();
    void updateButtonStates(uint8_t ms);
    void updateTouchState(uint8_t ms);
};

}  // namespace program

#endif
