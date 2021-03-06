#include "IO.h"

#include <Adafruit_MCP23017.h>
#include <Adafruit_STMPE610.h>
#include <EEPROM.h>
#include <RF24.h>

#include "System.h"

#define EEPROM_SIZE 512
#define EEPROM_VERSION 1
#define EEPROM_VERSION_ADDRESS 0
#define EEPROM_ANALOG_ADDRESS 1
#define EEPROM_ANALOG_COUNT 6
#define EEPROM_ANALOG_BYTES 8

#define TOUCH_TASK_BIT 0x01
#define RF_TASK_BIT 0x02
#define MCP_TASK_BIT 0x04

#define DIGITAL_AMT 12
#define ANALOG_AMT 7

#define ANALOG_TASK_DELAY_MS 50

#define ANALOG_JOY_MIN 250
#define ANALOG_JOY_MAX 150
#define ANALOG_TRIGGER_MIN 200
#define ANALOG_TRIGGER_MAX 350

#define ANALOG_MIN 0
#define ANALOG_MID 2047
#define ANALOG_MAX 4095

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 100
#define TS_MAXX 3800
#define TS_MINY 100
#define TS_MAXY 3750

const byte addressDiscovery[6] = "RC-00";

Adafruit_HX8357 gfx = Adafruit_HX8357(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN);
Adafruit_STMPE610 touch = Adafruit_STMPE610(STMPE_CS_PIN);
RF24 radio = RF24(RF_CE_PIN, RF_CS_PIN);
Adafruit_MCP23017 mcp;

xQueueHandle digitalInterruptQueue = NULL;
static TaskHandle_t inputHandlingTask;

AnalogMap voltage = {eVoltage, 2650, 1700, 1700, 2650, false};
AnalogMap leftJoyX = {eLeft_JoyX, ANALOG_MID, ANALOG_MID, ANALOG_MIN, ANALOG_MAX, false};
AnalogMap leftJoyY = {eLeft_JoyY, ANALOG_MID, ANALOG_MID, ANALOG_MIN, ANALOG_MAX, true};
AnalogMap rightJoyX = {eRight_JoyX, ANALOG_MID, ANALOG_MID, ANALOG_MIN, ANALOG_MAX, true};
AnalogMap rightJoyY = {eRight_JoyY, ANALOG_MID, ANALOG_MID, ANALOG_MIN, ANALOG_MAX, false};
AnalogMap leftTrigger = {eLeft_Trigger, ANALOG_MID, ANALOG_MID, ANALOG_MIN, ANALOG_MAX, false};
AnalogMap rightTrigger = {eRight_Trigger, ANALOG_MID, ANALOG_MID, ANALOG_MIN, ANALOG_MAX, false};

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

AnalogMap* analogMap[ANALOG_AMT] = {
    &leftJoyX,
    &leftJoyY,
    &rightJoyX,
    &rightJoyY,
    &leftTrigger,
    &rightTrigger,
    &voltage, // must be last element of array
};

void AnalogMap::setHomeMinMax(uint16_t val) {
    home = val;
    min = val;
    max = val;
}

void AnalogMap::reclampMinMax() {
    if (min > value) {
        min = value;
    }
    if (max < value) {
        max = value;
    }
}

int8_t AnalogMap::getPercentValue() {
    if (inputPin == eLeft_Trigger || inputPin == eRight_Trigger || inputPin == eVoltage) {
        return getMapValue(0, 100);
    }
    return getMapValue(-100, 100);
}

