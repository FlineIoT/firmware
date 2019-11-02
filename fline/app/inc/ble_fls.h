#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"
#include "ble_fls_api.h"

/* clang-format off */

#define FLS_BASE_UUID                  		    {{0x33, 0x4D, 0x08, 0x89, 0x3F, 0x1E, 0x1A, 0x12, 0xA5, 0x4D, 0xFD, 0xBF, 0x00, 0x00, 0x1C, 0xBC}} /**< Used vendor specific UUID. */
#define BLE_UUID_FLS_SERVICE 					0x0001                      /**< The UUID of the Fline Service. */
#define BLE_UUID_FLS_CONTROL_CHARACTERISTIC 	0x0002                      /**< The UUID of the Control Characteristic. */
#define BLE_UUID_FLS_DATA_CHARACTERISTIC 	    0x0003                      /**< The UUID of the Data Characteristic. */


/**@brief   Macro for defining a ble_fls instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_FLS_DEF(_name)                                     \
static ble_fls_t _name;                                        \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                             \
                     BLE_FLS_BLE_OBSERVER_PRIO,                \
                     ble_fls_on_ble_evt, &_name)

#define OPCODE_LENGTH   1                                                    /**< Length of opcode inside FLS packet. */
#define HANDLE_LENGTH   2                                                    /**< Length of handle inside FLS packet. */
#define MAX_FLS_LEN    (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH)  /**< Maximum size of a transmitted packet. */

/* clang-format on */

// Forward declaration of the ble_eqss_t type.
typedef struct ble_fls_s ble_fls_t;

/**@brief EQS Service event type. */
typedef enum {
    BLE_FLS_EVT_NOTIFICATION_ENABLED,  /**< FLS value notification enabled
                                           event. */
    BLE_FLS_EVT_NOTIFICATION_DISABLED, /**< FLS value notification disabled
                                           event. */
    BLE_FLS_EVT_CONTROL_PACKET_WRITTEN /**< FLS Control char written (only
                                           this characteristic can be written)
                                         */
} ble_fls_evt_type_t;

/**@brief EQS Service event. */
typedef struct
{
    ble_fls_evt_type_t  evt_type; /**< Type of event. */
    fls_control_t*      ctrl_data;
} ble_fls_evt_t;

/**@brief FLS event handler type. */
typedef void (*ble_fls_evt_handler_t)(ble_fls_t*     p_fls,
                                       ble_fls_evt_t* p_evt);

/**@brief FLS structure. This contains various status information for the
 * service. */
struct ble_fls_s
{
    uint8_t                uuid_type;   /**< UUID type for FLS Base UUID. */
    ble_fls_evt_handler_t  evt_handler; /**< Event handler to be called for
                                           handling events in the FLS. */
    uint16_t
        service_handle; /**< Handle of FLS (as provided by the BLE stack). */
    ble_gatts_char_handles_t
        control_handle; /**< Handle related to the Control characteristic. */
    ble_gatts_char_handles_t
        data_handle; /**< Handle related to the Data
                                   characteristic. */
    uint16_t
        conn_handle; /**< Handle of the current connection (as provided by the
                        BLE stack, is BLE_CONN_HANDLE_INVALID if not in a
                        connection). */
};

/**@brief FLS init structure. This contains all options and data needed for
 *        initialization of the service. */
typedef struct
{
    ble_fls_evt_handler_t evt_handler;                  /**< Event handler to be called for handling events in the FLS. */
    uint8_t* p_fls;                                     /**< If not NULL, initial value of the FLS Control characteristic. */
    ble_srv_cccd_security_mode_t fls_control_attr_md;   /**< Initial security level for Control attribute */
    ble_srv_cccd_security_mode_t fls_data_attr_md;      /**< Initial security level for data attribute */
} ble_fls_init_t;


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Equisense
 * Service
 *
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 * @param[in]   p_context   Equisense Service structure.
 */
void ble_fls_on_ble_evt(ble_evt_t const* p_ble_evt, void* p_context);

/**@brief Function for initializing the FLS.
 *
 * @param[out]  p_fls       FLS structure. This structure will have to be
 * supplied by the application. It will be initialized by this function, and
 * will later be used to identify this particular
 * service instance.
 * @param[in]   p_fls_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise
 * an error code.
 */
uint32_t ble_fls_init(ble_fls_t* p_fls, const ble_fls_init_t* p_fls_init);

void ble_fls_evt_handler(ble_fls_t* p_eqss, ble_fls_evt_t* p_evt);
