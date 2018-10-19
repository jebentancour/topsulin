#include <stdio.h>
#include <stdbool.h>

#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "softdevice_handler.h"
#include "app_util_platform.h"

#define NRF_LOG_MODULE_NAME "MAIN"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "display_SSD1306.h"
#include "font8x8.h"
#include "clock.h"
#include "gpio.h"
#include "ble_services.h"
#include "encoder.h"

#define IDLE_S          60
#define IDLE_TICKS      (IDLE_S * 1000)/CLOCK_TICK_MS

volatile uint8_t display_done_flag;
volatile uint8_t clock_tick_flag;
volatile uint8_t button_flag;
volatile uint8_t encoder_flag;

uint16_t idle_timer;

int main(void)
{
    nrf_delay_ms(200);

    NRF_LOG_INIT(clock_get_timestamp);
    NRF_LOG_INFO("Main init.\n");
    NRF_LOG_FLUSH();

    ble_services_init();

    idle_timer = 0;
    clock_tick_flag = 0;
    clock_tick_set_flag(&clock_tick_flag);
    clock_init();

    button_flag = 0;
    gpio_button_set_flag(&button_flag);
    gpio_init();

    display_done_flag = 0;
    display_done_set_flag(&display_done_flag);
    display_set_rotation(true);
    display_set_font(font8x8);
    display_init();

    display_clear();

    uint8_t topsulin[] = "TOPsulin";
    display_put_string(topsulin, sizeof(topsulin) - 1, 35, 0);
    uint8_t line[] = "____________";
    display_put_string(line, sizeof(line) - 1, 20, 1);
    uint8_t take_control[] = "Take control";
    display_put_string(take_control, sizeof(take_control) - 1, 20, 2);

    display_show();

    encoder_flag = 0;
    encoder_set_flag(&encoder_flag);
    encoder_set_direction(true);
    encoder_init();

    sd_power_mode_set(NRF_POWER_MODE_LOWPWR);

    while (true)
    {
        if (!display_done_flag) {
            display_process();
        }

        if(clock_tick_flag) {
            clock_tick_flag = 0;
            idle_timer++;
        }

        gpio_read();

        if(button_flag) {
            button_flag = 0;
            display_on();
            encoder_enable();
            advertising_start();
            read_glucose_measurement();
            idle_timer = 0;
        }

        if(encoder_flag) {
            encoder_flag = 0;
            NRF_LOG_INFO("Encoder position %d\n", encoder_get_position());
            NRF_LOG_FLUSH();
            idle_timer = 0;
        }

        if((idle_timer >= IDLE_TICKS)&&(!clock_tick_flag)&&(display_done_flag)&&(!button_flag)) {
            display_off();
            encoder_disable();
            sd_app_evt_wait();
            idle_timer = IDLE_TICKS;
        }
    }
}
