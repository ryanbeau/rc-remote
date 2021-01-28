#include "Program.h"

namespace program {

Application::Application(Adafruit_GFX& gfx, Adafruit_STMPE610& touch, RF24& radio) :
    _graphics(gfx),
    _touch(touch),
    _radio(radio) {
}

void Application::setup(byte* mac) {
    _mac = mac;
}

void Application::run(Screen* screen) {
    _screen = screen;
}

void Application::writePacket() {
    _radio.stopListening();

    // TODO : write to specific pipe with retry and offset delay
    //_radio.openWritingPipe(address);
    //_radio.setRetries(delay, count);

    Packet packet;  // TODO: build packet
    _radio.write(&packet, sizeof(packet));

    loop();

    // Discovery Message Example
    // uint8    type|version     0-16|0-16
    // byte*    name             "Tonky Tonk" 12
    // uint8    color|icon       0-16|0-16
    // byte[6]  MAC              00:0a:95:9d:68:16
    // 

    _radio.startListening();
}

void Application::readPacket() {
    byte pipe = 0;
    while (_radio.available(&pipe)) {
        uint8_t size = _radio.getDynamicPayloadSize();
        byte response[size];

        _radio.read(&response, sizeof(response));

        // pipe channel
        if (pipe == 0x00) {  // 0 = discovery

        } else { // paired device
        
        }

        // version of message
        uint8_t version = response[0] & 0x0F;  // right 4 bits

        // type of message
        switch (response[0] >> 4) { // left 4 bits
            case 0x00:  //  0:0000 = is channel empty
                break;
            case 0x09:  //  9:1001 = setup
                break;
            case 0x0F:  // 15:1111 = operation
                break;
        }
    }
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
    _analogStates[static_cast<uint8_t>(input)].setState(value);
}

}  // namespace program
