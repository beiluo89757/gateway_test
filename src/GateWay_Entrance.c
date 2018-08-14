#include "lucas.h"
#include "GateWay.h"

#define app_log(M, ...) custom_log("app", M, ##__VA_ARGS__)


// gateway_uart_t gateway_uart[UART_COUNT];
pthread_mutex_t stdio_tx_mutex;
GateWay_Context_t GateWay_Context;



OSStatus gateway_param_init(void)
{
    OSStatus err = -1;
    memset(&GateWay_Context, 0, sizeof(GateWay_Context));

    GateWay_Context.gw_uart_init                        = platform_uart_init;
    GateWay_Context.gw_422_handshake_fun                = gw_422_handshake_fun;
    GateWay_Context.gw_422_controller_scn_fun           = gw_422_controller_scn_fun;
    GateWay_Context.gw_422_controller_event_log_fun     = gw_422_controller_event_log_fun;
    GateWay_Context.gw_422_driver_scn_fun               = gw_422_driver_scn_fun;
    GateWay_Context.gw_422_driver_current_log_fun       = gw_422_driver_current_log_fun;

    err = 0;
exit:
    return err;
}



OSStatus gateway_init(void)
{
    OSStatus err = 0;

    pthread_t thread;

    uart_index_t uart = UART_COM1;

	if(pthread_mutex_init(&stdio_tx_mutex, NULL) < 0)
        printf(" stdio_tx_mutex init error\r\n");

    app_log("GateWay  start  working~~~~~~~~~~~~~~~~~~~");

#ifdef __APPLE__
    GateWay_Context.device_status_sem = dispatch_semaphore_create(0);
#else
    err = sem_init(&gw_param.device_status_sem,0,0);
    require_noerr(err,exit);
#endif

    err = GateWay_Context.gw_uart_init(UART_COM1,9600,DATA_WIDTH_8BIT,ODD_PARITY,STOP_BITS_1);
    require_noerr_action(err, exit, app_log("ERROR: GateWay uart inited failed."));

    
    err = pthread_create(&thread,NULL,uartRecv_task,(void*)&uart);
    require_noerr_action(err, exit, app_log("ERROR: Unable to start the uartRecv_task."));

    err = pthread_create(&thread,NULL,query_status_task,(void*)&GateWay_Context);
    require_noerr_action(err, exit, app_log("ERROR: Unable to start the query_status_task."));

    // err = pthread_create(&thread,NULL,gateway_ota_task,NULL);
    // require_noerr_action(err, exit, app_log("ERROR: Unable to start the gateway_ota_task."));


// #ifdef CLI_ENABLE
//     cli_init();
//     cli_register_commands( app_clis, sizeof(app_clis) / sizeof(struct cli_command) );
// #endif

    pthread_join(thread,NULL);
exit:
    return err;
}

extern OSStatus controller_event_log_to_json();
extern OSStatus write_to_file(char *buffer_write);
/**
 * GateWay Entrance
 * @param NULL
 * @return NULL
 */
int main(void)
{
    OSStatus err = 0;

    err = gateway_param_init();
    require_noerr(err,exit);

    err = gateway_init();
    require_noerr(err,exit);

    // iothub_client_sample_mqtt_run();
exit:
    return err;
}



