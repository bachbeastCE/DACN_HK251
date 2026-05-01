/*
 * global.h
 *
 *  Created on: Oct 10, 2025
 *      Author: NGUYEN DUY BACH
 */

#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include <main.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "config.h"
#include "serial.h"
#include "geodesic.h"
#include <math.h>
#include "cmsis_os.h"
#include "st7735.h"
#include "gps.h"
#include "serial.h"
#include "button.h"
//#include "ukf.h"
//#include "gps_ukf.h"
//#include "imu.h"
//#include "imu_10DOF.h"
#include "battery.h"
#include "gps_kf.h"
#include "LoRa.h"
#include "micro_aes.h"

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
extern double gps_hdop; 	//GPS_HDOP
extern uint16_t battery_percent;


extern struct geod_geodesic g;

extern osSemaphoreId spiDmaSemHandle;
extern osSemaphoreId i2cDmaSemHandle;

void Semaphore_init(void);

///////// TASK  ///////////

void TaskButton_init(void);
void TaskButton_run(void);

void TaskDebug_init(void);
void TaskDebug_run(void);


#endif /* INC_GLOBAL_H_ */
