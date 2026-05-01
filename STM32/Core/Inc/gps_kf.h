/*
 * gps_kf.h
 *
 *  Created on: Nov 20, 2025
 *      Author: ASUS
 */

#ifndef GPS_KF_H
#define GPS_KF_H

typedef struct {
    double x[6]; /* State vector: [lat, lon, height, v_lat, v_lon, v_height] */
    double P[6][6]; /* Uncertainty: 6x6 */
    double Q[6][6]; /* Process Noise: 6x6 */
    double R[3][3]; /* Measurement Noise - GPS, Barometer: 3x3 */
    double dt;      // Chu kỳ lấy mẫu (Sampling time)
} GPS_KF;

/*
 * @ Brief: Struct of acceleration in ENU
 *
 * */
typedef struct {
	double a_east; // m/s^2
	double a_north; // m/s^2
	double a_up; // m/s^2
} ENU_ACC_t;

extern GPS_KF kf_gps;

void GPS_KF_Init(GPS_KF *kf, double dt_imu);

/*
 * @Brief: The support function for converting acceleration and euler conner from IMU to acceleration in ENU
 * @param ENU_ACC_t result: Result for convertion
 * @param double a_x_meter, a_y_meter, a_z_meter: Acceleration of IMU
 * @param double roll_deg, pitch_deg, azi_deg: Euler conner
 */
void GPS_KF_Convert_Acceleration(double a_x_meter, double a_y_meter, double a_z_meter,double pitch_deg, double roll_deg, double azi_deg, double R[3][3]);

void GPS_KF_Filter(GPS_KF *kf,
                   double meas_lat, double meas_lon, double meas_alt, int is_new_meas);

#endif
