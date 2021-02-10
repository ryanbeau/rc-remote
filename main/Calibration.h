#include "System.h"

class AnalogCalibration : public Screen {
   public:
    void update(uint8_t ms) override;
    void onTouchEvent(TouchPoint& point) override;

   private:
    int8_t _joystick[2][2] {{0, 0}, {0, 0}};
    uint8_t _trigger[2] {0, 0};

    void updateJoystick(uint8_t joy, uint8_t axis, int8_t percent);
    void updateTrigger(uint8_t trigger, uint8_t percent);
};
