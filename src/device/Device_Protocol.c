#include "Device_Protocol.h"

#define device_log(M, ...) custom_log("Device", M, ##__VA_ARGS__)

extern GateWay_Context_t GateWay_Context;
extern void printRawData (char type, char *p, int len);

time_t seconds_start;
char *read_start_time[30]={0};
char *read_end_time[20]={0};
char *controller_end_time[20]={0};

const char head[]    = {0x1b, 0x5b, 0x48};
const char shake_1[] = {0x4d};
const char shake_2[] = {0x0d};
const char shake_3[] = {0x0d, 0x4d, 0x4d};

uint8_t recv_buf[FRAME_LENTH*3] = {0};

uint8_t previous_status[FRAME_LENTH] = {0};


uint8_t current_status[FRAME_LENTH] = {0};

update_status_t gateway_update_context;

char *controller_event_string ="{\"number\":%d,\"text\":\"%s\",\"cnt\":%d,\"age\":%d,\"position\":%d}";
char *drive_event_string ="{\"number\":%d,\"text\":\"%s\",\"time\":\"%s\",\"cnt\":%d}";
char *controller_scn_string ="\"%s\"";
char *drive_scn_string ="\"%s\"";

void uart_data_process(char *buffer, int datalen)
{      
    int err = 0;
    
    //printRawData(0,buffer,datalen);
    memcpy(recv_buf, buffer, datalen);
    lower_to_upper(recv_buf,datalen);
    if(0!=memcmp("CLOCK",recv_buf+3,5))
    {
        if((!memcmp(recv_buf,recv_buf+FRAME_LENTH,FRAME_LENTH))&&(!memcmp(recv_buf,recv_buf+2*FRAME_LENTH,FRAME_LENTH)))
        {
            memcpy(current_status, recv_buf, sizeof(current_status));      
            printRawData (CUR_STATUS, current_status, sizeof(current_status));
        }
    }
    else
    {
        memcpy(current_status, recv_buf, sizeof(current_status));      
        printRawData (CUR_STATUS, current_status, sizeof(current_status));       
    }
}
OSStatus wait_device_status()
{
    OSStatus err = -1;
    
    for(int i = 0;  i<50; i++)
    {
        msleep(50);
        if(0 != memcmp(current_status, previous_status ,FRAME_LENTH))
        {
            memcpy(previous_status,current_status,FRAME_LENTH);
            err = 0;
            break;
        }
    }
exit:
    return err;
}
OSStatus gw_422_handshake_fun(void)
{
    UartSend(UART_COM1,shake_1,1);
    msleep(280);
    UartSend(UART_COM1,shake_2,1);
    msleep(820);
    UartSend(UART_COM1,shake_3,3);
    wait_device_status();
    if(!memcmp("DISCONNECTED",current_status+23,12))
        return HAND_SHAKE;
    else
        return CON_SCN;
}



