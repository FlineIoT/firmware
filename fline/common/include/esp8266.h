#ifndef _ESP8266_H
#define _ESP8266_H

bool ESP_status(void);
bool ESP_restart(void);
bool ESP_enter_deep_sleep(uint32_t time);
int ESP_get_sleep_mode(void);
bool ESP_set_sleep_mode(int mode);

#endif