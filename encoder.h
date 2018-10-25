#include <stdint.h>
#include <stdbool.h>

void encoder_init(void);

void encoder_set_flag(volatile uint8_t* main_button_flag);

void encoder_set_direction(bool direction);

void encoder_enable(void);

void encoder_disable(void);

int32_t encoder_get_position(void);

void encoder_set_position(int32_t new_position);

void encoder_reset_position(void);
