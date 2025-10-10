/*
 * global.c
 *
 *  Created on: Oct 9, 2025
 *      Author: Admin
 */
#include "global.h"

uint16_t uart2_idx = 0;
uint16_t uart2_tranfer_count = 0;
uint8_t uart2_rx_buffer[UART2_BUFFER_SIZE];
uint8_t uart2_copy_flag = 1;

