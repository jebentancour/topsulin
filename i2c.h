/**
 * @defgroup I2C
 * @{
 *
 * @file i2c.h
 * 
 * @version 1.0
 * @author  Rodrigo De Soto, Maite Gil, José Bentancour.
 * @date 12 Julio 2018
 * 
 * @brief Módulo que proporciona funciones para manejar el periférico TWI (Two Wire Interface, compatible con I2C).
 * 
 * @paragraph 
 * 
 * Este módulo pertenece a la capa de abstracción de hardware y proporciona interfaces
 * para enviar datos. No soporta recepción.
 */
 
#include <stdint.h>

/**@brief Función de inicialización del módulo.
 */
void i2c_init(void);


/**@brief Función para setear la flag donde indicar el fin de transmisión de un byte.
 *
 * @param main_tx_flag    Puntero a una flag donde se indicara el fin de transmisión de un byte.
 */
void i2c_tx_set_flag(volatile uint8_t* main_tx_flag);


/**@brief Función para iniciar una transferencia de datos a un esclavo.
 *
 * @param address    La dirección de 7 bits del esclavo.
 */
void i2c_begin_transmission(uint8_t address);


/**@brief Función para transmitir un byte a un esclavo.
 *
 * @param value    El dato a transmitir.
 */
void i2c_write(uint8_t value);


/**@brief Función para terminar la transacción.
 */
void i2c_end_transmission(void);