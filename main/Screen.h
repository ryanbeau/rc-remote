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

    virtual void onButtonDown(DigitalInputs button) {}
    virtual void onButtonPressed(DigitalInputs button) {}
    virtual void onButtonUp(DigitalInputs button) {}
    virtual void onTouchDown(TS_Point p) {}
    virtual void onTouchPressed(TS_Point p) {}
    virtual void onTouchUp(TS_Point p) {}
};

class HUD : public Screen {
   public:
    void update(Adafruit_GFX& gfx, uint8_t ms) override;

   private:
    uint8_t mUpdated = 1;
};

#endif
