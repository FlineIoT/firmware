#include "sigfox.h"
#include "boards.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrfx_spim.h"
#include "app_error.h"
#include "nrf_log.h"
#include "WSG303.h"
/*#include "s2lp.h"

static const nrfx_spim_t spi = NRFX_SPIM_INSTANCE(0);
static volatile bool spi_xfer_done = true; // Flag used to indicate that SPI instance completed the transfer.*/

static bool _init = false;
static bool _busy = false;

void sigfox_init(void)
{
    /* S2LP init
    _s2lp_init();
    NRF_LOG_INFO("SL2P init ok");*/

    WSG_uart_init();
    NRF_LOG_INFO("WSG init ok");

    _init = true;
    _busy = false;
}

void sigfox_send_test(void)
{
    /*NRF_LOG_INFO("Step 1: Reset");
    WSG_reset();
    nrf_delay_ms(50);
    NRF_LOG_INFO("Step 2: help");
    WSG_help();*/
    nrf_delay_ms(50);
    NRF_LOG_INFO("Step 3: getID");
    WSG_getID();
    nrf_delay_ms(50);
    NRF_LOG_INFO("Step 4: getPAC");
    WSG_getPac();
    /*nrf_delay_ms(100);
    NRF_LOG_INFO("Step 3: sendTest");
    WSG_send_test();*/

    NRF_LOG_INFO("Step 5: WSG_uart_send_frame");
    nrf_delay_ms(100);
    WSG_uart_send_frame("42", 2, true);
    nrf_delay_ms(10000);
    NRF_LOG_INFO("Step 6: WSG_send_test");
    WSG_send_test();
}