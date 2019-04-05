#include "state.h"

#include <stdio.h>

#define NRF_LOG_MODULE_NAME "STATE"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "encoder.h"
#include "FEPD_2in13.h"
#include "GUI_Paint.h"
#include "ImageData.h"
#include "ble_services.h"
#include "clock.h"
#include "config_manager.h"

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

static char                   buffer[8];
static uint8_t                len;

static uint8_t                full_refresh;
static uint8_t                quick_refresh;

void state_init(){
  m_state = initial;

  memset(&m_prev_topsulin_meas, 0, sizeof(m_prev_topsulin_meas));
  m_prev_topsulin_meas.glu = 105;
  m_prev_topsulin_meas.cho = 10;
  m_prev_topsulin_meas.ins = 2;
  struct tm t;
  clock_get_time(&t);
  m_prev_topsulin_meas.glu_time = t;
  m_prev_topsulin_meas.cho_time = t;
  m_prev_topsulin_meas.ins_time = t;

  memset(&m_topsulin_meas, 0, sizeof(m_topsulin_meas));
  m_topsulin_meas.glu = 105;
  m_topsulin_meas.cho = 10;
  m_topsulin_meas.ins = 2;
  m_topsulin_meas.glu_time = t;
  m_topsulin_meas.cho_time = t;
  m_topsulin_meas.ins_time = t;

  full_refresh = 0;
  quick_refresh = 0;
}

#define ICON_V_POS            9
#define LEFT_ICON_H_POS       23
#define CENTER_ICON_H_POS     94
#define RIGHT_ICON_H_POS      165
#define NUMBER_V_POS          36
#define TIME_V_POS            10
#define LEFT_TIME_H_POS       8
#define CENTER_TIME_H_POS     79
#define RIGHT_TIME_H_POS      151

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
      // Definimos que 1 U de insulina se representa como 1 ml
      rec.context.medication.exponent   = 3; // Exponent: Decimal, -3
      rec.context.medication.mantissa   = m_topsulin_meas.ins;
    }

    add_glucose_measurement(rec);
  }
}

