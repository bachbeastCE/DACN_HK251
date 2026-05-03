/*
 * serial.h
 *
 *  Created on: Oct 10, 2025
 *      Author: NGUYEN DUY BACH
 */

#ifndef INC_SERIAL_H_
#define INC_SERIAL_H_

#include "global.h"
#include "string.h"

/*** Redefine if necessary ***/
#define SERIAL_UART_PORT huart6
#define TX_BUFFER_SIZE 256
#define TIMEOUT 100

extern UART_HandleTypeDef SERIAL_UART_PORT;

/*** Function ***/
void Serial_Print(const char *fmt, ...);

#endif /* INC_SERIAL_H_ */


