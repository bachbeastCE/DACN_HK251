/*
 * ukf.c
 *
 *  Created on: Sep 29, 2025
 *      Author: Admin
 */
#include "ukf.h"
#include "main.h"

ukf_t ukf;
static inline float wrap180(float a)
{
    while (a > 180.0f) a -= 360.0f;
    while (a < -180.0f) a += 360.0f;
    return a;
}
void ukfInit(ukf_t *ukf, I2C_HandleTypeDef *I2Cx){
	imu_init(I2Cx);
	ukf->W_a[0] = 0.33333f;
	ukf->W_c[0] = 0.33333f;
    ukf->dt = 0.02f;

    const float alpha = 0.5f;
    const float beta  = 2.0f;
    const float kappa = 2.0f;

//    const float L = 6.0f;
    const float lambda = alpha*alpha * (L + kappa) - L;

    ukf->W_a[0] = lambda / (L + lambda);
    ukf->W_c[0] = ukf->W_a[0] + (1.0f - alpha*alpha + beta);

    for (int i = 1; i < 2*L + 1; i++) {
        ukf->W_a[i] = 1.0f / (2.0f * (L + lambda));
        ukf->W_c[i] = ukf->W_a[i];
    }
//    for(int i = 0; i < N; i++){
//    	Serial_Print("W_c[%d] = %.5f\n", i, ukf->W_c[i]);
//    	Serial_Print("W_a[%d] = %.5f\n", i, ukf->W_a[i]);
//    }

	//Init Q



    ukf->Q[0][0] = (imu.goffsetx / 65.5) * (imu.goffsetx / 65.5) * ukf->dt * ukf->dt + ((imu.gyrox_noise / 65.5f) * (imu.gyrox_noise / 65.5f)) * ukf->dt * ukf->dt;
    ukf->Q[1][1] = (imu.goffsety / 65.5) * (imu.goffsety / 65.5) * ukf->dt * ukf->dt + ((imu.gyroy_noise / 65.5f) * (imu.gyroy_noise / 65.5f)) * ukf->dt * ukf->dt;
	ukf->Q[2][2] = (imu.goffsetz / 65.5) * (imu.goffsetz / 65.5) * ukf->dt * ukf->dt + ((imu.gyroz_noise / 65.5f) * (imu.gyroz_noise / 65.5f)) * ukf->dt * ukf->dt;
	ukf->Q[3][3] = (imu.goffsetx / 65.5) * (imu.goffsetx / 65.5) + ((imu.gyrox_noise / 65.5f) * (imu.gyrox_noise / 65.5f));
	ukf->Q[4][4] = (imu.goffsety / 65.5) * (imu.goffsety / 65.5) + ((imu.gyroy_noise / 65.5f) * (imu.gyroy_noise / 65.5f));
	ukf->Q[5][5] = (imu.goffsetz / 65.5) * (imu.goffsetz / 65.5) + ((imu.gyroz_noise / 65.5f) * (imu.gyroz_noise / 65.5f));

	for(int i = 0; i < L; i++){
		for(int j = 0; j < L; j++){
			if(i != j) ukf->Q[i][j] = 0.0f;
		}
	}
	Serial_Print("Q =\n");
	for(int i = 0; i < L; i++){
        for(int j = 0; j < L; j++){
            Serial_Print("%.5f ", ukf->Q[i][j]);
        }
        Serial_Print("\n");
    }

	//Init R
	ukf->R[0][0] = 0.01f;
	ukf->R[1][1] = 0.01f;
	ukf->R[2][2] = 0.05f;
	ukf->R[3][3] = 64.0f;
	for(int i = 0; i < 4; ++i){
		for(int j = 0; j < 4; ++j){
			if(i != j) ukf->R[i][j] = 0.0f;
		}
	}
//	 Serial_Print("R =\n");
//	 for(int i = 0; i < 4; i++){
//	        for(int j = 0; j < 4; j++){
//	            Serial_Print("%.5f ", ukf->R[i][j]);
//	        }
//	        Serial_Print("\n");
//	    }

	//Init x
	ukf->x[0] = 1.15f;
	ukf->x[1] = -0.57f;
	ukf->x[2] = 2.86f;
	ukf->x[3] = 0.57f;
	ukf->x[4] = -0.57f;
	ukf->x[5] = 0.29f;
//	Serial_Print("x = [");
//		for(int i = 0; i < L; i++){
//			Serial_Print("%.5f ", ukf->x[i]);
//		}
//		Serial_Print("]\n");


	//Init P
	ukf->P[0][0] = 11.46f;
	ukf->P[1][1] = 11.46f;
	ukf->P[2][2] = 17.19f;
	ukf->P[3][3] = 0.02f;
	ukf->P[4][4] = 0.02f;
	ukf->P[5][5] = 0.02f;
	for(int i = 0; i < L; i++){
		for(int j = 0; j < L; j++){
			if(i != j) ukf->P[i][j] = 0.0f;
		}
	}
//	Serial_Print("P =\n");
//	for(int i = 0; i < L; i++){
//        for(int j = 0; j < L; j++){
//            Serial_Print("%.5f ", ukf->P[i][j]);
//        }
//        Serial_Print("\n");
//    }
}

