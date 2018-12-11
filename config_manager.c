#include "config_manager.h"

#include "ble_gls.h"
#include "clock.h"

#define NRF_LOG_MODULE_NAME "CONF"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"


static global_conf_t m_global_conf;

void config_manager_init(void)
{
  memset(&m_global_conf, 0, sizeof(m_global_conf));
  m_global_conf.flags = CONFIG_CHO_FLAG | CONFIG_INSULIN_FLAG | CONFIG_GLUCOSE_FLAG;
  m_global_conf.g_portion = 250;
  m_global_conf.insulin_type = BLE_GLS_CONTEXT_MED_RAPID;
  m_global_conf.insulin_total = 300;
  m_global_conf.insulin_remaining = 120;
  const char* tmp = "Glucosee";
  strcpy(m_global_conf.name, tmp);
}

void config_manager_print(void)
{
  // Config
  uint8_t flags = m_global_conf.flags;
  NRF_LOG_INFO("GLU 0x%02X\n",flags & CONFIG_GLUCOSE_FLAG);
  NRF_LOG_INFO("INS 0x%02X\n",flags & CONFIG_INSULIN_FLAG);
  NRF_LOG_INFO("CHO 0x%02X\n",flags & CONFIG_CHO_FLAG);
  if(flags & CONFIG_COLOR_FLAG){
    NRF_LOG_INFO("FDO BLANCO\n");
  } else {
    NRF_LOG_INFO("FDO NEGRO\n");
  }
  if(flags & CONFIG_UNITS_FLAG){
    NRF_LOG_INFO("GLU mol/L\n");
  } else {
    NRF_LOG_INFO("GLU kg/L\n");
  }
  NRF_LOG_INFO("CAL 0x%02X\n",flags & CONFIG_BOLO_FLAG);
  if(flags & CONFIG_FLIP_FLAG){
    NRF_LOG_INFO("DIR ZURDO\n");
  } else {
    NRF_LOG_INFO("DIR DIESTRO\n");
  }
  if(flags & CONFIG_PORTION_FLAG){
    NRF_LOG_INFO("CHO en PORCIONES (%d g/porcion)\n", m_global_conf.g_portion);
  } else {
    NRF_LOG_INFO("CHO en g\n");
  }
  NRF_LOG_FLUSH();

  // Name
  NRF_LOG_INFO("NME %s\n", (uint32_t)m_global_conf.name);
  NRF_LOG_FLUSH();

  // Time
  struct tm t;
  clock_get_time(&t);
  char time_buffer[80];
  strftime(time_buffer, sizeof(time_buffer), "%x %X", &t);
  NRF_LOG_INFO("DTM %s\n", (uint32_t)time_buffer);
  NRF_LOG_FLUSH();

  // Calculator

  // Insulin
  NRF_LOG_INFO("INS type  0x%02X\n", m_global_conf.insulin_type);
  NRF_LOG_INFO("INS total %d\n", m_global_conf.insulin_total);
  NRF_LOG_INFO("INS rem   %d\n", m_global_conf.insulin_remaining);
  NRF_LOG_FLUSH();
}

void config_manager_set_flags(uint8_t flags)
{
  m_global_conf.flags = flags;
}

uint8_t config_manager_get_flags(void)
{
  return m_global_conf.flags;
}

void config_manager_set_portion(uint16_t portion)
{
  m_global_conf.g_portion = portion;
}

uint16_t config_manager_get_portion(void)
{
  return m_global_conf.g_portion;
}

void config_manager_set_insulin_type(uint8_t type)
{
  m_global_conf.insulin_type = type;
}

uint8_t config_manager_get_insulin_type(void)
{
  return m_global_conf.insulin_type;
}

void config_manager_set_insulin_total(uint16_t total)
{
  m_global_conf.insulin_total = total;
}

uint16_t config_manager_get_insulin_total(void)
{
  return m_global_conf.insulin_total;
}

void config_manager_set_insulin_remaining(uint16_t rem)
{
  m_global_conf.insulin_remaining = rem;
}

uint16_t config_manager_get_insulin_remaining(void)
{
  return m_global_conf.insulin_remaining;
}
