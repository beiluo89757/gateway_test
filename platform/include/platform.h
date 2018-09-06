#pragma once

#define UART_422    "/dev/tty.usbserial-FT1CPP6G"//"/dev/tty.usbserial-FTZ55T4K"//"/dev/tty.usbserial"//"/dev/ttyCOM1"
#define UART_232    "/dev/ttyCOM2"


typedef enum{
    UART_COM1,
    UART_COM2,
    UART_MAX, /* Denotes the total number of UART port aliases. Not a valid UART alias */
    UART_NONE,
}uart_index_t;

