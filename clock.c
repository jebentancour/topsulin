#include "clock.h"

#define NRF_LOG_MODULE_NAME "CLOCK"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "app_timer.h"
#include "app_util_platform.h"
#include "ble_date_time.h"

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
}

void clock_print(void)
{
    char buffer[80];
    struct tm m_time;
    clock_get_time(&m_time);
    strftime(buffer, sizeof(buffer), "%x %X", &m_time);
    NRF_LOG_INFO("Date %s\n", (uint32_t)buffer);
    NRF_LOG_FLUSH();
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

void clock_get_time(struct tm * t)
{
    CRITICAL_REGION_ENTER();
    time_t s = clock_ms_counter / 1000;
    struct tm * m_time;
    m_time = localtime(&s);
    t->tm_year   = m_time->tm_year;
    t->tm_mon    = m_time->tm_mon;
    t->tm_mday   = m_time->tm_mday;
    t->tm_hour   = m_time->tm_hour;
    t->tm_min    = m_time->tm_min;
    t->tm_sec    = m_time->tm_sec;
    CRITICAL_REGION_EXIT();
}

void clock_set_time(struct tm * timeptr)
{
    CRITICAL_REGION_ENTER();
    time_t ret;
    ret = mktime(timeptr);
    if( ret == -1 ) {
       NRF_LOG_INFO("Error: unable to make time using mktime\n");
    } else {
       NRF_LOG_INFO("time_t = %ld\n", ret);
       clock_ms_counter = ret * 1000;
    }
    CRITICAL_REGION_EXIT();
}
