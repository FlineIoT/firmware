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
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
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

void WSG_uart_send_frame (uint8_array_t data) {
    extern uint8_t payload[12 * 2 + 1]; //TODO: rely on data.size
    memset(payload, 0x00, sizeof(payload)); //TODO: rely on data.size
    cfg_bin_2_hexadecimal(data.p_data, data.size, (char *)payload);

    for (uint8_t i = 0; i < data.size; ++i) {
        app_uart_put((uint8_t)data.p_data[i]);
    }
}

void WSG_send_test(){
    const char * payload = "AT$Send_frame 1234 2 0";
    uint8_array_t data;
    data.p_data = (uint8_t *) payload;
    /*for (uint8_t i = 0; i < sizeof(payload); ++i) {
        app_uart_put((uint8_t)data[i]);
    }*/
    WSG_uart_send_frame(data);
}

bool WSG_send(const void* data, uint8_t len) {
	uint8_t* bytes = (uint8_t*)data;

    /*if(!isReady()) {
        return false;
    }*/

    //_lastSend = millis();

    //app_uart_put((uint8_t)'\0');
    app_uart_put((uint8_t)'A');
    app_uart_put((uint8_t)'T');
    app_uart_put((uint8_t)'$');
    app_uart_put((uint8_t)'S');
    app_uart_put((uint8_t)'F');
    app_uart_put((uint8_t)'=');
    for(uint8_t i = 0; i < len; ++i) {
        app_uart_put(bytes[i]);
    }
    app_uart_put(';');

    /*uint8_t ok = SFM10R1_nextReturn();
    if(ok == OK) {
        SFM10R1_nextReturn(); //SENT
        return true;
    }*/
    return false;
}

unsigned long WSG_getPac() {
    //app_uart_put((uint8_t)'\0');
    app_uart_put((uint8_t)'A');
    app_uart_put((uint8_t)'T');
    app_uart_put((uint8_t)'$');
    app_uart_put((uint8_t)'I');
    app_uart_put((uint8_t)'=');
    app_uart_put((uint8_t)'1');
    app_uart_put((uint8_t)'1');
    app_uart_put((uint8_t)';');
    app_uart_put((uint8_t)'\r');
    /*uint8_t * tx_data = (uint8_t *)("AT$I=11;");

    // Start sending one byte and see if you get the same
    for (uint32_t i = 0; i < MAX_TEST_DATA_BYTES; i++)
    {
        while (app_uart_put(tx_data[i]) != NRF_SUCCESS);

        nrf_delay_ms(2);
    }*/

    //Response is [byte1, byte2, ..., byteN, 'O', 'K']
    /*uint8_t response[8] = {0};
    uint8_t i = 0;
    //while(!_serial.available());
    //while(_serial.peek() != ';') {
    for (int i=0; i==2; i++) {
        response[i] = app_uart_get(&dummy);
        //while(!_serial.available());
        ++i;
    }
    app_uart_get(&dummy); //';'


    for(uint8_t j = 0; j < i-2; ++j) {
        id += response[j] << ((i-3-j) * 16);
    }*/

    unsigned long id = 20;
    return id;
}

unsigned long WSG_getID() {
    //app_uart_put((uint8_t)'\0');
    app_uart_put((uint8_t)'A');
    app_uart_put((uint8_t)'T');
    app_uart_put((uint8_t)'$');
    app_uart_put((uint8_t)'I');
    app_uart_put((uint8_t)'=');
    app_uart_put((uint8_t)'1');
    app_uart_put((uint8_t)'0');
    app_uart_put((uint8_t)';');
    app_uart_put((uint8_t)'\r');

    //Response is [byte1, byte2, ..., byteN, 'O', 'K']
    uint8_t response[8] = {0};
    uint8_t i = 0;
    //while(!_serial.available());
    //while(_serial.peek() != ';') {

    for (int i=0; i==2; i++) {
        response[i] = app_uart_get(&dummy);
        //while(!_serial.available());
        ++i;
    }
    app_uart_get(&dummy); //';'

    unsigned long id = 0;

    for(uint8_t j = 0; j < i-2; ++j) {
        id += response[j] << ((i-3-j) * 8);
    }

    return id;
}
