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

    while (SERIAL_UART_PORT.gState != HAL_UART_STATE_READY)
    {
        HAL_Delay(1);
    }

    // Gửi qua UART1 dùng DMA
    HAL_UART_Transmit_DMA(&SERIAL_UART_PORT, (uint8_t *)tx_buffer, len);
}

//void mprint (const char* format, ...){
//	char buffer[562];
//    va_list args;
//    va_start(args, format);
//    int len = vsnprintf(buffer, sizeof(buffer), format, args);
//    va_end(args);
//
//    if(len > 0) {
//        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, len, HAL_MAX_DELAY);
//    }
//}
