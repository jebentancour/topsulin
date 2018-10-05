#include <stdint.h>

#define APP_TIMER_PRESCALER     0       /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE 4       /**< Size of timer operation queues. */

#define MAX_LEN                 50      /**< Largo de mensaje maximo. */

/** Estructura que representa el estado interno del módulo. */
typedef struct {
    uint8_t advertising;
    uint8_t connected;
} ble_uart_status_t;


/**@brief Función de inicialización del módulo.
 */
void ble_uart_init(void);


/**@brief Función para obtener el estado del módulo.
 */
ble_uart_status_t ble_uart_get_status(void);


/**@brief Función para setear la flag donde indicar la llegada de un mensaje completo.
 *
 * @details Se considera que un mensaje esta completo cuando se recibe el caracter "\n".
 *
 * @param main_rx_flag    Puntero a una flag donde se indicara la llegada de un mensaje completo.
 */
void ble_uart_rx_set_flag(volatile uint8_t*);


/**@brief Función para setear la flag donde indicar el fin de transmisión de un mensaje completo.
 *
 * @param main_tx_flag    Puntero a una flag donde se indicara la llegada de un mensaje completo.
 */
void ble_uart_tx_set_flag(volatile uint8_t*);


/**@brief Función para obtener el mensaje recibido.
 *
 * @param msg_main    Puntero al arreglo donde se copiara el mensaje.
 *
 * @return El largo del mensaje.
 */
uint16_t ble_uart_get_msg(uint8_t*);


/**@brief Función para enviar un mensaje.
 *
 * @param msg_main  Puntero al arreglo donde se encuentra el mensaje.
 * @param length    El largo del mensaje.
 *
 */
void ble_uart_data_send(uint8_t * p_data, uint16_t length);


/**@brief Función iniciar el descubrimiento por Bluetooth.
 *
 * @warning Se debe esperar a que transcurra el tiempo APP_ADV_TIMEOUT_IN_SECONDS para volver a llamarla.
 */
void ble_uart_advertising_start(void);


/**@brief Función detener el descubrimiento por Bluetooth.
 */
void ble_uart_advertising_stop(void);


/**@brief Función terminar la conexión Bluetooth con otro dispositivo.
 */
void ble_uart_disconnect(void);
