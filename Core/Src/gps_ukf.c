/*
 * gps_ukf.c
 *
 *  Created on: Nov 12, 2025
 *      Author: ASUS
 */

#include "gps_ukf.h"
#include "main.h"
#include <string.h>
#include <math.h>

ukf_gps_t ukf_gps;

// -------------------- Initialize UKF -------------------- //
void ukf_gps_Init(ukf_gps_t *ukf_gps){
    if(!ukf_gps) return;

    ukf_gps->dt = SAMPLING_TIME; // Sampling time

    // ===== Initialize weights =====
    ukf_gps->W_a[0] = LAMBDA / (L + LAMBDA);
    ukf_gps->W_c[0] = ukf_gps->W_a[0] + (1.0f - ALPHA*ALPHA + BETA);
    float w = 1.0f / (2.0f*(L+LAMBDA));
    for(int i=1;i<N;i++){
    	ukf_gps->W_a[i] = w;
    	ukf_gps->W_c[i] = w;
    }

    // ===== Process noise Q =====
    memset(ukf_gps->Q, 0, sizeof(ukf_gps->Q));
    ukf_gps->Q[0][0] = ukf_gps->Q[1][1] = ukf_gps->Q[2][2] = 0.01f; // position
    ukf_gps->Q[3][3] = ukf_gps->Q[4][4] = ukf_gps->Q[5][5] = 0.1f;  // velocity

    // ===== Measurement noise R =====
    memset(ukf_gps->R, 0, sizeof(ukf_gps->R));
    ukf_gps->R[0][0] = 4.0f;   // σ_x^2
    ukf_gps->R[1][1] = 4.0f;   // σ_y^2
    ukf_gps->R[2][2] = 9.0f;   // σ_z^2

    // ===== State x =====
    ukf_gps->x[0] = ukf_gps->x[1] = ukf_gps->x[2] = 0.0f;  // position
    ukf_gps->x[3] = ukf_gps->x[4] = ukf_gps->x[5] = 0.0f; // velocity

    // ===== Covariance P =====
    memset(ukf_gps->P, 0, sizeof(ukf_gps->P));
    ukf_gps->P[0][0] = ukf_gps->P[1][1] = ukf_gps->P[2][2] = 6.0f;
    ukf_gps->P[3][3] = ukf_gps->P[4][4] = ukf_gps->P[5][5] = 0.02f;

    // ===== Control input u =====
    ukf_gps->u[0] = ukf_gps->u[1] = ukf_gps->u[2] = 0.0f;
}

// ---------------- Cholesky decomposition ---------------- //
static void cholesky(float P[L][L], float A[L][L]){
    for(int i=0;i<L;i++){
        for(int j=0;j<=i;j++){
            float sum = P[i][j];
            for(int k=0;k<j;k++) sum -= A[i][k]*A[j][k];
            if(i==j) A[i][j] = sqrtf(fmaxf(sum,0.0f));
            else A[i][j] = sum / A[j][j];
        }
        for(int j=i+1;j<L;j++) A[i][j]=0.0f;
    }
}

// ---------------- Sigma Points Generation ---------------- //
static void generate_sigma_points(ukf_gps_t *ukf_gps){
    float A[L][L];
    cholesky(ukf_gps->P, A);

    memcpy(ukf_gps->sigma[0], ukf_gps->x, sizeof(float)*L);
    for(int j=0;j<L;j++){
        for(int i=0;i<L;i++){
        	ukf_gps->sigma[j+1][i] = ukf_gps->x[i] + GAMMA*A[i][j];
        	ukf_gps->sigma[L+1+j][i] = ukf_gps->x[i] - GAMMA*A[i][j];
        }
    }
}

// ----------------- UKF Predict Step ----------------- //
static void ukf_gps_predict(ukf_gps_t *ukf_gps){
    generate_sigma_points(ukf_gps);

    for(int j=0;j<N;j++){
        // Position
    	ukf_gps->sigma[j][0] += ukf_gps->sigma[j][3]*ukf_gps->dt + 0.5f*ukf_gps->u[0]*ukf_gps->dt*ukf_gps->dt;
    	ukf_gps->sigma[j][1] += ukf_gps->sigma[j][4]*ukf_gps->dt + 0.5f*ukf_gps->u[1]*ukf_gps->dt*ukf_gps->dt;
    	ukf_gps->sigma[j][2] += ukf_gps->sigma[j][5]*ukf_gps->dt + 0.5f*ukf_gps->u[2]*ukf_gps->dt*ukf_gps->dt;
        // Velocity
    	ukf_gps->sigma[j][3] += ukf_gps->u[0]*ukf_gps->dt;
    	ukf_gps->sigma[j][4] += ukf_gps->u[1]*ukf_gps->dt;
    	ukf_gps->sigma[j][5] += ukf_gps->u[2]*ukf_gps->dt;
    }

    for(int i=0;i<L;i++){
    	ukf_gps->x_pred[i] = 0.0f;
        for(int j=0;j<N;j++) ukf_gps->x_pred[i] += ukf_gps->W_a[j]*ukf_gps->sigma[j][i];
    }

    for(int i=0;i<L;i++){
        for(int k=0;k<L;k++){
        	ukf_gps->P_pred[i][k] = ukf_gps->Q[i][k];
            for(int j=0;j<N;j++){
                float dx_i = ukf_gps->sigma[j][i]-ukf_gps->x_pred[i];
                float dx_k = ukf_gps->sigma[j][k]-ukf_gps->x_pred[k];
                ukf_gps->P_pred[i][k] += ukf_gps->W_c[j]*dx_i*dx_k;
            }
        }
    }
}

