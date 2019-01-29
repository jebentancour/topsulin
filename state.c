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
    sel_glu,
    sel_cho,
    sel_ins,
    input_cho,
    input_glu,
    input_ins
} internal_state_t;

typedef struct
{
    int glu;
    int glu_correction;
    struct tm glu_time;
    bool new_glu;
    int cho;
    int cho_correction;
    struct tm cho_time;
    bool new_cho;
    int ins;
    struct tm ins_time;
    bool new_ins;
} topsulin_meas_t;

static internal_state_t       m_state;
static topsulin_meas_t        m_topsulin_meas;
static char                   buffer[8];
static uint8_t                len;
static int                    encoder_pos;
static uint8_t                full_refresh;
static uint8_t                quick_refresh;

void state_init()
{
  m_state = initial;
  memset(&m_topsulin_meas, 0, sizeof(m_topsulin_meas));
  m_topsulin_meas.glu = 105;
  m_topsulin_meas.cho = 10;
  m_topsulin_meas.ins = 2;
  clock_get_time(&m_topsulin_meas.glu_time);
  clock_get_time(&m_topsulin_meas.cho_time);
  clock_get_time(&m_topsulin_meas.ins_time);
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
  if (m_topsulin_meas.new_glu || m_topsulin_meas.new_cho || m_topsulin_meas.new_ins){
    struct tm t;
    clock_get_time(&t);

    if (m_topsulin_meas.new_glu){
      clock_get_time(&m_topsulin_meas.glu_time);
    }

    if (m_topsulin_meas.new_cho){
      clock_get_time(&m_topsulin_meas.cho_time);
    }

    if (m_topsulin_meas.new_ins){
      clock_get_time(&m_topsulin_meas.ins_time);
    }

    ble_gls_rec_t rec;

    rec.meas.flags = BLE_GLS_MEAS_FLAG_TIME_OFFSET |
                     BLE_GLS_MEAS_FLAG_CONC_TYPE_LOC |
                     BLE_GLS_MEAS_FLAG_SENSOR_STATUS |
                     BLE_GLS_MEAS_FLAG_UNITS_KG_L;

    if (m_topsulin_meas.new_cho || m_topsulin_meas.new_ins){
      rec.meas.flags |= BLE_GLS_MEAS_FLAG_CONTEXT_INFO;
    }

    rec.meas.base_time.year                 = t.tm_year + 1900;
    rec.meas.base_time.month                = t.tm_mon + 1;
    rec.meas.base_time.day                  = t.tm_mday;
    rec.meas.base_time.hours                = t.tm_hour;
    rec.meas.base_time.minutes              = t.tm_min;
    rec.meas.base_time.seconds              = t.tm_sec;

    if (m_topsulin_meas.new_glu){
      rec.meas.glucose_concentration.exponent = -5;
      rec.meas.glucose_concentration.mantissa = m_topsulin_meas.glu;
    } else {
      rec.meas.glucose_concentration.exponent = 0;
      rec.meas.glucose_concentration.mantissa = 0;
    }

    rec.meas.time_offset                    = 0;
    rec.meas.type                           = BLE_GLS_MEAS_TYPE_UNDET_BLOOD;
    rec.meas.sample_location                = BLE_GLS_MEAS_LOC_NOT_AVAIL;
    rec.meas.sensor_status_annunciation     = 0;

    //rec.meas.sensor_status_annunciation     = BLE_GLS_MEAS_STATUS_BATT_LOW;

    if (m_topsulin_meas.new_cho){
      rec.context.flags |= BLE_GLS_CONTEXT_FLAG_CARB;
      rec.context.carbohydrate_id = BLE_GLS_CONTEXT_CARB_LUNCH;
      rec.context.carbohydrate.exponent = 0;
      rec.context.carbohydrate.mantissa = m_topsulin_meas.cho;
    }

    if (m_topsulin_meas.new_ins){
      rec.context.flags |= BLE_GLS_CONTEXT_FLAG_MED;
      rec.context.flags |= BLE_GLS_CONTEXT_FLAG_MED_L;
      rec.context.medication_id         = BLE_GLS_CONTEXT_MED_RAPID;
      rec.context.medication.exponent   = 3;
      rec.context.medication.mantissa   = m_topsulin_meas.ins;
    }

    add_glucose_measurement(rec);

    m_topsulin_meas.new_glu = false;
    m_topsulin_meas.new_cho = false;
    m_topsulin_meas.new_ins = false;
  }

  m_topsulin_meas.glu_correction = 0;
  m_topsulin_meas.cho_correction = 0;
}

