/*****************************************************************************
* | File      	:	EPD_2in13.c
* | Author      :   Waveshare team
* | Function    :
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2018-06-07
* | Info        :   Basic version
*
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>

#include "FEPD_2in13.h"
#include "GUI_Paint.h"
#include "GUI_Cache.h"
#include "Debug.h"

#include "../gpio.h"

const unsigned char lut_full_update[] = {
    0x80, 0x60, 0x40, 0x00, 0x00, 0x00, 0x00,       //LUT0: BB:     VS 0 ~7
    0x10, 0x60, 0x20, 0x00, 0x00, 0x00, 0x00,       //LUT1: BW:     VS 0 ~7
    0x80, 0x60, 0x40, 0x00, 0x00, 0x00, 0x00,       //LUT2: WB:     VS 0 ~7
    0x10, 0x60, 0x20, 0x00, 0x00, 0x00, 0x00,       //LUT3: WW:     VS 0 ~7
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,       //LUT4: VCOM:   VS 0 ~7

    0x03, 0x03, 0x00, 0x00, 0x02,                   // TP0 A~D RP0
    0x09, 0x09, 0x00, 0x00, 0x02,                   // TP1 A~D RP1
    0x03, 0x03, 0x00, 0x00, 0x02,                   // TP2 A~D RP2
    0x00, 0x00, 0x00, 0x00, 0x00,                   // TP3 A~D RP3
    0x00, 0x00, 0x00, 0x00, 0x00,                   // TP4 A~D RP4
    0x00, 0x00, 0x00, 0x00, 0x00,                   // TP5 A~D RP5
    0x00, 0x00, 0x00, 0x00, 0x00,                   // TP6 A~D RP6

    0x15, 0x41, 0xA8, 0x32, 0x30, 0x0A,
};

const unsigned char lut_partial_update[] = { //20 bytes
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,       //LUT0: BB:     VS 0 ~7
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,       //LUT1: BW:     VS 0 ~7
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,       //LUT2: WB:     VS 0 ~7
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,       //LUT3: WW:     VS 0 ~7
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,       //LUT4: VCOM:   VS 0 ~7

    0x0A, 0x00, 0x00, 0x00, 0x00,                   // TP0 A~D RP0
    0x00, 0x00, 0x00, 0x00, 0x00,                   // TP1 A~D RP1
    0x00, 0x00, 0x00, 0x00, 0x00,                   // TP2 A~D RP2
    0x00, 0x00, 0x00, 0x00, 0x00,                   // TP3 A~D RP3
    0x00, 0x00, 0x00, 0x00, 0x00,                   // TP4 A~D RP4
    0x00, 0x00, 0x00, 0x00, 0x00,                   // TP5 A~D RP5
    0x00, 0x00, 0x00, 0x00, 0x00,                   // TP6 A~D RP6

    0x15, 0x41, 0xA8, 0x32, 0x30, 0x0A,
};


/******************************************************************************
  function :	Software reset
  parameter:
******************************************************************************/
static void EPD_Reset(void)
{
    EPD_RST_1;
    DEV_Delay_ms(200);
    EPD_RST_0;
    DEV_Delay_ms(200);
    EPD_RST_1;
    DEV_Delay_ms(200);
}

/******************************************************************************
  function :	send command
  parameter:
     Reg : Command register
******************************************************************************/
static void EPD_SendCommand(UBYTE Reg)
{
    EPD_DC_0;
    EPD_CS_0;
    DEV_SPI_WriteByte(Reg);
    EPD_CS_1;
}

/******************************************************************************
  function :	send data
  parameter:
    Data : Write data
******************************************************************************/
static void EPD_SendData(UBYTE Data)
{
    EPD_DC_1;
    EPD_CS_0;
    DEV_SPI_WriteByte(Data);
    EPD_CS_1;
}

/******************************************************************************
  function :	Wait until the busy_pin goes LOW
  parameter:
******************************************************************************/
void EPD_WaitUntilIdle(void)
{
    DEBUG("e-Paper busy\r\n");
    while (EPD_BUSY_RD == 1) {     //LOW: idle, HIGH: busy
        DEV_Delay_ms(100);
    }
    DEBUG("e-Paper busy release\r\n");
}

