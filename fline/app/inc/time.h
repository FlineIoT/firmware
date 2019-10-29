#pragma once

#include <stdint.h>

#define TIME_TICK_TIMER_UNDEFINED 0xFFFFFFFF

void     time_init(void);
uint32_t time_get_uptime(void);
uint32_t time_get_boottime(void);
uint32_t time_get_time(void);
void time_set_time(uint32_t time);
uint32_t time_tick(void);
uint32_t time_tock(uint32_t num);
uint8_t time_get_time_hundredth();
