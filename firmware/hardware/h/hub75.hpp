#pragma once

#undef USE_GFX_ROOT
#undef NO_GFX

#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another

// Color definitions
#define BLACK565    0x0000
#define BLUE565     0x001F
#define RED565      0xF800
#define ORANGE565   0xfd40
#define GREEN565    0x07E0
#define CYAN565     0x07FF
#define MAGENTA565  0xF81F
#define YELLOW565   0xFFE0 
#define WHITE565    0xFFFF

//minimal correctly displaying brightness
#define MIN_BRIGHTNESS 15

void HUB75Init();
void HUB75TestScreen();
void HUB75SetBrigthness(uint8_t brightness);