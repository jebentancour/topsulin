#include "state.h"

#include <stdio.h>

#define NRF_LOG_MODULE_NAME "STATE"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "ble_gls_db.h"

#include "encoder.h"
#include "FEPD_2in13.h"
#include "GUI_Paint.h"
#include "GUI_Cache.h"
#include "ImageData.h"
#include "ble_services.h"
#include "clock.h"
#include "batt.h"
#include "config_manager.h"

#define LOW_VOLT  2300

#define MARGEN    6
#define ANCHO     250
#define ALTO      122

typedef enum {
    initial,
    sleep,
    input_cho,
    input_glu,
    input_ins
} internal_state_t;

typedef struct {
    int32_t glu;
    struct tm glu_time;
    int32_t cho;
    struct tm cho_time;
    int32_t ins;
    struct tm ins_time;
} topsulin_meas_t;

static bool new_ins;
static bool new_cho;
static bool new_glu;

static int cho_correction;
static int glu_correction;

static topsulin_meas_t        m_topsulin_meas;
static topsulin_meas_t        m_prev_topsulin_meas;

static internal_state_t       m_state;

static char                   buffer[16];
static uint8_t                len;

static uint8_t                full_refresh;
static uint8_t                quick_refresh;

static uint32_t               voltage;
static uint8_t                bt_state;

static bool                   was_full;
static bool                   was_partial;

void state_init(){
  m_state = initial;

  memset(&m_prev_topsulin_meas, 0, sizeof(m_prev_topsulin_meas));
  m_prev_topsulin_meas.glu = 0;
  m_prev_topsulin_meas.cho = 0;
  m_prev_topsulin_meas.ins = 0;
  struct tm t;
  clock_get_time(&t);
  m_prev_topsulin_meas.glu_time = t;
  m_prev_topsulin_meas.cho_time = t;
  m_prev_topsulin_meas.ins_time = t;

  memset(&m_topsulin_meas, 0, sizeof(m_topsulin_meas));
  m_topsulin_meas.glu = 0;
  m_topsulin_meas.cho = 0;
  m_topsulin_meas.ins = 0;
  m_topsulin_meas.glu_time = t;
  m_topsulin_meas.cho_time = t;
  m_topsulin_meas.ins_time = t;

  voltage = 0;
  bt_state = 0;

  full_refresh = 0;
  quick_refresh = 0;
}

