#include <stdio.h>
#include <stdbool.h>

#include "nrf_delay.h"

#define NRF_LOG_MODULE_NAME "MAIN"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "FEPD_2in13.h"
#include "GUI_Paint.h"
#include "ImageData.h"
#include "clock.h"
#include "gpio.h"
#include "ble_services.h"
#include "encoder.h"
#include "state.h"
#include "batt.h"
#include "config_manager.h"

#define IDLE_S          60
#define IDLE_TICKS      (IDLE_S * 1000)/CLOCK_TICK_MS

volatile uint8_t clock_tick_flag;
volatile uint8_t button_flag;
volatile uint8_t long_button_flag;
volatile uint8_t double_button_flag;
volatile uint8_t encoder_flag;
volatile uint8_t batt_flag;

uint8_t wake_up;
uint16_t idle_timer;

int main(void){

    NRF_LOG_INIT(clock_get_timestamp);
    NRF_LOG_INFO("-------------------- MAIN --------------------\n");
    NRF_LOG_FLUSH();

    struct tm m_tm;
    m_tm.tm_year = 2019 - 1900;
    m_tm.tm_mon = 1 - 1;
    m_tm.tm_mday = 1;
    m_tm.tm_hour = 12;
    m_tm.tm_min = 0;
    m_tm.tm_sec = 0;
    clock_set_time(&m_tm);

    state_init();

    config_manager_init();
    //config_manager_print();

    sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
    ble_services_init();
    //peer_manager_erase_bonds();

    clock_tick_flag = 0;
    clock_tick_set_flag(&clock_tick_flag);
    clock_init();

    uint32_t last = 0;
    uint32_t now = 0;

    DEV_ModuleInit();
    EPD_Init();
    if(config_manager_get_flags() & CONFIG_COLOR_FLAG){
      if (config_manager_get_flags() & CONFIG_FLIP_FLAG){
        GUI_NewImage(EPD_WIDTH, EPD_HEIGHT, IMAGE_ROTATE_180, IMAGE_COLOR_POSITIVE);
      } else {
        GUI_NewImage(EPD_WIDTH, EPD_HEIGHT, IMAGE_ROTATE_0, IMAGE_COLOR_POSITIVE);
      }
    } else {
      if (config_manager_get_flags() & CONFIG_FLIP_FLAG){
        GUI_NewImage(EPD_WIDTH, EPD_HEIGHT, IMAGE_ROTATE_180, IMAGE_COLOR_INVERTED);
      } else {
        GUI_NewImage(EPD_WIDTH, EPD_HEIGHT, IMAGE_ROTATE_0, IMAGE_COLOR_INVERTED);
      }
    }
    GUI_DrawBitMap(gImage_IMAGE_0);
    //EPD_DisplayFull();

    batt_flag = 0;
    batt_set_flag(&batt_flag);
    batt_init();
    batt_sample();

    while(!batt_flag){
        // wait...
    }
    batt_flag = 0;
    uint32_t voltage;
    voltage = batt_get();
    NRF_LOG_INFO("VCC = %d.%d V\n", voltage / 1000, voltage % 1000);

    button_flag = 0;
    gpio_button_set_flag(&button_flag);
    long_button_flag = 0;
    gpio_long_button_set_flag(&long_button_flag);
    double_button_flag = 0;
    gpio_double_button_set_flag(&double_button_flag);
    gpio_init();

    encoder_flag = 0;
    encoder_set_flag(&encoder_flag);
    encoder_set_direction((config_manager_get_flags() & CONFIG_FLIP_FLAG) == 0);
    encoder_init();

    NRF_LOG_FLUSH();

    wake_up = 1;
    idle_timer = IDLE_TICKS;
    last = clock_get_timestamp();

    while (true)
    {
        gpio_process(); // GPIO polling

        if(button_flag) {
            button_flag = 0;
            if (!wake_up) {
                NRF_LOG_INFO("Wake up!\r\n");
                NRF_LOG_FLUSH();
                clock_print();
                EPD_Init();
                advertising_start();
                encoder_enable();
                wake_up = 1;
            }
            state_on_event(button_pressed);
            idle_timer = 0;
        }

        if(long_button_flag) {
            long_button_flag = 0;
            state_on_event(long_button_pressed);
            idle_timer = 0;
        }

        if(double_button_flag) {
            double_button_flag = 0;
            state_on_event(double_button_pressed);
            idle_timer = 0;
        }

        if(encoder_flag) {
            encoder_flag = 0;
            state_on_event(encoder_update);
            idle_timer = 0;
        }

        if(clock_tick_flag) {
            clock_tick_flag = 0;
            if (wake_up){
              // Update BLE Time and Voltage Characteristics every second
              now = clock_get_timestamp();
              if (now - last >= 1000){
                last = now;
                state_on_event(time_update);
                time_ble_update();
                if(!batt_flag){
                  batt_sample();
                }
                while(!batt_flag){
                    // wait...
                }
                batt_flag = 0;
                batt_ble_update((uint16_t)batt_get());
              }
            }
            idle_timer++;
        }

        // If it is nothing to do...
        if((idle_timer >= IDLE_TICKS)&&(!clock_tick_flag)&&(!button_flag)&&(!long_button_flag)&&(!double_button_flag)&&(!encoder_flag)&&(!batt_flag)) {
            // prepare to sleep
            if (wake_up){
              state_sleep();
              advertising_stop();
              EPD_Sleep();
              encoder_disable();
            }
            wake_up = 0;

            // sleep and wait for event...
            sd_app_evt_wait();

            // wake up!
            idle_timer = IDLE_TICKS;
        }
    }
}
