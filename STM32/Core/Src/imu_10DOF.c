///*
// * imu_10DOF.c
// *
// *  Created on: Jan 28, 2026
// *      Author: Admin
// */
#include "imu_10DOF.h"
//
//uint8_t data_acce[6];
//uint8_t data_gyro[6];
//uint8_t data_mag[6];
//
//volatile IMU_PipelineState_t imuState = IMU_IDLE;
//const uint16_t i2c_timeout = 100;
//uint8_t chipid;
imu_t imu;
BMP280 bmp;
//void I2C_Scan(I2C_HandleTypeDef *I2Cx) {
//    for (uint8_t addr = 1; addr < 127; addr++) {
//        if (HAL_I2C_IsDeviceReady(I2Cx, addr << 1, 1, 10) == HAL_OK) {
//            Serial_Print("I2C device found at 0x%02X\r\n", addr);
//        }
//    }
//}
//
//void imu_init(I2C_HandleTypeDef *I2Cx){
//	accel_init(I2Cx);
//	gyro_init(I2Cx);
//	mag_init(I2Cx);
//	baro_init(I2Cx);
//	calibrate(I2Cx);
//}
//
//void imu_read(I2C_HandleTypeDef *I2Cx){
//	accel_read(I2Cx);
//	gyro_read(I2Cx);
//	mag_read(I2Cx);
//	baro_read(I2Cx);
//	float pitch_rad = atan2f(-imu.ax, sqrtf(imu.ay * imu.ay + imu.az * imu.az));
//	float roll_rad  = atan2f(imu.ay, sqrtf(imu.ax * imu.ax + imu.az * imu.az));
////	float roll_rad  = atan2f(imu.ay, imu.ax);
//
//	imu.pitch = pitch_rad * 180.0f / PI;
//	imu.roll  = roll_rad  * 180.0f / PI;
//	if (imu.pitch > 90.0f) imu.pitch = 90.0f;
//	if (imu.pitch < -90.0f) imu.pitch = -90.0f;
//	while (imu.roll > 180.0f) imu.roll -= 360.0f;
//	while (imu.roll < -180.0f) imu.roll += 360.0f;
//	float Mx = imu.mx * cosf(pitch_rad) + imu.my * sinf(roll_rad) * sinf(pitch_rad) + imu.mz * cosf(roll_rad) * sinf(pitch_rad);
//
//	float My = imu.my * cosf(roll_rad) - imu.mz * sinf(roll_rad);
//
//	imu.yaw = atan2f(My, Mx) * 180.0f / PI;
////	imu.yaw = atan2f(imu.my, imu.mx) * 180.0f / PI;
////	while (imu.yaw > 180.0f) imu.yaw -= 360.0f;
////	while (imu.yaw < -180.0f) imu.yaw += 360.0f;
//#if IMU_ENABLE_SERIAL_LOG
//    Serial_Print("mea_pitch = %.3f degree; ", imu.pitch);
//    Serial_Print("mea_yaw = %.3f degree; ", imu.yaw);
//    Serial_Print("mea_roll = %.3f degree; \n", imu.roll);
//    Serial_Print("mea_temperature = %.3f degree C; \n", imu.temperature);
//    Serial_Print("mea_altitude = %.3f m; \n", imu.altitude);
//    Serial_Print("mea_pressure = %.3f Pa; \n", imu.pressure);
//#endif
////    Serial_Print("#######################################\n");
//}
//
//void imu_compute_attitude(){
////	accel_read(I2Cx);
////	gyro_read(I2Cx);
////	mag_read(I2Cx);
////	baro_read(I2Cx);
//	float pitch_rad = atan2f(-imu.ax, sqrtf(imu.ay * imu.ay + imu.az * imu.az));
//	float roll_rad  = atan2f(imu.ay, sqrtf(imu.ax * imu.ax + imu.az * imu.az));
//
//	imu.pitch = pitch_rad * 180.0f / PI;
//	imu.roll  = roll_rad  * 180.0f / PI;
//	if (imu.pitch > 90.0f) imu.pitch = 90.0f;
//	if (imu.pitch < -90.0f) imu.pitch = -90.0f;
//	while (imu.roll > 180.0f) imu.roll -= 360.0f;
//	while (imu.roll < -180.0f) imu.roll += 360.0f;
//	float Mx = imu.mx * cosf(pitch_rad) + imu.my * sinf(roll_rad) * sinf(pitch_rad) + imu.mz * cosf(roll_rad) * sinf(pitch_rad);
//
//	float My = imu.my * cosf(roll_rad) - imu.mz * sinf(roll_rad);
//
//	imu.yaw = atan2f(My, Mx) * 180.0f / PI;
////#if IMU_ENABLE_SERIAL_LOG
//    Serial_Print("mea_pitch = %.3f degree; ", imu.pitch);
//    Serial_Print("mea_yaw = %.3f degree; ", imu.yaw);
//    Serial_Print("mea_roll = %.3f degree; \n", imu.roll);
////    Serial_Print("mea_temperature = %.3f degree C; ", imu.temperature);
////    Serial_Print("mea_altitude = %.3f m; ", imu.altitude);
////    Serial_Print("mea_pressure = %.3f Pa; \n", imu.pressure);
////#endif
////    Serial_Print("#######################################\n");
//}
//
void write1 (uint8_t reg, uint8_t value, I2C_HandleTypeDef *I2Cx, uint8_t devaddress)
{
	uint8_t data[2];
	data[0] = reg;
	data[1] = value;
	HAL_I2C_Master_Transmit (I2Cx, devaddress, data, 2, 100);
}
//
//void accel_init (I2C_HandleTypeDef *I2Cx) {
//    // check device ID WHO_AM_I
//	HAL_I2C_Mem_Read(I2Cx, ACCE_ADDR, ACCE_ID, 1, &chipid, 1, i2c_timeout);
//    if (chipid == 229)
//    {
//    	write(ACCE_POWER_CTL_REG, 0x00, I2Cx, ACCE_ADDR);  // reset all bits
//    	write(ACCE_BW_RATE_REG, 0x0A, I2Cx, ACCE_ADDR); // 100 Hz default
//    	write(ACCE_DATA_FORMAT_REG, 0x09, I2Cx, ACCE_ADDR); // FULL_RES, +-4g
//    	write(ACCE_POWER_CTL_REG, 0x08, I2Cx, ACCE_ADDR); // measure
//    	Serial_Print("ACCE wake up");
//    }
//}
//
//void gyro_init(I2C_HandleTypeDef *I2Cx)
//{
//    HAL_I2C_Mem_Read(I2Cx, GYRO_ADDR, GYRO_ID, 1, &chipid, 1, i2c_timeout);
//
//    if (chipid == 0x68)
//    {
//        //Reset
//        write(GYRO_PWR_MANAG_REG, 0x80, I2Cx, GYRO_ADDR);
//        HAL_Delay(50);
//        //Wake up
//        write(GYRO_PWR_MANAG_REG, 0x03, I2Cx, GYRO_ADDR);
//        //FS_SEL = 2000 degree/s, DLPF = 42Hz (1kHz internal)
//        write(GYRO_DLPF_CFG_REG, 0x1A, I2Cx, GYRO_ADDR);
//        // 0x1A = FS_SEL=3, DLPF_CFG=2
//        // Sample rate = 1kHz / (4+1) = 200Hz
//        write(GYRO_SMPLRT_DIV_REG, 0x04, I2Cx, GYRO_ADDR);
//
//        Serial_Print("GYRO ready\r\n");
//    }
//}
//
//void mag_init (I2C_HandleTypeDef *I2Cx) {
//    // check device ID WHO_AM_I
//	HAL_I2C_Mem_Read(I2Cx, MAG_ADDR, MAG_ID, 1, &chipid, 1, i2c_timeout);
//	//Serial_Print("chipid = %d\n", chipid);
//    if (chipid == 130)
//    {
//    	write(MAG_CTRL_REG1, 0x80, I2Cx, MAG_ADDR);  // soft reset all bits
//    	HAL_Delay(10);
//    	write(MAG_CTRL_REG1, 0x00, I2Cx, MAG_ADDR);  // SET/RESET
//    	HAL_Delay(10);
//    	write(MAG_CTRL_REG2, 0x49, I2Cx, MAG_ADDR); //power_cntl measure and wake up
//    	Serial_Print("MAG wake up");
//    }
//}
//
void BMP280_get_calib_values(I2C_HandleTypeDef *I2Cx)
{
	uint8_t rx_buff[24];

    HAL_I2C_Mem_Read(I2Cx, BAR_ADDR, 0x88, 1, rx_buff, 24, 10000);

	bmp.digT1=(rx_buff[0])+(rx_buff[1]<<8);
	bmp.digT2=(rx_buff[2])+(rx_buff[3]<<8);
	bmp.digT3=(rx_buff[4])+(rx_buff[5]<<8);
	bmp.digP1=(rx_buff[6])+(rx_buff[7]<<8);
	bmp.digP2=(rx_buff[8])+(rx_buff[9]<<8);
	bmp.digP3=(rx_buff[10])+(rx_buff[11]<<8);
	bmp.digP4=(rx_buff[12])+(rx_buff[13]<<8);
	bmp.digP5=(rx_buff[14])+(rx_buff[15]<<8);
	bmp.digP6=(rx_buff[16])+(rx_buff[17]<<8);
	bmp.digP7=(rx_buff[18])+(rx_buff[19]<<8);
	bmp.digP8=(rx_buff[20])+(rx_buff[21]<<8);
	bmp.digP9=(rx_buff[22])+(rx_buff[23]<<8);
}

