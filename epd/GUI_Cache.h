/*****************************************************************************
* | File      	:   GUI_Cache.h
* | Author      :   Waveshare team
* | Function    :	When the controller's memory is sufficient, open up a part 
*                   of the memory for the image cache
*----------------
* |	This version:   V1.0
* | Date        :   2018-06-29
* | Info        :
*
******************************************************************************/
#ifndef __GUI_CACHE_H
#define __GUI_CACHE_H

#include "DEV_Config.h"
#include "FEPD_2in13.h"

extern UBYTE ImageBuff[EPD_HEIGHT * EPD_WIDTH / 8];

#endif

