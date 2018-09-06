#include "lucas.h"
#include "Device_Protocol.h"

#define uart_recv_log(M, ...) custom_log("UART RECV", M, ##__VA_ARGS__)

size_t uart_get_one_packet(uart_index_t uart, char* inBuf, int inBufLen);

void *uartRecv_task(void *arg)
{
    int recvlen;

    char *inDataBuffer;

    uart_index_t uart = *(uart_index_t *)arg;

    inDataBuffer = malloc(UART_ONE_PACKAGE_LENGTH);

    while(1) 
    {
		memset(inDataBuffer,0,UART_ONE_PACKAGE_LENGTH);
        recvlen = uart_get_one_packet(uart,inDataBuffer, UART_ONE_PACKAGE_LENGTH);
        if (recvlen <= 0)
            continue; 
		uart_data_process(inDataBuffer,recvlen);
    }
exit:
    pthread_exit(NULL);
}


size_t uart_get_one_packet(uart_index_t uart, char* inBuf, int inBufLen)
{
	int err = 0;
	uint8_t *p;
	while(1)
	{
		p = inBuf;
		err = uart_recv( uart, p, 3, 0);
		if(err <=0 )
			goto exit;
		if((inBuf[0] != 0x1b)||(inBuf[1] != 0x5b)||(inBuf[2] != 0x48))
			goto exit;
		err = uart_recv( uart, p+3, FRAME_LENTH*3-3, 3);
		if(err <=0 )
			goto exit;
		return FRAME_LENTH*3;
	}
exit:
	return 0;
}
#if 0
size_t uart_get_one_packet(uart_index_t uart, char* inBuf, int inBufLen)
{
	int err = 0;
	uint8_t *p;
	while(1)
	{
		p = inBuf;
		err = uart_recv( uart, p, 3, 0);
		if(err <=0 )
			goto exit;
		if((inBuf[0] != 0x1b)||(inBuf[1] != 0x5b)||(inBuf[2] != 0x48))
			goto exit;
		err = uart_recv( uart, p+3, FRAME_LENTH*3-3, 3);
		if(err <=0 )
			goto exit;
		return FRAME_LENTH*3;
	}
exit:
	return 0;
}
#endif