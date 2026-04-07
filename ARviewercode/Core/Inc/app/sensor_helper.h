/*
 * File: sensor_helper.h
 * Purpose: Public interface for sensor startup, calibration, and calibrated
 * runtime reads used by the main application loop.
 *
 * Functions in this file:
 * - Sensors_Init: probes and configures the physical sensors.
 * - Sensors_ShowStartupCountdown: shows the green startup countdown.
 * - Sensors_Calibrate: captures still calibration samples and computes bias terms.
 * - Sensors_ShowInstructionScreen: shows the short instruction screen.
 * - Sensors_Read: returns the latest calibrated sensor samples.
 * - Sensors_GetReferenceFrame: returns the startup reference vectors for the EKF.
 */
#ifndef SENSOR_HELPER_H
#define SENSOR_HELPER_H

#include "main.h"
#include "Sensors/lsm6ds3tr-c_reg.h"
#include "Sensors/lis3mdl.h"

/* Flags that indicate whether each physical sensor responded at startup. */
extern uint8_t mag_available;
extern uint8_t accel_available;

void Sensors_Init(void);
void Sensors_ShowStartupCountdown(void);
void Sensors_Calibrate(void);
void Sensors_ShowInstructionScreen(void);
void Sensors_Read(float *accel_mg, float *gyro_mdps, float *mag_val);
void Sensors_GetReferenceFrame(float *accel_ref_mg, float *mag_ref);

#endif /* SENSOR_HELPER_H */
