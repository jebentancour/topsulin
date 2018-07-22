#include <stdint.h>

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "softdevice_handler.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

int main(void)
{
    NRF_LOG_INFO("InsuTest\n");
    NRF_LOG_FLUSH();

    sd_power_mode_set(NRF_POWER_MODE_LOWPWR);

    for (;;)
    {
        NRF_LOG_INFO("Going to sleep\n");
        NRF_LOG_FLUSH();
        
        sd_app_evt_wait();
    }
}
