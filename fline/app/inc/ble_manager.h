#pragma once

#include "ble.h"

#define APP_BLE_CONN_CFG_TAG            1                                       /**< A tag identifying the SoftDevice BLE configuration. */

void ble_mgr_init(void);
void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context);
void ble_mgr_disconnect(void);