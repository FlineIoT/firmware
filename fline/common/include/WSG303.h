#pragma once

#include "app_util_platform.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "nrf.h"
#include "bsp.h"
#include "nrf_error.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include "nrf_delay.h"
#include "nrf_drv_wdt.h"
//#include "nrf_drv_uart.h"
#include "app_uart.h"
//#include "device_settings.h"

#define MAX_TEST_DATA_BYTES     (15U)                /**< max number of test bytes to be used for tx and rx. */
#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256                         /**< UART RX buffer size. */

/*
    RX  8
    TX  6
    CTS 7
    RTS 5
*/

enum RETURN_CODE {
    OK = 'O',
    KO = 'K',
    SENT = 'S'
};

uint8_t dummy;

unsigned long _lastSend;

typedef struct
{
    uint16_t  size;                 /**< Number of array entries. */
    uint8_t * p_data;               /**< Pointer to array entries. */
} uint8_array_t;

void uart_error_handle(app_uart_evt_t * p_event);
void uart_init(void);
void uart_send_frame (uint8_array_t data);
unsigned long SFM10R1_getPac();
unsigned long SFM10R1_getID();
void SFM10R1_send_test();
bool SFM10R1_send(const void* data, uint8_t len);
