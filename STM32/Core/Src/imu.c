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
volatile uint8_t imu_data_ready = 0;
const uint16_t i2c_timeout = 100;
uint8_t chipid;
imu_t imu;
BMP280 bmp;

void I2C_Scan(I2C_HandleTypeDef *I2Cx) {
	Serial_Print("OK can call this function");
    for (uint8_t addr = 1; addr < 127; addr++) {
        if (HAL_I2C_IsDeviceReady(I2Cx, addr << 1, 1, 10) == HAL_OK) {
            Serial_Print("I2C device found at 0x%02X\r\n", addr);
        }
    }
}

static inline void write (uint8_t reg, uint8_t value, I2C_HandleTypeDef *I2Cx, uint8_t devaddress)
{
	uint8_t data[2];
	data[0] = reg;
	data[1] = value;
	HAL_I2C_Master_Transmit (I2Cx, devaddress, data, 2, i2c_timeout);
}

static inline void accel_convert()
{
    int16_t x_raw = (int16_t)((data_acce[0] << 8) | data_acce[1]);
    int16_t y_raw = (int16_t)((data_acce[2] << 8) | data_acce[3]);
    int16_t z_raw = (int16_t)((data_acce[4] << 8) | data_acce[5]);

    imu.ax = ((float)x_raw - imu.aoffsetx) * imu.ascalex / 4096.0f;
    imu.ay = ((float)y_raw - imu.aoffsety) * imu.ascaley / 4096.0f;
    imu.az = ((float)z_raw - imu.aoffsetz) * imu.ascalez / 4096.0f;

#if IMU_ENABLE_SERIAL_LOG
    Serial_Print("Acce_X = %.3f g; ", imu.ax);
    Serial_Print("Acce_Y = %.3f g; ", imu.ay);
    Serial_Print("Acce_Z = %.3f g \n", imu.az);
#endif
}

static inline void mag_convert()
{
    int16_t x_raw, y_raw, z_raw;

    x_raw = (int16_t)((data_mag[1] << 8) | data_mag[0]);
    y_raw = (int16_t)((data_mag[3] << 8) | data_mag[2]);
    z_raw = (int16_t)((data_mag[5] << 8) | data_mag[4]);

//    imu.moffsetx = 33.387;
//    imu.moffsety = 32.952;
//    imu.moffsetz = (-35.936);

//    imu.moffsetx = 0;
//    imu.moffsety = 0;
//    imu.moffsetz = 0;
//    imu.mscalex = 1;
//    imu.mscaley = 1;
//    imu.mscalez = 1;
    imu.mx = ((float)x_raw * mag_adj[0]) * 0.15 * imu.mscalex;
    imu.my = (((float)y_raw * mag_adj[1]) * 0.15 * imu.mscaley);
    imu.mz = (((float)z_raw * mag_adj[2]) * 0.15 * imu.mscalez);
    imu.mx = imu.mx + 3.345;   // bias X = -3.345
    imu.my = 58.07 - imu.my;  // bias Y = +58.07
    imu.mz = 0 - imu.mz - 17.31;
//    imu.my = -imu.my;
//    imu.mz = -imu.mz;


#if IMU_ENABLE_SERIAL_LOG
    Serial_Print("Mag_X = %.3f uT; ", imu.mx);
    Serial_Print("Mag_Y = %.3f uT; ", imu.my);
    Serial_Print("Mag_Z = %.3f uT; \n", imu.mz);
    Serial_Print("#\n");
#endif
}

static inline void gyro_convert()
{
    int16_t x_raw, y_raw, z_raw;

    x_raw = (int16_t)((data_gyro[0] << 8) | data_gyro[1]);
    y_raw = (int16_t)((data_gyro[2] << 8) | data_gyro[3]);
    z_raw = (int16_t)((data_gyro[4] << 8) | data_gyro[5]);

//    imu.gx = ((float)x_raw - imu.goffsetx) * imu.gscalex / 65.5f;
//    imu.gy = ((float)y_raw - imu.goffsety) * imu.gscaley / 65.5f;
//    imu.gz = ((float)z_raw - imu.goffsetz) * imu.gscalez/ 65.5f;
    imu.gx = ((float)x_raw) / 65.5f;
    imu.gy = ((float)y_raw) / 65.5f;
    imu.gz = ((float)z_raw) / 65.5f;

#if IMU_ENABLE_SERIAL_LOG
    Serial_Print("Gyro_X = %.3f deg/s; ", imu.gx);
    Serial_Print("Gyro_Y = %.3f deg/s; ", imu.gy);
    Serial_Print("Gyro_Z = %.3f deg/s \n", imu.gz);
#endif
}

