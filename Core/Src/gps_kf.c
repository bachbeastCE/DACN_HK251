/*
 * gps_kf.c
 *
 * Created on: Nov 20, 2025
 * Author: NGUYEN DUY BACH
 */

#include "gps_kf.h"
#include <string.h>
#include <math.h>

/* Constants Definition */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define METERS_PER_DEG 111319.5
#define DEG_TO_RAD (M_PI / 180.0)
#define G_EARTH 9.80665

GPS_KF kf_gps;
ENU_ACC_t enu_acc;

// ======================== Matrix utils ========================
static void mat3_inv(double A[3][3], double inv[3][3])
{
    double a = A[0][0], b = A[0][1], c = A[0][2];
    double d = A[1][0], e = A[1][1], f = A[1][2];
    double g = A[2][0], h = A[2][1], i = A[2][2];

    double det = a*(e*i - f*h) - b*(d*i - f*g) + c*(d*h - e*g);

    // Avoid division by zero
    if(fabs(det) < 1e-12) det = 1e-12;

    double invdet = 1.0/det;

    inv[0][0] = (e*i - f*h)*invdet;
    inv[0][1] = (c*h - b*i)*invdet;
    inv[0][2] = (b*f - c*e)*invdet;

    inv[1][0] = (f*g - d*i)*invdet;
    inv[1][1] = (a*i - c*g)*invdet;
    inv[1][2] = (c*d - a*f)*invdet;

    inv[2][0] = (d*h - e*g)*invdet;
    inv[2][1] = (g*b - a*h)*invdet;
    inv[2][2] = (a*e - b*d)*invdet;
}

// ======================== KF Init ========================
void GPS_KF_Init(GPS_KF *kf, double dt_imu)
{
    memset(kf, 0, sizeof(GPS_KF));
    kf->dt = dt_imu;

    // 1. Initialize P (Initial Covariance Estimate)
    // Moderate trust in position, lower trust in velocity (to allow faster convergence)
    for(int i=0; i<3; i++) kf->P[i][i] = 1.0;       // Position variance
    for(int i=3; i<6; i++) kf->P[i][i] = 5.0;       // Velocity variance

    // 2. Initialize Q (Process Noise Covariance)
    // Higher Q: Less trust in the model/IMU, system reacts faster but noisier.
    // Lower Q: More trust in the model, system is smoother but may lag.
    double q_pos = 1e-10; // Very low noise for position integration
    double q_vel = 1e-6;  // Velocity noise (due to accelerometer vibration/bias)

    for(int i=0; i<3; i++) kf->Q[i][i] = q_pos;
    for(int i=3; i<6; i++) kf->Q[i][i] = q_vel;

    // 3. Initialize R (Measurement Noise Covariance)
    // GPS Position
    kf->R[0][0] = 1e-9; // Lat variance (~meters)
    kf->R[1][1] = 1e-9; // Lon variance (~meters)

    // Barometer Altitude (High trust in Barometer relative altitude)
    kf->R[2][2] = 0.05; // 0.05 m^2 (Barometer is typically smoother than GPS)
}

// ======================== Predict Step ========================
void GPS_KF_Predict(GPS_KF *kf,
                    double a_north_mps2, // North Acceleration (m/s^2)
                    double a_east_mps2,  // East Acceleration (m/s^2)
                    double a_up_mps2)    // Up Acceleration (m/s^2)
{
    double dt = kf->dt;
    double dt2 = 0.5 * dt * dt;

    // --- 1. Convert Acceleration (Input Control u) ---
    // a. Convert a_north -> a_lat (deg/s^2)
    double u_lat = a_north_mps2 / METERS_PER_DEG;

    // b. Convert a_east -> a_lon (deg/s^2)
    // Cos(lat) is needed to compensate for Earth's curvature
    double lat_rad = kf->x[0] * DEG_TO_RAD;
    double cos_lat = cos(lat_rad);
    if(cos_lat < 0.0001) cos_lat = 0.0001; // Avoid division by zero

    double u_lon = a_east_mps2 / (METERS_PER_DEG * cos_lat);

    // c. Vertical acceleration (m/s^2) - Keep as is
    double u_alt = a_up_mps2;

    // --- 2. Update State Estimate (x = Fx + Bu) ---
    // Pos = Pos + Vel*dt + 0.5*a*dt^2
    kf->x[0] += kf->x[3] * dt + u_lat * dt2; // Lat
    kf->x[1] += kf->x[4] * dt + u_lon * dt2; // Lon
    kf->x[2] += kf->x[5] * dt + u_alt * dt2; // Alt

    // Vel = Vel + a*dt
    kf->x[3] += u_lat * dt; // vLat
    kf->x[4] += u_lon * dt; // vLon
    kf->x[5] += u_alt * dt; // vAlt

    // --- 3. Update Error Covariance (P = FPF^T + Q) ---
    // Optimized matrix multiplication for the specific structure of F
    double P_temp[6][6];

    // Step A: Calculate F * P
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            P_temp[i][j] = kf->P[i][j];
            if (i < 3) {
                // Rows 0,1,2 have an additional term from rows 3,4,5 scaled by dt
                P_temp[i][j] += kf->dt * kf->P[i+3][j];
            }
        }
    }

    // Step B: Calculate (F * P) * F^T + Q
    // Multiplying by F^T is equivalent to adding columns
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            kf->P[i][j] = P_temp[i][j];
            if (j < 3) {
                kf->P[i][j] += P_temp[i][j+3] * kf->dt;
            }
            // Add Process Noise
            kf->P[i][j] += kf->Q[i][j];
        }
    }
}

