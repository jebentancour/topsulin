#include "gpio.h"

#define NRF_LOG_MODULE_NAME "GPIO"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "clock.h"

#define LONG_CLICK_MS 1500

static volatile uint8_t* m_gpio_button_flag;
static volatile uint8_t* m_gpio_long_button_flag;

static uint32_t pulse_start = 0;
static uint32_t pulse_stop = 0;
static uint32_t now = 0;
static int      pulse_len = 0;
static uint8_t  long_timeout = 0;
static uint8_t  old_in = 1;
static uint8_t  new_in = 1;

void gpio_init() {
    nrf_gpio_cfg_input(SW_PIN,NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_output(LED_PIN);
}

void gpio_button_set_flag(volatile uint8_t* main_button_flag)
{
    m_gpio_button_flag = main_button_flag;
}

void gpio_long_button_set_flag(volatile uint8_t* main_long_button_flag)
{
    m_gpio_long_button_flag = main_long_button_flag;
}

void gpio_process(void) {

    new_in = nrf_gpio_pin_read(SW_PIN);

    now = clock_get_timestamp();

    if (old_in != new_in){

      nrf_delay_ms(1);

      if (!new_in){
        // Falling edge, button pressed
        //NRF_LOG_INFO("fall\r\n");
        //NRF_LOG_FLUSH();

        pulse_start = now;
      } else {
        // Rising edge, button released
        //NRF_LOG_INFO("rise\r\n");
        //NRF_LOG_FLUSH();

        pulse_stop = now;
        pulse_len = pulse_stop - pulse_start;

        if (pulse_len < LONG_CLICK_MS){
          *m_gpio_button_flag = 1;
          NRF_LOG_INFO("button_flag\r\n");
          NRF_LOG_FLUSH();
        }

        long_timeout = 0;
      }
    }

    if (!new_in && !long_timeout){
      pulse_len = now - pulse_start;
      if (pulse_len > LONG_CLICK_MS){
        *m_gpio_long_button_flag = 1;
        long_timeout = 1;
        NRF_LOG_INFO("long_button_flag\r\n");
        NRF_LOG_FLUSH();
      }
    }

    old_in = new_in;
};

void gpio_set_led(bool state) {
    if (state){
        nrf_gpio_pin_set(LED_PIN);
    } else {
        nrf_gpio_pin_clear(LED_PIN);
    }
}

void gpio_led_toggle(void) {
    nrf_gpio_pin_toggle(LED_PIN);
}
