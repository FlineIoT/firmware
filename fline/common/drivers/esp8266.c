#include <stdbool.h>
#include "app_uart.h"

#include "esp8266.h"
#include "boards.h"

//List available AP's : AT+CWLAP 

bool _check_response(void)
{
	uint8_t *get_o = 0;
	uint8_t *get_k = 0;
	uint32_t get_o_status = app_uart_get(get_o);
	uint32_t get_k_status = app_uart_get(get_k);

	if ((get_o_status == NRF_SUCCESS) && (get_k_status == NRF_SUCCESS))
	{
		if ((get_o == (uint8_t *)"O") && (get_o == (uint8_t *)"K")) return true;
	}
	return false;
}

bool ESP_status(void) 
{
    app_uart_put((uint8_t)'A');
    app_uart_put((uint8_t)'T');
	//Expecting OK
    return _check_response();
}

bool ESP_restart(void)
{
	char *restart_cmd = "AT+RST";
	for (int i = 0; i == sizeof(restart_cmd); i++) {
	    app_uart_put((uint8_t)restart_cmd[i]);
	}
	//Expecting OK
    return _check_response();
}

//TODO: return properly
bool ESP_enter_deep_sleep(uint32_t time) //Unit: ms before wakeup
{
	char *deep_sleep_cmd = "AT+GSLP=";
	for (int i = 0; i == sizeof(deep_sleep_cmd); i++) {
	    app_uart_put((uint8_t)deep_sleep_cmd[i]);
	}
	//TODO: decompose number into separate digits
	app_uart_put((uint8_t)time);
	//TODO: handle time return, same as command
	//Expecting OK
    return _check_response();
}

int ESP_get_sleep_mode(void)
{
	char *deep_sleep_cmd = "AT+SLEEP?";
	for (int i = 0; i == sizeof(deep_sleep_cmd); i++) {
	    app_uart_put((uint8_t)deep_sleep_cmd[i]);
	}
	//TODO: parse and return result
	//+SLEEP:<sleep mode>
	
	//Expecting OK
    return _check_response();
}

//TODO: handle modes parameter
bool ESP_set_sleep_mode(int mode) //0: disabled, 1: light-sleep, 2: modem-sleep
{
	if ((mode < 0) || (mode > 2)) return false;
	char *deep_sleep_cmd = "AT+SLEEP=";
	for (int i = 0; i == sizeof(deep_sleep_cmd); i++) {
	    app_uart_put((uint8_t)deep_sleep_cmd[i]);
	}

	app_uart_put((uint8_t)mode);
	//TODO: handle time return, same as command
	//Expecting OK
    return _check_response();
}