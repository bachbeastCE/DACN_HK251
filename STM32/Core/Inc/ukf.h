/*
 * ukf.h
 *
 *  Created on: Oct 1, 2025
 *      Author: Admin
 */

#ifndef INC_UKF_H_
#define INC_UKF_H_
#include"imu.h"

#define L 6       // state number
#define N (2*L+1) // number of sigma points
typedef struct {
    float x[L];            // vector state x
    float P[L][L];         // covariance matrix P
    float Q[L][L];         // process noise Q
    float R[4][4];         // Measurement errors (a_x,a_y,a_z,g_x,g_y,g_z,m_x,m_y)

    float sigma[N][L];     // sigma points
    float W_a[N];          // weights for mean
    float W_c[N];          // weights for covariance

    float x_pred[L];       // state predict
    float P_pred[L][L];    // covariance predict

    float z_pred[N][4];    // measurement prediction
    float S[4][4];         // measurement covariance
    float Cxz[L][4];       // cross-covariance
    float K[L][4];         // Kalman gain
    float z[4];            // Measurement matrix
    float dt;              // delta time
} ukf_t;

extern ukf_t ukf;
void cholesky(float P[L][L], float A[L][L]);
void generate_sigma_points(ukf_t *ukf);
void ukfInit(ukf_t* ukf, I2C_HandleTypeDef *I2Cx1, I2C_HandleTypeDef *I2Cx2);
void ukf_predict(ukf_t *ukf, imu_t *imu);
void ukf_update(ukf_t *ukf, imu_t *imu);
int8_t invert4x4(float inv[4][4], float A[4][4]);
void ukf_filter(ukf_t *ukf, imu_t *imu);
#endif /* INC_UKF_H_ */
