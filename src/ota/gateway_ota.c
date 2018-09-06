#include "lucas.h"
#include "gateway_ota.h"


#define http_log(M, ...) custom_log("http", M, ##__VA_ARGS__)

static unsigned char* user_strupr(unsigned char* szMsg)
{
    unsigned char *pcMsg = NULL;

    for (pcMsg = szMsg; ('\0' != *pcMsg); pcMsg++)
    {
        if (('a' <= *pcMsg) && (*pcMsg <= 'z'))
        {
            *pcMsg += ('A' - 'a');
        }
    }

    return szMsg;
}

static bool user_str2hex(unsigned char *src, uint8_t *dest, uint32_t dest_size)
{
    unsigned char hb = 0;
    unsigned char lb = 0;
    uint32_t i = 0, j = 0;
    uint32_t src_size = strlen((const char *)src);

    if ( (src_size % 2 != 0) || (src_size <= 0))
        return false;

    src = user_strupr( src );

    for ( i = 0; i < src_size; i ++ )
    {
        if(i > dest_size * 2)
            return false;

        hb = src[i];
        if ( hb >= 'A' && hb <= 'F' )
            hb = hb - 'A' + 10;
        else if ( hb >= '0' && hb <= '9' )
            hb = hb - '0';
        else
            return false;

        i++;
        lb = src[i];
        if ( lb >= 'A' && lb <= 'F' )
            lb = lb - 'A' + 10;
        else if ( lb >= '0' && lb <= '9' )
            lb = lb - '0';
        else
            return false;

        dest[j++] = (hb << 4) | (lb);
    }

    return true;
}

size_t header_cb(void *ptr, size_t size, size_t nmemb, void *stream) 
{
	USER_OTA_CONTEXT_T *ota_context = (USER_OTA_CONTEXT_T *)stream;
	int r;
    long len = 0;
	// http_log("%s",ptr);
	r = sscanf(ptr, "Content-Length: %ld\n", &len);
	if (r) /* Microsoft: we don't read the specs */
		ota_context->ota_info.file_total_len = len;
	return size * nmemb;
}
int progress_cb(void *ptr, double t, double d, double ultotal, double ulnow)
{
 
	// http_log("%d / %d (%f %%)\n", (int)d, (int)t, d*100.0/t);
 
	// float tmp = d*100.0/t;

	return 0;
}
 

int http_short_connection_nossl(const char *http_request_url, void *fptr, HTTP_WRITE_DATA_CB write_cb)
{
	int err = 0;
    CURLcode res; //定义CURLcode类型的变量，保存返回状态码
	CURL *curl_handle = NULL;//定义CURL类型的指针
	CURLcode r = CURLE_GOT_NOTHING;	
	USER_OTA_CONTEXT_T *ota_context = (USER_OTA_CONTEXT_T *)fptr;
	curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();        //初始化一个CURL类型的指针
	if (!curl_handle)
	{
		http_log("curl init failed\n");
		err = -1;
        goto exit;
	}
	curl_off_t local_file_len = -1 ;
	struct stat file_info;
  	int use_resume = 0;


	if(ota_context->fptr!=NULL)
	{
		if(stat(FILENAME, &file_info) == 0) 
		{
			local_file_len =  file_info.st_size;
			http_log("------------------------------- %ld",local_file_len);
			use_resume  = 1;
		}
	}
	curl_easy_setopt(curl_handle, CURLOPT_URL, http_request_url);

	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, SOCK_TIMEOUT);
	curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, SOCK_TIMEOUT);

    curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, header_cb);
	curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, fptr);

	// 设置文件续传的位置给libcurl
	curl_easy_setopt(curl_handle, CURLOPT_RESUME_FROM_LARGE, use_resume?local_file_len:0);

    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_cb);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, fptr);

    curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 0L);   
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);

	curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, progress_cb);
	curl_easy_setopt(curl_handle, CURLOPT_PROGRESSDATA, NULL);
 
	curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);

    res = curl_easy_perform(curl_handle);    //调用curl_easy_perform 执行我们的设置.并进行相关的操作. 在这里只在屏幕上显示出来.
	if (res != CURLE_OK)
	{
		switch(res)
		{
		case CURLE_UNSUPPORTED_PROTOCOL:
			http_log("不支持的协议,由URL的头部指定\n");break;
		case CURLE_COULDNT_CONNECT:
			http_log("不能连接到remote主机或者代理\n");break;
		case CURLE_HTTP_RETURNED_ERROR:
			http_log("http返回错误\n");break;
		case CURLE_READ_ERROR:
			http_log("读本地文件错误\n");break;
		default:
			http_log("返回值:%d\n",res);break;
		}
		err = -1;
	}

