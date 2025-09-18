/*
 * imu.c
 *
 *  Created on: Sep 17, 2025
 *      Author: Admin
 */

#include <math.h>
#include "imu.h"


uint8_t data_acce[6];
uint8_t data_gyro[6];
uint8_t data_mag[6];
const uint16_t i2c_timeout = 100;
uint8_t chipid;
imu_t imu;
void I2C_Scan(I2C_HandleTypeDef *I2Cx) {
	mprint("HELLO WORD");
    for (uint8_t addr = 1; addr < 127; addr++) {
        if (HAL_I2C_IsDeviceReady(I2Cx, addr << 1, 1, 10) == HAL_OK) {
            mprint("I2C device found at 0x%02X\r\n", addr);
        }
    }
}

void imu_init(I2C_HandleTypeDef *I2Cx){
	accel_init(I2Cx);
	gyro_init(I2Cx);
}

void imu_read(I2C_HandleTypeDef *I2Cx){
	accel_read(I2Cx);
	gyro_read(I2Cx);
}
void write (uint8_t reg, uint8_t value, I2C_HandleTypeDef *I2Cx, uint8_t devaddress)
{
	uint8_t data[2];
	data[0] = reg;
	data[1] = value;
	HAL_I2C_Master_Transmit (I2Cx, devaddress, data, 2, i2c_timeout);
}

void accel_init (I2C_HandleTypeDef *I2Cx) {
    // check device ID WHO_AM_I
	HAL_I2C_Mem_Read(I2Cx, ACCE_ADDR, ACCE_ID, 1, &chipid, 1, i2c_timeout);
    if (chipid == 229)
    {
    	write(ACCE_DATA_FORMAT_REG, 0x01, I2Cx, ACCE_ADDR);  // data_format range= +- 4g
    	write(ACCE_POWER_CTL_REG, 0x00, I2Cx, ACCE_ADDR);  // reset all bits
    	write(ACCE_POWER_CTL_REG, 0x08, I2Cx, ACCE_ADDR);  // power_cntl measure and wake up 8hz
    	mprint("ACCE wake up");
    }
}

void gyro_init (I2C_HandleTypeDef *I2Cx) {
    // check device ID WHO_AM_I
	HAL_I2C_Mem_Read(I2Cx, GYRO_ADDR, GYRO_ID, 1, &chipid, 1, i2c_timeout);
	//mprint("chipid = %d\n", chipid);
    if (chipid == 104)
    {
    	write(GYRO_FS_SEL_REG, 0x18, I2Cx, GYRO_ADDR);
    	write(GYRO_PWR_MANAG_REG, 0x80, I2Cx, GYRO_ADDR);  // reset all bits
    	write(GYRO_PWR_MANAG_REG, 0x01, I2Cx, GYRO_ADDR);  // power_cntl measure and wake up
    	mprint("GYRO wake up");
    }
}

void mag_init (I2C_HandleTypeDef *I2Cx) {
    // check device ID WHO_AM_I
	HAL_I2C_Mem_Read(I2Cx, GYRO_ADDR, GYRO_ID, 1, &chipid, 1, i2c_timeout);
	//mprint("chipid = %d\n", chipid);
    if (chipid == 104)
    {
    	write(GYRO_FS_SEL_REG, 0x18, I2Cx, GYRO_ADDR);  // data_format range= +- 4g
    	write(GYRO_PWR_MANAG_REG, 0x80, I2Cx, GYRO_ADDR);  // reset all bits
    	write(GYRO_PWR_MANAG_REG, 0x01, I2Cx, GYRO_ADDR);  // power_cntl measure and wake up
    	mprint("GYRO wake up");
    }
}

void accel_read(I2C_HandleTypeDef *I2Cx)
{
    if (HAL_I2C_Mem_Read(I2Cx, ACCE_ADDR, ACCE_DATAX0_REG, 1, data_acce, 6, i2c_timeout) != HAL_OK) {
        mprint("I2C read error!\r\n");
        return;
    }

    imu.accel_ADXL345.x_raw = (int16_t)((data_acce[1] << 8) | data_acce[0]);
    imu.accel_ADXL345.y_raw = (int16_t)((data_acce[3] << 8) | data_acce[2]);
    imu.accel_ADXL345.z_raw = (int16_t)((data_acce[5] << 8) | data_acce[4]);

    imu.accel_ADXL345.x_g = imu.accel_ADXL345.x_raw * 0.0039;
    imu.accel_ADXL345.y_g = imu.accel_ADXL345.y_raw * 0.0039;
    imu.accel_ADXL345.z_g = imu.accel_ADXL345.z_raw * 0.0039;

    mprint("Acce_X = %.3f g; \r", imu.accel_ADXL345.x_g);
    mprint("Acce_Y = %.3f g; \r", imu.accel_ADXL345.y_g);
    mprint("Acce_Z = %.3f g \n", imu.accel_ADXL345.z_g);
}

void gyro_read(I2C_HandleTypeDef *I2Cx)
{
    if (HAL_I2C_Mem_Read(I2Cx, GYRO_ADDR, GYRO_DATAX0_REG, 1, data_gyro, 6, i2c_timeout) != HAL_OK) {
        mprint("I2C read error!\r\n");
        return;
    }

    imu.gyro_ITG3205.x_raw = (int16_t)((data_gyro[0] << 8) | data_gyro[1]);
    imu.gyro_ITG3205.y_raw = (int16_t)((data_gyro[2] << 8) | data_gyro[3]);
    imu.gyro_ITG3205.z_raw = (int16_t)((data_gyro[4] << 8) | data_gyro[5]);

    imu.gyro_ITG3205.x_dps = imu.gyro_ITG3205.x_raw / 16.4f;
    imu.gyro_ITG3205.y_dps = imu.gyro_ITG3205.y_raw / 16.4f;
    imu.gyro_ITG3205.z_dps = imu.gyro_ITG3205.z_raw / 16.4f;

    mprint("Gyro_X = %.3f deg/s; \r", imu.gyro_ITG3205.x_dps);
    mprint("Gyro_Y = %.3f deg/s; \r", imu.gyro_ITG3205.y_dps);
    mprint("Gyro_Z = %.3f deg/s \n", imu.gyro_ITG3205.z_dps);
}
