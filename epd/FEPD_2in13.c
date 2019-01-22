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
#include "FEPD_2in13.h"
#include "GUI_Paint.h"
#include "GUI_Cache.h"
#include <stdlib.h>
#include "Debug.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * full screen update LUT
**/
const unsigned char lut_vcomDC[] = {
    0x00, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x60, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x00, 0x14, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00,
};
const unsigned char lut_ww[] = {
    0x40, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x40, 0x14, 0x00, 0x00, 0x00, 0x01,
    0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const unsigned char lut_bw[] = {
    0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
    0x90, 0x0F, 0x0F, 0x00, 0x00, 0x03,
    0x40, 0x0A, 0x01, 0x00, 0x00, 0x01,
    0xA0, 0x0E, 0x0E, 0x00, 0x00, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const unsigned char lut_wb[] = {
    0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
    0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const unsigned char lut_bb[] = {
    0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
    0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/**
 * partial screen update LUT
**/
const unsigned char lut_vcom1[] = {
    0x00, 0x19, 0x01, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00,
};
const unsigned char lut_ww1[] = {
    0x00, 0x19, 0x01, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const unsigned char lut_bw1[] = {
    0x80, 0x19, 0x01, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const unsigned char lut_wb1[] = {
    0x40, 0x19, 0x01, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const unsigned char lut_bb1[] = {
    0x00, 0x19, 0x01, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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
static void EPD_WaitUntilIdle(void)
{
	DEBUG("BUSY....\r\n");
    UBYTE busy;
    do {
        EPD_SendCommand(0x71);
        busy = EPD_BUSY_RD;
        busy =!(busy & 0x01);
    } while(busy);
    // DEV_Delay_ms(50);
		DEBUG("BUSY free....\r\n");
}

/******************************************************************************
function :	LUT download
parameter:
******************************************************************************/
static void EPD_SetFullReg(void)
{
    EPD_SendCommand(0X50);			 //VCOM AND DATA INTERVAL SETTING
    EPD_SendData(0x97);		       //WBmode:VBDF 17|D7 VBDW 97 VBDB 57		WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7

    unsigned int count;
    EPD_SendCommand(0x20);
    for(count=0; count<44; count++) {
        EPD_SendData(lut_vcomDC[count]);
    }

    EPD_SendCommand(0x21);
    for(count=0; count<42; count++) {
        EPD_SendData(lut_ww[count]);
    }

    EPD_SendCommand(0x22);
    for(count=0; count<42; count++) {
        EPD_SendData(lut_bw[count]);
    }

    EPD_SendCommand(0x23);
    for(count=0; count<42; count++) {
        EPD_SendData(lut_wb[count]);
    }

    EPD_SendCommand(0x24);
    for(count=0; count<42; count++) {
        EPD_SendData(lut_bb[count]);
    }
}

/******************************************************************************
function :	LUT download
parameter:
******************************************************************************/
static void EPD_SetPartReg(void)
{
    EPD_SendCommand(0x82);			//vcom_DC setting
    EPD_SendData(0x03);
    EPD_SendCommand(0X50);
    EPD_SendData(0x47);

    unsigned int count;
    EPD_SendCommand(0x20);
    for(count=0; count<44; count++) {
        EPD_SendData(lut_vcom1[count]);
    }

    EPD_SendCommand(0x21);
    for(count=0; count<42; count++) {
        EPD_SendData(lut_ww1[count]);
    }

    EPD_SendCommand(0x22);
    for(count=0; count<42; count++) {
        EPD_SendData(lut_bw1[count]);
    }

    EPD_SendCommand(0x23);
    for(count=0; count<42; count++) {
        EPD_SendData(lut_wb1[count]);
    }

    EPD_SendCommand(0x24);
    for(count=0; count<42; count++) {
        EPD_SendData(lut_bb1[count]);
    }
}

/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
UBYTE EPD_Init()
{
    EPD_Reset();

    EPD_SendCommand(0x01);	//POWER SETTING
    EPD_SendData(0x03);
    EPD_SendData(0x00);
    EPD_SendData(0x2b);
    EPD_SendData(0x2b);
    EPD_SendData(0x03);

    EPD_SendCommand(0x06);	//boost soft start
    EPD_SendData(0x17);     //A
    EPD_SendData(0x17);     //B
    EPD_SendData(0x17);     //C

    EPD_SendCommand(0x04);
    // EPD_WaitUntilIdle();

    EPD_SendCommand(0x00);	//panel setting
    EPD_SendData(0xbf);     //LUT from OTP 128x296
    EPD_SendData(0x0d);     //VCOM to 0V fast

    EPD_SendCommand(0x30);	//PLL setting
    EPD_SendData(0x3a);     // 3a 100HZ /29 150Hz /39 200HZ	/31 171HZ /3c 50hz

    EPD_SendCommand(0x61);	//resolution setting
    EPD_SendData(EPD_WIDTH);
    EPD_SendData((EPD_HEIGHT >> 8) & 0xff);
    EPD_SendData(EPD_HEIGHT& 0xff);

    EPD_SendCommand(0x82);	//vcom_DC setting
    EPD_SendData(0x28);

    return 0;
}

/******************************************************************************
function :	Turn On Display
parameter:
******************************************************************************/
void EPD_TurnOnDisplay(void)
{
    EPD_SendCommand(0x12);		  //DISPLAY REFRESH
    DEV_Delay_ms(1);            //!!!The delay here is necessary, 200uS at least!!!

    EPD_WaitUntilIdle();
}


/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/
void EPD_Clear(void)
{
    UWORD Width, Height;
    Width = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);
    Height = EPD_HEIGHT;

    EPD_SendCommand(0x10);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_SendData(0x00);
        }
    }
    DEV_Delay_ms(10);

    EPD_SendCommand(0x13);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_SendData(0xFF);
        }
    }
    DEV_Delay_ms(10);

    EPD_SetFullReg();
    EPD_TurnOnDisplay();
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void EPD_DisplayFull(void)
{
    UWORD Byte_Width, Byte_Height;
    Byte_Width = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);
    Byte_Height = EPD_HEIGHT;

    EPD_SendCommand(0x10);
    for (UWORD j = 0; j < Byte_Height; j++) {
        for (UWORD i = 0; i < Byte_Width; i++) {
            EPD_SendData(0x00);
        }
    }
    DEV_Delay_ms(10);

    EPD_SendCommand(0x13);
    for (UWORD j = 0; j < Byte_Height; j++) {
        for (UWORD i = 0; i < Byte_Width; i++) {
            EPD_SendData(ImageBuff[i + j * Byte_Width]);
        }
    }
    DEV_Delay_ms(10);

    EPD_SetFullReg();
    EPD_TurnOnDisplay();
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void EPD_DisplayPartial(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend)
{
    /* Set partial Windows */
    UWORD X0, Y0, X1, Y1;
    X0 = 0;
    Y0 = 0;
    X1 = 0;
    Y1 = 0;
    switch(GUI_Image.Image_Rotate) {
    case IMAGE_ROTATE_0:
        X0 = Xstart;
        Y0 = Ystart;
        X1 = Xend - 1;
        Y1 = Yend - 1;
        break;
    case IMAGE_ROTATE_180:
        X0 = GUI_Image.Image_Width - Xend;
        Y0 = GUI_Image.Image_Height - Yend;
        X1 = GUI_Image.Image_Width - Xstart - 1;
        Y1 = GUI_Image.Image_Height - Ystart - 1;
        break;
    }

    UWORD Dx0 = X0 / 8;
    UWORD Dx1 = X1 / 8;

    EPD_SetPartReg();
    EPD_SendCommand(0x91);		//This command makes the display enter partial mode
    EPD_SendCommand(0x90);		//resolution setting
    EPD_SendData(X0);           //x-start
    EPD_SendData(X1 - 1);       //x-end

    EPD_SendData(Y0 / 256);
    EPD_SendData(Y0 % 256);     //y-start
    EPD_SendData(Y1 / 256);
    EPD_SendData(Y1 % 256 - 1);  //y-end
    EPD_SendData(0x28);

    UWORD Width;
    Width = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);

    /* Send data */
    EPD_SendCommand(0x10);
    for (UWORD j = Y0; j < Y1 + 1; j++) {
        for (UWORD i = Dx0; i < Dx1 + 1; i++) {
            EPD_SendData(ImageBuff[i + j * Width]);
        }
    }

    EPD_SendCommand(0x13);
    for (UWORD j = Y0; j < Y1 + 1; j++) {
        for (UWORD i = Dx0; i < Dx1 + 1; i++) {
            EPD_SendData(~ImageBuff[i + j * Width]);
        }
    }

    /* Set partial refresh */
    EPD_TurnOnDisplay();
}


/******************************************************************************
function :	Enter sleep mode
parameter:
******************************************************************************/
void EPD_Sleep(void)
{
    EPD_SendCommand(0X50);
    EPD_SendData(0xf7);
    EPD_SendCommand(0X02);  	//power off
    EPD_SendCommand(0X07);  	//deep sleep
    EPD_SendData(0xA5);
}
