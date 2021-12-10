#include "driver/spi.h"
#include <osapi.h>
#include <gpio.h>

void spi_set_speed(int speed);

void ICACHE_FLASH_ATTR spi_master_init() {
	uint32 regvalue;

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2); //configure io to Hspi mode
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2); //configure io to Hspi mode
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2); //configure io to Hspi mode
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, 2); //configure io to Hspi mode

	CLEAR_PERI_REG_MASK(SPI_PIN(HSPI), SPI_IDLE_EDGE);
	CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_CK_OUT_EDGE);

	CLEAR_PERI_REG_MASK(SPI_CTRL(HSPI), SPI_WR_BIT_ORDER);
	CLEAR_PERI_REG_MASK(SPI_CTRL(HSPI), SPI_RD_BIT_ORDER);

	// By default format:CMD+ADDR+DATA
	//SET_PERI_REG_MASK(SPI_USER(HSPI), SPI_CS_SETUP | SPI_CS_HOLD | SPI_USR_COMMAND );
	// Disable flash operation mode
	CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_FLASH_MODE);
	// SPI mode type
	CLEAR_PERI_REG_MASK(SPI_SLAVE(HSPI), SPI_SLAVE_MODE);
	// SPI Send buffer
	//SET_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MISO_HIGHPART);// By default slave send buffer C0-C7
	//delay num
	//SET_PERI_REG_MASK(SPI_CTRL2(HSPI), ((0x1 & SPI_MISO_DELAY_NUM) << SPI_MISO_DELAY_NUM_S));
	//clear Daul or Quad lines transmission mode
	//CLEAR_PERI_REG_MASK(SPI_CTRL(HSPI), SPI_QIO_MODE | SPI_DIO_MODE | SPI_DOUT_MODE | SPI_QOUT_MODE);
	// Clear the data buffer.
	//uint8 i;
	//uint32 regAddr = REG_SPI_BASE(HSPI) + 0x40;
	//  for (i = 0; i < 16; ++i) {
	//        WRITE_PERI_REG(regAddr, 0);
	//        regAddr += 4;
	//    }
	// 80Mhz speed
	//WRITE_PERI_REG(SPI_CLOCK(HSPI), SPI_CLK_EQU_SYSCLK);
	spi_set_speed(SpiSpeed_1MHz);
	//WRITE_PERI_REG(SPI_CLOCK(HSPI),
	//					((3&SPI_CLKCNT_N)<<SPI_CLKCNT_N_S)|
	//					((1&SPI_CLKCNT_H)<<SPI_CLKCNT_H_S)|
	//					((3&SPI_CLKCNT_L)<<SPI_CLKCNT_L_S)); //clear bit 31,set SPI clock div
}

void ICACHE_FLASH_ATTR spi_set_speed(int speed) {
	uint8 i, k;
	i = (speed / 40) ? (speed / 40) : 1;
	k = speed / i;
	CLEAR_PERI_REG_MASK(SPI_CLOCK(HSPI), SPI_CLK_EQU_SYSCLK);
	WRITE_PERI_REG(SPI_CLOCK(HSPI),
			(((i - 1) & SPI_CLKDIV_PRE) << SPI_CLKDIV_PRE_S) | (((k - 1) & SPI_CLKCNT_N) << SPI_CLKCNT_N_S) | ((((k + 1) / 2 - 1) & SPI_CLKCNT_H) << SPI_CLKCNT_H_S) | (((k - 1) & SPI_CLKCNT_L) << SPI_CLKCNT_L_S)); //clear bit 31,set SPI clock div
}

void ICACHE_FLASH_ATTR spi_send_max7219_command(uint16 data)
{
	//wait ready
	while (READ_PERI_REG(SPI_CMD(HSPI))&SPI_USR);

	CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MOSI|SPI_USR_MISO|SPI_USR_COMMAND|SPI_USR_ADDR|SPI_USR_DUMMY);

	// Set data send buffer length.Max data length 64 bytes.
    WRITE_PERI_REG(SPI_USER1(HSPI),((95 & SPI_USR_MOSI_BITLEN)<<SPI_USR_MOSI_BITLEN_S));

    SET_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MOSI);

	// Load send buffer
	WRITE_PERI_REG(SPI_W0(HSPI), (data << 16) + data);
	WRITE_PERI_REG(SPI_W1(HSPI), (data << 16) + data);
	WRITE_PERI_REG(SPI_W2(HSPI), (data << 16) + data);

	// -----Start send data -----
	SET_PERI_REG_MASK(SPI_CMD(HSPI), SPI_USR);
	return;
}

void ICACHE_FLASH_ATTR spi_send_max7219_data(uint16* data)
{
	// Load send buffer
	WRITE_PERI_REG(SPI_W1(HSPI), (*(data+1) << 16) + (*(data+0)));
	WRITE_PERI_REG(SPI_W2(HSPI), (*(data+3) << 16) + (*(data+2)));
	WRITE_PERI_REG(SPI_W3(HSPI), (*(data+5) << 16) + (*(data+4)));

	//wait ready
	while (READ_PERI_REG(SPI_CMD(HSPI))&SPI_USR);
	//-----command-----
	SET_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_MOSI|SPI_USR_DOUTDIN);
	CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_USR_COMMAND|SPI_USR_ADDR|SPI_USR_DUMMY);

	// Set data send buffer length.Max data length 64 bytes.
	WRITE_PERI_REG(SPI_USER1(HSPI),((95 & SPI_USR_MOSI_BITLEN)<<SPI_USR_MOSI_BITLEN_S));
	// -----Start send data -----
	SET_PERI_REG_MASK(SPI_CMD(HSPI), SPI_USR);
	while(READ_PERI_REG(SPI_CMD(HSPI))&SPI_USR);
	// -----Wait for transmit done-----
	while (!(READ_PERI_REG(SPI_SLAVE(HSPI))&SPI_TRANS_DONE));
	CLEAR_PERI_REG_MASK(SPI_SLAVE(HSPI), SPI_TRANS_DONE);

	return;
}