static inline void read_all(I2C_HandleTypeDef *I2Cx){
    if (HAL_I2C_Mem_Read_IT(I2Cx, MPU_I2C_ADDR, MPU9250_ACCEL_XOUT_H, 1, data_acce, 6) != HAL_OK) {
#if IMU_ENABLE_SERIAL_LOG
        Serial_Print("I2C read error!\r\n");
#endif
        return;
    }

    if (osSemaphoreWait(i2cDmaSemHandle, osWaitForever) != osOK){
        Serial_Print("ACC OUT\n");
    }
	if (HAL_I2C_Mem_Read_IT(I2Cx, MPU_I2C_ADDR, MPU9250_GYRO_XOUT_H, 1, data_gyro, 6) != HAL_OK) {
#if IMU_ENABLE_SERIAL_LOG
	   Serial_Print("I2C read error!\r\n");
#endif
	   return;
	}
    if (osSemaphoreWait(i2cDmaSemHandle, osWaitForever) != osOK){
        Serial_Print("Gyro OUT\n");
    }
    if (HAL_I2C_Mem_Read_IT(I2Cx, AK8963_I2C_ADDR, AK8963_HXL, 1, data_mag, 7) != HAL_OK) {
#if IMU_ENABLE_SERIAL_LOG
        Serial_Print("I2C read error!\r\n");
#endif
        return;
    }
    if (osSemaphoreWait(i2cDmaSemHandle, osWaitForever) != osOK){
        Serial_Print("Mag OUT\n");
    }
    baro_read(I2Cx);
    accel_convert();
    gyro_convert();
    mag_convert();
}

void imu_init(I2C_HandleTypeDef *I2Cx){
	accel_gyro_Init(I2Cx);
	mag_init(I2Cx);
	baro_init(I2Cx);
	calibrate(I2Cx);
}

