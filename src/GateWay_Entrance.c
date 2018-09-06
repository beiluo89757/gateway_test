#include "lucas.h"
#include "GateWay.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#define app_log(M, ...) custom_log("app", M, ##__VA_ARGS__)


// gateway_uart_t gateway_uart[UART_COUNT];
pthread_mutex_t stdio_tx_mutex;
GateWay_Context_t GateWay_Context;


extern OSStatus controller_event_log_to_json();
extern OSStatus write_to_file(char *buffer_write);
int lucas_queue_test();

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
extern char shake_1[];
extern char shake_2[];
extern char shake_3[];


OSStatus gw_422_handshake_fun_test(void)
{
    UartSend(UART_COM1,shake_1,1);
    msleep(280);
    UartSend(UART_COM1,shake_2,1);
    msleep(820);
    UartSend(UART_COM1,shake_3,3);

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
//EVEN_PARITY  ODD_PARITY
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
#define MSGKEY  123456
// #include <mcheck.h>
/**
 * GateWay Entrance
 * @param NULL
 * @return NULL
 */
int main(void)
{
    OSStatus err = 0;
    // mtrace();
    err = gateway_param_init();
    require_noerr(err,exit);

    err = gateway_init();
    require_noerr(err,exit);

    // iothub_client_sample_mqtt_run();
    // app_log("================================================  (CON_SCN_INDEX)  %d",sizeof(gateway_update_context.controller_events));
// while(1)
// {
//     app_log("===============================");
    
    //controller_event_log_to_json();
    // update_to_cloud();
//     sleep(5);
// }
    // lucas_queue_test();
    // while(1)
    // {
    //     sleep(3);
    //     //app_log("12345");
    // }
exit:
    exit(1);
}
#define MAX_TEXT 512  
struct msg_st  
{  
    long int msg_type;  
    char text[MAX_TEXT];  
}; 

void *queue_receive(void *arg)
{
    struct msg_st data;
    int ret;

    while(1)
    {
        ret = msgrcv(*(int *)arg,&data,sizeof(data.text),0,0);
        if(-1 == ret)
        {
            perror("msgrcv");
        }
        else
            app_log("You wrote: %s\n",data.text);  
    }
}
void *queue_send(void *arg)
{
    OSStatus ret;
    struct msg_st data;

    char buffer[100];

    while(1)
    {
        app_log("Enter some text: ");  
        fgets(buffer, BUFSIZ, stdin);  
        data.msg_type =1;
        strcpy(data.text, buffer);  
        if(msgsnd(*(int*)arg, (void*)&data, MAX_TEXT, 0) == -1)  
        {  
            fprintf(stderr, "msgsnd failed\n");  
            exit(EXIT_FAILURE);  
        } 

    }
}    

int lucas_queue_test()
{
    key_t key;

    pthread_t thread_send,thread_recv;
    int err;
    int msgid;
    // key = ftok(".", 123);
    // app_log("key = [%x]\n", key);

    msgid = msgget((key_t)12345, IPC_CREAT|0666);

    if(-1 == msgid)
    {
        app_log("msgid");
        exit(1);
    }
    else
        app_log("msgid %d",msgid);

    err = pthread_create(&thread_send,NULL,queue_send,(void *)&msgid);
    require_noerr_action(err, exit, app_log("ERROR: queue_send task failed."));
    err = pthread_create(&thread_recv,NULL,queue_receive,(void *)&msgid);
    require_noerr_action(err, exit, app_log("ERROR: queue_receive task failed."));

    pthread_join(thread_send,NULL);

exit:
    return err;
}




