#include "Calibration.h"

#define CALIBRATION_ON_START 0x02
#define CALIBRATION_RUNNING 0x04
#define CALIBRATION_FINISHED 0x80
#define ALIGN_CENTER 0
#define ALIGN_LEFT -1
#define ALIGN_RIGHT 1
#define X_AXIS 0
#define Y_AXIS 1
#define ZERO_PERCENT F("0%")

const uint16_t bg = RGB565(0, 0, 0);
const uint16_t white = RGB565(255, 255, 255);
const uint16_t lightergray = RGB565(154, 154, 154);
const uint16_t lightgray = RGB565(117, 117, 117);
const uint16_t midgray = RGB565(83, 83, 83);
const uint16_t darkgray = RGB565(57, 57, 57);
const uint16_t darkergray = RGB565(32, 32, 32);
const uint16_t green = RGB565(0, 255, 0);

void drawPercent(uint16_t leftX, uint16_t topY, int8_t align, uint8_t p) {
    uint16_t cursorX = leftX;

    gfx.fillRect(leftX, topY, 24, 8, bg);  // erase percent%
    uint8_t digits = 2;
    if (p == 100) {
        digits = 4;
    } else if (p >= 10) {
        digits = 3;
    }

    if (align == 0) {
        cursorX += 12 - (digits * 3);
    } else if (align > 0) {
        cursorX += 24 - (digits * 6);
    }

    gfx.setTextColor(p == 100 ? green : white, bg);
    if (p == 0) {
        gfx.setCursor(cursorX, topY);
        gfx.print(ZERO_PERCENT);
    } else {
        gfx.setCursor(cursorX, topY);
        gfx.printf("%d%%", p);
    }
}

void AnalogCalibration::updateJoystick(uint8_t joy, uint8_t axis, int8_t percent) {
    if (_joystick[joy][axis] != percent || (_updated & CALIBRATION_ON_START)) {
        const int16_t x = 58 + (joy * 266);
        const int16_t y = 102;

        int16_t step = percent * 36 / 100;
        uint16_t color = percent == -100 || percent == 100 ? green : white;

        // new pos is Left or Up
        if (axis == X_AXIS) {
            if (percent <= 0) {
                if (_joystick[joy][axis] >= 0) {
                    gfx.fillRect(x + 61, y + 48, 36, 2, bg);      // erase Right step
                    drawPercent(x + 101, y + 45, ALIGN_LEFT, 0);  // draw Right% as 0
                }

                if (percent != 0) {
                    gfx.fillRect(x + 37 + step, y + 48, -step, 2, color);  // draw Left step
                    if (percent > _joystick[joy][axis]) {
                        gfx.fillRect(x + 1, y + 48, (36 + step), 2, bg);  // erase Left step
                    }
                    drawPercent(x - 26, y + 45, ALIGN_RIGHT, -percent);
                }
            }

            if (percent >= 0) {
                if (_joystick[joy][axis] <= 0) {
                    gfx.fillRect(x + 1, y + 48, 36, 2, bg);       // erase Left step
                    drawPercent(x - 26, y + 45, ALIGN_RIGHT, 0);  // draw Left% as 0
                }

                if (percent != 0) {
                    gfx.fillRect(x + 61, y + 48, step, 2, color);  // draw Right step
                    if (percent < _joystick[joy][axis]) {
                        gfx.fillRect(x + 61 + step, y + 48, (36 - step), 2, bg);  // erase Right step
                    }
                    drawPercent(x + 101, y + 45, ALIGN_LEFT, percent);
                }
            }
        } else {
            if (percent <= 0) {
                if (_joystick[joy][axis] >= 0) {
                    gfx.fillRect(x + 48, y + 61, 2, 36, bg);        // erase Bottom step
                    drawPercent(x + 37, y + 101, ALIGN_CENTER, 0);  // draw Bottom% as 0
                }

                if (percent != 0) {
                    gfx.fillRect(x + 48, y + 37 + step, 2, -step, color);  // draw Up step
                    if (percent > _joystick[joy][axis]) {
                        gfx.fillRect(x + 48, y + 1, 2, (36 + step), bg);  // erase Up step
                    }
                    drawPercent(x + 37, y - 10, ALIGN_CENTER, -percent);
                }
            }

            if (percent >= 0) {
                if (_joystick[joy][axis] <= 0) {
                    gfx.fillRect(x + 48, y + 1, 2, 36, bg);        // erase Top step
                    drawPercent(x + 37, y - 10, ALIGN_CENTER, 0);  // draw Top% as 0
                }

                if (percent != 0) {
                    gfx.fillRect(x + 48, y + 61, 2, step, color);  // draw Bottom step
                    if (percent < _joystick[joy][axis]) {
                        gfx.fillRect(x + 48, y + 61 + step, 2, (36 - step), bg);  // erase Bottom step
                    }
                    drawPercent(x + 37, y + 101, ALIGN_CENTER, percent);
                }
            }
        }

        _joystick[joy][axis] = percent;
    }
}

