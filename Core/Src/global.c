/*
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

struct geod_geodesic g;

