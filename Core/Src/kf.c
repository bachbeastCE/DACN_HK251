#include "kf.h"

void KalmanFilter_Init(KalmanFilter *kf, double Q, double R) {
    kf->x = 0.0f;      // Initial state estimate
    kf->P = 1.0f;      // Initial error covariance
    kf->Q = Q;         // Process noise covariance
    kf->R = R;         // Measurement noise covariance
}

float KalmanFilter_Update(KalmanFilter *kf, double z) {
    // Predicti on update
    double x_pred = kf->x;
    double P_pred = kf->P + kf->Q;

    // Measurement update (Kalman gain calculation)
    float K = P_pred / (P_pred + kf->R);

    // State update
    kf->x = x_pred + K * (z - x_pred);

    // Error covariance update
    kf->P = (1 - K) * P_pred;

    return kf->x;   // Return the filtered value
}/*
 * kf.c
 *
 *  Created on: Oct 16, 2025
 *      Author: ASUS
 */


