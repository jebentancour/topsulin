#include "clock.h"

#include "app_timer.h"
#include "app_util_platform.h"
#include "ble_date_time.h"

static ble_date_time_t m_time;

#define APP_TIMER_PRESCALER     0       /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE 4       /**< Size of timer operation queues. */

APP_TIMER_DEF(clock_timer_id);

static uint32_t clock_ticks = APP_TIMER_TICKS(CLOCK_TICK_MS, APP_TIMER_PRESCALER);
static uint64_t clock_ms_counter;

static volatile uint8_t * m_tick_flag;

static void clock_timeout_handler(void * clock_timeout_handler_pointer)
{
    clock_ms_counter += CLOCK_TICK_MS;
    *m_tick_flag = 1;
}

void clock_init(void)
{
    app_timer_create(&clock_timer_id, APP_TIMER_MODE_REPEATED, clock_timeout_handler);
    app_timer_start(clock_timer_id, clock_ticks, NULL);
    m_time.year = 2018;
    m_time.month = 11;
    m_time.day = 26;
    m_time.hours = 15;
    m_time.minutes = 16;
    m_time.seconds = 30;
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

ble_date_time_t clock_get_time(void)
{
    ble_date_time_t aux_time;
    CRITICAL_REGION_ENTER();
    aux_time = m_time;
    CRITICAL_REGION_EXIT();
    return aux_time;
}

void clock_set_time(ble_date_time_t t)
{
    CRITICAL_REGION_ENTER();
    m_time = t;
    CRITICAL_REGION_EXIT();
}
