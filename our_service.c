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
#include "batt.h"
#include "state.h"

#include "SEGGER_RTT.h"

#define NRF_LOG_MODULE_NAME "BLE OS"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

static uint16_t encode_sfloat(sfloat_t sf)
{
    uint16_t dec;
    dec = ((sf.exponent << 12) & 0xF000) | ((sf.mantissa <<  0) & 0x0FFF);
    return dec;
}

static sfloat_t decode_sfloat(uint16_t u)
{
    sfloat_t sf;
    sf.exponent = (u & 0xF000) >> 12;
    sf.mantissa = (u & 0x0FFF) >> 0;
    return sf;
}

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

    NRF_LOG_INFO("Write to CONFIG (len = %d, offset = %d)\n", data_len, offset);
    //NRF_LOG_HEXDUMP_INFO(p_data, data_len);

    if (data_len == 3 && offset == 0)
    {
        // Decode and save
        config_manager_set_flags(p_data[0]);
        config_manager_set_portion(uint16_decode(&p_data[1]));

        // Update
        config_char_update(p_our_service, p_data, data_len);
    }

    if (data_len == 4 && offset == 0)
    {
        // Decode and save
        config_manager_set_flags(p_data[0]);
        config_manager_set_portion(uint16_decode(&p_data[1]));
        config_manager_set_cho_interval(p_data[3]);

        // Update
        config_char_update(p_our_service, p_data, data_len);
    }

    if (data_len == 5 && offset == 0)
    {
        // Decode and save
        config_manager_set_flags(p_data[0]);
        config_manager_set_portion(uint16_decode(&p_data[1]));
        config_manager_set_cho_interval(p_data[3]);
        config_manager_set_ins_interval(p_data[4]);

        // Update
        config_char_update(p_our_service, p_data, data_len);
    }
}

static void on_name_write(ble_os_t * p_our_service, ble_gatts_evt_write_t * p_evt_write)
{
    uint8_t data_len = p_evt_write->len;
    uint8_t * p_data = p_evt_write->data;
    uint16_t offset = p_evt_write->offset;

    NRF_LOG_INFO("Write to NAME (len = %d, offset = %d)\n", data_len, offset);
    //NRF_LOG_HEXDUMP_INFO(p_data, data_len);

    if (data_len <= 20 && offset == 0)
    {
        // Save
        config_manager_set_name(p_data, data_len);

        // Update
        name_char_update(p_our_service, p_data, data_len);
    }
}

static void on_time_write(ble_os_t * p_our_service, ble_gatts_evt_write_t * p_evt_write)
{
    uint8_t data_len = p_evt_write->len;
    uint8_t * p_data = p_evt_write->data;
    uint16_t offset = p_evt_write->offset;

    NRF_LOG_INFO("Write to TIME (len = %d, offset = %d)\n", data_len, offset);
    //NRF_LOG_HEXDUMP_INFO(p_data, data_len);

    if (data_len == 7 && offset == 0)
    {
        // Decode
        ble_date_time_t ble_t;

        ble_date_time_decode(&ble_t, &p_data[0]);

        // Save
        struct tm t;
                                            /* C time library                   - BLE time service       */
        t.tm_year   = ble_t.year - 1900;    /* The number of years since 1900   - Year                   */
        t.tm_mon    = ble_t.month - 1;      /* Month, range 0 to 11             - 1 January              */
        t.tm_mday   = ble_t.day;            /* Day of the month, range 1 to 31  - Day of the month       */
        t.tm_hour   = ble_t.hours;          /* Hours, range 0 to 23             - Hours past midnight    */
        t.tm_min    = ble_t.minutes;        /* Minutes, range 0 to 59           - Minutes of the hour    */
        t.tm_sec    = ble_t.seconds;        /* Seconds, range 0 to 59           - Seconds of the minute  */

        //char buffer[80];
        //strftime(buffer, sizeof(buffer), "%x %X", &t);
        //NRF_LOG_INFO("Date to save %s\n", (uint32_t)buffer);
        //NRF_LOG_FLUSH();

        clock_set_time(&t);

        //NRF_LOG_INFO("New date\n");
        clock_print();

        // Update
        time_char_update(p_our_service);

        // Init operation
        state_begin();
    }
}

