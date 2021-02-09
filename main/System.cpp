#include "System.h"

static Screen* screen;

void setScreen(Screen* s) {
    screen = s;
}

uint8_t getElapsedMS() {
    static unsigned long prevMS;

    unsigned long currMS = millis();
    uint8_t ms = currMS - prevMS;
    if (currMS < prevMS) {
        ms = 0;
    }
    prevMS = currMS;
    return ms;
}

void update() {
    static const TickType_t xDelay = 50 / portTICK_PERIOD_MS;  // 50ms

    if (screen) {
        screen->update(getElapsedMS());
    }
}

void onGamepadEvent(GamepadEvent* e) {
    if (screen) screen->onGamepadEvent(e);
}

void onPayloadEvent(Payload* p) {
    if (screen) screen->onPayloadEvent(p);
}

void onTouchEvent(TouchPoint* p) {
    if (screen) screen->onTouchEvent(p);
}
