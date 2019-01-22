/**
 * Copyright (c) 2014 - 2017, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/** @file
 *
 * @defgroup ble_sdk_app_gls_main main.c
 * @{
 * @ingroup ble_sdk_app_gls
 * @brief Glucose Meter service Sample Application
 *
 * This file contains the source code for a sample application using the Glucose Meter service
 * (and also Battery and Device Information services). This application uses the
 * @ref srvlib_conn_params module.
 */

#include "ble_services.h"

#include "ble_advertising.h"
#include "ble_gls.h"
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "peer_manager.h"
#include "fds.h"
#include "fstorage.h"
#include "ble_conn_state.h"

#define NRF_LOG_MODULE_NAME "BLE"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "our_service.h"
#include "state.h"

#define NRF_CLOCK_LFCLKSRC              {.source = NRF_CLOCK_LF_SRC_XTAL, .rc_ctiv = 0, .rc_temp_ctiv = 0, .xtal_accuracy=NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}

#if (NRF_SD_BLE_API_VERSION == 3)
#define NRF_BLE_MAX_MTU_SIZE            GATT_MTU_SIZE_DEFAULT                       /**< MTU size used in the softdevice enabling and to reply to a BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST event. */
#endif

#define CENTRAL_LINK_COUNT              0                                           /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           1                                           /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define IS_SRVC_CHANGED_CHARACT_PRESENT 0                                           /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define DEVICE_NAME                     "Topsulin"                                  /**< Name of device. Will be included in the advertising data. */

#define APP_ADV_INTERVAL                40                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 25 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS      30                                          /**< The advertising timeout in units of seconds. */

#define APP_TIMER_PRESCALER             0                                           /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                           /**< Size of timer operation queues. */

#define SECURITY_REQUEST_DELAY          APP_TIMER_TICKS(400, APP_TIMER_PRESCALER)   /**< Delay after connection until Security Request is sent, if necessary (ticks). */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(10, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (10 ms). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)            /**< Maximum acceptable connection interval (100 ms) */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds). */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAM_UPDATE_COUNT     3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                  1                                           /**< Perform bonding. */
#define SEC_PARAM_MITM                  1                                           /**< Man In The Middle protection required (applicable when display module is detected). */
#define SEC_PARAM_LESC                  0                                           /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS              0                                           /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_DISPLAY_ONLY                /**< Display I/O capabilities. */
#define SEC_PARAM_OOB                   0                                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                           /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                          /**< Maximum encryption key size. */

#define PASSKEY_LENGTH                  6                                           /**< Length of pass-key received by the stack for display. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */

static uint16_t  m_conn_handle = BLE_CONN_HANDLE_INVALID;                           /**< Handle of the current connection. */
static ble_gls_t m_gls;                                                             /**< Structure used to identify the glucose service. */
static ble_os_t  m_our_service;

APP_TIMER_DEF(m_sec_req_timer_id);                                                  /**< Security Request timer. */

static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_GLUCOSE_SERVICE, BLE_UUID_TYPE_BLE}};  /**< Universally unique service identifiers. */

static ble_uuid_t m_more_adv_uuids[] = {{BLE_UUID_TOPSULIN_SERVICE, BLE_UUID_TYPE_VENDOR_BEGIN}};

pm_peer_id_t m_peer_to_be_deleted = PM_PEER_ID_INVALID;


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    ret_code_t err_code;

    switch (p_evt->evt_id)
    {
        case PM_EVT_BONDED_PEER_CONNECTED:
        {
            NRF_LOG_INFO("Connected to a previously bonded device.\n");
            // Start Security Request timer.
            err_code = app_timer_start(m_sec_req_timer_id, SECURITY_REQUEST_DELAY, NULL);
            APP_ERROR_CHECK(err_code);
        } break;

        case PM_EVT_CONN_SEC_SUCCEEDED:
        {
            pm_conn_sec_status_t conn_sec_status;

            // Check if the link is authenticated (meaning at least MITM).
            err_code = pm_conn_sec_status_get(p_evt->conn_handle, &conn_sec_status);
            APP_ERROR_CHECK(err_code);

            if (conn_sec_status.mitm_protected)
            {
                NRF_LOG_INFO("Link secured. Role: %d. conn_handle: %d, Procedure: %d\n",
                             ble_conn_state_role(p_evt->conn_handle),
                             p_evt->conn_handle,
                             p_evt->params.conn_sec_succeeded.procedure);

                state_begin();
            }
            else
            {
                // The peer did not use MITM, disconnect.
                NRF_LOG_INFO("Collector did not use MITM, disconnecting\n");
                err_code = pm_peer_id_get(m_conn_handle, &m_peer_to_be_deleted);
                APP_ERROR_CHECK(err_code);
                err_code = sd_ble_gap_disconnect(m_conn_handle,
                                                 BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
                APP_ERROR_CHECK(err_code);
            }
        } break;

        case PM_EVT_CONN_SEC_FAILED:
        {
            NRF_LOG_INFO("Failed to secure connection. Disconnecting.\n");
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            err_code = sd_ble_gap_disconnect(m_conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            //APP_ERROR_CHECK(err_code);
        } break;

        case PM_EVT_CONN_SEC_CONFIG_REQ:
        {
            // Reject pairing request from an already bonded peer.
            pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false};
            pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
        } break;

        case PM_EVT_STORAGE_FULL:
        {
            // Run garbage collection on the flash.
            err_code = fds_gc();
            if (err_code == FDS_ERR_BUSY || err_code == FDS_ERR_NO_SPACE_IN_QUEUES)
            {
                // Retry.
            }
            else
            {
                APP_ERROR_CHECK(err_code);
            }
        } break;

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
        {
            advertising_start();
        } break;

        case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
        {
            // The local database has likely changed, send service changed indications.
            pm_local_database_has_changed();
        } break;

        case PM_EVT_PEER_DATA_UPDATE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
        } break;

        case PM_EVT_PEER_DELETE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
        } break;

        case PM_EVT_PEERS_DELETE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
        } break;

        case PM_EVT_ERROR_UNEXPECTED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
        } break;

        case PM_EVT_CONN_SEC_START:
        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
        case PM_EVT_PEER_DELETE_SUCCEEDED:
        case PM_EVT_LOCAL_DB_CACHE_APPLIED:
        case PM_EVT_SERVICE_CHANGED_IND_SENT:
        case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
        default:
            break;
    }
    NRF_LOG_FLUSH();
}


