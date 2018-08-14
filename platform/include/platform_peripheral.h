#pragma once

#include "platform.h"
#include "debug.h"


/**
 * UART parity
 */
typedef enum
{
    NO_PARITY,
    ODD_PARITY,
    EVEN_PARITY,
} platform_uart_parity_t;

/**
 * UART stop bits
 */
typedef enum
{
    STOP_BITS_1,
    STOP_BITS_2,
} platform_uart_stop_bits_t;

/**
 * UART data width
 */
typedef enum
{
    DATA_WIDTH_5BIT,
    DATA_WIDTH_6BIT,
    DATA_WIDTH_7BIT,
    DATA_WIDTH_8BIT,
    DATA_WIDTH_9BIT
} platform_uart_data_width_t;
/**
 * UART configuration
 */
typedef struct _platform_uart_config
{
    u_int32_t                    baud_rate;
    platform_uart_data_width_t   data_width;
    platform_uart_parity_t       parity;
    platform_uart_stop_bits_t    stop_bits;
} platform_uart_config_t;



typedef struct _platform_uart_drivers
{
    char                   name[50];
    int                    fd;
    platform_uart_config_t uart_config;
}platform_uart_drivers_t;



OSStatus platform_uart_init(uart_index_t uart ,u_int32_t baud_rate, platform_uart_data_width_t data_width,platform_uart_parity_t parity,platform_uart_stop_bits_t stop_bits);

size_t uart_recv(uart_index_t uart_fd, char* inBuf, int inBufLen, int timeout);

size_t UartSend( uart_index_t uart, const void* data, u_int32_t size);