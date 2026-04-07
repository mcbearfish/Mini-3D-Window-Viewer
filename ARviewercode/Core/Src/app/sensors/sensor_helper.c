/*----------------------------------------------------------------------------------------*/
/*
 * File:    sensor_helper.c
 * Purpose: Sensor bring-up, startup calibration, startup UI screens, and
 *          bias-corrected sensor readout for the IMU and magnetometer stack.
 *
 * Functions contained in this file:
 * - DrawCenteredTextLine: Draws a centered status line using the bitmap font.
 * - DrawCountdownScreen: Draws the green keep-still countdown screen.
 * - DrawCalibrationScreen: Draws the red calibration screen.
 * - DrawCalibrationProgress: Updates the calibration progress bar.
 * - DrawReadyScreen: Draws the short READY confirmation screen.
 * - DrawInstructionScreen: Draws the interaction instructions screen.
 * - platform_write: Writes bytes to the IMU through the ST driver callback.
 * - platform_read: Reads bytes from the IMU through the ST driver callback.
 * - SENSOR_IO_Init: Board-support initialization shim for sensor drivers.
 * - SENSOR_IO_DeInit: Board-support deinitialization shim for sensor drivers.
 * - SENSOR_IO_Write: Writes one sensor register through the BSP wrapper.
 * - SENSOR_IO_Read: Reads one sensor register through the BSP wrapper.
 * - SENSOR_IO_ReadMultiple: Reads multiple sensor bytes through the BSP wrapper.
 * - SENSOR_IO_WriteMultiple: Writes multiple sensor bytes through the BSP wrapper.
 * - Sensors_Init: Probes and configures the IMU and magnetometer.
 * - Sensors_ShowStartupCountdown: Shows the startup countdown screen.
 * - Sensors_Calibrate: Collects still samples and computes startup bias terms.
 * - Sensors_ShowInstructionScreen: Shows the short instruction screen.
 * - Sensors_Read: Returns the latest calibrated sensor values.
 * - Sensors_GetReferenceFrame: Returns calibration reference vectors for the EKF.
 */
/*----------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------*/
// INCLUDES
/*----------------------------------------------------------------------------------------*/
#include "sensor_helper.h"
#include "gpu_driver.h"
#include <string.h>


/*----------------------------------------------------------------------------------------*/
// GLOBAL VARIABLES
/*----------------------------------------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;

/* Public availability flags checked by the main loop and EKF setup. */
uint8_t mag_available = 0;
uint8_t accel_available = 0;

/* Low-level IMU device context used by the ST sensor driver. */
static uint8_t lsm_addr = LSM6DS3TR_C_I2C_ADD_L & 0xFE;
static stmdev_ctx_t dev_ctx;
static uint8_t whoamI, rst;

/* Calibration values derived from the startup stillness capture. */
static float gyro_bias[3] = {0.0f, 0.0f, 0.0f};
static float accel_bias[3] = {0.0f, 0.0f, 0.0f};
static float mag_offset[3] = {0.0f, 0.0f, 0.0f};
static float accel_reference_mg[3] = {0.0f, 0.0f, 1000.0f};
static float mag_reference[3] = {1.0f, 0.0f, 0.0f};


