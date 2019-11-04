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

    CRITICAL_REGION_ENTER();
    add_glucose_measurement(rec);
    CRITICAL_REGION_EXIT();

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
  NRF_LOG_INFO("state_set_bt_state %d\n", state);
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

void state_update_config(void){
  if (m_state == initial) return;
  quick_refresh = 1;
  state_process_display();
}

void state_process_display(void){

  UWORD color;
  if (config_manager_get_flags() & CONFIG_COLOR_FLAG){
    color = WHITE;
  } else {
    color = BLACK;
  }

  UWORD rotate;
  bool flip = config_manager_get_flags() & CONFIG_FLIP_FLAG;
  if (flip){
    rotate = ROTATE_270;
  } else {
    rotate = ROTATE_90;
  }

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

    if (m_state == sleep){

      // BLANK
      Paint_NewImage(ImageBuff, EPD_WIDTH, EPD_HEIGHT, rotate, color);
      Paint_Clear(color);
      if (full_refresh){
        EPD_DisplayWindows(ImageBuff, 0, 0, EPD_WIDTH, EPD_HEIGHT);
      } else {
        EPD_DisplayPartWindows(ImageBuff, 0, 0, EPD_WIDTH, EPD_HEIGHT);
      }

      // GLU ICON
      Paint_NewImage(ImageBuff, 24, 24, rotate, !color);
      if (flip){
        Paint_DrawBitMap(gImage_icon_glu_flip_24);
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, 2*MARGEN+24, 204-12, 2*MARGEN+24+24, 204+12);
        } else {
          EPD_DisplayPartWindows(ImageBuff, 2*MARGEN+24, 204-12, 2*MARGEN+24+24, 204+12);
        }
      } else {
        Paint_DrawBitMap(gImage_icon_glu_24);
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, ALTO-MARGEN-24-24, 46-12, ALTO-MARGEN-24, 46+12);
        } else {
          EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-24-24, 46-12, ALTO-MARGEN-24, 46+12);
        }
      }

      // CHO ICON
      Paint_NewImage(ImageBuff, 24, 24, rotate, !color);
      if (flip){
        Paint_DrawBitMap(gImage_icon_cho_flip_24);
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, 2*MARGEN+24, 125-12, 2*MARGEN+24+24, 125+12);
        } else {
          EPD_DisplayPartWindows(ImageBuff, 2*MARGEN+24, 125-12, 2*MARGEN+24+24, 125+12);
        }
      } else {
        Paint_DrawBitMap(gImage_icon_cho_24);
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, ALTO-MARGEN-24-24, 125-12, ALTO-MARGEN-24, 125+12);
        } else {
          EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-24-24, 125-12, ALTO-MARGEN-24, 125+12);
        }
      }

      // INS ICON
      Paint_NewImage(ImageBuff, 24, 24, rotate, !color);
      if (flip){
        Paint_DrawBitMap(gImage_icon_ins_flip_24);
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, 2*MARGEN+24, 46-12, 2*MARGEN+24+24, 46+12);
        } else {
          EPD_DisplayPartWindows(ImageBuff, 2*MARGEN+24, 46-12, 2*MARGEN+24+24, 46+12);
        }
      } else {
        Paint_DrawBitMap(gImage_icon_ins_24);
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, ALTO-MARGEN-24-24, 204-12, ALTO-MARGEN-24, 204+12);
        } else {
          EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-24-24, 204-12, ALTO-MARGEN-24, 204+12);
        }
      }

      // GLU NUMBER
      len = sprintf(buffer, "%ld", m_topsulin_meas.glu);
      Paint_NewImage(ImageBuff, 24, len*17, rotate, color);
      Paint_Clear(color);
      Paint_DrawString_EN(0, 0, buffer, &Font24, color, !color);
      if (flip) {
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, ALTO+MARGEN-35-24, 204-(len*17)/2, ALTO+MARGEN-35, 204+(len*17)/2);
        } else {
          EPD_DisplayPartWindows(ImageBuff, ALTO+MARGEN-35-24, 204-(len*17)/2, ALTO+MARGEN-35, 204+(len*17)/2);
        }
      } else {
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, 35, 46-(len*17)/2, 35+24, 46+(len*17)/2);
        } else {
          EPD_DisplayPartWindows(ImageBuff, 35, 46-(len*17)/2, 35+24, 46+(len*17)/2);
        }
      }

      // GLU TIME
      len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.glu_time);
      Paint_NewImage(ImageBuff, 16, 5*11, rotate, color);
      Paint_Clear(color);
      Paint_DrawString_EN(0, 0, buffer, &Font16, color, !color);
      if (flip){
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, ALTO+MARGEN-10-16, 204-(len*11)/2, ALTO+MARGEN-10, 204+(len*11)/2);
        } else {
          EPD_DisplayPartWindows(ImageBuff, ALTO+MARGEN-10-16, 204-(len*11)/2, ALTO+MARGEN-10, 204+(len*11)/2);
        }
      } else {
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, 10, 46-(len*11)/2, 10+16, 46+(len*11)/2);
        } else {
          EPD_DisplayPartWindows(ImageBuff, 10, 46-(len*11)/2, 10+16, 46+(len*11)/2);
        }
      }

      // CHO NUMBER
      len = sprintf(buffer, "%ld", m_topsulin_meas.cho);
      Paint_NewImage(ImageBuff, 24, len*17, rotate, color);
      Paint_Clear(color);
      Paint_DrawString_EN(0, 0, buffer, &Font24, color, !color);
      if (flip){
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, ALTO+MARGEN-35-24, 125-(len*17)/2, ALTO+MARGEN-35, 125+(len*17)/2);
        } else {
          EPD_DisplayPartWindows(ImageBuff, ALTO+MARGEN-35-24, 125-(len*17)/2, ALTO+MARGEN-35, 125+(len*17)/2);
        }
      } else {
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, 35, 125-(len*17)/2, 35+24, 125+(len*17)/2);
        } else {
          EPD_DisplayPartWindows(ImageBuff, 35, 125-(len*17)/2, 35+24, 125+(len*17)/2);
        }
      }

      // CHO TIME
      len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.cho_time);
      Paint_NewImage(ImageBuff, 16, 5*11, rotate, color);
      Paint_Clear(color);
      Paint_DrawString_EN(0, 0, buffer, &Font16, color, !color);
      if (flip){
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, ALTO+MARGEN-10-16, 125-(len*11)/2, ALTO+MARGEN-10, 125+(len*11)/2);
        } else {
          EPD_DisplayPartWindows(ImageBuff, ALTO+MARGEN-10-16, 125-(len*11)/2, ALTO+MARGEN-10, 125+(len*11)/2);
        }
      } else {
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, 10, 125-(len*11)/2, 10+16, 125+(len*11)/2);
        } else {
          EPD_DisplayPartWindows(ImageBuff, 10, 125-(len*11)/2, 10+16, 125+(len*11)/2);
        }
      }

      // INS NUMBER
      if (config_manager_get_ins_interval() >= 10){
        len = sprintf(buffer, "%ld", m_topsulin_meas.ins / 10);
      } else {
        len = sprintf(buffer, "%ld.%ld", m_topsulin_meas.ins / 10, m_topsulin_meas.ins % 10);
      }
      Paint_NewImage(ImageBuff, 24, len*17, rotate, color);
      Paint_Clear(color);
      Paint_DrawString_EN(0, 0, buffer, &Font24, color, !color);
      if (flip){
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, ALTO+MARGEN-35-24, 46-(len*17)/2, ALTO+MARGEN-35, 46+(len*17)/2);
        } else {
          EPD_DisplayPartWindows(ImageBuff, ALTO+MARGEN-35-24, 46-(len*17)/2, ALTO+MARGEN-35, 46+(len*17)/2);
        }
      } else {
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, 35, 204-(len*17)/2, 35+24, 204+(len*17)/2);
        } else {
          EPD_DisplayPartWindows(ImageBuff, 35, 204-(len*17)/2, 35+24, 204+(len*17)/2);
        }
      }

      // INS TIME
      len = strftime(buffer, sizeof(buffer), "%H:%M", &m_topsulin_meas.ins_time);
      Paint_NewImage(ImageBuff, 16, 5*11, rotate, color);
      Paint_Clear(color);
      Paint_DrawString_EN(0, 0, buffer, &Font16, color, !color);
      if (flip){
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, ALTO+MARGEN-10-16, 46-(len*11)/2, ALTO+MARGEN-10, 46+(len*11)/2);
        } else {
          EPD_DisplayPartWindows(ImageBuff, ALTO+MARGEN-10-16, 46-(len*11)/2, ALTO+MARGEN-10, 46+(len*11)/2);
        }
      } else {
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, 10, 204-(len*11)/2, 10+16, 204+(len*11)/2);
        } else {
          EPD_DisplayPartWindows(ImageBuff, 10, 204-(len*11)/2, 10+16, 204+(len*11)/2);
        }
      }

    }

    if ((m_state != sleep)&&(quick_refresh)) {

      // DISPLAY GRID
      Paint_NewImage(ImageBuff, EPD_WIDTH, EPD_HEIGHT, ROTATE_270, color);
      Paint_Clear(color);
      if (m_state == input_glu){
        if (flip){
          Paint_DrawRectangle(MARGEN, MARGEN+24, (ANCHO-2*MARGEN)/3+MARGEN, ALTO-MARGEN, !color, DRAW_FILL_EMPTY, DOT_PIXEL_1X1);
        } else {
          Paint_DrawRectangle(2*(ANCHO-2*MARGEN)/3+MARGEN, MARGEN, ANCHO-MARGEN, ALTO-MARGEN-24, !color, DRAW_FILL_EMPTY, DOT_PIXEL_1X1);
        }
      }
      if (m_state == input_cho){
        if (flip){
          Paint_DrawRectangle((ANCHO-2*MARGEN)/3+MARGEN, MARGEN+24, 2*(ANCHO-2*MARGEN)/3+MARGEN, ALTO-MARGEN, !color, DRAW_FILL_EMPTY, DOT_PIXEL_1X1);
        } else {
          Paint_DrawRectangle((ANCHO-2*MARGEN)/3+MARGEN, MARGEN, 2*(ANCHO-2*MARGEN)/3+MARGEN, ALTO-MARGEN-24, !color, DRAW_FILL_EMPTY, DOT_PIXEL_1X1);
        }
      }
      if (m_state == input_ins){
        if (flip){
          Paint_DrawRectangle(2*(ANCHO-2*MARGEN)/3+MARGEN, MARGEN+24, ANCHO-MARGEN, ALTO-MARGEN, !color, DRAW_FILL_EMPTY, DOT_PIXEL_1X1);
        } else {
          Paint_DrawRectangle(MARGEN, MARGEN, (ANCHO-2*MARGEN)/3+MARGEN, ALTO-MARGEN-24, !color, DRAW_FILL_EMPTY, DOT_PIXEL_1X1);
        }
      }
      EPD_DisplayPartWindows(ImageBuff, 0, 0, EPD_WIDTH, EPD_HEIGHT);

      // GLU
      len = sprintf(buffer, " mg/dL");
      Paint_NewImage(ImageBuff, 12, len*7, rotate, color);
      Paint_Clear(color);
      Paint_DrawString_EN(0, 0, buffer, &Font12, color, !color);
      if (flip){
        EPD_DisplayPartWindows(ImageBuff, MARGEN+24+24, 2*(ANCHO-2*MARGEN)/3+MARGEN+24+2-12, MARGEN+24+24+12, 2*(ANCHO-2*MARGEN)/3+MARGEN+24+2+len*7-12);
      } else {
        EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-24-24, MARGEN+24+2, ALTO-MARGEN-24-24+12, MARGEN+24+2+len*7);
      }

      if (config_manager_get_flags() & CONFIG_BOLO_FLAG){
        len = sprintf(buffer, "%d mg/dL", config_manager_get_calc_corr().mantissa);
        Paint_NewImage(ImageBuff, 12, len*7, rotate, color);
        Paint_Clear(color);
        Paint_DrawString_EN(0, 0, buffer, &Font12, color, !color);
        if (flip){
          EPD_DisplayPartWindows(ImageBuff, ALTO-10-12, 204-(len*7)/2, ALTO-10, 204+(len*7)/2);
        } else {
          EPD_DisplayPartWindows(ImageBuff, 10, 46-(len*7)/2, 10+12, 46+(len*7)/2);
        }
      }

      if (new_glu){
        len = sprintf(buffer, "%ld", m_topsulin_meas.glu);
      } else {
        len = sprintf(buffer, " - ");
      }
      Paint_NewImage(ImageBuff, 24, len*17, rotate, color);
      Paint_Clear(color);
      Paint_DrawString_EN(0, 0, buffer, &Font24, color, !color);
      if (flip){
        EPD_DisplayPartWindows(ImageBuff, ALTO+MARGEN-35-24, 204-(len*17)/2, ALTO+MARGEN-35, 204+(len*17)/2);
      } else {
        EPD_DisplayPartWindows(ImageBuff, 35, 46-(len*17)/2, 35+24, 46+(len*17)/2);
      }

      // CHO
      len = sprintf(buffer, " %d g", config_manager_get_cho_interval());
      Paint_NewImage(ImageBuff, 12, len*7, rotate, color);
      Paint_Clear(color);
      Paint_DrawString_EN(0, 0, buffer, &Font12, color, !color);
      if (flip){
        EPD_DisplayPartWindows(ImageBuff, MARGEN+24+24, (ANCHO-2*MARGEN)/3+MARGEN+24+2-12, MARGEN+24+24+12, (ANCHO-2*MARGEN)/3+MARGEN+24+2+len*7-12);
      } else {
        EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-24-24, (ANCHO-2*MARGEN)/3+MARGEN+24+2, ALTO-MARGEN-24-24+12, (ANCHO-2*MARGEN)/3+MARGEN+24+2+len*7);
      }

      if (config_manager_get_flags() & CONFIG_BOLO_FLAG){
        len = sprintf(buffer, "%d g", config_manager_get_calc_sens());
        Paint_NewImage(ImageBuff, 12, len*7, rotate, color);
        Paint_Clear(color);
        Paint_DrawString_EN(0, 0, buffer, &Font12, color, !color);
        if (flip){
          EPD_DisplayPartWindows(ImageBuff, ALTO-10-12, 125-(len*7)/2, ALTO-10, 125+(len*7)/2);
        } else {
          EPD_DisplayPartWindows(ImageBuff, 10, 125-(len*7)/2, 10+12, 125+(len*7)/2);
        }
      }

      if (new_cho){
        len = sprintf(buffer, "%ld", m_topsulin_meas.cho);
      } else {
        len = sprintf(buffer, " - ");
      }
      Paint_NewImage(ImageBuff, 24, len*17, rotate, color);
      Paint_Clear(color);
      Paint_DrawString_EN(0, 0, buffer, &Font24, color, !color);
      if (flip){
        EPD_DisplayPartWindows(ImageBuff, ALTO+MARGEN-35-24, 125-(len*17)/2, ALTO+MARGEN-35, 125+(len*17)/2);
      } else {
        EPD_DisplayPartWindows(ImageBuff, 35, 125-(len*17)/2, 35+24, 125+(len*17)/2);
      }

      // INS
      len = sprintf(buffer, " %d.%d U", config_manager_get_ins_interval() / 10, config_manager_get_ins_interval() % 10);
      Paint_NewImage(ImageBuff, 12, len*7, rotate, color);
      Paint_Clear(color);
      Paint_DrawString_EN(0, 0, buffer, &Font12, color, !color);
      if (flip){
        EPD_DisplayPartWindows(ImageBuff, MARGEN+24+24, MARGEN+24+2-12-7, MARGEN+24+24+12, MARGEN+24+2+len*7-12-7);
      } else {
        EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-24-24, 2*(ANCHO-2*MARGEN)/3+MARGEN+24+2, ALTO-MARGEN-24-24+12, 2*(ANCHO-2*MARGEN)/3+MARGEN+24+2+len*7);
      }

      if (new_ins){
        if (config_manager_get_ins_interval() >= 10){
          len = sprintf(buffer, "%ld", m_topsulin_meas.ins / 10);
        } else {
          len = sprintf(buffer, "%ld.%ld", m_topsulin_meas.ins / 10, m_topsulin_meas.ins % 10);
        }
      } else {
        len = sprintf(buffer, " - ");
      }
      Paint_NewImage(ImageBuff, 24, len*17, rotate, color);
      Paint_Clear(color);
      Paint_DrawString_EN(0, 0, buffer, &Font24, color, !color);
      if (flip){
        EPD_DisplayPartWindows(ImageBuff, ALTO+MARGEN-35-24, 46-(len*17)/2, ALTO+MARGEN-35, 46+(len*17)/2);
      } else {
        EPD_DisplayPartWindows(ImageBuff, 35, 204-(len*17)/2, 35+24, 204+(len*17)/2);
      }

      // GLU ICON
      Paint_NewImage(ImageBuff, 24, 24, rotate, !color);
      Paint_Clear(color);
      if (flip){
        Paint_DrawBitMap(gImage_icon_glu_flip_24);
        EPD_DisplayPartWindows(ImageBuff, 2*MARGEN+24, 204-12+12+12, 2*MARGEN+24+24, 204+12+12+12);
      } else {
        Paint_DrawBitMap(gImage_icon_glu_24);
        EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-24-24, MARGEN+2, ALTO-MARGEN-24, MARGEN+24+2);
      }

      // CHO ICON
      Paint_NewImage(ImageBuff, 24, 24, rotate, !color);
      Paint_Clear(color);
      if (flip){
        Paint_DrawBitMap(gImage_icon_cho_flip_24);
        EPD_DisplayPartWindows(ImageBuff, 2*MARGEN+24, 125-12+12+12, 2*MARGEN+24+24, 125+12+12+12);
      } else {
        Paint_DrawBitMap(gImage_icon_cho_24);
        EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-24-24, (ANCHO-2*MARGEN)/3+MARGEN+2, ALTO-MARGEN-24, (ANCHO-2*MARGEN)/3+MARGEN+24+2);
      }

      // INS ICON
      Paint_NewImage(ImageBuff, 24, 24, rotate, !color);
      Paint_Clear(color);
      if (flip){
        Paint_DrawBitMap(gImage_icon_ins_flip_24);
        EPD_DisplayPartWindows(ImageBuff, 2*MARGEN+24, 46-12+12+12, 2*MARGEN+24+24, 46+12+12+12);
      } else {
        Paint_DrawBitMap(gImage_icon_ins_24);
        EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-24-24, 2*(ANCHO-2*MARGEN)/3+MARGEN+2, ALTO-MARGEN-24, 2*(ANCHO-2*MARGEN)/3+MARGEN+24+2);
      }

    }

    // NAME
    len = config_manager_get_name(buffer);
    Paint_NewImage(ImageBuff, 16, len*11, rotate, color);
    Paint_Clear(color);
    Paint_DrawString_EN(0, 0, buffer, &Font16, color, !color);
    if (flip){
      if (full_refresh){
        EPD_DisplayWindows(ImageBuff, 2*MARGEN, ANCHO-MARGEN-len*11, 2*MARGEN+16, ANCHO-MARGEN);
      } else {
        EPD_DisplayPartWindows(ImageBuff, 2*MARGEN, ANCHO-MARGEN-len*11, 2*MARGEN+16, ANCHO-MARGEN);
      }
    } else {
      if (full_refresh){
        EPD_DisplayWindows(ImageBuff, ALTO-MARGEN-16, MARGEN, ALTO-MARGEN, MARGEN+len*11);
      } else {
        EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-16, MARGEN, ALTO-MARGEN, MARGEN+len*11);
      }
    }

    // BATTERY
    uint16_t voltage;
    voltage = batt_get();
    //voltage = 0;
    if(voltage <= LOW_VOLT){
      Paint_NewImage(ImageBuff, 16, 16, rotate, color);
      if (flip){
        Paint_DrawBitMap(gImage_icon_low_flip_16);
        if (full_refresh) {
          EPD_DisplayWindows(ImageBuff, 2*MARGEN, MARGEN+16*4, 2*MARGEN+16, MARGEN+16*5);
        } else {
          EPD_DisplayPartWindows(ImageBuff, 2*MARGEN, MARGEN+16*4, 2*MARGEN+16, MARGEN+16*5);
        }
      } else {
        Paint_DrawBitMap(gImage_icon_low_16);
        if (full_refresh) {
          EPD_DisplayWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-48-16-16, ALTO-MARGEN, ANCHO-MARGEN-32-16-16);
        } else {
          EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-48-16-16, ALTO-MARGEN, ANCHO-MARGEN-32-16-16);
        }
      }
    }

    // CALC
    if (config_manager_get_flags() & CONFIG_BOLO_FLAG){
      Paint_NewImage(ImageBuff, 16, 16, rotate, !color);
      if (flip){
        Paint_DrawBitMap(gImage_icon_calc_flip_16);
        if (full_refresh) {
          EPD_DisplayWindows(ImageBuff, 2*MARGEN, MARGEN+16*3, 2*MARGEN+16, MARGEN+16*4);
        } else {
          EPD_DisplayPartWindows(ImageBuff, 2*MARGEN, MARGEN+16*3, 2*MARGEN+16, MARGEN+16*4);
        }
      } else {
        Paint_DrawBitMap(gImage_icon_calc_16);
        if (full_refresh) {
          EPD_DisplayWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-48-16, ALTO-MARGEN, ANCHO-MARGEN-32-16);
        } else {
          EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-48-16, ALTO-MARGEN, ANCHO-MARGEN-32-16);
        }
      }
    }

    // MEMORY ICON
    uint16_t num_records = ble_gls_db_num_records_get();
    if(num_records > 0){
      Paint_NewImage(ImageBuff, 16, 16, rotate, color);
      if (flip){
        Paint_DrawBitMap(gImage_icon_mem_flip_16);
        if (full_refresh) {
          EPD_DisplayWindows(ImageBuff, 2*MARGEN, MARGEN+16*2, 2*MARGEN+16, MARGEN+16*3);
        } else {
          EPD_DisplayPartWindows(ImageBuff, 2*MARGEN, MARGEN+16*2, 2*MARGEN+16, MARGEN+16*3);
        }
      } else {
        Paint_DrawBitMap(gImage_icon_mem_16);
        if (full_refresh) {
          EPD_DisplayWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-48, ALTO-MARGEN, ANCHO-MARGEN-32);
        } else {
          EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-48, ALTO-MARGEN, ANCHO-MARGEN-32);
        }
      }
    }

    // BLUETOOTH ON ICON
    if (bt_state == 1 || bt_state == 2){
      Paint_NewImage(ImageBuff, 16, 16, rotate, color);
      if (flip){
        Paint_DrawBitMap(gImage_icon_bt_flip_16);
        if (full_refresh) {
          EPD_DisplayWindows(ImageBuff, 2*MARGEN, MARGEN+16*1, 2*MARGEN+16, MARGEN+16*2);
        } else {
          EPD_DisplayPartWindows(ImageBuff, 2*MARGEN, MARGEN+16*1, 2*MARGEN+16, MARGEN+16*2);
        }
      } else {
        Paint_DrawBitMap(gImage_icon_bt_16);
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-32, ALTO-MARGEN, ANCHO-MARGEN-16);
        } else {
          EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-32, ALTO-MARGEN, ANCHO-MARGEN-16);
        }
      }
    }

    // BLUETOOTH CONNECTED ICON
    if (bt_state == 2){
      Paint_NewImage(ImageBuff, 16, 16, rotate, color);
      if (flip){
        Paint_DrawBitMap(gImage_icon_dev_flip_16);
        if (full_refresh) {
          EPD_DisplayWindows(ImageBuff, 2*MARGEN, MARGEN, 2*MARGEN+16, MARGEN+16);
        } else {
          EPD_DisplayPartWindows(ImageBuff, 2*MARGEN, MARGEN, 2*MARGEN+16, MARGEN+16);
        }
      } else {
        Paint_DrawBitMap(gImage_icon_dev_16);
        if (full_refresh){
          EPD_DisplayWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-16, ALTO-MARGEN, ANCHO-MARGEN);
        } else {
          EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-16, ALTO-MARGEN, ANCHO-MARGEN);
        }
      }
    }

  }

  if ((m_state == initial)&&(quick_refresh|full_refresh)){

    color = WHITE;
    rotate = ROTATE_90;

    if (bt_state == 0){
      Paint_NewImage(ImageBuff, ALTO, ANCHO, rotate, color);
      Paint_Clear(color);
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
      Paint_NewImage(ImageBuff, 16, len*11, rotate, color);
      Paint_Clear(color);
      Paint_DrawString_EN(0, 0, buffer, &Font16, color, !color);
      if (full_refresh){
        EPD_DisplayWindows(ImageBuff, ALTO-MARGEN-16, MARGEN, ALTO-MARGEN, MARGEN+len*11);
      } else {
        EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-16, MARGEN, ALTO-MARGEN, MARGEN+len*11);
      }

      // BLUETOOTH ON ICON
      Paint_NewImage(ImageBuff, 16, 16, rotate, color);
      Paint_DrawBitMap(gImage_icon_bt_16);
      if (full_refresh){
        EPD_DisplayWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-32, ALTO-MARGEN, ANCHO-MARGEN-16);
      } else {
        EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-32, ALTO-MARGEN, ANCHO-MARGEN-16);
      }

      // BLANK
      Paint_NewImage(ImageBuff, 16, 16, rotate, color);
      Paint_Clear(color);
      if (full_refresh){
        EPD_DisplayWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-16, ALTO-MARGEN, ANCHO-MARGEN);
      } else {
        EPD_DisplayPartWindows(ImageBuff, ALTO-MARGEN-16, ANCHO-MARGEN-16, ALTO-MARGEN, ANCHO-MARGEN);
      }

      Paint_NewImage(ImageBuff, 24, 6*17, rotate, color);
      Paint_Clear(color);
      if (full_refresh){
        EPD_DisplayWindows(ImageBuff, 10, 74, 10+24, 74+6*17);
      } else {
        EPD_DisplayPartWindows(ImageBuff, 10, 74, 10+24, 74+6*17);
      }
    }

    if (bt_state == 2){
      // BLUETOOTH CONNECTED ICON
      Paint_NewImage(ImageBuff, 16, 16, rotate, color);
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

  UWORD color;
  color = WHITE;

  UWORD rotate;
  rotate = ROTATE_90;

  Paint_NewImage(ImageBuff, 24, 6*17, rotate, color);
  Paint_Clear(color);
  EPD_DisplayPartWindows(ImageBuff, 10, 74, 10+24, 74+6*17);

  Paint_NewImage(ImageBuff, 24, 6*17, rotate, color);
  Paint_DrawString_EN(0, 0, pin, &Font24, color, !color);
  EPD_DisplayPartWindows(ImageBuff, 10, 74, 10+24, 74+6*17);

  EPD_TurnOnDisplay();
}