/******************************************************************************
  function :	Turn On Display
  parameter:
******************************************************************************/
void EPD_TurnOnDisplay(void)
{
    EPD_SendCommand(0x22);
    EPD_SendData(0xC7);
    //EPD_SendData(0x0c);
    EPD_SendCommand(0x20);
    EPD_WaitUntilIdle();
}

/******************************************************************************
  function :	Initialize the e-Paper register
  parameter:
******************************************************************************/
UBYTE EPD_Init(UBYTE update)
{
    UBYTE count;
    EPD_Reset();

    if (update == FULL_UPDATE) {
        EPD_WaitUntilIdle();
        EPD_SendCommand(0x12); // soft reset
        EPD_WaitUntilIdle();

        EPD_SendCommand(0x74); //set analog block control
        EPD_SendData(0x54);
        EPD_SendCommand(0x7E); //set digital block control
        EPD_SendData(0x3B);

        EPD_SendCommand(0x01); //Driver output control
        EPD_SendData(0xF9);
        EPD_SendData(0x00);
        EPD_SendData(0x00);

        EPD_SendCommand(0x11); //data entry mode
        EPD_SendData(0x01);

        EPD_SendCommand(0x44); //set Ram-X address start/end position
        EPD_SendData(0x00);
        EPD_SendData(0x0F);    //0x0C-->(15+1)*8=128

        EPD_SendCommand(0x45); //set Ram-Y address start/end position
        EPD_SendData(0xF9);   //0xF9-->(249+1)=250
        EPD_SendData(0x00);
        EPD_SendData(0x00);
        EPD_SendData(0x00);

        EPD_SendCommand(0x3C); //BorderWavefrom
        EPD_SendData(0x03);

        EPD_SendCommand(0x2C);     //VCOM Voltage
        EPD_SendData(0x55);    //

        EPD_SendCommand(0x03);
        EPD_SendData(lut_full_update[70]);

        EPD_SendCommand(0x04); //
        EPD_SendData(lut_full_update[71]);
        EPD_SendData(lut_full_update[72]);
        EPD_SendData(lut_full_update[73]);

        EPD_SendCommand(0x3A);     //Dummy Line
        EPD_SendData(lut_full_update[74]);
        EPD_SendCommand(0x3B);     //Gate time
        EPD_SendData(lut_full_update[75]);

        EPD_SendCommand(0x32);
        for (count = 0; count < 70; count++)
            EPD_SendData(lut_full_update[count]);

        EPD_SendCommand(0x4E);   // set RAM x address count to 0;
        EPD_SendData(0x00);
        EPD_SendCommand(0x4F);   // set RAM y address count to 0X127;
        EPD_SendData(0xF9);
        EPD_SendData(0x00);
        EPD_WaitUntilIdle();
    } else {
        EPD_SendCommand(0x2C);     //VCOM Voltage
        EPD_SendData(0x26);

        EPD_WaitUntilIdle();

        EPD_SendCommand(0x32);
        for (count = 0; count < 70; count++)
            EPD_SendData(lut_partial_update[count]);

        EPD_SendCommand(0x37);
        EPD_SendData(0x00);
        EPD_SendData(0x00);
        EPD_SendData(0x00);
        EPD_SendData(0x00);
        EPD_SendData(0x40);
        EPD_SendData(0x00);
        EPD_SendData(0x00);

        EPD_SendCommand(0x22);
        EPD_SendData(0xC0);
        EPD_SendCommand(0x20);
        EPD_WaitUntilIdle();

        EPD_SendCommand(0x3C); //BorderWavefrom
        EPD_SendData(0x01);
    }
    return 0;
}