static void state_save_meas(void){
  if (new_glu || new_cho || new_ins){
    struct tm t;
    clock_get_time(&t);

    if (new_glu){
      m_topsulin_meas.glu_time = t;
    }

    if (new_cho){
      m_topsulin_meas.cho_time = t;
    }

    if (new_ins){
      m_topsulin_meas.ins_time = t;
    }

    ble_gls_rec_t rec;

    rec.meas.flags = BLE_GLS_MEAS_FLAG_UNITS_KG_L | BLE_GLS_MEAS_FLAG_SENSOR_STATUS;

    if (new_glu){
      rec.meas.flags |= BLE_GLS_MEAS_FLAG_CONC_TYPE_LOC;
    }

    if (new_cho || new_ins){
      rec.meas.flags |= BLE_GLS_MEAS_FLAG_CONTEXT_INFO;
    }

    rec.meas.base_time.year                 = t.tm_year + 1900;
    rec.meas.base_time.month                = t.tm_mon + 1;
    rec.meas.base_time.day                  = t.tm_mday;
    rec.meas.base_time.hours                = t.tm_hour;
    rec.meas.base_time.minutes              = t.tm_min;
    rec.meas.base_time.seconds              = t.tm_sec;

    if (new_glu){
      rec.meas.glucose_concentration.exponent = -5;
      rec.meas.glucose_concentration.mantissa = m_topsulin_meas.glu;
      rec.meas.type                           = BLE_GLS_MEAS_TYPE_CAP_BLOOD;
      rec.meas.sample_location                = BLE_GLS_MEAS_LOC_NOT_AVAIL;
    }

    rec.meas.sensor_status_annunciation     = 0;
    //rec.meas.sensor_status_annunciation     = BLE_GLS_MEAS_STATUS_BATT_LOW;

    if (new_cho){
      rec.context.flags |= BLE_GLS_CONTEXT_FLAG_CARB;
      if (t.tm_hour <= 4){
          // cena
          rec.context.carbohydrate_id = BLE_GLS_CONTEXT_CARB_DINNER;
      } else {
          if (t.tm_hour <= 11){
              // desayuno
              rec.context.carbohydrate_id = BLE_GLS_CONTEXT_CARB_BREAKFAST;
          } else {
              if (t.tm_hour <= 15){
                  // almuerzo
                  rec.context.carbohydrate_id = BLE_GLS_CONTEXT_CARB_LUNCH;
              } else {
                  if (t.tm_hour <= 19){
                      // merienda
                      rec.context.carbohydrate_id =	BLE_GLS_CONTEXT_CARB_SNACK;
                  } else {
                      // cena
                      rec.context.carbohydrate_id = BLE_GLS_CONTEXT_CARB_DINNER;
                  }
              }
          }
      }
      rec.context.carbohydrate.exponent = 0; // Exponent: Decimal, -3
      rec.context.carbohydrate.mantissa = m_topsulin_meas.cho;
    }

    if (new_ins){
      rec.context.flags |= BLE_GLS_CONTEXT_FLAG_MED;
      rec.context.flags |= BLE_GLS_CONTEXT_FLAG_MED_L;
      rec.context.medication_id         = config_manager_get_insulin_type();
      // Definimos que 1 U de insulina se representa como 10 ml
      rec.context.medication.exponent   = 3; // Exponent: Decimal, -3
      rec.context.medication.mantissa   = m_topsulin_meas.ins;
    }

    add_glucose_measurement(rec);

    if(!new_glu){
        m_topsulin_meas.glu = m_prev_topsulin_meas.glu;
    }

    if(!new_cho || m_topsulin_meas.cho <= 0){
        m_topsulin_meas.cho = m_prev_topsulin_meas.cho;
    }

    if(!new_ins || m_topsulin_meas.ins <= 0){
        m_topsulin_meas.ins = m_prev_topsulin_meas.ins;
    }
  }
}

void state_set_bt_state(uint8_t state){
  if (bt_state == state){
    return;
  }
  bt_state = state;
  if (m_state == initial){
    if (state == 0){
      full_refresh = 1;
    }
    if (state == 1){
      quick_refresh = 1;
    }
    if (state == 2){
      quick_refresh = 1;
    }
  } else {
    quick_refresh = 1;
  }
  state_process_display();
}

void state_update_mem(void){
  quick_refresh = 1;
  state_process_display();
}

