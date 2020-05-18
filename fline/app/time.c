#include <stdint.h>
#include <string.h>

#include "app_timer.h"
#include "time.h"
#include "nrfx_wdt.h"
#include "nrf_log.h"

#define CLOCK_TIMEOUT_INTERVAL APP_TIMER_TICKS(1000)  // 1 second

typedef struct
{
    uint32_t last_update;
    uint32_t ticks;
    uint32_t boottime;
} rtc_t;

static uint32_t m_boottime = 0;
static rtc_t    m_uptime   = {0, 0, 0};

static uint32_t _time_sec = 0;

APP_TIMER_DEF(m_time_timer_id);

#define TICK_TOCK_TIMER_COUNT 4

static uint32_t _tick[TICK_TOCK_TIMER_COUNT];

static void time_timeout_handler(void* p_context)
{
    uint32_t now;

    // Count ticks since last timer expiry
    now = app_timer_cnt_get();
    m_uptime.ticks += 0xFFFFFF & (now - m_uptime.last_update);

    m_uptime.last_update = now;

    // Move ticks into seconds
    while (m_uptime.ticks >= 32768)
    {
        ++_time_sec;
        m_uptime.ticks -= 32768;
    }

    /* Feed the wdt in the clock*/
    // TODO WDT
    // nrfx_wdt_feed();
}

void time_init()
{
    uint32_t err_code;

    err_code = app_timer_create(&m_time_timer_id, APP_TIMER_MODE_REPEATED,
                                time_timeout_handler);

    APP_ERROR_CHECK(err_code);

    // Start application timers
    err_code = app_timer_start(m_time_timer_id, CLOCK_TIMEOUT_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}

uint32_t time_get_uptime()
{
    return _time_sec - m_boottime;
}

uint32_t time_get_boottime() { return m_boottime; }
uint32_t time_get_time() { return _time_sec; }
uint8_t  time_get_time_hundredth()
{
    uint32_t ticks_since_last_second_inc =
        app_timer_cnt_diff_compute(app_timer_cnt_get(), m_uptime.last_update);

    return (uint8_t)((uint32_t)(ticks_since_last_second_inc * 100) /
                     APP_TIMER_CLOCK_FREQ);
}

void time_set_time(uint32_t time)
{
    _time_sec = time;
    NRF_LOG_INFO("Sync -> time: %lu uptime: %lu boottime: %lu",
             time_get_time(), time_get_uptime(), time_get_boottime());
}

uint32_t time_tick()
{
    uint32_t i = 0;
    for (i = 0; i < TICK_TOCK_TIMER_COUNT; ++i)
    {
        if (_tick[i] == 0)
        {
            _tick[i] = app_timer_cnt_get();

            return i;
        }
    }

    NRF_LOG_ERROR("No more tick-tock timers");
    APP_ERROR_CHECK_BOOL(0);
    return 0;
}

uint32_t time_tock(uint32_t num)
{
    APP_ERROR_CHECK_BOOL(num < TICK_TOCK_TIMER_COUNT);

    uint32_t diff = 0;

    if (_tick[num] != 0)
    {
        uint32_t tock = app_timer_cnt_get();
        diff          = app_timer_cnt_diff_compute(tock, _tick[num]);
    }

    _tick[num] = 0;

    return diff;
}
