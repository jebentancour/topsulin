#include <stdint.h>

void batt_init(void);

void batt_disable(void);

void batt_sample(void);

uint32_t batt_get(void);

void batt_set_flag(volatile uint8_t* main_batt_flag);
