#include "Device_Protocol.h"

#define device_log(M, ...) custom_log("Device", M, ##__VA_ARGS__)

extern GateWay_Context_t GateWay_Context;
extern void printRawData (char type, char *p, int len);

char *read_start_time[30]={0};
char *read_end_time[20]={0};
char *controller_end_time[20]={0};

const char head[]    = {0x1b, 0x5b, 0x48};
const char shake_1[] = {0x4d};
const char shake_2[] = {0x0d};
const char shake_3[] = {0x0d, 0x4d, 0x4d};

uint8_t recv_buf[FRAME_LENTH*3] = {0};

uint8_t previous_status[FRAME_LENTH] = {0};

uint8_t current_status[FRAME_LENTH] = {0x1b,0x5b, 0x48, 0x20, 0x20, 0x6c ,0x6f,  0x63, 0x61, 0x6c, 
                                        0x20, 0x53, 0x56, 0x54, 0x20, 0x20, 0x20, 0x20, 0x20, 0x0d, 
                                        0x0a, 0x20, 0x20, 0x64, 0x69, 0x73, 0x63, 0x6f,  0x6e, 0x6e, 
                                        0x65, 0x63, 0x74, 0x65, 0x64, 0x20, 0x20};
update_status_t gateway_update_context;




void uart_data_process(char *buffer, int datalen)
{      
    int err = 0;
    
    //printRawData(0,buffer,datalen);
    memcpy(recv_buf, buffer, datalen);
    if((!memcmp(recv_buf,recv_buf+FRAME_LENTH,FRAME_LENTH))&&(!memcmp(recv_buf,recv_buf+2*FRAME_LENTH,FRAME_LENTH)))
    {
        lower_to_upper(recv_buf,FRAME_LENTH);
        memcpy(current_status, recv_buf, sizeof(current_status));      
        printRawData (CUR_STATUS, current_status, sizeof(current_status));
    }
}
OSStatus wait_device_status()
{
    int err = -1;
    for(int i = 0;  i<40; i++)
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
    JSON_Value* events = NULL;
    JSON_Object* events_root;
    



    JSON_Value *events_vaule = json_value_init_array();
    JSON_Array *events_arr = json_value_get_array(events_vaule);
    if(gateway_update_context.controller_events!=NULL)
    {
        free(gateway_update_context.controller_events);
        gateway_update_context.controller_events=NULL; 
    }
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
     
                        events = json_value_init_object();
                        events_root = json_object(events);
                        json_object_set_number(events_root, "number", gateway_status.controller_events.number);
                        json_object_set_string(events_root, "text", gateway_status.controller_events.text);
                        json_object_set_number(events_root, "cnt", gateway_status.controller_events.cnt);
                        json_object_set_number(events_root, "age", gateway_status.controller_events.age);
                        json_object_set_number(events_root, "position", gateway_status.controller_events.position);

                        json_array_append_value(events_arr,events);

                        index = CON_SCN_TEST_INFO_GOON;                                        
                    } 
                }           
                break;
            default:
                break;
        }

        if(index == HAND_SHAKE || index == CON_SCN || index == DRIVE_SCN)
        {
            if(index == DRIVE_SCN)
            {
                setStrTime(controller_end_time);
                index = DRIVE_CURRENT_LOG;
                char* encoded = json_serialize_to_string(events_vaule);   
                gateway_update_context.controller_events = calloc(strlen(encoded)+1,1);
                strcpy(gateway_update_context.controller_events,encoded);                     
                free(encoded); 
                device_log("================================================    \r\n%s",gateway_update_context.controller_events);   

            }
            err =  index;
            goto exit;
        }    
    }   
exit:
    if(events_vaule!=NULL)
    {
        json_value_free(events_vaule); 
    }

    return err;
}

