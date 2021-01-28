#ifndef RC_PROGRAM_H
#define RC_PROGRAM_H

#include <Adafruit_GFX.h>
#include <Adafruit_STMPE610.h>
#include <RF24.h>

#include "Screen.h"

using namespace io;

namespace program {

const byte addressDiscovery[6] = "RC-00";

class Application {
   public:
    Application(Adafruit_GFX& gfx, Adafruit_STMPE610& touch, RF24& radio);

    void setup(byte* mac);
    void run(Screen* screen);
    void update();
    void writePacket();
    void readPacket();
    void setDigitalState(DigitalInputs button, bool value);
    void setAnalogState(AnalogInputs input, uint16_t value);

   private:
    Adafruit_GFX& _graphics;
    Adafruit_STMPE610& _touch;
    RF24 _radio;
    Screen* _screen;
    uint64_t _prevMS;
    byte* _mac;
    TouchState _touchState;
    volatile ButtonState _buttonStates[BTN_COUNT];
    volatile AnalogState _analogStates[ANALOG_COUNT];
    //const byte pipes[6];

    uint8_t updateMS();
    void updateButtonStates(uint8_t ms);
    void updateTouchState(uint8_t ms);
};

}  // namespace program

#endif
