/*****************************************************************************
* | File      	:	DEV_Config.cpp
* | Author      : Waveshare team
* | Function    :
* | Info        :
*   Image scanning
*      Please use progressive scanning to generate images or fonts
*----------------
* |	This version:   V1.0
* | Date        :   2018-01-11
* | Info        :   Basic version
*
******************************************************************************/
#include "DEV_Config.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

/******************************************************************************
function:	Initialize Pins and SPI
******************************************************************************/

#define SPI_INSTANCE  0 /**< SPI instance index. */
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */

UBYTE DEV_ModuleInit(void)
{
    // GPIO INIT
    nrf_gpio_cfg_output(CS_PIN);
    nrf_gpio_cfg_output(DC_PIN);
    nrf_gpio_cfg_output(RST_PIN);
    nrf_gpio_cfg_input(BUSY_PIN,NRF_GPIO_PIN_PULLUP);

    // SPI INIT
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = NRF_DRV_SPI_PIN_NOT_USED;
    spi_config.miso_pin = NRF_DRV_SPI_PIN_NOT_USED;
    spi_config.mosi_pin = MOSI_PIN;
    spi_config.sck_pin  = CLK_PIN;
    nrf_drv_spi_init(&spi, &spi_config, NULL);

    EPD_CS_1;

    return 0;
}

/*********************************************
function:	Hardware interface
*********************************************/
UBYTE DEV_SPI_WriteByte(UBYTE value)
{
    return nrf_drv_spi_transfer(&spi, &value, sizeof(value), NULL, 0);
}

UBYTE DEV_SPI_ReadByte(UBYTE value)
{
	return DEV_SPI_WriteByte(value);
}

/******************************************************************************
function:	Millisecond delay
******************************************************************************/
void DEV_Delay_ms(UWORD xms)
{
    nrf_delay_ms(xms);
}
