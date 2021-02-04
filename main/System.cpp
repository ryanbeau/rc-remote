#include "System.h"

static Screen* screen;

void AnalogCalibration::update(uint8_t ms) {
    if (_updated) {
        const uint16_t white = RGB565(255, 255, 255);
        const uint16_t lightgray = RGB565(154, 154, 154);
        const uint16_t midgray = RGB565(104, 104, 104);
        const uint16_t darkgray = RGB565(14, 14, 14);
        const uint16_t green = RGB565(0, 255, 0);

        if (_updated == 255) {
            gfx.fillScreen(RGB565(33, 35, 36));

            leftJoyX = getAnalogMap(eGamepadAnalog::eLeft_JoyX);
            leftJoyY = getAnalogMap(eGamepadAnalog::eLeft_JoyY);
            rightJoyX = getAnalogMap(eGamepadAnalog::eRight_JoyX);
            rightJoyY = getAnalogMap(eGamepadAnalog::eRight_JoyY);
            leftTrigger = getAnalogMap(eGamepadAnalog::eLeft_Trigger);
            rightTrigger = getAnalogMap(eGamepadAnalog::eRight_Trigger);

            // delay to ensure analog has been read
            const TickType_t xDelay = 50 / portTICK_PERIOD_MS;
            vTaskDelay(xDelay);

            // set analog base, min & max
            leftJoyX->setBaseMinMax(leftJoyX->value);
            leftJoyY->setBaseMinMax(leftJoyY->value);
            rightJoyX->setBaseMinMax(rightJoyX->value);
            rightJoyY->setBaseMinMax(rightJoyY->value);
            leftTrigger->setBaseMinMax(leftTrigger->value);
            rightTrigger->setBaseMinMax(rightTrigger->value);
        }

        const uint16_t y = 102;
        for (uint8_t j = 0; j < 2; j++) {
            uint16_t x = 58 + (j * 266);
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
                } else {
                    // reclamp min/max - if value is below min or above max
                    leftJoyX->reclampMinMax();
                    leftJoyY->reclampMinMax();
                    rightJoyX->reclampMinMax();
                    rightJoyY->reclampMinMax();
                    leftTrigger->reclampMinMax();
                    rightTrigger->reclampMinMax();
                }

                // joy axis
                gfx.drawRect(x + 47, y + (i * 60), 4, 38, darkgray);
                gfx.drawFastVLine(x + 48, y + 1 + (i * 60), 36, lightgray);
                gfx.drawFastVLine(x + 49, y + 1 + (i * 60), 36, midgray);

                gfx.drawRect(x + (i * 60), y + 47, 38, 4, darkgray);
                gfx.drawFastHLine(x + 1 + (i * 60), y + 48, 36, midgray);
                gfx.drawFastHLine(x + 1 + (i * 60), y + 49, 36, lightgray);

                int16_t valX = j == 0 ? leftJoyX->getMapValue() : rightJoyX->getMapValue();

                if (i == 0 && valX < -56 || i == 1 && valX > 56) {
                    valX = map(valX < 0 ? -valX : valX, 0, ANALOG_MID, 0, 36);
                    gfx.fillRect(x + 1 + (i * 60) + (i == 0 ? 36 - valX : 0), y + 48, valX, 2, valX < 36 ? white : green);
                }

                int16_t valY = j == 0 ? leftJoyY->getMapValue() : rightJoyY->getMapValue();

                if (i == 0 && valY < -113) {
                    valY = map(-valY, 0, ANALOG_MID, 0, 36);
                    gfx.fillRect(x + 48, y + 1 + (i * 60) + 36 - valY, 2, valY, valX < 36 ? white : green);
                }
                if (i == 1 && valY > 113) {
                    valY = map(valY, 0, ANALOG_MID, 0, 36);
                    gfx.fillRect(x + 48, y + 1 + (i * 60) + 0, 2, valY, valX < 36 ? white : green);
                }
            }

            // trigger axis
            x = 219;
            gfx.drawRect(x + (j * 38), y, 4, 62, darkgray);
            gfx.drawFastVLine(x + 1 + (j * 38), y + 1, 60, lightgray);
            gfx.drawFastVLine(x + 2 + (j * 38), y + 1, 60, midgray);

            uint16_t valTrig = j == 0 ? leftTrigger->getMapValue() : rightTrigger->getMapValue();
            if (valTrig > 68) {
                valTrig = map(valTrig, 0, ANALOG_MAX, 0, 60);
                gfx.fillRect(x + 1 + (j * 38), y + 1 + (60 - valTrig), 2, valTrig, white);
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
    Serial.print(e->digital->inputPin % MCP_PIN_BIT);
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
