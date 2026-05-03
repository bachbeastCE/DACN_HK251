///*
// * imu_10DOF.h
// *
// *  Created on: Jan 28, 2026
// *      Author: Admin
// */
//
#ifndef INC_IMU_10DOF_H_
#define INC_IMU_10DOF_H_

#include <stdint.h>
#include "main.h"
#include <string.h>
#include <serial.h>
#include <math.h>
#include <stdio.h>
#include <stm32f4xx_hal.h>
//
//// ---- IMU ----
typedef struct imu_t {
	float ax, ay, az; //accel_ADXL345
	float aoffsetx, aoffsety, aoffsetz;
	float gx, gy, gz; //gyro_ITG3205
	float goffsetx, goffsety, goffsetz;
	float mx, my, mz; //mag_VCM5883L
	float moffsetx, moffsety, moffsetz;
	float mscalex, mscaley, mscalez;
	float gyrox_noise, gyroy_noise, gyroz_noise;
    float pitch; //elevation
    float yaw; //azimuth
    float roll;
    float temperature, pressure, altitude; //baro
} imu_t;
//
//
//#define PI 3.14159265358979323846f
//#define RAD_TO_DEG 57.295779513082320876798154814105
//
////Accelerometer Register
//#define ACCE_ID 0x00 // Chip ID register
//#define ACCE_POWER_CTL_REG 0x2D
//#define ACCE_BW_RATE_REG 0x2C
//#define ACCE_DATA_FORMAT_REG 0x31
//#define ACCE_DATAX0_REG 0x32
//#define ACCE_DATAX1_REG 0x33
//#define ACCE_DATAY0_REG 0x34
//#define ACCE_DATAY1_REG 0x35
//#define ACCE_DATAZ0_REG 0x36
//#define ACCE_DATAZ1_REG 0x37
//#define ACCE_OFSX_REG 0x1E
//#define ACCE_OFSY_REG 0x1F
//#define ACCE_OFSZ_REG 0x20
//#define ACCE_ADDR (0x53 << 1) // I2C 7-bit address = 0x0C
//
////Gyroscope Register
//#define GYRO_ID 0x00 // Chip ID register
//#define GYRO_DLPF_CFG_REG    0x16
////#define GYRO_FS_SEL_REG      0x16
//#define GYRO_SMPLRT_DIV_REG  0x15
//#define GYRO_PWR_MANAG_REG   0x3E
//#define GYRO_DATAX0_REG 0x1D
//#define GYRO_DATAX1_REG 0x1E
//#define GYRO_DATAY0_REG 0x1F
//#define GYRO_DATAY1_REG 0x20
//#define GYRO_DATAZ0_REG 0x21
//#define GYRO_DATAZ1_REG 0x22
//#define GYRO_ADDR (0x68 << 1) // I2C 7-bit address = 0x0C
//
////Magnetometer Register
//#define MAG_ID        0x0C   // Chip ID register
//#define MAG_ADDR          (0x0C << 1)  // I2C 7-bit address = 0x0C
//#define MAG_DATAX0_REG    0x00
//#define MAG_DATAX1_REG    0x01
//#define MAG_DATAY0_REG    0x02
//#define MAG_DATAY1_REG    0x03
//#define MAG_DATAZ0_REG    0x04
//#define MAG_DATAZ1_REG    0x05
//#define MAG_CTRL_REG2     0x0A
//#define MAG_CTRL_REG1     0x0B
//
//Barometer Register
#define BAR_ID        0xD0  // Chip ID register val = 0x58
#define BAR_ADDR          (0x77 << 1)  // I2C 7-bit address = 0x0C
#define BAR_RST  	  0xE0 // val = 0xB6
#define BAR_STATUS  	  0xF3  //bits: 3 measuring; 0 im_update
#define BAR_CTRL_MEAS  	  0xF4 //osrs_t[7:5]: temperature | osrs_p[4:2]: pressure | mode[1:0]
#define BAR_CONF  	  0xF5 //t_sb[7:5]: standby time | filter[4:2]: filter
#define BAR_DATAX0_REG    0x00
#define BMP280_REG_PRESS_XLSB  0xF9 //[3:0]bits: 7-4
#define BMP280_REG_PRESS_LSB   0xF8 //[11:4]
#define BMP280_REG_PRESS_MSB   0xF7 //[19:12]
#define BMP280_REG_TEMP_XLSB  0xFC //[3:0]bits: 7-4
#define BMP280_REG_TEMP_LSB   0xFB //[11:4]
#define BMP280_REG_TEMP_MSB   0xFA //[19:12]

