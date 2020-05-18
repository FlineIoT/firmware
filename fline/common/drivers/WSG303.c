#include "WSG303.h"

#include "boards.h"

#ifdef FACTORY_TESTS
#define IOT_ERROR(...)
#define IOT_WARN(...)
#else
#include "iot_trace.h"
#endif

#define MAX_TEST_DATA_BYTES     (15U)

void WSG_uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}

void WSG_uart_init(void)
{
    uint32_t err_code;

    /*uint8_t *rx_buf;
    uint8_t *tx_buf;*/

    const app_uart_comm_params_t comm_params =
      {
          WSG303S_TX_PIN,
          WSG303S_RX_PIN,
          RTS_PIN_NUMBER,
          CTS_PIN_NUMBER,
          APP_UART_FLOW_CONTROL_DISABLED,
          false,
          UART_BAUDRATE_BAUDRATE_Baud115200
      };

    /*const app_uart_buffers_t buffers =
        {
            rx_buf,
            UART_RX_BUF_SIZE,
            tx_buf,
            UART_TX_BUF_SIZE,  
        };*/
    //app_uart_init(&comm_params, &buffers, uart_error_handle, APP_IRQ_PRIORITY_LOWEST);

    APP_UART_FIFO_INIT(&comm_params,
                         UART_RX_BUF_SIZE,
                         UART_TX_BUF_SIZE,
                         WSG_uart_error_handle,
                         APP_IRQ_PRIORITY_LOWEST,
                         err_code);


    APP_ERROR_CHECK(err_code);
}

void cfg_bin_2_hexadecimal(const uint8_t *pBin, int binSize, char *pHexadecimal)
{
    uint8_t data, dH, dL;
    int i;

    for(i = 0; i < binSize; i++)
    {
        data = *pBin++;
        dH = (data >> 4);
        dL = (data & 0x0F);
        if(dH < 10)
            *pHexadecimal++ = ('0'+ dH);
        else
            *pHexadecimal++ = ('A'+ (dH-10));
        if(dL < 10)
            *pHexadecimal++ = ('0'+ dL);
        else
            *pHexadecimal++ = ('A'+ (dL-10));
    }
    *pHexadecimal = 0;
}

void WSG_send_header_old(void) {
    app_uart_put((uint8_t)'S');
    app_uart_put((uint8_t)'e');
    app_uart_put((uint8_t)'n');
    app_uart_put((uint8_t)'d');
    app_uart_put((uint8_t)'_');
    app_uart_put((uint8_t)'f');
    app_uart_put((uint8_t)'r');
    app_uart_put((uint8_t)'a');
    app_uart_put((uint8_t)'m');
    app_uart_put((uint8_t)'e');
    app_uart_put((uint8_t)' ');
}

//0 for DL, 1 for UL
void WSG_send_footer_old(bool is_downlink) {
    app_uart_put((uint8_t)' ');
    app_uart_put((uint8_t)'2'); //repetitions
    app_uart_put((uint8_t)' ');
    if (is_downlink) {
        app_uart_put((uint8_t)'0');
    } else {
        app_uart_put((uint8_t)'1');
    }
    app_uart_put((uint8_t)'\n');
    app_uart_put((uint8_t)'\r');
}

void WSG_uart_send_frame(const char* data, uint16_t size, bool is_downlink) {
	uint8_t* bytes = (uint8_t*)data;
    WSG_send_header_old();
    for (uint8_t i = 0; i < size; ++i) {
        app_uart_put((uint8_t)bytes[i]);
    }
    WSG_send_footer_old(is_downlink);
}

void WSG_send_test(){
    const char * payload = "Send_frame 1234 2 0\n";
    uint8_array_t data;
    data.p_data = (uint8_t *) payload;
    for (uint8_t i = 0; i < 23; ++i) {
        app_uart_put((uint8_t)data.p_data[i]);
    }
    //WSG_uart_send_frame(data);
}


unsigned long WSG_getID() {
    //app_uart_put((uint8_t)'\0');
    app_uart_put((uint8_t)'G');
    app_uart_put((uint8_t)'e');
    app_uart_put((uint8_t)'t');
    app_uart_put((uint8_t)'_');
    app_uart_put((uint8_t)'I');
    app_uart_put((uint8_t)'D');
    app_uart_put((uint8_t)'\n');
    app_uart_put((uint8_t)'\r');

    //Response is [byte1, byte2, ..., byteN, 'O', 'K']
    uint8_t response[8] = {0};
    uint8_t i = 0;

    for (int i=0; i<=7; i++) {
        response[i] = app_uart_get(&dummy);
        //while(!_serial.available());
        ++i;
    }
    app_uart_get(&dummy); //';'

    unsigned long id = 0;

    for(uint8_t j = 0; j < i-2; ++j) {
        id += response[j] << ((i-3-j) * 8);
    }

    NRF_LOG_INFO("ID: %d", id);
    //IOT_INFO("ID: %d", id);
    return id;
}

unsigned long WSG_getPac() {
    //app_uart_put((uint8_t)'\0');
    app_uart_put((uint8_t)'G');
    app_uart_put((uint8_t)'e');
    app_uart_put((uint8_t)'t');
    app_uart_put((uint8_t)'_');
    app_uart_put((uint8_t)'P');
    app_uart_put((uint8_t)'A');
    app_uart_put((uint8_t)'C');
    app_uart_put((uint8_t)'\n');
    app_uart_put((uint8_t)'\r');
    //uint8_t * tx_data = (uint8_t *)("AT$I=11;");
    
    uint8_t response[16] = {0};

    for (int i=0; i<=15; i++) {
        response[i] = app_uart_get(&dummy);
        //while(!_serial.available());
        ++i;
    }

    unsigned long pac = 0;

    for(uint8_t j = 0; j < 16; ++j) {
        pac += response[j] << (j * 8);
    }
    
    NRF_LOG_INFO("PAC: %d", pac);
    //IOT_INFO("PAC: %d", pac);
    return pac;
}

void WSG_reset() {
    app_uart_put((uint8_t)'R');
    app_uart_put((uint8_t)'e');
    app_uart_put((uint8_t)'s');
    app_uart_put((uint8_t)'e');
    app_uart_put((uint8_t)'t');
    app_uart_put((uint8_t)'\r');
}

void WSG_help() {
    app_uart_put((uint8_t)'h');
    app_uart_put((uint8_t)'e');
    app_uart_put((uint8_t)'l');
    app_uart_put((uint8_t)'p');
    app_uart_put((uint8_t)'\r');
}
