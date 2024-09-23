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

#define T67XX_ERROR_CONDITION   1 << 0
#define T67XX_ERROR_FLASH       1 << 1
#define T67XX_ERROR_CALIBRATION 1 << 2
//#DEFINE T67XX_ERROR_NA        1 << 3
//#DEFINE T67XX_ERROR_NA        1 << 4
//#DEFINE T67XX_ERROR_NA        1 << 5
//#DEFINE T67XX_ERROR_NA        1 << 6
//#DEFINE T67XX_ERROR_NA        1 << 7
#define T67XX_ERROR_RS232       1 << 8
#define T67XX_ERROR_RS485       1 << 9
#define T67XX_ERROR_I2C         1 << 10
#define T67XX_ERROR_WARMUP      1 << 11
//#DEFINE T67XX_ERROR_NA        1 << 12
//#DEFINE T67XX_ERROR_NA        1 << 13
//#DEFINE T67XX_ERROR_NA        1 << 14
#define T67XX_ERROR_SINGLEPOINTCALIBRATION 1 << 15

inline void T6703Init();
inline void T6703Loop();