/**@brief Function for handling Service errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void service_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling the Security Request timer timeout.
 *
 * @details This function will be called each time the Security Request timer expires.
 *
 * @param[in] p_context  Pointer used for passing some arbitrary information (context) from the
 *                       app_start_timer() call to the timeout handler.
 */
static void sec_req_timeout_handler(void * p_context)
{
    uint32_t err_code;

    if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        // Initiate bonding.
        NRF_LOG_DEBUG("Start encryption\n");
        err_code = pm_conn_secure(m_conn_handle, false);
        if (err_code != NRF_ERROR_INVALID_STATE)
        {
            APP_ERROR_CHECK(err_code);
        }
    }
    NRF_LOG_FLUSH();
}


/**@brief Function for updating glucose measurement and updating glucose characteristic in Glucose Service.
 */
void add_glucose_measurement(ble_gls_rec_t rec)
{
    uint32_t      err_code;

    err_code = ble_gls_glucose_new_meas(&m_gls, &rec);
    if (err_code != NRF_SUCCESS)
    {
        // Do nothing.
    }
}

void time_ble_update(void)
{
    time_char_update(&m_our_service);
}

void batt_ble_update(uint16_t v)
{
    dev_char_update(&m_our_service, v);
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    uint32_t err_code;

    // Initialize timer module.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);

    // Create Security Request timer.
    err_code = app_timer_create(&m_sec_req_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                sec_req_timeout_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_GLUCOSE_METER);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing services that will be used by the application.
 *
 * @details Initialize the Glucose, Battery and Device Information services.
 */
static void services_init(void)
{
    uint32_t       err_code;
    ble_gls_init_t gls_init;

    // Initialize Glucose Service - sample selection of feature bits.
    memset(&gls_init, 0, sizeof(gls_init));

    gls_init.evt_handler          = NULL;
    gls_init.error_handler        = service_error_handler;
    gls_init.feature              = 0;
    gls_init.feature             |= BLE_GLS_FEATURE_LOW_BATT;
    gls_init.feature             |= BLE_GLS_FEATURE_TEMP_HIGH_LOW;
    gls_init.feature             |= BLE_GLS_FEATURE_GENERAL_FAULT;
    gls_init.is_context_supported = true;

    err_code = ble_gls_init(&m_gls, &gls_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Our Service.
    our_service_init(&m_our_service);

}


/**@brief Function for handling the Connection Parameter events.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail configuration parameter, but instead we use the
 *                event handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAM_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("Fast advertising\n");
            state_on_event(ble_on);
            break; // BLE_ADV_EVT_FAST

        case BLE_ADV_EVT_IDLE:
            NRF_LOG_INFO("Idle advertising\n");
            state_on_event(ble_off);
            break; // BLE_ADV_EVT_IDLE

        default:
            break;
    }
    NRF_LOG_FLUSH();
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code = NRF_SUCCESS; // lint -save -e438 // Last value assigned to variable 'err_code' not used

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_DISCONNECTED:
        {
            NRF_LOG_INFO("Disconnected\n");
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            /*check if the last connected peer had not used MITM, if so, delete its bond information*/
            if (m_peer_to_be_deleted != PM_PEER_ID_INVALID)
            {
                ret_code_t ret_val = pm_peer_delete(m_peer_to_be_deleted);
                APP_ERROR_CHECK(ret_val);
                NRF_LOG_DEBUG("Collector's bond deleted\n");
                m_peer_to_be_deleted = PM_PEER_ID_INVALID;
            }
        } break; // BLE_GAP_EVT_DISCONNECTED

        case BLE_GAP_EVT_CONNECTED:
        {
            m_peer_to_be_deleted = PM_PEER_ID_INVALID;
            NRF_LOG_INFO("Connected\n");
            //err_code           = bsp_indication_set(BSP_INDICATE_CONNECTED);
            //APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            // Start Security Request timer.
            err_code = app_timer_start(m_sec_req_timer_id, SECURITY_REQUEST_DELAY, NULL);
            APP_ERROR_CHECK(err_code);
        } break; // BLE_GAP_EVT_CONNECTED

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout.\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTC_EVT_TIMEOUT

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout.\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTS_EVT_TIMEOUT

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            NRF_LOG_DEBUG("BLE_GAP_EVT_SEC_PARAMS_REQUEST\n");
            break; // BLE_GAP_EVT_SEC_PARAMS_REQUEST

        case BLE_GAP_EVT_PASSKEY_DISPLAY:
        {
            char passkey[PASSKEY_LENGTH + 1];
            memcpy(passkey, p_ble_evt->evt.gap_evt.params.passkey_display.passkey, PASSKEY_LENGTH);
            passkey[PASSKEY_LENGTH] = 0;
            // Don't send delayed Security Request if security procedure is already in progress.
            err_code = app_timer_stop(m_sec_req_timer_id);
            APP_ERROR_CHECK(err_code);

            NRF_LOG_INFO("Passkey: %s\n", nrf_log_push(passkey));
            state_show_pin(passkey);
        } break; // BLE_GAP_EVT_PASSKEY_DISPLAY

        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(m_conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break; // BLE_EVT_USER_MEM_REQUEST

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        {
            ble_gatts_evt_rw_authorize_request_t  req;
            ble_gatts_rw_authorize_reply_params_t auth_reply;

            req = p_ble_evt->evt.gatts_evt.params.authorize_request;

            if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
                {
                    if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                    err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                               &auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
        } break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST

#if (NRF_SD_BLE_API_VERSION == 3)
        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            err_code = sd_ble_gatts_exchange_mtu_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                       NRF_BLE_MAX_MTU_SIZE);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST
#endif

        default:
            // No implementation needed.
            break;
    }
    NRF_LOG_FLUSH();
}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    ble_conn_state_on_ble_evt(p_ble_evt);
    pm_on_ble_evt(p_ble_evt);
    ble_gls_on_ble_evt(&m_gls, p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
    ble_our_service_on_ble_evt(p_ble_evt, &m_our_service);
}