int16_t AnalogMap::getMapValue(int16_t toMin, int16_t toMax) {
    if (inverted) {
        int16_t temp = toMin;
        toMin = toMax;
        toMax = temp;
    }

    if (inputPin == eLeft_Trigger || inputPin == eRight_Trigger || inputPin == eVoltage) {
        if (value <= home + ANALOG_TRIGGER_MIN) {
            return toMin;  // toMin -> below home plus drift
        }
        if (value + ANALOG_TRIGGER_MAX >= max) {
            return toMax;  // toMax -> above max minus drift
        }
        return map(value, home + ANALOG_TRIGGER_MIN, max - ANALOG_TRIGGER_MAX, toMin, toMax);
    } else {
        int16_t toMid = (toMax - toMin) / 2 + toMin;
        if (value + ANALOG_JOY_MIN >= home && value <= home + ANALOG_JOY_MIN) {
            return toMid;  // toMid -> in home position +/- drift
        }
        if (value <= min + ANALOG_JOY_MAX) {
            return toMin;  // toMin -> below min plus drift
        }
        if (value + ANALOG_JOY_MAX >= max) {
            return toMax;  // toMax -> above max minus drift
        }

        if (value < home) {
            return map(value, min + ANALOG_JOY_MAX, home - ANALOG_JOY_MIN, toMin, toMid);
        }
        return map(value, home + ANALOG_JOY_MIN, max - ANALOG_JOY_MAX, toMid, toMax);
    }
}

AnalogMap* getAnalogMap(eGamepadAnalog gamepadAnalog) {
    for (uint8_t i = 0; i < ANALOG_AMT; i++) {
        if (analogMap[i]->inputPin == gamepadAnalog) {
            return analogMap[i];
        }
    }
}

void handleDigitalEvent(DigitalMap* map) {
    GamepadEvent ev = {digital : map};

    Serial.print(F("button:"));
    Serial.print(map->inputPin % MCP_PIN_BIT);
    Serial.print(F(" value:"));
    Serial.println(map->value);

    onGamepadEvent(ev);
}

void IRAM_ATTR isrDigitalHandler(void* arg) {
    uint32_t pin = (uint32_t)arg;
    xQueueSendFromISR(digitalInterruptQueue, &pin, NULL);
    portYIELD_FROM_ISR();
}

void IRAM_ATTR isrRFHandler() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(inputHandlingTask, RF_TASK_BIT, eSetBits, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR();
}

void IRAM_ATTR isrTouchHandler() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(inputHandlingTask, TOUCH_TASK_BIT, eSetBits, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR();
}

void IRAM_ATTR isrMCPHandler() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(inputHandlingTask, MCP_TASK_BIT, eSetBits, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR();
}