void state_process_display(void){

  if (full_refresh){
    if (!was_full){
      EPD_Init(FULL_UPDATE);
      was_full = true;
      was_partial = false;
    }
  }

  if (quick_refresh){
    if (!was_partial){
      EPD_Init(PART_UPDATE);
      was_partial = true;
      was_full = false;
    }
  }

  if ((m_state != initial)&&(quick_refresh|full_refresh)){
    Paint_NewImage(ImageBuff, EPD_WIDTH, EPD_HEIGHT, ROTATE_90, WHITE);
    Paint_Clear(0xff);
    EPD_DisplayWindows(ImageBuff, 0, 0, EPD_WIDTH, EPD_HEIGHT);

    // NAME
    len = config_manager_get_name(buffer);
    Paint_NewImage(ImageBuff, 16, len*11, ROTATE_90, WHITE);
    Paint_Clear(0xff);
    Paint_DrawString_EN(0, 0, buffer, &Font16, WHITE, BLACK);
    EPD_DisplayWindows(ImageBuff, 122-16, 0, 122, len*11);

    // BATTERY
    //voltage = batt_get();
    //len = sprintf(buffer, "%ld.%ld", voltage / 1000, (voltage % 1000) / 100);
    //Paint_NewImage(ImageBuff, 12, 3*7, ROTATE_90, WHITE);
    //Paint_Clear(0xff);
    //Paint_DrawString_EN(0, 0, buffer, &Font12, WHITE, BLACK);
    //EPD_DisplayWindows(ImageBuff, 122-12, 250-16*3-3*7, 122, 250-16*3);

    // MEMORY ICON
    uint16_t num_records = ble_gls_db_num_records_get();
    if(num_records > 0){
      Paint_NewImage(ImageBuff, 16, 16, ROTATE_90, WHITE);
      Paint_DrawBitMap(gImage_icon_mem_16);
      EPD_DisplayWindows(ImageBuff, 122-16, 250-16*3, 122, 250-16*2);
    }

    if (bt_state == 1 || bt_state == 2){
      // BLUETOOTH ON ICON
      Paint_NewImage(ImageBuff, 16, 16, ROTATE_90, WHITE);
      Paint_DrawBitMap(gImage_icon_bt_16);
      EPD_DisplayWindows(ImageBuff, 122-16, 250-16*2, 122, 250-16);
    }

    if (bt_state == 2){
      // BLUETOOTH CONNECTED ICON
      Paint_NewImage(ImageBuff, 16, 16, ROTATE_90, WHITE);
      Paint_DrawBitMap(gImage_icon_dev_16);
      EPD_DisplayWindows(ImageBuff, 122-16, 250-16, 122, 250-16+16);
    }

    if (config_manager_get_flags() & CONFIG_FLIP_FLAG){
      // TODO
    } else {
      // GLU ICON
      Paint_NewImage(ImageBuff, 24, 24, ROTATE_90, BLACK);
      Paint_DrawBitMap(gImage_icon_glu_24);
      EPD_DisplayWindows(ImageBuff, 122-24-24-1, 1+1, 122-24-24+24-1, 1+24+1);
      // CHO ICON
      Paint_NewImage(ImageBuff, 24, 24, ROTATE_90, BLACK);
      Paint_DrawBitMap(gImage_icon_cho_24);
      EPD_DisplayWindows(ImageBuff, 122-24-24-1, 1+82+4+1, 122-24-24+24-1, 1+82+4+24+1);
      // INS ICON
      Paint_NewImage(ImageBuff, 24, 24, ROTATE_90, BLACK);
      Paint_DrawBitMap(gImage_icon_ins_24);
      EPD_DisplayWindows(ImageBuff, 122-24-24-1, 1+82+4+82+4+1, 122-24-24+24-1, 1+82+4+82+4+24+1);
    }

    //GUI_DrawString_EN(LEFT_ICON_H_POS + 32, 72, "mg", &Font12, WHITE, BLACK);
    //GUI_DrawString_EN(LEFT_ICON_H_POS + 32, 60, "dL", &Font12, WHITE, BLACK);

    //len = sprintf(buffer, "%d", config_manager_get_cho_interval());
    //GUI_DrawString_EN(CENTER_ICON_H_POS + 32, 72, buffer, &Font12, WHITE, BLACK);
    //GUI_DrawString_EN(CENTER_ICON_H_POS + 32, 60, "grs", &Font12, WHITE, BLACK);

    //len = sprintf(buffer, "%d.%d", config_manager_get_ins_interval() / 10, config_manager_get_ins_interval() % 10);
    //GUI_DrawString_EN(RIGHT_ICON_H_POS + 32, 72, buffer, &Font12, WHITE, BLACK);
    //GUI_DrawString_EN(RIGHT_ICON_H_POS + 32, 60, "U", &Font12, WHITE, BLACK);

    if (m_state == sleep){

      // GLU NUMBER
      len = sprintf(buffer, "%ld", m_topsulin_meas.glu);
      Paint_NewImage(ImageBuff, 24, len*17, ROTATE_90, WHITE);
      Paint_Clear(0xff);
      Paint_DrawString_EN(0, 0, buffer, &Font24, WHITE, BLACK);
      EPD_DisplayWindows(ImageBuff, 35, 41-(len*17)/2, 35+24, 41+(len*17)/2);

      // GLU TIME
      len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.glu_time);
      Paint_NewImage(ImageBuff, 16, 5*11, ROTATE_90, WHITE);
      Paint_Clear(0xff);
      Paint_DrawString_EN(0, 0, buffer, &Font16, WHITE, BLACK);
      EPD_DisplayWindows(ImageBuff, 10, 41-(len*11)/2, 10+16, 41+(len*11)/2);

      // CHO NUMBER
      len = sprintf(buffer, "%ld", m_topsulin_meas.cho);
      Paint_NewImage(ImageBuff, 24, len*17, ROTATE_90, WHITE);
      Paint_Clear(0xff);
      Paint_DrawString_EN(0, 0, buffer, &Font24, WHITE, BLACK);
      EPD_DisplayWindows(ImageBuff, 35, 125-(len*17)/2, 35+24, 125+(len*17)/2);

      // CHO TIME
      len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.cho_time);
      Paint_NewImage(ImageBuff, 16, 5*11, ROTATE_90, WHITE);
      Paint_Clear(0xff);
      Paint_DrawString_EN(0, 0, buffer, &Font16, WHITE, BLACK);
      EPD_DisplayWindows(ImageBuff, 10, 125-(len*11)/2, 10+16, 125+(len*11)/2);

      // INS NUMBER
      if (config_manager_get_ins_interval() >= 10){
        len = sprintf(buffer, "%ld", m_topsulin_meas.ins / 10);
      } else {
        len = sprintf(buffer, "%ld.%ld", m_topsulin_meas.ins / 10, m_topsulin_meas.ins % 10);
      }
      Paint_NewImage(ImageBuff, 24, len*17, ROTATE_90, WHITE);
      Paint_Clear(0xff);
      Paint_DrawString_EN(0, 0, buffer, &Font24, WHITE, BLACK);
      EPD_DisplayWindows(ImageBuff, 35, 208-(len*17)/2, 35+24, 208+(len*17)/2);

      // INS TIME
      len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.ins_time);
      Paint_NewImage(ImageBuff, 16, 5*11, ROTATE_90, WHITE);
      Paint_Clear(0xff);
      Paint_DrawString_EN(0, 0, buffer, &Font16, WHITE, BLACK);
      EPD_DisplayWindows(ImageBuff, 10, 208-(len*11)/2, 10+16, 208+(len*11)/2);

    } else {

      // DISPLAY GRID
      Paint_NewImage(ImageBuff, EPD_WIDTH, EPD_HEIGHT, ROTATE_90, WHITE);
      Paint_Clear(0xff);
      if (m_state == input_glu){
        Paint_DrawRectangle(1, 24, 82, 122, BLACK, DRAW_FILL_EMPTY, DOT_PIXEL_1X1);
      }
      if (m_state == input_cho){
        Paint_DrawRectangle(1+82+4, 24, 82+82+4, 122, BLACK, DRAW_FILL_EMPTY, DOT_PIXEL_1X1);
      }
      if (m_state == input_ins){
        Paint_DrawRectangle(1+82+4+82+4, 24, 82+82+4+82, 122, BLACK, DRAW_FILL_EMPTY, DOT_PIXEL_1X1);
      }
      EPD_DisplayPartWindows(ImageBuff, 0, 0, EPD_WIDTH, EPD_HEIGHT);

      if (new_glu){
        len = sprintf(buffer, "%ld", m_topsulin_meas.glu);
      } else {
        len = sprintf(buffer, " - ");
      }
      Paint_NewImage(ImageBuff, 24, len*17, ROTATE_90, WHITE);
      Paint_Clear(0xff);
      Paint_DrawString_EN(0, 0, buffer, &Font24, WHITE, BLACK);
      EPD_DisplayPartWindows(ImageBuff, 35, 41-(len*17)/2, 35+24, 41+(len*17)/2);

      len = sprintf(buffer, "%d", config_manager_get_calc_corr().mantissa);
      //GUI_DrawString_EN(LEFT_TIME_H_POS, TIME_V_POS, buffer, &Font12, WHITE, BLACK);
      len = sprintf(buffer, "%d-%d", config_manager_get_calc_low().mantissa, config_manager_get_calc_high().mantissa);
      //GUI_DrawString_EN(LEFT_TIME_H_POS, TIME_V_POS + 12, buffer, &Font12, WHITE, BLACK);

      if (new_cho){
        len = sprintf(buffer, "%ld", m_topsulin_meas.cho);
      } else {
        len = sprintf(buffer, " - ");
      }
      Paint_NewImage(ImageBuff, 24, len*17, ROTATE_90, WHITE);
      Paint_Clear(0xff);
      Paint_DrawString_EN(0, 0, buffer, &Font24, WHITE, BLACK);
      EPD_DisplayPartWindows(ImageBuff, 35, 125-(len*17)/2, 35+24, 125+(len*17)/2);

      len = sprintf(buffer, "%d", config_manager_get_calc_sens());
      //GUI_DrawString_EN(CENTER_TIME_H_POS, TIME_V_POS, buffer, &Font12, WHITE, BLACK);

      if (new_ins){
        if (config_manager_get_ins_interval() >= 10){
          len = sprintf(buffer, "%ld", m_topsulin_meas.ins / 10);
        } else {
          len = sprintf(buffer, "%ld.%ld", m_topsulin_meas.ins / 10, m_topsulin_meas.ins % 10);
        }
      } else {
        len = sprintf(buffer, " - ");
      }
      Paint_NewImage(ImageBuff, 24, len*17, ROTATE_90, WHITE);
      Paint_Clear(0xff);
      Paint_DrawString_EN(0, 0, buffer, &Font24, WHITE, BLACK);
      EPD_DisplayPartWindows(ImageBuff, 35, 208-(len*17)/2, 35+24, 208+(len*17)/2);

      if (config_manager_get_flags() & CONFIG_BOLO_FLAG){
        //GUI_DrawString_EN(RIGHT_TIME_H_POS, TIME_V_POS, "Calc ON", &Font12, WHITE, BLACK);
      } else {
        //GUI_DrawString_EN(RIGHT_TIME_H_POS, TIME_V_POS, "Calc OFF", &Font12, WHITE, BLACK);
      }

    }

    EPD_TurnOnDisplay();
  }

  if ((m_state == initial)&&(quick_refresh|full_refresh)){

    if (bt_state == 0){
      Paint_NewImage(ImageBuff, ALTO, ANCHO, ROTATE_90, WHITE);
      Paint_DrawBitMap(gImage_initial);
      if(full_refresh){
        EPD_DisplayWindows(ImageBuff, 0, 0, ALTO, ANCHO);
      } else {
        EPD_DisplayPartWindows(ImageBuff, 0, 0, ALTO, ANCHO);
      }
    }

    if (bt_state == 1 || bt_state == 2){
      //BT NAME
      len = sprintf(buffer, "Topsulin-%04X", (uint16_t) NRF_FICR->DEVICEADDR[0] & 0xFFFF);
      Paint_NewImage(ImageBuff, 16, len*11, ROTATE_90, WHITE);
      Paint_Clear(WHITE);
      Paint_DrawString_EN(0, 0, buffer, &Font16, WHITE, BLACK);
      if (full_refresh){
        EPD_DisplayWindows(ImageBuff, ALTO-MARGEN-16, MARGEN, ALTO-MARGEN, MARGEN+len*11);
      } else {
        EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-16, MARGEN, ALTO-MARGEN, MARGEN+len*11);
      }

      // BLUETOOTH ON ICON
      Paint_NewImage(ImageBuff, 16, 16, ROTATE_90, WHITE);
      Paint_DrawBitMap(gImage_icon_bt_16);
      if (full_refresh){
        EPD_DisplayWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-32, ALTO-MARGEN, ANCHO-MARGEN-16);
      } else {
        EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-32, ALTO-MARGEN, ANCHO-MARGEN-16);
      }

      Paint_NewImage(ImageBuff, 16, 16, ROTATE_90, WHITE);
      Paint_Clear(WHITE);
      if (full_refresh){
        EPD_DisplayWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-16, ALTO-MARGEN, ANCHO-MARGEN);
      } else {
        EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-16, ALTO-MARGEN, ANCHO-MARGEN);
      }

      Paint_NewImage(ImageBuff, 24, 6*17, ROTATE_90, WHITE);
      Paint_Clear(WHITE);
      if (full_refresh){
        EPD_DisplayWindows(ImageBuff, 10, 74, 10+24, 74+6*17);
      } else {
        EPD_DisplayPartWindows(ImageBuff, 10, 74, 10+24, 74+6*17);
      }
    }

    if (bt_state == 2){
      // BLUETOOTH CONNECTED ICON
      Paint_NewImage(ImageBuff, 16, 16, ROTATE_90, WHITE);
      Paint_DrawBitMap(gImage_icon_dev_16);
      if (full_refresh){
        EPD_DisplayWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-16, ALTO-MARGEN, ANCHO-MARGEN);
      } else {
        EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-16, ALTO-MARGEN, ANCHO-MARGEN);
      }
    }

  }

  if (quick_refresh|full_refresh){
    EPD_TurnOnDisplay();
  }

  quick_refresh = 0;
  full_refresh = 0;

}

