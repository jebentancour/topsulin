#include <stdio.h>
#include <stdbool.h>

#include "nrf_delay.h"

#define NRF_LOG_MODULE_NAME "MAIN"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "FEPD_2in13.h"
#include "GUI_Paint.h"
#include "GUI_Cache.h"
#include "ImageData.h"
#include "clock.h"
#include "gpio.h"
#include "ble_services.h"
#include "encoder.h"
#include "state.h"
#include "batt.h"
#include "config_manager.h"

#define IDLE_S          90
#define IDLE_TICKS      (IDLE_S * 1000)/CLOCK_TICK_MS

volatile uint8_t clock_tick_flag;
volatile uint8_t button_flag;
volatile uint8_t long_button_flag;
volatile uint8_t long_long_button_flag;
volatile uint8_t double_button_flag;
volatile uint8_t encoder_flag;
volatile uint8_t batt_flag;
volatile uint8_t display_flag;
volatile uint8_t idle_flag;

uint8_t wake_up;
uint16_t idle_timer;

int main(void){

    NRF_LOG_INIT(clock_get_timestamp);

    struct tm m_tm;
    m_tm.tm_year = 2019 - 1900;
    m_tm.tm_mon = 1 - 1;
    m_tm.tm_mday = 1;
    m_tm.tm_hour = 12;
    m_tm.tm_min = 00;
    m_tm.tm_sec = 0;
    clock_set_time(&m_tm);

    display_flag = 0;
    state_display_set_flag(&display_flag);
    idle_flag = 0;
    state_idle_set_flag(&idle_flag);
    state_init();

    config_manager_init();
    //config_manager_print();

    sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
    ble_services_init();

    clock_tick_flag = 0;
    clock_tick_set_flag(&clock_tick_flag);
    clock_init();

    uint32_t last = 0;
    uint32_t now = 0;

    UWORD color;
    if (config_manager_get_flags() & CONFIG_COLOR_FLAG){
      color = WHITE;
    } else {
      color = BLACK;
    }

    DEV_ModuleInit();
    EPD_Init(FULL_UPDATE);
    Paint_NewImage(ImageBuff, EPD_WIDTH, EPD_HEIGHT, ROTATE_90, color);
    Paint_DrawBitMap(gImage_initial);
    EPD_DisplayWindows(ImageBuff, 0, 0, EPD_WIDTH, EPD_HEIGHT);
    EPD_TurnOnDisplay();
    EPD_Sleep();
    DEV_ModuleUninit();

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
    long_long_button_flag = 0;
    gpio_long_long_button_set_flag(&long_long_button_flag);
    double_button_flag = 0;
    gpio_double_button_set_flag(&double_button_flag);
    gpio_init();

    encoder_flag = 0;
    encoder_set_flag(&encoder_flag);
    encoder_set_direction((config_manager_get_flags() & CONFIG_FLIP_FLAG) == 0);
    encoder_init();

    wake_up = 1;
    idle_timer = IDLE_TICKS;
    last = clock_get_timestamp();

    NRF_LOG_FLUSH();

    while (true){

        gpio_process(); // GPIO polling

        if(button_flag){
            button_flag = 0;
            if (!wake_up) {
                NRF_LOG_INFO("Wake up from click!\r\n");
                NRF_LOG_FLUSH();
                encoder_enable();
                batt_init();
                wake_up = 1;
            }
            state_on_event(button_pressed);
            idle_timer = 0;
        }

        if(double_button_flag){
            double_button_flag = 0;
            if (wake_up) state_on_event(double_button_pressed);
            idle_timer = 0;
        }

        if(long_button_flag){
            long_button_flag = 0;
            if (wake_up) state_on_event(long_button_pressed);
            idle_timer = 0;
        }

        if(long_long_button_flag){
            long_long_button_flag = 0;
            advertising_toggle();
        }

        if(encoder_flag){
            encoder_flag = 0;
            if (wake_up) state_on_event(encoder_update);
            idle_timer = 0;
        }

        if(clock_tick_flag){
            clock_tick_flag = 0;
            if (wake_up){
              // Update BLE Time and Voltage Characteristics every second
              now = clock_get_timestamp();
              if (now - last >= 1000){
                last = now;
                time_ble_update();
                if(!batt_flag){
                  batt_sample();
                }
              }
              idle_timer++;
            }
        }

        if(batt_flag){
          batt_flag = 0;
          if (wake_up) batt_ble_update((uint16_t)batt_get());
        }

        if(idle_flag){
          idle_flag = 0;
          idle_timer = IDLE_TICKS;
        }

        if(idle_timer >= IDLE_TICKS){
            // prepare to sleep
            if (wake_up){
              NRF_LOG_INFO("Idle timeout\r\n");
              NRF_LOG_FLUSH();
              state_sleep();
              encoder_disable();
              batt_disable();
              wake_up = 0;
            }
            idle_timer = IDLE_TICKS;
        }

        if(display_flag){
          display_flag = 0;
          state_process_display();
        }

        // If it is nothing to do...
        if(!button_flag && !double_button_flag && !long_button_flag && !long_long_button_flag && !encoder_flag && !clock_tick_flag && !batt_flag && !idle_flag && !display_flag){
            // sleep and wait for event...
            sd_app_evt_wait();
        }
    }
}
