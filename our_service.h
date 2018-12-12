/* Copyright (c) Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 *   3. Neither the name of Nordic Semiconductor ASA nor the names of other
 *   contributors to this software may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *   4. This software must only be used in a processor manufactured by Nordic
 *   Semiconductor ASA, or in a processor manufactured by a third party that
 *   is used in combination with a processor manufactured by Nordic Semiconductor.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef OUR_SERVICE_H__
#define OUR_SERVICE_H__

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"

#include "config_manager.h"

#define BLE_UUID_TOPSULIN_BASE_UUID             {{0x04, 0x11, 0x52, 0x07, 0x9B, 0x44, 0x4D, 0xEC, 0xA7, 0x31, 0x05, 0x15, 0x00, 0x00, 0x19, 0x34}} // 128-bit base UUID
#define BLE_UUID_TOPSULIN_SERVICE               0xF65D
#define BLE_UUID_TOPSULIN_CONFIG_CHARACTERISTC  0xF65E
#define BLE_UUID_TOPSULIN_NAME_CHARACTERISTC    0xF65F
#define BLE_UUID_TOPSULIN_TIME_CHARACTERISTC    0xF660
#define BLE_UUID_TOPSULIN_CALC_CHARACTERISTC    0xF661
#define BLE_UUID_TOPSULIN_INS_CHARACTERISTC     0xF662
#define BLE_UUID_TOPSULIN_DEV_CHARACTERISTC     0xF663

/**@brief This structure contains various status information for our service.
 * The name is based on the naming convention used in Nordic's SDKs.
 * ble_ indicates that it is a Bluetooth Low Energy relevant structure and
 * os_ is short for Our Service.
 */
typedef struct
{
    uint16_t    conn_handle;        /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection).*/
    uint16_t    service_handle;     /**< Handle of Our Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t    config_char_handles;
    ble_gatts_char_handles_t    name_char_handles;
    ble_gatts_char_handles_t    time_char_handles;
    ble_gatts_char_handles_t    calc_char_handles;
    ble_gatts_char_handles_t    ins_char_handles;
    ble_gatts_char_handles_t    dev_char_handles;
}ble_os_t;


/**@brief Function for initializing our new service.
 *
 * @param[in]   p_our_service       Pointer to Our Service structure.
 */
void our_service_init(ble_os_t * p_our_service);


/**@brief Function for handling BLE Stack events related to our service and characteristic.
 *
 * @details Handles all events from the BLE stack of interest to Our Service.
 *
 * @param[in]   p_ble_evt           Event received from the BLE stack.
 * @param[in]   p_our_service       Our Service structure.
 */
void ble_our_service_on_ble_evt(ble_evt_t * p_ble_evt, void * p_context);


/**@brief Function for updating and sending new characteristic values
 *
 * @param[in]   p_our_service            Our Service structure.
 * @param[in]   characteristic_value     New characteristic value.
 */
void config_char_update(ble_os_t *p_our_service, uint8_t *characteristic_value);

void name_char_update(ble_os_t * p_our_service, uint8_t * p_data, uint16_t data_len);

void time_char_update(ble_os_t * p_our_service);

void insulin_char_update(ble_os_t * p_our_service, uint8_t * characteristic_value);

#endif  /* _ OUR_SERVICE_H__ */