void state_process_display(void){
  if ((m_state != initial)&&(quick_refresh|full_refresh)){
    GUI_Clear(WHITE);

    if (config_manager_get_flags() & CONFIG_FLIP_FLAG){
        GUI_DrawIcon(ICON_V_POS, LEFT_ICON_H_POS, gImage_icon_glu_flip, WHITE);
        GUI_DrawIcon(ICON_V_POS, CENTER_ICON_H_POS, gImage_icon_cho_flip, WHITE);
        GUI_DrawIcon(ICON_V_POS, RIGHT_ICON_H_POS, gImage_icon_ins_flip, WHITE);
    } else {
        GUI_DrawIcon(ICON_V_POS, LEFT_ICON_H_POS, gImage_icon_glu, WHITE);
        GUI_DrawIcon(ICON_V_POS, CENTER_ICON_H_POS, gImage_icon_cho, WHITE);
        GUI_DrawIcon(ICON_V_POS, RIGHT_ICON_H_POS, gImage_icon_ins, WHITE);
    }

    uint8_t glu_h_pos = 11;
    if(m_topsulin_meas.glu < 100){
        glu_h_pos += 8;
    }
    if(m_topsulin_meas.glu < 10){
        glu_h_pos += 9;
    }

    uint8_t cho_h_pos = 83;
    if(m_topsulin_meas.cho < 100){
        cho_h_pos += 8;
    }
    if(m_topsulin_meas.cho < 10){
        cho_h_pos += 9;
    }

    uint8_t ins_h_pos = 154;
    if(m_topsulin_meas.ins < 100){
        ins_h_pos += 8;
    }
    if(m_topsulin_meas.ins < 10){
        ins_h_pos += 9;
    }

    if (m_state == sleep){
      len = sprintf(buffer, "%ld", m_topsulin_meas.glu);
      GUI_DrawString_EN(glu_h_pos, NUMBER_V_POS, buffer, &Font24, WHITE, BLACK);
      len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.glu_time);
      GUI_DrawString_EN(LEFT_TIME_H_POS, TIME_V_POS, buffer, &Font16, WHITE, BLACK);

      len = sprintf(buffer, "%ld", m_topsulin_meas.cho);
      GUI_DrawString_EN(cho_h_pos, NUMBER_V_POS, buffer, &Font24, WHITE, BLACK);
      len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.cho_time);
      GUI_DrawString_EN(CENTER_TIME_H_POS, TIME_V_POS, buffer, &Font16, WHITE, BLACK);

      len = sprintf(buffer, "%ld", m_topsulin_meas.ins);
      GUI_DrawString_EN(ins_h_pos, NUMBER_V_POS, buffer, &Font24, WHITE, BLACK);
      len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.ins_time);
      GUI_DrawString_EN(RIGHT_TIME_H_POS, TIME_V_POS, buffer, &Font16, WHITE, BLACK);
    } else {
      if (m_state == input_glu){
        GUI_DrawRectangle(2, 4, 103, 70, BLACK, DRAW_FILL_EMPTY, DOT_PIXEL_2X2);
      }
      if (m_state == input_cho){
        GUI_DrawRectangle(2, 73, 103, 141, BLACK, DRAW_FILL_EMPTY, DOT_PIXEL_2X2);
      }
      if (m_state == input_ins){
        GUI_DrawRectangle(2, 143, 103, 210, BLACK, DRAW_FILL_EMPTY, DOT_PIXEL_2X2);
      }

      if (new_glu){
        len = sprintf(buffer, "%ld", m_topsulin_meas.glu);
      } else {
        glu_h_pos = 11;
        len = sprintf(buffer, "---");
      }
      GUI_DrawString_EN(glu_h_pos, NUMBER_V_POS, buffer, &Font24, WHITE, BLACK);
      len = sprintf(buffer, "mg/dL");
      GUI_DrawString_EN(LEFT_TIME_H_POS, TIME_V_POS, buffer, &Font16, WHITE, BLACK);

      if (new_cho){
        len = sprintf(buffer, "%ld", m_topsulin_meas.cho);
      } else {
        cho_h_pos = 83;
        len = sprintf(buffer, "---");
      }
      GUI_DrawString_EN(cho_h_pos, NUMBER_V_POS, buffer, &Font24, WHITE, BLACK);
      len = sprintf(buffer, " grs ");
      GUI_DrawString_EN(CENTER_TIME_H_POS, TIME_V_POS, buffer, &Font16, WHITE, BLACK);

      if (new_ins){
        len = sprintf(buffer, "%ld", m_topsulin_meas.ins);
      } else {
        ins_h_pos = 154;
        len = sprintf(buffer, "---");
      }
      GUI_DrawString_EN(ins_h_pos, NUMBER_V_POS, buffer, &Font24, WHITE, BLACK);
      if (m_topsulin_meas.ins != (glu_correction + cho_correction) && new_ins){
        len = sprintf(buffer, " -U- ");
      } else {
        len = sprintf(buffer, "  U  ");
      }
      GUI_DrawString_EN(RIGHT_TIME_H_POS, TIME_V_POS, buffer, &Font16, WHITE, BLACK);
    }

  }

  if (full_refresh){
    EPD_DisplayFull();
    full_refresh = 0;
  }

  if (quick_refresh){
    EPD_DisplayPartial(1, 1, 104, 212);
    quick_refresh = 0;
  }

}

