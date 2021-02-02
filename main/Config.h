#ifndef RC_CONFIG_H
#define RC_CONFIG_H

#define IO_CORE 0

// TOUCH
#define STMPE_CS_PIN 12
#define STMPE_IRQ_PIN 14

// TFT
#define TFT_CS_PIN 5
#define TFT_DC_PIN 13
#define TFT_MOSI_PIN 23
#define TFT_SCK_PIN 18
#define TFT_RST_PIN -1
#define TFT_MISO_PIN 19
#define TFT_BACKLIGHT_PIN 2
#define TFT_CH 0
#define TFT_RES 8
#define TFT_FRQ 5000

// RF
#define RF_CE_PIN 25
#define RF_CS_PIN 26
#define RF_IRQ_PIN 1

// ANALOG
#define VOLTAGE_PIN 4
#define L_JOY_X_PIN 32
#define L_JOY_Y_PIN 33
#define R_JOY_X_PIN 34
#define R_JOY_Y_PIN 35
#define L_TRIG_PIN 36
#define R_TRIG_PIN 39

// MCP23017 port expander
#define MCP_A_IRQ_PIN 0
#define MCP_B_IRQ_PIN 15

// DIGITAL
#define L_AUX_BTN_PIN 16
#define R_AUX_BTN_PIN 17

// note: below digital are using the MCP23017 port expander
#define L_BUMPER_BTN_MCP_PIN 0
#define R_BUMPER_BTN_MCP_PIN 8

#define L_DPAD_UP_MCP_PIN 1
#define L_DPAD_DN_MCP_PIN 2
#define L_DPAD_RT_MCP_PIN 3
#define L_DPAD_LT_MCP_PIN 4

#define R_DPAD_UP_MCP_PIN 9
#define R_DPAD_DN_MCP_PIN 10
#define R_DPAD_RT_MCP_PIN 11
#define R_DPAD_LT_MCP_PIN 12

#endif