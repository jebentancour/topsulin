/*****************************************************************************
* | File      	:   GUI_Paint.h
* | Author      :   Waveshare team
* | Function    :	Achieve drawing: draw points, lines, boxes, circles and
*                   their size, solid dotted line, solid rectangle hollow
*                   rectangle, solid circle hollow circle.
* | Info        :
*   Achieve display characters: Display a single character, string, number
*   Achieve time display: adaptive size display time minutes and seconds
*----------------
* |	This version:   V1.0
* | Date        :   2018-05-31
* | Info        :
*
******************************************************************************/
#ifndef __GUI_GUI_H
#define __GUI_GUI_H

#include "DEV_Config.h"
#include "fonts.h"

/**
 * Display orientation
**/
#define IMAGE_ROTATE_0            0
#define IMAGE_ROTATE_180          1

/**
 * Image color level
**/
#define IMAGE_COLOR_POSITIVE 0X01
#define IMAGE_COLOR_INVERTED 0X02

/**
 * Image attributes
**/
typedef struct {
    UWORD Image_Width;
    UWORD Image_Height;
    UWORD Image_Rotate;
    UWORD Image_Color;
    UWORD Memory_Width;
    UWORD Memory_Height;
} GUI_IMAGE;
extern GUI_IMAGE GUI_Image;

/**
 * Image color
**/
#define WHITE          0xFF
#define BLACK          0x00

#define IMAGE_BACKGROUND    WHITE
#define FONT_FOREGROUND     BLACK
#define FONT_BACKGROUND     WHITE

/**
 * The size of the point
**/
typedef enum {
    DOT_PIXEL_1X1  = 1,	// 1 x 1
    DOT_PIXEL_2X2  , 		// 2 X 2
    DOT_PIXEL_3X3  ,		// 3 X 3
    DOT_PIXEL_4X4  ,		// 4 X 4
    DOT_PIXEL_5X5  , 		// 5 X 5
    DOT_PIXEL_6X6  , 		// 6 X 6
    DOT_PIXEL_7X7  , 		// 7 X 7
    DOT_PIXEL_8X8  , 		// 8 X 8
} DOT_PIXEL;
#define DOT_PIXEL_DFT  DOT_PIXEL_1X1  //Default dot pilex

/**
 * Point size fill style
**/
typedef enum {
    DOT_FILL_AROUND  = 1,		// dot pixel 1 x 1
    DOT_FILL_RIGHTUP  , 		// dot pixel 2 X 2
} DOT_STYLE;
#define DOT_STYLE_DFT  DOT_FILL_AROUND  //Default dot pilex

/**
 * Line style, solid or dashed
**/
typedef enum {
    LINE_STYLE_SOLID = 0,
    LINE_STYLE_DOTTED,
} LINE_STYLE;

/**
 * Whether the graphic is filled
**/
typedef enum {
    DRAW_FILL_EMPTY = 0,
    DRAW_FILL_FULL,
} DRAW_FILL;

/**
 * Custom structure of a time attribute
**/
typedef struct {
    UWORD  Year;  //0000
    UBYTE  Month; //1 - 12
    UBYTE  Day;   //1 - 30
    UBYTE  Hour;  //0 - 23
    UBYTE  Min;   //0 - 59
    UBYTE  Sec;   //0 - 59
} GUI_TIME;
extern GUI_TIME sGUI_time;

//Init and Clear
void GUI_NewImage(UWORD Width, UWORD Height, UWORD Rotate, UWORD Color);
void GUI_Clear(UWORD Color);
void GUI_ClearWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color);

//Drawing
void GUI_DrawPoint(UWORD Xpoint, UWORD Ypoint, UWORD Color, DOT_PIXEL Dot_Pixel, DOT_STYLE Dot_FillWay);
void GUI_DrawLine(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color, LINE_STYLE Line_Style, DOT_PIXEL Dot_Pixel);
void GUI_DrawRectangle(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color, DRAW_FILL Filled , DOT_PIXEL Dot_Pixel);
void GUI_DrawCircle(UWORD X_Center, UWORD Y_Center, UWORD Radius, UWORD Color, DRAW_FILL Draw_Fill , DOT_PIXEL Dot_Pixel);

//Display string
void GUI_DrawChar(UWORD Xstart, UWORD Ystart, const char Acsii_Char, sFONT* Font, UWORD Color_Background, UWORD Color_Foreground);
void GUI_DrawString_EN(UWORD Xstart, UWORD Ystart, const char * pString, sFONT* Font, UWORD Color_Background, UWORD Color_Foreground);
void GUI_DrawNum(UWORD Xpoint, UWORD Ypoint, int32_t Nummber, sFONT* Font, UWORD Color_Background, UWORD Color_Foreground);
void GUI_DrawTime(UWORD Xstart, UWORD Ystart, GUI_TIME *pTime, sFONT* Font, UWORD Color_Background, UWORD Color_Foreground);

//pic
void GUI_DrawBitMap(const unsigned char* image_buffer);

#endif /* GUI_GUI_H */
