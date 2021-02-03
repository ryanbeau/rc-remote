#ifndef RC_SYSTEM_H
#define RC_SYSTEM_H

#include "IO.h"

class Screen {
   public:
    virtual ~Screen() {}
    virtual void update(uint8_t ms) = 0;
    virtual void onGamepadEvent(GamepadEvent* event) {}
    virtual void onPayloadEvent(Payload* payload) {}
    virtual void onTouchEvent(TS_Point* point) {}

   protected:
    uint8_t _updated = 255;
};

class HUD : public Screen {
   public:
    void update(uint8_t ms) override;
};

extern void setScreen(Screen* s);
extern void update();
extern void onGamepadEvent(GamepadEvent* e);
extern void onPayloadEvent(Payload* p);
extern void onTouchEvent(TS_Point* p);

#endif
