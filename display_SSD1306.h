#include <stdint.h>

#define BLACK 0
#define WHITE 1
#define INVERSE 2

void display_done_set_flag(volatile uint8_t* main_done_flag);

void display_init(void);

void display_off(void);

void display_on(void);

void display_process(void);

void display_show(void);

void display_clear(void);

void display_draw_pixel(int16_t x, int16_t y, uint16_t color);