/*----------------------------------------------------------------------------------------*/
/* Function: DrawCenteredTextLine
 * Purpose:  Draw a single centered text line using the tiny built-in bitmap font.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
static void DrawCenteredTextLine(int y, const char *text, uint16_t color, int scale) {
  int width = (int)strlen(text) * 6 * scale;  // Estimate text width from glyph width and scale.
  int x = (SCR_WIDTH - width) / 2;            // Compute centered X position.
  if (x < 0) {
    x = 0; // Clamp to the left edge if the text is wider than the screen.
  }
  GPU_DrawText(x, y, text, color, scale); // Draw the centered text line.
}


/*----------------------------------------------------------------------------------------*/
/* Function: DrawCountdownScreen
 * Purpose:  Draw the green startup hold-still countdown.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
static void DrawCountdownScreen(int value) {
  char countdown_text[2] = {'0', '\0'}; // One-digit countdown plus string terminator.

  countdown_text[0] = (char)('0' + value); // Convert the numeric countdown value to ASCII.

  GPU_FillScreen(COLOR_GREEN); // Clear the screen with the startup background color.
  DrawCenteredTextLine(60, "KEEP STILL", COLOR_BLACK, 5);      // Top instruction line.
  DrawCenteredTextLine(120, "FOR 5 SECONDS", COLOR_BLACK, 4);  // Secondary instruction line.
  DrawCenteredTextLine(210, countdown_text, COLOR_BLACK, 10);      // Large numeric countdown.
}


/*----------------------------------------------------------------------------------------*/
/* Function: DrawCalibrationScreen
 * Purpose:  Draw the red calibration screen before and during averaging.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
static void DrawCalibrationScreen(void) {
  GPU_FillScreen(COLOR_RED); // Fill screen with calibration background color.
  DrawCenteredTextLine(110, "CALIBRATING", COLOR_WHITE, 5); // Main calibration label.
  DrawCenteredTextLine(180, "KEEP STILL", COLOR_WHITE, 4);  // Reminder to the user.
}


/*----------------------------------------------------------------------------------------*/
/* Function: DrawCalibrationProgress
 * Purpose:  Draw the calibration progress bar. The outer white rectangle stays fixed while
 *           the green fill grows as more calibration samples are collected.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
static void DrawCalibrationProgress(int progress_width) {
  GPU_FillRect(88, 245, 304, 24, COLOR_WHITE); // Draw the white border/background of the bar.
  GPU_FillRect(92, 249, 296, 16, COLOR_BLACK); // Clear the inner bar area before drawing progress.
  if (progress_width > 0) {
    // Fill the completed portion of the progress bar.
    GPU_FillRect(92, 249, progress_width, 16, COLOR_GREEN);
  }
}


/*----------------------------------------------------------------------------------------*/
/* Function: DrawReadyScreen
 * Purpose:  Draw the brief READY confirmation after calibration completes.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
static void DrawReadyScreen(void) {
  GPU_FillScreen(COLOR_GREEN); // Show a green success background.
  DrawCenteredTextLine(120, "READY", COLOR_BLACK, 6); // Draw the READY message.
}


/*----------------------------------------------------------------------------------------*/
/* Function: DrawInstructionScreen
 * Purpose:  Draw the post-calibration instruction screen shown before live rendering.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
static void DrawInstructionScreen(void) {
  GPU_FillScreen(COLOR_BLACK); // Clear to black before instruction text.
  DrawCenteredTextLine(96, "TILT TO ROTATE", COLOR_WHITE, 4);       // Primary interaction instruction.
  DrawCenteredTextLine(168, "HOLD STILL TO RESET", COLOR_WHITE, 3); // Reset instruction.
}


/*----------------------------------------------------------------------------------------*/
/* Function: platform_write
 * Purpose:  Write one or more bytes to the LSM6DS3TR-C over I2C through the callback
 *           interface expected by the ST sensor driver.
 * Returns:  0 to satisfy the ST driver callback contract.
 */
/*----------------------------------------------------------------------------------------*/
static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len) {
  // Write sensor register bytes over I2C.
  HAL_I2C_Mem_Write(handle, lsm_addr, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)bufp, len, 1000);
  return 0; // ST driver expects 0 on success.
}


/*----------------------------------------------------------------------------------------*/
/* Function: platform_read
 * Purpose:  Read one or more bytes from the LSM6DS3TR-C over I2C.
 * Returns:  0 to satisfy the ST driver callback contract.
 */
/*----------------------------------------------------------------------------------------*/
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
  // Read sensor register bytes over I2C.
  HAL_I2C_Mem_Read(handle, lsm_addr, reg, I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
  return 0; // ST driver expects 0 on success.
}


/*----------------------------------------------------------------------------------------*/
/* Function: SENSOR_IO_Init
 * Purpose:  Provide the BSP compatibility initialization shim required by the ST sensor
 *           libraries.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void SENSOR_IO_Init(void)
{
    // No extra BSP setup is needed in this project.
}

/*----------------------------------------------------------------------------------------*/
/* Function: SENSOR_IO_DeInit
 * Purpose:  Provide the BSP compatibility deinitialization shim required by the ST sensor
 *           libraries.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void SENSOR_IO_DeInit(void)
{
    HAL_I2C_DeInit(&hi2c1); // Tear down the shared I2C peripheral.
}


/*----------------------------------------------------------------------------------------*/
/* Function: SENSOR_IO_Write
 * Purpose:  Write a single sensor register through the board support wrapper.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void SENSOR_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value) {
  // Read one register value.
  HAL_I2C_Mem_Write(&hi2c1, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, &Value, 1, 1000);
}


/*----------------------------------------------------------------------------------------*/
/* Function: SENSOR_IO_Read
 * Purpose:  Read a single sensor register through the board support wrapper.
 * Returns:  The register value that was read.
 */