static void on_calc_write(ble_os_t * p_our_service, ble_gatts_evt_write_t * p_evt_write)
{
    uint8_t data_len = p_evt_write->len;
    uint8_t * p_data = p_evt_write->data;
    uint16_t offset = p_evt_write->offset;

    NRF_LOG_INFO("Write to CALC (len = %d, offset = %d)\n", data_len, offset);
    //NRF_LOG_HEXDUMP_INFO(p_data, data_len);

    if (data_len == 8 && offset == 0)
    {
        // Save
        config_manager_set_calc_low(decode_sfloat(uint16_decode(&p_data[0])));
        config_manager_set_calc_high(decode_sfloat(uint16_decode(&p_data[2])));
        config_manager_set_calc_sens(uint16_decode(&p_data[4]));
        config_manager_set_calc_corr(decode_sfloat(uint16_decode(&p_data[6])));

        // Update
        calc_char_update(p_our_service, p_data);
    }
}

static void on_insulin_write(ble_os_t * p_our_service, ble_gatts_evt_write_t * p_evt_write)
{
    uint8_t data_len = p_evt_write->len;
    uint8_t * p_data = p_evt_write->data;
    uint16_t offset = p_evt_write->offset;

    NRF_LOG_INFO("Write to INSULIN (len = %d, offset = %d)\n", data_len, offset);
    //NRF_LOG_HEXDUMP_INFO(p_data, data_len);

    if (data_len == 11 && offset == 0)
    {
        // Save
        config_manager_set_insulin_type(p_data[0]);
        config_manager_set_insulin_total(uint16_decode(&p_data[1]));
        config_manager_set_insulin_remaining(uint16_decode(&p_data[3]));
        config_manager_set_insulin_start(uint16_decode(&p_data[5]));
        config_manager_set_insulin_max(uint16_decode(&p_data[7]));
        config_manager_set_insulin_duration(uint16_decode(&p_data[9]));

        // Update
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
    uint8_t print = 0;

    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (p_evt_write->handle == p_our_service->config_char_handles.value_handle)
    {
        on_config_write(p_our_service, p_evt_write);
        //print = 1;
    }

    if (p_evt_write->handle == p_our_service->name_char_handles.value_handle)
    {
        on_name_write(p_our_service, p_evt_write);
        //print = 1;
    }

    if (p_evt_write->handle == p_our_service->time_char_handles.value_handle)
    {
        on_time_write(p_our_service, p_evt_write);
        //print = 1;
    }

    if (p_evt_write->handle == p_our_service->calc_char_handles.value_handle)
    {
        on_calc_write(p_our_service, p_evt_write);
        //print = 1;
    }

    if (p_evt_write->handle == p_our_service->ins_char_handles.value_handle)
    {
        on_insulin_write(p_our_service, p_evt_write);
        //print = 1;
    }

    if (print){
        config_manager_print();
    }

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

    char_uuid.uuid      = BLE_UUID_TOPSULIN_CONFIG_CHARACTERISTIC;

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
    char_md.char_props.notify   = 1;

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
    value[0] = config_manager_get_flags();
    uint16_encode(config_manager_get_portion(), &value[1]);
    value[3] = config_manager_get_cho_interval();
    value[4] = config_manager_get_ins_interval();
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

    char_uuid.uuid      = BLE_UUID_TOPSULIN_NAME_CHARACTERISTIC;

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
    char_md.char_props.notify   = 1;

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
    attr_char_value.init_len    = 20;
    uint8_t value[20]           = "Glucosee\0\0\0\0\0\0\0\0\0\0\0\0";
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

    char_uuid.uuid      = BLE_UUID_TOPSULIN_TIME_CHARACTERISTIC;

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
    char_md.char_props.notify   = 1;

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
    ble_date_time_t ble_t;
    struct tm t;
    clock_get_time(&t);
                                        /* BLE time service       - C time library                   */
    ble_t.year    = t.tm_year + 1900;   /* Year                   - The number of years since 1900   */
    ble_t.month   = t.tm_mon + 1;       /* 1 January              - Month, range 0 to 11             */
    ble_t.day     = t.tm_mday;          /* Day of the month       - Day of the month, range 1 to 31  */
    ble_t.hours   = t.tm_hour;          /* Hours past midnight    - Hours, range 0 to 23             */
    ble_t.minutes = t.tm_min;           /* Minutes of the hour    - Minutes, range 0 to 59           */
    ble_t.seconds = t.tm_sec;           /* Seconds of the minute  - Seconds, range 0 to 59           */

    ble_date_time_encode(&ble_t, &value[0]);
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

static uint32_t calc_char_add(ble_os_t * p_our_service)
{
    // Add a custom characteristic UUID
    uint32_t            err_code;
    ble_uuid_t          char_uuid;
    ble_uuid128_t       base_uuid = BLE_UUID_TOPSULIN_BASE_UUID;

    char_uuid.uuid      = BLE_UUID_TOPSULIN_CALC_CHARACTERISTIC;

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
    char_md.char_props.notify   = 1;

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
    attr_char_value.max_len     = 8;
    attr_char_value.init_len    = 8;
    uint8_t value[8];
    uint16_encode(encode_sfloat(config_manager_get_calc_low()), &value[0]);
    uint16_encode(encode_sfloat(config_manager_get_calc_high()), &value[2]);
    uint16_encode(config_manager_get_calc_sens(), &value[4]);
    uint16_encode(encode_sfloat(config_manager_get_calc_corr()), &value[6]);
    //NRF_LOG_HEXDUMP_INFO(value, sizeof(value));
    attr_char_value.p_value     = value;

    // Add our new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_our_service->service_handle,
                                   &char_md,
                                   &attr_char_value,
                                   &p_our_service->calc_char_handles);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}

static uint32_t insulin_char_add(ble_os_t * p_our_service)
{
    // Add a custom characteristic UUID
    uint32_t            err_code;
    ble_uuid_t          char_uuid;
    ble_uuid128_t       base_uuid = BLE_UUID_TOPSULIN_BASE_UUID;

    char_uuid.uuid      = BLE_UUID_TOPSULIN_INS_CHARACTERISTIC;

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
    char_md.char_props.notify   = 1;

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
    attr_char_value.max_len     = 11;
    attr_char_value.init_len    = 11;
    uint8_t value[11];
    value[0] = config_manager_get_insulin_type();
    uint16_encode(config_manager_get_insulin_total(), &value[1]);
    uint16_encode(config_manager_get_insulin_remaining(), &value[3]);
    uint16_encode(config_manager_get_insulin_start(), &value[5]);
    uint16_encode(config_manager_get_insulin_max(), &value[7]);
    uint16_encode(config_manager_get_insulin_duration(), &value[9]);
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

static uint32_t device_char_add(ble_os_t * p_our_service)
{
    // Add a custom characteristic UUID
    uint32_t            err_code;
    ble_uuid_t          char_uuid;
    ble_uuid128_t       base_uuid = BLE_UUID_TOPSULIN_BASE_UUID;

    char_uuid.uuid      = BLE_UUID_TOPSULIN_DEV_CHARACTERISTIC;

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
    char_md.char_props.notify   = 1;

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
    value[0] = config_manager_get_version();
    uint16_encode(config_manager_get_serial_number(), &value[1]);
    uint16_encode(0x0000, &value[3]);
    //NRF_LOG_HEXDUMP_INFO(value, sizeof(value));
    attr_char_value.p_value     = value;

    // Add our new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_our_service->service_handle,
                                   &char_md,
                                   &attr_char_value,
                                   &p_our_service->dev_char_handles);
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
    calc_char_add(p_our_service);
    insulin_char_add(p_our_service);
    device_char_add(p_our_service);
}


/**@brief Functions for updating Characteristics values.
 *
 * @param[in]   p_our_service        Our Service structure.
 */
void config_char_update(ble_os_t * p_our_service, uint8_t * characteristic_value, uint16_t data_len)
{
    // Update characteristic value
    //if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    //{
      uint16_t               len = data_len;
      ble_gatts_hvx_params_t hvx_params;
      memset(&hvx_params, 0, sizeof(hvx_params));

      hvx_params.handle = p_our_service->config_char_handles.value_handle;
      hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
      hvx_params.offset = 0;
      hvx_params.p_len  = &len;
      hvx_params.p_data = characteristic_value;

      sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);
    //}
}