OSStatus gw_422_controller_scn_fun(void)
{
    OSStatus err =0;

    int goon_count = 0;

    index_t index = CON_SCN_INDEX;
    gateway_status_t gateway_status={0};
    memset(&gateway_update_context.controller_status,0,sizeof(gateway_update_context.controller_status));

    JSON_Value *scn_value = json_value_init_array();
    JSON_Array *scn_arr = json_value_get_array(scn_value);
    if(gateway_update_context.controller_scn!=NULL)
    {
        free(gateway_update_context.controller_scn);
        gateway_update_context.controller_scn=NULL; 
    }
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
                            json_array_append_string(scn_arr,gateway_status.controller_status.scn);
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
                if(goon_count++ >= CONTROLLER_SCN_INFO_MAX)
                {
                    if(gateway_status.controller_status.scn[0]==0)
                        index = DRIVE_SCN;
                    else
                        index = CON_EVENT_LOG;
                    break;
                }
                UartSend(UART_COM1,">",1); 
                if(0 == wait_device_status())
                {
                    if(!memcmp("BASELINE USED:",current_status+3,14))
                    {
                        if(!memcmp("NO.  ",current_status+21,4))
                        {
                            if(0 != memcmp(gateway_status.controller_status.scn,current_status+21,11))
                            {
                                memcpy(gateway_status.controller_status.scn,current_status+21,11);
                                json_array_append_string(scn_arr,gateway_status.controller_status.scn);
                            }
                            else
                                index = CON_EVENT_LOG;
                        }
                        else
                        {
                            if(0 !=memcmp(gateway_status.controller_status.scn,current_status+25,11))
                            {
                                memcpy(gateway_status.controller_status.scn,current_status+25,11);
                                json_array_append_string(scn_arr,gateway_status.controller_status.scn);
                            }
                            else
                                index = CON_EVENT_LOG;
                        }
                    }  
                }
                break;
            default:
                break;
        }
         
        if(index == HAND_SHAKE || index == CON_EVENT_LOG || index == DRIVE_SCN)
        {
            if(index == CON_EVENT_LOG)
            {
                char* controller_scn = json_serialize_to_string(scn_value);          
                gateway_update_context.controller_scn = calloc(strlen(controller_scn)+1,1);    
                strcpy(gateway_update_context.controller_scn,controller_scn);            
                free(controller_scn); 
            }
            err =  index;
            goto exit;
        }   
    }
exit:
    if(scn_value!=NULL)
    {
        json_value_free(scn_value); 
    }
    return err;
}
OSStatus gw_422_driver_scn_fun(void)
{
    OSStatus err =0;
    int goon_count = 0, clock_count = 0 ;
    index_t index = DRIVE_SCN_INDEX;
    gateway_status_t gateway_status={0};
    memset(&gateway_update_context.drive_status,0,sizeof(gateway_update_context.drive_status));
    JSON_Value *drive_scn_value = json_value_init_array();
    JSON_Array *drive_scn_arr = json_value_get_array(drive_scn_value);
    if(gateway_update_context.drive_scn!=NULL)
    {
        free(gateway_update_context.drive_scn);
        gateway_update_context.drive_scn=NULL; 
    }
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
                if(goon_count++ >= DRIVE_SCN_INFO_MAX)
                {
                    index = DRIVE_SCN_INFO_CLOCK;
                }
                UartSend(UART_COM1,">",1); 

                wait_device_status();
                if(!memcmp("DRIVE APP SCN",current_status+3,13)||!memcmp("GDCB-SW SCN",current_status+3,11)||!memcmp("PRIMARY LDR SCN",current_status+3,15))
                {
                    if(!memcmp("NO.  ",current_status+21,4))
                    {
                        
                        if(0!= memcmp(gateway_status.drive_status.scn,current_status+25,11))
                        {
                            memcpy(gateway_status.drive_status.scn,current_status+25,11);
                            json_array_append_string(drive_scn_arr,gateway_status.drive_status.scn);
                        }
                    }
                    else
                    {
                        if(0!= memcmp(gateway_status.drive_status.scn,current_status+21,11))
                        {
                            memcpy(gateway_status.drive_status.scn,current_status+25,11);
                            json_array_append_string(drive_scn_arr,gateway_status.drive_status.scn);
                        }
                    }
                    device_log("+++++++++++++++++++++++++++++++++++++++++++++++ %s \n",gateway_status.drive_status.scn);
                }  
                printRawData (CUR_STATUS, current_status, sizeof(current_status));      

                break;
            case DRIVE_SCN_INFO_CLOCK:
                device_log("+++++++++++++++++++++++++++++++++++++++++++++++  (DRIVE_SCN_INFO_CLOCK)\n");
                if(clock_count++ >= DRIVE_SCN_CLOCK_MAX)
                {
                    index = DRIVE_CURRENT_LOG;
                }
                UartSend(UART_COM1,">",1); 
                wait_device_status();
                if(!memcmp("CLOCK",current_status+3,5))
                {
                    memcpy(gateway_update_context.drive_status.time,current_status+21,16);
                    index = DRIVE_CURRENT_LOG;
                }
                printRawData (CUR_STATUS, current_status, sizeof(current_status));      
                break;
            default:
                break;
        }
         
        if(index == HAND_SHAKE || index == DRIVE_CURRENT_LOG || index == UPDATE_EVENT_LOG)
        {
            if(index == DRIVE_CURRENT_LOG)
            {
                char* drive_scn = json_serialize_to_string(drive_scn_value);          
                gateway_update_context.drive_scn = calloc(strlen(drive_scn)+1,1);    
                strcpy(gateway_update_context.drive_scn,drive_scn);            
                free(drive_scn);   
                device_log("++++++++++++++++++++++ \r\n %s",gateway_update_context.drive_scn);             
            }
            err =  index;
            goto exit;
        }   
    }
