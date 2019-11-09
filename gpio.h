#include <stdint.h>
#include <stdbool.h>

#define SW_PIN 30

void gpio_init(void);

void gpio_button_set_flag(volatile uint8_t* main_button_flag);

void gpio_long_button_set_flag(volatile uint8_t* main_long_button_flag);

void gpio_long_long_button_set_flag(volatile uint8_t* main_long_long_button_flag);

void gpio_double_button_set_flag(volatile uint8_t* main_double_button_flag);

void gpio_process(void);
