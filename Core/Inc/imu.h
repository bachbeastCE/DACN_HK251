/*
 * imu.h
 *
 *  Created on: Sep 17, 2025
 *      Author: Admin
 */

#ifndef INC_IMU_H_
#define INC_IMU_H_
#include <stdint.h>
#include "main.h"

// ---- ACCELEROMETER ----
typedef struct {
    int16_t x_raw, y_raw, z_raw;
    float x_g, y_g, z_g;
} accel_t;

// ---- GYROSCOPE ----
typedef struct {
	int16_t x_raw, y_raw, z_raw;
	float x_dps, y_dps, z_dps;
} gyro_t;

// ---- MAGNETOMETER ----
typedef struct {
	int16_t x_raw, y_raw, z_raw;
	float x_uT, y_uT, z_uT;
} mag_t;

// ---- IMU ----
typedef struct {
    accel_t accel_ADXL345;
    gyro_t gyro_ITG3205;
    mag_t mag_VCM5883L;
    uint8_t status;       // 0=ok, 1=error
} imu_t;
#define RAD_TO_DEG 57.295779513082320876798154814105

//Accele reg
#define ACCE_ID 0x00
#define ACCE_POWER_CTL_REG 0x2D
#define ACCE_BW_RATE_REG 0x2C
#define ACCE_DATA_FORMAT_REG 0x31
#define ACCE_DATAX0_REG 0x32
#define ACCE_DATAX1_REG 0x33
#define ACCE_DATAY0_REG 0x34
#define ACCE_DATAY1_REG 0x35
#define ACCE_DATAZ0_REG 0x36
#define ACCE_DATAZ1_REG 0x37
#define ACCE_ADDR (0x53 << 1)

////gyro reg
#define GYRO_ID 0x00
#define GYRO_DLPF_CFG_REG    2
#define GYRO_FS_SEL_REG      0x16
#define GYRO_SMPLRT_DIV_REG  0x15
#define GYRO_PWR_MANAG_REG   0x3E
#define GYRO_DATAX0_REG 0x1D
#define GYRO_DATAX1_REG 0x1E
#define GYRO_DATAY0_REG 0x1F
#define GYRO_DATAY1_REG 0x20
#define GYRO_DATAZ0_REG 0x21
#define GYRO_DATAZ1_REG 0x22
#define GYRO_ADDR (0x68 << 1)

////mag reg
#define MAG_ID 0x00
#define GYRO_DLPF_CFG_REG    2
#define GYRO_FS_SEL_REG      0x16
#define GYRO_SMPLRT_DIV_REG  0x15
#define GYRO_PWR_MANAG_REG   0x3E
#define GYRO_DATAX0_REG 0x1D
#define GYRO_DATAX1_REG 0x1E
#define GYRO_DATAY0_REG 0x1F
#define GYRO_DATAY1_REG 0x20
#define GYRO_DATAZ0_REG 0x21
#define GYRO_DATAZ1_REG 0x22
#define MAG_ADDR (0x0C << 1)
// ---- Khai báo biến toàn cục ----
extern imu_t imu;

// ---- INIT && READ && WRITE ----
void write (uint8_t reg, uint8_t value, I2C_HandleTypeDef *I2Cx, uint8_t devaddress);
void accel_init(I2C_HandleTypeDef *I2Cx);
void accel_read(I2C_HandleTypeDef *I2Cx);

void gyro_init(I2C_HandleTypeDef *I2Cx);
void gyro_read(I2C_HandleTypeDef *I2Cx);

void mag_init(I2C_HandleTypeDef *I2Cx);
void mag_read(I2C_HandleTypeDef *I2Cx);

void imu_init(I2C_HandleTypeDef *I2Cx);
void imu_read(I2C_HandleTypeDef *I2Cx);
// Scan address
void I2C_Scan(I2C_HandleTypeDef *I2Cx);
#endif /* INC_IMU_H_ */
