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
    updateTouchState(ms);

    if (_screen) {
        _screen->update(_graphics, ms);
    }
}

void Application::updateButtonStates(uint8_t ms) {
    for (uint8_t i = 0; i < BTN_COUNT; i++) {
        ButtonStates prev = _buttonStates[i].getState();
        _buttonStates[i].updateState(ms);
        ButtonStates state = _buttonStates[i].getState();

        // screen on-event
        if (prev != state && _screen) {
            switch (state) {
                case ButtonStates::Down:
                    _screen->onButtonDown(static_cast<DigitalInputs>(i));
                    break;
                case ButtonStates::Pressed:
                    _screen->onButtonPressed(static_cast<DigitalInputs>(i));
                    break;
                case ButtonStates::Up:
                    _screen->onButtonUp(static_cast<DigitalInputs>(i));
                    break;
            }
        }
    }
}

void Application::updateTouchState(uint8_t ms) {
    ButtonStates prev = _touchState.getState();

    if (_touch.touched()) {
        _touchState.setPoint(_touch.getPoint());
        _touchState.setState(true);
    } else {
        _touchState.setState(false);
    }
    _touchState.updateState(ms);

    ButtonStates state = _touchState.getState();
    if (prev != state && _screen) {
        switch (state) {
            case ButtonStates::Down:
                _screen->onTouchDown(_touchState.getPoint());
                break;
            case ButtonStates::Pressed:
                _screen->onTouchPressed(_touchState.getPoint());
                break;
            case ButtonStates::Up:
                _screen->onTouchUp(_touchState.getPoint());
                break;
        }
    }
}

void Application::setDigitalState(DigitalInputs button, bool value) {
    _buttonStates[static_cast<uint8_t>(button)].setState(value);
}

void Application::setAnalogState(AnalogInputs input, uint16_t value) {

}

}  // namespace program