typedef enum{
	 BMP280_SLEEP_MODE = 0,
	 BMP280_FORCED_MODE = 1,
	 BMP280_NORMAL_MODE = 3
} BMP280_POWER_MODE;

typedef enum{
	 BMP280_SKIP = 0,
	 BMP280_ULTRA_LOW_POWER = 1, //X1
	 BMP280_LOW_POWER = 2, //X2
	 BMP280_STANDARD_RESOLUTION = 3, //X4
	 STANDARD_HIGH_RESOLUTION = 4, //X8
	 BMP280_ULTRA_HIGH_RESOLUTION = 5, //X16
} BMP280_OVERSAMPLING;

typedef enum{
	 BMP280_FILTER_OFF = 0,
	 BMP280_FILTER_2 = 1, //X1
	 BMP280_FILTER_4 = 2, //X2
	 BMP280_FILTER_8 = 3, //X4
	 BMP280_FILTER_16 = 4, //X8
} BMP280_FILTER;

typedef enum {
    BMP280_STANDBY_05 = 0,      //stand 0.5ms
    BMP280_STANDBY_62 = 1,     	//stand 62.5ms
    BMP280_STANDBY_125 = 2,     //stand 125ms
    BMP280_STANDBY_250 = 3,     //stand 250ms
    BMP280_STANDBY_500 = 4,     //stand 500ms
    BMP280_STANDBY_1000 = 5,    //stand 1s
    BMP280_STANDBY_2000 = 6,    //stand 2s BMP280
    BMP280_STANDBY_4000 = 7,    //stand 4s BMP280
} BMP280_StandbyTime;

typedef struct{
	BMP280_POWER_MODE mode;
	BMP280_StandbyTime standtime;
	BMP280_OVERSAMPLING pressure_oversampling;
	BMP280_FILTER filter;
	uint16_t digP1, digT1;
	int16_t digP2, digP3, digP4, digP5, digP6, digP7, digP8, digP9, digT2, digT3;
} BMP280;
//// ---- Declare IMU ----
extern imu_t imu;
extern BMP280 bmp;
//extern uint8_t data_acce[6];
//extern uint8_t data_gyro[6];
//extern uint8_t data_mag[6];
//typedef enum {
//    IMU_IDLE,
//    IMU_ACC_TX,
//    IMU_ACC_RX,
//    IMU_MAG_TX,
//    IMU_MAG_RX,
//    IMU_DONE
//} IMU_PipelineState_t;
//
//extern volatile IMU_PipelineState_t imuState;
//// ---- INIT && READ && WRITE ----
//void accel_init(I2C_HandleTypeDef *I2Cx);
//void accel_read(I2C_HandleTypeDef *I2Cx);
//
//void gyro_init(I2C_HandleTypeDef *I2Cx);
//void gyro_read(I2C_HandleTypeDef *I2Cx);
//
//void mag_init(I2C_HandleTypeDef *I2Cx);
//void mag_read(I2C_HandleTypeDef *I2Cx);
//
void baro_init(I2C_HandleTypeDef *I2Cx);
void baro_read(I2C_HandleTypeDef *I2Cx);
//
//void imu_init(I2C_HandleTypeDef *I2Cx);
//void imu_read(I2C_HandleTypeDef *I2Cx);
//void imu_compute_attitude();
//// Scan address
//void I2C_Scan(I2C_HandleTypeDef *I2Cx);
//void calibrate(I2C_HandleTypeDef *I2Cx);
//void baro_read_dma_start(I2C_HandleTypeDef *I2Cx);
//void mag_read_dma_start(I2C_HandleTypeDef *I2Cx);
//void acc_read_dma_start(I2C_HandleTypeDef *I2Cx);
//void gyro_read_dma_start(I2C_HandleTypeDef *I2Cx);
//void imu_start_dma(I2C_HandleTypeDef *I2Cx);
#endif /* INC_IMU_10DOF_H_ */