/*----------------------------------------------------------------------------------------*/
uint8_t SENSOR_IO_Read(uint8_t Addr, uint8_t Reg) {
  uint8_t Value = 0; // Temporary storage for the returned register value.
  // Read one register value.
  HAL_I2C_Mem_Read(&hi2c1, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, &Value, 1, 1000);
  return Value; // Return the byte to the caller. */
}


/*----------------------------------------------------------------------------------------*/
/* Function: SENSOR_IO_ReadMultiple
 * Purpose:  Read multiple sensor bytes through the board support wrapper.
 * Returns:  The HAL I2C status code from the read operation.
 */
/*----------------------------------------------------------------------------------------*/
uint16_t SENSOR_IO_ReadMultiple(uint8_t Addr, uint8_t Reg, uint8_t *Buffer, uint16_t Length) {
  // Read a block of bytes.
  return HAL_I2C_Mem_Read(&hi2c1, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length, 1000);
}


/*----------------------------------------------------------------------------------------*/
/* Function: SENSOR_IO_WriteMultiple
 * Purpose:  Write multiple sensor bytes through the board support wrapper.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void SENSOR_IO_WriteMultiple(uint8_t Addr, uint8_t Reg, uint8_t *Buffer, uint16_t Length) {
  // Write a block of bytes.
  HAL_I2C_Mem_Write(&hi2c1, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length, 1000);
}


/*----------------------------------------------------------------------------------------*/
/* Function: Sensors_Init
 * Purpose:  Probe the magnetometer and IMU, then configure their sample rates and ranges.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void Sensors_Init(void) {
  LIS3MDL_I2C_Address = LIS3MDL_MAG_I2C_ADDRESS_HIGH; // Try the first possible magnetometer I2C address.
  uint8_t id = LIS3MDL_MagReadID(); // Read the magnetometer identity register.
  if (id != I_AM_LIS3MDL) {
      LIS3MDL_I2C_Address = LIS3MDL_MAG_I2C_ADDRESS_LOW; // Fall back to the second possible address.
      id = LIS3MDL_MagReadID(); // Try reading identity again at the alternate address.
  }
  if (id == I_AM_LIS3MDL) {
      mag_available = 1; // Mark the magnetometer as present.
      MAGNETO_InitTypeDef LIS3MDL_InitStruct; // Magnetometer configuration struct.
      // XY performance and ODR.
      LIS3MDL_InitStruct.Register1 = LIS3MDL_MAG_TEMPSENSOR_ENABLE | LIS3MDL_MAG_OM_XY_HIGH | LIS3MDL_MAG_ODR_80_HZ;
      // Range and reset settings.
      LIS3MDL_InitStruct.Register2 = LIS3MDL_MAG_FS_4_GA | LIS3MDL_MAG_REBOOT_DEFAULT | LIS3MDL_MAG_SOFT_RESET_DEFAULT;
      // Continuous conversion mode.
      LIS3MDL_InitStruct.Register3 = LIS3MDL_MAG_CONFIG_NORMAL_MODE | LIS3MDL_MAG_CONTINUOUS_MODE;
      // Z performance and endian setting.
      LIS3MDL_InitStruct.Register4 = LIS3MDL_MAG_OM_Z_HIGH | LIS3MDL_MAG_BLE_LSB;
      // Block data update setting.
      LIS3MDL_InitStruct.Register5 = LIS3MDL_MAG_BDU_MSBLSB;
      LIS3MDL_MagInit(LIS3MDL_InitStruct); // Apply magnetometer configuration.
  }

  dev_ctx.write_reg = platform_write; // Attach low-level write callback to ST driver context
  dev_ctx.read_reg = platform_read; // Attach low-level read callback to ST driver context.
  dev_ctx.handle = &hi2c1; // Pass shared I2C handle into the driver context.
  lsm_addr = LSM6DS3TR_C_I2C_ADD_L & 0xFE; // Try the low IMU address first.
  for(int i=0; i<5; i++) {
      lsm6ds3tr_c_device_id_get(&dev_ctx, &whoamI); // Read the IMU identity register.
      if (whoamI == LSM6DS3TR_C_ID) break; // Stop retrying if the correct device responded.
      HAL_Delay(10); // Small delay before the next retry.
  }
  if (whoamI != LSM6DS3TR_C_ID) {
      lsm_addr = LSM6DS3TR_C_I2C_ADD_H & 0xFE; // Fall back to the alternate IMU address.
      lsm6ds3tr_c_device_id_get(&dev_ctx, &whoamI); // Try reading the identity register again.
  }

  if (whoamI == LSM6DS3TR_C_ID) {
    accel_available = 1; // Mark IMU/accelerometer path as present.
    lsm6ds3tr_c_reset_set(&dev_ctx, PROPERTY_ENABLE); // Request an IMU reset.
    do { lsm6ds3tr_c_reset_get(&dev_ctx, &rst); } while (rst); // Wait until reset is finished.
    lsm6ds3tr_c_block_data_update_set(&dev_ctx, PROPERTY_ENABLE); // Prevent partial multi-byte reads.
    lsm6ds3tr_c_xl_data_rate_set(&dev_ctx, LSM6DS3TR_C_XL_ODR_104Hz); // Set accelerometer output rate.
    lsm6ds3tr_c_gy_data_rate_set(&dev_ctx, LSM6DS3TR_C_GY_ODR_104Hz); // Set gyroscope output rate.
    lsm6ds3tr_c_xl_full_scale_set(&dev_ctx, LSM6DS3TR_C_2g); // Set accelerometer range.
    lsm6ds3tr_c_gy_full_scale_set(&dev_ctx, LSM6DS3TR_C_2000dps); // Set gyroscope range.
  }
}


/*----------------------------------------------------------------------------------------*/
/* Function: Sensors_ShowStartupCountdown
 * Purpose:  Show the green countdown that tells the user to keep the device still.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void Sensors_ShowStartupCountdown(void) {
  int count; // Current countdown value shown on the screen.

  for (count = 3; count >= 0; count--) {
      DrawCountdownScreen(count); // Draw this countdown frame.
      HAL_Delay(1000); // Hold it on-screen for one second.
  }
}


/*----------------------------------------------------------------------------------------*/
/* Function: Sensors_Calibrate
 * Purpose:  Average a fixed set of samples while the box is still to estimate bias terms.
 *           Gyroscope bias comes from the average gyro reading at rest, accelerometer bias
 *           comes from the gravity-aligned average, and magnetometer reference data is
 *           captured when the magnetometer is present.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void Sensors_Calibrate(void) {
  if (!accel_available) return; // Skip calibration if the IMU was not detected.
  int16_t raw_gy[3], raw_xl[3]; // Temporary raw gyro and accelerometer sample storage.
  int samples = 520; // Number of still samples used to build the average.
  int last_progress_width = -1; // Tracks the last drawn progress width so redraws are minimized.

  DrawCalibrationScreen(); // Show the red calibration screen.
  DrawCalibrationProgress(0); // Start with an empty progress bar. */

  // Running sums for each sensor axis.
  float g_accum[3] = {0,0,0}, a_accum[3] = {0,0,0}, m_accum[3] = {0,0,0};
  for(int i=0; i<samples; i++) {
      uint8_t drdy; // Data-ready flag from the IMU.
      do { lsm6ds3tr_c_gy_flag_data_ready_get(&dev_ctx, &drdy); } while (!drdy); // Wait for a fresh gyro sample.
      lsm6ds3tr_c_angular_rate_raw_get(&dev_ctx, raw_gy); // Read raw gyro data.
      lsm6ds3tr_c_acceleration_raw_get(&dev_ctx, raw_xl); // Read raw accelerometer data.

      // Accumulate raw sensor readings so an average can be computed later.
      for(int j=0; j<3; j++) {
          g_accum[j] += lsm6ds3tr_c_from_fs2000dps_to_mdps(raw_gy[j]); // Add converted gyro sample to running sum.
          a_accum[j] += lsm6ds3tr_c_from_fs2g_to_mg(raw_xl[j]); // Add converted accel sample to running sum.
      }
      if (mag_available) {
          int16_t m[3]; // Temporary raw magnetometer sample storage.
          LIS3MDL_MagReadXYZ(m); // Read current magnetometer sample.
          for (int j = 0; j < 3; j++) {
              m_accum[j] += (float)m[j]; // Add raw magnetometer sample to running sum.
          }
      }

      // Update the visible progress bar only when its width changes.
      {
          int progress_width = ((i + 1) * 296) / samples; // Convert sample count into pixel width.
          if (progress_width != last_progress_width) {
              DrawCalibrationProgress(progress_width); // Redraw the progress bar with the new width.
              last_progress_width = progress_width; // Remember this width so identical redraws are skipped.
          }
      }
  }
  for(int j=0; j<3; j++) {
      gyro_bias[j] = g_accum[j] / samples; // Average gyro bias on this axis.
      accel_bias[j] = a_accum[j] / samples; // Average accel reading on this axis.
  }

  // The box starts flat, so gravity is expected on the local Z axis.
  accel_bias[2] -= 1000.0f; // Remove 1 g from the vertical accelerometer bias.

  if (mag_available) {
      for (int j = 0; j < 3; j++) {
          mag_reference[j] = m_accum[j] / samples; // Average magnetic field reference on this axis.
          mag_offset[j] = 0.0f; // No hard-iron offset compensation is applied here.
      }
  }
  for (int j = 0; j < 3; j++) {
      // Reference gravity vector after bias removal.
      accel_reference_mg[j] = (a_accum[j] / samples) - accel_bias[j];
  }
  DrawReadyScreen(); // Show READY to indicate calibration completed.
  HAL_Delay(1000); // Hold READY long enough to be seen.
  GPU_FillScreen(COLOR_BLACK); // Clear screen before the next UI stage.
}