OSStatus gw_422_controller_event_log_fun(void)
{
    int err = 0;
    int ret = 0;
    int goon_count = 0;
    index_t index = CON_SCN_INDEX;
    gateway_status_t gateway_status={0};

    memset(gateway_update_context.controller_events,0,sizeof(gateway_update_context.controller_events));
    memset(&gateway_update_context.controller_status,0,sizeof(gateway_update_context.controller_status));
    gateway_update_context.controller_events_num = 0;

    while(1)
    {   
        switch(index)
        {
            case CON_SCN_INDEX:
                device_log("================================================  (CON_SCN_INDEX)");
                if(!memcmp("disconnected",current_status+23,12))
                {
                    memcpy(previous_status,current_status,FRAME_LENTH);
                    index = HAND_SHAKE;
                }
                else if(!memcmp("1:GECB",current_status+3,6))
                {
                    memcpy(previous_status,current_status,FRAME_LENTH);
                    index = CON_SCN_GECB_MENU;

                }
                else
                {             
                    UartSend(UART_COM1,"M",1);   
                    msleep(500);
                    UartSend(UART_COM1,"M",1);
                    if(0 == wait_device_status())
                    {
                        if(!memcmp("1:GECB",current_status+3,6))
                            index = CON_SCN_GECB_MENU;
                    }
                }
                break;
            case CON_SCN_GECB_MENU:
                setStrTime(read_start_time);

                device_log("================================================  (CON_SCN_GECB_MENU)\n");
                UartSend(UART_COM1,"1",1); 
                wait_device_status();
                if(!memcmp("  GECB  - MENU",current_status+3,14))
                    index = CON_SCN_SYSTEM_MENU;
                else        
                    index = CON_SCN_INDEX;
                break;
            case CON_SCN_SYSTEM_MENU:
                device_log("================================================  (CON_SCN_SYSTEM_MENU)\n");
                UartSend(UART_COM1,"1",1);   
                wait_device_status();
                if(!memcmp(" SYSTEM - MENU",current_status+3,14))
                    index = CON_SCN_TEST_MENU;
                else        
                    index = CON_SCN_INDEX;
                break;

            case CON_SCN_TEST_MENU:
                device_log("================================================  (CON_SCN_TEST_MENU)\n");
                UartSend(UART_COM1,"2",1);  
                wait_device_status();
                if(!memcmp("  TEST  - MENU",current_status+3,14))
                    index = CON_SCN_TEST_INFO;
                else        
                    index = CON_SCN_INDEX;
                break;
            case CON_SCN_TEST_INFO:
                device_log("================================================  (EVENT_LOG_INFO)\n");

                UartSend(UART_COM1,"1",1);  
                wait_device_status();   
                if(!memcmp("SAVEDRUNS", current_status+10, 9))
                {
                    char tmp[6]={0};
                    memcpy(tmp, current_status+4, 5);
                    gateway_update_context.controller_status.no_of_runs = atoi(tmp);
                    //device_log("================================================   no_of_runs : [%d]",gateway_status.controller_status.no_of_runs);
                    if(!memcmp("SAVEDMINS", current_status+28, 9))
                    {
                        char tmp[7]={0};
                        memcpy(tmp, current_status+21, 6);
                        gateway_update_context.controller_status.age = atoi(tmp);
                        //device_log("================================================   age : [%d]",gateway_status.controller_status.age);
                    }
                    index = CON_SCN_TEST_INFO_GOON;
                }
                else
                    index = CON_SCN_INDEX; 
                break; 
            case CON_SCN_TEST_INFO_GOON:
                device_log("================================================  (CON_SCN_TEST_INFO_GOON)\n");
                if(goon_count++ >= 100)
                {
                    index = DRIVE_SCN;
                    break;
                }
                UartSend(UART_COM1,">",1);  

                if(0 == wait_device_status())
                {
                    if(!memcmp("     LOGGED     ",current_status+21,16))
                    {
                        device_log("############   collect event log    ########################");
                        index = DRIVE_SCN;
                    }  
                    else if(current_status[21]=='C'&&current_status[26]=='T'&&current_status[34]=='P')
                    {
                        char tmp_num[4]={0};
                        char tmp_cnt[4]={0};
                        char tmp_age[7]={0};
                        char tmp_possition[3]={0};
                        memcpy(tmp_num, current_status+3, 4);
                        gateway_status.controller_events.number = atoi(tmp_num);
                        memcpy(gateway_status.controller_events.text, current_status+8, 10);
                        memcpy(tmp_cnt, current_status+22, 3);
                        gateway_status.controller_events.cnt = atoi(tmp_cnt);
                        memcpy(tmp_age, current_status+27, 6);
                        gateway_status.controller_events.age = atoi(tmp_age);
                        memcpy(tmp_possition, current_status+35, 2);
                        gateway_status.controller_events.position = atoi(tmp_possition);

                        if(gateway_update_context.controller_events_num<100)
                        {
                                sprintf(gateway_update_context.controller_events[gateway_update_context.controller_events_num++].event_log,controller_event_string,
                                gateway_status.controller_events.number,
                                gateway_status.controller_events.text,
                                gateway_status.controller_events.cnt,
                                gateway_status.controller_events.age,
                                gateway_status.controller_events.position);
                        }
                        index = CON_SCN_TEST_INFO_GOON;                                        
                    } 
                }           
                break;
            default:
                break;
        }

        if(index == HAND_SHAKE || index == CON_SCN || index == DRIVE_SCN)
        {
            // if(index == DRIVE_SCN)
            // {
            //     setStrTime(controller_end_time); 
            //     index = DRIVE_CURRENT_LOG;
            // }
            err =  index;
            goto exit;
        }    
    }   
exit:
    return err;
}

