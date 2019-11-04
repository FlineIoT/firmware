#include "sigfox.h"
#include "boards.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrfx_spim.h"
#include "app_error.h"
#include "nrf_log.h"
#include "s2lp.h"

static const nrfx_spim_t spi = NRFX_SPIM_INSTANCE(0);
static volatile bool spi_xfer_done = true; /**< Flag used to indicate that SPI instance
                                                completed the transfer. */

void sigfox_init(void)
{
    /* S2LP init */
    _s2lp_init();
    NRF_LOG_INFO("SL2P init ok");
}
