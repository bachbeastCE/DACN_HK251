/*
 * battery.h
 *
 *  Created on: Nov 5, 2025
 *      Author: NGUYEN DUY BACH
 */

#ifndef INC_BATTERY_H_
#define INC_BATTERY_H_

#include "main.h"

#define BATTERY_HANDLE_ADC hadc1
extern ADC_HandleTypeDef BATTERY_HANDLE_ADC;

#define BAT_R1_VALUE 218000.0f
#define BAT_R2_VALUE 66000.0f

#define MAX_CAPACITY_VOLTAGE 4.2f
#define MIN_CAPACITY_VOLTAGE 3.0f
#define V_REF 3.2f



#define ADC_SAMPLES 20
#define ADC_SAMPLES_TIME 1000 //500ms

#define BAT_RTOS 1

void Battery_Init(void);
void Battery_Run(void);
float Battery_Get_Voltage(void);
float Battery_Get_Percent(void);

void Battery_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);

#endif /* INC_BATTERY_H_ */
