/*
 * global.h
 *
 *  Created on: Oct 10, 2025
 *      Author: NGUYEN DUY BACH
 */

#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include <stdio.h>
#include <string.h>
#include <main.h>
#include "st7735.h"
#include "fonts.h"
#include "testimg.h"

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



#endif /* INC_GLOBAL_H_ */
