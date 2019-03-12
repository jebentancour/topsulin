#include "encoder.h"

#include "nrf_drv_qdec.h"
#include "nrf_gpio.h"

#define NRF_LOG_MODULE_NAME "ENCODER"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define A_PIN 1
#define B_PIN 0

static volatile int32_t m_accdblread;
static volatile int32_t m_accread;
static volatile int32_t m_prev_accread;
static volatile int32_t m_vel;
static volatile int32_t m_position;
static volatile int32_t m_report_position;
static volatile uint8_t* m_encoder_flag;
static volatile bool m_direction;
static volatile bool m_play;
static volatile bool m_direction_change;


static void qdec_event_handler(nrf_drv_qdec_event_t event)
{
    if (event.type == NRF_QDEC_EVENT_REPORTRDY)
    {
        m_accdblread        = event.data.report.accdbl;
        m_accread           = event.data.report.acc;

        m_vel = m_accread - m_prev_accread;
        if (m_vel == 0){
          m_vel = 1;
        }
        if (m_vel < 0){
          m_vel = - m_vel;
        }

        m_direction_change  = (m_prev_accread * m_accread) < 0;

        m_prev_accread = m_accread;

        if (m_play & !m_direction_change){
          if (m_direction) {
            m_position          -= m_vel * m_accread;
          } else {
            m_position          += m_vel * m_accread;
          }
          if (!(m_accdblread > 0)){
            if (m_report_position != (m_position / 4)) {
              m_report_position   = m_position / 4;
              //NRF_LOG_INFO("m_position %d\n", m_position);
              //NRF_LOG_INFO("m_report_position %d\n", m_report_position);
              //NRF_LOG_FLUSH();
              *m_encoder_flag     = 1;
            }
          }
        }
    }
}

void encoder_set_direction(bool direction)
{
  m_direction = direction;
}

void encoder_init(void)
{
  m_position = 0;
  m_report_position = 0;
  m_play = true;
  m_direction_change = false;

  uint32_t err_code;

  nrf_drv_qdec_config_t qdec_cfg =
  {
      .reportper = NRF_QDEC_REPORTPER_120,                    /**< Report period in samples. */
      .sampleper = NRF_QDEC_SAMPLEPER_128us,                  /**< Sampling period in microseconds. */
      .psela = A_PIN,                                         /**< Pin number for A input. */
      .pselb = B_PIN,                                         /**< Pin number for B input. */
      .pselled = 0xFFFFFFFF,                                  /**< Pin number for LED output. */
      .ledpre = 0,                                            /**< Time (in microseconds) how long LED is switched on before sampling. */
      .ledpol = 0,                                            /**< Active LED polarity. */
      .dbfen = true,                                          /**< State of debouncing filter. */
      .sample_inten = false,                                  /**< Enabling sample ready interrupt. */
      .interrupt_priority = 3,                                /**< QDEC interrupt priority. */
  };

  err_code = nrf_drv_qdec_init(&qdec_cfg, qdec_event_handler);
  APP_ERROR_CHECK(err_code);

  nrf_gpio_cfg_input(qdec_cfg.pselb,NRF_GPIO_PIN_PULLUP);     /**< Needed when you don't have external pullups*/
  nrf_gpio_cfg_input(qdec_cfg.psela,NRF_GPIO_PIN_PULLUP);     /**< Needed when you don't have external pullups*/

  nrf_drv_qdec_enable();
}

void encoder_enable(void)
{
  nrf_drv_qdec_enable();
}

void encoder_disable(void)
{
  nrf_drv_qdec_disable();
}

void encoder_set_flag(volatile uint8_t* main_encoder_flag)
{
  m_encoder_flag = main_encoder_flag;
}

int32_t encoder_get_position(void)
{
  int32_t aux;
  CRITICAL_REGION_ENTER();
  aux = m_report_position;
  CRITICAL_REGION_EXIT();
  return aux;
}

void encoder_set_position(int32_t new_position)
{
  CRITICAL_REGION_ENTER();
  m_position = new_position * 4;
  m_report_position = new_position;
  CRITICAL_REGION_EXIT();
}

void encoder_reset_position(void)
{
  CRITICAL_REGION_ENTER();
  m_position = 0;
  m_report_position = 0;
  CRITICAL_REGION_EXIT();
}

void encoder_pause(void)
{
  CRITICAL_REGION_ENTER();
  m_play = false;
  CRITICAL_REGION_EXIT();
}

void encoder_play(void)
{
  CRITICAL_REGION_ENTER();
  m_play = true;
  CRITICAL_REGION_EXIT();
}
