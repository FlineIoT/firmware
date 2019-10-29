#include "ble_fls_control.h"
#include "nrf_log.h"
#include "nrf_soc.h"
#include "app_error.h"

static ble_fls_t* _fls_handle = NULL;

static bool _notification_enabled = false;

uint32_t ble_fls_data_send(uint8_t* data_pkt, uint32_t len)
{
    APP_ERROR_CHECK_BOOL(len <= (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - 3));

    uint32_t err_code = NRF_ERROR_INVALID_STATE;
    
    if(_fls_handle->conn_handle != BLE_CONN_HANDLE_INVALID && _notification_enabled)
    {
        uint16_t               hvx_len;
        ble_gatts_hvx_params_t hvx_params = {0};

        hvx_len = len;

        hvx_params.handle = _fls_handle->data_handle.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &hvx_len;
        hvx_params.p_data = (uint8_t*)data_pkt;

        err_code = sd_ble_gatts_hvx(_fls_handle->conn_handle, &hvx_params);
        if (err_code == NRF_SUCCESS && (*(hvx_params.p_len) != len))
        {
            err_code = NRF_ERROR_DATA_SIZE;
        }
    }

    return err_code;
}

/**@brief Function for adding the FLS Processed Data characteristic.
 *
 * @param[in]   p_fls        FLSS structure.
 * @param[in]   p_fls_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t _data_char_add(ble_fls_t*            p_fls,
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

void ble_fls_data_notify(bool en)
{
    _notification_enabled = en;
}

uint32_t ble_fls_data_init(ble_fls_t* p_fls, const ble_fls_init_t* p_fls_init)
{
    APP_ERROR_CHECK_BOOL(p_fls != NULL);

    _fls_handle = p_fls;

    return _data_char_add(p_fls, p_fls_init);
}
