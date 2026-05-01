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

struct geod_geodesic g;
COORDINATES_t raw_coordinates;
COORDINATES_t kf_point;

int time = 0;
uint8_t kf_start =0;
double R_matrix[3][3];

int is_gps_new = 0;



//// GLOBAL PARAMETER OF LORA TASK & SECURITY
//LoRa myLoRa;
//uint8_t key[] = {0x2b, 0x7e, 0x15, 0x16,0x28, 0xae, 0xd2, 0xa6,0xab, 0xf7, 0x15, 0x88,0x09, 0xcf, 0x4f, 0x3c};
//LOC_DATA_PAYLOAD loc_data_payload;
//LOC_DATA_HEADER loc_data_header;
//uint8_t gcm_nonce[GCM_NONCE_LEN];
//uint8_t plain_text_buffer[LOC_DATA_HEADER_SIZE + LOC_DATA_PAYLOAD_SIZE];
//uint8_t cipher_text_buffer[LOC_DATA_HEADER_SIZE + LOC_DATA_PAYLOAD_SIZE + GCM_TAG_LEN];


osMutexId button_mutexHandle;
osMutexDef(button_mutex);

osSemaphoreId spiDmaSemHandle;
osSemaphoreId i2cDmaSemHandle;
osSemaphoreDef(I2C_DMA_SEM);
osSemaphoreDef(SPI_DMA_SEM);



void Semaphore_init(void){
	i2cDmaSemHandle = osSemaphoreCreate(osSemaphore(I2C_DMA_SEM), 1);
	spiDmaSemHandle = osSemaphoreCreate(osSemaphore(SPI_DMA_SEM), 1);
}

void Mutex_init(void){
	button_mutexHandle = osMutexCreate(osMutex(button_mutex));
}




void TaskButton_init(void){
	//Button_Init();
}

void TaskButton_run(void){
    // Đổi về 100ms để đoạn check Timeout có tác dụng
//    osStatus status = osMutexWait(button_mutexHandle, 100);

//    if (status == osOK) {
        getKeyInput(); // Hàm này chạy xong phải thoát ngay, không được chứa HAL_Delay!
//        osMutexRelease(button_mutexHandle);
//    }
//    else if (status == osEventTimeout) {
//        Serial_Print("Warning: TaskButton wait mutex timeout\n\r");
//    }
    osDelay(10);
}


void TaskDebug_init(void){
	Serial_Print("START DEBUGING\n\r");
};


void TaskDebug_run(void){
    // Biến cục bộ để lưu trạng thái nút nhấn
    uint8_t mid_button_pressed = 0;
    uint8_t right_button_pressed = 0;

//    osStatus status = osMutexWait(button_mutexHandle, 100);

//    if (status == osOK) {
//        // Kiểm tra ĐỘC LẬP từng nút nhấn
        if(isButtonPressed(0)) {
            mid_button_pressed = 1;
        }

        // Dùng 'if' thay vì 'else if'
        if(isButtonPressed(1)) {
            right_button_pressed = 1;
        }

        // Nhả Mutex NGAY LẬP TỨC
 //       osMutexRelease(button_mutexHandle);

//    } else if (status == osEventTimeout) {
//        Serial_Print("Warning: TaskDebug wait mutex timeout\n\r");
//    }

    // Xử lý việc in ấn (tốn thời gian) ở BÊN NGOÀI vùng Mutex
    // Có thể in cả 2 nếu người dùng nhấn cả 2
    if (mid_button_pressed == 1) {
        Serial_Print("Press MID button\n\r");
    }

    if (right_button_pressed == 1) {
        Serial_Print("Press RIGHT button\n\r");
    }

    osDelay(100);
}






