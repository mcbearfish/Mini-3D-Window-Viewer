/*
 * File: kalman_filter.h
 * Purpose: Public interface for the compact attitude estimator used to convert
 * raw IMU readings into a stable orientation quaternion.
 *
 * Functions in this file:
 * - EKF_Init: initializes the orientation filter state.
 * - EKF_SetReference: sets the startup gravity and magnetic reference vectors.
 * - EKF_Predict: advances the orientation estimate using gyro input.
 * - EKF_Update: corrects the estimate using accel and magnetometer data.
 */
#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

#include <stdint.h>

/* Compact EKF-like attitude estimator state used by the main loop. */
typedef struct {
    float x_hat_data[4];    // Current orientation estimate stored as a quaternion.
    float gyro_bias[3];     // Estimated gyroscope bias on each axis.
    float error_cov[3];     // Covariance/error estimate for the three correction axes.
    float gravity_ref[3];   // Reference gravity direction captured during startup calibration.
    float mag_ref[3];       // Reference magnetic field direction captured during startup calibration.
    float filtered_accel[3];// Low-pass filtered accelerometer vector used during correction.
    float filtered_mag[3];  // Low-pass filtered magnetometer vector used during correction.
    float mag_ref_norm;     // Magnitude of the startup magnetometer reference vector.
    float last_gyro_norm;   // Magnitude of the most recent gyro sample, used as a motion indicator.
    uint8_t use_mag;        // Flag indicating whether magnetometer correction should be used.
} EKF_TypeDef;

void EKF_Init(EKF_TypeDef *ekf);
void EKF_SetReference(EKF_TypeDef *ekf, float ax, float ay, float az, float mx, float my, float mz);
void EKF_Predict(EKF_TypeDef *ekf, float gx, float gy, float gz, float dt);
void EKF_Update(EKF_TypeDef *ekf, float ax, float ay, float az, float mx, float my, float mz);

#endif // KALMAN_FILTER_H