exit:
	curl_easy_cleanup(curl_handle);
	curl_global_cleanup();
	return err;
}

int user_ota_check_cb(void *ptr, size_t size, size_t nmemb, void *stream)
{
	printf("%s",ptr);
	USER_OTA_CONTEXT_T *ota_context = (USER_OTA_CONTEXT_T *)stream;
	JSON_Value * ota_info_value;
	JSON_Object* ota_info_obj;
	ota_info_value = json_parse_string(ptr);
	ota_info_obj = json_object(ota_info_value);

	memset(&ota_context->ota_info, 0, sizeof(ota_context->ota_info));
	memcpy(ota_context->ota_info.ota_version, json_object_get_string(ota_info_obj,"version"), strlen(json_object_get_string(ota_info_obj,"version")));
	memcpy(ota_context->ota_info.file_url, json_object_get_string(ota_info_obj,"bin"), strlen(json_object_get_string(ota_info_obj,"bin")));
	memcpy(ota_context->ota_info.file_md5, json_object_get_string(ota_info_obj,"bin_md5"), strlen(json_object_get_string(ota_info_obj,"bin_md5")));

	json_value_free(ota_info_value);

	http_log("ota_version :%s",ota_context->ota_info.ota_version);
	http_log("file_url    :%s",ota_context->ota_info.file_url);
	http_log("file_md5    :%s",ota_context->ota_info.file_md5);

	return size * nmemb;	
}
int file_download_data_cb(void *ptr, size_t size, size_t nmemb, void *stream)
{
	USER_OTA_CONTEXT_T *user_ota_context = (USER_OTA_CONTEXT_T *)stream;
	
	fwrite(ptr,size,nmemb,user_ota_context->fptr);
	user_ota_context->ota_info.download_len += size * nmemb;

	user_ota_context->download_state_cb(HTTP_FILE_DOWNLOAD_STATE_LOADING,user_ota_context);
	return size * nmemb;	
}

int user_ota_check_with_device_id(const char *host_name, bool is_ssl, const char *device_id, USER_OTA_CONTEXT_T *ota_context)
{
	int err  = 0;
	if(host_name!=NULL)
	{
		sprintf(ota_context->ota_check_url,"%s%s",host_name,device_id);
	}

	http_log("ota_check_url ~~~~~~~   %s",ota_context->ota_check_url);

    if(is_ssl == true)
    {
        err = http_short_connection_nossl( ota_context->ota_check_url, ota_context, user_ota_check_cb);
    }
	else
    {
        err = http_short_connection_nossl( ota_context->ota_check_url, ota_context, user_ota_check_cb);
    }

	if(strcmp(ota_context->ota_info.ota_version, DEVICE_VERSION))
	{
		ota_context->is_update = true;
		http_log("ota_check_url ~~~~~~~   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~NEED OTA");

	}
	else{
		http_log("ota_check_url ~~~~~~~   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ NOT NEED OTA");
	}
	
	return err;
}
int file_md5_check(USER_OTA_CONTEXT_T *user_ota_context)
{
	int err = -1;
    uint8_t *md5_calc = NULL;
	uint8_t md5_recv[16] = {0};

    FILE* fptr;
    if ((fptr = fopen(FILENAME,"r")) == NULL)
    {
        http_log("fopen file error:%s\n",FILENAME);
        return -1;
    }

    md5_calc =md5File(fptr);

    http_log("FLASH READ: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
            md5_calc[0],md5_calc[1],md5_calc[2],md5_calc[3],
            md5_calc[4],md5_calc[5],md5_calc[6],md5_calc[7],
            md5_calc[8],md5_calc[9],md5_calc[10],md5_calc[11],
            md5_calc[12],md5_calc[13],md5_calc[14],md5_calc[15]);


    user_str2hex(user_ota_context->ota_info.file_md5, md5_recv, sizeof(md5_recv));

    if ( memcmp( md5_recv, md5_calc, sizeof(md5_recv)) == 0 )
    {
        http_log( "OTA SUCCESS!\r\n" );
    }
}