//-------------------------------------------------
// Cholesky decomposition helper
void cholesky(float P[L][L], float A[L][L]) {
    const float eps = 1e-9f;
    for(int i = 0; i < L; i++){
        for(int j = 0; j <= i; j++){
            float sum = P[i][j];
            for(int k = 0; k < j; k++) sum -= A[i][k] * A[j][k];
            if (i == j) {
                // guard, force PD
                A[i][j] = sqrtf(fmaxf(sum, eps));
            } else {
                float denom = A[j][j];
                if (denom < eps) denom = eps;
                A[i][j] = sum / denom;
            }
        }
        // zero upper triangle
        for(int j = i + 1; j < L; j++) A[i][j] = 0.0f;
    }
}

//-------------------------------------------------
// Sigma points generation
//void generate_sigma_points(ukf_t *ukf) {
//    float A[L][L];
//    cholesky(ukf->P, A);
////    Serial_Print("A = \n");
////    for(int i = 0; i < L; i++){
////        for(int j = 0; j < L; j++){
////            Serial_Print("%.6f ", A[i][j]);
////        }
////        Serial_Print("\n");
////    }
//
//    float scale = 3;
//    memcpy(ukf->sigma[0], ukf->x, sizeof(float)*L); // sigma0
//
//    for(int j = 0; j < L; j++){
//        for(int i = 0; i < L; i++){
//            ukf->sigma[j + 1][i] = ukf->x[i] + scale*A[i][j];
//            ukf->sigma[L + 1 + j][i] = ukf->x[i] - scale*A[i][j];
//        }
//    }
////    Serial_Print("Sigma points:\n");
////    for(int j = 0; j < N; j++){
////        Serial_Print("sigma[%d] = [", j);
////        for(int i = 0; i < L; i++){
////            Serial_Print("%.6f", ukf->sigma[j][i]);
////            if(i < L-1) Serial_Print(", ");
////        }
////        Serial_Print("]\n");
////    }
//}

void generate_sigma_points(ukf_t *ukf) {
    float A[L][L];
    cholesky(ukf->P, A);

    const float alpha = 0.5f;
    const float kappa = 2.0f;
    float lambda = alpha*alpha*(L + kappa) - L;
    float scale = sqrtf(fmaxf(L + lambda, 1e-12f));

    memcpy(ukf->sigma[0], ukf->x, sizeof(float)*L);

    for(int j = 0; j < L; j++){
        for(int i = 0; i < L; i++){
            ukf->sigma[j + 1][i]     = ukf->x[i] + scale*A[i][j];
            ukf->sigma[L + 1 + j][i] = ukf->x[i] - scale*A[i][j];
        }
    }
}

