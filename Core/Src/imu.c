/*
 * imu.c
 *
 *  Created on: Sep 17, 2025
 *      Author: Admin
 */


#include "imu.h"

uint8_t data_acce[6];
uint8_t data_gyro[6];
uint8_t data_mag[6];
const uint16_t i2c_timeout = 100;
uint8_t chipid;
imu_t imu;
void I2C_Scan(I2C_HandleTypeDef *I2Cx) {
//	mprint("HELLO WORD");
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
	calibrate(I2Cx);
}

void imu_read(I2C_HandleTypeDef *I2Cx){
	accel_read(I2Cx);
	gyro_read(I2Cx);
	mag_read(I2Cx);
	float pitch_rad = atan2f(imu.ax, sqrtf(imu.ay * imu.ay + imu.az * imu.az));
	float roll_rad  = atan2f(imu.ay, sqrtf(imu.ax * imu.ax + imu.az * imu.az));

	imu.pitch = pitch_rad * 180.0f / PI;
	imu.roll  = roll_rad  * 180.0f / PI;

	float Mx = imu.mx * cosf(pitch_rad) + imu.mz * sinf(pitch_rad);
	float My = imu.mx * sinf(roll_rad) * sinf(pitch_rad) + imu.my * cosf(roll_rad) - imu.mz * sinf(roll_rad) * cosf(pitch_rad);

	imu.yaw = atan2f(My, Mx) * 180.0f / PI;
    if (imu.yaw < 0) imu.yaw += 360.0;
    mprint("mea_pitch = %.3f degree; ", imu.pitch);
    mprint("mea_yaw = %.3f degree; ", imu.yaw);
    mprint("mea_roll = %.3f degree; \n", imu.roll);
//    mprint("#######################################\n");
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

    imu.ax = (x_raw - imu.aoffsetx) * 0.004f;
    imu.ay = (y_raw - imu.aoffsety) * 0.004f;
    imu.az = 1 + (z_raw - imu.aoffsetz) * 0.004f;

//    mprint("Acce_X = %.3f g; ", imu.ax);
//    mprint("Acce_Y = %.3f g; ", imu.ay);
//    mprint("Acce_Z = %.3f g \n", imu.az);
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

    imu.gx = (x_raw - imu.goffsetx) / 14.375f;
    imu.gy = (y_raw - imu.goffsety) / 14.375f;
    imu.gz = (z_raw - imu.goffsetz) / 14.375f;

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

    imu.mx = (x_raw - imu.moffsetx) / 3000.0f;
    imu.my = (y_raw - imu.moffsety) / 3000.0f;
    imu.mz = (z_raw - imu.moffsetz) / 3000.0f;


//    mprint("Mag_X = %.3f Gauss; ", imu.mx);
//    mprint("Mag_Y = %.3f Gauss; ", imu.my);
//    mprint("Mag_Z = %.3f Gauss \n", imu.mz);
}

void calibrate(I2C_HandleTypeDef *I2Cx) {
    //Gyro variables
    int64_t sum_gx = 0, sum_gy = 0, sum_gz = 0;
    int16_t gx_raw, gy_raw, gz_raw;
    //Accel variables
    int64_t sum_ax = 0, sum_ay = 0, sum_az = 0;
    int16_t ax_raw, ay_raw, az_raw;
    //Mag variables
    int64_t sum_mx = 0, sum_my = 0, sum_mz = 0;
    int16_t mx_raw, my_raw, mz_raw;

    uint32_t CALIB_SAMPLES = 1000;

    for (uint32_t i = 0; i < CALIB_SAMPLES; i++) {
        //Read Gyro
        if (HAL_I2C_Mem_Read(I2Cx, GYRO_ADDR, GYRO_DATAX0_REG, 1, data_gyro, 6, 100) != HAL_OK) {
            mprint("I2C read error!\r\n");
            return;
        }
        gx_raw = (int16_t)((data_gyro[0] << 8) | data_gyro[1]);
        gy_raw = (int16_t)((data_gyro[2] << 8) | data_gyro[3]);
        gz_raw = (int16_t)((data_gyro[4] << 8) | data_gyro[5]);
        sum_gx += gx_raw;
        sum_gy += gy_raw;
        sum_gz += gz_raw;
        //Read Accel
        if (HAL_I2C_Mem_Read(I2Cx, ACCE_ADDR, ACCE_DATAX0_REG, 1, data_acce, 6, 100) != HAL_OK) {
            mprint("I2C read error!\r\n");
            return;
        }
        ax_raw = (int16_t)((data_acce[1] << 8) | data_acce[0]);
        ay_raw = (int16_t)((data_acce[3] << 8) | data_acce[2]);
        az_raw = (int16_t)((data_acce[5] << 8) | data_acce[4]);
        sum_ax += ax_raw;
        sum_ay += ay_raw;
        sum_az += az_raw;

        //Read Magnetometer
        if (HAL_I2C_Mem_Read(I2Cx, MAG_ADDR, MAG_DATAX0_REG, 1, data_mag, 6, 100) != HAL_OK) {
            mprint("I2C read error!\r\n");
            return;
        }
        mx_raw = (int16_t)((data_mag[1] << 8) | data_mag[0]);
        my_raw = (int16_t)((data_mag[3] << 8) | data_mag[2]);
        mz_raw = (int16_t)((data_mag[5] << 8) | data_mag[4]);
        sum_mx += mx_raw;
        sum_my += my_raw;
        sum_mz += mz_raw;
        HAL_Delay(2); // ~500Hz
    }

    // ----- Compute offsets / biases -----
    imu.aoffsetx = (float)sum_ax / CALIB_SAMPLES;
    imu.aoffsety = (float)sum_ay / CALIB_SAMPLES;
    imu.aoffsetz = (float)sum_az / CALIB_SAMPLES;

    imu.goffsetx = (float)sum_gx / CALIB_SAMPLES;
    imu.goffsety = (float)sum_gy / CALIB_SAMPLES;
    imu.goffsetz = (float)sum_gz / CALIB_SAMPLES;

    imu.moffsetx = (float)sum_mx / CALIB_SAMPLES;
    imu.moffsety = (float)sum_my / CALIB_SAMPLES;
    imu.moffsetz = (float)sum_mz / CALIB_SAMPLES;

    mprint("Calibration done!\r\n");
    mprint("Aoffsetx: %.3f g; ", imu.aoffsetx);
    mprint("Aoffsety: %.3f g; ", imu.aoffsety);
    mprint("Aoffsetz: %.3f g;\n", imu.aoffsetz);
    mprint("Goffsetx = %.3f degree/s; ", imu.goffsetx);
    mprint("Goffsety = %.3f degree/s; ", imu.goffsety);
    mprint("Goffsetz = %.3f degree/s;\n", imu.goffsetz);
    mprint("Moffsetx = %.3f Gauss; ", imu.moffsetx);
    mprint("Moffsety = %.3f Gauss; ", imu.moffsety);
    mprint("Moffsetz = %.3f Gauss;\n", imu.moffsetz);
    mprint("=============================================\n");
}