void imu_compute_attitude(I2C_HandleTypeDef *I2Cx){
	read_all(I2Cx);
	float pitch_rad = atan2f(-imu.ax, sqrtf(imu.ay * imu.ay + imu.az * imu.az));
	float roll_rad  = atan2f(imu.ay, sqrtf(imu.ax * imu.ax + imu.az * imu.az));

	imu.pitch = pitch_rad * 180.0f / PI;
	imu.roll  = roll_rad  * 180.0f / PI;
	if (imu.pitch > 90.0f) imu.pitch = 90.0f;
	if (imu.pitch < -90.0f) imu.pitch = -90.0f;
	while (imu.roll > 180.0f) imu.roll -= 360.0f;
	while (imu.roll < -180.0f) imu.roll += 360.0f;
	float Mx = imu.mx * cosf(pitch_rad) + imu.my * sinf(roll_rad) * sinf(pitch_rad) + imu.mz * cosf(roll_rad) * sinf(pitch_rad);

	float My = imu.my * cosf(roll_rad) - imu.mz * sinf(roll_rad);

	imu.yaw = atan2f(My, Mx) * 180.0f / PI;
//#if IMU_ENABLE_SERIAL_LOG
	    Serial_Print("mea_pitch = %.3f degree; ", imu.pitch);
	    Serial_Print("mea_yaw = %.3f degree; ", imu.yaw);
	    Serial_Print("mea_roll = %.3f degree; \n", imu.roll);
	    Serial_Print("mea_temperature = %.3f degree C; \n", imu.temperature);
	    Serial_Print("mea_altitude = %.3f m; \n", imu.altitude);
	    Serial_Print("mea_pressure = %.3f Pa; \n", imu.pressure);
//#endif
//    Serial_Print("#######################################\n");
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
			 mag_adj[i] = (((float)(asa[i] - 128) / 256.0f) + 1.0f);
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
    	write(BAR_RST, 0xB6, I2Cx, BAR_ADDR);  // reset all bits
    	// Wait until finished copying over the NVP data.
    	uint8_t status;
    	do {
    	    HAL_I2C_Mem_Read(I2Cx, BAR_ADDR, BAR_STATUS, 1, &status, 1, 100);
    	} while (status & 0x01);
    	BMP280_get_calib_values(I2Cx);
        // Sleep
        write(BAR_CTRL_MEAS, 0x00, I2Cx, BAR_ADDR);
        // Config: t_sb=125ms, filter=16
        write(BAR_CONF, 0x50, I2Cx, BAR_ADDR);
        // Ctrl_meas: osrs_t=x16, osrs_p=x16, NORMAL
        write(BAR_CTRL_MEAS, 0xB7, I2Cx, BAR_ADDR);
        Serial_Print("BARO wake up");
    }
}

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
    imu.altitude = 44307.69396f *
               (1.0f - powf((float)(imu.pressure / 101325.0), 1.0f / 5.255f));
}
//void calibrate(I2C_HandleTypeDef *I2Cx)
//{
//    // =====================================================
//    // CONFIG
//    // =====================================================
//
//    const uint32_t BLOCK_SAMPLES = 500;   // samples per block
//    const uint32_t NUM_BLOCKS    = 60;    // total blocks
//
//    // 200 samples * 5ms = 1 second/block
//
//    float bias_x[NUM_BLOCKS];
//    float bias_y[NUM_BLOCKS];
//    float bias_z[NUM_BLOCKS];
//
//    int16_t gx_raw, gy_raw, gz_raw;
//
//    Serial_Print("\r\n=== ESTIMATE GYRO BIAS RANDOM WALK ===\r\n");
//    Serial_Print("KEEP IMU STILL...\r\n");
//
//    HAL_Delay(3000);
//
//    // =====================================================
//    // STEP 1 : ESTIMATE BIAS OVER TIME
//    // =====================================================
//
//    for(uint32_t block = 0; block < NUM_BLOCKS; block++)
//    {
//        int64_t sum_gx = 0;
//        int64_t sum_gy = 0;
//        int64_t sum_gz = 0;
//
//        for(uint32_t i = 0; i < BLOCK_SAMPLES; i++)
//        {
//            if(HAL_I2C_Mem_Read(I2Cx, MPU_I2C_ADDR, MPU9250_GYRO_XOUT_H,
//                                1, data_gyro, 6, 100) != HAL_OK){
//                return;
//            }
//
//            gx_raw = (int16_t)((data_gyro[0] << 8) | data_gyro[1]);
//            gy_raw = (int16_t)((data_gyro[2] << 8) | data_gyro[3]);
//            gz_raw = (int16_t)((data_gyro[4] << 8) | data_gyro[5]);
//            sum_gx += gx_raw;
//            sum_gy += gy_raw;
//            sum_gz += gz_raw;
//            HAL_Delay(2);
//        }
//
//        // mean bias of current block
//        bias_x[block] = ((float)sum_gx / BLOCK_SAMPLES) / 65.5f;
//        bias_y[block] = ((float)sum_gy / BLOCK_SAMPLES) / 65.5f;
//        bias_z[block] = ((float)sum_gz / BLOCK_SAMPLES) / 65.5f;
//
//        Serial_Print("Block %lu : "
//                     "Bx=%.6f "
//                     "By=%.6f "
//                     "Bz=%.6f\r\n",
//                     block,
//                     bias_x[block],
//                     bias_y[block],
//                     bias_z[block]);
//    }
//
//    // =====================================================
//    // STEP 2 : COMPUTE BIAS RANDOM WALK VARIANCE
//    // =====================================================
//    float var_bx = 0.0f;
//    float var_by = 0.0f;
//    float var_bz = 0.0f;
//    for(uint32_t k = 1; k < NUM_BLOCKS; k++)
//    {
//        float dbx = bias_x[k] - bias_x[k - 1];
//        float dby = bias_y[k] - bias_y[k - 1];
//        float dbz = bias_z[k] - bias_z[k - 1];
//        var_bx += dbx * dbx;
//        var_by += dby * dby;
//        var_bz += dbz * dbz;
//    }
//    var_bx /= (NUM_BLOCKS - 1);
//    var_by /= (NUM_BLOCKS - 1);
//    var_bz /= (NUM_BLOCKS - 1);
//
//    // =====================================================
//    // SAVE
//    // =====================================================
//    imu.gbias_var_x = var_bx;
//    imu.gbias_var_y = var_by;
//    imu.gbias_var_z = var_bz;
//    // =====================================================
//    // PRINT
//    // =====================================================
//
//    Serial_Print("\r\n=== GYRO BIAS RANDOM WALK VAR ===\r\n");
//    Serial_Print("Var Bx = %.10f\r\n", imu.gbias_var_x);
//    Serial_Print("Var By = %.10f\r\n", imu.gbias_var_y);
//    Serial_Print("Var Bz = %.10f\r\n", imu.gbias_var_z);
//    Serial_Print("=================================\r\n");
//}
void calibrate(I2C_HandleTypeDef *I2Cx) {
    // ===== Gyro + Accel calibration =====
//    int64_t sum_gx = 0, sum_gy = 0, sum_gz = 0;
//    int64_t sum_ax = 0, sum_ay = 0, sum_az = 0;


    uint32_t CALIB_SAMPLES = 1000;
//    int16_t gx_sample[CALIB_SAMPLES], gy_sample[CALIB_SAMPLES], gz_sample[CALIB_SAMPLES];
//    int16_t ax_raw, ay_raw, az_raw;
    Serial_Print("=== Keep IMU still... Calibrating gyro & accel... ===\r\n");
    HAL_Delay(100);
//    int16_t gx_raw, gy_raw, gz_raw;
//    for (uint32_t i = 0; i < CALIB_SAMPLES; i++) {
//        // --- Read Gyro ---
//        if (HAL_I2C_Mem_Read(I2Cx, MPU_I2C_ADDR, MPU9250_GYRO_XOUT_H, 1, data_gyro, 6, 100) != HAL_OK)
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
//        if (HAL_I2C_Mem_Read(I2Cx, MPU_I2C_ADDR, MPU9250_ACCEL_XOUT_H, 1, data_acce, 6, 100) != HAL_OK)
//            return;
//
//        ax_raw = (int16_t)((data_acce[0] << 8) | data_acce[1]);
//
//        az_raw = (int16_t)((data_acce[4] << 8) | data_acce[5]);
//
//        ay_raw = (int16_t)((data_acce[2] << 8) | data_acce[3]);
//        sum_ax += ax_raw;
//        sum_ay += ay_raw;
//        sum_az += az_raw;
//
//        HAL_Delay(2);
//    }
//    Serial_Print("ACC RAW: %d %d %d | GYRO RAW: %d %d %d\r\n",
//                 ax_raw, ay_raw, az_raw,
//                 gx_raw, gy_raw, gz_raw);
//
//    imu.goffsetx = (float)sum_gx / CALIB_SAMPLES;
//    imu.goffsety = (float)sum_gy / CALIB_SAMPLES;
//    imu.goffsetz = (float)sum_gz / CALIB_SAMPLES;
//    imu.aoffsetx = (float)sum_ax / CALIB_SAMPLES;
//    imu.aoffsety = (float)sum_ay / CALIB_SAMPLES;
//    imu.aoffsetz = (float)sum_az / CALIB_SAMPLES - 4096;
//    sum_gx = 0;
//    sum_gy = 0;
//    sum_gz = 0;
    // ===== ACCEL OFFSET (RAW) =====
    imu.aoffsetx = 244.577f;
    imu.aoffsety = 170.902f;
    imu.aoffsetz = 3768.861f - 4096;
    imu.ascalex = 0.982f;
    imu.ascaley = 0.993f;
    imu.ascalez = 1.013f;
    imu.mscalex = 1.0647f;
    imu.mscaley = 0.9433f;
    imu.mscalez = 0.9993f;
    // ===== GYRO OFFSET (RAW) =====
    imu.goffsetx = -25.028f;
    imu.goffsety = 31.544f;
    imu.goffsetz = 6.769f;
    imu.gscalex = 1.027f;
    imu.gscaley = 1.064f;
    imu.gscalez = 0.992f;
	imu.gbias_var_x = 0.00009f;
    imu.gbias_var_y = 0.00005f;
    imu.gbias_var_z = 0.00005f;
	imu.gscalex_var_x = 1e-6f;
    imu.gscaley_var_y = 1e-6f;
    imu.gscalez_var_z = 1e-6f;
//    float var_gx = 0, var_gy = 0, var_gz = 0;
//    for (uint32_t i = 0; i < CALIB_SAMPLES; i++) {
//        var_gx = var_gx + (gx_sample[i] - imu.goffsetx) * (gx_sample[i] - imu.goffsetx);
//        var_gy = var_gy + (gy_sample[i] - imu.goffsety) * (gy_sample[i] - imu.goffsety);
//        var_gz = var_gz + (gz_sample[i] - imu.goffsetz) * (gz_sample[i] - imu.goffsetz);
//    }
//
//    var_gx /= (CALIB_SAMPLES - 1);
//    var_gy /= (CALIB_SAMPLES - 1);
//    var_gz /= (CALIB_SAMPLES - 1);

//    imu.gyrox_noise = sqrtf(var_gx);
//    imu.gyroy_noise = sqrtf(var_gy);
//    imu.gyroz_noise = sqrtf(var_gz);
    imu.gyrox_noise = 9.7054;
    imu.gyroy_noise = 13.0541;
    imu.gyroz_noise = 6.1487;

#if IMU_ENABLE_SERIAL_LOG
    Serial_Print("Gyro & Accel calibration done!\r\n");
    Serial_Print("Ax_off=%.3f, Ay_off=%.3f, Az_off=%.3f\r\n, Gx_off=%.3f, Gy_off=%.3f, Gz_off=%.3f\r\n", imu.aoffsetx, imu.aoffsety, imu.aoffsetz, imu.goffsetx, imu.goffsety, imu.goffsetz);
    Serial_Print("Gyro Noise (1σ): Nx=%.6f, Ny=%.6f, Nz=%.6f\r\n",
                 imu.gyrox_noise, imu.gyroy_noise, imu.gyroz_noise);
    // ===== Magnetometer calibration =====
    Serial_Print("=== Rotate IMU slowly in all directions (figure-8) ===\r\n");
#endif

//    HAL_Delay(200);
//
//    float mx_min = 1e9, my_min = 1e9, mz_min = 1e9;
//    float mx_max = -1e9, my_max = -1e9, mz_max = -1e9;
//    int16_t mx_raw, my_raw, mz_raw;
//
//    uint32_t MAG_SAMPLES = 1000;
//    for (uint32_t i = 0; i < MAG_SAMPLES; i++) {
//        if (HAL_I2C_Mem_Read(I2Cx, AK8963_I2C_ADDR, AK8963_HXL, 1, data_mag, 7, 100) != HAL_OK)
//            return;
//
//        mx_raw = (int16_t)((data_mag[1] << 8) | data_mag[0]);
//        my_raw = (int16_t)((data_mag[3] << 8) | data_mag[2]);
//        mz_raw = (int16_t)((data_mag[5] << 8) | data_mag[4]);
//
//        float mx = (float)mx_raw * mag_adj[0];
//        float my = (float)my_raw * mag_adj[1];
//        float mz = (float)mz_raw * mag_adj[2];
//
//        if (mx < mx_min) mx_min = mx;
//        if (mx > mx_max) mx_max = mx;
//        if (my < my_min) my_min = my;
//        if (my > my_max) my_max = my;
//        if (mz < mz_min) mz_min = mz;
//        if (mz > mz_max) mz_max = mz;
//
//        HAL_Delay(3);
//    }
//
//    imu.moffsetx = ((mx_max + mx_min)/2.0f) * 0.15f;
//    imu.moffsety = ((my_max + my_min)/2.0f) * 0.15f;
//    imu.moffsetz = ((mz_max + mz_min)/2.0f) * 0.15f;
//    float mx_scale = (mx_max - mx_min) / 2.0f;
//    float my_scale = (my_max - my_min) / 2.0f;
//    float mz_scale = (mz_max - mz_min) / 2.0f;
//    float avg_scale = (mx_scale + my_scale + mz_scale) / 3.0f;
//    imu.mscalex = avg_scale / mx_scale;
//    imu.mscaley = avg_scale / my_scale;
//    imu.mscalez = avg_scale / mz_scale;
//
//    Serial_Print("Magnetometer calibration done!\r\n");
//    Serial_Print("Offsets:\r\n");
//    Serial_Print(" Mx_off=%.2f, My_off=%.2f, Mz_off=%.2f\r\n", imu.moffsetx, imu.moffsety, imu.moffsetz);
//    Serial_Print("=============================================\r\n");
}
float readAccelAverageX(I2C_HandleTypeDef *I2Cx, uint32_t samples){
    uint8_t data[6];
    int16_t ax_raw;
    float sum = 0;
    for(uint32_t i = 0; i < samples; i++)
    {
        HAL_I2C_Mem_Read(I2Cx,
                         MPU_I2C_ADDR,
                         MPU9250_ACCEL_XOUT_H,
                         1,
                         data,
                         6,
                         100);
        ax_raw = (int16_t)((data[0] << 8) | data[1]);
        sum += ax_raw;
        HAL_Delay(2);
    }
    return sum / samples;
}

