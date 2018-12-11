/*****************************************************************************
* | File      	: DEV_Config.h
* | Author      : Waveshare team
* | Function    :	debug with prntf
* | Info        :
*   Image scanning
*      Please use progressive scanning to generate images or fonts
*----------------
* |	This version:   V1.0
* | Date        :   2018-01-11
* | Info        :   Basic version
*
******************************************************************************/
#ifndef _DEV_CONFIG_H_
#define _DEV_CONFIG_H_

#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include <stdint.h>

/**
 * data
**/
#define UBYTE   uint8_t
#define UWORD   uint16_t
#define UDOUBLE uint32_t

/**
 * e-Paper GPIO
**/
#define MISO_PIN        20
#define MOSI_PIN        19
#define CLK_PIN         18

#define CS_PIN          22
#define DC_PIN          23
#define RST_PIN         24
#define BUSY_PIN        25

#define EPD_CS_0		nrf_gpio_pin_clear(CS_PIN)
#define EPD_CS_1		nrf_gpio_pin_set(CS_PIN)

#define EPD_DC_0		nrf_gpio_pin_clear(DC_PIN)
#define EPD_DC_1		nrf_gpio_pin_set(DC_PIN)

#define EPD_RST_0		nrf_gpio_pin_clear(RST_PIN)
#define EPD_RST_1		nrf_gpio_pin_set(RST_PIN)

#define EPD_BUSY_RD		nrf_gpio_pin_read(BUSY_PIN)

UBYTE DEV_ModuleInit(void);

UBYTE DEV_SPI_WriteByte(UBYTE value);
UBYTE DEV_SPI_ReadByte(UBYTE value);

void DEV_Delay_ms(UWORD xms);

#endif