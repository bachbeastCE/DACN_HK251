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
uint8_t asa[3];
float mag_adj[3];
const uint16_t i2c_timeout = 100;
uint8_t chipid;
imu_t imu;
void I2C_Scan(I2C_HandleTypeDef *I2Cx) {
	Serial_Print("OK can call this function");
    for (uint8_t addr = 1; addr < 127; addr++) {
    	//Serial_Serial_Print(addr);
        if (HAL_I2C_IsDeviceReady(I2Cx, addr << 1, 1, 10) == HAL_OK) {
            Serial_Print("I2C device found at 0x%02X\r\n", addr);
        }
    }
}

void imu_init(I2C_HandleTypeDef *I2Cx){
	accel_gyro_Init(I2Cx);
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
	if (imu.pitch > 90.0f) imu.pitch = 90.0f;
	if (imu.pitch < -90.0f) imu.pitch = -90.0f;
//	while (imu.roll > 180.0f) imu.roll -= 360.0f;
//	while (imu.roll < -180.0f) imu.roll += 360.0f;
	float Mx = imu.mx * cosf(pitch_rad) + imu.mz * sinf(pitch_rad);
	float My = imu.mx * sinf(roll_rad) * sinf(pitch_rad) + imu.my * cosf(roll_rad) - imu.mz * sinf(roll_rad) * cosf(pitch_rad);

	imu.yaw = atan2f(My, Mx) * 180.0f / PI;
    if (imu.yaw < 0) imu.yaw += 360.0;
    Serial_Print("mea_pitch = %.3f degree; ", imu.pitch);
    Serial_Print("mea_yaw = %.3f degree; ", imu.yaw);
    Serial_Print("mea_roll = %.3f degree; \n", imu.roll);
//    Serial_Print("#######################################\n");
}

void write (uint8_t reg, uint8_t value, I2C_HandleTypeDef *I2Cx, uint8_t devaddress)
{
	uint8_t data[2];
	data[0] = reg;
	data[1] = value;
	HAL_I2C_Master_Transmit (I2Cx, devaddress, data, 2, i2c_timeout);
}

void accel_gyro_Init (I2C_HandleTypeDef *I2Cx) {
    // check device ID WHO_AM_I
	HAL_I2C_Mem_Read(I2Cx, MPU_I2C_ADDR, MPU9250_WHO_AM_I, 1, &chipid, 1, i2c_timeout);
	//Serial_Serial_Print("Chip id == %d", chipid);
    if (chipid == 0x71) //0x71
    {
    	write(MPU9250_PWR_MGMT_1, 0x80, I2Cx, MPU_I2C_ADDR); //reset sensor
    	HAL_Delay(100);
    	write(MPU9250_PWR_MGMT_1, CLOCK_PLL_GYRO_X, I2Cx, MPU_I2C_ADDR);
    	HAL_Delay(10);
    	write(MPU9250_PWR_MGMT_2, 0x00, I2Cx, MPU_I2C_ADDR); //0x00 = measure gyro and acce mode
    	write(MPU9250_CONFIG, GYRO_DLPF_41HZ, I2Cx, MPU_I2C_ADDR); //Lowpass filter 41Hz delay 4.9ms => internal frequency = 1kHz
    	write(MPU9250_SMPLRT_DIV, 0x03, I2Cx, MPU_I2C_ADDR); //sample = (internal frequency)/(1 + SMPLRT_DIV) = 1000/(1 + 3) = 250Hz
    	write(MPU9250_GYRO_CONFIG, GYRO_RANGE_500DPS, I2Cx, MPU_I2C_ADDR);
    	write(MPU9250_ACCEL_CONFIG, ACCE_RANGE_8G, I2Cx, MPU_I2C_ADDR);
    	write(MPU9250_ACCEL_CONFIG_2, ACCE_DLPF_41HZ, I2Cx, MPU_I2C_ADDR);
    	write(INT_PIN_CFG, 0x02, I2Cx, MPU_I2C_ADDR);
    	Serial_Print("Acce & Gyro wake up \n");
    }
}