OSStatus gw_422_controller_scn_fun(void)
{
    OSStatus err =0;

    int goon_count = 0;
    int START_GOON;
    int CON_SCN_ERR = 0;
    index_t index = CON_SCN_INDEX;
    
    gateway_status_t gateway_status={0};

    uint8_t controller_scn_first_page[FRAME_LENTH] = {0};

    memset(gateway_update_context.controller_scn,0,sizeof(gateway_update_context.controller_scn));
    gateway_update_context.controller_scn_num = 0;

    while(1)
    {   
        switch(index)
        {
            case CON_SCN_INDEX:
                device_log("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  (CON_SCN_INDEX)");
                
                if(!memcmp("DISCONNECTED",current_status+23,12))
                {
                    memcpy(previous_status,current_status,FRAME_LENTH);
                    index = HAND_SHAKE;
                }
                else if(!memcmp("1:GECB",current_status+3,6))
                {
                    memcpy(previous_status,current_status,FRAME_LENTH);
                    index = CON_SCN_GECB_MENU;
                }
                else
                {             
                    UartSend(UART_COM1,"M",1);   
                    msleep(300);
                    UartSend(UART_COM1,"M",1);
                    if(0 == wait_device_status())
                    {
                        if(!memcmp("1:GECB",current_status+3,6))
                            index = CON_SCN_GECB_MENU;
                    }
                }
                printRawData (CUR_STATUS, current_status, sizeof(current_status));
                break;
            case CON_SCN_GECB_MENU:
                device_log("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  (CON_SCN_GECB_MENU)\n");
                UartSend(UART_COM1,"1",1); 
                wait_device_status();
                if(!memcmp("  GECB  - MENU",current_status+3,14))
                    index = CON_SCN_SYSTEM_MENU;
                else        
                    index = CON_SCN_INDEX;
                break;
            case CON_SCN_SYSTEM_MENU:
                device_log("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  (CON_SCN_SYSTEM_MENU)\n");
                UartSend(UART_COM1,"1",1);   
                wait_device_status();
                if(!memcmp(" SYSTEM - MENU",current_status+3,14))
                    index = CON_SCN_TEST_MENU;
                else        
                    index = CON_SCN_INDEX;
                break;

            case CON_SCN_TEST_MENU:
                device_log("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  (CON_SCN_TEST_MENU)\n");
                UartSend(UART_COM1,"2",1);  
                wait_device_status();
                if(!memcmp("  TEST  - MENU",current_status+3,14))
                    index = CON_SCN_TEST_INFO;
                else        
                    index = CON_SCN_INDEX;
                break;
            case CON_SCN_TEST_INFO:
                device_log("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  (CON_SCN_TEST_INFO)\n");
                UartSend(UART_COM1,"3",1);  
                if(0 == wait_device_status())
                {
                    if(!memcmp("NO.  ", current_status+3, 4))
                    {
                        device_log("1~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

                        if(!memcmp("30780",current_status+3, 14)||!memcmp("30782", current_status+3, 14))
                        {
                            device_log("2~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
                            memcpy(gateway_status.controller_status.scn,current_status+25,11);
                            sprintf(gateway_update_context.controller_scn[gateway_update_context.controller_scn_num++].scn,controller_scn_string,gateway_status.controller_status.scn);
                            index = CON_SCN_EVENT_LOG;
                        }
                        else
                            index = CON_SCN_TEST_INFO_GOON; 
                    }
                    else
                        index = CON_SCN_TEST_INFO_GOON; 
                }
                else
                   index = CON_SCN_INDEX;              
                break; 
            case CON_SCN_TEST_INFO_GOON:
                device_log("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  (CON_SCN_TEST_INFO_GOON)\n");
                if(goon_count++ >= CONTROLLER_SCN_INFO_MAX_10)
                {
                    if(gateway_update_context.controller_scn_num==0)
                        index = DRIVE_SCN;
                    else
                        index = CON_EVENT_LOG;
                    break;
                }
                UartSend(UART_COM1,">",1); 
                wait_device_status();
                if(START_GOON == 0)
                {
                    START_GOON = 1;
                    memcpy(controller_scn_first_page,current_status,37);
                }
                else 
                {
                    if(0==memcmp(controller_scn_first_page,current_status,37))
                    {
                        index = CON_EVENT_LOG;
                        break;
                    }
                }
                if(!memcmp("BASELINE USED:",current_status+3,14))
                {
                    if(!memcmp("NO.  ",current_status+21,4))
                    {
                        CON_SCN_ERR = 0;
                        for(int i=0; i<gateway_update_context.controller_scn_num; i++)
                        {
                            if((0 != memcmp(gateway_update_context.controller_scn[gateway_update_context.controller_scn_num].scn,current_status+21,11))||\
                               (0 != memcmp(gateway_update_context.controller_scn[gateway_update_context.controller_scn_num].scn,"           ",11)))
                            {
                                CON_SCN_ERR = -1;
                                break;
                            }
                        }
                        if(CON_SCN_ERR ==0){
                            memcpy(gateway_status.controller_status.scn,current_status+21,11);
                            if(gateway_update_context.controller_scn_num<20)
                                sprintf(gateway_update_context.controller_scn[gateway_update_context.controller_scn_num++].scn,controller_scn_string,gateway_status.controller_status.scn);
                            device_log("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~222  %s\n",gateway_update_context.controller_scn[gateway_update_context.controller_scn_num-1].scn);
                        }
                        // if(0 != memcmp(gateway_status.controller_status.scn,current_status+21,11))
                        // {
                        //     device_log("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  33333\n");
                        //     memcpy(gateway_status.controller_status.scn,current_status+21,11);
                        //     if(gateway_update_context.controller_scn_num<20)
                        //     {
                        //         sprintf(gateway_update_context.controller_scn[gateway_update_context.controller_scn_num++].scn,controller_scn_string,gateway_status.controller_status.scn);
                        //         device_log("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~222  %s\n",gateway_update_context.controller_scn[gateway_update_context.controller_scn_num-1].scn);
                        //     }
                        // }   
                        // else
                        //     index = CON_EVENT_LOG;
                    }
                    // else if(!memcmp("GP",current_status+21,2))/////////////////
                    // {
                    //     if(0 !=memcmp(gateway_status.controller_status.scn,current_status+25,11))
                    //     {
                    //         memcpy(gateway_status.controller_status.scn,current_status+25,11);
                    //         if(gateway_update_context.controller_scn_num<20)
                    //             sprintf(gateway_update_context.controller_scn[gateway_update_context.controller_scn_num++].scn,controller_scn_string,gateway_status.controller_status.scn);
                    //     }
                    //     else
                    //         index = CON_EVENT_LOG;
                    // }
                }  
                printRawData (CUR_STATUS, current_status, sizeof(current_status));
                break;
            default:
                break;
        }
         
        if(index == HAND_SHAKE || index == CON_EVENT_LOG || index == DRIVE_SCN)
        {
            err =  index;
            goto exit;
        }   
    }
exit:
    return err;
}
OSStatus gw_422_driver_scn_fun(void)
{
    OSStatus err =0;
    int goon_count = 0, START_GOON = 0;
    index_t index = DRIVE_SCN_INDEX;

    gateway_status_t gateway_status={0};
    
    uint8_t drive_scn_first_page[FRAME_LENTH] = {0};

    memset(gateway_update_context.drive_scn,0,sizeof(gateway_update_context.drive_scn));
    memset(&gateway_update_context.drive_status,0,sizeof(gateway_update_context.drive_status));
    gateway_update_context.drive_scn_num = 0;
    while(1)
    {   
        switch(index)
        {
            case DRIVE_SCN_INDEX:
                device_log("+++++++++++++++++++++++++++++++++++++++++++++++ (DRIVE_SCN_INDEX)");
                if(!memcmp("disconnected",current_status+23,12))
                {
                    memcpy(previous_status,current_status,FRAME_LENTH);
                    index = HAND_SHAKE;
                }
                else if(!memcmp("DRIVE",current_status+13,5))
                {
                    memcpy(previous_status,current_status,FRAME_LENTH);
                    index = DRIVE_SCN_SYSTEM;
                }
                else
                {             
                    UartSend(UART_COM1,"M",1);   
                    msleep(300);
                    UartSend(UART_COM1,"M",1);
                    if(0 == wait_device_status())
                    {
                        if(!memcmp("DRIVE",current_status+13,5))
                            index = DRIVE_SCN_SYSTEM;
                        else
                            index = UPDATE_EVENT_LOG;
                    }
                    else
                        index = UPDATE_EVENT_LOG;
                }
                printRawData (CUR_STATUS, current_status, sizeof(current_status));

                break;
            case DRIVE_SCN_SYSTEM:
                device_log("+++++++++++++++++++++++++++++++++++++++++++++++  (DRIVE_SCN_SYSTEM)\n");
                UartSend(UART_COM1,"2",1); 
                if(0 == wait_device_status())
                {
                    if(!memcmp("DRIVE SYSTEM   4",current_status+3,16))
                    {

                        index = DRIVE_SCN_MONITOR;
                    }
                    else
                        index = DRIVE_SCN_STATUS;
                }
                else
                    index = DRIVE_SCN_INDEX;
                printRawData (CUR_STATUS, current_status, sizeof(current_status));
                break;
            case DRIVE_SCN_MONITOR:
                device_log("+++++++++++++++++++++++++++++++++++++++++++++++  (DRIVE_SCN_MONITOR)\n");
                UartSend(UART_COM1,"4",1);   
                wait_device_status();
                if(!memcmp("              <>",current_status+21,16))
                    index = DRIVE_SCN_STATUS;
                else        
                    index = DRIVE_SCN_INDEX;
                printRawData (CUR_STATUS, current_status, sizeof(current_status));
                break;

            case DRIVE_SCN_STATUS:
                device_log("+++++++++++++++++++++++++++++++++++++++++++++++  (DRIVE_SCN_STATUS)\n");
                UartSend(UART_COM1,"1",1);  
                wait_device_status();
                if(!memcmp("              <>",current_status+21,16))
                    index = DRIVE_SCN_INFO;
                else        
                    index = DRIVE_SCN_INDEX;
                printRawData (CUR_STATUS, current_status, sizeof(current_status));
                break;
            case DRIVE_SCN_INFO:
                device_log("+++++++++++++++++++++++++++++++++++++++++++++++  (DRIVE_SCN_INFO)\n");

                UartSend(UART_COM1,"1",1);  
                wait_device_status();
                if(!memcmp(" --", current_status+16, 3)||!memcmp("-- ", current_status+16, 3)||!memcmp(" UP", current_status+16, 3)||!memcmp(" DN", current_status+16, 3))
                {
                    index = DRIVE_SCN_INFO_GOON;
                }
                else
                    index = DRIVE_SCN_INDEX;     
                printRawData (CUR_STATUS, current_status, sizeof(current_status));      
                break; 
            case DRIVE_SCN_INFO_GOON:
                device_log("+++++++++++++++++++++++++++++++++++++++++++++++  (DRIVE_SCN_INFO_GOON)\n");
                UartSend(UART_COM1,">",1); 

                wait_device_status();
                if(START_GOON == 0)
                {
                    START_GOON = 1;
                    memcpy(drive_scn_first_page,current_status,37);
                }
                else 
                {
                    if(0==memcmp(drive_scn_first_page,current_status,37))
                    {
                        index = DRIVE_CURRENT_LOG;
                        break;
                    }
                }
                if(0==memcmp("DRIVE APP SCN",current_status+3,13)||!memcmp("GDCB-SW SCN",current_status+3,11)||!memcmp("PRIMARY LDR SCN",current_status+3,15))
                {
                    memcpy(gateway_status.drive_status.scn,current_status+24,5);
                    if(gateway_update_context.drive_scn_num<20)
                    {
                        sprintf(gateway_update_context.drive_scn[gateway_update_context.drive_scn_num++].scn,drive_scn_string,gateway_status.drive_status.scn);
                        device_log("+++++++++++++++++++++++++++++++++++++++++++++++ %s \n",gateway_update_context.drive_scn[gateway_update_context.drive_scn_num]);

                    }

                    //device_log("+++++++++++++++++++++++++++++++++++++++++++++++ %s \n",gateway_status.drive_status.scn);
                } 
                if((0==memcmp(current_status+2+16+2+5,":",1))&&(0==memcmp(current_status+2+16+2+8,":",1))&&(0==memcmp(current_status+2+16+2+11,":",1))&&(0==memcmp(current_status+2+16+2+14,".",1)))
                {
                    device_log("perhapers be clock");
                    if(!memcmp("CLOCK",current_status+3,5))
                    {
                        device_log("Must be clock");
                        memcpy(gateway_update_context.drive_status.time,current_status+21,16);
                        device_log("+++++++++++++++++++++++++++++++++++++++++++++++ %s \n",gateway_status.drive_status.time);
                    }
                } 
                printRawData (lucas_status, current_status, sizeof(current_status));      
                break;
            default:
                break;
        }
         device_log("+++++++++++++++++++++++++++++++++++++++++++++++ %s \n",gateway_status.drive_status.time);
        if(index == HAND_SHAKE || index == DRIVE_CURRENT_LOG || index == UPDATE_EVENT_LOG)
        {
            err =  index;
            goto exit;
        }   
    }
exit:
    return err;
}

OSStatus gw_422_driver_current_log_fun(void)
{
    int err =0;
    int goon_count = 0, clock_count = 0 ;
    index_t index = DRIVE_SCN_INDEX;
    gateway_status_t gateway_status={0};

    memset(gateway_update_context.drive_events,0,sizeof(gateway_update_context.drive_events));
    gateway_update_context.drive_events_num = 0;
    gateway_status.drive_events.cnt = 1;
    while(1)
    {   
        switch(index)
        {
            case DRIVE_SCN_INDEX:
                device_log("------------------------------------------------ (DRIVE_SCN_INDEX)");

                if(!memcmp("disconnected",current_status+23,12))
                {
                    memcpy(previous_status,current_status,FRAME_LENTH);
                    index = HAND_SHAKE;
                }
                else if(!memcmp("DRIVE",current_status+13,5))
                {
                    memcpy(previous_status,current_status,FRAME_LENTH);
                    index = DRIVE_SCN_SYSTEM;
                }
                else
                {             
                    UartSend(UART_COM1,"M",1);   
                    msleep(300);
                    UartSend(UART_COM1,"M",1);
                    if(0 == wait_device_status())
                    {
                        if(!memcmp("DRIVE",current_status+13,5))
                            index = DRIVE_SCN_SYSTEM;
                        else
                            index = UPDATE_EVENT_LOG;
                    }
                    else
                        index = DRIVE_SCN_INDEX;
                }
                printRawData (CUR_STATUS, current_status, sizeof(current_status));      

                break;
            case DRIVE_SCN_SYSTEM:
                device_log("------------------------------------------------  (DRIVE_SCN_SYSTEM)\n");
                UartSend(UART_COM1,"2",1); 
                if(0 == wait_device_status())
                {
                    if(!memcmp("DRIVE SYSTEM   4",current_status+3,16))
                        index = DRIVE_SCN_MONITOR;
                    else

                        index = DRIVE_VIEW_CURRENT;
                }
                else
                    index = DRIVE_SCN_INDEX;
                                    printRawData (CUR_STATUS, current_status, sizeof(current_status));      

                break;
            case DRIVE_SCN_MONITOR:
                device_log("------------------------------------------------  (DRIVE_SCN_MONITOR)\n");
                UartSend(UART_COM1,"4",1);   
                wait_device_status();
                if(!memcmp("              <>",current_status+21,16))
                    index = DRIVE_VIEW_CURRENT;
                else        
                    index = DRIVE_SCN_INDEX;
                                    printRawData (CUR_STATUS, current_status, sizeof(current_status));      

                break;

            case DRIVE_VIEW_CURRENT:
                device_log("------------------------------------------------  (DRIVE_VIEW_CURRENT)\n");
                UartSend(UART_COM1,"2",1);  
                wait_device_status();
                if(!memcmp("              <>",current_status+21,16))
                    index = DRIVE_CURRENT_INFO;
                else        
                    index = DRIVE_SCN_INDEX;
                                    printRawData (CUR_STATUS, current_status, sizeof(current_status));      

                break;
            case DRIVE_CURRENT_INFO:
                device_log("------------------------------------------------  (DRIVE_CURRENT_INFO)\n");
                UartSend(UART_COM1,"1",1);  
                if(0 == wait_device_status())
                {
                    char tmp[4]={0};
                    memcpy(tmp, current_status+3, 3);
                    memcpy(gateway_status.drive_events.text, current_status+7, 12);
                    memcpy(gateway_status.drive_events.time, current_status+21, 16);
                    gateway_status.drive_events.number = atoi(tmp); 
                    if(gateway_update_context.drive_events_num<100)
                        sprintf(gateway_update_context.drive_events[gateway_update_context.drive_events_num++].event_log,drive_event_string,
                        gateway_status.drive_events.number,
                        gateway_status.drive_events.text,
                        gateway_status.drive_events.time,
                        gateway_status.drive_events.cnt);
                    index =  DRIVE_CURRENT_INFO_GOON;
                }
                else
                    index = DRIVE_SCN_INDEX;  
                printRawData (CUR_STATUS, current_status, sizeof(current_status));      
         
                break; 
            case DRIVE_CURRENT_INFO_GOON:
                device_log("------------------------------------------------  (DRIVE_CURRENT_INFO_GOON)\n");
                if(goon_count++>=70)
                {
                    index = UPDATE_EVENT_LOG;
                }
                UartSend(UART_COM1,">",1); 

                if(0 == wait_device_status())
                {
                    if(0 != memcmp("END OF LIST", current_status+3, 11))
                    {
                        char tmp_1[4]={0};
                        memcpy(tmp_1, current_status+3, 3);
                        memcpy(gateway_status.drive_events.text, current_status+7, 12);
                        memcpy(gateway_status.drive_events.time, current_status+21, 16);
                        gateway_status.drive_events.number = atoi(tmp_1); 

                        if(gateway_update_context.drive_events_num<100)
                            sprintf(gateway_update_context.drive_events[gateway_update_context.drive_events_num++].event_log,drive_event_string,
                            gateway_status.drive_events.number,
                            gateway_status.drive_events.text,
                            gateway_status.drive_events.time,
                            gateway_status.drive_events.cnt);

                        index =  DRIVE_CURRENT_INFO_GOON;
                    }
                    else
                        index = UPDATE_EVENT_LOG;
                }              
                printRawData (CUR_STATUS, current_status, sizeof(current_status));      

                break;
            default:
                break;
        }
         
        if(index == HAND_SHAKE || index == UPDATE_EVENT_LOG)
        {
            err =  index;
            goto exit;
        }   
    }
exit:
    return err;
}

void *query_status_task(void *arg)
{
    index_t index = CON_SCN;

    GateWay_Context_t gateway = *(GateWay_Context_t*)arg;
    while(1)
    {   
        switch(index)
        {
            case HAND_SHAKE:  
                device_log("#################     start handshake    #################");
                index = gateway.gw_422_handshake_fun(); 
                break;
            case CON_SCN:
                
                device_log("################# start collect scn info #################");
                if(gateway_update_context.controller_scn_num==0)
                {
                    seconds_start = time((time_t *)NULL);
                    index = gateway.gw_422_controller_scn_fun();
                }
                else
                    index = CON_EVENT_LOG;
                break;
            case CON_EVENT_LOG:
                seconds_start = time((time_t *)NULL);
                device_log("################# start collect controller event_log info #################");
                index = gateway.gw_422_controller_event_log_fun(); 
                break;
            case DRIVE_SCN:
                device_log("################# start collect drive scn info #################");
                if(gateway_update_context.drive_scn_num==0)
                    index = gateway.gw_422_driver_scn_fun(); 
                else
                    index = DRIVE_CURRENT_LOG;
                break;
            case DRIVE_CURRENT_LOG:
                device_log("################# start collect drive event_log info #################");
                index = gateway.gw_422_driver_current_log_fun(); 
                break;
            case UPDATE_EVENT_LOG:
                 device_log("################# start update  event_log  #################");
                // sleep(5);
                update_to_cloud();
                //controller_event_log_to_json();
                // goto exit;
                index = CON_EVENT_LOG;
                break;
            default:
                break;
        }
    }
exit:
    pthread_exit(NULL);
}


// void *query_status_task(void *arg)
// {
//     index_t index = DRIVE_SCN;

//     GateWay_Context_t gateway = *(GateWay_Context_t*)arg;

//     while(1)
//     {
//         sleep(10);
//         // update_to_cloud();
//     }
// exit:
//     pthread_exit(NULL);
// }

