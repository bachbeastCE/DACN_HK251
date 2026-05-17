/*
 * battery.c
 *
 *  Created on: May 1, 2026
 *      Author: NGUYEN DUY BACH
 *      Refactored for CMSIS-RTOS V1 Integration
 */

#include "battery.h"
#include <math.h>
#include "global.h"
#include "cmsis_os.h"

// Biến static dùng nội bộ cho việc lấy mẫu trong ISR
static volatile uint32_t adc_sum = 0;
static volatile uint8_t adc_count = 0;

// Biến lưu kết quả lọc EMA
static volatile float filtered_batt_voltage = 0.0f;
static const float ALPHA = 0.1f;
static uint8_t is_first_read = 3;

// Định nghĩa Semaphore theo chuẩn CMSIS-RTOS V1
osSemaphoreId batterySemHandle = NULL;
osSemaphoreDef(batterySem);

/**
 * @brief Khởi tạo module đo pin
 */
void Battery_Init(void)
{
    adc_sum = 0;
    adc_count = 0;

    // Tạo Binary Semaphore nếu chưa được tạo
    if (batterySemHandle == NULL)
    {
        batterySemHandle = osSemaphoreCreate(osSemaphore(batterySem), 1);

        // Mặc định trừ đi 1 token để Semaphore ở trạng thái khóa (Block ngay từ đầu)
        osSemaphoreWait(batterySemHandle, 0);
    }
}

/**
 * @brief Callback khi ADC chuyển đổi xong (Được gọi từ ngữ cảnh Ngắt - ISR)
 */
void Battery_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == BATTERY_HANDLE_ADC.Instance)
    {
        uint32_t value = HAL_ADC_GetValue(hadc);
        adc_sum += value;
        adc_count++;

        if (adc_count < ADC_SAMPLES)
        {
            // Tiếp tục kích hoạt phát súng tiếp theo bằng hàm _IT
            HAL_ADC_Start_IT(hadc);
        }
        else
        {
            // Đã đủ số mẫu, dừng bộ quét ADC ngắt
            HAL_ADC_Stop_IT(hadc);

            // Giải phóng Semaphore để đánh thức hàm đang đợi
            if (batterySemHandle != NULL)
            {
                osSemaphoreRelease(batterySemHandle);
            }
        }
    }
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

/**
 * @brief Thực hiện 1 chu kỳ lấy mẫu và tính toán điện áp Pin (Blocking dùng Semaphore)
 */
void Battery_Run(void)
{
    float battery_voltage_tmp = 0.0f;

    if (batterySemHandle == NULL) return;

    // KHẮC PHỤC LỖI: Xả sạch toàn bộ Token cũ còn sót lại trong Semaphore (nếu có)
    while (osSemaphoreWait(batterySemHandle, 0) > 0);

    // Reset các biến đếm trước khi kích hoạt đợt quét mới
    adc_sum = 0;
    adc_count = 0;

    // Bật phát súng kích hoạt ADC đầu tiên bằng Ngắt
    HAL_ADC_Start_IT(&BATTERY_HANDLE_ADC);

    // Chờ Ngắt thu thập đủ mẫu thông qua Semaphore (Timeout 100ms chống treo)
    if (osSemaphoreWait(batterySemHandle, 200) > 0)
    {
        // ---- ĐO THÀNH CÔNG: Tính toán dữ liệu hình học ----
        float avg_adc = (float)adc_sum / (float)ADC_SAMPLES;
        float v_adc = (avg_adc * V_REF) / 4095.0f;

        battery_voltage_tmp = v_adc * ((BAT_R1_VALUE + BAT_R2_VALUE) / (float)BAT_R1_VALUE);

        // Kẹp dòng bảo vệ ngưỡng điện áp
        if (battery_voltage_tmp > MAX_CAPACITY_VOLTAGE)
            battery_voltage_tmp = MAX_CAPACITY_VOLTAGE;
        else if (battery_voltage_tmp < MIN_CAPACITY_VOLTAGE)
            battery_voltage_tmp = MIN_CAPACITY_VOLTAGE;

        // Bộ lọc EMA xử lý mượt nhiễu
        if (is_first_read > 0)
        {
            filtered_batt_voltage = battery_voltage_tmp;
            is_first_read--;
        }
        else
        {
            filtered_batt_voltage = (ALPHA * battery_voltage_tmp) + ((1.0f - ALPHA) * filtered_batt_voltage);
        }

#if BATTERY_ENABLE_SERIAL_LOG
        Serial_Print("[Battery] Measured: %.2f V, Filtered: %.2f V\r\n", battery_voltage_tmp, filtered_batt_voltage);
#endif
    }
    else
    {
        // ---- THẤT BẠI: Quá 200ms kẹt phần cứng ----
        HAL_ADC_Stop_IT(&BATTERY_HANDLE_ADC);
#if BATTERY_ENABLE_SERIAL_LOG
        Serial_Print("[Battery] Error: ADC Hardware Timeout!\r\n");
#endif
    }
}

/**
 * @brief Lấy giá trị điện áp pin hiện tại (Volt)
 */
float Battery_Get_Voltage(void)
{
    return filtered_batt_voltage;
}

/**
 * @brief Lấy phần trăm dung lượng pin hiện tại (%)
 */
float Battery_Get_Percent(void)
{
    return VoltageToPercent(filtered_batt_voltage);
}
