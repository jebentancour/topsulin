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

#include <stdint.h>
#include <string.h>
#include "sdk_common.h"
#include "our_service.h"
#include "ble_srv_common.h"
#include "ble_date_time.h"
#include "app_error.h"
#include "clock.h"
#include "SEGGER_RTT.h"

#define NRF_LOG_MODULE_NAME "BLE OS"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"


/**@brief Function for handling the Topsulin Config write event.
 *
 * @param[in] p_our_service   Service instance.
 * @param[in] p_evt_write     WRITE event to be handled.
 */
static void on_config_write(ble_os_t * p_our_service, ble_gatts_evt_write_t * p_evt_write)
{
    uint8_t data_len = p_evt_write->len;
    uint8_t * p_data = p_evt_write->data;
    uint16_t offset = p_evt_write->offset;
    NRF_LOG_INFO("Write to CONFIG\n");
    NRF_LOG_HEXDUMP_INFO(p_data, data_len);
    if (data_len == 3 && offset == 0)
    {
        config_manager_set_flags(p_data[0]);
        config_manager_set_portion(uint16_decode(&p_data[1]));
        config_char_update(p_our_service, p_data);
    }
}

static void on_insulin_write(ble_os_t * p_our_service, ble_gatts_evt_write_t * p_evt_write)
{
    uint8_t data_len = p_evt_write->len;
    uint8_t * p_data = p_evt_write->data;
    uint16_t offset = p_evt_write->offset;
    NRF_LOG_INFO("Write to INSULIN\n");
    NRF_LOG_HEXDUMP_INFO(p_data, data_len);
    if (data_len == 5 && offset == 0)
    {
        config_manager_set_insulin_type(p_data[0]);
        config_manager_set_insulin_total(uint16_decode(&p_data[1]));
        config_manager_set_insulin_remaining(uint16_decode(&p_data[3]));
        insulin_char_update(p_our_service, p_data);
    }
}

/**@brief Function for handling the WRITE event.
 *
 * @details Handles WRITE events from the BLE stack.
 *
 * @param[in] p_our_service      Service structure.
 * @param[in] p_ble_evt          Event received from the BLE stack.
 */
static void on_write(ble_os_t * p_our_service, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (p_evt_write->handle == p_our_service->config_char_handles.value_handle)
    {
        on_config_write(p_our_service, p_evt_write);
    }

    if (p_evt_write->handle == p_our_service->name_char_handles.value_handle)
    {
        NRF_LOG_INFO("Write to NAME\n");
        //NRF_LOG_INFO("Write len: %#04x\n", p_evt_write->len);
    }

    if (p_evt_write->handle == p_our_service->time_char_handles.value_handle)
    {
        NRF_LOG_INFO("Write to TIME\n");
        //NRF_LOG_INFO("Write len: %#04x\n", p_evt_write->len);
    }

    if (p_evt_write->handle == p_our_service->calc_char_handles.value_handle)
    {
        NRF_LOG_INFO("Write to CALC\n");
        //NRF_LOG_INFO("Write len: %#04x\n", p_evt_write->len);
    }

    if (p_evt_write->handle == p_our_service->ins_char_handles.value_handle)
    {
        on_insulin_write(p_our_service, p_evt_write);
    }

    config_manager_print();
    NRF_LOG_FLUSH();
}


void ble_our_service_on_ble_evt(ble_evt_t * p_ble_evt, void * p_context)
{
  	ble_os_t * p_our_service = (ble_os_t *) p_context;
		// Implement switch case handling BLE events related to our service.
    switch (p_ble_evt->header.evt_id)
    {
      case BLE_GAP_EVT_CONNECTED:
          p_our_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
          break;
      case BLE_GAP_EVT_DISCONNECTED:
          p_our_service->conn_handle = BLE_CONN_HANDLE_INVALID;
          break;
      case BLE_GATTS_EVT_WRITE:
          on_write(p_our_service, p_ble_evt);
          break;
      default:
          // No implementation needed.
          break;
    }
}


