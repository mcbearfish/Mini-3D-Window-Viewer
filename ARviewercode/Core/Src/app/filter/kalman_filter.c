/*----------------------------------------------------------------------------------------*/
/*
 * File:    kalman_filter.c
 * Purpose: Orientation estimator that fuses gyroscope, accelerometer, and optional
 *          magnetometer data (if available) into a stable quaternion for the demo.
 *
 * Functions contained in this file:
 * - normalize_quat: Normalizes a quaternion.
 * - normalize_vec3: Normalizes a 3D vector and returns its original magnitude.
 * - dot3: Computes a 3D dot product.
 * - clampf: Clamps a float to a range.
 * - cross3: Computes a 3D cross product.
 * - rotate_world_to_body: Rotates a world vector into the body frame.
 * - apply_small_angle: Applies a small-angle correction to the quaternion.
 * - EKF_Init: Resets the filter to its default state.
 * - EKF_SetReference: Loads startup gravity and magnetic reference vectors.
 * - EKF_Predict: Predicts the next orientation from gyro motion.
 * - EKF_Update: Corrects the orientation using accel and magnetometer data.
 */
/*----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------*/
// INCLUDES
/*----------------------------------------------------------------------------------------*/
#include "kalman_filter.h"
#include <math.h>
#include <string.h>


/*----------------------------------------------------------------------------------------*/
/* Function: normalize_quat
 * Purpose:  Normalize a quaternion stored as a float array so it remains a valid unit
 *           rotation after prediction or correction steps.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
static void normalize_quat(float *q) {
    float norm = sqrtf(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]); // Compute quaternion magnitude.
    if (norm > 0.0f) {
        for (int i = 0; i < 4; i++) q[i] /= norm; // Divide each component by the magnitude.
    }
}


/*----------------------------------------------------------------------------------------*/
/* Function: normalize_vec3
 * Purpose:  Normalize a 3D vector in place and return the original vector magnitude so the
 *           caller can still reason about the raw size of the input.
 * Returns:  The original magnitude of the vector before normalization.
 */
/*----------------------------------------------------------------------------------------*/
static float normalize_vec3(float *v) {
    float norm = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]); // Compute vector magnitude before modifying it.
    if (norm > 1e-6f) {
        v[0] /= norm; // Normalize X component.
        v[1] /= norm; // Normalize Y component.
        v[2] /= norm; // Normalize Z component.
    }
    return norm; // Return the original length to the caller.
}


/*----------------------------------------------------------------------------------------*/
/* Function: dot3
 * Purpose:  Compute the dot product of two 3D vectors.
 * Returns:  The dot product of vectors a and b.
 */
/*----------------------------------------------------------------------------------------*/
static float dot3(const float *a, const float *b) {
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2]; // Standard 3D dot product.
}


/*----------------------------------------------------------------------------------------*/
/* Function: clampf
 * Purpose:  Clamp a floating-point value to a closed interval.
 * Returns:  The clamped value.
 */
/*----------------------------------------------------------------------------------------*/
static float clampf(float value, float min_value, float max_value) {
    if (value < min_value) {
        return min_value; // Clamp to lower bound.
    }
    if (value > max_value) {
        return max_value; // Clamp to upper bound.
    }
    return value; // Value is already within range.
}


