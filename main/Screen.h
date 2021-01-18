#ifndef RC_SCREEN_H
#define RC_SCREEN_H

#include <Adafruit_GFX.h>

#include <vector>

#include "IO.h"

// RGB565 convert 24bit RGB(8,8,8) [0-255, 0-255, 0-255] to 16bit RGB(5,6,5) [0-31, 0-63, 0-31].
#define RGB565(r, g, b) (((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3))

using namespace io;

class Screen {
   public:
    virtual ~Screen() {}
    virtual void update(Adafruit_GFX& gfx, uint8_t ms) = 0;

    virtual void onButtonDown(Buttons button) {}
    virtual void onButtonPressed(Buttons button) {}
    virtual void onButtonUp(Buttons button) {}
    virtual void onTouchDown(uint16_t x, uint16_t y, uint8_t z) {}
    virtual void onTouchPressed(uint16_t x, uint16_t y, uint8_t z) {}
    virtual void onTouchUp(uint16_t x, uint16_t y, uint8_t z) {}
};

class HUD : public Screen {
   public:
    void update(Adafruit_GFX& gfx, uint8_t ms) override;

   private:
    uint8_t mUpdated = 1;
};

#endif
