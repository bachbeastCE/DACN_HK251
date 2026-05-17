/*
 * global.c
 *
 *  Created on: Jan 25, 2026
 *      Author: Admin
 */

#include "global.h"
#include "config.h"

double loc_gps_lon = 0.0;
double loc_gps_lat = 0.0;
double loc_gps_alt = 0.0;
double loc_azi = 1.0;
double loc_pitch = 2.0;
double loc_roll = 3.0;
double tag_gps_lon = 0.0;
double tag_gps_lat = 0.0;
double tag_gps_alt = 0.0;
double tag_distance = 100.0;
double gps_hdop = 0.0;
double ax_mps2 = 0;
double ay_mps2 = 0;
double az_mps2 = 0;
double baro_alt = 16;
int time = 0;
uint8_t kf_start = 0;
double R_matrix[3][3];

volatile uint8_t is_gps_new = 0;


struct geod_geodesic g;
COORDINATES_t raw_coordinates;
COORDINATES_t kf_point;
LoRa myLoRa;

//// GLOBAL PARAMETER OF LORA TASK & SECURITY
uint8_t key[] = {0x2b, 0x7e, 0x15, 0x16,0x28, 0xae, 0xd2, 0xa6,0xab, 0xf7, 0x15, 0x88,0x09, 0xcf, 0x4f, 0x3c};
LOC_DATA_PAYLOAD loc_data_payload;
LOC_DATA_HEADER loc_data_header;
uint8_t gcm_nonce[GCM_NONCE_LEN];
uint8_t plain_text_buffer[LOC_DATA_HEADER_SIZE + LOC_DATA_PAYLOAD_SIZE];
uint8_t cipher_text_buffer[LOC_DATA_HEADER_SIZE + LOC_DATA_PAYLOAD_SIZE + GCM_TAG_LEN];

//// RTOS GLOBAL VARIABLES
osSemaphoreId spiDmaSemHandle;
osSemaphoreId i2cDmaSemHandle;
osSemaphoreDef(I2C_DMA_SEM);
osSemaphoreDef(SPI_DMA_SEM);

//// SYSTEM CONTROL

uint8_t startup = 1;

void Semaphore_init(void){
	i2cDmaSemHandle = osSemaphoreCreate(osSemaphore(I2C_DMA_SEM), 1);
	spiDmaSemHandle = osSemaphoreCreate(osSemaphore(SPI_DMA_SEM), 1);
}

void TaskIMU_init(I2C_HandleTypeDef *I2Cx){
	I2C_Scan(I2Cx);
	ukfInit(&ukf, I2Cx);
}

void TaskIMU_run(I2C_HandleTypeDef *I2Cx){
	if (osSemaphoreWait(i2cDmaSemHandle, osWaitForever) == osOK){
		Serial_Print("IMU Run\n\r");
		imu_compute_attitude(I2Cx);
		ukf_filter(&ukf, &imu);
		//         loc_azi   = imu.mx;
		 //         loc_pitch = imu.my;
		 //         loc_roll  = imu.mz;
		 //         loc_azi   = imu.gx;
		 //         loc_pitch = imu.gy;
		 //         loc_roll  = imu.gz;

		 loc_roll  = ukf.x[0];
		 loc_pitch = -ukf.x[1];
		 loc_azi   = ukf.x[2];

		 //         loc_azi = imu.yaw;

//		 float yaw_offset = 180.0f - 360.0f;
		 loc_azi += 100;

		 if (loc_azi < 0) loc_azi += 360.0f;
		 if (loc_azi >= 360.0f) loc_azi -= 360.0f;

		 //         loc_pitch = imu.pitch;
		 //         loc_azi   = imu.yaw;
		 //         loc_roll  = imu.roll;

		 osSemaphoreRelease(i2cDmaSemHandle);
	}


}

