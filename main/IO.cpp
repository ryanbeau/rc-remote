#include "IO.h"

#include <RF24.h>

#include "System.h"

#define DIGITAL_AMT 10
#define ANALOG_AMT 6

#define OFFSET_INIT_JOY 0
#define LIMIT_INIT_JOY 255

#define OFFSET_INIT_TRIGGER 100
#define LIMIT_INIT_TRIGGER 155

bool initialized = false;
static xQueueHandle inputQueue = NULL;

const byte addressDiscovery[6] = "RC-00";

Adafruit_STMPE610 touch = Adafruit_STMPE610(STMPE_CS_PIN, STMPE_SDI_PIN, STMPE_SDO_PIN, STMPE_SCK_PIN);
RF24 radio = RF24(RF_CE_PIN, RF_CS_PIN);

typedef struct {
    uint8_t inputPin;
    bool value;
} DigitalMap;

static DigitalMap digitalMap[DIGITAL_AMT] = {
    {static_cast<uint8_t>(Gamepad::L_DPad_Up), false},
    {static_cast<uint8_t>(Gamepad::L_DPad_Down), false},
    {static_cast<uint8_t>(Gamepad::L_DPad_Right), false},
    {static_cast<uint8_t>(Gamepad::L_DPad_Left), false},
    {static_cast<uint8_t>(Gamepad::R_DPad_Up), false},
    {static_cast<uint8_t>(Gamepad::R_DPad_Down), false},
    {static_cast<uint8_t>(Gamepad::R_DPad_Right), false},
    {static_cast<uint8_t>(Gamepad::R_DPad_Left), false},
    {static_cast<uint8_t>(Gamepad::L_Aux), false},
    {static_cast<uint8_t>(Gamepad::R_Aux), false},
};

typedef struct {
    uint8_t inputPin;
    uint16_t value;  // analogRead() returns 0 to 4095
    uint16_t limit;
    uint16_t offset;
} AnalogMap;

static AnalogMap analogMap[ANALOG_AMT] = {
    {static_cast<uint8_t>(Gamepad::L_Joy_X), 0, LIMIT_INIT_JOY, OFFSET_INIT_JOY},
    {static_cast<uint8_t>(Gamepad::L_Joy_Y), 0, LIMIT_INIT_JOY, OFFSET_INIT_JOY},
    {static_cast<uint8_t>(Gamepad::R_Joy_X), 0, LIMIT_INIT_JOY, OFFSET_INIT_JOY},
    {static_cast<uint8_t>(Gamepad::R_Joy_Y), 0, LIMIT_INIT_JOY, OFFSET_INIT_JOY},
    {static_cast<uint8_t>(Gamepad::L_Trigger), 0, LIMIT_INIT_TRIGGER, OFFSET_INIT_TRIGGER},
    {static_cast<uint8_t>(Gamepad::R_Trigger), 0, LIMIT_INIT_TRIGGER, OFFSET_INIT_TRIGGER},
};

void IRAM_ATTR isrHandler(void* arg) {
    uint32_t pin = (uint32_t)arg;
    xQueueSendFromISR(inputQueue, &pin, NULL);
}

void inputRFTask(void* arg) {
    const TickType_t xDelay = 50 / portTICK_PERIOD_MS;  // 50ms

    uint8_t pipe;
    while (1) {
        vTaskDelay(xDelay);

        while (radio.available(&pipe)) {
            Payload payload;
            radio.read(&payload, sizeof(payload));

            OnPayloadEvent(&payload);
        }
    }
}

void inputDigitalTask(void* arg) {
    uint32_t pin;
    GamepadEvent ev;

    while (1) {
        if (xQueueReceive(inputQueue, &pin, portMAX_DELAY)) {
            for (uint8_t i = 0; i < DIGITAL_AMT; i++) {
                if (digitalMap[i].inputPin == pin) {
                    digitalMap[i].value = digitalRead(pin);

                    ev.type = digitalMap[i].value ? buttonDown : buttonUp;
                    ev.input = static_cast<Gamepad>(pin);
                    ev.button = digitalMap[i].value;

                    OnGamepadEvent(&ev);
                    break;
                }
            }
        }
    }
}

void inputAnalogTask(void* arg) {
    const TickType_t xDelay = 50 / portTICK_PERIOD_MS;  // 50ms
    uint16_t value;

    while (1) {
        vTaskDelay(xDelay);
        for (uint8_t i = 0; i < ANALOG_AMT; i++) {
            value = analogRead(analogMap[i].inputPin);
            analogMap[i].value = value;
        }
    }
}

void inputTouchTask(void* arg) {
    const TickType_t xDelay = 50 / portTICK_PERIOD_MS;  // 50ms
    TS_Point point;

    while (1) {
        vTaskDelay(xDelay);
        if (touch.touched()) {
            point = touch.getPoint();
            OnTouchEvent(&point);
        }
    }
}

void ioInit() {
    if (!initialized) {
        // radio - RF24
        radio.begin();
        radio.setChannel(84);
        radio.openReadingPipe(0, addressDiscovery);  // open to discovery address
        radio.enableDynamicPayloads();
        radio.startListening();

        // touch - STMPE610
        touch.begin();

        // digital input
        inputQueue = xQueueCreate(10, sizeof(uint8_t));
        for (uint8_t i = 0; i < DIGITAL_AMT; i++) {
            pinMode(digitalMap[i].inputPin, INPUT);
            attachInterruptArg(digitalMap[i].inputPin, isrHandler, (void*)static_cast<uint32_t>(digitalMap[i].inputPin), CHANGE);
        }

        // TODO : interrupt for RF
        // TODO : interrupt for Touch

        // tasks
        xTaskCreatePinnedToCore(&inputRFTask, "rfIO", 1500, NULL, 10, NULL, IO_CORE);
        xTaskCreatePinnedToCore(&inputDigitalTask, "digitalIO", 1500, NULL, 11, NULL, IO_CORE);
        xTaskCreatePinnedToCore(&inputAnalogTask, "analogIO", 1500, NULL, 12, NULL, IO_CORE);
        xTaskCreatePinnedToCore(&inputTouchTask, "touchIO", 1500, NULL, 13, NULL, IO_CORE);

        initialized = true;
    }
}
