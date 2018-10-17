#include <stdint.h>

#include "ble_services.h"

#define CLOCK_TICK_MS   100

void clock_init(void);

uint32_t clock_get_timestamp(void);

void clock_tick_set_flag(volatile uint8_t* main_tick_flag);
