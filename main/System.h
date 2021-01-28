#ifndef RC_SYSTEM_H
#define RC_SYSTEM_H

#include "IO.h"

void OnGamepadEvent(GamepadEvent* event);
void OnPayloadEvent(Payload* payload);
void OnTouchEvent(TS_Point* point);

#endif