void state_on_event(event_t event){
  switch(m_state){
    case initial:
      if (event == ble_on){
        //GUI_DrawIcon(2, 4, gImage_icon_bt, WHITE);
        GUI_DrawBitMap(gImage_IMAGE_1);
        full_refresh = 1;
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
        encoder_set_position(m_topsulin_meas.cho / 5);
        m_state = input_cho;
        quick_refresh = 1;
      }
      if (event == encoder_update){
        new_glu = true;
        m_topsulin_meas.glu = encoder_get_position();
        if(m_topsulin_meas.glu < 0){
          encoder_reset_position();
          m_topsulin_meas.glu = 0;
        }
        if(m_topsulin_meas.glu >= 1000){
          //encoder_reset_position();
          m_topsulin_meas.glu = 999;
          encoder_set_position(m_topsulin_meas.glu);
        }


        if (m_topsulin_meas.glu >= config_manager_get_calc_high().mantissa){
            glu_correction = 1 + (m_topsulin_meas.glu - config_manager_get_calc_high().mantissa) / config_manager_get_calc_corr().mantissa;
        } else if (m_topsulin_meas.glu <= config_manager_get_calc_low().mantissa) {
            glu_correction = (m_topsulin_meas.glu - config_manager_get_calc_low().mantissa) / config_manager_get_calc_corr().mantissa - 1;
        } else {
            glu_correction = 0;
            if (new_cho && (cho_correction != 0)){
              m_topsulin_meas.ins = cho_correction;
            } else {
              new_ins = false;
            }
        }

        if (glu_correction > 0){
          new_ins = true;
          m_topsulin_meas.ins = 0;
          if (new_glu){
              m_topsulin_meas.ins += glu_correction;
          }
          if (new_cho){
              m_topsulin_meas.ins += cho_correction;
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
            m_topsulin_meas.ins += cho_correction;
        } else {
          m_topsulin_meas.ins = m_prev_topsulin_meas.ins;
          new_ins = false;
        }
        quick_refresh = 1;
      }
      break;
    case input_cho:
      if (event == button_pressed){
        encoder_set_position(m_topsulin_meas.ins);
        m_state = input_ins;
        quick_refresh = 1;
      }
      if (event == encoder_update){
        new_cho = true;
        m_topsulin_meas.cho = encoder_get_position() * 5;
        if(m_topsulin_meas.cho < 0){
          encoder_reset_position();
          m_topsulin_meas.cho = 0;
        }
        if(m_topsulin_meas.cho >= 1000){
          //encoder_reset_position();
          m_topsulin_meas.cho = 995;
          encoder_set_position(m_topsulin_meas.cho / 5);
        }

        cho_correction = m_topsulin_meas.cho / config_manager_get_calc_sens();

        if (cho_correction == 0){
          if (new_glu && (glu_correction != 0)){
            m_topsulin_meas.ins = glu_correction;
          } else {
            new_ins = false;
          }
        }

        if (cho_correction > 0){
          new_ins = true;
          m_topsulin_meas.ins = 0;
          if (new_glu){
              m_topsulin_meas.ins += glu_correction;
          }
          if (new_cho){
              m_topsulin_meas.ins += cho_correction;
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
          m_topsulin_meas.ins += glu_correction;
          if (m_topsulin_meas.ins == 0){
              new_ins = false;
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
        m_topsulin_meas.ins = encoder_get_position();
        if(m_topsulin_meas.ins <= 0){
          encoder_reset_position();
          m_topsulin_meas.ins = 0;
          new_ins = false;
        }
        if(m_topsulin_meas.ins >= 1000){
          //encoder_reset_position();
          m_topsulin_meas.ins = 999;
          encoder_set_position(m_topsulin_meas.ins);
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

  if ((m_state != initial)&&(m_state != sleep)&&(event == time_update)){
    quick_refresh = 1;
  }

  state_process_display();
}

void state_show_pin(char* pin){
  //if (m_state == initial){
    //GUI_ClearWindows(1, 1, 51, 212, WHITE);
    //GUI_DrawIcon(2, 4, gImage_icon_lock, WHITE);
    //GUI_DrawString_EN(40, 12, pin, &Font24, WHITE, BLACK);
    GUI_ClearWindows(1, 1, 104, 212, WHITE);
    GUI_DrawIcon(5, LEFT_ICON_H_POS, gImage_icon_lock, WHITE);
    GUI_DrawString_EN(CENTER_TIME_H_POS, NUMBER_V_POS, pin, &Font24, WHITE, BLACK);
    EPD_DisplayFull();
  //}
}

void state_begin(){
  if (m_state == initial){
    m_state = sleep;
    full_refresh = 1;
    state_process_display();
  }
}

void state_sleep(){
  if (m_state == initial){
    NRF_LOG_INFO("Going to sleep in initial state\n");
    NRF_LOG_FLUSH();
    GUI_DrawBitMap(gImage_IMAGE_0);
    EPD_DisplayFull();
  } else {
    if (m_state != sleep){
      NRF_LOG_INFO("Going to sleep: m_state != sleep\n");
      NRF_LOG_FLUSH();
      m_topsulin_meas.glu = m_prev_topsulin_meas.glu;
      m_topsulin_meas.cho = m_prev_topsulin_meas.cho;
      m_topsulin_meas.ins = m_prev_topsulin_meas.ins;
      m_state = sleep;
      full_refresh = 1;
      state_process_display();
    } else {
      NRF_LOG_INFO("Going to sleep: m_state == sleep\n");
      NRF_LOG_FLUSH();
    }
  }
}