//-------------------------------------------------
// Predict step (state transition using gyro)
void ukf_predict(ukf_t *ukf, imu_t *imu) {
    generate_sigma_points(ukf);

    // propagate sigma points through state model
    for(int j = 0; j < N; ++j){
        // roll/pitch/yaw update with gyro minus bias
        ukf->sigma[j][0] += (imu->gx - ukf->sigma[j][3]) * ukf->dt; // roll
        ukf->sigma[j][1] += (imu->gy - ukf->sigma[j][4]) * ukf->dt; // pitch
        ukf->sigma[j][2] += (imu->gz - ukf->sigma[j][5]) * ukf->dt; // yaw
    }

    // compute predicted mean
    for(int i = 0;i < L; ++i){
        ukf->x_pred[i] = 0;
        for(int j = 0; j < N; j++){
            ukf->x_pred[i] += ukf->W_a[j]*ukf->sigma[j][i];
        }
    }
    while (ukf->x_pred[0] > 180.0f) ukf->x_pred[0] -= 360.0f;
    while (ukf->x_pred[0] < -180.0f) ukf->x_pred[0] += 360.0f;

    // pitch: clamp [-90, 90]
    if (ukf->x_pred[1] > 90.0f) ukf->x_pred[1] = 90.0f;
    if (ukf->x_pred[1] < -90.0f) ukf->x_pred[1] = -90.0f;

    // yaw: [-180, 180]
    while (ukf->x_pred[2] > 180.0f) ukf->x_pred[2] -= 360.0f;
    while (ukf->x_pred[2] < -180.0f) ukf->x_pred[2] += 360.0f;


    // compute predicted covariance
    for(int i = 0; i < L; ++i){
        for(int k = 0; k < L; ++k){
            ukf->P_pred[i][k] = ukf->Q[i][k]; // start with process noise
            for(int j = 0; j < N; ++j){
                float dx_i = ukf->sigma[j][i] - ukf->x_pred[i];
                float dx_k = ukf->sigma[j][k] - ukf->x_pred[k];
                ukf->P_pred[i][k] += ukf->W_c[j] * dx_i * dx_k;
            }
        }
    }
}

//-------------------------------------------------


