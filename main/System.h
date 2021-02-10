#ifndef RC_SYSTEM_H
#define RC_SYSTEM_H

#include "IO.h"

class Screen {
   public:
    virtual ~Screen() {}
    virtual void update(uint8_t ms) = 0;
    virtual void onGamepadEvent(GamepadEvent& event) {}
    virtual void onPayloadEvent(Payload& payload) {}
    virtual void onTouchEvent(TouchPoint& point) {}

   protected:
    uint8_t _updated = 255;
};

extern void setScreen(Screen* s);
extern void update();
extern void onGamepadEvent(GamepadEvent& e);
extern void onPayloadEvent(Payload& p);
extern void onTouchEvent(TouchPoint& p);

#endif