void state_process_display(void)
{
  struct tm m_time;
  clock_get_time(&m_time);
  switch(m_state){
    case initial:
      break;
    case sleep:
        GUI_Clear(WHITE);

        GUI_DrawIcon(ICON_V_POS, LEFT_ICON_H_POS, gImage_icon_glu, WHITE);
        len = sprintf(buffer, "%03d", m_topsulin_meas.glu);
        buffer[len] = '\0';
        GUI_DrawString_EN(11, NUMBER_V_POS, buffer, &Font24, WHITE, BLACK);
        len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.glu_time);
        GUI_DrawString_EN(LEFT_TIME_H_POS, TIME_V_POS, buffer, &Font16, WHITE, BLACK);

        GUI_DrawIcon(ICON_V_POS, CENTER_ICON_H_POS, gImage_icon_cho, WHITE);
        len = sprintf(buffer, "%02d", m_topsulin_meas.cho);
        buffer[len] = '\0';
        GUI_DrawString_EN(91, NUMBER_V_POS, buffer, &Font24, WHITE, BLACK);
        len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.cho_time);
        GUI_DrawString_EN(CENTER_TIME_H_POS, TIME_V_POS, buffer, &Font16, WHITE, BLACK);

        GUI_DrawIcon(ICON_V_POS, RIGHT_ICON_H_POS, gImage_icon_ins, WHITE);
        len = sprintf(buffer, "%02d", m_topsulin_meas.ins);
        buffer[len] = '\0';
        GUI_DrawString_EN(162, NUMBER_V_POS, buffer, &Font24, WHITE, BLACK);
        len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.ins_time);
        GUI_DrawString_EN(RIGHT_TIME_H_POS, TIME_V_POS, buffer, &Font16, WHITE, BLACK);
      break;
    case sel_glu:
        GUI_Clear(WHITE);

        GUI_DrawRectangle(1, 1, 104, 70, BLACK, DRAW_FILL_FULL, DOT_PIXEL_DFT);
        GUI_DrawIcon(ICON_V_POS, LEFT_ICON_H_POS, gImage_icon_glu, BLACK);
        len = sprintf(buffer, "%03d", m_topsulin_meas.glu);
        buffer[len] = '\0';
        GUI_DrawString_EN(11, NUMBER_V_POS, buffer, &Font24, BLACK, WHITE);
        len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.glu_time);
        GUI_DrawString_EN(LEFT_TIME_H_POS, TIME_V_POS, buffer, &Font16, BLACK, WHITE);

        GUI_DrawIcon(ICON_V_POS, CENTER_ICON_H_POS, gImage_icon_cho, WHITE);
        len = sprintf(buffer, "%02d", m_topsulin_meas.cho);
        buffer[len] = '\0';
        GUI_DrawString_EN(91, NUMBER_V_POS, buffer, &Font24, WHITE, BLACK);
        len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.cho_time);
        GUI_DrawString_EN(CENTER_TIME_H_POS, TIME_V_POS, buffer, &Font16, WHITE, BLACK);

        GUI_DrawIcon(ICON_V_POS, RIGHT_ICON_H_POS, gImage_icon_ins, WHITE);
        len = sprintf(buffer, "%02d", m_topsulin_meas.ins);
        buffer[len] = '\0';
        GUI_DrawString_EN(162, NUMBER_V_POS, buffer, &Font24, WHITE, BLACK);
        len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.ins_time);
        GUI_DrawString_EN(RIGHT_TIME_H_POS, TIME_V_POS, buffer, &Font16, WHITE, BLACK);
      break;
    case sel_cho:
        GUI_Clear(WHITE);

        GUI_DrawIcon(ICON_V_POS, LEFT_ICON_H_POS, gImage_icon_glu, WHITE);
        len = sprintf(buffer, "%03d", m_topsulin_meas.glu);
        buffer[len] = '\0';
        GUI_DrawString_EN(11, NUMBER_V_POS, buffer, &Font24, WHITE, BLACK);
        len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.glu_time);
        GUI_DrawString_EN(LEFT_TIME_H_POS, TIME_V_POS, buffer, &Font16, WHITE, BLACK);

        GUI_DrawRectangle(1, 72, 104, 141, BLACK, DRAW_FILL_FULL, DOT_PIXEL_DFT);
        GUI_DrawIcon(ICON_V_POS, CENTER_ICON_H_POS, gImage_icon_cho, BLACK);
        len = sprintf(buffer, "%02d", m_topsulin_meas.cho);
        buffer[len] = '\0';
        GUI_DrawString_EN(91, NUMBER_V_POS, buffer, &Font24, BLACK, WHITE);
        len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.cho_time);
        GUI_DrawString_EN(CENTER_TIME_H_POS, TIME_V_POS, buffer, &Font16, BLACK, WHITE);

        GUI_DrawIcon(ICON_V_POS, RIGHT_ICON_H_POS, gImage_icon_ins, WHITE);
        len = sprintf(buffer, "%02d", m_topsulin_meas.ins);
        buffer[len] = '\0';
        GUI_DrawString_EN(162, NUMBER_V_POS, buffer, &Font24, WHITE, BLACK);
        len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.ins_time);
        GUI_DrawString_EN(RIGHT_TIME_H_POS, TIME_V_POS, buffer, &Font16, WHITE, BLACK);
      break;
    case sel_ins:
        GUI_Clear(WHITE);

        GUI_DrawIcon(ICON_V_POS, LEFT_ICON_H_POS, gImage_icon_glu, WHITE);
        len = sprintf(buffer, "%03d", m_topsulin_meas.glu);
        buffer[len] = '\0';
        GUI_DrawString_EN(11, NUMBER_V_POS, buffer, &Font24, WHITE, BLACK);
        len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.glu_time);
        GUI_DrawString_EN(LEFT_TIME_H_POS, TIME_V_POS, buffer, &Font16, WHITE, BLACK);

        GUI_DrawIcon(ICON_V_POS, CENTER_ICON_H_POS, gImage_icon_cho, WHITE);
        len = sprintf(buffer, "%02d", m_topsulin_meas.cho);
        buffer[len] = '\0';
        GUI_DrawString_EN(91, NUMBER_V_POS, buffer, &Font24, WHITE, BLACK);
        len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.cho_time);
        GUI_DrawString_EN(CENTER_TIME_H_POS, TIME_V_POS, buffer, &Font16, WHITE, BLACK);

        GUI_DrawRectangle(1, 142, 104, 212, BLACK, DRAW_FILL_FULL, DOT_PIXEL_DFT);
        GUI_DrawIcon(ICON_V_POS, RIGHT_ICON_H_POS, gImage_icon_ins, BLACK);
        len = sprintf(buffer, "%02d", m_topsulin_meas.ins);
        buffer[len] = '\0';
        GUI_DrawString_EN(162, NUMBER_V_POS, buffer, &Font24, BLACK, WHITE);
        len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.ins_time);
        GUI_DrawString_EN(RIGHT_TIME_H_POS, TIME_V_POS, buffer, &Font16, BLACK, WHITE);
      break;
    case input_glu:
        GUI_Clear(WHITE);

        GUI_DrawRectangle(1, 1, 104, 141, BLACK, DRAW_FILL_FULL, DOT_PIXEL_DFT);
        GUI_DrawIcon(ICON_V_POS, CENTER_ICON_H_POS, gImage_icon_glu, BLACK);
        len = sprintf(buffer, "%03d", m_topsulin_meas.glu);
        buffer[len] = '\0';
        GUI_DrawString_EN(11, 56, buffer, &Font24, BLACK, WHITE);

        len = strftime(buffer, sizeof(buffer), "%H:%M", &m_time);
        GUI_DrawString_EN(LEFT_TIME_H_POS, TIME_V_POS,  buffer, &Font16, BLACK, WHITE);

        GUI_DrawIcon(ICON_V_POS, RIGHT_ICON_H_POS, gImage_icon_ins, WHITE);
        len = sprintf(buffer, "%02d", m_topsulin_meas.glu_correction);
        buffer[len] = '\0';
        GUI_DrawString_EN(162, NUMBER_V_POS, buffer, &Font24, WHITE, BLACK);
      break;
    case input_cho:
        GUI_Clear(WHITE);

        GUI_DrawRectangle(1, 1, 104, 141, BLACK, DRAW_FILL_FULL, DOT_PIXEL_DFT);
        GUI_DrawIcon(ICON_V_POS, CENTER_ICON_H_POS, gImage_icon_cho, BLACK);
        len = sprintf(buffer, "%02d", m_topsulin_meas.cho);
        buffer[len] = '\0';
        GUI_DrawString_EN(11, 56, buffer, &Font24, BLACK, WHITE);

        len = strftime(buffer, sizeof(buffer), "%H:%M", &m_time);
        GUI_DrawString_EN(LEFT_TIME_H_POS, TIME_V_POS,  buffer, &Font16, BLACK, WHITE);

        GUI_DrawIcon(ICON_V_POS, RIGHT_ICON_H_POS, gImage_icon_ins, WHITE);
        len = sprintf(buffer, "%02d", m_topsulin_meas.cho_correction);
        buffer[len] = '\0';
        GUI_DrawString_EN(162, NUMBER_V_POS, buffer, &Font24, WHITE, BLACK);
      break;
    case input_ins:
        GUI_Clear(WHITE);

        GUI_DrawRectangle(1, 1, 104, 141, BLACK, DRAW_FILL_FULL, DOT_PIXEL_DFT);
        GUI_DrawIcon(ICON_V_POS, CENTER_ICON_H_POS, gImage_icon_ins, BLACK);
        len = sprintf(buffer, "%02d", m_topsulin_meas.ins);
        buffer[len] = '\0';
        GUI_DrawString_EN(11, 56, buffer, &Font24, BLACK, WHITE);

        len = strftime(buffer, sizeof(buffer), "%H:%M", &m_time);
        GUI_DrawString_EN(LEFT_TIME_H_POS, TIME_V_POS,  buffer, &Font16, BLACK, WHITE);

        len = sprintf(buffer, "%02d", m_topsulin_meas.glu_correction);
        buffer[len] = '\0';
        GUI_DrawString_EN(162, 50, buffer, &Font24, WHITE, BLACK);
        len = sprintf(buffer, "%02d", m_topsulin_meas.cho_correction);
        buffer[len] = '\0';
        GUI_DrawString_EN(162, 26, buffer, &Font24, WHITE, BLACK);
      break;
    default:
      break;
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

