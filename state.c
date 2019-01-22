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
    uint32_t glu;
    uint32_t glu_correction;
    uint32_t cho;
    uint32_t cho_correction;
    uint32_t ins;
} topsulin_meas_t;

static internal_state_t       m_state;
static internal_state_t       last_state;
static topsulin_meas_t        m_topsulin_meas;
static char                   buffer[8];
static uint8_t                len;
static int                    encoder_pos;
static uint8_t                refresh;

void state_init()
{
  m_state = initial;
  last_state = initial;
  memset(&m_topsulin_meas, 0, sizeof(m_topsulin_meas));
  refresh = 0;
}

void state_process_display(void)
{
  uint8_t change;
  if (last_state != m_state){
    change = 1;
    NRF_LOG_INFO("Display change\n");
    NRF_LOG_FLUSH();
  } else {
    change = 0;
  }
  switch(m_state){
    case initial:
      break;
    case sleep:
      if (change){
        GUI_Clear(WHITE);

        GUI_DrawIcon(7, 19, gImage_icon_glu, WHITE);
        GUI_DrawString_EN(11, 26, "105", &Font24, WHITE, BLACK);
        GUI_DrawString_EN(9, 10, "08:05", &Font16, WHITE, BLACK);

        GUI_DrawIcon(7, 90, gImage_icon_cho, WHITE);
        GUI_DrawString_EN(91, 26, "15", &Font24, WHITE, BLACK);
        GUI_DrawString_EN(79, 10, "10:20", &Font16, WHITE, BLACK);

        GUI_DrawIcon(7, 161, gImage_icon_ins, WHITE);
        GUI_DrawString_EN(162, 26, "02", &Font24, WHITE, BLACK);
        GUI_DrawString_EN(149, 10, "08:05", &Font16, WHITE, BLACK);

        EPD_DisplayFull();
      }
      break;
    case sel_glu:
      if (change){
        GUI_Clear(WHITE);

        GUI_DrawRectangle(1, 1, 104, 70, BLACK, DRAW_FILL_FULL, DOT_PIXEL_DFT);
        GUI_DrawIcon(7, 19, gImage_icon_glu, BLACK);
        GUI_DrawString_EN(11, 26, "105", &Font24, BLACK, WHITE);
        GUI_DrawString_EN(9, 10, "08:05", &Font16, BLACK, WHITE);

        GUI_DrawIcon(7, 90, gImage_icon_cho, WHITE);
        GUI_DrawString_EN(91, 26, "15", &Font24, WHITE, BLACK);
        GUI_DrawString_EN(79, 10, "10:20", &Font16, WHITE, BLACK);

        GUI_DrawIcon(7, 161, gImage_icon_ins, WHITE);
        GUI_DrawString_EN(162, 26, "02", &Font24, WHITE, BLACK);
        GUI_DrawString_EN(149, 10, "08:05", &Font16, WHITE, BLACK);

        EPD_DisplayPartial(1, 1, 104, 212);
        //EPD_DisplayFull();
      }
      break;
    case sel_cho:
      if (change){
        GUI_Clear(WHITE);

        GUI_DrawIcon(7, 19, gImage_icon_glu, WHITE);
        GUI_DrawString_EN(11, 26, "105", &Font24, WHITE, BLACK);
        GUI_DrawString_EN(9, 10, "08:05", &Font16, WHITE, BLACK);

        GUI_DrawRectangle(1, 71, 104, 140, BLACK, DRAW_FILL_FULL, DOT_PIXEL_DFT);
        GUI_DrawIcon(7, 90, gImage_icon_cho, BLACK);
        GUI_DrawString_EN(91, 26, "15", &Font24, BLACK, WHITE);
        GUI_DrawString_EN(79, 10, "10:20", &Font16, BLACK, WHITE);

        GUI_DrawIcon(7, 161, gImage_icon_ins, WHITE);
        GUI_DrawString_EN(162, 26, "02", &Font24, WHITE, BLACK);
        GUI_DrawString_EN(149, 10, "08:05", &Font16, WHITE, BLACK);

        EPD_DisplayPartial(1, 1, 104, 212);
        //EPD_DisplayFull();
      }
      break;
    case sel_ins:
      if (change){
        GUI_Clear(WHITE);

        GUI_DrawIcon(7, 19, gImage_icon_glu, WHITE);
        GUI_DrawString_EN(11, 26, "105", &Font24, WHITE, BLACK);
        GUI_DrawString_EN(9, 10, "08:05", &Font16, WHITE, BLACK);

        GUI_DrawIcon(7, 90, gImage_icon_cho, WHITE);
        GUI_DrawString_EN(91, 26, "15", &Font24, WHITE, BLACK);
        GUI_DrawString_EN(79, 10, "10:20", &Font16, WHITE, BLACK);

        GUI_DrawRectangle(1, 141, 104, 212, BLACK, DRAW_FILL_FULL, DOT_PIXEL_DFT);
        GUI_DrawIcon(7, 161, gImage_icon_ins, BLACK);
        GUI_DrawString_EN(162, 26, "02", &Font24, BLACK, WHITE);
        GUI_DrawString_EN(149, 10, "08:05", &Font16, BLACK, WHITE);

        EPD_DisplayPartial(1, 1, 104, 212);
        //EPD_DisplayFull();
      }
      break;
    case input_glu:
      if(change || refresh){
        GUI_Clear(WHITE);

        GUI_DrawRectangle(1, 1, 104, 140, BLACK, DRAW_FILL_FULL, DOT_PIXEL_DFT);
        GUI_DrawIcon(7, 90, gImage_icon_glu, BLACK);
        NRF_LOG_INFO("Glu %ld\n", m_topsulin_meas.glu);
        NRF_LOG_FLUSH();
        len = sprintf(buffer, "%ld", m_topsulin_meas.glu);
        buffer[len] = '\0';
        GUI_DrawString_EN(11, 56, buffer, &Font24, BLACK, WHITE);
        GUI_DrawString_EN(9, 10, "12:10", &Font16, BLACK, WHITE);

        GUI_DrawIcon(7, 161, gImage_icon_ins, WHITE);
        GUI_DrawString_EN(162, 26, "01", &Font24, WHITE, BLACK);
      }
      if (change){
        EPD_DisplayFull();
      } else {
        if (refresh){
          EPD_DisplayPartial(1, 1, 104, 212);
        }
      }
      break;
    case input_cho:
      if (change){
        GUI_Clear(WHITE);

        GUI_DrawRectangle(1, 1, 104, 140, BLACK, DRAW_FILL_FULL, DOT_PIXEL_DFT);
        GUI_DrawIcon(7, 90, gImage_icon_cho, BLACK);
        GUI_DrawString_EN(21, 56, "60", &Font24, BLACK, WHITE);
        GUI_DrawString_EN(9, 10, "12:10", &Font16, BLACK, WHITE);

        GUI_DrawIcon(7, 161, gImage_icon_ins, WHITE);
        GUI_DrawString_EN(162, 26, "04", &Font24, WHITE, BLACK);

        //EPD_DisplayPartial(1, 1, 104, 212);
        EPD_DisplayFull();
      }
      break;
    case input_ins:
      if (change){
        GUI_Clear(WHITE);

        GUI_DrawRectangle(1, 1, 104, 140, BLACK, DRAW_FILL_FULL, DOT_PIXEL_DFT);
        GUI_DrawIcon(7, 90, gImage_icon_ins, BLACK);
        GUI_DrawString_EN(21, 56, "05", &Font24, BLACK, WHITE);
        GUI_DrawString_EN(9, 10, "12:10", &Font16, BLACK, WHITE);

        GUI_DrawString_EN(162, 50, "01", &Font24, WHITE, BLACK);
        GUI_DrawString_EN(162, 26, "04", &Font24, WHITE, BLACK);

        //EPD_DisplayPartial(1, 1, 104, 212);
        EPD_DisplayFull();
      }
      break;
    default:
      break;
  }
  last_state = m_state;
  refresh = 0;
}