void baro_init (I2C_HandleTypeDef *I2Cx) {
    // check device ID WHO_AM_I
	uint8_t id;
	HAL_I2C_Mem_Read(I2Cx, BAR_ADDR, BAR_ID, 1, &id, 1, 100);
	//Serial_Print("chipid = %d\n", chipid);
    if (id == 0x58)
    {
    	write1(BAR_RST, 0xB6, I2Cx, BAR_ADDR);  // reset all bits
    	// Wait until finished copying over the NVP data.
    	uint8_t status;
    	do {
    	    HAL_I2C_Mem_Read(I2Cx, BAR_ADDR, BAR_STATUS, 1, &status, 1, 100);
    	} while (status & 0x01);
    	BMP280_get_calib_values(I2Cx);
        // Sleep
        write1(BAR_CTRL_MEAS, 0x00, I2Cx, BAR_ADDR);
        // Config: t_sb=125ms, filter=16
        write1(BAR_CONF, 0x50, I2Cx, BAR_ADDR);
        // Ctrl_meas: osrs_t=x16, osrs_p=x16, NORMAL
        write1(BAR_CTRL_MEAS, 0xB7, I2Cx, BAR_ADDR);
        Serial_Print("BARO wake up");
    }
}
//
//void mag_read_dma_start(I2C_HandleTypeDef *hi2c)
//{
//    static uint8_t reg = MAG_DATAX0_REG;
//
//    imuState = IMU_MAG_TX;
//
//    HAL_StatusTypeDef st = HAL_I2C_Master_Transmit_IT(hi2c, MAG_ADDR, &reg, 1);
//
//    if (st != HAL_OK)
//    {
//        Serial_Print("MAG TX FAIL ret=%d state=%d err=0x%lx\n",
//                     st, hi2c->State, hi2c->ErrorCode);
//    }
//}
//
//void acc_read_dma_start(I2C_HandleTypeDef *hi2c)
//{
//    static uint8_t reg = ACCE_DATAX0_REG;
//
//    imuState = IMU_ACC_TX;
//    Serial_Print("HELLO1\n");
//    Serial_Print("I2C state=%d lock=%d\n", hi2c->State, hi2c->Lock);
//    HAL_StatusTypeDef st = HAL_I2C_Master_Transmit_IT(hi2c, ACCE_ADDR, &reg, 1);
//    Serial_Print("HELLO\n");
//    if (st != HAL_OK)
//    {
//        Serial_Print("ACC TX FAIL ret=%d state=%d err=0x%lx\n",
//                     st, hi2c->State, hi2c->ErrorCode);
//    }
//}
//
//void gyro_read_dma_start(I2C_HandleTypeDef *I2Cx){
////	Serial_Print("OK gyro!\n");
//	HAL_StatusTypeDef st = HAL_I2C_Mem_Read_DMA(I2Cx, GYRO_ADDR, GYRO_DATAX0_REG, 1, data_gyro, 6);
//	if (st != HAL_OK)
//	{
//	    Serial_Print("I2C GYRO DMA START FAIL\n");
//	}
//}
//
////void baro_read_dma_start(I2C_HandleTypeDef *I2Cx){
////    uint8_t status;
////    uint8_t rx_buf[6];
////
////    do {
////    	HAL_I2C_Mem_Read(I2Cx, BAR_ADDR, BAR_STATUS, 1, &status, 1, i2c_timeout);
////    } while (status & 0x09);// bit3(measuring) | bit0(im_update)
////	HAL_StatusTypeDef st = HAL_I2C_Mem_Read_DMA(I2Cx, BAR_ADDR, 0xF7, 1, data_gyro, 6);
//////	Serial_Print("OK!\n");
////	if (st != HAL_OK)
////	{
////	    Serial_Print("I2C GYRO DMA START FAIL\n");
////	}
////}
void baro_read(I2C_HandleTypeDef *I2Cx)
{
    uint8_t status;
    uint8_t rx_buf[6];

    do {
    	HAL_I2C_Mem_Read(I2Cx, BAR_ADDR, BAR_STATUS, 1, &status, 1, 100);
    } while (status & 0x09);// bit3(measuring) | bit0(im_update)


    //read press + temp
    if (HAL_I2C_Mem_Read(I2Cx, BAR_ADDR, 0xF7, I2C_MEMADD_SIZE_8BIT, rx_buf, 6, 100) != HAL_OK)
    {
        Serial_Print("BMP280 read error\n");
        return;
    }

    //raw
    uint32_t adc_P = ((uint32_t)rx_buf[0] << 12) | ((uint32_t)rx_buf[1] << 4)  | (rx_buf[2] >> 4);

    uint32_t adc_T = ((uint32_t)rx_buf[3] << 12) | ((uint32_t)rx_buf[4] << 4)  | (rx_buf[5] >> 4);

    //Temperature compensation
    int32_t var1, var2, t_fine;
    var1 = ((((int32_t)adc_T >> 3) - ((int32_t)bmp.digT1 << 1)) *
            ((int32_t)bmp.digT2)) >> 11;
    var2 = (((((int32_t)adc_T >> 4) - ((int32_t)bmp.digT1)) *
              (((int32_t)adc_T >> 4) - ((int32_t)bmp.digT1))) >> 12) *
            ((int32_t)bmp.digT3) >> 14;

    t_fine = var1 + var2;
    imu.temperature = ((t_fine * 5 + 128) >> 8)* 0.01f;

    //Pressure compensation
    int64_t varp1, varp2, p;
    varp1 = ((int64_t)t_fine) - 128000;

    varp2 = varp1 * varp1 * (int64_t)bmp.digP6;
    varp2 = varp2 + ((varp1 * (int64_t)bmp.digP5) << 17);
    varp2 = varp2 + (((int64_t)bmp.digP4) << 35);

    varp1 = ((varp1 * varp1 * (int64_t)bmp.digP3) >> 8)
         + ((varp1 * (int64_t)bmp.digP2) << 12);

    varp1 = (((((int64_t)1) << 47) + varp1) * (int64_t)bmp.digP1) >> 33;

    if (varp1 == 0) {
        imu.pressure = 0;
        return;
    }

    p = 1048576 - adc_P;
    p = (((p << 31) - varp2) * 3125) / varp1;

    varp1 = ((int64_t)bmp.digP9 * (p >> 13) * (p >> 13)) >> 25;
    varp2 = ((int64_t)bmp.digP8 * p) >> 19;

    p = ((p + varp1 + varp2) >> 8) + (((int64_t)bmp.digP7) << 4);

    //Pa
    imu.pressure = (float)p / 256.0f;

    //Altitude
    imu.altitude = 44330.0f *
               (1.0f - powf((float)(imu.pressure / 101325.0), 1.0f / 5.255f));
}

