#include "gpio.h"

#include "nrf_delay.h"
#include "nrf_gpio.h"

#define SW_PIN 9

static volatile uint8_t* m_gpio_button_flag;

void gpio_init() {
    nrf_gpio_cfg_input(SW_PIN,NRF_GPIO_PIN_PULLUP);
}

void gpio_button_set_flag(volatile uint8_t* main_button_flag)
{
    m_gpio_button_flag = main_button_flag;
}

void gpio_process(void) {
    static uint8_t old_in;
    static uint8_t new_in;
    new_in = nrf_gpio_pin_read(SW_PIN);
    if (old_in && !new_in) {
      nrf_delay_ms(1);
      if (!nrf_gpio_pin_read(SW_PIN)) {
        *m_gpio_button_flag = 1;
      }
    }
    old_in = new_in;
};
