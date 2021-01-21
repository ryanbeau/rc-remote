#ifndef RC_IO_H
#define RC_IO_H

#include <Arduino.h>
#include <Adafruit_STMPE610.h>
#include <stdint.h>

#define ANALOG_COUNT 6
#define BTN_COUNT 8
#define INPUT_DEBOUNCE_MS 50
#define TOUCH_DEBOUNCE_MS 50

namespace io {

struct Packet {

};

enum class AnalogInputs : uint8_t {
    LeftJoyX,
    LeftJoyY,
    RightJoyX,
    RightJoyY,
    LeftTrigger,
    RightTrigger,
};

enum class DigitalInputs : uint8_t {
    Up,
    Down,
    Right,
    Left,
    A,
    B,
    C,
    D,
};

enum class ButtonStates : uint8_t {
    None,
    Down,
    Pressed,
    Up,
};

class AnalogState {
   public:
    void setState(uint16_t value) volatile;

   private:
    volatile int16_t _value;
};

class ButtonState {
   public:
    void setState(bool pressed) volatile;
    void updateState(uint8_t ms) volatile;
    ButtonStates getState() volatile {
        return _state;
    }

   private:
    volatile bool _pressed = false;
    volatile int8_t _debounceDelay = 0;
    ButtonStates _state = ButtonStates::None;
};

class TouchState {
   public:
    void setPoint(TS_Point p);
    void setState(bool pressed);
    void updateState(uint8_t ms);
    ButtonStates getState() {
        return _state;
    }
    TS_Point getPoint() {
        return _point;
    }

   private:
    bool _pressed = false;
    int8_t _debounceDelay = 0;
    ButtonStates _state = ButtonStates::None;
    TS_Point _point = TS_Point(0, 0, 0);
};

}  // namespace io

#endif