void name_char_update(ble_os_t * p_our_service, uint8_t * p_data, uint16_t data_len)
{
    // Update characteristic value
    //if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    //{
      uint16_t               len = 20;
      ble_gatts_hvx_params_t hvx_params;
      memset(&hvx_params, 0, sizeof(hvx_params));

      uint8_t characteristic_value[20];
      uint8_t i;
      for(i = 0; i < 20; i++){
        if(i < data_len){
          characteristic_value[i] = p_data[i];
        } else {
          characteristic_value[i] = 0x00;
        }
      }
      NRF_LOG_HEXDUMP_INFO(characteristic_value, sizeof(characteristic_value));

      hvx_params.handle = p_our_service->name_char_handles.value_handle;
      hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
      hvx_params.offset = 0;
      hvx_params.p_len  = &len;
      hvx_params.p_data = characteristic_value;

      sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);
    //}
}

void time_char_update(ble_os_t * p_our_service)
{
    // Update characteristic value
    //if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    //{
      uint16_t               len = 7;
      ble_gatts_hvx_params_t hvx_params;
      memset(&hvx_params, 0, sizeof(hvx_params));

      hvx_params.handle = p_our_service->time_char_handles.value_handle;
      hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
      hvx_params.offset = 0;
      hvx_params.p_len  = &len;

      static uint8_t characteristic_value[7];
      ble_date_time_t ble_t;
      struct tm t;
      clock_get_time(&t);
                                          /* BLE time service       - C time library                   */
      ble_t.year    = t.tm_year + 1900;   /* Year                   - The number of years since 1900   */
      ble_t.month   = t.tm_mon + 1;       /* 1 January              - Month, range 0 to 11             */
      ble_t.day     = t.tm_mday;          /* Day of the month       - Day of the month, range 1 to 31  */
      ble_t.hours   = t.tm_hour;          /* Hours past midnight    - Hours, range 0 to 23             */
      ble_t.minutes = t.tm_min;           /* Minutes of the hour    - Minutes, range 0 to 59           */
      ble_t.seconds = t.tm_sec;           /* Seconds of the minute  - Seconds, range 0 to 59           */

      ble_date_time_encode(&ble_t, &characteristic_value[0]);
      //NRF_LOG_HEXDUMP_INFO(characteristic_value, sizeof(characteristic_value));

      hvx_params.p_data = characteristic_value;

      sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);
    //}
}