void state_on_event(event_t event)
{
  switch(m_state){
    case initial:
      if (event == ble_on){
        //GUI_DrawBitMap(gImage_IMAGE_0);
        GUI_DrawIcon(7, 1, gImage_icon_bt, WHITE);
        EPD_DisplayPartial(1, 1, 104, 212);
      }
      break;
    case sleep:
      if (event == button_pressed){
        encoder_reset_position();
        m_state = sel_glu;
      }
      break;
    case sel_glu:
      if (event == button_pressed){
        m_state = input_glu;
      }
      if (event == encoder_update){
        encoder_pos = encoder_get_position();
        if (encoder_pos > 0){
          m_state = sel_cho;
        }
        encoder_reset_position();
      }
      break;
    case sel_cho:
      if (event == button_pressed){
        m_state = input_cho;
      }
      if (event == encoder_update){
        encoder_pos = encoder_get_position();
        if (encoder_pos > 0){
          m_state = sel_ins;
        }
        if (encoder_pos < 0) {
          m_state = sel_glu;
        }
        encoder_reset_position();
      }
      break;
    case sel_ins:
      if (event == button_pressed){
        m_state = input_ins;
      }
      if (event == encoder_update){
        encoder_pos = encoder_get_position();
        if (encoder_pos < 0){
          m_state = sel_cho;
        }
        encoder_reset_position();
      }
      break;
    case input_glu:
      if (event == button_pressed){
        m_state = sel_cho;
      }
      if (event == encoder_update){
        m_topsulin_meas.glu = encoder_get_position();
        refresh = 1;
      }
      break;
    case input_cho:
      if (event == button_pressed){
        m_state = sel_ins;
      }
      break;
    case input_ins:
      if (event == button_pressed){
        m_state = sleep;
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
    GUI_DrawIcon(1, 1, gImage_icon_lock, WHITE);
    GUI_DrawString_EN(40, 8, pin, &Font24, WHITE, BLACK);
    EPD_DisplayFull();
  }
}

void state_begin(){
  if (m_state == initial){
    m_state = sleep;
    state_process_display();
  }
}

void state_sleep(){
  if (m_state == initial){
    NRF_LOG_INFO("Going to sleep in initial state\n");
    GUI_DrawBitMap(gImage_IMAGE_0);
    EPD_DisplayFull();
  } else {
    m_state = sleep;
    NRF_LOG_INFO("Going to sleep\n");
    state_process_display();
  }
  NRF_LOG_FLUSH();
}