void mag_init (I2C_HandleTypeDef *I2Cx) {
    // check device ID WHO_AM_I
	HAL_I2C_Mem_Read(I2Cx, AK8963_I2C_ADDR, AK8963_WIA, 1, &chipid, 1, i2c_timeout);
	Serial_Print("chipid = %d\n", chipid);
    if (chipid == 0x48)
    {
    	write(AK8963_CNTL2, 0x01, I2Cx, AK8963_I2C_ADDR); //reset all bits
    	HAL_Delay(20);
    	write(AK8963_CNTL1, 0x00, I2Cx, AK8963_I2C_ADDR);  // power down
    	HAL_Delay(10);
    	write(AK8963_CNTL1, 0x0F, I2Cx, AK8963_I2C_ADDR); //access fuse rom
    	HAL_Delay(10);
    	HAL_I2C_Mem_Read(I2Cx, AK8963_I2C_ADDR, AK8963_ASAX, 1, asa, 3, i2c_timeout);
    	for (int i = 0; i < 3; i++)
			{
			   mag_adj[i] = ((float)(asa[i] - 128) / 256.0f) + 1.0f;
			}
    	Serial_Print("ASA: %d %d %d\r\n", asa[0], asa[1], asa[2]);
    	Serial_Print("Adj: %.3f %.3f %.3f\r\n", mag_adj[0], mag_adj[1], mag_adj[2]);
    	write(AK8963_CNTL1, 0x00, I2Cx, AK8963_I2C_ADDR);  // power down
    	HAL_Delay(10);
    	write(AK8963_CNTL1, 0x16, I2Cx, AK8963_I2C_ADDR);  // continous mode
    	HAL_Delay(10);

    	Serial_Print("MAG wake up \n");
    }
}


void accel_read(I2C_HandleTypeDef *I2Cx)
{
    if (HAL_I2C_Mem_Read(I2Cx, MPU_I2C_ADDR, MPU9250_ACCEL_XOUT_H, 1, data_acce, 6, i2c_timeout) != HAL_OK) {
        Serial_Print("I2C read error!\r\n");
        return;
    }

    int16_t x_raw = (int16_t)((data_acce[0] << 8) | data_acce[1]);
    int16_t y_raw = (int16_t)((data_acce[2] << 8) | data_acce[3]);
    int16_t z_raw = (int16_t)((data_acce[4] << 8) | data_acce[5]);

    imu.ax = ((float)x_raw - imu.aoffsetx) / 4096.0f;
    imu.ay = ((float)y_raw - imu.aoffsety) / 4096.0f;
    imu.az = 1 + ((float)z_raw - imu.aoffsetz) / 4096.0f;
    Serial_Print("Acce_X = %.3f g; ", imu.ax);
    Serial_Print("Acce_Y = %.3f g; ", imu.ay);
    Serial_Print("Acce_Z = %.3f g \n", imu.az);
}

void gyro_read(I2C_HandleTypeDef *I2Cx)
{
	if (HAL_I2C_Mem_Read(I2Cx, MPU_I2C_ADDR, MPU9250_GYRO_XOUT_H, 1, data_gyro, 6, i2c_timeout) != HAL_OK) {
	   Serial_Print("I2C read error!\r\n");
	   return;
	}

    int16_t x_raw, y_raw, z_raw;

    x_raw = (int16_t)((data_gyro[0] << 8) | data_gyro[1]);
    y_raw = (int16_t)((data_gyro[2] << 8) | data_gyro[3]);
    z_raw = (int16_t)((data_gyro[4] << 8) | data_gyro[5]);

    imu.gx = ((float)x_raw) / 65.5f;
    imu.gy = ((float)y_raw) / 65.5f;
    imu.gz = ((float)z_raw) / 65.5f;

    Serial_Print("Gyro_X = %.3f deg/s; ", imu.gx);
    Serial_Print("Gyro_Y = %.3f deg/s; ", imu.gy);
    Serial_Print("Gyro_Z = %.3f deg/s \n", imu.gz);
}

