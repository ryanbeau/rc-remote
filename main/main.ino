#include <Adafruit_HX8357.h>
#include <Arduino.h>
#include <WiFi.h>

#include "Config.h"
#include "Program.h"

using namespace program;
using namespace io;

Adafruit_STMPE610 touch = Adafruit_STMPE610(STMPE_CS_PIN, STMPE_SDI_PIN, STMPE_SDO_PIN, STMPE_SCK_PIN);
Adafruit_HX8357 gfx = Adafruit_HX8357(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN);
RF24 radio = RF24(RF_CE_PIN, RF_CS_PIN);

hw_timer_t* rfTimer;
portMUX_TYPE rfTimerMux = portMUX_INITIALIZER_UNLOCKED;

Application* application = new Application(gfx, touch, radio);

byte mac[6];

void IRAM_ATTR rfTimerISR() {
    // get analog inputs for packet write
    application->setAnalogState(AnalogInputs::LeftJoyX, analogRead(LT_JOY_X_PIN));
    application->setAnalogState(AnalogInputs::LeftJoyY, analogRead(LT_JOY_Y_PIN));
    application->setAnalogState(AnalogInputs::RightJoyX, analogRead(RT_JOY_X_PIN));
    application->setAnalogState(AnalogInputs::RightJoyY, analogRead(RT_JOY_Y_PIN));
    application->setAnalogState(AnalogInputs::LeftTrigger, analogRead(LT_TRIG_PIN));
    application->setAnalogState(AnalogInputs::RightTrigger, analogRead(RT_TRIG_PIN));

    // stop timer as this may take longer than its regular interval
    timerStop(rfTimer);

    // mutex ISR to prevent overlapping with loop's application->readPacket()
    portENTER_CRITICAL_ISR(&rfTimerMux);
    application->writePacket();
    portEXIT_CRITICAL_ISR(&rfTimerMux);

    // resume timer
    timerStart(rfTimer);
}

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
    pinMode(BTN_UP_PIN, INPUT);  // up
    pinMode(BTN_DN_PIN, INPUT);  // down
    pinMode(BTN_LT_PIN, INPUT);  // left
    pinMode(BTN_RT_PIN, INPUT);  // right
    pinMode(BTN_A_PIN, INPUT);   // a
    pinMode(BTN_B_PIN, INPUT);   // b
    pinMode(BTN_C_PIN, INPUT);   // c
    pinMode(BTN_D_PIN, INPUT);   // d

    // button state ISR
    attachInterrupt(digitalPinToInterrupt(BTN_UP_PIN), []() { application->setDigitalState(DigitalInputs::Up, digitalRead(BTN_UP_PIN)); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BTN_DN_PIN), []() { application->setDigitalState(DigitalInputs::Down, digitalRead(BTN_DN_PIN)); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BTN_LT_PIN), []() { application->setDigitalState(DigitalInputs::Left, digitalRead(BTN_LT_PIN)); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BTN_RT_PIN), []() { application->setDigitalState(DigitalInputs::Right, digitalRead(BTN_RT_PIN)); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BTN_A_PIN), []() { application->setDigitalState(DigitalInputs::A, digitalRead(BTN_A_PIN)); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BTN_B_PIN), []() { application->setDigitalState(DigitalInputs::B, digitalRead(BTN_B_PIN)); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BTN_C_PIN), []() { application->setDigitalState(DigitalInputs::C, digitalRead(BTN_C_PIN)); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BTN_D_PIN), []() { application->setDigitalState(DigitalInputs::D, digitalRead(BTN_D_PIN)); }, CHANGE);

    // graphics
    gfx.begin();
    gfx.setRotation(3);  //480x320

    // radio
    radio.begin();
    radio.setChannel(84);
    radio.openReadingPipe(0, addressDiscovery);  // open to discovery address
    radio.enableDynamicPayloads();
    radio.startListening();

    // mac
    WiFi.macAddress(mac);

    // application
    application->setup(mac);
    application->run(new HUD());

    // timer
    rfTimer = timerBegin(0, 80, true);  // 80 MHz -> timer increment 1,000,000/sec
    timerAttachInterrupt(rfTimer, &rfTimerISR, true);
    timerAlarmWrite(rfTimer, 50000, true);  // interval: 50ms
    timerAlarmEnable(rfTimer);
}

void loop() {
    // mutex to prevent overlapping with timer ISR's application->writePacket()
    portENTER_CRITICAL(&rfTimerMux);
    application->readPacket();
    portEXIT_CRITICAL(&rfTimerMux);

    application->update();
}
