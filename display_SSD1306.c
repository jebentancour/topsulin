#include "display_SSD1306.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "i2c.h"
#include "app_util_platform.h"
#include "app_error.h"

#define NRF_LOG_MODULE_NAME "DISPLAY"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define SSD1306_I2C_ADDRESS 0x3C

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA

#define SSD1306_SETVCOMDETECT 0xDB

#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9

#define SSD1306_SETMULTIPLEX 0xA8

#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10

#define SSD1306_SETSTARTLINE 0x40

#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR 0x22

#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8

#define SSD1306_SEGREMAP 0xA0

#define SSD1306_CHARGEPUMP 0x8D

#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2

#define SSD1306_ACTIVATE_SCROLL 0x2F
#define SSD1306_DEACTIVATE_SCROLL 0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A

#define SSD1306_LCDWIDTH 128
#define SSD1306_LCDHEIGHT 32

// the memory buffer for the LCD

// Los bytes estan colgados de arriba a abajo y van avanzando de izquierda a derecha:
// byte0       byte1         ..... byte127
// bit0        bit0
// bit1        bit1
// bit2        bit2
// ....        ....
// byte(128*i) byte(128*i+1)

static uint8_t buffer[SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8];/* = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0xF0, 0xF0, 0x00,
0x00, 0x00, 0x00, 0xC0, 0xE0, 0x70, 0x30, 0x30, 0x70, 0xE0,
0xC0, 0x00, 0x00, 0x60, 0x60, 0x30, 0x30, 0xB0, 0xF0, 0xE0,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x20, 0x30, 0x30, 0xF0, 0xF0, 0x00, 0x00, 0x00, 0x00,
0x00, 0xC0, 0xF0, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00,
0x00, 0x00, 0x80, 0x80, 0x80, 0xE0, 0xE0, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x80, 0xE0, 0xF0, 0x30, 0x30, 0x70, 0xE0, 0xC0,
0x00, 0x00, 0xE0, 0xE0, 0x30, 0x30, 0x30, 0xF0, 0xE0, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80,
0x60, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x3F, 0x7F, 0xE0, 0xC0, 0xC0, 0xE0, 0x7F, 0x3F, 0x00,
0x10, 0x70, 0xF0, 0xC3, 0xC3, 0xC7, 0x7F, 0x7C, 0x18, 0x00,
0x00, 0x10, 0x7C, 0x7F, 0x7F, 0x7F, 0x7E, 0x18, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x60, 0x67,
0xC7, 0xC3, 0xC3, 0x7F, 0x7E, 0x18, 0x00, 0x00, 0x00, 0x1F,
0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x3F, 0x1F, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x1F, 0x7F, 0xF0, 0xC0, 0xC0, 0xE0, 0x7F, 0x3F, 0x00, 0x80,
0xC0, 0xE0, 0xF0, 0xF8, 0xDE, 0xCF, 0xC3, 0xC0, 0x00, 0x00,
0x80, 0x58, 0x3C, 0x7E, 0x3F, 0x1F, 0x0F, 0x07, 0x07, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x60, 0x90, 0x90, 0x90, 0x60, 0x00,
0x00, 0x40, 0x00, 0xC0, 0x30, 0x10, 0x10, 0x30, 0xC0, 0x00,
0x80, 0xF0, 0x90, 0x90, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xF0, 0x00, 0x00,
0x00, 0xE0, 0x10, 0x10, 0x10, 0xE0, 0x00, 0x00, 0x40, 0x00,
0x40, 0x20, 0x10, 0x10, 0xB0, 0x40, 0x00, 0x60, 0x10, 0x10,
0x30, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x60, 0x90, 0x90, 0xF0, 0x60, 0x00, 0x00, 0x40,
0x00, 0xE0, 0x30, 0x10, 0x10, 0x60, 0x80, 0x00, 0xC0, 0xF0,
0x90, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x07, 0x08, 0x08, 0x08, 0x07, 0x00, 0x00, 0x08,
0x00, 0x03, 0x0C, 0x08, 0x08, 0x0C, 0x03, 0x00, 0x0C, 0x08,
0x08, 0x0C, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x07,
0x08, 0x08, 0x08, 0x07, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08,
0x0C, 0x0B, 0x09, 0x08, 0x00, 0x06, 0x08, 0x08, 0x0C, 0x07,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x05, 0x08, 0x08, 0x08, 0x07, 0x00, 0x00, 0x08, 0x00, 0x07,
0x0C, 0x08, 0x08, 0x06, 0x01, 0x04, 0x0C, 0x08, 0x08, 0x0D,
0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00
};*/

