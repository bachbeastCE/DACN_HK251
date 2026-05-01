/*
 * serial.c
 *
 *  Created on: Oct 10, 2025
 *      Author: NGUYEN DUY BACH
 */

#include "serial.h"

static uint8_t tx_buffer[TX_BUFFER_SIZE];

void Serial_Print(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int len = vsnprintf((char *)tx_buffer, TX_BUFFER_SIZE, fmt, args);
    va_end(args);

    if (len > TX_BUFFER_SIZE)
        len = TX_BUFFER_SIZE;

    // Send via UART6
    HAL_UART_Transmit(&SERIAL_UART_PORT, (uint8_t *)tx_buffer, len,100);
}

