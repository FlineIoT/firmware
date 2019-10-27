#pragma once

#include "stdint.h"
#include "ble_fls_api.h"
#include "ble_fls.h"

void ble_fls_control_evt_handler(ble_fls_t* p_fls, ble_fls_evt_t* p_evt);

uint32_t ble_fls_control_send(fls_control_t* ctrl_pkt);
uint32_t ble_fls_control_init(ble_fls_t* p_fls, const ble_fls_init_t* p_fls_init);
