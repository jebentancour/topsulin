#include "state.h"

#include <stdio.h>

#include "encoder.h"
#include "display_SSD1306.h"
#include "ble_services.h"
#include "clock.h"

typedef enum {
    off,
    input_cho,
    input_glu,
    input_ins
} internal_state_t;

static internal_state_t       m_state;
static char                   encoder[32];
static uint8_t                len;

void state_init()
{
  m_state = off;
}

void state_process_display(void)
{
  display_clear();
  switch(m_state){
    case off:
      display_put_string("TOPsulin", 35, 0);
      display_put_string("------------", 20, 1);
      display_put_string("Take control", 20, 2);
      break;
    case input_cho:
      display_put_string("Carbohidratos", 0, 0);
      len = sprintf(encoder, "%li g", encoder_get_position());
      display_put_buff(encoder, len, 20, 2);
      break;
    case input_glu:
      display_put_string("Glucemia", 0, 0);
      len = sprintf(encoder, "%li mmol/l", encoder_get_position());
      display_put_buff(encoder, len, 30, 2);
      break;
    case input_ins:
      display_put_string("Insulina", 0, 0);
      len = sprintf(encoder, "%li U", encoder_get_position());
      display_put_buff(encoder, len, 20, 2);
      break;
  }
  display_show();
}

void state_on_event(event_t event)
{
  switch(m_state){
    case off:
      if (event == button_pressed){
        encoder_reset_position();
        m_state = input_cho;
      }
      break;
    case input_cho:
      if (event == button_pressed){
        encoder_reset_position();
        m_state = input_glu;
      }
      break;
    case input_glu:
      if (event == button_pressed){
        encoder_reset_position();
        m_state = input_ins;
      }
      break;
    case input_ins:
      if (event == button_pressed){
        ble_gls_rec_t rec;

        static int16_t s_mantissa = 550;
        static int16_t s_exponent = -3;

        uint32_t  ms = clock_get_timestamp();
        uint8_t   s = (ms / 1000) % 60;
        uint8_t   m = (ms / 60000) % 60;
        uint8_t   h = ms / 3600000;

        // simulate the reading of a glucose measurement.
        rec.meas.flags = BLE_GLS_MEAS_FLAG_TIME_OFFSET |
                         BLE_GLS_MEAS_FLAG_CONC_TYPE_LOC |
                         BLE_GLS_MEAS_FLAG_CONTEXT_INFO |
                         BLE_GLS_MEAS_FLAG_SENSOR_STATUS |
                         BLE_GLS_MEAS_FLAG_UNITS_MOL_L;
        rec.meas.base_time.year                 = 2018;
        rec.meas.base_time.month                = 10;
        rec.meas.base_time.day                  = 1;
        rec.meas.base_time.hours                = h;
        rec.meas.base_time.minutes              = m;
        rec.meas.base_time.seconds              = s;
        rec.meas.glucose_concentration.exponent = s_exponent;
        rec.meas.glucose_concentration.mantissa = s_mantissa;
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

        // change values for next read.
        s_mantissa += 23;
        if (s_mantissa > 939)
        {
            s_mantissa -= 434;
        }

        add_glucose_measurement(rec);
        m_state = off;
      }
      break;
  }
  state_process_display();
}

void state_off()
{
  m_state = off;
}
