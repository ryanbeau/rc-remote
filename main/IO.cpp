#include "IO.h"

namespace io {

void AnalogState::setState(uint16_t value) volatile {
    _value = value;
}

void ButtonState::setState(bool pressed) volatile {
    if (pressed && _state != ButtonStates::Pressed && _state != ButtonStates::Up) {
        _debounceDelay = INPUT_DEBOUNCE_MS;
    }
    _pressed = pressed;
}

void ButtonState::updateState(uint8_t ms) volatile {
    switch (_state) {
        case ButtonStates::None:
            if (_debounceDelay > 0) {
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
    if (_debounceDelay > 0) {
        _debounceDelay -= ms;
    }
}

void TouchState::setPoint(TS_Point p) {
    _point = p;
}

void TouchState::setState(bool pressed) {
    if (_state != ButtonStates::Pressed && _state != ButtonStates::Up) {
        _debounceDelay = TOUCH_DEBOUNCE_MS;
    }
    _pressed = pressed;
}

void TouchState::updateState(uint8_t ms) {
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
    if (_debounceDelay > 0) {
        _debounceDelay -= ms;
    }
}

}  // namespace io
