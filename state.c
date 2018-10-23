#include "state.h"

#include <stdint.h>
#include <stdio.h>

#include "encoder.h"
#include "display_SSD1306.h"
#include "ble_services.h"

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
        m_state = input_cho;
      }
      break;
    case input_cho:
      if (event == button_pressed){
        m_state = input_glu;
      }
      break;
    case input_glu:
      if (event == button_pressed){
        m_state = input_ins;
      }
      break;
    case input_ins:
      if (event == button_pressed){
        read_glucose_measurement();
        m_state = off;
      }
      break;
  }
  state_process_display();
}

void state_disable()
{
  m_state = off;
  state_process_display();
}
