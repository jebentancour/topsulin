#include <stdint.h>
#include <stdbool.h>

void gpio_init(void);

void gpio_button_set_flag(volatile uint8_t* main_button_flag);

void gpio_process(void);

void gpio_set_led(bool state);

void gpio_led_toggle(void);
