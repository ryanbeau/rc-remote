#include "IO.h"

#include <Adafruit_MCP23017.h>
#include <RF24.h>

#include "System.h"

#define TOUCH_BIT 0x01
#define RF_BIT 0x02
#define MCP_BIT 0x04

#define DIGITAL_AMT 12
#define ANALOG_AMT 7

const byte addressDiscovery[6] = "RC-00";

Adafruit_HX8357 gfx = Adafruit_HX8357(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN);
Adafruit_STMPE610 touch = Adafruit_STMPE610(STMPE_CS_PIN);
RF24 radio = RF24(RF_CE_PIN, RF_CS_PIN);
Adafruit_MCP23017 mcp;

xQueueHandle digitalInterruptQueue = NULL;
static TaskHandle_t inputHandlingTask;

DigitalMap digitalMap[DIGITAL_AMT] = {
    {eLeft_DpadUp, false},
    {eLeft_DpadDown, false},
    {eLeft_DpadRight, false},
    {eLeft_DpadLeft, false},
    {eRight_DpadUp, false},
    {eRight_DpadDown, false},
    {eRight_DpadRight, false},
    {eRight_DpadLeft, false},
    {eLeft_Bumper, false},
    {eRight_Bumper, false},
    {eLeft_Aux, false},
    {eRight_Aux, false},
};

AnalogMap analogMap[ANALOG_AMT] = {
    {eVoltage, ANALOG_MIN, ANALOG_MIN, ANALOG_MIN, ANALOG_MAX, false},
    {eLeft_JoyX, ANALOG_MID, ANALOG_MID, ANALOG_MIN, ANALOG_MAX, false},
    {eLeft_JoyY, ANALOG_MID, ANALOG_MID, ANALOG_MIN, ANALOG_MAX, false},
    {eRight_JoyX, ANALOG_MID, ANALOG_MID, ANALOG_MIN, ANALOG_MAX, true},
    {eRight_JoyY, ANALOG_MID, ANALOG_MID, ANALOG_MIN, ANALOG_MAX, true},
    {eLeft_Trigger, ANALOG_MID, ANALOG_MID, ANALOG_MIN, ANALOG_MAX, false},
    {eRight_Trigger, ANALOG_MID, ANALOG_MID, ANALOG_MIN, ANALOG_MAX, false},
};

void handleDigitalEvent(DigitalMap* map) {
    GamepadEvent ev = {digital : map};

    onGamepadEvent(&ev);
}

void IRAM_ATTR isrDigitalHandler(void* arg) {
    uint32_t pin = (uint32_t)arg;
    xQueueSendFromISR(digitalInterruptQueue, &pin, NULL);
    portYIELD_FROM_ISR();
}

void IRAM_ATTR isrRFHandler() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(inputHandlingTask, RF_BIT, eSetBits, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR();
}

void IRAM_ATTR isrTouchHandler() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(inputHandlingTask, TOUCH_BIT, eSetBits, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR();
}

void IRAM_ATTR isrMCPHandler() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(inputHandlingTask, MCP_BIT, eSetBits, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR();
}

void inputAnalogTask(void* arg) {
    static const TickType_t xDelay = 50 / portTICK_PERIOD_MS;  // 50ms
    static const float max = 255.0f;

    while (1) {
        vTaskDelay(xDelay);
        for (uint8_t i = 0; i < ANALOG_AMT; i++) {
            analogMap[i].value = analogRead(analogMap[i].inputPin);  // range: 0 to 4095

            if (analogMap[i].inputPin == eLeft_Trigger || analogMap[i].inputPin == eRight_Trigger) {
                // TODO : handle trigger value
            } else {
                // TODO : handle joystick value
            }
        }
    }
}

void inputDigitalTask(void* arg) {
    uint32_t pin;
    bool val;

    while (1) {
        if (xQueueReceive(digitalInterruptQueue, &pin, portMAX_DELAY)) {
            val = digitalRead(pin);
            for (uint8_t i = 0; i < DIGITAL_AMT; i++) {
                if (digitalMap[i].inputPin == pin && digitalMap[i].value != val) {
                    digitalMap[i].value = val;
                    handleDigitalEvent(&digitalMap[i]);
                    break;
                }
            }
        }
    }
}

