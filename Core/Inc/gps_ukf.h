/*
 * gps_ukf.h
 *
 *  Created on: Nov 12, 2025
 *      Author: ASUS
 */

#ifndef INC_UKF_H_
#define INC_UKF_H_

#include "imu.h"
#include <math.h>

// ==================== UKF_GPS CONFIG ==================== //
#define L 6               // State dimension (px,py,pz,vx,vy,vz)
#define N (2*L + 1)       // Number of sigma points

// --- Unscented Transform Parameters ---
#define ALPHA   (1e-3f)   // Spread of sigma points (1e-3 ~ 1e-1)
#define BETA    (2.0f)    // For Gaussian distributions, beta = 2 is optimal
#define KAPPA   (0.0f)    // Secondary scaling parameter
#define LAMBDA  (ALPHA*ALPHA*((float)L + KAPPA) - (float)L)
#define GAMMA   (sqrtf((float)L + LAMBDA))

#define SAMPLING_TIME 0.04f

// ==================================================== //

typedef struct{
	float x;
	float y;
	float z;
} enu_t;

typedef struct {
    // ----- State and Covariances -----
    float x[L];
    float P[L][L];
    float Q[L][L];
    float R[3][3];

    // ----- Control input -----
    float u[3];          // control input: accel [ax, ay, az]

    // ----- Sigma Points and Weights -----
    float sigma[N][L];
    float W_a[N];
    float W_c[N];

    // ----- Predicted State & Covariance -----
    float x_pred[L];
    float P_pred[L][L];

    // ----- Measurement-related -----
    float z_pred[N][3];
    float z[3];
    float S[3][3];
    float Cxz[L][3];
    float K[L][3];

    // ----- System -----
    float dt;
} ukf_gps_t;

// ======== Function Prototypes ======================= //
extern ukf_gps_t ukf_gps;

void ukf_gps_Init(ukf_gps_t *ukf);
void ukf_gps_filter(ukf_gps_t *ukf, enu_t *enu, float* acc);

#endif /* INC_UKF_H_ */