int user_ota_start(USER_OTA_CONTEXT_T *user_ota_context)
{
    int err = -1;

    require(user_ota_context != NULL , exit);

FILE_DOWNLOAD_START:

    if(user_ota_context->is_update == true) 
    {
        http_log("update gateway firmware : %s",user_ota_context->ota_info.file_url);

		if(user_ota_context->retry_count==0)
			user_ota_context->fptr = fopen(FILENAME,"w");
		else
			user_ota_context->fptr = fopen(FILENAME,"a");
		require_action(user_ota_context->fptr != NULL, exit, err = -1);

		user_ota_context->download_state_cb(HTTP_FILE_DOWNLOAD_STATE_START,NULL);

		if(user_ota_context->is_ssl == true)
		{
			err = http_short_connection_nossl( user_ota_context->ota_info.file_url, user_ota_context, file_download_data_cb);
		}
		else
		{
			err = http_short_connection_nossl( user_ota_context->ota_info.file_url, user_ota_context, file_download_data_cb);
		}
		require_noerr( err, exit );
		user_ota_context->is_success = true;
    }
	else
		return -1;

exit:
	if(user_ota_context->fptr!=NULL)
		fclose(user_ota_context->fptr);
	if(user_ota_context->is_success == false)
	{
		http_log("--------------------------------------------------- 22222");
		user_ota_context->retry_count ++;
		if(user_ota_context->retry_count<3)
		{
			user_ota_context->download_state_cb(HTTP_FILE_DOWNLOAD_STATE_FAILED_AND_RETRY,NULL);
			goto FILE_DOWNLOAD_START;
		}
		else
			user_ota_context->download_state_cb(HTTP_FILE_DOWNLOAD_STATE_FAILED,NULL);
	}
	else{
		file_md5_check(user_ota_context);
		user_ota_context->download_state_cb(HTTP_FILE_DOWNLOAD_STATE_SUCCESS,NULL);
	}
    return err;
}
void gateway_ota_shell(void)
{
    system("pwd");
    system("sh OtisMDOtaControl.sh");
}
//file download state callback
int file_download_state_cb(int state,void *usr_args)
{
    int err = -1;
	USER_OTA_CONTEXT_T *user_ota_context = (USER_OTA_CONTEXT_T *)usr_args;
	if(state == HTTP_FILE_DOWNLOAD_STATE_LOADING)
	{
		http_log("=========file download state:    %s       %d / %d (%f %%)\n", CMD_STRING[state],
		user_ota_context->ota_info.download_len, user_ota_context->ota_info.file_total_len, 
		user_ota_context->ota_info.download_len*100.0/user_ota_context->ota_info.file_total_len);
	}
	else
		http_log("=========file download state:    %s       \n", CMD_STRING[state]);
	if(state == HTTP_FILE_DOWNLOAD_STATE_SUCCESS)
		gateway_ota_shell();

exit:
	return err;
}

void *gateway_ota_task(void *arg)
{
	int err = 0;
	USER_OTA_CONTEXT_T user_ota_context;
	user_ota_context.is_success = false;
	user_ota_context.is_update = false;
	user_ota_context.is_ssl    = false;
	user_ota_context.download_state_cb = file_download_state_cb;

	user_ota_context.fptr = NULL;
    err = user_ota_check_with_device_id(OTA_DOMAIN, user_ota_context.is_ssl, DEVICE_ID, &user_ota_context);
	require_noerr(err, exit);

	err = user_ota_start( &user_ota_context);
	require_noerr(err, exit);
exit:
	pthread_exit(NULL);
}