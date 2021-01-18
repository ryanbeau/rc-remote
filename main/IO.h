#ifndef RC_IO_H
#define RC_IO_H

#include <Arduino.h>
#include <stdint.h>

#define BTN_COUNT 8
#define DEBOUNCE_DELAY_MS 50

namespace io {

enum class Buttons : uint8_t {
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

class ButtonState {
   public:
    void setState(bool pressed) volatile;
    void updateState(uint8_t ms) volatile;
    ButtonStates getState() volatile {
        return _state;
    }

   private:
    volatile bool _pressed = false;
    volatile ButtonStates _state = ButtonStates::None;
    volatile int8_t _debounceDelay = 0;
};

class TouchState {
   public:
    void resetState();
    void setState(uint16_t x, uint16_t y, uint8_t z);
   private:
    bool _pressed = false;
};

}  // namespace io

#endif