/*----------------------------------------------------------------------------------------*/
/* Function: Sensors_ShowInstructionScreen
 * Purpose:  Show the instructions that explain the interaction model.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void Sensors_ShowInstructionScreen(void) {
  DrawInstructionScreen(); // Draw the instruction screen.
  HAL_Delay(3500); // Hold it on-screen long enough to read.
  GPU_FillScreen(COLOR_BLACK); // Clear screen before entering live mode.
}


/*----------------------------------------------------------------------------------------*/
/* Function: Sensors_Read
 * Purpose:  Return the latest bias-corrected samples from each available sensor.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void Sensors_Read(float *accel_mg, float *gyro_mdps, float *mag_val) {
    if (!accel_available) return; // Abort if the IMU was never initialized.
    uint8_t drdy; // Data-ready flag returned by the IMU.
    int16_t raw[3]; // Temporary raw sample buffer reused for accel and gyro.
    lsm6ds3tr_c_xl_flag_data_ready_get(&dev_ctx, &drdy); // Check whether a new accel sample is ready.
    if (drdy) {
      lsm6ds3tr_c_acceleration_raw_get(&dev_ctx, raw); // Read raw accelerometer data.
      //Convert to mg and remove bias.
      for(int j=0; j<3; j++) accel_mg[j] = lsm6ds3tr_c_from_fs2g_to_mg(raw[j]) - accel_bias[j];
    }
    lsm6ds3tr_c_gy_flag_data_ready_get(&dev_ctx, &drdy); // Check whether a new gyro sample is ready.
    if (drdy) {
      lsm6ds3tr_c_angular_rate_raw_get(&dev_ctx, raw); // Read raw gyro data.
      // Convert to mdps and remove bias.
      for(int j=0; j<3; j++) gyro_mdps[j] = lsm6ds3tr_c_from_fs2000dps_to_mdps(raw[j]) - gyro_bias[j];
    }
    if (mag_available) {
        int16_t m[3]; // Temporary raw magnetometer sample.
        LIS3MDL_MagReadXYZ(m); // Read current magnetometer values.
        // Convert to float and remove offset.
        for(int j=0; j<3; j++) mag_val[j] = (float)m[j] - mag_offset[j];
    }
}


/*----------------------------------------------------------------------------------------*/
/* Function: Sensors_GetReferenceFrame
 * Purpose:  Copy out the calibrated reference vectors used to initialize the EKF.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void Sensors_GetReferenceFrame(float *accel_ref_mg, float *mag_ref) {
    if (accel_ref_mg != NULL) {
        for (int i = 0; i < 3; i++) {
            accel_ref_mg[i] = accel_reference_mg[i]; // Copy one accelerometer reference component.
        }
    }
    if (mag_ref != NULL) {
        for (int i = 0; i < 3; i++) {
            mag_ref[i] = mag_reference[i]; // Copy one magnetometer reference component.
        }
    }
}
