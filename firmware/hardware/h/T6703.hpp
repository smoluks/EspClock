//Thanks drug123 https://github.com/drug123/T67XX
#pragma once

#include "../../libs/pt-1.4/pt.h"

#define T67XX_I2C_ADDR 0x15

#define T67XX_REG_FIRMWARE 0x1389
#define T67XX_REG_STATUS 0x138A
#define T67XX_REG_PPM 0x138B
#define T67XX_REG_RESET 0x03E8
#define T67XX_REG_SPCAL 0x03EC
#define T67XX_REG_FLASH_UPDATE 0x03ED
#define T67XX_REG_ADDRESS 0x0FA5
#define T67XX_REG_ABC_LOGIC 0x03EE
#define T67XX_REG_MOD_MODE 0x100B

#define T67XX_READ_DELAY 10
#define T67XX_MEASURE_DELAY 2250

inline void T6703Init();
inline void T6703Loop();
