/*
 * gps_kf.h
 *
 *  Created on: Nov 20, 2025
 *      Author: ASUS
 */

#ifndef GPS_KF_H
#define GPS_KF_H



typedef struct {
    double x[3];    // lat, lon, alt
    double P[3][3]; // covariance
    double Q[3][3]; // process noise
    double R[3][3]; // measurement noise
    double dt;
} GPS_KF;

extern GPS_KF kf_gps;

void GPS_KF_Init(GPS_KF *kf);

void GPS_KF_Filter(GPS_KF *kf, double meas_lat, double meas_lon, double meas_alt,
                   double a_lat, double a_lon, double a_alt);

#endif