// Update step (measurement: gyro delta)
void ukf_update(ukf_t *ukf, imu_t *imu) {
    //Transform sigma points to measurement space
	//const float G = 9.80665f;
	for (int j = 0; j < N; ++j) {
        float roll_j  = ukf->sigma[j][0] * PI / 180.0f;
        float pitch_j = ukf->sigma[j][1] * PI / 180.0f;
        // predict accelerometer (gravity only) in body frame from orientation
        float ax_p =  -sinf(pitch_j);
        float ay_p =  sinf(roll_j) * cosf(pitch_j);
        float az_p =  cosf(roll_j) * cosf(pitch_j);

        ukf->z_pred[j][0] = ax_p;
        ukf->z_pred[j][1] = ay_p;
        ukf->z_pred[j][2] = az_p;
        ukf->z_pred[j][3] = wrap180(ukf->sigma[j][2]); // yaw
    }


    //Compute mean z
    float z_mean[4] = {0};
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < N; ++j) {
            z_mean[i] += ukf->W_a[j] * ukf->z_pred[j][i];
        }
    }

    float sin_sum = 0.0f, cos_sum = 0.0f;
    for (int j = 0; j < N; ++j) {
        float yaw = ukf->z_pred[j][3] * PI / 180.0f;
        sin_sum += ukf->W_a[j] * sinf(yaw);
        cos_sum += ukf->W_a[j] * cosf(yaw);
    }
    z_mean[3] = atan2f(sin_sum, cos_sum) * 180.0f / PI;

    //Compute measurement covariance S
    for (int i = 0; i < 4; ++i) {
    	for (int k = 0; k < 4; ++k) {
			ukf->S[i][k] = ukf->R[i][k];
			for (int j = 0; j < N; ++j) {
				float dzi = (i == 3) ?
							wrap180(ukf->z_pred[j][3] - z_mean[3]) :
							(ukf->z_pred[j][i] - z_mean[i]);

				float dzk = (k == 3) ?
							wrap180(ukf->z_pred[j][3] - z_mean[3]) :
							(ukf->z_pred[j][k] - z_mean[k]);

				ukf->S[i][k] += ukf->W_c[j] * dzi * dzk;
			}
		}
	}


    for(int i=0;i<4;i++) {
        for(int j=0;j<4;j++)
            ukf->S[i][j] = 0.5f*(ukf->S[i][j] + ukf->S[j][i]);
        ukf->S[i][i] += 1e-6f;
    }

    //Compute cross covariance Cxz
    for (int i = 0; i < L; ++i) {
    	for (int k = 0; k < 4; ++k) {
			ukf->Cxz[i][k] = 0;
			for (int j = 0; j < N; ++j) {
				float dx = ukf->sigma[j][i] - ukf->x_pred[i];
				if (i == 2) dx = wrap180(dx); // yaw state

				float dz = (k == 3) ?
						   wrap180(ukf->z_pred[j][3] - z_mean[3]) :
						   (ukf->z_pred[j][k] - z_mean[k]);

				ukf->Cxz[i][k] += ukf->W_c[j] * dx * dz;
			}
		}
	}



    //Kalman gain K = Cxz * S^-1
    float S_inv[4][4];
    if (invert4x4(S_inv, ukf->S) < 0) {
    	Serial_Print("loi day ne %d\n", invert4x4(S_inv, ukf->S));
        ukf->x[1] = 10000;
        ukf->x[0] = 10000;
        ukf->x[3] = 10000;
        return;
    }
    for (int i = 0; i < L; i++) {
        for (int k = 0; k < 4; k++) {
            ukf->K[i][k] = 0;
            for (int j = 0; j < 4; j++) {
                ukf->K[i][k] += ukf->Cxz[i][j] * S_inv[j][k];
            }
        }
    }

    //Update state: x = x_pred + K*(z - z_mean)
    float norm = sqrtf(imu->ax*imu->ax + imu->ay*imu->ay + imu->az*imu->az);
    ukf->z[0] = imu->ax / norm;
    ukf->z[1] = imu->ay / norm;
    ukf->z[2] = imu->az / norm;
    ukf->z[3] = wrap180(imu->yaw);

    for (int i = 0; i < L; ++i) {
    	float correction = 0;
		for (int k = 0; k < 4; ++k) {
			float innov = (k == 3) ?
						  wrap180(ukf->z[3] - z_mean[3]) :
						  (ukf->z[k] - z_mean[k]);
			correction += ukf->K[i][k] * innov;
		}
		ukf->x[i] = ukf->x_pred[i] + correction;
		if (i == 2) ukf->x[i] = wrap180(ukf->x[i]); // yaw
	}


//    Serial_Print("pitch = %.3f degree; ", ukf->x[1]);
//    Serial_Print("yaw = %.3f degree; ", ukf->x[2]);
//    Serial_Print("roll = %.3f degree; \n", ukf->x[0]);
    //Update covariance: P = P_pred - K*S*K^T
    float KS[L][4];
    for (int i = 0; i < L; i++) {
        for (int j = 0; j < 4; j++) {
            KS[i][j] = 0;
            for (int k = 0; k < 4; k++) {
                KS[i][j] += ukf->K[i][k] * ukf->S[k][j];
            }
        }
    }

    for (int i = 0; i < L; i++) {
        for (int j = 0; j < L; j++) {
            float val = ukf->P_pred[i][j];
            for (int k = 0; k < 4; k++) {
                val -= KS[i][k] * ukf->K[j][k];
            }
            ukf->P[i][j] = val;
        }
    }
    while (ukf->x[0] > 180.0f) ukf->x[0] -= 360.0f;
    while (ukf->x[0] < -180.0f) ukf->x[0] += 360.0f;

    // clamp pitch into [-90, 90]
    ukf->x[0] = wrap180(ukf->x[0]);      // roll
    if (ukf->x[1] >  90.0f) ukf->x[1] =  90.0f;
    if (ukf->x[1] < -90.0f) ukf->x[1] = -90.0f;
    ukf->x[2] = wrap180(ukf->x[2]);      // yaw
}

