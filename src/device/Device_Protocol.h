#pragma once

#include "GateWay.h"

#define UART_ONE_PACKAGE_LENGTH 1024
#define FRAME_LENTH 37

#define CONTROLLER_SCN_INFO_MAX_10      10
#define DRIVE_SCN_INFO_MAX              10
#define DRIVE_SCN_CLOCK_MAX             5


typedef enum{

    HAND_SHAKE,

    CON_SCN,
    CON_EVENT_LOG,
    
    CON_SCN_INDEX,
    CON_SCN_GECB_MENU,
    CON_SCN_SYSTEM_MENU,
    CON_SCN_TEST_MENU,
    CON_SCN_TEST_INFO,
    CON_SCN_TEST_INFO_GOON,
    CON_SCN_EVENT_LOG,

    DRIVE_SCN,
    DRIVE_CURRENT_LOG,

    DRIVE_SCN_INDEX,
    DRIVE_SCN_SYSTEM,
    DRIVE_SCN_MONITOR,
    DRIVE_SCN_STATUS,
    DRIVE_SCN_INFO,
    DRIVE_SCN_INFO_GOON,
    DRIVE_SCN_INFO_CLOCK,
    DRIVE_VIEW_CURRENT,
    DRIVE_CURRENT_INFO,
    DRIVE_CURRENT_INFO_GOON,

    UPDATE_EVENT_LOG

}index_t;
         
typedef struct _controller_events{
    u_int32_t   number;
    char        text[12];
    u_int32_t   cnt;
    u_int32_t   age;
    u_int32_t   position;
}controller_events_t;

typedef struct _controller_status{
    char        scn[12];
    u_int32_t   no_of_runs;
    u_int32_t   age;
}controller_status_t;

typedef struct _drive_events{
    u_int32_t   number;
    char        text[12];
    char        time[17];
    u_int32_t   cnt;
}drive_events_t;

typedef struct _drive_stauts{
    char        scn[12];
    char        time[17];
    u_int32_t   age;
}drive_status_t;


typedef struct _gateway_status
{   
    controller_status_t  controller_status;
    controller_events_t  controller_events;

    drive_status_t       drive_status;  
    drive_events_t       drive_events; 

}gateway_status_t;

// typedef struct _update_status
// {
//     controller_status_t     controller_status;
//     drive_status_t          drive_status;

//     char                    *controller_scn;
//     char                    *controller_events;

//     char                    *drive_scn; 
//     char                    *drive_events; 

// }update_status_t;



typedef struct _controller_scn
{
    char scn[38];
}controller_scn_t;

typedef struct _controller_event
{
    char event_log[200];
}controller_event_t;

typedef struct _drive_scn
{
    char scn[38];
}drive_scn_t;

typedef struct _drive_event
{
    char event_log[200];
}drive_event_t;

typedef struct _update_status
{
    controller_status_t     controller_status;
    drive_status_t          drive_status;

    char                    controller_scn_num;
    char                    controller_events_num;

    char                    drive_scn_num;
    char                    drive_events_num;

    controller_scn_t        controller_scn[20];
    controller_event_t      controller_events[100];

    drive_scn_t             drive_scn[20];
    drive_event_t           drive_events[100];

}update_status_t;









