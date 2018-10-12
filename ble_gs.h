#include <stdint.h>

void ble_gs_init(void);

void battery_level_update(uint8_t  battery_level);

void read_glucose_measurement(void);

void advertising_start(void);