/**@brief Function for adding a new characterstic to our service.
 *
 * @param[in]   p_our_service        Our Service structure.
 */
static uint32_t config_char_add(ble_os_t * p_our_service)
{
    // Add a custom characteristic UUID
    uint32_t            err_code;
    ble_uuid_t          char_uuid;
    ble_uuid128_t       base_uuid = BLE_UUID_TOPSULIN_BASE_UUID;

    char_uuid.uuid      = BLE_UUID_TOPSULIN_CONFIG_CHARACTERISTC;

    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(err_code);

    // Add read/write properties to our characteristic
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1;
    char_md.char_props.write = 1;

    // Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc                = BLE_GATTS_VLOC_STACK;
    char_md.p_cccd_md           = &cccd_md;
    //char_md.char_props.notify   = 1;

    // Configure the attribute metadata
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc        = BLE_GATTS_VLOC_STACK;

    // Set read/write security levels to our characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    // Configure the characteristic value attribute
    ble_gatts_attr_t    attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;

    // Set characteristic length in number of bytes
    attr_char_value.max_len     = 3;
    attr_char_value.init_len    = 3;
    uint8_t value[3];
    value[0] = config_manager_get_flags();
    uint16_encode(config_manager_get_portion(), &value[1]);
    //NRF_LOG_HEXDUMP_INFO(value, sizeof(value));
    attr_char_value.p_value     = value;

    // Add our new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_our_service->service_handle,
                                   &char_md,
                                   &attr_char_value,
                                   &p_our_service->config_char_handles);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}

static uint32_t name_char_add(ble_os_t * p_our_service)
{
    // Add a custom characteristic UUID
    uint32_t            err_code;
    ble_uuid_t          char_uuid;
    ble_uuid128_t       base_uuid = BLE_UUID_TOPSULIN_BASE_UUID;

    char_uuid.uuid      = BLE_UUID_TOPSULIN_NAME_CHARACTERISTC;

    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(err_code);

    // Add read/write properties to our characteristic
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1;
    char_md.char_props.write = 1;

    // Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc                = BLE_GATTS_VLOC_STACK;
    char_md.p_cccd_md           = &cccd_md;
    //char_md.char_props.notify   = 1;

    // Configure the attribute metadata
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc        = BLE_GATTS_VLOC_STACK;

    // Set read/write security levels to our characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    // Configure the characteristic value attribute
    ble_gatts_attr_t    attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;

    // Set characteristic length in number of bytes
    attr_char_value.max_len     = 20;
    attr_char_value.init_len    = 8;
    uint8_t value[20]           = "Glucosee";
    //NRF_LOG_HEXDUMP_INFO(value, 20);
    attr_char_value.p_value     = value;

    // Add our new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_our_service->service_handle,
                                   &char_md,
                                   &attr_char_value,
                                   &p_our_service->name_char_handles);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}

static uint32_t time_char_add(ble_os_t * p_our_service)
{
    // Add a custom characteristic UUID
    uint32_t            err_code;
    ble_uuid_t          char_uuid;
    ble_uuid128_t       base_uuid = BLE_UUID_TOPSULIN_BASE_UUID;

    char_uuid.uuid      = BLE_UUID_TOPSULIN_TIME_CHARACTERISTC;

    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(err_code);

    // Add read/write properties to our characteristic
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1;
    char_md.char_props.write = 1;

    // Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc                = BLE_GATTS_VLOC_STACK;
    char_md.p_cccd_md           = &cccd_md;
    //char_md.char_props.notify   = 1;

    // Configure the attribute metadata
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc        = BLE_GATTS_VLOC_STACK;

    // Set read/write security levels to our characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    // Configure the characteristic value attribute
    ble_gatts_attr_t    attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;

    // Set characteristic length in number of bytes
    attr_char_value.max_len     = 7;
    attr_char_value.init_len    = 7;
    uint8_t value[7];
    ble_date_time_t t;
    t = clock_get_time();
    ble_date_time_encode(&t, &value[0]);
    //NRF_LOG_HEXDUMP_INFO(value, sizeof(value));
    attr_char_value.p_value     = value;

    // Add our new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_our_service->service_handle,
                                   &char_md,
                                   &attr_char_value,
                                   &p_our_service->time_char_handles);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}

