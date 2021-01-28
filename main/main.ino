#include <Adafruit_HX8357.h>
#include <Arduino.h>
#include <WiFi.h>

#include "Config.h"
#include "IO.h"

typedef void (*StateFunction_t)( void );

StateFunction_t state;

void setup() {
    //state = &setup;
    ioInit();
}

void loop() {
    state();
}