void state_on_event(event_t event){
  switch(m_state){
    case initial:
      if (event == ble_on){
        state_set_bt_state(1);
      }
      if (event == ble_off){
        state_set_bt_state(0);
      }
      break;
    case sleep:
      if (event == button_pressed){
        new_glu = false;
        new_cho = false;
        new_ins = false;
        glu_correction = 0;
        cho_correction = 0;
        m_prev_topsulin_meas = m_topsulin_meas;
        encoder_set_position(m_topsulin_meas.glu);
        m_state = input_glu;
        quick_refresh = 1;
      }
      break;
    case input_glu:
      if (event == button_pressed){
        encoder_set_position(m_topsulin_meas.cho / config_manager_get_cho_interval());
        m_state = input_cho;
        quick_refresh = 1;
      }
      if (event == encoder_update){
        new_glu = true;
        m_topsulin_meas.glu = encoder_get_position();

        if(m_topsulin_meas.glu <= 0){
          new_glu = false;
          m_topsulin_meas.glu = 0;
          encoder_reset_position();
        }

        if(m_topsulin_meas.glu >= 1000){
          m_topsulin_meas.glu = 999;
          encoder_set_position(m_topsulin_meas.glu);
        }

        if (config_manager_get_flags() & CONFIG_BOLO_FLAG){
            if (m_topsulin_meas.glu >= config_manager_get_calc_high().mantissa){
                glu_correction = 1 + (m_topsulin_meas.glu - config_manager_get_calc_high().mantissa) / config_manager_get_calc_corr().mantissa;
            } else if (m_topsulin_meas.glu <= config_manager_get_calc_low().mantissa) {
                glu_correction = (m_topsulin_meas.glu - config_manager_get_calc_low().mantissa) / config_manager_get_calc_corr().mantissa - 1;
            } else {
                glu_correction = 0;
                if (new_cho && (cho_correction != 0)){
                  new_ins = true;
                  m_topsulin_meas.ins = cho_correction * config_manager_get_ins_interval();
                } else {
                  new_ins = false;
                  m_topsulin_meas.ins = m_prev_topsulin_meas.ins;
                }
            }

            if (glu_correction > 0){
              new_ins = true;
              m_topsulin_meas.ins = 0;
              if (new_glu){
                  m_topsulin_meas.ins += glu_correction * config_manager_get_ins_interval();
              }
              if (new_cho){
                  m_topsulin_meas.ins += cho_correction * config_manager_get_ins_interval();
              }
            } else {
                if (glu_correction < 0){
                    new_cho = true;
                    m_topsulin_meas.cho = - glu_correction * config_manager_get_calc_sens();
                } else {
                    new_cho = false;
                    m_topsulin_meas.cho = m_prev_topsulin_meas.cho;
                }
            }
        }
        quick_refresh = 1;
      }
      if (event == long_button_pressed){
        m_topsulin_meas.glu = m_prev_topsulin_meas.glu;
        if (!new_glu){
            state_sleep();
            break;
        }
        new_glu = false;
        glu_correction = 0;
        m_topsulin_meas.ins = 0;
        if (new_cho){
            m_topsulin_meas.ins += cho_correction * config_manager_get_ins_interval();
        } else {
          m_topsulin_meas.ins = m_prev_topsulin_meas.ins;
          new_ins = false;
        }
        quick_refresh = 1;
      }
      break;
    case input_cho:
      if (event == button_pressed){
        encoder_set_position(m_topsulin_meas.ins / config_manager_get_ins_interval());
        m_state = input_ins;
        quick_refresh = 1;
      }
      if (event == encoder_update){
        new_cho = true;
        m_topsulin_meas.cho = encoder_get_position() * config_manager_get_cho_interval();
        if(m_topsulin_meas.cho <= 0){
          encoder_reset_position();
          m_topsulin_meas.cho = 0;
          new_cho = false;
        }
        if(m_topsulin_meas.cho >= 1000){
          m_topsulin_meas.cho = 1000 - config_manager_get_cho_interval();
          encoder_set_position(m_topsulin_meas.cho / config_manager_get_cho_interval());
        }

        if (config_manager_get_flags() & CONFIG_BOLO_FLAG){
            cho_correction = m_topsulin_meas.cho / config_manager_get_calc_sens();

            if (cho_correction == 0){
              if (new_glu && (glu_correction != 0)){
                m_topsulin_meas.ins = glu_correction * config_manager_get_ins_interval();
              } else {
                new_ins = false;
                m_topsulin_meas.ins = m_prev_topsulin_meas.ins;
              }
            }

            if (cho_correction > 0){
              new_ins = true;
              m_topsulin_meas.ins = 0;
              if (new_glu){
                  m_topsulin_meas.ins += glu_correction * config_manager_get_ins_interval();
              }
              if (new_cho){
                  m_topsulin_meas.ins += cho_correction * config_manager_get_ins_interval();
              }
              if (m_topsulin_meas.ins <= 0){
                  new_ins = false;
                  m_topsulin_meas.ins = m_prev_topsulin_meas.ins;
              }
            }
        }

        quick_refresh = 1;
      }
      if (event == long_button_pressed){
        m_topsulin_meas.cho = m_prev_topsulin_meas.cho;
        if (!new_cho){
            state_sleep();
            break;
        }
        new_cho = false;
        cho_correction = 0;
        m_topsulin_meas.ins = 0;
        if (new_glu){
          m_topsulin_meas.ins += 10 * glu_correction;
          if (m_topsulin_meas.ins == 0){
              new_ins = false;
              m_topsulin_meas.ins = m_prev_topsulin_meas.ins;
          }
        } else {
          m_topsulin_meas.ins = m_prev_topsulin_meas.ins;
          new_ins = false;
        }
        quick_refresh = 1;
      }
      break;
    case input_ins:
      if (event == button_pressed){
        encoder_set_position(m_topsulin_meas.glu);
        m_state = input_glu;
        quick_refresh = 1;
      }
      if (event == encoder_update){
        new_ins = true;
        m_topsulin_meas.ins = encoder_get_position() * config_manager_get_ins_interval();
        if(m_topsulin_meas.ins <= 0){
          encoder_reset_position();
          m_topsulin_meas.ins = m_prev_topsulin_meas.ins;
          new_ins = false;
        }
        if(m_topsulin_meas.ins >= 1000){
          m_topsulin_meas.ins = 1000 - config_manager_get_ins_interval();
          encoder_set_position(m_topsulin_meas.ins / config_manager_get_ins_interval());
        }
        quick_refresh = 1;
      }
      if (event == long_button_pressed){
        m_topsulin_meas.ins = m_prev_topsulin_meas.ins;
        if (!new_ins){
            state_sleep();
            break;
        }
        new_ins = false;
        quick_refresh = 1;
      }
      break;
    default:
      break;
  }

  if ((m_state != initial)&&(event == double_button_pressed)){
    state_save_meas();
    m_state = sleep;
    full_refresh = 1;
  }

  state_process_display();
}

