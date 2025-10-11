/*
 * global.h
 *
<<<<<<< HEAD
 *  Created on: Oct 9, 2025
 *      Author: Admin
=======
 *  Created on: Oct 10, 2025
 *      Author: NGUYEN DUY BACH
>>>>>>> gps_update
 */

#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_
<<<<<<< HEAD
#include <main.h>
#include <stdio.h>
#include <string.h>

#include "gps.h"
#include "geodesic.h"

//UART TRANMISION
#define UART2_BUFFER_SIZE 1024

extern uint16_t uart2_idx;
extern uint16_t uart2_tranfer_count;
extern uint8_t uart2_copy_flag;
extern uint8_t uart2_rx_buffer[UART2_BUFFER_SIZE];
//extern UART_HandleTypeDef huart2;
=======

extern double loc_gps_lon; 	//GPS LOCAL LONGTITUDE
extern double loc_gps_lat; 	//GPS LOCAL LATITUDE
extern double loc_gps_alt; 	//GPS LOCAL ALTITUDE
extern double loc_azi; 		//LOCAL AZIMUTH
extern double loc_pitch; 	//LOCAL PITCH
extern double loc_yaw; 		//LOCAL PITCH
extern double loc_roll; 	//LOCAL PITCH

extern double tag_gps_lon; 	//GPS TAGGET  LONGTITUDE
extern double tag_gps_lat; 	//GPS TAGGET LATITUDE
extern double tag_gps_alt; 	//GPS TAGGET ALTITUDE
extern double tag_distance; //TAGGET DISTANCE

>>>>>>> gps_update


#endif /* INC_GLOBAL_H_ */
