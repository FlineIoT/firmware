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
