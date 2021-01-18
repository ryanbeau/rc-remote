#include "IO.h"

namespace io {

void ButtonState::setState(bool on) volatile {
    if (on && _state != ButtonStates::Pressed && _state != ButtonStates::Up) {
        _debounceDelay = DEBOUNCE_DELAY_MS;
    }
    _pressed = on;
}

void ButtonState::updateState(uint8_t ms) volatile {
    if (_debounceDelay > 0) {
        _debounceDelay -= ms;
    }

    switch (_state) {
        case ButtonStates::None:
            if (_pressed) {
                _state = ButtonStates::Down;
            }
            break;
        case ButtonStates::Down:
            _state = ButtonStates::Pressed;
            break;
        case ButtonStates::Pressed:
            if (!_pressed && _debounceDelay <= 0) {
                _state = ButtonStates::Up;
            }
            break;
        case ButtonStates::Up:
            _state = ButtonStates::None;
            break;
    }
}

void TouchState::setState(uint16_t x, uint16_t y, uint8_t z) {
}

void TouchState::resetState() {
    _pressed = false;
}

}  // namespace io