void state_on_event(event_t event)
{
  switch(m_state){
    case initial:
      if (event == ble_on){
        GUI_DrawIcon(8, 4, gImage_icon_bt, WHITE);
        EPD_DisplayPartial(1, 1, 104, 212);
      }
      break;
    case sleep:
      if (event == button_pressed){
        encoder_reset_position();
        m_state = sel_glu;
        quick_refresh = 1;
      }
      state_save_meas();
      break;
    case sel_glu:
      if (event == button_pressed){
        m_state = input_glu;
        encoder_set_position(m_topsulin_meas.glu);
        full_refresh = 1;
      }
      if (event == encoder_update){
        encoder_pos = encoder_get_position();
        if (encoder_pos > 0){
          m_state = sel_cho;
          quick_refresh = 1;
        }
        encoder_reset_position();
      }
      if (event == long_button_pressed){
        m_state = sleep;
        state_save_meas();
        full_refresh = 1;
      }
      break;
    case sel_cho:
      if (event == button_pressed){
        m_state = input_cho;
        encoder_set_position(m_topsulin_meas.cho);
        if (m_topsulin_meas.glu_correction < 0){
          m_topsulin_meas.cho = ( - m_topsulin_meas.glu_correction) * config_manager_get_calc_sens();
        }
        full_refresh = 1;
      }
      if (event == encoder_update){
        encoder_pos = encoder_get_position();
        if (encoder_pos > 0){
          m_state = sel_ins;
          quick_refresh = 1;
        }
        if (encoder_pos < 0) {
          m_state = sel_glu;
          quick_refresh = 1;
        }
        encoder_reset_position();
      }
      if (event == long_button_pressed){
        m_state = sleep;
        state_save_meas();
        full_refresh = 1;
      }
      break;
    case sel_ins:
      if (event == button_pressed){
        m_state = input_ins;
        encoder_set_position(m_topsulin_meas.ins);
        m_topsulin_meas.ins = 0;
        if (m_topsulin_meas.new_glu){
          m_topsulin_meas.ins += m_topsulin_meas.glu_correction;
        }
        if (m_topsulin_meas.new_cho){
          m_topsulin_meas.ins += m_topsulin_meas.cho_correction;
        }
        full_refresh = 1;
      }
      if (event == encoder_update){
        encoder_pos = encoder_get_position();
        if (encoder_pos < 0){
          m_state = sel_cho;
          quick_refresh = 1;
        }
        encoder_reset_position();
      }
      if (event == long_button_pressed){
        m_state = sleep;
        state_save_meas();
        full_refresh = 1;
      }
      break;
    case input_glu:
      if (event == button_pressed){
        m_state = sel_cho;
        m_topsulin_meas.new_glu = true;
        clock_get_time(&m_topsulin_meas.glu_time);
        full_refresh = 1;
      }
      if (event == encoder_update){
        m_topsulin_meas.glu = encoder_get_position();
        if(((int)m_topsulin_meas.glu) < 0){
          m_topsulin_meas.glu = 0;
          encoder_reset_position();
        }
        if (m_topsulin_meas.glu >= config_manager_get_calc_high().mantissa){
          m_topsulin_meas.glu_correction = 1 + (m_topsulin_meas.glu - config_manager_get_calc_high().mantissa) / config_manager_get_calc_corr().mantissa;
        } else if (m_topsulin_meas.glu <= config_manager_get_calc_low().mantissa) {
          m_topsulin_meas.glu_correction = (m_topsulin_meas.glu - config_manager_get_calc_low().mantissa) / config_manager_get_calc_corr().mantissa - 1;
        } else {
          m_topsulin_meas.glu_correction = 0;
        }
        quick_refresh = 1;
      }
      if (event == long_button_pressed){
        m_state = sel_glu;
        m_topsulin_meas.new_glu = false;
        state_save_meas();
        full_refresh = 1;
      }
      break;
    case input_cho:
      if (event == button_pressed){
        m_state = sel_ins;
        m_topsulin_meas.new_cho = true;
        clock_get_time(&m_topsulin_meas.cho_time);
        full_refresh = 1;
      }
      if (event == encoder_update){
        m_topsulin_meas.cho = encoder_get_position();
        if(((int)m_topsulin_meas.cho) < 0){
          m_topsulin_meas.cho = 0;
          encoder_reset_position();
        }
        m_topsulin_meas.cho_correction = m_topsulin_meas.cho / config_manager_get_calc_sens();
        quick_refresh = 1;
      }
      if (event == long_button_pressed){
        m_state = sel_cho;
        m_topsulin_meas.new_cho = false;
        state_save_meas();
        full_refresh = 1;
      }
      break;
    case input_ins:
      if (event == button_pressed){
        m_state = sleep;
        m_topsulin_meas.new_ins = true;
        clock_get_time(&m_topsulin_meas.ins_time);
        state_save_meas();
        full_refresh = 1;
      }
      if (event == encoder_update){
        m_topsulin_meas.ins = encoder_get_position();
        if(((int)m_topsulin_meas.ins) < 0){
          m_topsulin_meas.ins = 0;
          encoder_reset_position();
        }
        quick_refresh = 1;
      }
      if (event == long_button_pressed){
        m_state = sel_ins;
        m_topsulin_meas.new_ins = false;
        state_save_meas();
        full_refresh = 1;
      }
      break;
    default:
      break;
  }
  state_process_display();
}

void state_show_pin(char* pin){
  if (m_state == initial){
    GUI_ClearWindows(1, 1, 51, 212, WHITE);
    GUI_DrawIcon(2, 4, gImage_icon_lock, WHITE);
    GUI_DrawString_EN(40, 12, pin, &Font24, WHITE, BLACK);
    EPD_DisplayFull();
  }
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
      m_state = sleep;
      NRF_LOG_INFO("Going to sleep: m_state != sleep\n");
      NRF_LOG_FLUSH();
      full_refresh = 1;
      state_process_display();
    } else {
      NRF_LOG_INFO("Going to sleep: m_state == sleep\n");
      NRF_LOG_FLUSH();
    }
  }
}
