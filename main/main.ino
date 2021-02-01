#include <Arduino.h>

#include "System.h"

void setup() {
    initIO();

    setScreen(new HUD());
}

void loop() {
    update();
    // vTaskDelete(NULL); // Deletes this loop() and frees this task of priority 1
}
