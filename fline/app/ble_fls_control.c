#include "ble_fls_control.h"
#include "nrf_log.h"
#include "nrf_soc.h"
#include "app_error.h"
#include "../../components/libraries/bootloader/nrf_bootloader_info.h"
#include "version.h"
#include "app_scheduler.h"
#include "ble_manager.h"

#define CONTROL_BUFF_SIZE 16

#define _increment_idx(idx) \
    if (++(idx) >= CONTROL_BUFF_SIZE) (idx) = 0

#define unpack_uint32(b) \
    (((b)[3] << 24) | ((b)[2] << 16) | ((b)[1] << 8) | (b)[0])
#define unpack_uint16(b) (((b)[1] << 8) | (b)[0])

static fls_control_t _cmd_buf[CONTROL_BUFF_SIZE];
static uint8_t        rd_idx = 0;
static uint8_t        wr_idx = 0;
static uint8_t        count  = 0;

static ble_fls_t* _fls_handle = NULL;

static uint32_t _send_command(void)
{
    uint32_t ret = NRF_SUCCESS;

    // TODO Check connection and if peer subscribed to notification o/ Control

    while (count)
    {
        NRF_LOG_INFO("Sending opcode: 0x%02X", _cmd_buf[rd_idx].opcode);
        ret = ble_fls_control_send(&_cmd_buf[rd_idx]);
        if (ret != NRF_SUCCESS && ret != NRF_ERROR_INVALID_STATE)
        {
            NRF_LOG_WARNING("Control char update: 0x%X", ret);
            return ret;  // come back later
        }
        else if (ret == NRF_ERROR_INVALID_STATE)  // disconnected
        {
            NRF_LOG_ERROR("INVALID STATE. Reset FLS Control queue");

            // Clear the queue
            rd_idx = 0;
            wr_idx = 0;
            count  = 0;

            return ret;
        }

        // Success, control packet can be removed from the queue
        count--;
        _increment_idx(rd_idx);
    }

    return ret;
}

/* Indication is needed by default */
static void _send_command_evt(void* evt, uint16_t size)
{
    UNUSED_PARAMETER(evt);
    UNUSED_PARAMETER(size);
    _send_command();
}

// Returns success if item was queued, regardless of queue empty status
static uint32_t _queue_command(uint32_t opcode, uint8_t* data, uint8_t len)
{
    uint32_t ret = NRF_SUCCESS;
    uint32_t err_code;

    if (len > FLS_CONTROL_DATA_MAX_LEN)
    {
        ret = NRF_ERROR_INVALID_PARAM;
    }
    else if (count == CONTROL_BUFF_SIZE)
    {
        ret = NRF_ERROR_NO_MEM;

        /* Disconnect in order to reset BLE connection / sync state */
        ble_mgr_disconnect();
    }
    else
    {
        _cmd_buf[wr_idx].opcode = (fls_opcode_t)opcode;
        _cmd_buf[wr_idx].len    = len;

        if (data != NULL) memcpy(_cmd_buf[wr_idx].data, data, len);

        count++;
        _increment_idx(wr_idx);

        err_code = app_sched_event_put(NULL, 0, _send_command_evt);
        APP_ERROR_CHECK(err_code);
    }

#ifdef DEBUG
    if (ret)
    {
        NRF_LOG_INFO("Queue command ret: %ld", ret);
    }
#endif

    return ret;
}

static uint32_t _resp_err(uint8_t opcode)
{
    return _queue_command((uint32_t)(opcode | FLS_CONTROL_RESPONSE_BIT), NULL, 0);
}

/**
 * @brief Packet written to the characteristic are going through this function
 * @details These packet can be written by either the peripheral (sent) or the
 * central (received)
 */
void ble_fls_control_evt_handler(ble_fls_t* p_fls, ble_fls_evt_t* p_evt)
{
    UNUSED_PARAMETER(p_fls);

    uint32_t err_code;

    if (p_evt->evt_type != BLE_FLS_EVT_CONTROL_PACKET_WRITTEN)
    {
        return;
    }

    fls_control_t* ctrl_pkt = p_evt->ctrl_data;

    NRF_LOG_INFO("FLS Control opcode written: 0x%02X, len: %u", (uint8_t) ctrl_pkt->opcode,
             ctrl_pkt->len);

    switch (ctrl_pkt->opcode)
    {
        case PERIPHERAL_SYS_TIME:
        break;

        case PERIPHERAL_SYS_UPTIME:
        break;

        case PERIPHERAL_SYS_REVISION:
        break;

        case PERIPHERAL_SYS_SERIAL:
        break;

        case PERIPHERAL_SYS_RESET:
        break;

        case PERIPHERAL_SYS_BOOTLOADER:
            if (ctrl_pkt->len > 1)
            {
                _resp_err(ctrl_pkt->opcode);
                break;
            }
            NRF_LOG_INFO("Reboot into bootloader");

            err_code = sd_power_gpregret_clr(0, 0xffffffff);
            APP_ERROR_CHECK(err_code);
            err_code = sd_power_gpregret_set(0, BOOTLOADER_DFU_START);
            APP_ERROR_CHECK(err_code);
            break;
        default:break;
    }
}


 
/**
 * @brief Function for sending data through control char
 * @details Send a packet through control char
 *
 * @param ctrl_pkt Packet to send (20 bytes max)
 *
 * @return Error code. NRF_SUCCESS if everything went well
 */
uint32_t ble_fls_control_send(fls_control_t* ctrl_pkt)
{
    uint32_t err_code;

    // Send value if connected and notifying
    if (_fls_handle->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        uint16_t               hvx_len;
        ble_gatts_hvx_params_t hvx_params = {0};
        uint16_t               len        = (uint16_t)(ctrl_pkt->len + 2);

        hvx_len = len;

        hvx_params.handle = _fls_handle->control_handle.value_handle;
        hvx_params.type = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &hvx_len;
        hvx_params.p_data = (uint8_t*)ctrl_pkt;

        err_code = sd_ble_gatts_hvx(_fls_handle->conn_handle, &hvx_params);

        if ((err_code == NRF_SUCCESS) && (*(hvx_params.p_len) != len))
        {
            err_code = NRF_ERROR_DATA_SIZE;
        }
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

/**@brief Function for adding the FLS Control characteristic.
 *
 * @param[in]   p_fls        FLS structure.
 * @param[in]   p_fls_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t _control_char_add(ble_fls_t*            p_fls,
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
    cccd_md.write_perm = p_fls_init->fls_control_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write    = 1;
    char_md.char_props.notify   = 1;
    // char_md.char_props.indicate = 1; // Must implement on_hvc handling
    char_md.p_char_user_desc    = NULL;
    char_md.p_char_pf           = NULL;
    char_md.p_user_desc_md      = NULL;
    char_md.p_cccd_md           = &cccd_md;
    char_md.p_sccd_md           = NULL;

    ble_uuid.type = p_fls->uuid_type;
    ble_uuid.uuid = BLE_UUID_FLS_CONTROL_CHARACTERISTIC;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_fls_init->fls_control_attr_md.read_perm;
    attr_md.write_perm = p_fls_init->fls_control_attr_md.write_perm;
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
                                           &p_fls->control_handle);
}


uint32_t ble_fls_control_init(ble_fls_t* p_fls, const ble_fls_init_t* p_fls_init)
{
    APP_ERROR_CHECK_BOOL(p_fls != NULL);

    _fls_handle = p_fls;

    return _control_char_add(p_fls, p_fls_init);
}
