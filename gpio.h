#include <stdint.h>

void gpio_init(void);

void gpio_button_set_flag(volatile uint8_t* main_button_flag);

void gpio_process(void);