/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in] sys_evt  System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    // Dispatch the system event to the fstorage module, where it will be
    // dispatched to the Flash Data Storage (FDS) module.
    fs_sys_event_handler(sys_evt);

    // Dispatch to the Advertising module last, since it will check if there are any
    // pending flash operations in fstorage. Let fstorage process system events first,
    // so that it can report correctly to the Advertising module.
    ble_advertising_on_sys_evt(sys_evt);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT, PERIPHERAL_LINK_COUNT);

    // Enable BLE stack.
#if (NRF_SD_BLE_API_VERSION == 3)
    ble_enable_params.gatt_enable_params.att_mtu = NRF_BLE_MAX_MTU_SIZE;
#endif
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for erase Peer Manager bonds.
 */
void peer_manager_erase_bonds(void)
{
    uint32_t err_code;

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("Bonds erased!\n");
}


/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertising_init(void)
{
    uint32_t               err_code;
    ble_advdata_t          advdata;
    ble_advdata_t          scanrsp;
    ble_adv_modes_config_t options;

    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;;
    advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = m_adv_uuids;

    memset(&scanrsp, 0, sizeof(scanrsp));
    scanrsp.uuids_complete.uuid_cnt = sizeof(m_more_adv_uuids) / sizeof(m_more_adv_uuids[0]);
    scanrsp.uuids_complete.p_uuids  = m_more_adv_uuids;

    memset(&options, 0, sizeof(options));
    options.ble_adv_fast_enabled  = true;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = ble_advertising_init(&advdata, &scanrsp, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting advertising.
 */
void advertising_start(void)
{
    ble_advertising_start(BLE_ADV_MODE_FAST);
}

/**@brief Function for stop advertising.
 */
void advertising_stop(void)
{
    sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    sd_ble_gap_adv_stop();
}


/**@brief Function for application main entry.
 */
void ble_services_init(void)
{
    //NRF_LOG_INFO("GLS init\n");
    timers_init();
    ble_stack_init();
    peer_manager_init();
    services_init();
    gap_params_init();
    advertising_init();
    conn_params_init();
    //advertising_start();
}

/**
 * @}
 */
