#include "config_manager.h"

#include "ble_gls.h"

#define NRF_LOG_MODULE_NAME "CONF"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"


static global_conf_t m_global_conf;

void config_manager_init(void)
{
  m_global_conf.flags = CONFIG_CHO_FLAG | CONFIG_INSULIN_FLAG | CONFIG_GLUCOSE_FLAG;
  m_global_conf.g_portion = 250;
  m_global_conf.insulin_type = BLE_GLS_CONTEXT_MED_RAPID;
  m_global_conf.insulin_total = 300;
  m_global_conf.insulin_remaining = 300;
}

void config_manager_print(void)
{
  uint8_t flags = m_global_conf.flags;
  NRF_LOG_INFO("Glucemia\t\t\t\t\t\t0x%02X\n",flags & CONFIG_GLUCOSE_FLAG);
  NRF_LOG_INFO("Insulina\t\t\t\t\t\t0x%02X\n",flags & CONFIG_INSULIN_FLAG);
  NRF_LOG_INFO("Carbohidratos\t\t\t\t\t0x%02X\n",flags & CONFIG_CHO_FLAG);
  NRF_LOG_INFO("Invertir colores\t\t\t\t\t0x%02X\n",flags & CONFIG_COLOR_FLAG);
  NRF_LOG_INFO("Unidades glicemia\t\t\t\t\t0x%02X\n",flags & CONFIG_UNITS_FLAG);
  NRF_LOG_INFO("Calculador de bolo\t\t\t\t0x%02X\n",flags & CONFIG_BOLO_FLAG);
  NRF_LOG_INFO("Orientacion\t\t\t\t\t\t0x%02X\n",flags & CONFIG_FLIP_FLAG);
  NRF_LOG_INFO("CHO en porciones\t\t\t\t\t0x%02X\n",flags & CONFIG_PORTION_FLAG);
  NRF_LOG_INFO("1 porcion = %d g\n", m_global_conf.g_portion);
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