void state_show_pin(char* pin){
  if (!was_partial){
    EPD_Init(PART_UPDATE);
    was_partial = true;
    was_full = false;
  }

  Paint_NewImage(ImageBuff, 24, 6*17, ROTATE_90, WHITE);
  Paint_Clear(WHITE);
  EPD_DisplayPartWindows(ImageBuff, 10, 74, 10+24, 74+6*17);

  Paint_NewImage(ImageBuff, 24, 6*17, ROTATE_90, WHITE);
  Paint_DrawString_EN(0, 0, pin, &Font24, WHITE, BLACK);
  EPD_DisplayPartWindows(ImageBuff, 10, 74, 10+24, 74+6*17);

  EPD_TurnOnDisplay();
}

void state_show_pin_error(void){
  if (!was_partial){
    EPD_Init(PART_UPDATE);
    was_partial = true;
    was_full = false;
  }

  Paint_NewImage(ImageBuff, 24, 6*17, ROTATE_90, WHITE);
  Paint_Clear(WHITE);
  EPD_DisplayPartWindows(ImageBuff, 10, 74, 10+24, 74+6*17);

  Paint_NewImage(ImageBuff, 24, 5*17, ROTATE_90, WHITE);
  Paint_DrawString_EN(0, 0, "ERROR", &Font24, WHITE, BLACK);
  EPD_DisplayPartWindows(ImageBuff, 10, 82, 10+24, 82+5*17);

  EPD_TurnOnDisplay();
}

