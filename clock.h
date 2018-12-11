#include <stdint.h>
#include <time.h>

#include "ble_services.h"

#define CLOCK_TICK_MS   100

void clock_init(void);

void clock_print(void);

uint32_t clock_get_timestamp(void);

void clock_tick_set_flag(volatile uint8_t * main_tick_flag);

void clock_get_time(struct tm * t);

void clock_set_time(struct tm * timeptr);