//void accel_read(I2C_HandleTypeDef *I2Cx)
//{
////    if (HAL_I2C_Mem_Read(I2Cx, ACCE_ADDR, ACCE_DATAX0_REG, 1, data_acce, 6, i2c_timeout) != HAL_OK) {
////#if IMU_ENABLE_SERIAL_LOG
////        Serial_Print("I2C read error!\r\n");
////#endif
////        return;
////    }
//    int16_t x_raw, y_raw, z_raw;
//    x_raw = (int16_t)((data_acce[1] << 8) | data_acce[0]);
//    y_raw = (int16_t)((data_acce[3] << 8) | data_acce[2]);
//    z_raw = (int16_t)((data_acce[5] << 8) | data_acce[4]);
//
//    imu.ax = (x_raw - imu.aoffsetx) * 0.004f;
//    imu.ay = (y_raw - imu.aoffsety) * 0.004f;
//    imu.az = 1 + (z_raw - imu.aoffsetz) * 0.004f;
//
//#if IMU_ENABLE_SERIAL_LOG
//    Serial_Print("Acce_X = %.3f g; ", imu.ax);
//    Serial_Print("Acce_Y = %.3f g; ", imu.ay);
//    Serial_Print("Acce_Z = %.3f g \n", imu.az);
//#endif
//}
//
//void gyro_read(I2C_HandleTypeDef *I2Cx)
//{
////    if (HAL_I2C_Mem_Read(I2Cx, GYRO_ADDR, GYRO_DATAX0_REG, 1, data_gyro, 6, i2c_timeout) != HAL_OK) {
////#if IMU_ENABLE_SERIAL_LOG
////	   Serial_Print("I2C read error!\r\n");
////#endif
////        return;
////    }
//
//    int16_t x_raw, y_raw, z_raw;
//    x_raw = (int16_t)((data_gyro[0] << 8) | data_gyro[1]);
//    y_raw = (int16_t)((data_gyro[2] << 8) | data_gyro[3]);
//    z_raw = (int16_t)((data_gyro[4] << 8) | data_gyro[5]);
//
//    imu.gx = (x_raw - imu.goffsetx) / 14.375f;
//    imu.gy = (y_raw - imu.goffsety) / 14.375f;
//    imu.gz = (z_raw - imu.goffsetz) / 14.375f;
//
//#if IMU_ENABLE_SERIAL_LOG
//    Serial_Print("Gyro_X = %.3f deg/s; ", imu.gx);
//    Serial_Print("Gyro_Y = %.3f deg/s; ", imu.gy);
//    Serial_Print("Gyro_Z = %.3f deg/s \n", imu.gz);
//#endif
//}
//
//void mag_read(I2C_HandleTypeDef *I2Cx)
//{
////    if (HAL_I2C_Mem_Read(I2Cx, MAG_ADDR, MAG_DATAX0_REG, 1, data_mag, 6, i2c_timeout) != HAL_OK) {
////#if IMU_ENABLE_SERIAL_LOG
////        Serial_Print("I2C read error!\r\n");
////#endif
////        return;
////    }
//
//    int16_t x_raw, y_raw, z_raw;
//    x_raw = (int16_t)((data_mag[1] << 8) | data_mag[0]);
//    y_raw = (int16_t)((data_mag[3] << 8) | data_mag[2]);
//    z_raw = (int16_t)((data_mag[5] << 8) | data_mag[4]);
//
//    // debug
////    Serial_Print("Raw: X=%d, Y=%d, Z=%d\n",
////           imu.x_raw,
////           imu.y_raw,
////           imu.z_raw);
//
//    imu.mx = 100 * (x_raw - imu.moffsetx) / 3000.0f;
//    imu.my = 100 * (y_raw - imu.moffsety) / 3000.0f;
//    imu.mz = 100 * (z_raw - imu.moffsetz) / 3000.0f;
//
//
//#if IMU_ENABLE_SERIAL_LOG
//    Serial_Print("Mag_X = %.3f uT; ", imu.mx);
//    Serial_Print("Mag_Y = %.3f uT; ", imu.my);
//    Serial_Print("Mag_Z = %.3f uT; \n", imu.mz);
//    Serial_Print("#\n");
//#endif
//}
//
//void calibrate(I2C_HandleTypeDef *I2Cx) {
//    // ===== Gyro + Accel calibration =====
//    int64_t sum_gx = 0, sum_gy = 0, sum_gz = 0;
//    int64_t sum_ax = 0, sum_ay = 0, sum_az = 0;
//
//
//    uint32_t CALIB_SAMPLES = 1000;
//    int16_t gx_sample[CALIB_SAMPLES], gy_sample[CALIB_SAMPLES], gz_sample[CALIB_SAMPLES];
//    int16_t ax_sample[CALIB_SAMPLES];
//    int16_t ay_sample[CALIB_SAMPLES];
//    int16_t az_sample[CALIB_SAMPLES];
//    int16_t ax_raw, ay_raw, az_raw;
//    Serial_Print("=== Keep IMU still... Calibrating gyro & accel... ===\r\n");
//    HAL_Delay(100);
//    int16_t gx_raw, gy_raw, gz_raw;
//    for (uint32_t i = 0; i < CALIB_SAMPLES; i++) {
//        // --- Read Gyro ---
//        if (HAL_I2C_Mem_Read(I2Cx, GYRO_ADDR, GYRO_DATAX0_REG, 1, data_gyro, 6, i2c_timeout) != HAL_OK)
//            return;
//
//
//        gx_raw = (int16_t)((data_gyro[0] << 8) | data_gyro[1]);
//        gy_raw = (int16_t)((data_gyro[2] << 8) | data_gyro[3]);
//        gz_raw = (int16_t)((data_gyro[4] << 8) | data_gyro[5]);
//        gx_sample[i] = gx_raw;
//        gy_sample[i] = gy_raw;
//        gz_sample[i] = gz_raw;
//        sum_gx += gx_raw;
//        sum_gy += gy_raw;
//        sum_gz += gz_raw;
//
//        // --- Read Accel ---
//        if (HAL_I2C_Mem_Read(I2Cx, ACCE_ADDR, ACCE_DATAX0_REG, 1, data_acce, 6, i2c_timeout) != HAL_OK)
//            return;
//
//        ax_raw = (int16_t)((data_acce[1] << 8) | data_acce[0]);
//        ay_raw = (int16_t)((data_acce[3] << 8) | data_acce[2]);
//        az_raw = (int16_t)((data_acce[5] << 8) | data_acce[4]);
//        ax_sample[i] = ax_raw;
//        ay_sample[i] = ay_raw;
//        az_sample[i] = az_raw;
//        sum_ax += ax_raw;
//        sum_ay += ay_raw;
//        sum_az += az_raw;
//
//        HAL_Delay(2);
//    }
//
//    imu.goffsetx = (float)sum_gx / CALIB_SAMPLES;
//    imu.goffsety = (float)sum_gy / CALIB_SAMPLES;
//    imu.goffsetz = (float)sum_gz / CALIB_SAMPLES;
//    imu.aoffsetx = (float)sum_ax / CALIB_SAMPLES;
//    imu.aoffsety = (float)sum_ay / CALIB_SAMPLES;
//    imu.aoffsetz = (float)sum_az / CALIB_SAMPLES;
//    sum_gx = 0;
//    sum_gy = 0;
//    sum_gz = 0;
//    float var_gx = 0, var_gy = 0, var_gz = 0;
//    for (uint32_t i = 0; i < CALIB_SAMPLES; i++) {
//        var_gx = var_gx + (gx_sample[i] - imu.goffsetx) * (gx_sample[i] - imu.goffsetx);
//        var_gy = var_gy + (gy_sample[i] - imu.goffsety) * (gy_sample[i] - imu.goffsety);
//        var_gz = var_gz + (gz_sample[i] - imu.goffsetz) * (gz_sample[i] - imu.goffsetz);
//    }
//    float var_ax = 0, var_ay = 0, var_az = 0;
//
//    for (uint32_t i = 0; i < CALIB_SAMPLES; i++) {
//        var_ax += (ax_sample[i] - imu.aoffsetx) * (ax_sample[i] - imu.aoffsetx);
//        var_ay += (ay_sample[i] - imu.aoffsety) * (ay_sample[i] - imu.aoffsety);
//        var_az += (az_sample[i] - imu.aoffsetz) * (az_sample[i] - imu.aoffsetz);
//    }
//
//    var_ax /= (CALIB_SAMPLES - 1);
//    var_ay /= (CALIB_SAMPLES - 1);
//    var_az /= (CALIB_SAMPLES - 1);
//
//    var_gx /= (CALIB_SAMPLES - 1);
//    var_gy /= (CALIB_SAMPLES - 1);
//    var_gz /= (CALIB_SAMPLES - 1);
//
//    imu.gyrox_noise = sqrtf(var_gx);
//    imu.gyroy_noise = sqrtf(var_gy);
//    imu.gyroz_noise = sqrtf(var_gz);
//
////#if IMU_ENABLE_SERIAL_LOG
//    Serial_Print("Gyro & Accel calibration done!\r\n");
//    Serial_Print("Ax_off=%.3f, Ay_off=%.3f, Az_off=%.3f\r\n, Gx_off=%.3f, Gy_off%=%.3f, Gz_off=%.3f\r\n", imu.aoffsetx, imu.aoffsety, imu.aoffsetz, imu.goffsetx, imu.goffsety, imu.goffsetz);
//    Serial_Print("Gyro Noise (1σ): Nx=%.6f, Ny=%.6f, Nz=%.6f\r\n",
//                 imu.gyrox_noise, imu.gyroy_noise, imu.gyroz_noise);
//    // ===== Magnetometer calibration =====
//    Serial_Print("=== Rotate IMU slowly in all directions (figure-8) ===\r\n");
////#endif
//
//    HAL_Delay(200);
//
////    float mx_min = 1e9, my_min = 1e9, mz_min = 1e9;
////    float mx_max = -1e9, my_max = -1e9, mz_max = -1e9;
////    int16_t mx_raw, my_raw, mz_raw;
////
////    uint32_t MAG_SAMPLES = 1000;
////    for (uint32_t i = 0; i < MAG_SAMPLES; i++) {
////        if (HAL_I2C_Mem_Read(I2Cx, AK8963_I2C_ADDR, AK8963_HXL, 1, data_mag, 7, 100) != HAL_OK)
////            return;
////
////        mx_raw = (int16_t)((data_mag[1] << 8) | data_mag[0]);
////        my_raw = (int16_t)((data_mag[3] << 8) | data_mag[2]);
////        mz_raw = (int16_t)((data_mag[5] << 8) | data_mag[4]);
////
////        float mx = (float)mx_raw * mag_adj[0];
////        float my = (float)my_raw * mag_adj[1];
////        float mz = (float)mz_raw * mag_adj[2];
////
////        if (mx < mx_min) mx_min = mx;
////        if (mx > mx_max) mx_max = mx;
////        if (my < my_min) my_min = my;
////        if (my > my_max) my_max = my;
////        if (mz < mz_min) mz_min = mz;
////        if (mz > mz_max) mz_max = mz;
////
////        HAL_Delay(3);
////    }
////
////    imu.moffsetx = ((mx_max + mx_min)/2.0f) * 0.15f;
////    imu.moffsety = ((my_max + my_min)/2.0f) * 0.15f;
////    imu.moffsetz = ((mz_max + mz_min)/2.0f) * 0.15f;
////    float mx_scale = (mx_max - mx_min) / 2.0f;
////    float my_scale = (my_max - my_min) / 2.0f;
////    float mz_scale = (mz_max - mz_min) / 2.0f;
////    float avg_scale = (mx_scale + my_scale + mz_scale) / 3.0f;
////    imu.mscalex = avg_scale / mx_scale;
////    imu.mscaley = avg_scale / my_scale;
////    imu.mscalez = avg_scale / mz_scale;
////
////    Serial_Print("Magnetometer calibration done!\r\n");
////    Serial_Print("Offsets:\r\n");
////    Serial_Print(" Mx_off=%.2f, My_off=%.2f, Mz_off=%.2f\r\n", imu.moffsetx, imu.moffsety, imu.moffsetz);
////    Serial_Print(" x=%.2f, y=%.2f, z=%.2f\r\n", var_ax/ 4096.0f, var_ay/ 4096.0f, var_az/ 4096.0f);
//    Serial_Print("=============================================\r\n");
//}
//
//
