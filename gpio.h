#include <stdint.h>
#include <stdbool.h>

#define SW_PIN 30
#define LED_PIN 10

void gpio_init(void);

void gpio_button_set_flag(volatile uint8_t* main_button_flag);

void gpio_long_button_set_flag(volatile uint8_t* main_long_button_flag);

void gpio_process(void);

void gpio_set_led(bool state);

void gpio_led_toggle(void);