void AnalogCalibration::updateTrigger(uint8_t trigger, uint8_t percent) {
    if (_trigger[trigger] != percent || (_updated & CALIBRATION_ON_START)) {
        const uint16_t x = 220 + (trigger * 38);

        int16_t step = percent * 60 / 100;
        uint16_t color = percent == 100 ? green : white;

        if (percent > 0) {
            gfx.fillRect(x, 163 - step, 2, step, color);
            if (percent < _trigger[trigger]) {
                gfx.fillRect(x, 103, 2, (60 - step), bg);
            }
            drawPercent(x - 11, 92, ALIGN_CENTER, percent);
        } else {
            gfx.fillRect(x, 103, 2, 60, bg);
            drawPercent(x - 11, 92, ALIGN_CENTER, 0);
        }

        _trigger[trigger] = percent;
    }
}

void AnalogCalibration::update(uint8_t ms) {
    if (_updated) {
        if (_updated == 255) {
            gfx.fillScreen(bg);

            gfx.setTextColor(white, bg);

            gfx.setCursor(174, 17);
            gfx.print(F("Controller Calibration"));

            gfx.setCursor(147, 200);
            gfx.print(F("Ensure the control sticks are"));
            gfx.setCursor(147, 212);
            gfx.print(F("centered before pressing Start."));

            gfx.drawRoundRect(170, 70, 140, 75, 8, white);  // controller
            gfx.drawCircle(190, 106, 10, white);            // left joy
            gfx.fillRect(189, 105, 3, 3, white);            // left joy knob
            gfx.drawCircle(289, 106, 10, white);            // right joy
            gfx.fillRect(288, 105, 3, 3, white);            // right joy knob
            gfx.drawRect(209, 104, 62, 35, white);          // screen
            // left dpad
            gfx.fillRect(182, 124, 3, 3, white);
            gfx.fillRect(179, 127, 9, 3, white);
            gfx.fillRect(182, 130, 3, 3, white);
            // right dpad
            gfx.fillRect(295, 124, 3, 3, white);
            gfx.fillRect(292, 127, 9, 3, white);
            gfx.fillRect(295, 130, 3, 3, white);

            // start button
            gfx.fillRoundRect(157, 260, 166, 40, 8, white);

            gfx.setTextColor(bg);
            gfx.setCursor(225, 276);
            gfx.print(F("START"));

            _updated = 0;
        }

        if (_updated & CALIBRATION_RUNNING) {
            if (_updated & CALIBRATION_ON_START) {
                gfx.fillRect(156, 260, 168, 46, bg);  // erase button
                gfx.fillRect(170, 70, 140, 75, bg);   // erase controller icon
                gfx.fillRect(147, 200, 186, 20, bg);  // erase warning text

                gfx.setTextColor(lightergray, bg);

                gfx.setCursor(70, 60);
                gfx.print(F("Left Joystick"));

                gfx.setCursor(217, 60);
                gfx.print(F("Triggers"));

                gfx.setCursor(331, 60);
                gfx.print(F("Right Joystick"));

                gfx.setCursor(23, 295);
                gfx.print(F("Push the joysticks and triggers to their maximum range in all directions."));
            }

            // reclamp min/max - if value is below min or above max
            leftJoyX.reclampMinMax();
            leftJoyY.reclampMinMax();
            rightJoyX.reclampMinMax();
            rightJoyY.reclampMinMax();
            leftTrigger.reclampMax();
            rightTrigger.reclampMax();

            if (!(_updated & CALIBRATION_FINISHED) && isAnalogCalibrated()) {
                gfx.fillRoundRect(157, 200, 166, 40, 8, white);

                gfx.setTextColor(bg);
                gfx.setCursor(216, 216);
                gfx.print(F("COMPLETE"));

                _updated |= CALIBRATION_FINISHED;
            }

            const uint16_t y = 102;
            for (uint8_t j = 0; j < 2; j++) {
                if (_updated & CALIBRATION_ON_START) {
                    uint16_t x = 58 + (j * 266);

                    for (uint8_t i = 0; i < 2; i++) {
                        // joy circle
                        gfx.drawFastHLine(x + 45, y + 42 + (i * 13), 8, lightgray);
                        gfx.drawFastHLine(x + 45, y + 43 + (i * 11), 8, midgray);
                        gfx.drawFastVLine(x + 42 + (i * 13), y + 45, 8, lightgray);
                        gfx.drawFastVLine(x + 43 + (i * 11), y + 45, 8, midgray);

                        for (uint8_t k = 0; k < 2; k++) {
                            gfx.drawPixel(x + 42 + (i * 13), y + 44 + (k * 9), midgray);
                            gfx.drawPixel(x + 43 + (i * 11), y + 43 + (k * 11), midgray);
                            gfx.drawPixel(x + 44 + (i * 9), y + 42 + (k * 13), midgray);
                            gfx.drawPixel(x + 44 + (i * 9), y + 44 + (k * 9), midgray);
                            gfx.drawPixel(x + 43 + (i * 11), y + 44 + (k * 9), lightgray);
                            gfx.drawPixel(x + 44 + (i * 9), y + 43 + (k * 11), lightgray);
                        }

                        // joy axis outline
                        gfx.drawRect(x + 47, y + (i * 60), 4, 38, midgray);
                        gfx.drawRect(x + (i * 60), y + 47, 38, 4, midgray);
                    }

                    if (_updated & CALIBRATION_ON_START) {
                        gfx.drawRect(219 + (j * 38), y, 4, 62, midgray);
                    }
                }

                updateJoystick(j, X_AXIS, j == 0 ? leftJoyX.getPercentValue() : rightJoyX.getPercentValue());
                updateJoystick(j, Y_AXIS, j == 0 ? leftJoyY.getPercentValue() : rightJoyY.getPercentValue());

                updateTrigger(j, j == 0 ? leftTrigger.getPercentValue() : rightTrigger.getPercentValue());
            }

            if (_updated & CALIBRATION_ON_START) {
                _updated &= ~CALIBRATION_ON_START;
            }
        }
    }
}

void AnalogCalibration::onTouchEvent(TouchPoint* point) {
    // set analog base, min & max
    if (!(_updated & CALIBRATION_RUNNING) && point->x >= 157 && point->x <= 323 && point->y >= 260 && point->y <= 300) {
        leftJoyX.setHomeMinMax(leftJoyX.value);
        leftJoyY.setHomeMinMax(leftJoyY.value);
        rightJoyX.setHomeMinMax(rightJoyX.value);
        rightJoyY.setHomeMinMax(rightJoyY.value);
        leftTrigger.setHomeMinMax(leftTrigger.value);
        rightTrigger.setHomeMinMax(rightTrigger.value);

        _updated = CALIBRATION_ON_START | CALIBRATION_RUNNING;
    }

    if (_updated & CALIBRATION_FINISHED && point->x >= 157 && point->x <= 157 + 166 && point->y >= 200 && point->y <= 200 + 40) {
        // complete calibration
    }
}