// ----------------- UKF Update Step ----------------- //
static int8_t invert3x3(float inv[3][3], float A[3][3]){
    float det = A[0][0]*(A[1][1]*A[2][2]-A[1][2]*A[2][1])
              - A[0][1]*(A[1][0]*A[2][2]-A[1][2]*A[2][0])
              + A[0][2]*(A[1][0]*A[2][1]-A[1][1]*A[2][0]);
    if(fabs(det)<1e-6f) return -1;
    float invdet = 1.0f/det;

    inv[0][0] =  (A[1][1]*A[2][2]-A[1][2]*A[2][1])*invdet;
    inv[0][1] = -(A[0][1]*A[2][2]-A[0][2]*A[2][1])*invdet;
    inv[0][2] =  (A[0][1]*A[1][2]-A[0][2]*A[1][1])*invdet;
    inv[1][0] = -(A[1][0]*A[2][2]-A[1][2]*A[2][0])*invdet;
    inv[1][1] =  (A[0][0]*A[2][2]-A[0][2]*A[2][0])*invdet;
    inv[1][2] = -(A[0][0]*A[1][2]-A[0][2]*A[1][0])*invdet;
    inv[2][0] =  (A[1][0]*A[2][1]-A[1][1]*A[2][0])*invdet;
    inv[2][1] = -(A[0][0]*A[2][1]-A[0][1]*A[2][0])*invdet;
    inv[2][2] =  (A[0][0]*A[1][1]-A[0][1]*A[1][0])*invdet;
    return 0;
}

static void ukf_gps_update(ukf_gps_t *ukf_gps, enu_t *enu){
    if(!ukf_gps || !enu) return;

    // Sigma points in measurement space
    for(int j=0;j<N;j++){
    	ukf_gps->z_pred[j][0] = ukf_gps->sigma[j][0];
    	ukf_gps->z_pred[j][1] = ukf_gps->sigma[j][1];
    	ukf_gps->z_pred[j][2] = ukf_gps->sigma[j][2];
    }

    // Predicted measurement mean
    float z_mean[3]={0};
    for(int i=0;i<3;i++)
        for(int j=0;j<N;j++) z_mean[i] += ukf_gps->W_a[j]*ukf_gps->z_pred[j][i];

    // Measurement covariance S
    for(int i=0;i<3;i++){
        for(int k=0;k<3;k++){
        	ukf_gps->S[i][k] = ukf_gps->R[i][k];
            for(int j=0;j<N;j++){
                float dz_i = ukf_gps->z_pred[j][i]-z_mean[i];
                float dz_k = ukf_gps->z_pred[j][k]-z_mean[k];
                ukf_gps->S[i][k] += ukf_gps->W_c[j]*dz_i*dz_k;
            }
        }
    }

    // Cross covariance
    for(int i=0;i<L;i++){
        for(int k=0;k<3;k++){
        	ukf_gps->Cxz[i][k]=0;
            for(int j=0;j<N;j++){
                float dx = ukf_gps->sigma[j][i]-ukf_gps->x_pred[i];
                float dz = ukf_gps->z_pred[j][k]-z_mean[k];
                ukf_gps->Cxz[i][k] += ukf_gps->W_c[j]*dx*dz;
            }
        }
    }

    float S_inv[3][3];
    if(invert3x3(S_inv, ukf_gps->S)<0) return; // S gần nghịch đảo

    // Kalman gain
    for(int i=0;i<L;i++){
        for(int k=0;k<3;k++){
        	ukf_gps->K[i][k]=0;
            for(int j=0;j<3;j++) ukf_gps->K[i][k] += ukf_gps->Cxz[i][j]*S_inv[j][k];
        }
    }

    // Measurement
    ukf_gps->z[0]=enu->x; ukf_gps->z[1]=enu->y; ukf_gps->z[2]=enu->z;

    // Update state
    for(int i=0;i<L;i++){
        float corr=0;
        for(int k=0;k<3;k++) corr += ukf_gps->K[i][k]*(ukf_gps->z[k]-z_mean[k]);
        ukf_gps->x[i] = ukf_gps->x_pred[i]+corr;
    }

    // Update covariance
    float KS[L][3];
    for(int i=0;i<L;i++)
        for(int j=0;j<3;j++){
            KS[i][j]=0;
            for(int k=0;k<3;k++) KS[i][j] += ukf_gps->K[i][k]*ukf_gps->S[k][j];
        }

    for(int i=0;i<L;i++)
        for(int j=0;j<L;j++){
            float val = ukf_gps->P_pred[i][j];
            for(int k=0;k<3;k++) val -= KS[i][k]*ukf_gps->K[j][k];
            ukf_gps->P[i][j] = val;
        }
}

// ----------------- UKF Filter Step ----------------- //
void ukf_gps_filter(ukf_gps_t *ukf, enu_t *enu, float* acc){
    if(!ukf || !enu || !acc) return;

    ukf->u[0]=acc[0]; ukf->u[1]=acc[1]; ukf->u[2]=acc[2];
    ukf_gps_predict(ukf);
    ukf_gps_update(ukf, enu);
}
