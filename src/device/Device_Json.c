#include "GateWay.h"

#define Json_log(M, ...) custom_log("Json", M, ##__VA_ARGS__)


extern update_status_t gateway_update_context;
extern char *read_start_time[30];
extern char *read_end_time[20];
extern char *controller_end_time[20];

OSStatus controller_event_log_to_json()
{

    JSON_Value * file_value = NULL;
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

    JSON_Value*  controller_scn_value = NULL;
    JSON_Array*  controller_scn_array;
    JSON_Value*  controller_events_value = NULL;
    JSON_Array*  controller_events_array;

    JSON_Value*  drive_scn_value = NULL;
    JSON_Array*  drive_scn_array;
    JSON_Value*  drive_events_value = NULL;
    JSON_Array*  drive_events_array;

    JSON_Value* controller_scn_vaule_index = NULL;
    JSON_Value* controller_event_vaule_index = NULL;

    JSON_Value* drive_scn_vaule_index = NULL;
    JSON_Value* drive_event_vaule_index = NULL;

    file_value= json_parse_file("../lucas_test1.txt");

    if(file_value == NULL)
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


        json_object_set_value(file_obj,"svt",file_svt_value);
        json_object_set_string(file_obj,"device_id","luacs_test");
        //json_object_set_string(file_obj,"reporting_time","2018/08/09 18:00:00");
        


        json_object_set_string(file_obj,"start_time",read_start_time);
        json_object_set_string(file_obj,"end_time",read_end_time);
        json_object_set_string(file_obj,"controller_start_time",read_start_time);
        json_object_set_string(file_obj,"controller_end_time",controller_end_time);
        json_object_set_string(file_obj,"drive_start_time",controller_end_time);
        json_object_set_string(file_obj,"drive_end_time",read_end_time);



        json_object_set_value(file_svt_obj,"controller",file_controller_value);

        json_object_set_value(file_controller_obj, "scn", file_controller_scn_value);
        json_object_set_number(file_controller_obj, "no_of_runs", gateway_update_context.controller_status.no_of_runs);
        json_object_set_number(file_controller_obj, "age", gateway_update_context.controller_status.age);
        json_object_set_value(file_controller_obj, "events", file_controller_events_value);    

        json_object_set_value(file_svt_obj,"drive",file_drive_value);
        json_object_set_value(file_drive_obj, "scn", file_drive_scn_value);
        json_object_set_string(file_drive_obj, "time", "123456");
        json_object_set_value(file_drive_obj, "events", file_drive_events_value);    
    }
    else
    {
        Json_log("###################################################        TURE");
        file_obj = json_object(file_value);
        file_svt_value = json_object_get_value(file_obj,"svt");
        file_svt_obj = json_object(file_svt_value);

        
        file_controller_value = json_object_get_value(file_svt_obj,"controller");
        file_controller_obj = json_object(file_controller_value);

        file_drive_value = json_object_get_value(file_svt_obj,"drive");
        file_drive_obj = json_object(file_drive_value);

        file_controller_scn_array = json_object_get_array(file_controller_obj,"scn");
        file_controller_events_array = json_object_get_array(file_controller_obj,"events");

        file_drive_scn_array = json_object_get_array(file_drive_obj,"scn");
        file_drive_events_array = json_object_get_array(file_drive_obj,"events");

        
    }
    // char local_time[20]={0};
    // setStrTime(local_time);
    // json_object_set_string(file_obj,"reporting_time",local_time);
    // json_object_set_string(file_obj,"starting_time",local_time);
    

    if(gateway_update_context.controller_scn!=NULL)
    {
        controller_scn_value = json_parse_string(gateway_update_context.controller_scn);
        controller_scn_array = json_value_get_array(controller_scn_value);
        
        int file_controller_scn_count = json_array_get_count(controller_scn_array);
        Json_log("################################        %d",file_controller_scn_count); 
        json_array_clear(file_controller_scn_array);
        for(int k=0;k<file_controller_scn_count;k++) 
        {
            controller_scn_vaule_index = json_value_deep_copy(json_array_get_value(controller_scn_array,k));
            json_array_append_value(file_controller_scn_array,controller_scn_vaule_index);
        } 
    }

    if(gateway_update_context.controller_events!=NULL)
    {
        controller_events_value = json_parse_string(gateway_update_context.controller_events);
        controller_events_array = json_value_get_array(controller_events_value);

        int file_controller_events_count = json_array_get_count(file_controller_events_array);
        Json_log("################################        %d",file_controller_events_count);

        int controller_events_count = json_array_get_count(controller_events_array);
        Json_log("################################         %d",controller_events_count);  

        int total = controller_events_count + file_controller_events_count;
        if(total <=100)
        {
            for(int i=0;i<controller_events_count;i++)
            {
                controller_event_vaule_index = json_value_deep_copy(json_array_get_value(controller_events_array,i));
                json_array_append_value(file_controller_events_array,controller_event_vaule_index);
            }          
        }
        else
        {
            for(int j=0;j<total - 100;j++)
            {
                json_array_remove(file_controller_events_array, j);
            }
            for(int i=0;i<controller_events_count;i++)
            {
                controller_event_vaule_index = json_value_deep_copy(json_array_get_value(controller_events_array,i));
                json_array_append_value(file_controller_events_array,controller_event_vaule_index);
            }          
        }
    }
    if(gateway_update_context.drive_scn!=NULL)
    {
        
        drive_scn_value = json_parse_string(gateway_update_context.drive_scn);
        drive_scn_array = json_value_get_array(drive_scn_value);
        
        int drive_scn_count = json_array_get_count(drive_scn_array);
        Json_log("################################        %d",drive_scn_count); 
        json_array_clear(file_drive_scn_array);
        for(int t=0;t<drive_scn_count;t++) 
        {
            drive_scn_vaule_index = json_value_deep_copy(json_array_get_value(drive_scn_array,t));
            json_array_append_value(file_drive_scn_array,drive_scn_vaule_index);
        } 
    }
    if(gateway_update_context.drive_events!=NULL)
    {
        drive_events_value = json_parse_string(gateway_update_context.drive_events);
        drive_events_array = json_value_get_array(drive_events_value);

        int file_drive_events_count = json_array_get_count(file_drive_events_array);
        Json_log("################################        %d",file_drive_events_count);

        int drive_events_count = json_array_get_count(drive_events_array);
        Json_log("################################         %d",drive_events_count);  

        int total = drive_events_count + file_drive_events_count;
        if(total <=100)
        {
            for(int i=0;i<drive_events_count;i++)
            {
                drive_event_vaule_index = json_value_deep_copy(json_array_get_value(drive_events_array,i));
                json_array_append_value(file_drive_events_array,drive_event_vaule_index);
            }          
        }
        else
        {
            for(int j=0;j<total - 100;j++)
            {
                json_array_remove(file_drive_events_array, j);
            }
            for(int i=0;i<drive_events_count;i++)
            {
                JSON_Value* vaule_index = json_value_deep_copy(json_array_get_value(drive_events_array,i));
                json_array_append_value(file_drive_events_array,vaule_index);
            }          
        }
    }
    json_serialize_to_file(file_value,"../lucas_test1.txt");
   
//     JSON_Value *  read_value = json_parse_file("../lucas_test1.txt");

//     char *test_log = json_serialize_to_string(read_value);
//     device_log("*****************   %s\r\n",test_log);
//     if(test_log!=NULL)
//         free(test_log);

// if(read_value!=NULL)
//     json_value_free(read_value);


if(file_value!=NULL)
    json_value_free(file_value);

if(controller_scn_value!=NULL)
    json_value_free(controller_scn_value);
if(controller_events_value!=NULL)
    json_value_free(controller_events_value);
if(drive_scn_value!=NULL)
    json_value_free(drive_scn_value);
if(drive_events_value!=NULL)
    json_value_free(drive_events_value);

        Json_log("################################ ################################       json exit"); 

}



OSStatus write_to_file(char *buffer_write)
{
    int fd;
    ssize_t length_w;
    char *testwrite = "../testwrite22222.txt";
    if((fd = open(testwrite, O_RDWR|O_CREAT,0777))<0)
        printf("open %s failed\n",testwrite); 
	length_w = write(fd,buffer_write,strlen(buffer_write));
	if(length_w == -1)
	{
		printf("write\r\n");
	}
	else{
		printf("Write Function OK!\n");
	}	
}
