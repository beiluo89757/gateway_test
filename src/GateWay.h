#pragma once

#include "lucas.h"
#include "Device_Protocol.h"

typedef int (*GATEWAY_FUNC)(void);
typedef int (*GATEWAY_UART_INIT_FUNC)(uart_index_t uart ,u_int32_t baud_rate, platform_uart_data_width_t data_width,platform_uart_parity_t parity,platform_uart_stop_bits_t stop_bits);



typedef struct _GateWay_Context{
#ifdef __APPLE__
    dispatch_semaphore_t    device_status_sem;
#else
    sem_t                   device_status_sem;
#endif    
    GATEWAY_UART_INIT_FUNC   gw_uart_init;             
    GATEWAY_FUNC             gw_422_handshake_fun;      
           
    GATEWAY_FUNC             gw_422_controller_scn_fun;                                           
    GATEWAY_FUNC             gw_422_controller_event_log_fun;

    GATEWAY_FUNC             gw_422_driver_scn_fun;
    GATEWAY_FUNC             gw_422_driver_current_log_fun;

} GateWay_Context_t;






OSStatus gw_422_handshake_fun(void);

OSStatus gw_422_controller_scn_fun(void);
OSStatus gw_422_controller_event_log_fun(void);

OSStatus gw_422_driver_scn_fun(void);
OSStatus gw_422_driver_current_log_fun(void);

void query_status_task(void *arg);
void uartRecv_task(void *arg);

void gateway_ota_task(void *arg);


