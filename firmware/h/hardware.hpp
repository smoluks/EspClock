#pragma once

//-----Pins-----
//HUB75
#define R1_PIN 25
#define G1_PIN 27
#define B1_PIN 26
#define R2_PIN 14
#define G2_PIN 13
#define B2_PIN 12
#define A_PIN 2
#define B_PIN 0
#define C_PIN 17
#define D_PIN 5
#define E_PIN -1
#define LAT_PIN 4
#define OE_PIN 18
#define CLK_PIN 16

//ADC
#define LIGHT_ANALOG_PIN 34
#define VOLTAGE_PIN 36
#define CURRENT_PIN 39

//EINT
#define FUSB302_INT_PIN 35

//TOUCH
#define TOUCH_PIN 32

//I2C
#define SDA_PIN 21
#define SCL_PIN 22

//General out
#define CHARGER_EN_PIN 33

//-----Serial-----
#define DY1703Serial Serial
#define DebugSerial Serial
#define DEBUG_SPEED 115200

//-----HUB75-----
#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another

//minimal correctly displaying brightness
#define MIN_BRIGHTNESS 15