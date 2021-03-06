#include <Arduino.h>

#include "Calibration.h"

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        ;  // wait for serial port to connect. Needed for native USB
    }

    initIO();

    setScreen(new AnalogCalibration());
}

void loop() {
    update();
    // vTaskDelete(NULL); // Deletes this loop() and frees this task of priority 1
}