static volatile uint8_t* m_done_flag;         /**< Bandera que indica que la pantalla fue actualizada. */
static uint32_t m_buffer_index;
static uint8_t m_show_req;
static uint8_t m_process_begin;
static uint8_t m_display_off;
static bool rotate_screen;
static volatile uint8_t i2c_tx_flag;

static void ssd1306_command(uint8_t command) {
    i2c_tx_flag = 0;                              // Reset flag
    i2c_write(0x80);                              // Set OLED command mode
    while(!i2c_tx_flag){};                        // Wait...
    i2c_tx_flag = 0;                              // Reset flag
    i2c_write(command);                           // Send command
    while(!i2c_tx_flag){};                        // Wait...
    i2c_end_transmission();                       // End I2C communication
}

static void ssd1306_data(uint8_t data) {
    i2c_tx_flag = 0;                              // Reset flag
    i2c_write(0x40);                              // Set OLED data mode
    while(!i2c_tx_flag){};                        // Wait...
    i2c_tx_flag = 0;                              // Reset flag
    i2c_write(data);                              // Send data
    while(!i2c_tx_flag){};                        // Wait...
    i2c_end_transmission();                       // End I2C communication
}

void display_done_set_flag(volatile uint8_t* main_done_flag)
{
    m_done_flag = main_done_flag;
}

void display_set_rotation(bool rotation)
{
    rotate_screen = rotation;
}

void display_init(void) {
    //NRF_LOG_INFO("Module init.\n");

    m_display_off = 0;
    m_buffer_index = 0;
    m_show_req = 0;

    // I2C Init
    i2c_tx_set_flag(&i2c_tx_flag);                  // Initialize I2C
    i2c_tx_flag = 0;
    i2c_init();
    i2c_begin_transmission(SSD1306_I2C_ADDRESS);    // Begin I2C communication

    // Init sequence
    ssd1306_command(SSD1306_DISPLAYOFF);            // 0xAE
    ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);    // 0xD5
    ssd1306_command(0x80);                          // the suggested ratio 0x80

    ssd1306_command(SSD1306_SETMULTIPLEX);          // 0xA8
    ssd1306_command(SSD1306_LCDHEIGHT - 1);

    ssd1306_command(SSD1306_SETDISPLAYOFFSET);      // 0xD3
    ssd1306_command(0x0);                           // no offset
    ssd1306_command(SSD1306_SETSTARTLINE | 0x0);    // line #0
    ssd1306_command(SSD1306_CHARGEPUMP);            // 0x8D
    ssd1306_command(0x14);
    ssd1306_command(SSD1306_MEMORYMODE);            // 0x20
    ssd1306_command(0x00);                          // 0x0 act like ks0108
    ssd1306_command(SSD1306_SEGREMAP | 0x1);
    ssd1306_command(SSD1306_COMSCANDEC);

    ssd1306_command(SSD1306_SETCOMPINS);            // 0xDA
    ssd1306_command(0x02);
    ssd1306_command(SSD1306_SETCONTRAST);
    ssd1306_command(0xFF);

    ssd1306_command(SSD1306_SETPRECHARGE);          // 0xd9
    ssd1306_command(0xF1);
    ssd1306_command(SSD1306_SETVCOMDETECT);         // 0xDB
    ssd1306_command(0x40);

    ssd1306_command(SSD1306_DISPLAYALLON_RESUME);   // 0xA4
    ssd1306_command(SSD1306_NORMALDISPLAY);         // 0xA6

    ssd1306_command(SSD1306_DEACTIVATE_SCROLL);

    ssd1306_command(SSD1306_DISPLAYON);

    //NRF_LOG_INFO("Module init done.\n");
    //NRF_LOG_FLUSH();
}

