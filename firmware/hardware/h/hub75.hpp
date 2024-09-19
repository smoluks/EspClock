#pragma once

#undef USE_GFX_ROOT
#undef NO_GFX

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

void HUB75Init();
void HUB75SetBrigthness(uint8_t brightness);
void HUB75DecreaseBrigthnessLimit();
void HUB75ResetBrigthnessLimit();