/*----------------------------------------------------------------------------------------*/
/* Function: cross3
 * Purpose:  Compute the cross product of two 3D vectors.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
static void cross3(const float *a, const float *b, float *out) {
    out[0] = a[1]*b[2] - a[2]*b[1]; // X component of cross product.
    out[1] = a[2]*b[0] - a[0]*b[2]; // Y component of cross product.
    out[2] = a[0]*b[1] - a[1]*b[0]; // Z component of cross product.
}


/*----------------------------------------------------------------------------------------*/
/* Function: rotate_world_to_body
 * Purpose:  Rotate a world-space reference vector into the current body frame using the
 *           estimated quaternion.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
static void rotate_world_to_body(const float *q, const float *v, float *out) {
    float q0 = q[0]; // Quaternion scalar term.
    float q1 = q[1]; // Quaternion X term.
    float q2 = q[2]; // Quaternion Y term.
    float q3 = q[3]; // Quaternion Z term.

    // Rotated X
    out[0] = (1.0f - 2.0f * (q2*q2 + q3*q3)) * v[0] + 2.0f * (q1*q2 - q0*q3) * v[1] + 2.0f * (q1*q3 + q0*q2) * v[2];
    // Rotated Y
    out[1] = 2.0f * (q1*q2 + q0*q3) * v[0] + (1.0f - 2.0f * (q1*q1 + q3*q3)) * v[1] + 2.0f * (q2*q3 - q0*q1) * v[2];
    // Rotated Z
    out[2] = 2.0f * (q1*q3 - q0*q2) * v[0] + 2.0f * (q2*q3 + q0*q1) * v[1] + (1.0f - 2.0f * (q1*q1 + q2*q2)) * v[2];
}


/*----------------------------------------------------------------------------------------*/
/* Function: apply_small_angle
 * Purpose:  Apply a small-angle correction vector to the quaternion during the EKF update
 *           step after combining accelerometer and magnetometer error terms.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
static void apply_small_angle(float *q, const float *error) {
    float dq[4] = {1.0f, 0.5f * error[0], 0.5f * error[1], 0.5f * error[2]}; // Small-angle delta quaternion approximation.
    float updated[4]; // Holds the corrected quaternion before copy-back.

    updated[0] = dq[0]*q[0] - dq[1]*q[1] - dq[2]*q[2] - dq[3]*q[3]; // Corrected scalar term.
    updated[1] = dq[0]*q[1] + dq[1]*q[0] + dq[2]*q[3] - dq[3]*q[2]; // Corrected X term.
    updated[2] = dq[0]*q[2] - dq[1]*q[3] + dq[2]*q[0] + dq[3]*q[1]; // Corrected Y term.
    updated[3] = dq[0]*q[3] + dq[1]*q[2] - dq[2]*q[1] + dq[3]*q[0]; // Corrected Z term.

    memcpy(q, updated, sizeof(updated)); // Copy corrected quaternion back into the caller buffer.
    normalize_quat(q); // Re-normalize after correction.
}


/*----------------------------------------------------------------------------------------*/
/* Function: EKF_Init
 * Purpose:  Reset the estimator state to a known default identity orientation and initialize
 *           its covariance, reference, and filtered sensor values.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void EKF_Init(EKF_TypeDef *ekf) {
    memset(ekf, 0, sizeof(*ekf));  // Clear all estimator state.
    ekf->x_hat_data[0] = 1.0f;     // Start from identity quaternion.
    ekf->error_cov[0] = 0.2f;      // Initial covariance for correction axis 0.
    ekf->error_cov[1] = 0.2f;      // Initial covariance for correction axis 1.
    ekf->error_cov[2] = 0.2f;      // Initial covariance for correction axis 2.
    ekf->gravity_ref[2] = 1.0f;    // Default gravity reference points along +Z.
    ekf->mag_ref[0] = 1.0f;        // Default magnetic reference points along +X.
    ekf->filtered_accel[2] = 1.0f; // Seed accel filter with nominal gravity vector.
    ekf->filtered_mag[0] = 1.0f;   // Seed mag filter with nominal magnetic vector.
    ekf->mag_ref_norm = 1.0f;      // Default magnetic magnitude.
}


/*----------------------------------------------------------------------------------------*/
/* Function: EKF_SetReference
 * Purpose:  Load the startup gravity and magnetic field references that define what the EKF
 *           should consider to be the neutral orientation.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void EKF_SetReference(EKF_TypeDef *ekf, float ax, float ay, float az, float mx, float my, float mz) {
    ekf->gravity_ref[0] = ax; // Store gravity reference X.
    ekf->gravity_ref[1] = ay; // Store gravity reference Y.
    ekf->gravity_ref[2] = az; // Store gravity reference Z.
    if (normalize_vec3(ekf->gravity_ref) < 1e-3f) {
        ekf->gravity_ref[0] = 0.0f; // Fallback gravity X if calibration was invalid.
        ekf->gravity_ref[1] = 0.0f; // Fallback gravity Y if calibration was invalid.
        ekf->gravity_ref[2] = 1.0f; // Fallback gravity Z if calibration was invalid.
    }

    ekf->mag_ref[0] = mx; // Store magnetic reference X.
    ekf->mag_ref[1] = my; // Store magnetic reference Y.
    ekf->mag_ref[2] = mz; // Store magnetic reference Z.
    ekf->mag_ref_norm = sqrtf(mx*mx + my*my + mz*mz); // Save raw magnetic field magnitude.
    ekf->use_mag = normalize_vec3(ekf->mag_ref) >= 1e-3f ? 1u : 0u; // Enable mag correction only if the reference is valid.

    memcpy(ekf->filtered_accel, ekf->gravity_ref, sizeof(ekf->filtered_accel)); // Seed accel filter from reference.
    memcpy(ekf->filtered_mag, ekf->mag_ref, sizeof(ekf->filtered_mag)); // Seed mag filter from reference.
}


/*----------------------------------------------------------------------------------------*/
/* Function: EKF_Predict
 * Purpose:  Predict the next orientation state by integrating the gyroscope reading and
 *           updating the filter covariance.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void EKF_Predict(EKF_TypeDef *ekf, float gx, float gy, float gz, float dt) {
    float *q = ekf->x_hat_data; // Convenience pointer to the live quaternion state.
    ekf->last_gyro_norm = sqrtf(gx*gx + gy*gy + gz*gz); // Store current motion magnitude for trust calculations.
    gx -= ekf->gyro_bias[0]; // Remove estimated X gyro bias.
    gy -= ekf->gyro_bias[1]; // Remove estimated Y gyro bias.
    gz -= ekf->gyro_bias[2]; // Remove estimated Z gyro bias.

    float q_dot[4]; // Quaternion derivative for this predict step.
    q_dot[0] = 0.5f * (-q[1] * gx - q[2] * gy - q[3] * gz); // Scalar derivative.
    q_dot[1] = 0.5f * ( q[0] * gx + q[2] * gz - q[3] * gy); // X derivative.
    q_dot[2] = 0.5f * ( q[0] * gy - q[1] * gz + q[3] * gx); // Y derivative.
    q_dot[3] = 0.5f * ( q[0] * gz + q[1] * gy - q[2] * gx); // Z derivative.

    for (int i = 0; i < 4; i++) q[i] += q_dot[i] * dt; // Integrate quaternion forward in time.
    normalize_quat(q); // Keep quaternion unit length after prediction.
    for (int i = 0; i < 3; i++) {
        ekf->error_cov[i] += 0.01f * dt; // Slowly inflate covariance between corrections.
        if (ekf->error_cov[i] > 1.0f) {
            ekf->error_cov[i] = 1.0f; // Cap covariance growth.
        }
    }
}


/*----------------------------------------------------------------------------------------*/
/* Function: EKF_Update
 * Purpose:  Correct the predicted orientation using accelerometer and optional magnetometer
 *           measurements, with trust values that change depending on current motion.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void EKF_Update(EKF_TypeDef *ekf, float ax, float ay, float az, float mx, float my, float mz) {
    float accel[3] = {ax, ay, az};                       // Current accelerometer sample.
    float mag[3] = {mx, my, mz};                         // Current magnetometer sample.
    float gravity_pred[3];                               // Predicted gravity vector in body frame.
    float mag_pred[3];                                   // Predicted magnetic vector in body frame.
    float error_acc[3] = {0.0f, 0.0f, 0.0f};             // Accel correction error.
    float error_mag[3] = {0.0f, 0.0f, 0.0f};             // Magnetometer correction error.
    float correction[3] = {0.0f, 0.0f, 0.0f};            // Final combined correction vector.
    float accel_norm_raw = sqrtf(ax*ax + ay*ay + az*az); // Raw accel magnitude before normalization.
    float accel_norm = normalize_vec3(accel);            // Normalize accel and keep original magnitude.
    float accel_motion = fabsf(accel_norm_raw - 1.0f);   // Deviation from 1 g, used to estimate linear motion.
    float gyro_motion = ekf->last_gyro_norm;             // Rotational motion estimate from predict step.
    // Trust accel less during strong motion.
    float accel_trust = clampf(1.0f - 3.5f * accel_motion - 0.2f * gyro_motion, 0.0f, 1.0f);
    float mag_norm_raw = sqrtf(mx*mx + my*my + mz*mz);  // Raw magnetic magnitude.
    float mag_trust = 0.0f;                             // Magnetometer trust starts at zero until validated.

    rotate_world_to_body(ekf->x_hat_data, ekf->gravity_ref, gravity_pred); // Predict what gravity should look like in body frame.
    normalize_vec3(gravity_pred); // Use only direction, not magnitude.

    if (accel_norm > 1e-3f) {
        for (int i = 0; i < 3; i++) {
            // Low-pass filter accel direction.
            ekf->filtered_accel[i] = 0.65f * ekf->filtered_accel[i] + 0.35f * accel[i];
        }
        normalize_vec3(ekf->filtered_accel); // Renormalize filtered accel direction.
    }

    if (accel_trust > 0.05f) {
        // Cross product gives the correction axis from predicted to measured gravity.
        cross3(gravity_pred, ekf->filtered_accel, error_acc);
    }

    if (ekf->use_mag && normalize_vec3(mag) > 1e-3f) {
        float mag_h[3];      // Measured horizontal magnetic component.
        float mag_pred_h[3]; // Predicted horizontal magnetic component.
        // Relative magnetic magnitude error.
        float mag_norm_error = (ekf->mag_ref_norm > 1e-3f) ? fabsf(mag_norm_raw - ekf->mag_ref_norm) / ekf->mag_ref_norm : 0.0f;

        // Trust mag less when the field looks disturbed.
        mag_trust = clampf(1.0f - 2.0f * mag_norm_error - 0.1f * gyro_motion, 0.0f, 1.0f);
        for (int i = 0; i < 3; i++) {
            // Low-pass filter magnetometer direction.
            ekf->filtered_mag[i] = 0.60f * ekf->filtered_mag[i] + 0.40f * mag[i];
        }
        normalize_vec3(ekf->filtered_mag); // Use only direction, not magnitude.

        rotate_world_to_body(ekf->x_hat_data, ekf->mag_ref, mag_pred); // Predict magnetic direction in body frame.
        for (int i = 0; i < 3; i++) {
            // Remove vertical component from predicted mag.
            mag_pred_h[i] = mag_pred[i] - dot3(mag_pred, gravity_pred) * gravity_pred[i];
            // Remove vertical component from measured mag.
            mag_h[i] = ekf->filtered_mag[i] - dot3(ekf->filtered_mag, gravity_pred) * gravity_pred[i];
        }

        if (mag_trust > 0.05f && normalize_vec3(mag_pred_h) > 1e-3f && normalize_vec3(mag_h) > 1e-3f) {
            // Horizontal magnetic mismatch becomes yaw correction.
            cross3(mag_pred_h, mag_h, error_mag);
        }
    }

    for (int i = 0; i < 3; i++) {
        // Accelerometer gain for this axis.
        float k_acc = accel_trust * (ekf->error_cov[i] / (ekf->error_cov[i] + 0.10f));
        // Magnetometer gain for this axis.
        float k_mag = mag_trust * (ekf->error_cov[i] / (ekf->error_cov[i] + 0.25f));

        // Weighted blend of accel and mag correction.
        correction[i] = 0.42f * k_acc * error_acc[i] + 0.10f * k_mag * error_mag[i];
        if (accel_trust > 0.5f && gyro_motion < 0.35f) {
            // Slowly adapt gyro bias when motion is calm.
            ekf->gyro_bias[i] += 0.006f * correction[i];
        }
        // Reduce covariance when a strong correction was applied.
        ekf->error_cov[i] *= (1.0f - 0.22f * (k_acc + k_mag));
        if (ekf->error_cov[i] < 1e-4f) {
            ekf->error_cov[i] = 1e-4f; // Keep covariance from collapsing to zero.
        }
    }

    apply_small_angle(ekf->x_hat_data, correction); // Apply the final correction back into the quaternion estimate.
}