int8_t invert4x4(float inv[4][4], float A[4][4]) {
    float det;
    float a = A[0][0], b = A[0][1], c = A[0][2], d = A[0][3];
    float e = A[1][0], f = A[1][1], g = A[1][2], h = A[1][3];
    float i = A[2][0], j = A[2][1], k = A[2][2], l = A[2][3];
    float m = A[3][0], n = A[3][1], o = A[3][2], p = A[3][3];

    // Calculate determinant
    det = a*(f*(k*p - o*l) - g*(j*p - n*l) + h*(j*o - n*k))
        - b*(e*(k*p - o*l) - g*(i*p - m*l) + h*(i*o - m*k))
        + c*(e*(j*p - n*l) - f*(i*p - m*l) + h*(i*n - m*j))
        - d*(e*(j*o - n*k) - f*(i*o - m*k) + g*(i*n - m*j));

    if (fabs(det) < 1e-6f) return -1;

    float invdet = 1.0f / det;

    inv[0][0] =  (f*(k*p - o*l) - g*(j*p - n*l) + h*(j*o - n*k)) * invdet;
    inv[0][1] = -(b*(k*p - o*l) - c*(j*p - n*l) + d*(j*o - n*k)) * invdet;
    inv[0][2] =  (b*(g*p - o*h) - c*(f*p - n*h) + d*(f*o - n*g)) * invdet;
    inv[0][3] = -(b*(g*l - k*h) - c*(f*l - j*h) + d*(f*k - j*g)) * invdet;

    inv[1][0] = -(e*(k*p - o*l) - g*(i*p - m*l) + h*(i*o - m*k)) * invdet;
    inv[1][1] =  (a*(k*p - o*l) - c*(i*p - m*l) + d*(i*o - m*k)) * invdet;
    inv[1][2] = -(a*(g*p - o*h) - c*(e*p - m*h) + d*(e*o - m*g)) * invdet;
    inv[1][3] =  (a*(g*l - k*h) - c*(e*l - i*h) + d*(e*k - i*g)) * invdet;

    inv[2][0] =  (e*(j*p - n*l) - f*(i*p - m*l) + h*(i*n - m*j)) * invdet;
    inv[2][1] = -(a*(j*p - n*l) - b*(i*p - m*l) + d*(i*n - m*j)) * invdet;
    inv[2][2] =  (a*(f*p - n*h) - b*(e*p - m*h) + d*(e*n - m*f)) * invdet;
    inv[2][3] = -(a*(f*l - j*h) - b*(e*l - i*h) + d*(e*j - i*f)) * invdet;

    inv[3][0] = -(e*(j*o - n*k) - f*(i*o - m*k) + g*(i*n - m*j)) * invdet;
    inv[3][1] =  (a*(j*o - n*k) - b*(i*o - m*k) + c*(i*n - m*j)) * invdet;
    inv[3][2] = -(a*(f*o - n*g) - b*(e*o - m*g) + c*(e*n - m*f)) * invdet;
    inv[3][3] =  (a*(f*k - j*g) - b*(e*k - i*g) + c*(e*j - i*f)) * invdet;

    return 0;
}

void ukf_filter(ukf_t *ukf, imu_t *imu) {
    ukf_predict(ukf, imu);
    ukf_update(ukf, imu);

//#if  IMU_UKF_ENABLE_SERIAL_LOG
    Serial_Print("pitch_update = %.3f degree; ", ukf->x[1]);
    Serial_Print("yaw_update = %.3f degree; ", ukf->x[2]);
    Serial_Print("roll_update = %.3f degree; \n", ukf->x[0]);
    Serial_Print("#\n");
//#endif
}
