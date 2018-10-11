#include "clock.h"

#include "app_timer.h"
#include "nrf_drv_clock.h"
#include "app_util_platform.h"

#define APP_TIMER_PRESCALER     0       /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE 4       /**< Size of timer operation queues. */

APP_TIMER_DEF(clock_timer_id);

static volatile uint8_t* m_tick_flag;

static uint32_t clock_ticks = APP_TIMER_TICKS(CLOCK_TICK_MS, APP_TIMER_PRESCALER);
static uint64_t clock_ms_counter;

static void clock_timeout_handler(void * clock_timeout_handler_pointer)
{
    clock_ms_counter += CLOCK_TICK_MS;
    *m_tick_flag = 1;
}

void clock_init(void)
{
    app_timer_create(&clock_timer_id, APP_TIMER_MODE_REPEATED, clock_timeout_handler);
    app_timer_start(clock_timer_id, clock_ticks, NULL);
}

uint32_t clock_get_timestamp(void)
{
    uint32_t aux;
    CRITICAL_REGION_ENTER();
    aux = clock_ms_counter;
    CRITICAL_REGION_EXIT();
    return aux;
}

void clock_tick_set_flag(volatile uint8_t* main_tick_flag)
{
    m_tick_flag = main_tick_flag;
}