void inputAnalogTask(void* arg) {
    static const TickType_t xDelay = ANALOG_TASK_DELAY_MS / portTICK_PERIOD_MS;  // 50ms
    static const float max = 255.0f;

    while (1) {
        for (uint8_t i = 0; i < ANALOG_AMT; i++) {
            analogMap[i]->value = analogRead(analogMap[i]->inputPin);  // range: 0 to 4095

            if (analogMap[i]->inputPin == eLeft_Trigger || analogMap[i]->inputPin == eRight_Trigger) {
                // TODO : handle trigger value
            } else {
                // TODO : handle joystick value
            }
        }
        vTaskDelay(xDelay);
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
    TouchPoint touchPoint;
    uint8_t pipe;

    while (1) {
        // wait to be notified of an interrupt
        xResult = xTaskNotifyWait(ULONG_MAX, ULONG_MAX, &notifiedValue, portMAX_DELAY);

        if (xResult == pdPASS) {
            // Touch
            if (notifiedValue & TOUCH_TASK_BIT) {
                point = touch.getPoint();

                if (point.y >= TS_MINX && point.y <= TS_MAXX && point.x >= TS_MINY && point.x <= TS_MAXY) {
                    touchPoint.x = map(point.y, TS_MINX, TS_MAXX, 0, gfx.width());
                    touchPoint.y = map(point.x, TS_MINY, TS_MAXY, gfx.height(), 0);

                    Serial.print(F("touch x:"));
                    Serial.print(touchPoint.x);
                    Serial.print(F(" y:"));
                    Serial.println(touchPoint.y);

                    onTouchEvent(touchPoint);
                }
            }

            // RF
            if (notifiedValue & RF_TASK_BIT) {
                while (radio.available(&pipe)) {
                    Payload payload;
                    radio.read(&payload, sizeof(payload));

                    onPayloadEvent(payload);
                }
            }

            // MCP port extender
            if (notifiedValue & MCP_TASK_BIT) {
                uint8_t pin = mcp.getLastInterruptPin();
                bool value = mcp.getLastInterruptPinValue();
                for (uint8_t i = 0; i < DIGITAL_AMT; i++) {
                    if (digitalMap[i].inputPin % MCP_PIN_BIT == pin && digitalMap[i].value != value) {
                        digitalMap[i].value = value;

                        if (pin == eLeft_Bumper) {
                            touch.writeRegister8(STMPE_INT_STA, 0xFF);
                        }

                        handleDigitalEvent(&digitalMap[i]);
                    }
                }
            }
        }
    }
}

union AnalogMemory {
    struct __attribute__((__packed__)) {
        uint8_t pin;
        bool inverted;
        uint16_t home;
        uint16_t min;
        uint16_t max;
    } inputs[EEPROM_ANALOG_COUNT];
    byte value[EEPROM_ANALOG_BYTES * EEPROM_ANALOG_COUNT];
};

void loadEEPROM() {
    uint8_t version = EEPROM.read(EEPROM_VERSION_ADDRESS);

    Serial.print(F("EEPROM: "));
    Serial.println(version);

    if (version == 1) {
        AnalogMemory storedAnalog;

        EEPROM.readBytes(EEPROM_ANALOG_ADDRESS, &storedAnalog.value, EEPROM_ANALOG_BYTES * EEPROM_ANALOG_COUNT);

        for (uint8_t i = 0; i < EEPROM_ANALOG_COUNT; i++) {
            for (uint8_t j = 0; j < ANALOG_AMT; j++) {
                if (analogMap[j]->inputPin == storedAnalog.inputs[i].pin) {
                    analogMap[j]->inverted = storedAnalog.inputs[i].inverted;
                    analogMap[j]->home = storedAnalog.inputs[i].home;
                    analogMap[j]->min = storedAnalog.inputs[i].min;
                    analogMap[j]->max = storedAnalog.inputs[i].max;

                    Serial.print(F("pin:"));
                    Serial.print(storedAnalog.inputs[i].pin);
                    Serial.print(F(",inv:"));
                    Serial.print(storedAnalog.inputs[i].inverted);
                    Serial.print(F(",home:"));
                    Serial.print(storedAnalog.inputs[i].home);
                    Serial.print(F(",min:"));
                    Serial.print(storedAnalog.inputs[i].min);
                    Serial.print(F(",max:"));
                    Serial.println(storedAnalog.inputs[i].max);
                    break;
                }
            }
        }
    }
}

void saveEEPROM() {
    AnalogMemory storedAnalog = {
        inputs: {
            {pin: leftJoyX.inputPin, inverted: leftJoyX.inverted, home: leftJoyX.home, min: leftJoyX.min, max: leftJoyX.max},
            {pin: leftJoyY.inputPin, inverted: leftJoyY.inverted, home: leftJoyY.home, min: leftJoyY.min, max: leftJoyY.max},
            {pin: rightJoyX.inputPin, inverted: rightJoyX.inverted, home: rightJoyX.home, min: rightJoyX.min, max: rightJoyX.max},
            {pin: rightJoyY.inputPin, inverted: rightJoyY.inverted, home: rightJoyY.home, min: rightJoyY.min, max: rightJoyY.max},
            {pin: leftTrigger.inputPin, inverted: leftTrigger.inverted, home: leftTrigger.home, min: leftTrigger.min, max: leftTrigger.max},
            {pin: rightTrigger.inputPin, inverted: rightTrigger.inverted, home: rightTrigger.home, min: rightTrigger.min, max: rightTrigger.max},
        }
    };

    EEPROM.write(EEPROM_VERSION_ADDRESS, EEPROM_VERSION);
    EEPROM.writeBytes(EEPROM_ANALOG_ADDRESS, &storedAnalog.value, EEPROM_ANALOG_BYTES * EEPROM_ANALOG_COUNT);
    EEPROM.commit();

    for (uint8_t i = 0; i < EEPROM_ANALOG_COUNT; i++) {
        Serial.print(F("pin:"));
        Serial.print(storedAnalog.inputs[i].pin);
        Serial.print(F(",inv:"));
        Serial.print(storedAnalog.inputs[i].inverted);
        Serial.print(F(",home:"));
        Serial.print(storedAnalog.inputs[i].home);
        Serial.print(F(",min:"));
        Serial.print(storedAnalog.inputs[i].min);
        Serial.print(F(",max:"));
        Serial.println(storedAnalog.inputs[i].max);
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
        gfx.invertDisplay(false); //prevent randomly inverted tft when power is flickered off/on

        if (!touch.begin()) {
            Serial.println(F("Touch.begin() failed"));
        }

        // radio - RF24
        radio.begin();
        radio.setChannel(84);
        radio.openReadingPipe(0, addressDiscovery);  // open to discovery address
        radio.enableDynamicPayloads();
        radio.startListening();
        pinMode(RF_IRQ_PIN, INPUT);
        attachInterrupt(RF_IRQ_PIN, isrRFHandler, PULLDOWN);

        // touch - STMPE610
        pinMode(STMPE_IRQ_PIN, INPUT_PULLDOWN);
        attachInterrupt(STMPE_IRQ_PIN, isrTouchHandler, CHANGE);

        // mcp port extender
        mcp.begin();
        mcp.setupInterrupts(true, false, LOW);
        mcp.readGPIOAB();
        pinMode(MCP_A_IRQ_PIN, INPUT);
        attachInterrupt(MCP_A_IRQ_PIN, isrMCPHandler, CHANGE);

        // digital input
        digitalInterruptQueue = xQueueCreate(10, sizeof(uint8_t));
        for (uint8_t i = 0; i < DIGITAL_AMT; i++) {
            if (digitalMap[i].inputPin >= MCP_PIN_BIT) {
                // MCP pins
                mcp.pinMode(digitalMap[i].inputPin % MCP_PIN_BIT, INPUT);
                mcp.setupInterruptPin(digitalMap[i].inputPin % MCP_PIN_BIT, CHANGE);
            } else {
                // MCU pins
                pinMode(digitalMap[i].inputPin, INPUT);
                attachInterruptArg(digitalMap[i].inputPin, isrDigitalHandler, (void*)digitalMap[i].inputPin, CHANGE);
            }
        }

        // tasks
        xTaskCreatePinnedToCore(&inputTask, "inputIO", 2048, NULL, 13, &inputHandlingTask, IO_CORE);
        xTaskCreatePinnedToCore(&inputDigitalTask, "digitalIO", 1500, NULL, 11, NULL, IO_CORE);
        xTaskCreatePinnedToCore(&inputAnalogTask, "analogIO", 1500, NULL, 12, NULL, IO_CORE);

        EEPROM.begin(EEPROM_SIZE);

        loadEEPROM();

        initialized = true;
    }
}

void analogInputsResample() {
    const TickType_t xDelay = ANALOG_TASK_DELAY_MS / portTICK_PERIOD_MS;
    const uint8_t samples = 16; // max for uint16 -> 16*4095=65520 [uint16: 0 to 65,535]
    uint16_t values[samples][ANALOG_AMT - 1];

    // get 10x sets of inputs for averaging
    for (uint8_t i = 0; i < samples; i++) {
        vTaskDelay(xDelay);
        for (uint8_t j = 0; j < ANALOG_AMT - 1; j++) {
            values[i][j] = analogMap[j]->value;
        }
    }

    // average the inputs
    uint16_t value;
    for (uint8_t j = 0; j < ANALOG_AMT - 1; j++) {
        value = 0;
        for (uint8_t i = 0; i < samples; i++) {
            value += values[i][j];
        }
        analogMap[j]->setHomeMinMax(value / samples);
    }
}

bool isCalibrated(AnalogMap& a) {
    if (a.inputPin == eLeft_Trigger || a.inputPin == eRight_Trigger || a.inputPin == eVoltage) {
        return a.home + ANALOG_TRIGGER_MIN + ANALOG_TRIGGER_MAX < a.max;
    }
    return a.min + ANALOG_JOY_MAX + ANALOG_JOY_MIN < a.home && a.home + ANALOG_JOY_MIN + ANALOG_JOY_MAX < a.max;
}

bool isAnalogCalibrated() {
    return isCalibrated(leftJoyX) && isCalibrated(leftJoyY) && 
        isCalibrated(rightJoyX) && isCalibrated(rightJoyY) &&
        isCalibrated(leftTrigger) && isCalibrated(rightTrigger);
}