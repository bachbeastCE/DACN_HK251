/*
 * gps_kf.h
 *
 *  Created on: Nov 20, 2025
 *      Author: ASUS
 */

#include "gps_kf.h"
#include <string.h>
#include <math.h>

GPS_KF kf_gps;

// ======================== Matrix utils ========================
static void mat3_inv(double A[3][3], double inv[3][3])
{
    double a = A[0][0], b = A[0][1], c = A[0][2];
    double d = A[1][0], e = A[1][1], f = A[1][2];
    double g = A[2][0], h = A[2][1], i = A[2][2];

    double det = a*(e*i - f*h) - b*(d*i - f*g) + c*(d*h - e*g);
    if(fabs(det) < 1e-12) det = 1e-12;  // tránh chia 0

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

// ======================== KF init ========================
void GPS_KF_Init(GPS_KF *kf)
{
    memset(kf, 0, sizeof(GPS_KF));

    // Covariance khởi tạo
    for(int i=0;i<3;i++) kf->P[i][i] = 1.0;

    // Process noise (Tăng = phản ứng nhanh)
    kf->Q[0][0] =  5e-11;
    kf->Q[1][1] =  4e-11;
    kf->Q[2][2] = 0.001;

    // Process noise (Càng lớn = Mượt hơn)
    kf->R[0][0] = kf->R[1][1] = 1e-7;
    kf->R[2][2] = 16; // alt m²

    kf->dt = 0.5;
}

// ======================== Predict ========================
void GPS_KF_Predict(GPS_KF *kf,
                    double a_lat_mps2,
                    double a_lon_mps2,
                    double a_alt,
                    double current_lat_deg)
{
// 	Không có mô hình động học, loc nhiễu đơn thuần
//    double dt2 = 0.5 * kf->dt * kf->dt;
//
//    // --- Chuyển gia tốc từ m/s² sang deg/s² ---
//    double lat_rad = current_lat_deg * M_PI / 180.0;
//
//    double a_lat_deg = a_lat_mps2 / 111111.0;
//    double a_lon_deg = a_lon_mps2 / (111111.0 * cos(lat_rad));
//
//    // --- Cập nhật trạng thái (position only, velocity bỏ) ---
//    kf->x[0] += a_lat_deg * dt2;
//    kf->x[1] += a_lon_deg * dt2;
//    kf->x[2] += a_alt * dt2;

    // --- Cập nhật ma trận hiệp phương sai ---
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
            kf->P[i][j] += kf->Q[i][j];
}

// ======================== Update ========================
void GPS_KF_Update(GPS_KF *kf, double lat, double lon, double alt)
{
    // Lấy đo
    double z[3] = {lat, lon, alt};

    // Sai số đo: y = z - x
    double y[3] = { z[0]-kf->x[0], z[1]-kf->x[1], z[2]-kf->x[2] };

    // S = P + R (vì H = I)
    double S[3][3];
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
            S[i][j] = kf->P[i][j] + kf->R[i][j];

    // Invert S
    double S_inv[3][3];
    mat3_inv(S,S_inv);

    // ======================== Kalman Gain ========================
    double K[3][3] = {0};
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
            for(int k=0;k<3;k++)
                K[i][j] += kf->P[i][k] * S_inv[k][j];
    // ======================== End Kalman Gain ========================

    // Cập nhật trạng thái: x = x + K * y
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
            kf->x[i] += K[i][j] * y[j];

    // Cập nhật hiệp phương sai: P = (I-K)*P
    double I_K[3][3];
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
            I_K[i][j] = (i==j ? 1.0 : 0.0) - K[i][j];

    double newP[3][3] = {0};
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
            for(int k=0;k<3;k++)
                newP[i][j] += I_K[i][k] * kf->P[k][j];

    memcpy(kf->P, newP, sizeof(newP));
}

// ======================== Filter 1 chu kì ========================
void GPS_KF_Filter(GPS_KF *kf, double meas_lat, double meas_lon, double meas_alt,
                   double a_lat, double a_lon, double a_alt)
{
    // Predict
    GPS_KF_Predict(kf, a_lat, a_lon, a_alt,kf->x[0]);

    // Update
    GPS_KF_Update(kf, meas_lat, meas_lon, meas_alt);
}
