#pragma once
#include "common.h"

#define DEVICE_ID           "123456"
#define DEVICE_VERSION      "001"

#define FIRST_URL  "api.easylink.io/v1/rom/lastversion.json?product_id=123456"
#define FILENAME   "gateway.bakBin"

#define OTA_DOMAIN                                  ("api.easylink.io/v1/rom/lastversion.json?product_id=")

#define OTA_CHECK_URL_MAX_LEN                       (256)
#define OTA_FILE_URL_MAX_LEN                        (256)
#define OTA_COMPONENT_MD5_MAX_LEN                   (64)
#define OTA_COMPONENT_VERSION_MAX_LEN               (32)



/*将相应的枚举类型转为字符串(这种方法很重要)*/
#define FOREACH_CMD(CMD) \
        CMD(HTTP_FILE_DOWNLOAD_STATE_START)   \
        CMD(HTTP_FILE_DOWNLOAD_STATE_SUCCESS)  \
        CMD(HTTP_FILE_DOWNLOAD_STATE_LOADING)   \
        CMD(HTTP_FILE_DOWNLOAD_STATE_FAILED)  \
        CMD(HTTP_FILE_DOWNLOAD_STATE_FAILED_AND_RETRY)   \
        CMD(HTTP_FILE_DOWNLOAD_STATE_REDIRET)  \
        CMD(HTTP_FILE_DOWNLOAD_STATE_MAX)   \
        CMD(HTTP_FILE_DOWNLOAD_STATE_NONE)  \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,
 
enum CMD_ENUM {
    FOREACH_CMD(GENERATE_ENUM)
};
 
static const char *CMD_STRING[] = {
    FOREACH_CMD(GENERATE_STRING)
};


// typedef enum {
//     HTTP_FILE_DOWNLOAD_STATE_START,
//     HTTP_FILE_DOWNLOAD_STATE_SUCCESS,
//     HTTP_FILE_DOWNLOAD_STATE_LOADING,
//     HTTP_FILE_DOWNLOAD_STATE_FAILED,
//     HTTP_FILE_DOWNLOAD_STATE_FAILED_AND_RETRY,
//     HTTP_FILE_DOWNLOAD_STATE_REDIRET,
//     HTTP_FILE_DOWNLOAD_STATE_MAX,
//     HTTP_FILE_DOWNLOAD_STATE_NONE
// }HTTP_FILE_DOWNLOAD_STATE_E; //download state

typedef bool (*HTTP_WRITE_DATA_CB)(void *ptr, size_t size, size_t nmemb, void *stream);
typedef void (*FILE_DOWNLOAD_STATE_CB) (int state, void *usr_args);

typedef struct _OTA_COMPONENT_INFO{
    u_int32_t   file_total_len;
    u_int32_t   download_len;
    char        ota_version[OTA_COMPONENT_VERSION_MAX_LEN];
    char        file_md5[OTA_COMPONENT_MD5_MAX_LEN];
    char        file_url[OTA_FILE_URL_MAX_LEN];
}OTA_COMPONENT_INFO_T;

typedef struct _user_ota_context{
    bool                    is_success;
    bool                    is_ssl;
    bool                    is_update;
    char                    retry_count;
    char                    ota_check_url[OTA_CHECK_URL_MAX_LEN];
    FILE                    *fptr;
    OTA_COMPONENT_INFO_T    ota_info;
    FILE_DOWNLOAD_STATE_CB  download_state_cb
}USER_OTA_CONTEXT_T;








#define SOCK_TIMEOUT 5

#define OTA_STATE_NOTREADY           0
#define OTA_STATE_READY              1
#define OTA_STATE_CONNECTING         2
#define OTA_STATE_GETHEAD            3
#define OTA_STATE_DOWNLOADING        4
#define OTA_STATE_DOWNLOADFINISH     5
#define OTA_STATE_ERROR              6