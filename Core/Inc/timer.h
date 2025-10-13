/*
 * timer.h
 *
 *  Created on: Oct 13, 2025
 *      Author: ASUS
 */

#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include "main.h"

/*** Redefine if necessary ***/
#define HANDLE_SW_TIMER htim2
#define HANDLE_SW_INSTANCE TIM2

extern TIM_HandleTypeDef HANDLE_SW_TIMER;

#define MAX_TIMER 4
extern uint8_t timer_flag[MAX_TIMER];

void Timer_Init(void);
void Timer_Set(int index, int counter);
void Timer_Run(void);

#endif /* INC_TIMER_H_ */
