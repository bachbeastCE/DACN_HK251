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
#include <string.h>
#include <math.h>

// ---- IMU ----
typedef struct {
	float ax, ay, az; //accel_ADXL345
	float aoffsetx, aoffsety, aoffsetz;
	float gx, gy, gz; //gyro_ITG3205
	float goffsetx, goffsety, goffsetz;
	float mx, my, mz; //mag_VCM5883L
	float moffsetx, moffsety, moffsetz;
    float pitch; //elevation
    float yaw; //azimuth
    float roll;
} imu_t;
extern imu_t imu;

#define PI 3.14159265358979323846f
#define RAD_TO_DEG 57.295779513082320876798154814105

//Accelerometer Register
#define ACCE_ID 0x00 // Chip ID register
#define ACCE_POWER_CTL_REG 0x2D
#define ACCE_BW_RATE_REG 0x2C
#define ACCE_DATA_FORMAT_REG 0x31
#define ACCE_DATAX0_REG 0x32
#define ACCE_DATAX1_REG 0x33
#define ACCE_DATAY0_REG 0x34
#define ACCE_DATAY1_REG 0x35
#define ACCE_DATAZ0_REG 0x36
#define ACCE_DATAZ1_REG 0x37
#define ACCE_OFSX_REG 0x1E
#define ACCE_OFSY_REG 0x1F
#define ACCE_OFSZ_REG 0x20
#define ACCE_ADDR (0x53 << 1) // I2C 7-bit address = 0x0C

//Gyroscope Register
#define GYRO_ID 0x00 // Chip ID register
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
#define GYRO_ADDR (0x68 << 1) // I2C 7-bit address = 0x0C

//Magnetometer Register
#define MAG_ID        0x0C   // Chip ID register
#define MAG_ADDR          (0x0C << 1)  // I2C 7-bit address = 0x0C
#define MAG_DATAX0_REG    0x00
#define MAG_DATAX1_REG    0x01
#define MAG_DATAY0_REG    0x02
#define MAG_DATAY1_REG    0x03
#define MAG_DATAZ0_REG    0x04
#define MAG_DATAZ1_REG    0x05
#define MAG_CTRL_REG2     0x0A
#define MAG_CTRL_REG1     0x0B
// ---- Declare IMU ----


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
void calibrate(I2C_HandleTypeDef *I2Cx);

#endif /* INC_IMU_H_ */
