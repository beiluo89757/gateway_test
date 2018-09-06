#include "GateWay.h"

#define Json_log(M, ...) custom_log("Json", M, ##__VA_ARGS__)

extern update_status_t gateway_update_context;
extern char *read_start_time[30];
extern char *read_end_time[20];
extern char *controller_end_time[20];

OSStatus write_to_file(char *buffer_write)
{
    int fd;
    ssize_t length_w;
    char *testwrite = "../testwrite22222.txt";
    if ((fd = open(testwrite, O_RDWR | O_CREAT, 0777)) < 0)
        printf("open %s failed\n", testwrite);
    length_w = write(fd, buffer_write, strlen(buffer_write));
    if (length_w == -1)
    {
        printf("write\r\n");
    }
    else
    {
        printf("Write Function OK!\n");
    }
}
extern char *drive_event_string;
int strcmp_lucas (const char *p1, const char *p2)
{
  const unsigned char *s1 = (const unsigned char *) p1;
  const unsigned char *s2 = (const unsigned char *) p2;
  unsigned char c1, c2;
  do
    {
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;
      if (c1 == '\0')
        return c1 - c2;
    }
  while (c1 == c2);
  return c1 - c2;
}
OSStatus controller_event_log_to_json()
{

    JSON_Value *file_value = NULL;
    JSON_Object *file_obj;

    JSON_Value *file_svt_value = NULL;
    JSON_Object *file_svt_obj;

    JSON_Value *file_controller_value = NULL;
    JSON_Object *file_controller_obj;

    JSON_Value *file_drive_value = NULL;
    JSON_Object *file_drive_obj;

    JSON_Value *file_controller_scn_value = NULL;
    JSON_Array *file_controller_scn_array;

    JSON_Value *file_controller_events_value = NULL;
    JSON_Array *file_controller_events_array;

    JSON_Value *file_drive_scn_value = NULL;
    JSON_Array *file_drive_scn_array;

    JSON_Value *file_drive_events_value = NULL;
    JSON_Array *file_drive_events_array;

    JSON_Value *file_controller_event_vaule_index = NULL;
    JSON_Value *file_drive_event_vaule_index = NULL;

    bool status_need_update = true;
    bool drive_need_update = true;

    bool write_file = false;

    char local_time[20]={0};
    update_status_t file_update_tmp;



    JSON_Value *append_value=NULL;


    char *file_controll_events_string =NULL;
    char *file_controll_scn_string = NULL;
    char *file_drive_events_string =NULL;
    char *file_drive_scn_string = NULL;

    memset(&file_update_tmp,0,sizeof(file_update_tmp));

    file_value = json_parse_file(EVENTLOG_FILE_NAME);
    char *time2 =NULL;
    if (file_value == NULL)
    {
        Json_log("###################################################        NULL");
        file_value = json_value_init_object();
        file_obj = json_object(file_value);

        file_svt_value = json_value_init_object();
        file_svt_obj = json_object(file_svt_value);

        file_controller_value = json_value_init_object();
        file_controller_obj = json_object(file_controller_value);

        file_drive_value = json_value_init_object();
        file_drive_obj = json_object(file_drive_value);

        file_controller_scn_value = json_value_init_array();
        file_controller_scn_array = json_value_get_array(file_controller_scn_value);
        file_controller_events_value = json_value_init_array();
        file_controller_events_array = json_value_get_array(file_controller_events_value);

        file_drive_scn_value = json_value_init_array();
        file_drive_scn_array = json_value_get_array(file_drive_scn_value);
        file_drive_events_value = json_value_init_array();
        file_drive_events_array = json_value_get_array(file_drive_events_value);

        json_object_set_value(file_obj, "svt", file_svt_value);
        json_object_set_string(file_obj, "device_id", "luacs_test");
        char local_time[20]={0};
        setStrTime(local_time);
        json_object_set_string(file_obj,"reporting_time",local_time);

        json_object_set_value(file_svt_obj, "controller", file_controller_value);

        json_object_set_value(file_controller_obj, "scn", file_controller_scn_value);
        json_object_set_number(file_controller_obj, "no_of_runs", gateway_update_context.controller_status.no_of_runs);
        json_object_set_number(file_controller_obj, "age", gateway_update_context.controller_status.age);
        json_object_set_value(file_controller_obj, "events", file_controller_events_value);

        json_object_set_value(file_svt_obj, "drive", file_drive_value);

        json_object_set_value(file_drive_obj, "scn", file_drive_scn_value);
        json_object_set_string(file_drive_obj, "time", "123");
        json_object_set_value(file_drive_obj, "events", file_drive_events_value);
        write_file = true;
    }
    else
    {
        Json_log("###################################################        TURE");
        file_obj = json_object(file_value);
        file_svt_value = json_object_get_value(file_obj, "svt");
        file_svt_obj = json_object(file_svt_value);

        setStrTime(local_time);
        json_object_set_string(file_obj,"reporting_time",local_time);

        file_controller_value = json_object_get_value(file_svt_obj, "controller");
        file_controller_obj = json_object(file_controller_value);

        int no_of_runs = json_object_get_number(file_controller_obj, "no_of_runs");
        if(no_of_runs !=gateway_update_context.controller_status.no_of_runs)
        {
            write_file = true;
            Json_log("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$       ========================================= ");
            json_object_set_number(file_controller_obj, "no_of_runs", gateway_update_context.controller_status.no_of_runs);
        }

        int age = json_object_get_number(file_controller_obj, "age");
        if(age !=gateway_update_context.controller_status.age)
        {
            write_file = true;
            Json_log("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$    %d   ========================================= ",age);
            json_object_set_number(file_controller_obj, "age", gateway_update_context.controller_status.age);
        }

        file_drive_value = json_object_get_value(file_svt_obj, "drive");
        file_drive_obj = json_object(file_drive_value);



        time2 = json_object_get_string(file_drive_obj, "time");
        if(0!=strcmp(time,gateway_update_context.drive_status.time))
        {
            write_file = true;
            json_object_set_string(file_drive_obj, "time", gateway_update_context.drive_status.time);
        }

        file_controller_scn_array = json_object_get_array(file_controller_obj, "scn");
        file_controller_events_array = json_object_get_array(file_controller_obj, "events");

        file_drive_scn_array = json_object_get_array(file_drive_obj, "scn");
        file_drive_events_array = json_object_get_array(file_drive_obj, "events");
    }

    Json_log("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$        %d", gateway_update_context.drive_scn_num);
    Json_log("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$        %d", gateway_update_context.drive_events_num);
    Json_log("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$        %d", gateway_update_context.controller_scn_num);
    Json_log("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$        %d", gateway_update_context.controller_events_num);
    // Json_log("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$    %s  ========================================= ",gateway_update_context.drive_events[0].event_log );
    // Json_log("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$    %s  ========================================= ",gateway_update_context.drive_events[1].event_log );
    
    /*************************************  drive_scn   *************************************/
    /*************************************  drive_scn   *************************************/
    if (gateway_update_context.drive_scn_num > 0)
    {
        int file_drive_scn_count = json_array_get_count(file_drive_scn_array);
        Json_log("################################        %d", file_drive_scn_count);
        Json_log("################################        %d", gateway_update_context.drive_scn_num);
        for(int i=0; i<gateway_update_context.drive_scn_num; i++)
        {
            status_need_update = true; 
            for(int j=0; j<file_drive_scn_count; j++)
            {
                file_drive_scn_string = json_serialize_to_string(json_array_get_value(file_drive_scn_array, j));
                if(file_drive_scn_string!=NULL)
                {
                    // Json_log("################################        %s    [%d]"   , file_controll_scn_string,i);
                    // Json_log("################################22        %s    [%d]"   , gateway_update_context.drive_scn[i].scn,i);
                    if(0==strcmp(file_drive_scn_string,gateway_update_context.drive_scn[i].scn))
                    {   
                        Json_log("--------------------------------------------------------------con scn same");

                        status_need_update = false;   
                        if(file_drive_scn_string!=NULL)
                        {
                            free(file_drive_scn_string);
                            file_drive_scn_string=NULL;
                        }
                        break; 
                    }
                    if(file_drive_scn_string!=NULL)
                    {
                        free(file_drive_scn_string);
                        file_drive_scn_string=NULL;
                    }
                }
            }
            if(status_need_update == true)
            {
                write_file = true;
                Json_log("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$       ========================================= ");
                strcpy(file_update_tmp.drive_scn[file_update_tmp.drive_scn_num++].scn,gateway_update_context.drive_scn[i].scn);
                Json_log("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$    %s  ========================================= ",gateway_update_context.drive_scn[i].scn );

                Json_log("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$    %s  ========================================= ",file_update_tmp.drive_scn[file_update_tmp.drive_scn_num].scn );

            }            
        }
        if(file_update_tmp.drive_scn_num>0)
        {
            int total = file_update_tmp.drive_scn_num + file_drive_scn_count;
            if (total <= 20)
            {
                for (int i = 0; i < file_update_tmp.drive_scn_num; i++)
                {
                    append_value = json_parse_string(file_update_tmp.drive_scn[i].scn);
                    json_array_append_value(file_drive_scn_array,append_value);
                }
            }
            else
            {
                for (int j = 0; j < total - 20; j++)
                {
                    json_array_remove(file_drive_scn_array, j);
                }
                for (int i = 0; i < file_update_tmp.drive_scn_num; i++)
                {
                    append_value = json_parse_string(file_update_tmp.drive_scn[i].scn);
                    json_array_append_value(file_drive_scn_array,append_value);
                }
            }
        } 
        else
        {
            // Json_log("*****************   NO  NEED UPDATE    *****************\r\n");
        }            
    }  
    memset(gateway_update_context.drive_events[0].event_log,0,200);
// sprintf(gateway_update_context.drive_events[0].event_log, drive_event_string,12,"POWER ON  ",21,18,0);
sprintf(gateway_update_context.drive_events[0].event_log, drive_event_string,88,"haha","heihei",66);

/*************************************  drive_events   *************************************/
/*************************************  drive_events   *************************************/
//strcpy(gateway_update_context.drive_events[0].event_log, "{\"number\":12,\"text\":\"POWER ON  \",\"cnt\":21,\"age\":18,\"position\":0}");
gateway_update_context.drive_events_num = 1;
    if (gateway_update_context.drive_events_num != 0)
    {
        int file_drive_events_count = json_array_get_count(file_drive_events_array);
        // Json_log("################################        %d", file_drive_events_count);
        // Json_log("################################        %d", gateway_update_context.drive_events_num);

        for(int i=0; i<gateway_update_context.drive_events_num; i++)
        {
            status_need_update = true; 
             
            // Json_log("#################   %s   #################   [%d]\r\n", gateway_update_context.drive_events[i].event_log,i);
            for(int j=0; j<file_drive_events_count; j++)
            {
                file_drive_event_vaule_index = json_array_get_value(file_drive_events_array, j);
                if(file_drive_event_vaule_index!=NULL)
                {
                    file_drive_events_string = json_serialize_to_string(file_drive_event_vaule_index);
                    if(file_drive_events_string!=NULL)
                    {
                        // Json_log("#################   %s   #################   [%d]\r\n", file_controll_drive_string,j);

                        if(0==strcmp(file_drive_events_string,gateway_update_context.drive_events[i].event_log))
                        {   
                            status_need_update = false;   
                            if(file_drive_events_string!=NULL)
                            {
                                free(file_drive_events_string);
                                file_drive_events_string=NULL;
                            }
                            break; 
                        }
                        if(file_drive_events_string!=NULL)
                        {
                            free(file_drive_events_string);
                            file_drive_events_string=NULL;
                        }
                    }
                }
            }
            if(status_need_update == true)
            {
                write_file = true;
                Json_log("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$       ========================================= ");
                strcpy(file_update_tmp.drive_events[file_update_tmp.drive_events_num++].event_log,gateway_update_context.drive_events[i].event_log);
                Json_log("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$    %s  ========================================= ",gateway_update_context.drive_events[i].event_log );

                Json_log("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$    %s  ========================================= ",file_update_tmp.drive_events[file_update_tmp.drive_events_num-1].event_log );

            }            
        }
        // Json_log("=========================   @@@@@   [%d] *****************\r\n",file_update_tmp.drive_events_num);
        if(file_update_tmp.drive_events_num>0)
        {
            Json_log("*****************   NEED UPDATE  11111   *****************\r\n");
            int total = file_update_tmp.drive_events_num + file_drive_events_count;
            if (total <= 100)
            {
                for (int i = 0; i < file_update_tmp.drive_events_num; i++)
                {
                    Json_log("*****************   NEED UPDATE  33333   *****************\r\n");
                    append_value = json_parse_string(file_update_tmp.drive_events[i].event_log);
                    json_array_append_value(file_drive_events_array,append_value);
                }
            }
            else
            {
                for (int j = 0; j < total - 100; j++)
                {
                    json_array_remove(file_drive_events_array, j);
                }
                for (int i = 0; i < file_update_tmp.drive_events_num; i++)
                {
                    append_value = json_parse_string(file_update_tmp.drive_events[i].event_log);
                    json_array_append_value(file_drive_events_array,append_value);
                }
            }
        } 
        else
        {
            // Json_log("*****************   NO  NEED UPDATE    *****************\r\n");
        }            
    }  


/*************************************  controller_scn   *************************************/
/*************************************  controller_scn   *************************************/
    if (gateway_update_context.controller_scn_num > 0)
    {
        int file_controller_scn_count = json_array_get_count(file_controller_scn_array);
        Json_log("################################        %d", file_controller_scn_count);
        Json_log("################################        %d", gateway_update_context.controller_scn_num);
        for(int i=0; i<gateway_update_context.controller_scn_num; i++)
        {
            status_need_update = true; 
            for(int j=0; j<file_controller_scn_count; j++)
            {
                file_controll_scn_string = json_serialize_to_string(json_array_get_value(file_controller_scn_array, j));
                if(file_controll_scn_string!=NULL)
                {
                    // Json_log("################################        %s    [%d]"   , file_controll_scn_string,i);
                    // Json_log("################################22        %s    [%d]"   , gateway_update_context.controller_scn[i].scn,i);
                    if(0==strcmp(file_controll_scn_string,gateway_update_context.controller_scn[i].scn))
                    {   
                        Json_log("--------------------------------------------------------------con scn same");

                        status_need_update = false;   
                        if(file_controll_scn_string!=NULL)
                        {
                            free(file_controll_scn_string);
                            file_controll_scn_string=NULL;
                        }
                        break; 
                    }
                    if(file_controll_scn_string!=NULL)
                    {
                        free(file_controll_scn_string);
                        file_controll_scn_string=NULL;
                    }
                }
            }
            if(status_need_update == true)
            {
                write_file = true;
                Json_log("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$       ========================================= ");
                strcpy(file_update_tmp.controller_scn[file_update_tmp.controller_scn_num++].scn,gateway_update_context.controller_scn[i].scn);
            }            
        }
        if(file_update_tmp.controller_scn_num>0)
        {
            int total = file_update_tmp.controller_scn_num + file_controller_scn_count;
            if (total <= 20)
            {
                for (int i = 0; i < file_update_tmp.controller_scn_num; i++)
                {
                    append_value = json_parse_string(file_update_tmp.controller_scn[i].scn);
                    json_array_append_value(file_controller_scn_array,append_value);
                }
            }
            else
            {
                for (int j = 0; j < total - 20; j++)
                {
                    json_array_remove(file_controller_scn_array, j);
                }
                for (int i = 0; i < file_update_tmp.controller_scn_num; i++)
                {
                    append_value = json_parse_string(file_update_tmp.controller_scn[i].scn);
                    json_array_append_value(file_controller_scn_array,append_value);
                }
            }
        } 
        else
        {
            // Json_log("*****************   NO  NEED UPDATE    *****************\r\n");
        }            
    }  
/*************************************  controller_events   *************************************/
/*************************************  controller_events   *************************************/
    if (gateway_update_context.controller_events_num != 0)
    {
        int file_controller_events_count = json_array_get_count(file_controller_events_array);
        // Json_log("################################        %d", file_controller_events_count);
        // Json_log("################################        %d", gateway_update_context.controller_events_num);

        for(int i=0; i<gateway_update_context.controller_events_num; i++)
        //for(int i=0; i<1; i++)
        {
            status_need_update = true; 
             
            // Json_log("#################   %s   #################   [%d]\r\n", gateway_update_context.controller_events[i].event_log,i);
            for(int j=0; j<file_controller_events_count; j++)
            {
                file_controller_event_vaule_index = json_array_get_value(file_controller_events_array, j);
                if(file_controller_event_vaule_index!=NULL)
                {
                    file_controll_events_string = json_serialize_to_string(file_controller_event_vaule_index);
                    if(file_controll_events_string!=NULL)
                    {
                        // Json_log("################################        %s    [%d]"   , file_controll_events_string,i);
                        // Json_log("################################22        %s    [%d]"   , gateway_update_context.controller_events[i].event_log,i);

                        if(0==strcmp_lucas(file_controll_events_string,gateway_update_context.controller_events[i].event_log))
                        {   
                            Json_log("--------------------------------------------------------------con scn same");
                            status_need_update = false;   
                            if(file_controll_events_string!=NULL)
                            {
                                free(file_controll_events_string);
                                file_controll_events_string=NULL;
                            }
                            break; 
                        }
                        if(file_controll_events_string!=NULL)
                        {
                            free(file_controll_events_string);
                            file_controll_events_string=NULL;
                        }
                    }
                }
            }
            if(status_need_update == true)
            {
                write_file = true;
                strcpy(file_update_tmp.controller_events[file_update_tmp.controller_events_num++].event_log,gateway_update_context.controller_events[i].event_log);
            }            
        }
        // Json_log("=========================   @@@@@   [%d] *****************\r\n",file_update_tmp.controller_events_num);
        if(file_update_tmp.controller_events_num>0)
        {
            Json_log("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$       ========================================= ");

            // Json_log("*****************   NEED UPDATE     *****************\r\n");
            int total = file_update_tmp.controller_events_num + file_controller_events_count;
            if (total <= 100)
            {
                for (int i = 0; i < file_update_tmp.controller_events_num; i++)
                {
                    append_value = json_parse_string(file_update_tmp.controller_events[i].event_log);
                    json_array_append_value(file_controller_events_array,append_value);
                }
            }
            else
            {
                for (int j = 0; j < total - 100; j++)
                {
                    json_array_remove(file_controller_events_array, j);
                }
                for (int i = 0; i < file_update_tmp.controller_events_num; i++)
                {
                    append_value = json_parse_string(file_update_tmp.controller_events[i].event_log);
                    json_array_append_value(file_controller_events_array,append_value);
                }
            }
        } 
        else
        {
            // Json_log("*****************   NO  NEED UPDATE    *****************\r\n");
        }            
    }  
    if(write_file ==true)
    {
        Json_log("*****************   write file   *****************\r\n");
        json_serialize_to_file_pretty(file_value, EVENTLOG_FILE_NAME);
    }
exit:

    if (file_value != NULL)
    {
        Json_log("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 1");
        json_value_free(file_value);
    }
    Json_log("################################ ################################       json exit");
}




