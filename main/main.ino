#include <Arduino.h>
#include <Adafruit_HX8357.h>
#include <Adafruit_STMPE610.h>

#include "Program.h"

#define STMPE_CS 0
#define STMPE_SDI 0
#define STMPE_SDO 0
#define STMPE_SCK 0

#define TFT_CS 16
#define TFT_DC 17
#define TFT_MOSI 4
#define TFT_RST -1

#define LCD_CH 0
#define LCD_PIN 13
#define LCD_RES 8
#define LCD_FRQ 5000

#define BTN_UP_PIN 34
#define BTN_DN_PIN 35
#define BTN_RT_PIN 36
#define BTN_LT_PIN 39

#define BTN_A_PIN 32
#define BTN_B_PIN 33
#define BTN_C_PIN 25
#define BTN_D_PIN 26

#define LT_JOY_X 0
#define LT_JOY_Y 0
#define RT_JOY_X 0
#define RT_JOY_Y 0
#define LT_TRIGGER 0
#define RT_TRIGGER 0

using namespace program;
using namespace io;

Adafruit_STMPE610 touch = Adafruit_STMPE610(STMPE_CS, STMPE_SDI, STMPE_SDO, STMPE_SCK);
Adafruit_HX8357 gfx = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

Application* application = new Application(gfx, touch);

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        ;  // wait for serial port to connect. Needed for native USB
    }

    // lcd
    ledcSetup(LCD_CH, LCD_FRQ, LCD_RES);
    ledcAttachPin(LCD_PIN, LCD_CH);
    ledcWrite(LCD_CH, 255);
    
    // buttons
    pinMode(BTN_UP_PIN, INPUT); // up
    pinMode(BTN_DN_PIN, INPUT); // down
    pinMode(BTN_LT_PIN, INPUT); // left
    pinMode(BTN_RT_PIN, INPUT); // right
    pinMode(BTN_A_PIN, INPUT);  // a
    pinMode(BTN_B_PIN, INPUT);  // b
    pinMode(BTN_C_PIN, INPUT);  // c
    pinMode(BTN_D_PIN, INPUT);  // d

    // button state ISR
    attachInterrupt(digitalPinToInterrupt(BTN_UP_PIN), [](){ application->setDigitalState(DigitalInputs::Up, digitalRead(BTN_UP_PIN)); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BTN_DN_PIN), [](){ application->setDigitalState(DigitalInputs::Down, digitalRead(BTN_DN_PIN)); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BTN_LT_PIN), [](){ application->setDigitalState(DigitalInputs::Left, digitalRead(BTN_LT_PIN)); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BTN_RT_PIN), [](){ application->setDigitalState(DigitalInputs::Right, digitalRead(BTN_RT_PIN)); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BTN_A_PIN), [](){ application->setDigitalState(DigitalInputs::A, digitalRead(BTN_A_PIN)); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BTN_B_PIN), [](){ application->setDigitalState(DigitalInputs::B, digitalRead(BTN_B_PIN)); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BTN_C_PIN), [](){ application->setDigitalState(DigitalInputs::C, digitalRead(BTN_C_PIN)); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BTN_D_PIN), [](){ application->setDigitalState(DigitalInputs::D, digitalRead(BTN_D_PIN)); }, CHANGE);

    gfx.begin();
    gfx.setRotation(3); //480x320

    application->run(new HUD());
}

void loop() {
    application->setAnalogState(AnalogInputs::LeftJoyX, analogRead(LT_JOY_X));
    application->setAnalogState(AnalogInputs::LeftJoyY, analogRead(LT_JOY_Y));
    application->setAnalogState(AnalogInputs::RightJoyX, analogRead(RT_JOY_X));
    application->setAnalogState(AnalogInputs::RightJoyY, analogRead(RT_JOY_Y));
    application->setAnalogState(AnalogInputs::LeftTrigger, analogRead(LT_TRIGGER));
    application->setAnalogState(AnalogInputs::RightTrigger, analogRead(RT_TRIGGER));

    application->update();
}
