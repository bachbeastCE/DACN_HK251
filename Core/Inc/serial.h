/*
 * serial.h
 *
 *  Created on: Oct 10, 2025
 *      Author: ASUS
 */

#ifndef INC_SERIAL_H_
#define INC_SERIAL_H_

/*** Redefine if necessary ***/
#define SERIAL_UART1_PORT huart1
extern UART_HandleTypeDef SERIAL_UART1_PORT

void Serial_Print();

#endif /* INC_SERIAL_H_ */