void TaskGPS_init(void){
	GPS_Init();
	GPS_KF_Init(&kf_gps, 0.05);
	geod_init(&g, 6378137, 1/298.257223563);
}


void TaskGPS_run(void){
	if (osSemaphoreWait(i2cDmaSemHandle, osWaitForever) == osOK){
				Serial_Print("Task GPS\n\r");
				baro_alt = 16; //
				gps_hdop = GPS_HDOP_Get();
				GPS_KF_Convert_Acceleration(imu.ax, imu.ay, imu.az,
				                                  loc_pitch, loc_roll, loc_azi, R_matrix);
					is_gps_new = 0;
					if (GPS_Status_Get()){ // Hàm này chỉ kiểm tra cờ, không được delay
				          GPS_Coordinates_Get(&raw_coordinates);
				          is_gps_new = 1;

				          // KHỞI TẠO LẦN ĐẦU (Nếu chưa chạy)
				          if (!kf_start)
				          {
				              // Set vị trí ban đầu
				              kf_gps.x[0] = raw_coordinates.Lat;
				              kf_gps.x[1] = raw_coordinates.Lon;
				              //baro_alt = raw_coordinates.Alt;

				              // Nếu dùng Baro thì set độ cao baro làm gốc
				              kf_gps.x[2] = baro_alt;

				              // Reset vận tốc về 0
				              kf_gps.x[3] = 0; kf_gps.x[4] = 0; kf_gps.x[5] = 0;
				              kf_start = 1;
				          }
				      }

				      if (kf_start)
				      {
				    	  GPS_KF_Filter(&kf_gps,
				                        raw_coordinates.Lat,
				                        raw_coordinates.Lon,
				                        baro_alt,
				                        is_gps_new);

				          loc_gps_lat = kf_gps.x[0];
				          loc_gps_lon = kf_gps.x[1];
				          loc_gps_alt = kf_gps.x[2];

				      }

				  loc_gps_alt = 16;
			      double pitch_rad = loc_pitch * M_PI / 180.0;
			      double s12 = tag_distance * cos(pitch_rad);
			      tag_gps_alt = loc_gps_alt + tag_distance * sin(pitch_rad);

			      geod_direct(&g, loc_gps_lat, loc_gps_lon, loc_azi, s12, &tag_gps_lat, &tag_gps_lon, NULL);
			      osSemaphoreRelease(i2cDmaSemHandle);
			}
}



void TaskLCD_init(){
	ST7735_init();
}
void TaskLCD_run(){
	ST7735_Run();
}

void TaskBattery_init(void)
{
#if BATTERY_ENABLE_SERIAL_LOG
    Serial_Print("[Battery] Initialize battery ADC\r\n");
#endif

    Battery_Init();

    // Mồi bộ lọc 3 lần liên tiếp
    for (int i = 0; i < 3; i++)
    {
        Battery_Run();
        osDelay(1000);
    }

#if BATTERY_ENABLE_SERIAL_LOG
    Serial_Print("[Battery] Initialized battery successfully\r\n");
#endif
}

void TaskBattery_run(void)
{
    Battery_Run();
}

void TaskLoRa_init(void){
		Serial_Print("[LoRa] Initialize LoRa\n\r");
		myLoRa = LoRa_create(GPIOA, GPIO_PIN_15, GPIOA, GPIO_PIN_10, &hspi3); //PA4 SPI3 CS, PA10 RESET
		myLoRa.frequency             = 434;							  // default = 433 MHz
		myLoRa.spredingFactor        = SF_7;							// default = SF_7
		myLoRa.bandWidth			 = BW_125KHz;				  // default = BW_125KHz
		myLoRa.crcRate				 = CR_4_5;						// default = CR_4_5
		myLoRa.power				= POWER_20db;				// default = 20db
		myLoRa.overCurrentProtection = 120; 							// default = 100 mA
		myLoRa.preamble				       = 10;		  					// default = 8;

		Serial_Print("[LoRa] Reset LoRa\n\r");
		LoRa_reset(&myLoRa);


		if(LoRa_init(&myLoRa)  == LORA_OK){
			LoRa_setSyncWord(&myLoRa, 0xF1);
			Serial_Print("[LoRa] Initialize LoRa successfully\n\r");
		}
		else{
			Serial_Print("[LoRa] Initialize LoRa failed\n\r");
		}

		uint8_t ver = LoRa_read(&myLoRa, 0x42);
				Serial_Print("[LoRa] Version: 0x%02X\r\n", ver);


}

