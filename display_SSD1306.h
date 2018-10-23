#include <stdint.h>
#include <stdbool.h>

#define BLACK 0
#define WHITE 1
#define INVERSE 2

void display_done_set_flag(volatile uint8_t* main_done_flag);

void display_set_rotation(bool rotation);

void display_init(void);

void display_off(void);

void display_on(void);

void display_process(void);

void display_show(void);

void display_clear(void);

void display_draw_pixel(int16_t x, int16_t y, uint16_t color);

void display_set_font(const uint8_t* font);

void display_put_char(char ch, uint8_t x, uint8_t y);

void display_put_buff(char * s, uint8_t len, uint8_t x, uint8_t y);

void display_put_string(char * s, uint8_t x, uint8_t y);
