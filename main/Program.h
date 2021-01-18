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
    void setButtonState(Buttons button, bool state);

   private:
    Adafruit_GFX& _graphics;
    Adafruit_STMPE610& _touch;
    Screen* _screen;
    unsigned long _prevMS;
    TouchState _touchState;
    volatile ButtonState _buttonStates[BTN_COUNT];

    uint8_t updateMS();
    void updateButtonStates(uint8_t ms);
    void updateTouchState();
};

}  // namespace program

#endif
