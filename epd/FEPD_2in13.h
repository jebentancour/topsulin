/*****************************************************************************
* | File  	:	EPD_2in13.h
* | Author  :   Waveshare team
* | Function:
* | Info:
*----------------
* |	This version:   V1.0
* | Date:   2018-06-07
* | Info:   Basic version
*
******************************************************************************/
#ifndef __FEPD_2IN13_H_
#define __FEPD_2IN13_H_

#include "DEV_Config.h"

// Display resolution
#define EPD_WIDTH   104
#define EPD_HEIGHT  212

UBYTE EPD_Init(void);
void EPD_Clear(void);
void EPD_DisplayFull(void);
void EPD_DisplayPartial(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend);
void EPD_Sleep(void);

#endif /* EPD2IN13_H */
