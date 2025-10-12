/*
 * serial.c
 *
 *  Created on: Oct 10, 2025
 *      Author: NGUYEN DUY BACH
 */

#include "serial.h"

static uint8_t tx_buffer[TX_BUFFER_SIZE];

int __io_putchar(int ch)
{
    char c = (char)ch;
    Serial_Print(&c, 1);
    return ch;
}

void Serial_Print(const char *data , size_t len)
{
    // Send directly if length of data smaller than buffer size
    if (len <= TX_BUFFER_SIZE)
    {
        HAL_UART_Transmit_DMA(&huart1, (uint8_t*)data, len);
        return;
    }

    // On the contrary, cut into chunks and send
    size_t offset = 0;
    while (offset < len)
    {
        size_t chunk = len - offset;
        if (chunk > TX_BUFFER_SIZE)
            chunk = TX_BUFFER_SIZE;

        memcpy(tx_buffer, &data[offset], chunk);
        offset += chunk; //Set new offset

        // Send packet
        HAL_UART_Transmit_DMA(&huart1, tx_buffer, chunk);

        // Wait for sending successfully
        while (huart1.gState != HAL_UART_STATE_READY)
        {
            HAL_Delay(1);
        }
    }
}
