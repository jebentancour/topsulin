#include "i2c.h"

#include <stdint.h>

#include "nrf.h"
#include "nrf_nvic.h"

#define NRF_LOG_MODULE_NAME "I2C"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define I2C_PRIORITY            1

#define SCL_PIN 03
#define SDA_PIN 05

static volatile uint8_t*        m_tx_flag;     /**< Bandera que indica que un byte fue enviado. */

void SPI0_TWI0_IRQHandler(void)
{
    *m_tx_flag = 1;                             /* Aviso incluso si hay error. */
    if (NRF_TWI0->EVENTS_TXDSENT != 0)
    {
        NRF_TWI0->EVENTS_TXDSENT = 0;           /* Event clear. */
    }
    if (NRF_TWI0->EVENTS_ERROR != 0)
    {
        NRF_LOG_INFO("TX Error.\r\n");
        NRF_TWI0->ERRORSRC = 0x07;              /* Errors clear. */
        NRF_TWI0->EVENTS_ERROR = 0;             /* Event clear. */
    }
}

static void twi_conf(void){
    NRF_TWI0->INTENSET       = TWI_INTENSET_TXDSENT_Msk;        /* Enable interrupt on TXDSENT event. */
    NRF_TWI0->INTENSET       = TWI_INTENSET_ERROR_Msk;          /* Enable interrupt on ERROR event. */    
    NRF_TWI0->FREQUENCY      = TWI_FREQUENCY_FREQUENCY_K400;    /* 400 kbps. */
    NRF_TWI0->PSELSCL        = SCL_PIN;                         /* Pin select for SCL. */ 
    NRF_TWI0->PSELSDA        = SDA_PIN;                         /* Pin select for SDA. */
    NRF_TWI0->EVENTS_TXDSENT = 0;                               /* Event clear. */
    NRF_TWI0->EVENTS_ERROR   = 0;                               /* Event clear. */
    NRF_TWI0->POWER          = 1;                               /* Peripheral power control. */
    NRF_TWI0->ENABLE         = TWI_ENABLE_ENABLE_Enabled;       /* Enable two-wire master. */
}

void i2c_init(){
    NRF_LOG_INFO("Module init.\r\n");
    twi_conf();                                         /* Configuro el periferico TWI. */
    sd_nvic_SetPriority(SPI0_TWI0_IRQn, I2C_PRIORITY);  /* Seteo la prioridad de la interrupcion. */
    sd_nvic_EnableIRQ(SPI0_TWI0_IRQn);                  /* Habilito la interrupcion. */
    NRF_LOG_INFO("Module init done.\r\n");
    NRF_LOG_FLUSH();
}

void i2c_tx_set_flag(volatile uint8_t* main_tx_flag)
{
    m_tx_flag = main_tx_flag;
}

void i2c_begin_transmission(uint8_t address)
{
    NRF_TWI0->ADDRESS = address;        /* Address used in the two-wire transfer. */
}

void i2c_write(uint8_t value)
{
    NRF_TWI0->TXD = value;              /* TX data register. */
    NRF_TWI0->TASKS_STARTTX = 1;        /* Start 2-Wire master transmit sequence. */
}

void i2c_end_transmission(void)
{
    NRF_TWI0->TASKS_STOP = 1;           /* Stop 2-Wire transaction. */
}