/*
 * imu.c
 *
 *  Created on: Sep 17, 2025
 *      Author: Admin
 */

#include <math.h>
#include "imu.h"

const float PI = 3.14159265f;
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
	mag_init(I2Cx);
}

void imu_read(I2C_HandleTypeDef *I2Cx){
	accel_read(I2Cx);
	gyro_read(I2Cx);
	mag_read(I2Cx);
	imu.pitch = atan2(-imu.ax, sqrt(imu.ay * imu.ay + imu.az * imu.az)) * 180.0 / PI;
	imu.yaw = atan2(imu.my, imu.mx);

//    mprint("pitch = %.3f degree; ", imu.pitch);
//    mprint("yaw = %.3f degree; \n", imu.yaw);
    mprint("#######################################\n");
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
    	write(ACCE_DATA_FORMAT_REG, 0x0B, I2Cx, ACCE_ADDR);  // data_format range= +- 16g, full reso
    	write(ACCE_POWER_CTL_REG, 0x00, I2Cx, ACCE_ADDR);  // reset all bits
    	write(ACCE_POWER_CTL_REG, 0x08, I2Cx, ACCE_ADDR);  // power_cntl measure and wake up 8hz
    	write(ACCE_OFSX_REG, 0x04, I2Cx, ACCE_ADDR);
    	write(ACCE_OFSY_REG, 0x09, I2Cx, ACCE_ADDR);
    	write(ACCE_OFSZ_REG, 0x0A, I2Cx, ACCE_ADDR);
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
	HAL_I2C_Mem_Read(I2Cx, MAG_ADDR, MAG_ID, 1, &chipid, 1, i2c_timeout);
	//mprint("chipid = %d\n", chipid);
    if (chipid == 130)
    {
    	write(MAG_CTRL_REG1, 0x80, I2Cx, MAG_ADDR);  // reset all bits
    	HAL_Delay(10);
    	write(MAG_CTRL_REG2, 0x41, I2Cx, MAG_ADDR); //power_cntl measure and wake up
    	mprint("MAG wake up");
    }
}

void accel_read(I2C_HandleTypeDef *I2Cx)
{
    if (HAL_I2C_Mem_Read(I2Cx, ACCE_ADDR, ACCE_DATAX0_REG, 1, data_acce, 6, i2c_timeout) != HAL_OK) {
        mprint("I2C read error!\r\n");
        return;
    }
    int16_t x_raw, y_raw, z_raw;
    x_raw = (int16_t)((data_acce[1] << 8) | data_acce[0]);
    y_raw = (int16_t)((data_acce[3] << 8) | data_acce[2]);
    z_raw = (int16_t)((data_acce[5] << 8) | data_acce[4]);

    imu.ax = x_raw * 0.004f;
    imu.ay = y_raw * 0.004f;
    imu.az = z_raw * 0.004f;

    mprint("Acce_X = %.3f g; ", imu.ax);
    mprint("Acce_Y = %.3f g; ", imu.ay);
    mprint("Acce_Z = %.3f g \n", imu.az);
}

void gyro_read(I2C_HandleTypeDef *I2Cx)
{
    if (HAL_I2C_Mem_Read(I2Cx, GYRO_ADDR, GYRO_DATAX0_REG, 1, data_gyro, 6, i2c_timeout) != HAL_OK) {
        mprint("I2C read error!\r\n");
        return;
    }

    int16_t x_raw, y_raw, z_raw;
    x_raw = (int16_t)((data_gyro[0] << 8) | data_gyro[1]);
    y_raw = (int16_t)((data_gyro[2] << 8) | data_gyro[3]);
    z_raw = (int16_t)((data_gyro[4] << 8) | data_gyro[5]);

    imu.gx = x_raw / 16.4f;
    imu.gy = y_raw / 16.4f;
    imu.gz = z_raw / 16.4f;

//    mprint("Gyro_X = %.3f deg/s; ", imu.gx);
//    mprint("Gyro_Y = %.3f deg/s; ", imu.gy);
//    mprint("Gyro_Z = %.3f deg/s \n", imu.gz);
}

void mag_read(I2C_HandleTypeDef *I2Cx)
{
    if (HAL_I2C_Mem_Read(I2Cx, MAG_ADDR, MAG_DATAX0_REG, 1, data_mag, 6, i2c_timeout) != HAL_OK) {
        mprint("I2C read error!\r\n");
        return;
    }

    int16_t x_raw, y_raw, z_raw;
    x_raw = (int16_t)((data_mag[1] << 8) | data_mag[0]);
    y_raw = (int16_t)((data_mag[3] << 8) | data_mag[2]);
    z_raw = (int16_t)((data_mag[5] << 8) | data_mag[4]);

    // debug
//    mprint("Raw: X=%d, Y=%d, Z=%d\n",
//           imu.x_raw,
//           imu.y_raw,
//           imu.z_raw);

    imu.mx = x_raw * 0.0122f;
    imu.my = y_raw * 0.0122f;
    imu.mz = z_raw * 0.0122f;


//    mprint("Mag_X = %.3f uT; ", imu.mx);
//    mprint("Mag_Y = %.3f uT; ", imu.my);
//    mprint("Mag_Z = %.3f uT \n", imu.mz);
}
