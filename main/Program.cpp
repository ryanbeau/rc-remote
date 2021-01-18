#include "Program.h"

namespace program {

Application::Application(Adafruit_GFX& gfx, Adafruit_STMPE610& touch) :
    _graphics(gfx),
    _touch(touch) {
}

void Application::run(Screen* screen) {
    _screen = screen;
}

uint8_t Application::updateMS() {
    unsigned long currMS = millis();
    uint8_t ms = currMS - _prevMS;
    if (currMS < _prevMS) {  // if rolled over
        ms = currMS;
    }
    _prevMS = currMS;
    return ms;
}

void Application::update() {
    uint8_t ms = updateMS();

    updateButtonStates(ms);
    updateTouchState();

    if (_screen) {
        _screen->update(_graphics, ms);
    }
}

void Application::updateButtonStates(uint8_t ms) {
    for (uint8_t i = 0; i < BTN_COUNT; i++) {
        ButtonStates state = _buttonStates[i].getState();
        _buttonStates[i].updateState(ms);

        // screen on-event
        if (state != _buttonStates[i].getState() && _screen) {
            switch (state) {
                case ButtonStates::Down:
                    _screen->onButtonDown(static_cast<Buttons>(i));
                    break;
                case ButtonStates::Pressed:
                    _screen->onButtonPressed(static_cast<Buttons>(i));
                    break;
                case ButtonStates::Up:
                    _screen->onButtonUp(static_cast<Buttons>(i));
                    break;
            }
        }
    }
}

void Application::updateTouchState() {
    if (_touch.touched()) {
        uint16_t x, y;
        uint8_t z;
        while (!_touch.bufferEmpty()) {
            _touch.readData(&x, &y, &z);
        }
        _touchState.setState(x, y, z);
        _touch.writeRegister8(STMPE_INT_STA, 0xFF);  // reset all ints
    } else {
        _touchState.resetState();
    }
}

void Application::setButtonState(Buttons button, bool state) {
    _buttonStates[static_cast<uint8_t>(button)].setState(state);
}

}  // namespace program
