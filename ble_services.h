#include <stdint.h>

#include "ble_gls.h"

void ble_services_init(void);

void add_glucose_measurement(ble_gls_rec_t rec);

void peer_manager_erase_bonds(void);

void advertising_start(void);
