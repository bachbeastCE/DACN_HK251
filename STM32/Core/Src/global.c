/*
 * global.c
 *
 *  Created on: Jan 25, 2026
 *      Author: Admin
 */

#include "global.h"

double loc_gps_lon = 0.0;
double loc_gps_lat = 0.0;
double loc_gps_alt = 0.0;
double loc_azi = 0.0;
double loc_pitch = 0.0;
double loc_yaw = 0.0;
double loc_roll = 0.0;
double tag_gps_lon = 0.0;
double tag_gps_lat = 0.0;
double tag_gps_alt = 0.0;
double tag_distance = 0.0;
double gps_hdop = 0.0;

int time = 0;
uint8_t kf_start =0;
double R_matrix[3][3];

int is_gps_new = 0;


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

void Semaphore_init(void){
	i2cDmaSemHandle = osSemaphoreCreate(osSemaphore(I2C_DMA_SEM), 1);
	spiDmaSemHandle = osSemaphoreCreate(osSemaphore(SPI_DMA_SEM), 1);
}


void TaskLoRa_init(void){
		myLoRa = LoRa_create(GPIOA, GPIO_PIN_4, GPIOA, GPIO_PIN_10, &hspi1); //PA4 SPI3 CS, PA10 RESET
		myLoRa.frequency             = 434;							  // default = 433 MHz
		myLoRa.spredingFactor        = SF_7;							// default = SF_7
		myLoRa.bandWidth			 = BW_125KHz;				  // default = BW_125KHz
		myLoRa.crcRate				 = CR_4_5;						// default = CR_4_5
		myLoRa.power				= POWER_20db;				// default = 20db
		myLoRa.overCurrentProtection = 120; 							// default = 100 mA
		myLoRa.preamble				       = 10;		  					// default = 8;

		LoRa_reset(&myLoRa);

		#ifdef LORA_ENABLE_SERIAL_LOG
			uint8_t ver = LoRa_read(&myLoRa, 0x42);
			Serial_Print("[LoRa] Version: 0x%02X\r\n", ver);
		#endif

		LoRa_init(&myLoRa);
		LoRa_setSyncWord(&myLoRa, 0x12);
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

#ifdef LORA_TEST_TRANSFER
	Serial_Print("[LoRa] Initial test LoRa data\r\n");
	loc_data_payload.device_id = 0XFFFF;
	loc_data_payload.gps_hdop = 1.23;
	loc_data_payload.loc_gps_lon = 123.45678;
	loc_data_payload.loc_gps_lat = 123.45678;
	loc_data_payload.loc_gps_alt = 12345678;
	loc_data_payload.tag_gps_lon = 123.45678;
	loc_data_payload.tag_gps_lat = 123.45678;
	loc_data_payload.tag_gps_alt = 12345678;
	loc_data_payload.tag_distance = 12345678;
#endif

	loc_data_header.seq_num++;
	memcpy(plain_text_buffer,&loc_data_header, sizeof(LOC_DATA_HEADER));
	memcpy(plain_text_buffer+8,&loc_data_payload, sizeof(LOC_DATA_PAYLOAD));
	memset(gcm_nonce, 0,  GCM_NONCE_LEN);
	memcpy(gcm_nonce, &loc_data_header, sizeof(LOC_DATA_HEADER)); //Copy 8 bytes from header to gcm_nonce to create specific nonce of one time tranfer
	memcpy(cipher_text_buffer, (uint8_t*)&loc_data_header, sizeof(LOC_DATA_HEADER));
	AES_GCM_encrypt(key, gcm_nonce, &loc_data_header, LOC_DATA_HEADER_SIZE , &loc_data_payload, LOC_DATA_PAYLOAD_SIZE, cipher_text_buffer + sizeof(LOC_DATA_HEADER));
	LoRa_transmit_DMA(&myLoRa, cipher_text_buffer, sizeof(cipher_text_buffer), 400);

#ifdef LORA_ENABLE_SERIAL_LOG
	Serial_Print("[LoRa] transfered success\r\n");
#endif

	 osDelay(2000);
}

void TaskButton_init(void){
	//Button_Init();
}

void TaskButton_run(void){
	getKeyInput();
    osDelay(10);
}


void TaskDebug_init(void){
	Serial_Print("[Debug] START DEBUGING\n\r");
};


void TaskDebug_run(void){
	if(isButtonPressed(0)) {
		Serial_Print("[Debug] Press MID button\n\r");
	}
	if(isButtonPressed(1)) {
		Serial_Print("[Debug] Press RIGHT button\n\r");
	}
    osDelay(100);
}