void state_show_pin_ok(void){
  if (!was_partial){
    EPD_Init(PART_UPDATE);
    was_partial = true;
    was_full = false;
  }

  Paint_NewImage(ImageBuff, 24, 6*17, ROTATE_90, WHITE);
  Paint_Clear(WHITE);
  EPD_DisplayPartWindows(ImageBuff, 10, 74, 10+24, 74+6*17);

  Paint_NewImage(ImageBuff, 24, 2*17, ROTATE_90, WHITE);
  Paint_DrawString_EN(0, 0, "OK", &Font24, WHITE, BLACK);
  EPD_DisplayPartWindows(ImageBuff, 10, 108, 10+24, 108+2*17);

  EPD_TurnOnDisplay();
}

static bool clock_set = false;
static bool gls_write = false;

void state_clock_set(){
  if(!clock_set){
    clock_set = true;
    if(gls_write){
      state_begin();
    }
  }
}

void state_gls_write(){
  if(!gls_write){
    gls_write = true;
    if(clock_set){
      state_begin();
    }
  }
}

void state_begin(){
  if (m_state == initial){
    NRF_LOG_INFO("Begin operation\n");
    NRF_LOG_FLUSH();
    m_state = sleep;
    full_refresh = 1;
    state_process_display();
  }
}

void state_sleep(){
  if (m_state == initial){
    NRF_LOG_INFO("Going to sleep in initial state\n");
    NRF_LOG_FLUSH();
  } else {
    if (m_state != sleep){
      NRF_LOG_INFO("Going to sleep\n");
      NRF_LOG_FLUSH();
      m_topsulin_meas.glu = m_prev_topsulin_meas.glu;
      m_topsulin_meas.cho = m_prev_topsulin_meas.cho;
      m_topsulin_meas.ins = m_prev_topsulin_meas.ins;
      m_state = sleep;
      full_refresh = 1;
      state_process_display();
    } else {
      NRF_LOG_INFO("Already sleeping\n");
      NRF_LOG_FLUSH();
    }
  }
}
