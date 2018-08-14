#include "platform_peripheral.h"

#define platform_uart_log(M, ...) custom_log("platform_uart", M, ##__VA_ARGS__)

OSStatus uart_set_option(platform_uart_drivers_t uart);

platform_uart_drivers_t platform_uart_drivers[UART_MAX];

OSStatus platform_uart_init(uart_index_t uart ,uint32_t baud_rate, platform_uart_data_width_t data_width,platform_uart_parity_t parity,platform_uart_stop_bits_t stop_bits)
{

	OSStatus err = 0;
	if(uart == UART_COM1)
		strcpy(platform_uart_drivers[uart].name,UART_422);
	else if(uart == UART_COM2)
		strcpy(platform_uart_drivers[uart].name,UART_232);

	platform_uart_drivers[uart].uart_config.baud_rate = baud_rate;
	platform_uart_drivers[uart].uart_config.data_width = data_width;
	platform_uart_drivers[uart].uart_config.parity = parity;
	platform_uart_drivers[uart].uart_config.stop_bits = stop_bits;

	if((platform_uart_drivers[uart].fd = open(platform_uart_drivers[uart].name, O_RDWR|O_NOCTTY|O_NDELAY))<0)
    {
		platform_uart_log("open %s is failed",platform_uart_drivers[uart].name);
		err = -1;
		goto exit;
	}
    else
    {
        platform_uart_log("open %s is success , uart_fd: %d",platform_uart_drivers[uart].name,platform_uart_drivers[uart].fd);
		err = uart_set_option(platform_uart_drivers[uart]);
		if(err<0)
			goto exit;
    }
exit:
	return err;
}

OSStatus uart_set_option(platform_uart_drivers_t uart)
{
	struct termios newtio,oldtio;
	if  ( tcgetattr( uart.fd,&oldtio)  !=  0) { 
		platform_uart_log("SetupSerial 1");
		return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag  |=  CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	switch( uart.uart_config.data_width )
	{
	case DATA_WIDTH_7BIT:
		newtio.c_cflag |= CS7;
		break;
	case DATA_WIDTH_8BIT:
		newtio.c_cflag |= CS8;
		break;
	}

	switch( uart.uart_config.parity )
	{
	case ODD_PARITY:
		newtio.c_cflag |= PARENB;
		newtio.c_cflag |= PARODD;
		newtio.c_iflag |= (INPCK | ISTRIP);
		break;
	case EVEN_PARITY: 
		newtio.c_iflag |= (INPCK | ISTRIP);
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	case NO_PARITY:  
		newtio.c_cflag &= ~PARENB;
		break;
	}

	switch( uart.uart_config.baud_rate )
	{
	case 2400:
		cfsetispeed(&newtio, B2400);
		cfsetospeed(&newtio, B2400);
		break;
	case 4800:
		cfsetispeed(&newtio, B4800);
		cfsetospeed(&newtio, B4800);
		break;
	case 9600:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	case 115200:
		cfsetispeed(&newtio, B115200);
		cfsetospeed(&newtio, B115200);
		break;
	default:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	}
	if( uart.uart_config.stop_bits == STOP_BITS_1 )
		newtio.c_cflag &=  ~CSTOPB;
	else if ( uart.uart_config.stop_bits == STOP_BITS_2 )
		newtio.c_cflag |=  CSTOPB;

	newtio.c_cc[VTIME]  = 0;
	newtio.c_cc[VMIN] = 0;
	tcflush(uart.fd,TCIFLUSH);
	if((tcsetattr(uart.fd,TCSANOW,&newtio))!=0)
	{
		platform_uart_log("com set error");
		return -1;
	}
	platform_uart_log("set uart option done!");
	return 0;
}


size_t uart_recv(uart_index_t uart, char* inBuf, int inBufLen, int timeout)
{
	int ret = 0;
    int datalen = 0;
    int index =0;
  
 	fd_set readfs;  
	struct timeval Timeout;

	Timeout.tv_sec = timeout;
	Timeout.tv_usec = 0;

    while(1)
    {
		FD_ZERO(&readfs);
        FD_SET(platform_uart_drivers[uart].fd, &readfs); 
		if(Timeout.tv_sec == 0)
			ret = select(platform_uart_drivers[uart].fd+1, &readfs, NULL, NULL, NULL);
		else
			ret = select(platform_uart_drivers[uart].fd+1, &readfs, NULL, NULL, &Timeout);	
		if(ret > 0)
		{
			if(FD_ISSET(platform_uart_drivers[uart].fd,&readfs))
			{
				datalen += read(platform_uart_drivers[uart].fd, inBuf+datalen, inBufLen-datalen);
				if(datalen < inBufLen)
				{			
					continue;
				}
				else
				{
					return datalen;
				}
			}
		}
		else
		{
			return 0;
		}
    }
}

size_t UartSend( uart_index_t uart, const void* data, uint32_t size)
{
	int send_len =0;
	send_len = write(platform_uart_drivers[uart].fd,data, size);
	// printRawData(U_SEND,data,size);
    return send_len;
}