void TaskLoRa_run(void){

#ifdef LORA_ENABLE_SERIAL_LOG
	Serial_Print("[LoRa] transfer starting\r\n");
#endif

	loc_data_payload.device_id = LORA_DEVICE_ID;
	loc_data_payload.gps_hdop = gps_hdop;
	loc_data_payload.loc_gps_lon = loc_gps_lon;
	loc_data_payload.loc_gps_lat = loc_gps_lat;
	loc_data_payload.loc_gps_alt = loc_gps_alt;
	loc_data_payload.tag_gps_lon = tag_gps_lon;
	loc_data_payload.tag_gps_lat = tag_gps_lon;
	loc_data_payload.tag_gps_alt = tag_gps_alt;
	loc_data_payload.tag_distance = tag_distance;



//#ifdef LORA_TEST_TRANSFER
//	Serial_Print("[LoRa] Initial test LoRa data\r\n");
//	loc_data_payload.device_id = 0XFFFF;
//	loc_data_payload.gps_hdop = 1.23;
//	loc_data_payload.loc_gps_lon = 106.805482;
//	loc_data_payload.loc_gps_lat = 10.880748;
//	loc_data_payload.loc_gps_alt = 1000;
//	loc_data_payload.tag_gps_lon = 106.798498;
//	loc_data_payload.tag_gps_lat = 10.874163;
//	loc_data_payload.tag_gps_alt = 1000;
//	loc_data_payload.tag_distance = 12345678;
//#endif

	loc_data_header.seq_num++;
	memcpy(plain_text_buffer,&loc_data_header, sizeof(LOC_DATA_HEADER));
	memcpy(plain_text_buffer+8,&loc_data_payload, sizeof(LOC_DATA_PAYLOAD));
	memset(gcm_nonce, 0,  GCM_NONCE_LEN);
	memcpy(gcm_nonce, &loc_data_header, sizeof(LOC_DATA_HEADER)); //Copy 8 bytes from header to gcm_nonce to create specific nonce of one time tranfer
	memcpy(cipher_text_buffer, (uint8_t*)&loc_data_header, sizeof(LOC_DATA_HEADER));
	AES_GCM_encrypt(key, gcm_nonce, &loc_data_header, LOC_DATA_HEADER_SIZE , &loc_data_payload, LOC_DATA_PAYLOAD_SIZE, cipher_text_buffer + sizeof(LOC_DATA_HEADER));
	//LoRa_transmit_DMA(&myLoRa, cipher_text_buffer, sizeof(cipher_text_buffer), 400);
	Serial_Print("[LoRa] transferring\r\n");
	LoRa_transmit(&myLoRa, cipher_text_buffer, sizeof(cipher_text_buffer), 400);

#ifdef LORA_ENABLE_SERIAL_LOG
	Serial_Print("[LoRa] transfered success\r\n");
#endif

}

void TaskButton_init(void){
	Serial_Print("[Button] Intitial button\n\r");
}

void TaskButton_run(void){
	getKeyInput();
}


void TaskDebug_init(void){
	Serial_Print("[Debug] Intitial Debug\n\r");
};


void TaskDebug_run(void){
//	if(isButtonPressed(0)) {
//		Serial_Print("[Debug] Press Button\n\r");
//	}
	//Serial_Print("[Debug] Run Debug\n\r");
}