void calc_char_update(ble_os_t * p_our_service, uint8_t * characteristic_value)
{
    // Update characteristic value
    //if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    //{
      uint16_t               len = 8;
      ble_gatts_hvx_params_t hvx_params;
      memset(&hvx_params, 0, sizeof(hvx_params));

      hvx_params.handle = p_our_service->calc_char_handles.value_handle;
      hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
      hvx_params.offset = 0;
      hvx_params.p_len  = &len;
      hvx_params.p_data = characteristic_value;

      sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);
    //}
}

void insulin_char_update(ble_os_t * p_our_service, uint8_t * characteristic_value)
{
    // Update characteristic value
    //if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    //{
      uint16_t               len = 5;
      ble_gatts_hvx_params_t hvx_params;
      memset(&hvx_params, 0, sizeof(hvx_params));

      hvx_params.handle = p_our_service->ins_char_handles.value_handle;
      hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
      hvx_params.offset = 0;
      hvx_params.p_len  = &len;
      hvx_params.p_data = characteristic_value;

      sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);
    //}
}

void dev_char_update(ble_os_t * p_our_service, uint16_t v)
{
    // Update characteristic value
    //if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    //{
      uint16_t               len = 5;
      ble_gatts_hvx_params_t hvx_params;
      memset(&hvx_params, 0, sizeof(hvx_params));

      hvx_params.handle = p_our_service->dev_char_handles.value_handle;
      hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
      hvx_params.offset = 0;
      hvx_params.p_len  = &len;

      static uint8_t characteristic_value[5];
      characteristic_value[0] = config_manager_get_version();
      uint16_encode(config_manager_get_serial_number(), &characteristic_value[1]);
      uint16_encode(v, &characteristic_value[3]);

      //NRF_LOG_HEXDUMP_INFO(characteristic_value, sizeof(characteristic_value));

      hvx_params.p_data = characteristic_value;

      sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);
    //}
}