void inputTask(void* arg) {
    BaseType_t xResult;
    uint32_t notifiedValue;

    TS_Point point;
    uint8_t pipe;

    while (1) {
        // wait to be notified of an interrupt
        xResult = xTaskNotifyWait(pdFALSE, ULONG_MAX, &notifiedValue, portMAX_DELAY);

        if (xResult == pdPASS) {
            // Touch
            if (notifiedValue & TOUCH_BIT) {
                if (touch.touched()) {
                    point = touch.getPoint();

                    onTouchEvent(&point);
                }
            }

            // RF
            if (notifiedValue & RF_BIT) {
                while (radio.available(&pipe)) {
                    Payload payload;
                    radio.read(&payload, sizeof(payload));

                    onPayloadEvent(&payload);
                }
            }

            // MCP port extender
            if (notifiedValue & MCP_BIT) {
                uint8_t pin = mcp.getLastInterruptPin();
                bool value = mcp.digitalRead(pin); //clears MCP
                for (uint8_t i = 0; i < DIGITAL_AMT; i++) {
                    if (digitalMap[i].inputPin == pin && digitalMap[i].value != value) {
                        digitalMap[i].value = digitalRead(pin);

                        handleDigitalEvent(&digitalMap[i]);
                    }
                }
            }
        }
    }
}

void initIO() {
    static bool initialized = false;

    if (!initialized) {
        // backlight & tft graphics
        ledcSetup(TFT_CH, TFT_FRQ, TFT_RES);
        ledcAttachPin(TFT_BACKLIGHT_PIN, TFT_CH);
        ledcWrite(TFT_CH, 255);

        gfx.begin();
        gfx.setRotation(1);

        // radio - RF24
        radio.begin();
        radio.setChannel(84);
        radio.openReadingPipe(0, addressDiscovery);  // open to discovery address
        radio.enableDynamicPayloads();
        radio.startListening();
        pinMode(RF_IRQ_PIN, INPUT);
        attachInterrupt(RF_IRQ_PIN, isrRFHandler, PULLDOWN);

        // touch - STMPE610
        touch.begin();
        pinMode(STMPE_IRQ_PIN, INPUT);
        attachInterrupt(STMPE_IRQ_PIN, isrTouchHandler, PULLDOWN);

        // mcp port extender
        mcp.begin();
        mcp.setupInterrupts(true, false, LOW);
        mcp.readGPIOAB();
        pinMode(MCP_A_IRQ_PIN, INPUT);
        attachInterrupt(MCP_A_IRQ_PIN, isrMCPHandler, FALLING);

        // digital input
        digitalInterruptQueue = xQueueCreate(10, sizeof(uint8_t));
        for (uint8_t i = 0; i < DIGITAL_AMT; i++) {
            if (digitalMap[i].inputPin == eLeft_Aux || digitalMap[i].inputPin == eRight_Aux) {
                // MCU pins
                pinMode(digitalMap[i].inputPin, INPUT);
                attachInterruptArg(digitalMap[i].inputPin, isrDigitalHandler, (void*)static_cast<uint32_t>(digitalMap[i].inputPin), CHANGE);
            } else {
                // MCP pins
                mcp.pinMode(digitalMap[i].inputPin, INPUT);
                mcp.setupInterruptPin(digitalMap[i].inputPin, CHANGE);
            }
        }

        // tasks
        xTaskCreatePinnedToCore(&inputTask, "inputIO", 1500, NULL, 13, &inputHandlingTask, IO_CORE);
        xTaskCreatePinnedToCore(&inputDigitalTask, "digitalIO", 1500, NULL, 11, NULL, IO_CORE);
        xTaskCreatePinnedToCore(&inputAnalogTask, "analogIO", 1500, NULL, 12, NULL, IO_CORE);

        initialized = true;
    }
}
