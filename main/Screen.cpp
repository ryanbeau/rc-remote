#include "Screen.h"

void HUD::update(Adafruit_GFX& g, uint8_t ms) {
    if (mUpdated) {
        g.fillScreen(0);

        mUpdated = 0;
    }
}
