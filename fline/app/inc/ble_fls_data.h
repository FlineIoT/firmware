#pragma once

#include "stdint.h"
#include "ble_fls_api.h"
#include "ble_fls.h"

uint32_t ble_fls_data_send(uint8_t* data_pkt, uint32_t len);
void ble_fls_data_notify(bool en);
uint32_t ble_fls_data_init(ble_fls_t* p_fls, const ble_fls_init_t* p_fls_init);
