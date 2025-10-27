#ifndef KALMAN_H_
#define KALMAN_H_

typedef struct {
    double x;           // State estimate
    double P;           // Error covariance
    double Q;           // Process noise covariance
    double R;           // Measurement noise covariance
} KalmanFilter;

void KalmanFilter_Init(KalmanFilter *kf, double Q, double R);
float KalmanFilter_Update(KalmanFilter *kf, double z);

#endif /* KALMAN_H_ */