static uint32_t insulin_char_add(ble_os_t * p_our_service)
{
    // Add a custom characteristic UUID
    uint32_t            err_code;
    ble_uuid_t          char_uuid;
    ble_uuid128_t       base_uuid = BLE_UUID_TOPSULIN_BASE_UUID;

    char_uuid.uuid      = BLE_UUID_TOPSULIN_INS_CHARACTERISTC;

    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(err_code);

    // Add read/write properties to our characteristic
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1;
    char_md.char_props.write = 1;

    // Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc                = BLE_GATTS_VLOC_STACK;
    char_md.p_cccd_md           = &cccd_md;
    //char_md.char_props.notify   = 1;

    // Configure the attribute metadata
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc        = BLE_GATTS_VLOC_STACK;

    // Set read/write security levels to our characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    // Configure the characteristic value attribute
    ble_gatts_attr_t    attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;

    // Set characteristic length in number of bytes
    attr_char_value.max_len     = 5;
    attr_char_value.init_len    = 5;
    uint8_t value[5];
    value[0] = config_manager_get_insulin_type();
    uint16_encode(config_manager_get_insulin_total(), &value[1]);
    uint16_encode(config_manager_get_insulin_remaining(), &value[3]);
    //NRF_LOG_HEXDUMP_INFO(value, sizeof(value));
    attr_char_value.p_value     = value;

    // Add our new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_our_service->service_handle,
                                   &char_md,
                                   &attr_char_value,
                                   &p_our_service->ins_char_handles);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}


/**@brief Function for initiating our new service.
 *
 * @param[in]   p_our_service        Our Service structure.
 */
void our_service_init(ble_os_t * p_our_service)
{
    // Add UUIDs to BLE stack table
    uint32_t          err_code;
    ble_uuid128_t     base_uuid = BLE_UUID_TOPSULIN_BASE_UUID;
    ble_uuid_t        service_uuid;

    service_uuid.uuid =  BLE_UUID_TOPSULIN_SERVICE;

    err_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid.type);
    APP_ERROR_CHECK(err_code);

    p_our_service->conn_handle = BLE_CONN_HANDLE_INVALID;

    // Add our service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                    &service_uuid,
                                    &p_our_service->service_handle);
    APP_ERROR_CHECK(err_code);

    config_char_add(p_our_service);
    name_char_add(p_our_service);
    time_char_add(p_our_service);
    insulin_char_add(p_our_service);
}


void config_char_update(ble_os_t * p_our_service, uint8_t * characteristic_value)
{
    // Update characteristic value
    if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
      uint16_t               len = 3;
      ble_gatts_hvx_params_t hvx_params;
      memset(&hvx_params, 0, sizeof(hvx_params));

      hvx_params.handle = p_our_service->config_char_handles.value_handle;
      hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
      hvx_params.offset = 0;
      hvx_params.p_len  = &len;
      hvx_params.p_data = characteristic_value;

      sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);
    }
}

void insulin_char_update(ble_os_t * p_our_service, uint8_t * characteristic_value)
{
    // Update characteristic value
    if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
      uint16_t               len = 5;
      ble_gatts_hvx_params_t hvx_params;
      memset(&hvx_params, 0, sizeof(hvx_params));

      hvx_params.handle = p_our_service->ins_char_handles.value_handle;
      hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
      hvx_params.offset = 0;
      hvx_params.p_len  = &len;
      hvx_params.p_data = characteristic_value;

      sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);
    }
}
