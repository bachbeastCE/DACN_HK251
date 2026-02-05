/*
 * imu.h
 *
 *  Created on: Sep 17, 2025
 *      Author: Admin
 */

#ifndef INC_IMU_H_
#define INC_IMU_H_
#include <stdint.h>
#include <serial.h>
#include "main.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "imu_10DOF.h"

// ---- IMU ----
//typedef struct {
//	float ax, ay, az; //accel_ADXL345
//	float aoffsetx, aoffsety, aoffsetz;
//	float gx, gy, gz; //gyro_ITG3205
//	float goffsetx, goffsety, goffsetz;
//	float mx, my, mz; //mag_VCM5883L
//	float moffsetx, moffsety, moffsetz;
//	float mscalex, mscaley, mscalez;
//	float gyrox_noise, gyroy_noise, gyroz_noise;
//    float pitch; //elevation
//    float yaw; //azimuth
//    float roll;
//} imu_t;
//extern imu_t imu;
extern volatile uint8_t imu_data_ready;
#define PI 3.14159265358979323846f
#define RAD_TO_DEG 57.295779513082320876798154814105


#define MPU_I2C_ADDR            (0x68 << 1)
#define WHO_AM_ADDR             0x75
#define INT_PIN_CFG 			0x37
//------------------ POWER MANAGEMENT ------------------
#define MPU9250_PWR_MGMT_1      0x6B //reset
#define MPU9250_PWR_MGMT_2      0x6C //turn on or off for 3 axis
//------------------ SAMPLE RATE ------------------
#define MPU9250_SMPLRT_DIV      0x19

//------------------ IDENTIFICATION ------------------
#define MPU9250_WHO_AM_I        0x75  // should return 0x71
//------------------ CONFIGURATION REGISTERS ------------------
#define MPU9250_CONFIG          0x1A  // DLPF, EXT_SYNC
#define MPU9250_GYRO_CONFIG     0x1B  // Gyro full-scale range (250, 500, 1000, 2000)
#define MPU9250_ACCEL_CONFIG    0x1C  // Accel full-scale range (2, 4, 8, 16)
#define MPU9250_ACCEL_CONFIG_2  0x1D  // Accel DLPF, filter bandwidth

typedef enum {
    CLOCK_INTERNAL_8MHZ = 0x00,
    CLOCK_PLL_GYRO_X    = 0x01,
    CLOCK_PLL_GYRO_Y    = 0x02,
    CLOCK_PLL_GYRO_Z    = 0x03
} MPU_ClockSource;

//------------------ GYROSCOPE ------------------
#define MPU9250_GYRO_XOUT_H     0x43
#define MPU9250_GYRO_XOUT_L     0x44
#define MPU9250_GYRO_YOUT_H     0x45
#define MPU9250_GYRO_YOUT_L     0x46
#define MPU9250_GYRO_ZOUT_H     0x47
#define MPU9250_GYRO_ZOUT_L     0x48

#define MPU9250_XG_OFFSET_H     0x13
#define MPU9250_XG_OFFSET_L     0x14
#define MPU9250_YG_OFFSET_H     0x15
#define MPU9250_YG_OFFSET_L     0x16
#define MPU9250_ZG_OFFSET_H     0x17
#define MPU9250_ZG_OFFSET_L     0x18

typedef enum{//use DLPF => bit 1 and bit 0 must be 0
	GYRO_RANGE_250DPS = 0x00, //00000000
	GYRO_RANGE_500DPS = 0x08, //00001000
	GYRO_RANGE_1000DPS = 0x10, //00010000
	GYRO_RANG_2000DPS = 0x18, //00011000
} gyro_range;

typedef enum {
    GYRO_DLPF_5HZ   = 0x06,
    GYR0_DLPF_10HZ  = 0x05,
    GYRO_DLPF_20HZ  = 0x04,
    GYRO_DLPF_41HZ  = 0x03,
    GYRO_DLPF_92HZ  = 0x02,
    GYRO_DLPF_184HZ = 0x01,
    GYR0_DLPF_250HZ = 0x00
} gyro_DLPF_Bandwidth;

