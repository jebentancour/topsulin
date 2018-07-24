/** @file
 * @defgroup tw_scanner main.c
 * @{
 * @ingroup nrf_twi_example
 * @brief TWI Sensor Example main file.
 *
 * This file contains the source code for a sample application using TWI.
 *
 */

#include <stdio.h>
#include "app_util_platform.h"
#include "app_error.h"

#include "display_SSD1306.h"

#define NRF_LOG_MODULE_NAME "MAIN"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

/**
 * @brief Function for main application entry.
 */
int main(void)
{
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_INFO("Display test.\r\n");
    NRF_LOG_FLUSH();
    
    display_init();
    display_show();

    while (true)
    {
        /* Empty loop. */
    }
}

/** @} */