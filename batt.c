#include "batt.h"

#include "nrf_drv_adc.h"

#define NRF_LOG_MODULE_NAME "BATT"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define DBG_IN_MV   1200
#define ADC_MAX     255

static volatile uint8_t* m_batt_flag;
static uint32_t voltage;

void batt_set_flag(volatile uint8_t* main_batt_flag)
{
    m_batt_flag = main_batt_flag;
}

static void adc_event_handler(nrf_drv_adc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_ADC_EVT_SAMPLE)
    {
        voltage = (p_event->data.sample.sample * 3 * DBG_IN_MV) / ADC_MAX;
        *m_batt_flag = 1;
    }
}

void batt_init(void)
{
  voltage = 0;
  ret_code_t ret_code;
  nrf_drv_adc_config_t config = NRF_DRV_ADC_DEFAULT_CONFIG;
  ret_code = nrf_drv_adc_init(&config, adc_event_handler);
  APP_ERROR_CHECK(ret_code);
}

void batt_sample(void)
{
    nrf_drv_adc_channel_t m_channel_config =
    {{{
      .resolution = NRF_ADC_CONFIG_RES_8BIT,
      .input      = NRF_ADC_CONFIG_SCALING_SUPPLY_ONE_THIRD,
      .reference  = NRF_ADC_CONFIG_REF_VBG,
      .ain        = NRF_ADC_CONFIG_INPUT_DISABLED
    }}, NULL};

    nrf_drv_adc_sample_convert(&m_channel_config, NULL);
}

uint32_t batt_get(void)
{
  uint32_t aux;
  CRITICAL_REGION_ENTER();
  aux = voltage;
  CRITICAL_REGION_EXIT();
  return aux;
}
