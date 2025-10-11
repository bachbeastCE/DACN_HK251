/*
 * global.c
 *
<<<<<<< HEAD
 *  Created on: Oct 9, 2025
 *      Author: Admin
 */
#include "global.h"

uint16_t uart2_idx = 0;
uint16_t uart2_tranfer_count = 0;
uint8_t uart2_rx_buffer[UART2_BUFFER_SIZE];
uint8_t uart2_copy_flag = 1;

=======
 *  Created on: Oct 10, 2025
 *      Author: NGUYEN DUY BACH
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
>>>>>>> gps_update
