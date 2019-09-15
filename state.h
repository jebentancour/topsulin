#include <stdint.h>

typedef enum {
    button_pressed,
    long_button_pressed,
    double_button_pressed,
    encoder_update,
    time_update,
    ble_on,
    ble_off
} event_t;

void state_init(void);

void state_process_display(void);

void state_on_event(event_t event);

void state_show_pin(char* pin);

void state_show_pin_error(void);

void state_show_pin_ok(void);

void state_begin(void);

void state_clock_set(void);

void state_gls_write(void);

void state_sleep(void);

void state_set_bt_state(uint8_t state);

void state_update_mem(void);