//     JSON_Value *read_value = json_parse_file(EVENTLOG_FILE_NAME);

//     char *test_log = json_serialize_to_string(read_value);
//     Json_log("*****************   %s\r\n", test_log);
//     if (test_log != NULL)
//         free(test_log);

//     if (read_value != NULL)
//         json_value_free(read_value);


static const char* connectionString = "HostName=LoCostCnTest.azure-devices.net;DeviceId=lucas_test;SharedAccessKey=V9OtnPy4Y1kNGgbgdgWQGHp8MYngxijv6nJijce2hUg=";

char *upload_azure="\
------------------CHINA LOW COST IOT TEST------------------\r\n\r\n\
--------------------%s--------------------\r\n\r\n\
----------------Macrosoft Azrue IOT HUB UTC----------------\r\n\r\n\
File   creation time   : [%d]S\r\n\r\n\
Robustel Device String : %s\r\n\r\n\
Controll SCN    Num    : [%d]\r\n\r\n\
Controll events Num    : [%d]\r\n\r\n\
Device   SCN    Num    : [%d]\r\n\r\n\
Device   events Num    : [%d]\r\n\r\n\
----------------------------END----------------------------\r\n\r\n";



void update_to_cloud(void)
{

    char local_time[20]={0};
    setStrTime(local_time);

    JSON_Value *file_value = NULL;
    JSON_Object *file_obj;

    JSON_Value *file_svt_value = NULL;
    JSON_Object *file_svt_obj;

    JSON_Value *file_controller_value = NULL;
    JSON_Object *file_controller_obj;

    JSON_Value *file_drive_value = NULL;
    JSON_Object *file_drive_obj;

    JSON_Value *file_controller_scn_value = NULL;
    JSON_Array *file_controller_scn_array;

    JSON_Value *file_controller_events_value = NULL;
    JSON_Array *file_controller_events_array;

    JSON_Value *file_drive_scn_value = NULL;
    JSON_Array *file_drive_scn_array;

    JSON_Value *file_drive_events_value = NULL;
    JSON_Array *file_drive_events_array;

    JSON_Value *append_value=NULL;

    file_value = json_value_init_object();
    file_obj = json_object(file_value);

    file_svt_value = json_value_init_object();
    file_svt_obj = json_object(file_svt_value);

    file_controller_value = json_value_init_object();
    file_controller_obj = json_object(file_controller_value);

    file_drive_value = json_value_init_object();
    file_drive_obj = json_object(file_drive_value);

    file_controller_scn_value = json_value_init_array();
    file_controller_scn_array = json_value_get_array(file_controller_scn_value);
    file_controller_events_value = json_value_init_array();
    file_controller_events_array = json_value_get_array(file_controller_events_value);

    file_drive_scn_value = json_value_init_array();
    file_drive_scn_array = json_value_get_array(file_drive_scn_value);
    file_drive_events_value = json_value_init_array();
    file_drive_events_array = json_value_get_array(file_drive_events_value);



    json_object_set_value(file_obj, "svt", file_svt_value);
    json_object_set_string(file_obj, "device_id", "luacs_test");

    json_object_set_string(file_obj,"reporting_time",local_time);

    json_object_set_value(file_svt_obj, "controller", file_controller_value);

    json_object_set_value(file_controller_obj, "scn", file_controller_scn_value);
    json_object_set_number(file_controller_obj, "no_of_runs", gateway_update_context.controller_status.no_of_runs);
    json_object_set_number(file_controller_obj, "age", gateway_update_context.controller_status.age);
    json_object_set_value(file_controller_obj, "events", file_controller_events_value);

    json_object_set_value(file_svt_obj, "drive", file_drive_value);

    json_object_set_value(file_drive_obj, "scn", file_drive_scn_value);
    json_object_set_string(file_drive_obj, "time", "123");
    json_object_set_value(file_drive_obj, "events", file_drive_events_value);

    for (int i = 0; i < gateway_update_context.drive_scn_num; i++)
    {
        append_value = json_parse_string(gateway_update_context.drive_scn[i].scn);
        json_array_append_value(file_drive_scn_array,append_value);
    }
    for (int i = 0; i < gateway_update_context.drive_events_num; i++)
    {
        append_value = json_parse_string(gateway_update_context.drive_events[i].event_log);
        json_array_append_value(file_drive_events_array,append_value);
    }
    for (int i = 0; i < gateway_update_context.controller_scn_num; i++)
    {
        append_value = json_parse_string(gateway_update_context.controller_scn[i].scn);
        json_array_append_value(file_drive_scn_array,append_value);
    }
    for (int i = 0; i < gateway_update_context.controller_events_num; i++)
    {
        append_value = json_parse_string(gateway_update_context.controller_events[i].event_log);
        json_array_append_value(file_drive_events_array,append_value);
    }

    // struct msg_st data;
    
    char *upload_string = json_serialize_to_string(file_value);
    Json_log("*****************   %s\r\n", upload_string);

    // push_to_queue(GateWay_Context.msgid, upload_string, 0);
    Json_log("------------------0000");

    char haha[2000]={0};
    sprintf(haha,upload_azure,local_time,520,connectionString,1,2,1,2);
    Json_log("------------------   \r\n%s",haha);

exit:
    if (upload_string != NULL)
        free(upload_string);

    if (file_value != NULL)
        json_value_free(file_value);
}


// char *upload_azure="\
// ------------------CHINA LOW COST IOT TEST------------------\r\n\r\n\
// --------------------%s--------------------\r\n\r\n\
// ----------------Macrosoft Azrue IOT HUB UTC----------------\r\n\r\n\
// File   creation time   : %d\r\n\r\n\
// Robustel Device String : %s\r\n\r\n\
// Controll SCN    Num    : %d\r\n\r\n\
// Controll events Num    : %d\r\n\r\n\
// Device   SCN    Num    : %d\r\n\r\n\
// Device   events Num    : %d\r\n\r\n\
// ----------------------------END----------------------------\r\n\r\n";