//------------------ ACCELEROMETER ------------------
#define MPU9250_ACCEL_XOUT_H    0x3B
#define MPU9250_ACCEL_XOUT_L    0x3C
#define MPU9250_ACCEL_YOUT_H    0x3D
#define MPU9250_ACCEL_YOUT_L    0x3E
#define MPU9250_ACCEL_ZOUT_H    0x3F
#define MPU9250_ACCEL_ZOUT_L    0x40

#define MPU9250_XA_OFFSET_H     0x77
#define MPU9250_XA_OFFSET_L     0x78
#define MPU9250_YA_OFFSET_H     0x7A
#define MPU9250_YA_OFFSET_L     0x7B
#define MPU9250_ZA_OFFSET_H     0x7D
#define MPU9250_ZA_OFFSET_L     0x7E

typedef enum{
	ACCE_RANGE_2G = 0x00,
	ACCE_RANGE_4G = 0x01,
	ACCE_RANGE_8G = 0x10,
	ACCE_RANG_16G = 0x11,
} acce_range;

typedef enum {
    ACCE_DLPF_460HZ  = 0x00, // A_DLPFCFG = 0
    ACCE_DLPF_184HZ  = 0x01, // A_DLPFCFG = 1
    ACCE_DLPF_92HZ   = 0x02, // A_DLPFCFG = 2
    ACCE_DLPF_41HZ   = 0x03, // A_DLPFCFG = 3
    ACCE_DLPF_20HZ   = 0x04, // A_DLPFCFG = 4
    ACCE_DLPF_10HZ   = 0x05, // A_DLPFCFG = 5
    ACCE_DLPF_5HZ    = 0x06  // A_DLPFCFG = 6
} acce_DLPF_Bandwidth;

//------------------ MAGNETOMETER (AK8963) ------------------
#define AK8963_I2C_ADDR       (0x0C << 1)
#define AK8963_WIA            0x00  // Who Am I register (0x48)
#define AK8963_ST1            0x02  // Data status 1
#define AK8963_HXL            0x03  // X-axis data low byte
#define AK8963_HXH            0x04  // X-axis data high byte
#define AK8963_HYL            0x05  // Y-axis data low byte
#define AK8963_HYH            0x06  // Y-axis data high byte
#define AK8963_HZL            0x07  // Z-axis data low byte
#define AK8963_HZH            0x08  // Z-axis data high byte
#define AK8963_ST2            0x09  // Data status 2 (check overflow)
#define AK8963_CNTL1          0x0A  // Control register 1 (mode select)
#define AK8963_CNTL2          0x0B  // Control register 2 (reset)
#define AK8963_ASAX           0x10  // Sensitivity adjustment X
#define AK8963_ASAY           0x11  // Sensitivity adjustment Y
#define AK8963_ASAZ           0x12  // Sensitivity adjustment Z



// ---- INIT && READ && WRITE ----
void write (uint8_t reg, uint8_t value, I2C_HandleTypeDef *I2Cx, uint8_t devaddress);
void accel_gyro_Init (I2C_HandleTypeDef *I2Cx);
void accel_read(I2C_HandleTypeDef *I2Cx);
void gyro_read(I2C_HandleTypeDef *I2Cx);

void mag_init(I2C_HandleTypeDef *I2Cx);
void mag_read(I2C_HandleTypeDef *I2Cx);

void imu_init(I2C_HandleTypeDef *I2Cx1, I2C_HandleTypeDef *I2Cx2);
void imu_compute_attitude();

// Scan address
void I2C_Scan(I2C_HandleTypeDef *I2Cx);
void calibrate(I2C_HandleTypeDef *I2Cx);

void mag_read_dma_start(I2C_HandleTypeDef *I2Cx);
void acc_read_dma_start(I2C_HandleTypeDef *I2Cx);
void gyro_read_dma_start(I2C_HandleTypeDef *I2Cx);
void imu_start_dma(I2C_HandleTypeDef *I2Cx);
#endif /* INC_IMU_H_ */
