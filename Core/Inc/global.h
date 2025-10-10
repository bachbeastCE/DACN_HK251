/*
 * global.h
 *
 *  Created on: Oct 9, 2025
 *      Author: Admin
 */

#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_
#include <main.h>
#include <stdio.h>
#include <string.h>

#include "gps.h"
#include "geodesic.h"

//UART TRANMISION
#define UART2_BUFFER_SIZE 1024

extern uint16_t uart2_idx;
extern uint16_t uart2_tranfer_count;
extern uint8_t uart2_copy_flag;
extern uint8_t uart2_rx_buffer[UART2_BUFFER_SIZE];
//extern UART_HandleTypeDef huart2;


#endif /* INC_GLOBAL_H_ */
