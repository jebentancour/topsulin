#include "config_manager.h"

#include "clock.h"

#define NRF_LOG_MODULE_NAME "CONF"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "encoder.h"
#include "FEPD_2in13.h"
#include "GUI_Paint.h"

#define VERSION 1
#define SN      1

static global_conf_t m_global_conf;

void config_manager_init(void)
{
  memset(&m_global_conf, 0, sizeof(m_global_conf));
  // Config
  m_global_conf.flags = CONFIG_COLOR_FLAG | CONFIG_CHO_FLAG | CONFIG_INSULIN_FLAG | CONFIG_GLUCOSE_FLAG;
  //m_global_conf.flags |= CONFIG_FLIP_FLAG;
  m_global_conf.g_portion = 250;
  // Name
  const char* tmp = "Topsulin";
  strcpy(m_global_conf.name, tmp);
  // Calc
  m_global_conf.calc_low.mantissa = 90;
  m_global_conf.calc_low.exponent = -3;
  m_global_conf.calc_high.mantissa = 150;
  m_global_conf.calc_high.exponent = -3;
  m_global_conf.calc_sens = 15;
  m_global_conf.calc_corr.mantissa = 50;
  m_global_conf.calc_corr.exponent = -3;
  // Insulin
  m_global_conf.insulin_type = BLE_GLS_CONTEXT_MED_RAPID;
  m_global_conf.insulin_total = 300;
  m_global_conf.insulin_remaining = 120;
  m_global_conf.insulin_start = 30;     // 30 min
  m_global_conf.insulin_max = 120;      // 2 h
  m_global_conf.insulin_duration = 360; // 6 h

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
  NRF_LOG_INFO("CALC LOW  %dx10^%d kg/L\n", m_global_conf.calc_low.mantissa, m_global_conf.calc_low.exponent);
  NRF_LOG_INFO("CALC HIGH %dx10^%d kg/L\n", m_global_conf.calc_high.mantissa, m_global_conf.calc_high.exponent);
  NRF_LOG_INFO("CALC SENS %d g\n", m_global_conf.calc_sens);
  NRF_LOG_INFO("CALC CORR %dx10^%d kg/L\n", m_global_conf.calc_corr.mantissa, m_global_conf.calc_corr.exponent);
  NRF_LOG_FLUSH();

  // Insulin
  NRF_LOG_INFO("INS type  0x%02X\n", m_global_conf.insulin_type);
  NRF_LOG_INFO("INS total %d U\n", m_global_conf.insulin_total);
  NRF_LOG_INFO("INS rem   %d U\n", m_global_conf.insulin_remaining);
  NRF_LOG_INFO("INS start %d min\n", m_global_conf.insulin_start);
  NRF_LOG_INFO("INS max   %d min\n", m_global_conf.insulin_max);
  NRF_LOG_INFO("INS dur   %d min\n", m_global_conf.insulin_duration);
  NRF_LOG_FLUSH();

  // Device
  NRF_LOG_INFO("DEV v %d\n", VERSION);
  NRF_LOG_INFO("DEV SN %d\n", SN);
  NRF_LOG_FLUSH();
}

void config_manager_set_flags(uint8_t flags)
{
  m_global_conf.flags = flags;
  /*encoder_set_direction((config_manager_get_flags() & CONFIG_FLIP_FLAG) == 0);
  if(flags & CONFIG_COLOR_FLAG){
    if (flags & CONFIG_FLIP_FLAG){
      GUI_NewImage(EPD_WIDTH, EPD_HEIGHT, IMAGE_ROTATE_180, IMAGE_COLOR_POSITIVE);
    } else {
      GUI_NewImage(EPD_WIDTH, EPD_HEIGHT, IMAGE_ROTATE_0, IMAGE_COLOR_POSITIVE);
    }
  } else {
    if (flags & CONFIG_FLIP_FLAG){
      GUI_NewImage(EPD_WIDTH, EPD_HEIGHT, IMAGE_ROTATE_180, IMAGE_COLOR_INVERTED);
    } else {
      GUI_NewImage(EPD_WIDTH, EPD_HEIGHT, IMAGE_ROTATE_0, IMAGE_COLOR_INVERTED);
    }
  }*/
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

void config_manager_set_insulin_start(uint16_t start)
{
  m_global_conf.insulin_start = start;
}

uint16_t config_manager_get_insulin_start(void)
{
  return m_global_conf.insulin_start;
}

void config_manager_set_insulin_max(uint16_t max)
{
  m_global_conf.insulin_max = max;
}

uint16_t config_manager_get_insulin_max(void)
{
  return m_global_conf.insulin_max;
}

void config_manager_set_insulin_duration(uint16_t d)
{
  m_global_conf.insulin_duration = d;
}

uint16_t config_manager_get_insulin_duration(void)
{
  return m_global_conf.insulin_duration;
}

void config_manager_set_name(uint8_t* data, uint8_t data_len)
{
  uint8_t i;
  for(i = 0; i < 20; i++){
    if(i < data_len){
      m_global_conf.name[i] = (char)data[i];
    } else {
      m_global_conf.name[i] = '\0';
    }
  }
}

sfloat_t config_manager_get_calc_low(void)
{
  return m_global_conf.calc_low;
}

void config_manager_set_calc_low(sfloat_t low)
{
  m_global_conf.calc_low = low;
}

sfloat_t config_manager_get_calc_high(void)
{
  return m_global_conf.calc_high;
}

void config_manager_set_calc_high(sfloat_t high)
{
  m_global_conf.calc_high = high;
}

uint16_t config_manager_get_calc_sens(void)
{
  return m_global_conf.calc_sens;
}

void config_manager_set_calc_sens(uint16_t s)
{
  m_global_conf.calc_sens = s;
}

sfloat_t config_manager_get_calc_corr(void)
{
  return m_global_conf.calc_corr;
}

void config_manager_set_calc_corr(sfloat_t corr)
{
  m_global_conf.calc_corr = corr;
}

uint8_t config_manager_get_version()
{
  return VERSION;
}

uint16_t config_manager_get_serial_number()
{
  return SN;
}
