/*
 * max7219.c
 *
 *  Created on: 28 мар. 2018 г.
 *      Author: Администратор
 */

#include "c_types.h"
#include "osapi.h"
#include "driver/max7219.h"

void max7219_send_videobuffer(uint8_t* videobuffer)
{
	uint16_t buffer[6];
	uint8_t i;
	uint8_t j;

	for(i = 0; i < 8; i++)
	{
		for(j = 0; j<6; j++)
			buffer[j] = (videobuffer[j*8+i] << 8) + i + 1;

		spi_send_max7219_data(buffer);
	}
}

void max7219_send_decodemode(uint8_t bitmask)
{
	spi_send_max7219_command((bitmask << 8) + 0x0009);
}

void max7219_send_intensity(uint8_t intensity)
{
	spi_send_max7219_command(((intensity & 0x0F) << 8) + 0x000A);
}

void max7219_send_scanlimit(uint8_t scanlimit)
{
	spi_send_max7219_command(((scanlimit & 0x07) << 8) + 0x0B);
}

void max7219_send_shutdown(bool shutdown)
{
	spi_send_max7219_command(((shutdown ? 0 : 1) << 8)+ 0x000C);
}

void max7219_send_test(bool on)
{
	spi_send_max7219_command(((on ? 1 : 0) << 8) + 0x000F);
}