void state_show_pin_error(void){
  if (!was_partial){
    EPD_Init(PART_UPDATE);
    was_partial = true;
    was_full = false;
  }

  UWORD color;
  color = WHITE;

  UWORD rotate;
  rotate = ROTATE_90;

  Paint_NewImage(ImageBuff, 24, 6*17, rotate, color);
  Paint_Clear(color);
  EPD_DisplayPartWindows(ImageBuff, 10, 74, 10+24, 74+6*17);

  Paint_NewImage(ImageBuff, 24, 5*17, rotate, color);
  Paint_DrawString_EN(0, 0, "ERROR", &Font24, color, !color);
  EPD_DisplayPartWindows(ImageBuff, 10, 82, 10+24, 82+5*17);

  EPD_TurnOnDisplay();
}

void state_show_pin_ok(void){
  if (!was_partial){
    EPD_Init(PART_UPDATE);
    was_partial = true;
    was_full = false;
  }

  UWORD color;
  color = WHITE;

  UWORD rotate;
  rotate = ROTATE_90;

  Paint_NewImage(ImageBuff, 24, 6*17, rotate, color);
  Paint_Clear(color);
  EPD_DisplayPartWindows(ImageBuff, 10, 74, 10+24, 74+6*17);

  Paint_NewImage(ImageBuff, 24, 2*17, rotate, color);
  Paint_DrawString_EN(0, 0, "OK", &Font24, color, !color);
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
