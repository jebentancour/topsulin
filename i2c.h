#include <stdint.h>

void i2c_init(void);

void i2c_tx_set_flag(volatile uint8_t* main_tx_flag);

void i2c_begin_transmission(uint8_t address);

void i2c_write(uint8_t value);

void i2c_end_transmission(void);