/**
    @brief: private function to specify the memory area for data R/W
*/
static void EPD_SetWindows(int x_start, int y_start, int x_end, int y_end)
{
    EPD_SendCommand(0x44);
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    EPD_SendData((x_start >> 3) & 0xFF);
    EPD_SendData((x_end >> 3) & 0xFF);
    EPD_SendCommand(0x45);
    EPD_SendData(y_start & 0xFF);
    EPD_SendData((y_start >> 8) & 0xFF);
    EPD_SendData(y_end & 0xFF);
    EPD_SendData((y_end >> 8) & 0xFF);
}

/**
    @brief: private function to specify the start point for data R/W
*/
static void EPD_SetCursor(int x, int y)
{
    EPD_SendCommand(0x4E);
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    EPD_SendData((x >> 3) & 0xFF);
    EPD_SendCommand(0X4F);
    EPD_SendData(y & 0xFF);
    EPD_SendData((y >> 8) & 0xFF);
    EPD_WaitUntilIdle();
}

/******************************************************************************
  function :	Clear screen
  parameter:
******************************************************************************/
void EPD_Clear(void)
{
    UWORD Width, Height;
    Width = (EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8 ) : (EPD_WIDTH / 8 + 1);
    Height = EPD_HEIGHT;
    EPD_SendCommand(0x24);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_SendData(0XFF);
        }
    }
    EPD_TurnOnDisplay();
}

/******************************************************************************
  function :	Sends the image buffer in RAM to e-Paper and displays
  parameter:
******************************************************************************/
void EPD_Display(UBYTE *Image)
{
    UWORD Width, Height;
    Width = (EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8 ) : (EPD_WIDTH / 8 + 1);
    Height = EPD_HEIGHT;

    EPD_SendCommand(0x24);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_SendData(Image[i + j * Width]);
        }
    }
    EPD_TurnOnDisplay();
}

void EPD_DisplayWindows(UBYTE *Image, UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend)
{
    UWORD Width, Height;
    Width = ((Xend - Xstart) % 8 == 0) ? ((Xend - Xstart) / 8 ) : ((Xend - Xstart) / 8 + 1);
    Height = Yend - Ystart;

    EPD_SetWindows(Xstart, Ystart, Xend, Yend);
    UWORD i, j;
    for (j = 0; j < Height; j++) {
        EPD_SetCursor(Xstart, Ystart + j);
        EPD_SendCommand(0x24);
        for (i = 0; i < Width; i++) {
            EPD_SendData(Image[i + j * Width]);
        }
    }
}

void EPD_DisplayPart(UBYTE *Image)
{
    UWORD Width, Height;
    Width = (EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8 ) : (EPD_WIDTH / 8 + 1);
    Height = EPD_HEIGHT;
    EPD_SendCommand(0x24);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_SendData(Image[i + j * Width]);
        }
    }

    EPD_SendCommand(0x26);   //Write Black and White image to RAM
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_SendData(~Image[i + j * Width]);
        }
    }
    EPD_TurnOnDisplay();
}

void EPD_DisplayPartWindows(UBYTE *Image, UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend)
{
    UWORD Width, Height;
    Width = ((Xend - Xstart) % 8 == 0) ? ((Xend - Xstart) / 8 ) : ((Xend - Xstart) / 8 + 1);
    Height = Yend - Ystart;

    EPD_SetWindows(Xstart, Ystart, Xend, Yend);
    UWORD i, j;
    for (j = 0; j < Height; j++) {
        EPD_SetCursor(Xstart, Ystart + j);
        EPD_SendCommand(0x24);
        for (i = 0; i < Width; i++) {
            EPD_SendData(Image[i + j * Width]);
        }
    }

    for (j = 0; j < Height; j++) {
        EPD_SetCursor(Xstart, Ystart + j);
        EPD_SendCommand(0x26);
        for (i = 0; i < Width; i++) {
            EPD_SendData(~Image[i + j * Width]);
        }
    }
}

/******************************************************************************
  function :	Enter sleep mode
  parameter:
******************************************************************************/
void EPD_Sleep(void)
{
    EPD_SendCommand(0x22); //POWER OFF
    EPD_SendData(0xC3);
    EPD_SendCommand(0x20);

    EPD_SendCommand(0x10); //enter deep sleep
    EPD_SendData(0x01);
    DEV_Delay_ms(100);
}
