#include <stdint.h>

typedef enum {
    button_pressed,
    long_button_pressed,
    encoder_update,
    time_update,
    ble_on,
    ble_off
} event_t;

void state_init(void);

void state_on_event(event_t event);

void state_show_pin(char* pin);

void state_begin(void);

void state_sleep(void);
