/** @file
 * 
 * @brief Main file. Starting point of the application
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "nrf_delay.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "bsp_btn_ble.h"
#include "nrf_pwr_mgmt.h"
#include "app_scheduler.h"
#include "ble_manager.h"
#include "version.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "time.h"
#include "sigfox.h"
#include "bme280.h"

#define SCHED_MAX_EVENT_DATA_SIZE       APP_TIMER_SCHED_EVENT_DATA_SIZE             /**< Maximum size of scheduler events. */
#define SCHED_QUEUE_SIZE                20                                          /**< Maximum number of events in the scheduler queue. */

#define APP_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
#define MAIN_INTERVAL_MS                2000 //Should be 630000

APP_TIMER_DEF(main_wakeup_timer_id);
APP_TIMER_DEF(main_sec_tick_timer_id);
bool main_wakeup_evt_expired = false;
uint8_t   m_main_sec_tick;


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    // Initialize timer module.
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for the Event Scheduler initialization.
 */
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    app_sched_execute();
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}

static void main_sec_tick_timer_handler(void * p_context)
{
    m_main_sec_tick++;
    //cPrintLog(CDBG_MAIN_LOG, "main_sec_tick_timer_handler\n");
    /*if (send_state) {
        uint8_t payload_data[12];
        for (int i=0; i<=11; i++){
            payload_data[i]=0x12;
        }
        send_state = false;
        Sigfox_send_payload(payload_data);
    }*/
}

static void main_sec_tick_timer_init(void)
{
    uint32_t err_code;

    err_code = app_timer_create(&main_sec_tick_timer_id, APP_TIMER_MODE_REPEATED, main_sec_tick_timer_handler);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(main_sec_tick_timer_id, APP_TIMER_TICKS(1000), NULL);
    APP_ERROR_CHECK(err_code);

}

static void main_wakeup_timer_timeout_handler(void * p_context)
{
    NRF_LOG_INFO("Wakeup timer expired\n");
    main_wakeup_evt_expired = true;
}

static void main_wakeup_timer_init(void)
{
    uint32_t err_code;
    err_code = app_timer_create(&main_wakeup_timer_id, APP_TIMER_MODE_REPEATED, main_wakeup_timer_timeout_handler);
    APP_ERROR_CHECK(err_code);

}
static void main_wakeup_timer_start(uint32_t msec)
{
    uint32_t err_code;
    uint32_t timeout_ticks; 

    timeout_ticks = APP_TIMER_TICKS(msec);
    NRF_LOG_INFO("Wakeup start ticks:%d\n", timeout_ticks);
    err_code = app_timer_start(main_wakeup_timer_id, timeout_ticks, NULL);  //timer to wake up every 600s
    APP_ERROR_CHECK(err_code);

}

/**@brief Function for application main entry.
 */
int main(void)
{
    // Initialize.
    log_init();
    NRF_LOG_INFO("Fline v%u,%u,%u", FIRMWARE_VERSION_MAJOR, FIRMWARE_VERSION_MINOR, FIRMWARE_VERSION_PATCH);

    timers_init();
    time_init();    // Internal clock init

    power_management_init();
    ble_stack_init();
    scheduler_init();

    /* Init BLE services */
    ble_mgr_init();

    NRF_LOG_INFO("Pre sigfox_init");
    sigfox_init();
    NRF_LOG_INFO("Pre sigfox_send_test");
    sigfox_send_test();
    NRF_LOG_INFO("Post sigfox_send_test");


    nrf_delay_ms(200);
    NRF_LOG_INFO("BME280 Init");
    
    BME280_Ret BME280RetVal = bme280_init();
    if (BME280_RET_OK == BME280RetVal) {
        NRF_LOG_INFO("BME280 init Done\r\n");
    }
    else {
        NRF_LOG_ERROR("BME280 init Failed: Error Code: %d\r\n", (int32_t)BME280RetVal); 
    }

    //setup BME280 if present
    uint8_t conf = bme280_read_reg(BME280REG_CTRL_MEAS);
    NRF_LOG_INFO("CONFIG: %x\r\n", conf);

    bme280_set_oversampling_hum(BME280_OVERSAMPLING_1);
    bme280_set_oversampling_temp(BME280_OVERSAMPLING_1);
    bme280_set_oversampling_press(BME280_OVERSAMPLING_1);

    conf = bme280_read_reg(BME280REG_CTRL_MEAS);
    //NRF_LOG_INFO("CONFIG: %x\r\n", conf);
    //Start sensor read for next pass
    bme280_set_mode(BME280_MODE_FORCED);
    NRF_LOG_INFO("BME280 configuration done\r\n");

    static int32_t raw_t  = 0;
    static uint32_t raw_p = 0;
    static uint32_t raw_h = 0;


    for(int i = 0; i <=5; i++)
    {

        NRF_LOG_INFO("Reading BME 280 values: \n");
        raw_t = bme280_get_temperature();
        raw_p = bme280_get_pressure() * 0.003906; // (/25600*100);
        raw_h = bme280_get_humidity() * 0.097656;// (/1024*100)
        //bme280_set_mode(BME280_MODE_SLEEP);

        NRF_LOG_INFO("temperature: %d, pressure: %d, humidity: %d \r\n", raw_t, raw_p, raw_h);
    }

    main_wakeup_timer_init();
    main_sec_tick_timer_init();
    main_wakeup_timer_start(MAIN_INTERVAL_MS);  // Here you shoud set up a timer to wake up every 618s

    // Enter main loop.
    for (;;)
    {
        if (main_wakeup_evt_expired) { //Able to send sigfox message
            NRF_LOG_INFO("Timer expired temperature: %d, pressure: %d, humidity: %d \r\n", raw_t, raw_p, raw_h);
            main_wakeup_evt_expired=false;
            break; 
            
        }
        idle_state_handle();
    }
}


/**
 * @}
 */
