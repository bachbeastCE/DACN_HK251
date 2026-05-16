/*
 * battery.c
 *
 *  Created on: Nov 5, 2025
 *      Author: NGUYEN DUY BACH
 */

#include "battery.h"
#include <math.h>
#include "global.h"

static volatile uint32_t adc_sum = 0;
static volatile uint8_t adc_count = 0;
static volatile uint8_t adc_ready = 0;

static float battery_voltage_tmp = 0.0f;
static float battery_percent_tmp = 0.0f;

//EMA filter
static float filtered_batt_voltage = 0.0f;
static const float ALPHA = 0.1f;
static uint8_t is_first_read = 3;


/**
 * @brief Khởi tạo module đo pin (bắt đầu đo ADC)
 */
void Battery_Init(void)
{
    adc_sum = 0;
    adc_count = 0;
    adc_ready = 0;

    //HAL_ADC_Start_IT(&BATTERY_HANDLE_ADC);
    HAL_ADC_Start(&BATTERY_HANDLE_ADC);
}

/**
 * @brief Callback khi ADC chuyển đổi xong (được gọi từ HAL)
 */
void Battery_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == BATTERY_HANDLE_ADC.Instance)
    {
#if BATTERY_ENABLE_SERIAL_LOG
    	//Serial_Print("[Battery] Battery callback\n\r",battery_percent_tmp);
#endif
    	uint32_t value = HAL_ADC_GetValue(hadc);
        adc_sum += value;
        adc_count++;

        if (adc_count < ADC_SAMPLES)
        {
        	//HAL_ADC_Start_IT(hadc);
        	HAL_ADC_Start(&BATTERY_HANDLE_ADC);

        }
        else{
        	adc_ready = 1;
        	//HAL_ADC_Stop_IT(hadc);
        	HAL_ADC_Stop(hadc);
        }

    }
}

/**
 * Map voltage to percent according to:
 * 4.2 -> 100%
 * 3.2 -> 10%
 * 3.0 -> 0%  (fast drop between 3.2 -> 3.0)
 *
 * alpha controls steepness below 3.2 (alpha >= 1).
 */
static float VoltageToPercent(float v)
{
    const float V_FULL = 4.20f;
    const float V_LINEAR_LOW = 3.20f; // boundary: end of linear region
    const float V_EMPTY = 3.00f;
    const float P_FULL = 100.0f;
    const float P_AT_3_2 = 10.0f;

    if (v >= V_FULL) return P_FULL;
    if (v <= V_EMPTY) return 0.0f;

    if (v >= V_LINEAR_LOW) {
        // linear mapping between 3.2..4.2 -> 10..100
        float slope = (P_FULL - P_AT_3_2) / (V_FULL - V_LINEAR_LOW); // = 90 / 1.0 = 90
        return P_AT_3_2 + (v - V_LINEAR_LOW) * slope;
    } else {
        // fast drop mapping between 3.0..3.2 -> 0..10
        float x = (v - V_EMPTY) / (V_LINEAR_LOW - V_EMPTY); // normalized 0..1
        if (x < 0.0f) x = 0.0f;
        if (x > 1.0f) x = 1.0f;
        const float alpha = 2.0f; // chỉnh đây để tăng/giảm độ dốc (>=1)
        float p = P_AT_3_2 * powf(x, alpha); // 10 * x^alpha
        return p;
    }
}

/**
 * @brief Xử lý sau khi đủ mẫu ADC (chạy trong main loop)
 */
void Battery_Run(void)
{
	if (adc_ready)
    {
        float avg_adc = (float)adc_sum / (float)ADC_SAMPLES;
        float v_adc = (avg_adc * V_REF) / 4095.0f;
#if BATTERY_ENABLE_SERIAL_LOG
        Serial_Print("[Battery] Read ADC voltage: %.2f V \r\n", v_adc);
#endif

        battery_voltage_tmp = v_adc * ((BAT_R1_VALUE + BAT_R2_VALUE) / (float)BAT_R1_VALUE);

#if BATTERY_ENABLE_SERIAL_LOG
        Serial_Print("[Battery] Read battery voltage: %.2f V \r\n", battery_voltage_tmp);
#endif

//         Giới hạn điện áp đo (tránh sai do nhiễu)
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
			// Từ lần thứ 2 trở đi, áp dụng công thức lọc EMA
			filtered_batt_voltage = (ALPHA * battery_voltage_tmp) + ((1.0f - ALPHA) * filtered_batt_voltage);
		}
#if BATTERY_ENABLE_SERIAL_LOG
        Serial_Print("[Battery] Filtered battery voltage: %.2f V \r\n", filtered_batt_voltage);
#endif

        // Tính % pin
        //battery_percent_tmp = VoltageToPercent(battery_voltage_tmp);

        adc_sum = 0;
        adc_count = 0;
        adc_ready = 0;

        HAL_ADC_Start(&BATTERY_HANDLE_ADC);
    }
}

/**
 * @brief Lấy giá trị điện áp pin (Volt)
 */
float Battery_Get_Voltage(void)
{
#if BATTERY_ENABLE_SERIAL_LOG
	Serial_Print("[Battery] Get Battery voltage: %.2f V \r\n", battery_voltage_tmp);
#endif
	return filtered_batt_voltage;
}

/**
 * @brief Lấy phần trăm dung lượng pin (%)
 */
float Battery_Get_Percent(void)
{
#if BATTERY_ENABLE_SERIAL_LOG
	Serial_Print("[Battery] Get Battery percent: %.1f %%\r\n",battery_percent_tmp);
#endif
	return battery_percent_tmp;
}


