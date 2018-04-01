/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/1/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "driver\spi.h"
#include "user_interface.h"
#include "driver/uart.h"

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABBBCDDD
 *                A : rf cal
 *                B : at parameters
 *                C : rf init data
 *                D : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 8;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

//spi_byte_write_espslave(HSPI,0xAA);
//spi_WR_espslave(HSPI);
//SET_PERI_REG_MASK(SPI_PIN(SPI), SPI_CS_DIS);
//CLEAR_PERI_REG_MASK(SPI_PIN(SPI), SPI_CS_DIS)

void ICACHE_FLASH_ATTR user_rf_pre_init(void)
{

}

void ICACHE_FLASH_ATTR user_init(void)
{
	uart_init(BIT_RATE_115200,BIT_RATE_115200);
	spi_master_init();
	//
	os_printf("\nSDK version:%s\n", system_get_sdk_version());
	//

	//
	//os_delay_us(60000);
	//max7219_send_test(false);
	//max7219_send_intensity(0x1);
	max7219_send_test(false);
	max7219_send_shutdown(false);
	//
	uint8_t videobuffer[48];
	memset(videobuffer, 0, 48);
	videobuffer[0] = 1;
	max7219_send_videobuffer(videobuffer);
	//
}
