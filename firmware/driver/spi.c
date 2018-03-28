#include "driver/spi.h"
#include <osapi.h>
#include <gpio.h>

void spi_set_speed(int speed);

void ICACHE_FLASH_ATTR spi_master_init() {
	uint32 regvalue;

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2); //configure io to Hspi mode
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2); //configure io to Hspi mode
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2); //configure io to Hspi mode

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15); //configure io to gpio

	CLEAR_PERI_REG_MASK(SPI_PIN(HSPI), SPI_IDLE_EDGE);
	CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_CK_OUT_EDGE);

	CLEAR_PERI_REG_MASK(SPI_CTRL(HSPI), SPI_WR_BIT_ORDER);
	CLEAR_PERI_REG_MASK(SPI_CTRL(HSPI), SPI_RD_BIT_ORDER);

	// Disable flash operation mode
	CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_FLASH_MODE);
	// SPI mode type
	CLEAR_PERI_REG_MASK(SPI_SLAVE(HSPI), SPI_SLAVE_MODE);
	// SPI Send buffer
	SET_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MISO_HIGHPART);// By default slave send buffer C0-C7
	// 80Mhz speed
	//WRITE_PERI_REG(SPI_CLOCK(HSPI), SPI_CLK_EQU_SYSCLK);
	spi_set_speed(SpiSpeed_10MHz);
	// By default format:CMD+ADDR+DATA
	SET_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MOSI);
	//SET_PERI_REG_MASK(SPI_USER(HSPI), SPI_CS_SETUP | SPI_CS_HOLD | SPI_USR_MOSI );
	//delay num
	SET_PERI_REG_MASK(SPI_CTRL2(HSPI),
			((0x1 & SPI_MISO_DELAY_NUM) << SPI_MISO_DELAY_NUM_S));
}

void spi_set_speed(int speed) {
	uint8 i, k;
	i = (speed / 40) ? (speed / 40) : 1;
	k = speed / i;
	CLEAR_PERI_REG_MASK(SPI_CLOCK(HSPI), SPI_CLK_EQU_SYSCLK);
	WRITE_PERI_REG(SPI_CLOCK(HSPI),
			(((i - 1) & SPI_CLKDIV_PRE) << SPI_CLKDIV_PRE_S) | (((k - 1) & SPI_CLKCNT_N) << SPI_CLKCNT_N_S) | ((((k + 1) / 2 - 1) & SPI_CLKCNT_H) << SPI_CLKCNT_H_S) | (((k - 1) & SPI_CLKCNT_L) << SPI_CLKCNT_L_S)); //clear bit 31,set SPI clock div
}

void spi_mast_byte_write(uint16 data)
{
	uint32 regvalue;

	while (READ_PERI_REG(SPI_CMD(HSPI)) & SPI_USR);

	CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MOSI|SPI_USR_MISO);

	SET_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_COMMAND);

	//SPI_FLASH_USER2 bit28-31 is cmd length,cmd bit length is value(0-15)+1,
	// bit15-0 is cmd value.
	WRITE_PERI_REG(SPI_USER2(HSPI),	((15)<<SPI_USR_COMMAND_BITLEN_S)|((uint32)data));

	SET_PERI_REG_MASK(SPI_CMD(HSPI), SPI_USR);

	while (READ_PERI_REG(SPI_CMD(HSPI)) & SPI_USR);
}

void spi_cs_down()
{
	GPIO_OUTPUT_SET(15, 0);
}

void spi_cs_up()
{
	GPIO_OUTPUT_SET(15, 1);
}