float readAccelAverageY(I2C_HandleTypeDef *I2Cx, uint32_t samples){
    uint8_t data[6];
    int16_t ay_raw;
    float sum = 0;
    for(uint32_t i = 0; i < samples; i++)
    {
        HAL_I2C_Mem_Read(I2Cx,
                         MPU_I2C_ADDR,
                         MPU9250_ACCEL_XOUT_H,
                         1,
                         data,
                         6,
                         100);

        ay_raw = (int16_t)((data[2] << 8) | data[3]);
        sum += ay_raw;
        HAL_Delay(2);
    }
    return sum / samples;
}

float readAccelAverageZ(I2C_HandleTypeDef *I2Cx, uint32_t samples){
    uint8_t data[6];
    int16_t az_raw;
    float sum = 0;
    for(uint32_t i = 0; i < samples; i++)
    {
        HAL_I2C_Mem_Read(I2Cx,
                         MPU_I2C_ADDR,
                         MPU9250_ACCEL_XOUT_H,
                         1,
                         data,
                         6,
                         100);

        az_raw = (int16_t)((data[4] << 8) | data[5]);
        sum += az_raw;
        HAL_Delay(2);
    }
    return sum / samples;
}

//void calibrate(I2C_HandleTypeDef *I2Cx)
//{
//    const uint32_t SAMPLES = 1000;
//
//    // MPU9250 ±8g
//    const float IDEAL_1G = 4096.0f;
//
//    float ax_pos, ax_neg;
//    float ay_pos, ay_neg;
//    float az_pos, az_neg;
//
//    Serial_Print("\r\n===== ACCEL SCALE CALIBRATION =====\r\n");
//
//    // =====================================================
//    // X AXIS
//    // =====================================================
//
//    Serial_Print("Place +X UP\r\n");
//    HAL_Delay(5000);
//
//    ax_pos = readAccelAverageX(I2Cx, SAMPLES);
//
//    Serial_Print("Place -X UP\r\n");
//    HAL_Delay(5000);
//
//    ax_neg = readAccelAverageX(I2Cx, SAMPLES);
//
//    // =====================================================
//    // Y AXIS
//    // =====================================================
//
//    Serial_Print("Place +Y UP\r\n");
//    HAL_Delay(5000);
//
//    ay_pos = readAccelAverageY(I2Cx, SAMPLES);
//
//    Serial_Print("Place -Y UP\r\n");
//    HAL_Delay(5000);
//
//    ay_neg = readAccelAverageY(I2Cx, SAMPLES);
//
//    // =====================================================
//    // Z AXIS
//    // =====================================================
//
//    Serial_Print("Place +Z UP\r\n");
//    HAL_Delay(5000);
//
//    az_pos = readAccelAverageZ(I2Cx, SAMPLES);
//
//    Serial_Print("Place -Z UP\r\n");
//    HAL_Delay(5000);
//
//    az_neg = readAccelAverageZ(I2Cx, SAMPLES);
//
//    // =====================================================
//    // CALCULATE SCALE
//    // =====================================================
//
//    imu.ascalex =
//        (2.0f * IDEAL_1G) / (ax_pos - ax_neg);
//
//    imu.ascaley =
//        (2.0f * IDEAL_1G) / (ay_pos - ay_neg);
//
//    imu.ascalez =
//        (2.0f * IDEAL_1G) / (az_pos - az_neg);
//
//    // =====================================================
//    // PRINT
//    // =====================================================
//
//    Serial_Print("\r\nAccel Scale Done!\r\n");
//
//    Serial_Print("Scale X = %.6f\r\n", imu.ascalex);
//    Serial_Print("Scale Y = %.6f\r\n", imu.ascaley);
//    Serial_Print("Scale Z = %.6f\r\n", imu.ascalez);
//
//    Serial_Print("====================================\r\n");
//}
