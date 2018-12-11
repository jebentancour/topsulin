#include <stdio.h>
#include <stdbool.h>

#include "nrf_delay.h"

#define NRF_LOG_MODULE_NAME "MAIN"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

//#include "display_SSD1306.h"
//#include "font8x8.h"
#include "clock.h"
#include "gpio.h"
#include "ble_services.h"
#include "encoder.h"
//#include "state.h"
#include "batt.h"
#include "config_manager.h"

#define IDLE_S          30
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

    config_manager_init();
    config_manager_print();

    ble_services_init();

    idle_timer = 0;
    clock_tick_flag = 0;
    clock_tick_set_flag(&clock_tick_flag);
    clock_init();

    uint32_t last = 0;
    uint32_t now = 0;

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

    NRF_LOG_FLUSH();
    while (true)
    {
        //display_process();

        gpio_process(); // GPIO polling

        if(button_flag) {
            button_flag = 0;
            if (!wake_up) {
                gpio_set_led(true);
                //display_on();
                encoder_enable();
                advertising_start();
                NRF_LOG_INFO("Wake up!\r\n");
                NRF_LOG_FLUSH();
                clock_print();
                wake_up = 1;
            } else {
              // simulate the reading of a glucose measurement
              struct tm t;
              clock_get_time(&t);

              char time_buffer[80];
              strftime(time_buffer, sizeof(time_buffer), "%x %X", &t);
              NRF_LOG_INFO("New reading %s\n", (uint32_t)time_buffer);
              NRF_LOG_FLUSH();

              ble_gls_rec_t rec;

              rec.meas.flags = BLE_GLS_MEAS_FLAG_TIME_OFFSET |
                               BLE_GLS_MEAS_FLAG_CONC_TYPE_LOC |
                               BLE_GLS_MEAS_FLAG_CONTEXT_INFO |
                               BLE_GLS_MEAS_FLAG_SENSOR_STATUS |
                               BLE_GLS_MEAS_FLAG_UNITS_MOL_L;
              rec.meas.base_time.year                 = t.tm_year + 1900;
              rec.meas.base_time.month                = t.tm_mon + 1;
              rec.meas.base_time.day                  = t.tm_mday;
              rec.meas.base_time.hours                = t.tm_hour;
              rec.meas.base_time.minutes              = t.tm_min;
              rec.meas.base_time.seconds              = t.tm_sec;
              rec.meas.glucose_concentration.exponent = -3;
              rec.meas.glucose_concentration.mantissa = 105;
              rec.meas.time_offset                    = 0;
              rec.meas.type                           = BLE_GLS_MEAS_TYPE_UNDET_BLOOD;
              rec.meas.sample_location                = BLE_GLS_MEAS_LOC_NOT_AVAIL;
              rec.meas.sensor_status_annunciation     = 0;
              //rec.meas.sensor_status_annunciation     = BLE_GLS_MEAS_STATUS_BATT_LOW;
              rec.context.flags = BLE_GLS_CONTEXT_FLAG_CARB |
                                  BLE_GLS_CONTEXT_FLAG_MED |
                                  BLE_GLS_CONTEXT_FLAG_MED_L;
              rec.context.carbohydrate_id = BLE_GLS_CONTEXT_CARB_LUNCH;
              rec.context.carbohydrate.exponent = 0;
              rec.context.carbohydrate.mantissa = 259;
              rec.context.medication_id         = BLE_GLS_CONTEXT_MED_RAPID;
              rec.context.medication.exponent   = 3;
              rec.context.medication.mantissa   = 150;

              add_glucose_measurement(rec);
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
            if (wake_up){
              // Update BLE Time Characteristic every second
              now = clock_get_timestamp();
              if (now - last >= 1000){
                //clock_print();
                time_ble_update();
                last = now;
              }
            }
            idle_timer++;
        }

        // If it is nothing to do...
        if((idle_timer >= IDLE_TICKS)&&(!clock_tick_flag)/*&&(display_done_flag)*/&&(!button_flag)&&(!encoder_flag)) {
            // prepare to sleep
            gpio_set_led(false);
            wake_up = 0;
            encoder_disable();
            //state_off();
            //display_off();

             // sleep and wait for event...
            sd_app_evt_wait();

            // wake up!
            idle_timer = IDLE_TICKS;
        }
    }
}
