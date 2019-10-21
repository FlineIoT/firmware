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
#define SPI_READ_BIT 0x01
#define SPI_WRITE_BIT 0x00
#define SPI_COMMAND_BIT 0x80
#define SPI_ADDRESS_BIT 0x00

/**
 * @brief SPI user event handler.
 *
 * Flag indicating transfer is done is set.
 * Callback function called if flushing fifo.
 *
 * @param event
 */
static void spi_event_handler(nrfx_spim_evt_t const* event, void* context)
{
    UNUSED_VARIABLE(event);
    UNUSED_VARIABLE(context);

    spi_xfer_done = true;
}

static void _s2lp_spi_init()
{
    nrfx_spim_config_t spi_config = NRFX_SPIM_DEFAULT_CONFIG;
    spi_config.mode               = NRF_SPIM_MODE_1; 
    spi_config.ss_pin             = NRFX_SPIM_PIN_NOT_USED; // S2LP_CS_PIN; // or NRFX_SPIM_PIN_NOT_USED if handled manually from here
    spi_config.miso_pin           = S2LP_MISO_PIN;
    spi_config.mosi_pin           = S2LP_MOSI_PIN;
    spi_config.sck_pin            = S2LP_SCLK_PIN;
    spi_config.frequency          = NRF_SPIM_FREQ_8M;

    APP_ERROR_CHECK(nrfx_spim_init(&spi, &spi_config, spi_event_handler, NULL));
}

static void _s2lp_reset(void)
{
    nrf_gpio_pin_set(S2LP_SDN_PIN);
    nrf_delay_ms(10);
    nrf_gpio_pin_clear(S2LP_SDN_PIN);
}

static void _s2lp_read_address(uint8_t addr, uint8_t * value)
{
    uint8_t m_tx_buf[4] = {0};
    uint8_t m_rx_buf[4] = {0};

    m_tx_buf[0] = (uint8_t)(SPI_ADDRESS_BIT | SPI_READ_BIT);    // Header
    m_tx_buf[1] = (uint8_t)(addr);                              // Address

    nrfx_spim_xfer_desc_t xfer_desc = {.p_tx_buffer = m_tx_buf,
                                       .tx_length   = 2,
                                       .p_rx_buffer = m_rx_buf,
                                       .rx_length   = 4};

    while (!spi_xfer_done)
    {
        __WFE();
    }
    spi_xfer_done = false;

    nrf_gpio_pin_clear(S2LP_CS_PIN);
    APP_ERROR_CHECK(nrfx_spim_xfer(&spi, &xfer_desc, 0));

    while (!spi_xfer_done)
    {
        __WFE();
    }

    nrf_gpio_pin_set(S2LP_CS_PIN);

    NRF_LOG_INFO("0x%02x%02x", m_rx_buf[3], m_rx_buf[2]);
    NRF_LOG_INFO("0x%02x%02x", m_rx_buf[0], m_rx_buf[1]);

    *value = m_rx_buf[2];
}

static void _s2lp_init(void)
{
    /* Put the radio off */
    nrf_gpio_cfg_output(S2LP_SDN_PIN);
    _s2lp_reset();

    /* SPI init */
    _s2lp_spi_init();

    /* Check ID */
    uint8_t dev_id = 0;
    _s2lp_read_address(S2LP_REG_DEVICE_INFO1, &dev_id);

    /* IRQ config and enable */ 

    /*** Front End Module (FEM) Initialization ***/
    /* Set XTAL frequency with offset */
    // S2LPRadioSetXtalFrequency(XTAL_FREQUENCY+XTAL_FREQUENCY_OFFSET);
    /* Set the frequency base */
    // S2LPManagementSetBand(BOARD_FREQUENCY_BAND);

    /* TCXO Initialization */
    // NO TCXO 


    // Reset chip
    _s2lp_reset();

    /* Init the Sigfox Library and the device for Sigfox communication*/
    // Call to ST_Sigfox_Init : ST_Sigfox_Init.c
    //
    
}

void sigfox_init(void)
{
    /* S2LP init */
    _s2lp_init();
    NRF_LOG_INFO("SL2P init ok");
}
