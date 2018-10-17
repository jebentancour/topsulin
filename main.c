//#include <stdio.h>

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

#define IDLE_S          60
#define IDLE_TICKS      (IDLE_S * 1000)/CLOCK_TICK_MS

volatile uint8_t display_done_flag;
volatile uint8_t clock_tick_flag;
volatile uint8_t button_flag;

uint16_t idle_timer;

int main(void)
{
    NRF_LOG_INIT(clock_get_timestamp);
    NRF_LOG_INFO("Init.\n");
    NRF_LOG_FLUSH();

    ble_services_init();

    clock_tick_flag = 0;
    clock_tick_set_flag(&clock_tick_flag);
    clock_init();

    idle_timer = 0;

    button_flag = 0;
    gpio_button_set_flag(&button_flag);
    gpio_init();

    nrf_delay_ms(100);
    display_done_set_flag(&display_done_flag);
    display_set_font(font8x8);
    display_init();

    display_clear();
    uint8_t topsulin[] = "TOPsulin";
    display_put_string(topsulin, sizeof(topsulin) - 1, 30, 1);
    uint8_t take_control[] = "Take control";
    display_put_string(take_control, sizeof(take_control) - 1, 20, 2);

    display_show();

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
            advertising_start();
            read_glucose_measurement();
            idle_timer = 0;
        }

        if((idle_timer >= IDLE_TICKS)&&(!clock_tick_flag)&&(display_done_flag)&&(!button_flag)) {
            display_off();
            sd_app_evt_wait();
            idle_timer = IDLE_TICKS;
        }
    }
}
