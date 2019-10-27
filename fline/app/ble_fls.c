#include "ble_fls.h"
#include "string.h"
#include "ble_srv_common.h"
#include "nordic_common.h"
#include "nrf_log.h"
#include "ble_fls_control.h"
#include "app_error.h"

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_fls       FLS structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_fls_t* p_fls, ble_evt_t* p_ble_evt)
{
    p_fls->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}

/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_fls       FLS structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_fls_t* p_fls, ble_evt_t* p_ble_evt)
{
    p_fls->conn_handle = BLE_CONN_HANDLE_INVALID;
}

/**@brief Function for handling write events to the FLS Data characteristic.
 *
 * @param[in]   p_fls         FLS structure.
 * @param[in]   p_evt_write   Write event received from the BLE stack.
 */
static void on_fls_data_cccd_write(
    ble_fls_t* p_fls, ble_gatts_evt_write_t* p_evt_write)
{
    if (p_evt_write->len == 2)
    {
        // CCCD written, update notification state
        if (p_fls->evt_handler != NULL)
        {
            ble_fls_evt_t evt;

            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                evt.evt_type = BLE_FLS_EVT_NOTIFICATION_ENABLED;
                NRF_LOG_INFO("Subscribed to notifications on Data char");
            }
            else
            {
                evt.evt_type = BLE_FLS_EVT_NOTIFICATION_DISABLED;
                NRF_LOG_INFO("Unsubscribed to Data char");
            }

            p_fls->evt_handler(p_fls, &evt);
        }
    }
}


/**
 * @brief Function for handling value written to the control char CCCD
 *  Warns the Fline Service Control Char module
 *
 * @param p_fls FLS service handle
 * @param p_evt_write GATTS encapsulation of data written to control char
 */
static void on_fls_control_cccd_write(ble_fls_t*            p_fls,
                                       ble_gatts_evt_write_t* p_evt_write)
{
    if (p_evt_write->len == 2)
    {
        // CCCD written, update notification state
        if (p_fls->evt_handler != NULL)
        {
            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                NRF_LOG_INFO("Subscribed to notification on Control char");
            }
            else
            {
                NRF_LOG_INFO("Unsubscribed to Control char");
            }
        }
    }
}

/**
 * @brief Function for handling value written to the control char values desc
 *
 * @param p_fls FLS service handle
 * @param p_evt_write GATTS encapsulation of data written to control char
 */
static void on_fls_control_value_write(ble_fls_t*            p_fls,
                                        ble_gatts_evt_write_t* p_evt_write)
{
    if (p_evt_write->len >= 2)  // 2 byte header plus length field
    {
        if (p_fls->evt_handler != NULL)
        {
            ble_fls_evt_t evt;
            evt.evt_type  = BLE_FLS_EVT_CONTROL_PACKET_WRITTEN;
            evt.ctrl_data = (fls_control_t*) p_evt_write->data;

            /* Make sure the received control opcode length is correct 
             * {opcode (1B), length (1B), data[length] (length bytes)} */
            if (p_evt_write->len == (evt.ctrl_data->len + 2))
            {
                p_fls->evt_handler(p_fls, &evt);
                return;
            }
            else
            {
                NRF_LOG_ERROR("Control packet length wrong");
            }
        }
    }
    else
    {
        NRF_LOG_ERROR("Control packet must be at least 2 bytes");
    }
}

/**@brief Function for handling the Write event.
 *
 * @param[in]   p_fls       FLS structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_fls_t* p_fls, ble_evt_t* p_ble_evt)
{
    ble_gatts_evt_write_t* p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    /* Peer un/subscribed to notifications on Data char? */
    if (p_evt_write->handle == p_fls->data_handle.cccd_handle)
    {
        on_fls_data_cccd_write(p_fls, p_evt_write);
    }

    /* Peer un/subscribed to notifications/indications on Control char? */
    if (p_evt_write->handle == p_fls->control_handle.cccd_handle)
    {
        on_fls_control_cccd_write(p_fls, p_evt_write);
    }

    /* Data written to control char */
    if (p_evt_write->handle == p_fls->control_handle.value_handle)
    {
        on_fls_control_value_write(p_fls, p_evt_write);
    }
}


/**@brief Function for handling the HVC event.
 *
 * @details Handles HVC events from the BLE stack.
 *
 * @param[in]   p_fls       Fline service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_hvc(ble_fls_t* p_fls, ble_evt_t* p_ble_evt)
{
    /* HVC not available without Indication */
}

/**
 * @brief Function for handling event with BLE connection
 * @details Parse event and dispatch
 *
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 * @param[in]   p_context   Fline Service structure.
 */
void ble_fls_on_ble_evt(ble_evt_t const* p_ble_evt, void* p_context)
{
    if ((p_context == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    ble_fls_t* p_fls = (ble_fls_t*)p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_fls, (ble_evt_t*)p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_fls, (ble_evt_t*)p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_fls, (ble_evt_t*)p_ble_evt);
            break;

        case BLE_GATTS_EVT_HVC:
            on_hvc(p_fls, (ble_evt_t*)p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**
 * @brief Handle events on Fline service
 * 
 * @param p_fls             FLS structure.
 * @param p_evt             Event info
 */
void ble_fls_evt_handler(ble_fls_t* p_fls, ble_fls_evt_t* p_evt)
{
    // If control char written, send to FLS Control event handler:
    if(p_evt->evt_type == BLE_FLS_EVT_CONTROL_PACKET_WRITTEN)
    {
        ble_fls_control_evt_handler(p_fls, p_evt);
    }
}



/**@brief Function for adding the FLS Processed Data characteristic.
 *
 * @param[in]   p_fls        FLSS structure.
 * @param[in]   p_fls_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t fls_data_char_add(ble_fls_t*            p_fls,
                                             const ble_fls_init_t* p_fls_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    uint8_t             data_packet[] = {0};

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    cccd_md.write_perm =
        p_fls_init->fls_data_attr_md.cccd_write_perm;
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write  = 0;
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_fls->uuid_type;
    ble_uuid.uuid = BLE_UUID_FLS_DATA_CHARACTERISTIC;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_fls_init->fls_data_attr_md.read_perm;
    attr_md.write_perm = p_fls_init->fls_data_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(fls_control_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = MAX_FLS_LEN;
    attr_char_value.p_value   = data_packet;

    return sd_ble_gatts_characteristic_add(p_fls->service_handle, &char_md,
                                           &attr_char_value,
                                           &p_fls->data_handle);
}

/**
 * @brief Init FLS service
 * @details Add service and characteristics
 *
 * @param p_fls FLS service handler
 * @param p_fls_init Init params
 *
 * @return NRF_SUCCESS or error code
 */
uint32_t ble_fls_init(ble_fls_t* p_fls, const ble_fls_init_t* p_fls_init)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t fls_base_uuid = FLS_BASE_UUID;

    // Initialize service structure
    p_fls->evt_handler = p_fls_init->evt_handler;
    p_fls->conn_handle = BLE_CONN_HANDLE_INVALID;

    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&fls_base_uuid, &p_fls->uuid_type);
    APP_ERROR_CHECK(err_code);

    ble_uuid.type = p_fls->uuid_type;
    ble_uuid.uuid = BLE_UUID_FLS_SERVICE;

    // Add service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid,
                                        &p_fls->service_handle);
    APP_ERROR_CHECK(err_code);

    // Add Control characteristic
    err_code = ble_fls_control_init(p_fls, p_fls_init);
    APP_ERROR_CHECK(err_code);

    // Add Data characteristic
    err_code = fls_data_char_add(p_fls, p_fls_init);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}