// ======================== Update Step ========================
void GPS_KF_Update(GPS_KF *kf, double lat, double lon, double alt)
{
    // 1. Calculate Innovation: y = z - Hx
    // H selects only the first 3 elements (Position), so y = z - x_pos
    double y[3];
    y[0] = lat - kf->x[0];
    y[1] = lon - kf->x[1];
    y[2] = alt - kf->x[2];

    // 2. Calculate Innovation Covariance: S = H P H^T + R
    // Since H = [I 0], H P H^T is simply the top-left 3x3 block of P
    double S[3][3];
    for(int i=0; i<3; i++) {
        for(int j=0; j<3; j++) {
            S[i][j] = kf->P[i][j] + kf->R[i][j];
        }
    }

    // 3. Invert S
    double S_inv[3][3];
    mat3_inv(S, S_inv);

    // 4. Calculate Kalman Gain: K = P H^T S_inv
    // K size is 6x3.
    // P H^T corresponds to the first 3 columns of P
    double K[6][3];
    for(int i=0; i<6; i++) {       // 6 rows of K
        for(int j=0; j<3; j++) {   // 3 columns of K
            K[i][j] = 0;
            for(int k=0; k<3; k++) { // Multiply with S_inv 3x3
                K[i][j] += kf->P[i][k] * S_inv[k][j];
            }
        }
    }

    // 5. Update State Estimate: x = x + K * y
    for(int i=0; i<6; i++) {
        for(int j=0; j<3; j++) {
            kf->x[i] += K[i][j] * y[j];
        }
    }

    // 6. Update Error Covariance: P = (I - K H) P
    // Equivalent to: P = P - K * (H * P)
    // H * P is the first 3 rows of P

    // Temporary matrix to store K * (H * P)
    double P_decrement[6][6] = {0};

    for(int i=0; i<6; i++) {       // Rows of K
        for(int j=0; j<6; j++) {   // Columns of P
            for(int k=0; k<3; k++) { // Only run 3 times because H*P has only 3 rows of data
                P_decrement[i][j] += K[i][k] * kf->P[k][j];
            }
        }
    }

    // Subtract from old P
    for(int i=0; i<6; i++) {
        for(int j=0; j<6; j++) {
            kf->P[i][j] -= P_decrement[i][j];
        }
    }
}

// ======================== Coordinate Transformation ========================
// NOTE: Changed 'result' to pointer to return values
void GPS_KF_Convert_Acceleration(double a_x_meter, double a_y_meter, double a_z_meter,
                                 double pitch_deg, double roll_deg, double azi_deg, double R[3][3])
{
    // 1. Convert degree to radian
    double to_rad = M_PI / 180.0;

    double theta = pitch_deg * to_rad; // Pitch (Y axis)
    double phi   = roll_deg  * to_rad; // Roll (X axis)
    double psi   = azi_deg   * to_rad; // Yaw/Azimuth (Z axis)

    // 2. Calculate sin & cos
    double cTheta = cos(theta);
    double sTheta = sin(theta);

    double cPhi   = cos(phi);
    double sPhi   = sin(phi);

    double cPsi   = cos(psi);
    double sPsi   = sin(psi);

    // 3. Update Rotation Matrix (Body to ENU)
    // Row 0 (East Axis)
    R[0][0] = cTheta * cPsi;
    R[0][1] = sPhi * sTheta * cPsi - cPhi * sPsi;
    R[0][2] = cPhi * sTheta * cPsi + sPhi * sPsi;

    // Row 1 (North Axis)
    R[1][0] = cTheta * sPsi;
    R[1][1] = sPhi * sTheta * sPsi + cPhi * cPsi;
    R[1][2] = cPhi * sTheta * sPsi - sPhi * cPsi;

    // Row 2 (Up Axis)
    R[2][0] = -sTheta;
    R[2][1] = sPhi * cTheta;
    R[2][2] = cPhi * cTheta;

    // 4. Transform Acceleration
    // NOTE: Using input variables a_x_meter, a_y_meter, a_z_meter
    enu_acc.a_east  = R[0][0]*a_x_meter + R[0][1]*a_y_meter + R[0][2]*a_z_meter;
    enu_acc.a_north = R[1][0]*a_x_meter + R[1][1]*a_y_meter + R[1][2]*a_z_meter;
    enu_acc.a_up    = R[2][0]*a_x_meter + R[2][1]*a_y_meter + R[2][2]*a_z_meter;

    // 5. Gravity Compensation (Remove gravity from Up axis)
    //enu_acc->acc_up = result->acc_up - G_EARTH;
}

// ======================== Main Filter Interface ========================
void GPS_KF_Filter(GPS_KF *kf,
                   double meas_lat, double meas_lon, double meas_alt, int is_new_meas)
{
    // 1. Prediction Step: Always performed at every IMU cycle
    GPS_KF_Predict(kf, enu_acc.a_north, enu_acc.a_east, enu_acc.a_up);

    // 2. Update Step: Performed only when new measurement is available
    if (is_new_meas == 1) {
        GPS_KF_Update(kf, meas_lat, meas_lon, meas_alt);
    }
}
