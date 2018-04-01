#pragma once

#include "driver\spi_register.h"
#include "ets_sys.h"

typedef enum
{
    SpiSpeed_0_5MHz     = 160,
    SpiSpeed_1MHz       = 80,
    SpiSpeed_2MHz       = 40,
    SpiSpeed_5MHz       = 16,
    SpiSpeed_8MHz       = 10,
    SpiSpeed_10MHz      = 8,
} SpiSpeed;

/*SPI number define*/
#define SPI 			0
#define HSPI			1

//spi master init funtion
void spi_master_init();

//use spi send 16bit data
void spi_send_max7219_command(uint16 data);

void spi_send_max7219_data(uint16* data);