void display_off(void) {
    if (!m_display_off) {
      m_display_off = 1;
      ssd1306_command(SSD1306_DISPLAYOFF);
    }
}

void display_on(void) {
    if (m_display_off) {
      m_display_off = 0;
      ssd1306_command(SSD1306_DISPLAYON);
    }
}

void display_process(void) {
    if (!*m_done_flag) {
        if (m_process_begin) {
            m_process_begin = 0;
            m_show_req = 0;

            ssd1306_command(SSD1306_COLUMNADDR);
            ssd1306_command(0);                         // Column start address (0 = reset)
            ssd1306_command(SSD1306_LCDWIDTH-1);        // Column end address (127 = reset)

            ssd1306_command(SSD1306_PAGEADDR);
            ssd1306_command(0);                         // Page start address (0 = reset)
            ssd1306_command(3);                         // Page end address

            m_buffer_index = 0;
        }
        if (m_buffer_index < (SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8)) {
            uint8_t value;
            value = buffer[(SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8) - 1 - m_buffer_index];
            uint8_t flip;
            flip = 0;
            for (uint8_t i = 0; i < 8; i++){
              if (value & (0x01 << i)){
                flip |= (0x01 << (7 - i));
              }
            }
            if (rotate_screen)
            {
              ssd1306_data(flip);
            } else {
              ssd1306_data(value);
            }
            m_buffer_index++;
        } else {
            if (m_show_req) {
                m_process_begin = 1;
            } else {
                *m_done_flag = 1;
            }
        }
    } else {
        if (m_show_req) {
            *m_done_flag = 0;
        }
    }
}

void display_show(void) {
    m_show_req = 1;
}

void display_clear(void) {
    for (uint32_t i=0; i<(SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8); i++) {
        buffer[i] = 0;
    }
}

void display_draw_pixel(int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (x >= SSD1306_LCDWIDTH) || (y < 0) || (y >= SSD1306_LCDHEIGHT))
        return;

    switch (color) {
        case WHITE:   buffer[x+ (y/8)*SSD1306_LCDWIDTH] |=  (1 << (y&7)); break;
        case BLACK:   buffer[x+ (y/8)*SSD1306_LCDWIDTH] &= ~(1 << (y&7)); break;
        case INVERSE: buffer[x+ (y/8)*SSD1306_LCDWIDTH] ^=  (1 << (y&7)); break;
    }
}

static const uint8_t* m_font;      // Current font.
static uint8_t m_font_offset = 2;  // Font bytes for meta data.
static uint8_t m_font_width;       // Font witdth.
static uint8_t m_font_height;      // Font height.

void display_set_font(const uint8_t* font)
{
    m_font = font;
    m_font_width = m_font[0];
    m_font_height = m_font[2];
}

void display_put_char(char ch, uint8_t x, uint8_t y)
{
    if (!m_font) return;
    if (x > SSD1306_LCDWIDTH - m_font_width) return;
    if (y > SSD1306_LCDHEIGHT / 8 - m_font_height) return;
    // Ignore non-printable ASCII characters. This can be modified for
    // multilingual font.
    if(ch < 32 || ch > 127)
    {
        ch = ' ';
    }
    uint8_t i;
    for(i=0;i<m_font_width;i++)
    {
       // Font array starts at 0, ASCII starts at 32
       buffer[x + SSD1306_LCDWIDTH * y + i] = m_font[(ch-32) * m_font_width + m_font_offset + i];
    }
}

void display_put_buff(char * s, uint8_t len, uint8_t x, uint8_t y)
{
    uint8_t i;
    for (i=0;i<len;i++){
        display_put_char(s[i], x + m_font_width * i, y);
    }
}

void display_put_string(char * s, uint8_t x, uint8_t y)
{
    display_put_buff(s, strlen(s), x, y);
}
