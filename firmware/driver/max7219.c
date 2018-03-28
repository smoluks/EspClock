/*
 * max7219.c
 *
 *  Created on: 28 мар. 2018 г.
 *      Author: Администратор
 */

#include "c_types.h"
#include "driver/max7219.h"

void max7219_send_test(bool on)
{
	spi_cs_down();
	uint8_t i;

	for(i = 0; i < 6; i++)
		spi_mast_byte_write(0x010F);

	spi_cs_up();
}

