#include "System.h"

static Screen* screen;

void HUD::update(uint8_t ms) {
    if (_updated) {
        const uint16_t white = RGB565(255, 255, 255);
        const uint16_t lightgray = RGB565(154, 154, 154);
        const uint16_t midgray = RGB565(104, 104, 104);
        const uint16_t darkgray = RGB565(14, 14, 14);

        if (_updated == 255) {
            gfx.fillScreen(RGB565(33, 35, 36));
        }

        if (_updated) {
            for (uint8_t j = 0; j < 2; j++) {
                uint16_t x = 58 + (j * 266);
                uint16_t y = 102;
                for (uint8_t i = 0; i < 2; i++) {
                    // joy circle
                    if (_updated == 255) {
                        gfx.drawFastHLine(x + 44, y + 41 + (i * 15), 10, darkgray);
                        gfx.drawFastHLine(x + 45, y + 42 + (i * 13), 8, lightgray);
                        gfx.drawFastHLine(x + 45, y + 43 + (i * 11), 8, midgray);
                        gfx.drawFastHLine(x + 45, y + 44 + (i * 9), 8, darkgray);

                        gfx.drawFastVLine(x + 41 + (i * 15), y + 44, 10, darkgray);
                        gfx.drawFastVLine(x + 42 + (i * 13), y + 45, 8, lightgray);
                        gfx.drawFastVLine(x + 43 + (i * 11), y + 45, 8, midgray);
                        gfx.drawFastVLine(x + 44 + (i * 9), y + 45, 8, darkgray);

                        for (uint8_t k = 0; k < 2; k++) {
                            gfx.drawPixel(x + 42 + (i * 13), y + 44 + (k * 9), midgray);
                            gfx.drawPixel(x + 43 + (i * 11), y + 43 + (k * 11), midgray);
                            gfx.drawPixel(x + 44 + (i * 9), y + 42 + (k * 13), midgray);
                            gfx.drawPixel(x + 44 + (i * 9), y + 44 + (k * 9), midgray);
                            gfx.drawPixel(x + 43 + (i * 11), y + 44 + (k * 9), lightgray);
                            gfx.drawPixel(x + 44 + (i * 9), y + 43 + (k * 11), lightgray);
                            gfx.drawPixel(x + 42 + (i * 13), y + 43 + (k * 11), darkgray);
                            gfx.drawPixel(x + 43 + (i * 11), y + 42 + (k * 13), darkgray);
                        }
                    }

                    // joy axis
                    gfx.drawRect(x + 47, y + (i * 60), 4, 38, darkgray);
                    gfx.drawFastVLine(x + 48, y + 1 + (i * 60), 36, lightgray);
                    gfx.drawFastVLine(x + 49, y + 1 + (i * 60), 36, midgray);

                    gfx.drawRect(x + (i * 60), y + 47, 38, 4, darkgray);
                    gfx.drawFastHLine(x + 1 + (i * 60), y + 48, 36, midgray);
                    gfx.drawFastHLine(x + 1 + (i * 60), y + 49, 36, lightgray);

                    int16_t valX = analogRead(j == 0 ? L_JOY_X_PIN : R_JOY_X_PIN) - ANALOG_MID;
                    int16_t valY = analogRead(j == 0 ? L_JOY_Y_PIN : R_JOY_Y_PIN) - ANALOG_MID;
                    if (j == 1) {
                        valX = -valX;
                        valY = -valY;
                    }

                    if (i == 0 && valX < -56 || i == 1 && valX > 56) {
                        valX = static_cast<uint8_t>(static_cast<float>(valX < 0 ? -valX : valX) / static_cast<float>(ANALOG_MID) * 36.0f);
                        gfx.fillRect(x + 1 + (i * 60) + (i == 0 ? 36 - valX : 0), y + 48, valX, 2, white);
                    }

                    if (i == 0 && valY < -56 || i == 1 && valY > 56) {
                        valY = static_cast<uint8_t>(static_cast<float>(valY < 0 ? -valY : valY) / static_cast<float>(ANALOG_MID) * 36.0f);
                        gfx.fillRect(x + 48, y + 1 + (i * 60) + (i == 0 ? 36 - valY : 0), 2, valY, white);
                    }
                }

                // trigger axis
                gfx.drawRect(x + 161 + (j * 38), y, 4, 62, darkgray);
                gfx.drawFastVLine(x + 162 + (j * 38), y + 1, 60, lightgray);
                gfx.drawFastVLine(x + 163 + (j * 38), y + 1, 60, midgray);

                uint16_t valTrig = analogRead(j == 0 ? L_TRIG_PIN : R_TRIG_PIN);
                if (valTrig > ANALOG_MID + 34) {
                    valTrig -= ANALOG_MID;
                    valTrig = static_cast<uint8_t>(static_cast<float>(valTrig) / static_cast<float>(ANALOG_MID) * 60.0f);
                    gfx.fillRect(x + 162 + (j * 38), y + 1 + (60 - valTrig), 2, valTrig, white);
                }
            }
        }

        _updated = 1;
    }
}

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
    Serial.print(F("button:"));
    Serial.print(e->digital->inputPin);
    Serial.print(F(" value:"));
    Serial.println(e->digital->value);

    if (screen) screen->onGamepadEvent(e);
}

void onPayloadEvent(Payload* p) {
    if (screen) screen->onPayloadEvent(p);
}

void onTouchEvent(TS_Point* p) {
    if (screen) screen->onTouchEvent(p);
}
