#include <stdint.h>

#define CONFIG_GLUCOSE_FLAG                     0x01
#define CONFIG_INSULIN_FLAG                     0x02
#define CONFIG_CHO_FLAG                         0x04
#define CONFIG_COLOR_FLAG                       0x08
#define CONFIG_UNITS_FLAG                       0x10
#define CONFIG_BOLO_FLAG                        0x20
#define CONFIG_FLIP_FLAG                        0x40
#define CONFIG_PORTION_FLAG                     0x80

typedef struct
{
    uint8_t   flags;
    uint16_t  g_portion;
    uint8_t   insulin_type;
    uint16_t  insulin_total;
    uint16_t  insulin_remaining;
    uint16_t  insulin_start;
    uint16_t  insulin_max;
    uint16_t  insulin_duration;
    char      name[20];
}global_conf_t;

void config_manager_init(void);

void config_manager_print(void);

void config_manager_set_flags(uint8_t flags);

uint8_t config_manager_get_flags(void);

void config_manager_set_portion(uint16_t portion);

uint16_t config_manager_get_portion(void);

void config_manager_set_insulin_type(uint8_t type);

uint8_t config_manager_get_insulin_type(void);

void config_manager_set_insulin_total(uint16_t total);

uint16_t config_manager_get_insulin_total(void);

void config_manager_set_insulin_remaining(uint16_t rem);

uint16_t config_manager_get_insulin_remaining(void);

void config_manager_set_insulin_start(uint16_t start);

uint16_t config_manager_get_insulin_start(void);

void config_manager_set_insulin_max(uint16_t max);

uint16_t config_manager_get_insulin_max(void);

void config_manager_set_insulin_duration(uint16_t d);

uint16_t config_manager_get_insulin_duration(void);

void config_manager_set_name(uint8_t* data, uint8_t data_len);