void mag_read(I2C_HandleTypeDef *I2Cx)
{

    if (HAL_I2C_Mem_Read(I2Cx, AK8963_I2C_ADDR, AK8963_HXL, 1, data_mag, 7, i2c_timeout) != HAL_OK) {
        Serial_Print("I2C read error!\r\n");
        return;
    }
    int16_t x_raw, y_raw, z_raw;

    x_raw = (int16_t)((data_mag[1] << 8) | data_mag[0]);
    y_raw = (int16_t)((data_mag[3] << 8) | data_mag[2]);
    z_raw = (int16_t)((data_mag[5] << 8) | data_mag[4]);

    imu.moffsetx = 0;
    imu.moffsety = 0;
    imu.moffsetz = 0;
    imu.mx = ((float)x_raw * mag_adj[0] * 0.15f - imu.moffsetx);
    imu.my = ((float)y_raw * mag_adj[1] * 0.15f - imu.moffsety);
    imu.mz = ((float)z_raw * mag_adj[2] * 0.15f - imu.moffsetz);

    Serial_Print("Mag_X = %.3f uT; ", imu.mx);
    Serial_Print("Mag_Y = %.3f uT; ", imu.my);
    Serial_Print("Mag_Z = %.3f uT \n", imu.mz);
}

void calibrate(I2C_HandleTypeDef *I2Cx) {
    // ===== Gyro + Accel calibration =====
    int64_t sum_gx = 0, sum_gy = 0, sum_gz = 0;
    int64_t sum_ax = 0, sum_ay = 0, sum_az = 0;


    uint32_t CALIB_SAMPLES = 1000;
    int16_t gx_sample[CALIB_SAMPLES], gy_sample[CALIB_SAMPLES], gz_sample[CALIB_SAMPLES];
    int16_t ax_raw, ay_raw, az_raw;
    Serial_Print("=== Keep IMU still... Calibrating gyro & accel... ===\r\n");
    HAL_Delay(1000);
    int16_t gx_raw, gy_raw, gz_raw;
    for (uint32_t i = 0; i < CALIB_SAMPLES; i++) {
        // --- Read Gyro ---
        if (HAL_I2C_Mem_Read(I2Cx, MPU_I2C_ADDR, MPU9250_GYRO_XOUT_H, 1, data_gyro, 6, 100) != HAL_OK)
            return;


        gx_raw = (int16_t)((data_gyro[0] << 8) | data_gyro[1]);
        gy_raw = (int16_t)((data_gyro[2] << 8) | data_gyro[3]);
        gz_raw = (int16_t)((data_gyro[4] << 8) | data_gyro[5]);
        gx_sample[i] = gx_raw;
        gy_sample[i] = gy_raw;
        gz_sample[i] = gz_raw;
        sum_gx += gx_raw;
        sum_gy += gy_raw;
        sum_gz += gz_raw;

        // --- Read Accel ---
        if (HAL_I2C_Mem_Read(I2Cx, MPU_I2C_ADDR, MPU9250_ACCEL_XOUT_H, 1, data_acce, 6, 100) != HAL_OK)
            return;

        ax_raw = (int16_t)((data_acce[0] << 8) | data_acce[1]);
        ay_raw = (int16_t)((data_acce[2] << 8) | data_acce[3]);
        az_raw = (int16_t)((data_acce[4] << 8) | data_acce[5]);
        sum_ax += ax_raw;
        sum_ay += ay_raw;
        sum_az += az_raw;

        HAL_Delay(5);
    }

    imu.goffsetx = (float)sum_gx / CALIB_SAMPLES;
    imu.goffsety = (float)sum_gy / CALIB_SAMPLES;
    imu.goffsetz = (float)sum_gz / CALIB_SAMPLES;
    imu.aoffsetx = (float)sum_ax / CALIB_SAMPLES;
    imu.aoffsety = (float)sum_ay / CALIB_SAMPLES;
    imu.aoffsetz = (float)sum_az / CALIB_SAMPLES;
    sum_gx = 0;
    sum_gy = 0;
    sum_gz = 0;
    float var_gx = 0, var_gy = 0, var_gz = 0;
    for (uint32_t i = 0; i < CALIB_SAMPLES; i++) {
        var_gx = var_gx + (gx_sample[i] - imu.goffsetx) * (gx_sample[i] - imu.goffsetx);
        var_gy = var_gy + (gy_sample[i] - imu.goffsety) * (gy_sample[i] - imu.goffsety);
        var_gz = var_gz + (gz_sample[i] - imu.goffsetz) * (gz_sample[i] - imu.goffsetz);
    }

    var_gx /= (CALIB_SAMPLES - 1);
    var_gy /= (CALIB_SAMPLES - 1);
    var_gz /= (CALIB_SAMPLES - 1);

    imu.gyrox_noise = sqrtf(var_gx);
    imu.gyroy_noise = sqrtf(var_gy);
    imu.gyroz_noise = sqrtf(var_gz);


    Serial_Print("Gyro & Accel calibration done!\r\n");
    Serial_Print("Ax_off=%.3f, Ay_off=%.3f, Az_off=%.3f\r\n, Gx_off=%.3f, Gy_off%=%.3f, Gz_off=%.3f\r\n", imu.aoffsetx / 4096.0f, imu.aoffsety / 4096.0f, imu.aoffsetz / 4096.0f, imu.goffsetx/65.5f, imu.goffsety/65.5f, imu.goffsetz/65.5f);
    Serial_Print("Gyro Noise (1σ): Nx=%.6f, Ny=%.6f, Nz=%.6f\r\n",
                 imu.gyrox_noise/65.5f, imu.gyroy_noise/65.5f, imu.gyroz_noise/65.5f);
    // ===== Magnetometer calibration =====
    Serial_Print("=== Rotate IMU slowly in all directions (figure-8) ===\r\n");
    HAL_Delay(2000);

    float mx_min = 1e9, my_min = 1e9, mz_min = 1e9;
    float mx_max = -1e9, my_max = -1e9, mz_max = -1e9;
    int16_t mx_raw, my_raw, mz_raw;

    uint32_t MAG_SAMPLES = 1000;
    for (uint32_t i = 0; i < MAG_SAMPLES; i++) {
        if (HAL_I2C_Mem_Read(I2Cx, AK8963_I2C_ADDR, AK8963_HXL, 1, data_mag, 7, 100) != HAL_OK)
            return;

        mx_raw = (int16_t)((data_mag[1] << 8) | data_mag[0]);
        my_raw = (int16_t)((data_mag[3] << 8) | data_mag[2]);
        mz_raw = (int16_t)((data_mag[5] << 8) | data_mag[4]);

        float mx = (float)mx_raw * mag_adj[0];
        float my = (float)my_raw * mag_adj[1];
        float mz = (float)mz_raw * mag_adj[2];

        if (mx < mx_min) mx_min = mx;
        if (mx > mx_max) mx_max = mx;
        if (my < my_min) my_min = my;
        if (my > my_max) my_max = my;
        if (mz < mz_min) mz_min = mz;
        if (mz > mz_max) mz_max = mz;

        HAL_Delay(10);
    }

    imu.moffsetx = ((mx_max + mx_min)/2.0f) * 0.15f;
    imu.moffsety = ((my_max + my_min)/2.0f) * 0.15f;
    imu.moffsetz = ((mz_max + mz_min)/2.0f) * 0.15f;
    float mx_scale = (mx_max - mx_min) / 2.0f;
    float my_scale = (my_max - my_min) / 2.0f;
    float mz_scale = (mz_max - mz_min) / 2.0f;
    float avg_scale = (mx_scale + my_scale + mz_scale) / 3.0f;
    imu.mscalex = avg_scale / mx_scale;
    imu.mscaley = avg_scale / my_scale;
    imu.mscalez = avg_scale / mz_scale;

    Serial_Print("Magnetometer calibration done!\r\n");
    Serial_Print("Offsets:\r\n");
    Serial_Print(" Mx_off=%.2f, My_off=%.2f, Mz_off=%.2f\r\n", imu.moffsetx, imu.moffsety, imu.moffsetz);
    Serial_Print("=============================================\r\n");
}

