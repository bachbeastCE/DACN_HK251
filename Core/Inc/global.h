/*
 * global.h
 *
 *  Created on: Sep 18, 2025
 *  Author: NGUYEN DUY BACH
 */

#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include <main.h>
#include <stdio.h>
#include <string.h>

#include "gps.h"
#include "geodesic.h"

//UART TRANMISION
#define UART3_BUFFER_SIZE 1024

extern uint16_t uart3_idx;
extern uint16_t uart3_tranfer_count;
extern uint8_t uart3_copy_flag;
extern uint8_t uart3_rx_buffer[UART3_BUFFER_SIZE];
extern UART_HandleTypeDef huart2;



#endif /* INC_GLOBAL_H_ */
