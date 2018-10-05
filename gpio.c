#include "gpio.h"

#include <stdint.h>

#include "nrf.h"
#include "nrf_nvic.h"

#define NRF_LOG_MODULE_NAME "GIO"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
  
#define GPIOTE_PRIORITY 1

#define BTN_PIN 9

//static volatile uint8_t* m_gpio_button_flag;

void gpio_init() {
    NRF_LOG_INFO("Module init.\r\n");
    NRF_GPIO->PIN_CNF[BTN_PIN] = ((GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) | (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos) | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) | (GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos));
    NRF_LOG_INFO("Module init done.\r\n");
    NRF_LOG_FLUSH();
}

void gpio_button_set_flag(volatile uint8_t* main_button_flag)
{
    //m_gpio_button_flag = main_button_flag;
}

void gpio_read(void) {
    
};