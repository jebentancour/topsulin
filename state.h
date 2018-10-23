#include <stdint.h>

typedef enum {
    button_pressed,
    encoder_update,
    time_update
} event_t;

void state_init(void);

void state_on_event(event_t event);

void state_disable(void);
