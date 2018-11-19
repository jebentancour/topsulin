#include <stdio.h>
#include <stdbool.h>

#include "nrf_delay.h"

#define NRF_LOG_MODULE_NAME "MAIN"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

//#include "display_SSD1306.h"
#include "font8x8.h"
#include "clock.h"
#include "gpio.h"
#include "ble_services.h"
#include "encoder.h"
//#include "state.h"
#include "batt.h"

#define IDLE_S          10
#define IDLE_TICKS      (IDLE_S * 1000)/CLOCK_TICK_MS

//volatile uint8_t display_done_flag;
volatile uint8_t clock_tick_flag;
volatile uint8_t button_flag;
volatile uint8_t encoder_flag;
volatile uint8_t batt_flag;

uint8_t wake_up;
uint16_t idle_timer;

int main(void)
{
    //nrf_delay_ms(1000);

    NRF_LOG_INIT(clock_get_timestamp);
    NRF_LOG_INFO("Setup\n");
    NRF_LOG_FLUSH();

    ble_services_init();

    idle_timer = 0;
    clock_tick_flag = 0;
    clock_tick_set_flag(&clock_tick_flag);
    clock_init();

    button_flag = 0;
    gpio_button_set_flag(&button_flag);
    gpio_init();

    //state_init();

    //display_done_flag = 0;
    //display_done_set_flag(&display_done_flag);
    //display_set_rotation(true);
    //display_set_font(font8x8);
    //display_init();

    encoder_flag = 0;
    encoder_set_flag(&encoder_flag);
    encoder_set_direction(true);
    //encoder_init();

    batt_flag = 0;
    batt_set_flag(&batt_flag);
    batt_init();
    batt_sample();

    wake_up = 1;
    sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
    
    gpio_set_led(true);

    while (true)
    {
        //display_process();

        gpio_process();

        if(button_flag) {
            button_flag = 0;
            if (!wake_up) {
                gpio_set_led(true);
                //display_on();
                encoder_enable();
                advertising_start();
                NRF_LOG_INFO("Wake up!\r\n");
                NRF_LOG_FLUSH();
                wake_up = 1;
            }
            //state_on_event(button_pressed);
            idle_timer = 0;
        }

        if(encoder_flag) {
            encoder_flag = 0;
            //state_on_event(encoder_update);
            idle_timer = 0;
        }

        if(clock_tick_flag) {
            clock_tick_flag = 0;
            //state_on_event(time_update);
            idle_timer++;
        }

        if((idle_timer >= IDLE_TICKS)&&(!clock_tick_flag)/*&&(display_done_flag)*/&&(!button_flag)&&(!encoder_flag)) {
            gpio_set_led(false);
            wake_up = 0;
            encoder_disable();
            //state_off();
            //display_off();
            sd_app_evt_wait();
            idle_timer = IDLE_TICKS;
        }
    }
}
