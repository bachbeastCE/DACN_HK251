/*
 * battery.c
 *
 *  Created on: Nov 5, 2025
 *      Author: NGUYEN DUY BACH
 */

#include "battery.h"
#include <math.h>
#include "global.h"
#include "cmsis_os.h" // Thêm để dùng osDelay trong Task nếu cần

static float battery_voltage_tmp = 0.0f;
static float battery_percent_tmp = 0.0f;

// EMA filter
static float filtered_batt_voltage = 0.0f;
static const float ALPHA = 0.1f;
static uint8_t is_first_read = 3;

void Battery_Init(void)
{
    // Chỉ reset trạng thái bộ lọc ban đầu
    battery_voltage_tmp = 0.0f;
    battery_percent_tmp = 0.0f;
}

/**
 * Map voltage to percent according to:
 * 4.2 -> 100%
 * 3.2 -> 10%
 * 3.0 -> 0%
 */
static float VoltageToPercent(float v)
{
    const float V_FULL = 4.20f;
    const float V_LINEAR_LOW = 3.20f;
    const float V_EMPTY = 3.00f;
    const float P_FULL = 100.0f;
    const float P_AT_3_2 = 10.0f;

    if (v >= V_FULL) return P_FULL;
    if (v <= V_EMPTY) return 0.0f;

    if (v >= V_LINEAR_LOW) {
        float slope = (P_FULL - P_AT_3_2) / (V_FULL - V_LINEAR_LOW);
        return P_AT_3_2 + (v - V_LINEAR_LOW) * slope;
    } else {
        float x = (v - V_EMPTY) / (V_LINEAR_LOW - V_EMPTY);
        if (x < 0.0f) x = 0.0f;
        if (x > 1.0f) x = 1.0f;
        const float alpha = 2.0f;
        return P_AT_3_2 * powf(x, alpha);
    }
}

void Battery_Run(void)
{
    uint32_t adc_sum_local = 0;
    uint8_t valid_samples = 0;

    for (uint8_t i = 0; i < ADC_SAMPLES; i++)
    {
        HAL_ADC_Start(&BATTERY_HANDLE_ADC); // Kích hoạt mẫu đo

        if (HAL_ADC_PollForConversion(&BATTERY_HANDLE_ADC, 1) == HAL_OK)
        {
            adc_sum_local += HAL_ADC_GetValue(&BATTERY_HANDLE_ADC);
            valid_samples++;
        }

        HAL_ADC_Stop(&BATTERY_HANDLE_ADC);
    }

    if (valid_samples > 0)
    {
        float avg_adc = (float)adc_sum_local / (float)valid_samples;
        float v_adc = (avg_adc * V_REF) / 4095.0f;

#if BATTERY_ENABLE_SERIAL_LOG
        Serial_Print("[Battery] Read ADC voltage: %.2f V \r\n", v_adc);
#endif

        battery_voltage_tmp = v_adc * ((BAT_R1_VALUE + BAT_R2_VALUE) / (float)BAT_R1_VALUE);

#if BATTERY_ENABLE_SERIAL_LOG
        Serial_Print("[Battery] Read battery voltage: %.2f V \r\n", battery_voltage_tmp);
#endif


        if (battery_voltage_tmp > MAX_CAPACITY_VOLTAGE)
            battery_voltage_tmp = MAX_CAPACITY_VOLTAGE;
        else if (battery_voltage_tmp < MIN_CAPACITY_VOLTAGE)
            battery_voltage_tmp = MIN_CAPACITY_VOLTAGE;

        if (is_first_read > 0)
        {
            filtered_batt_voltage = battery_voltage_tmp;
            is_first_read--;
        }
        else
        {
            filtered_batt_voltage = (ALPHA * battery_voltage_tmp) + ((1.0f - ALPHA) * filtered_batt_voltage);
        }

        battery_percent_tmp = VoltageToPercent(filtered_batt_voltage);

#if BATTERY_ENABLE_SERIAL_LOG
        Serial_Print("[Battery] Filtered battery voltage: %.2f V \r\n", filtered_batt_voltage);
#endif
    }
}

float Battery_Get_Voltage(void)
{
#if BATTERY_ENABLE_SERIAL_LOG
	Serial_Print("[Battery] Get Battery voltage: %.2f V \r\n", battery_voltage_tmp);
#endif
	return filtered_batt_voltage;
}


float Battery_Get_Percent(void)
{
#if BATTERY_ENABLE_SERIAL_LOG
	Serial_Print("[Battery] Get Battery percent: %.1f %%\r\n", battery_percent_tmp);
#endif
	return battery_percent_tmp;
}