exit:
    if(drive_scn_value!=NULL)
    {
        json_value_free(drive_scn_value);
    }
    return err;
}
OSStatus gw_422_driver_current_log_fun(void)
{
    int err =0;
    int goon_count = 0, clock_count = 0 ;
    index_t index = DRIVE_SCN_INDEX;
    gateway_status_t gateway_status={0};
    JSON_Value *events = NULL;
    JSON_Object *events_root;
    JSON_Value *events_vaule = json_value_init_array();
    JSON_Array *events_arr = json_value_get_array(events_vaule);
    if(gateway_update_context.drive_events!=NULL)
    {
        free(gateway_update_context.drive_events);
        gateway_update_context.drive_events=NULL; 
    }
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

                    events = json_value_init_object();
                    events_root = json_object(events);
                    json_object_set_number(events_root, "number", gateway_status.drive_events.number);
                    json_object_set_string(events_root, "text", gateway_status.drive_events.text);
                    json_object_set_string(events_root, "time", gateway_status.drive_events.time);
                    json_object_set_number(events_root, "cnt", gateway_status.drive_events.cnt);

                    json_array_append_value(events_arr,events);
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

                        events = json_value_init_object();
                        events_root = json_object(events);
                        json_object_set_number(events_root, "number", gateway_status.drive_events.number);
                        json_object_set_string(events_root, "text", gateway_status.drive_events.text);
                        json_object_set_string(events_root, "time", gateway_status.drive_events.time);
                        json_object_set_number(events_root, "cnt", gateway_status.drive_events.cnt);

                        json_array_append_value(events_arr,events);
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
            if(index == UPDATE_EVENT_LOG)
            {
                setStrTime(read_end_time);
                char* drive_events = json_serialize_to_string(events_vaule);          
                gateway_update_context.drive_events = calloc(strlen(drive_events)+1,1);    
                strcpy(gateway_update_context.drive_events,drive_events);            
                free(drive_events);                
            }
            err =  index;
            goto exit;
        }   
    }
exit:
    if(events_vaule!=NULL)
    {
        json_value_free(events_vaule);
    }
    return err;
}
void query_status_task(void *arg)
{
    index_t index = CON_EVENT_LOG;

    GateWay_Context_t gateway = *(GateWay_Context_t*)arg;

    while(1)
    {   
        switch(index)
        {
            case HAND_SHAKE:  
                device_log("#################     start handshake    #################");
                index = gateway.gw_422_handshake_fun(); 
                index = CON_EVENT_LOG;
                break;
            case CON_SCN:
                device_log("################# start collect scn info #################");
                index = gateway.gw_422_controller_scn_fun();
                break;
            case CON_EVENT_LOG:
                device_log("################# start collect controller event_log info #################");
                index = gateway.gw_422_controller_event_log_fun(); 
                break;
            case DRIVE_SCN:
                device_log("################# start collect drive scn info #################");
                index = gateway.gw_422_driver_scn_fun(); 
                break;
            case DRIVE_CURRENT_LOG:
                device_log("################# start collect drive event_log info #################");
                index = gateway.gw_422_driver_current_log_fun(); 
                break;
            case UPDATE_EVENT_LOG:
                controller_event_log_to_json();
                goto exit;
                index = CON_EVENT_LOG;
                break;
            default:
                break;
        }
    }
exit:
    pthread_exit(NULL);
}

