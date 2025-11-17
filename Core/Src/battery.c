/*
 * battery.c
 *
 *  Created on: Nov 5, 2025
 *      Author: NGUYEN DUY BACH
 */

#include "battery.h"
#include <math.h>

static volatile uint32_t adc_sum = 0;
static volatile uint8_t adc_count = 0;
static volatile uint8_t adc_ready = 0;

static float battery_voltage_tmp = 0.0f;
static float battery_percent_tmp = 0.0f;

/**
 * @brief Khởi tạo module đo pin (bắt đầu đo ADC)
 */
void Battery_Init(void)
{
    adc_sum = 0;
    adc_count = 0;
    adc_ready = 0;

    HAL_ADC_Start_IT(&BATTERY_HANDLE_ADC);
}

/**
 * @brief Callback khi ADC chuyển đổi xong (được gọi từ HAL)
 */
void Battery_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == BATTERY_HANDLE_ADC.Instance)
    {
        uint32_t value = HAL_ADC_GetValue(hadc);
        adc_sum += value;
        adc_count++;

        if (adc_count >= ADC_SAMPLES)
        {
            adc_ready = 1;
            HAL_ADC_Stop_IT(hadc); // dừng ADC khi đủ mẫu
        }
        else
        {
            HAL_ADC_Start_IT(hadc); // tiếp tục đo mẫu kế tiếp
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
        // Trung bình giá trị ADC
        float avg_adc = (float)adc_sum / (float)ADC_SAMPLES;

        // Chuyển sang điện áp ADC (Vref = 3.3V, 12-bit)
        float v_adc = (avg_adc * 3.3f) / 4095.0f;

        // Điện áp thực tế của pin qua chia áp

        battery_voltage_tmp = 0.1 + v_adc * ((BAT_R1_VALUE + BAT_R2_VALUE) / (float)BAT_R1_VALUE);

        // Giới hạn điện áp đo (tránh sai do nhiễu)
//        if (battery_voltage_tmp > MAX_CAPACITY_VOLTAGE)
//            battery_voltage_tmp = MAX_CAPACITY_VOLTAGE;
//        else if (battery_voltage_tmp < MIN_CAPACITY_VOLTAGE)
//            battery_voltage_tmp = MIN_CAPACITY_VOLTAGE;

        // Tính % pin
        battery_percent_tmp = VoltageToPercent(battery_voltage_tmp);

        // Reset để đo lại
        adc_sum = 0;
        adc_count = 0;
        adc_ready = 0;

        // Bắt đầu chu kỳ đo mới
        HAL_ADC_Start_IT(&BATTERY_HANDLE_ADC);
    }
}

/**
 * @brief Lấy giá trị điện áp pin (Volt)
 */
float Battery_Get_Voltage(void)
{
#if BATTERY_ENABLE_SERIAL_LOG
	Serial_Print("Battery voltage: %.2f V \r\n", battery_voltage_tmp);
#endif
	return battery_voltage_tmp;
}

/**
 * @brief Lấy phần trăm dung lượng pin (%)
 */
float Battery_Get_Percent(void)
{
#if BATTERY_ENABLE_SERIAL_LOG
	Serial_Print("Battery percent: %.1f %%\r\n",battery_percent_tmp);
#endif
	return battery_percent_tmp;
}


