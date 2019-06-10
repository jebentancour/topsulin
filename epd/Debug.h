/*****************************************************************************
* | File      	:	Debug.h
* | Author      :   Waveshare team
* | Function    :	debug with printf
* | Info        :
*   Image scanning
*      Please use progressive scanning to generate images or fonts
*----------------
* |	This version:   V1.0
* | Date        :   2018-01-11
* | Info        :   Basic version
*
******************************************************************************/
#ifndef __DEBUG_H
#define __DEBUG_H

#define NRF_LOG_MODULE_NAME "EPD"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define USE_DEBUG 1
#if USE_DEBUG
	#define DEBUG(__info,...) NRF_LOG_INFO(__info,##__VA_ARGS__)
#else
	#define DEBUG(__info,...)
#endif

#endif
