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
#define EPD_WIDTH       122
#define EPD_HEIGHT      250

#define FULL_UPDATE 0
#define PART_UPDATE 1

UBYTE EPD_Init(UBYTE update);
void EPD_Clear(void);
void EPD_TurnOnDisplay(void);
void EPD_Display(UBYTE *Image);
void EPD_DisplayWindows(UBYTE *Image, UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend);
void EPD_DisplayPart(UBYTE *Image);
void EPD_DisplayPartWindows(UBYTE *Image, UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend);
void EPD_Sleep(void);

#endif /* EPD2IN13_H */
