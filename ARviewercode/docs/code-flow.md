# Code Flow

This document summarizes the runtime flow of the AR viewer firmware and the relationship between the main application modules.

## Main Runtime Flow

```text
Power On / main()
|
+-- HAL_Init()
+-- SystemClock_Config()
+-- MX_GPIO_Init()
+-- MX_FMC_Init()
+-- I2C_Bus_Reset()
+-- MX_I2C1_Init()
|
+-- Engine3D_Init()
|   \-- GPU_Init()
|       \-- LCD init + clear screen
|
+-- Sensors_Init()
|   +-- Probe magnetometer
|   |   \-- If found: configure LIS3MDL
|   \-- Probe IMU
|       \-- If found: configure LSM6DS3TR-C
|
+-- Sensors_ShowStartupCountdown()
+-- Sensors_Calibrate()
|   +-- Draw calibration UI
|   +-- Collect still samples
|   +-- Compute gyro bias
|   +-- Compute accel bias/reference
|   \-- Capture mag reference if available
|
+-- Sensors_ShowInstructionScreen()
+-- Sensors_GetReferenceFrame()
+-- EKF_Init()
+-- EKF_SetReference()
|
\-- while (1)
    +-- Sensors_Read()
    |   +-- Read accel
    |   +-- Read gyro
    |   \-- Read mag if available
    |
    +-- Convert gyro mdps to rad/s
    +-- Compute dt from HAL_GetTick
    +-- EKF_Predict()
    +-- Convert accel mg to g
    +-- Magnetometer available?
    |   +-- Yes -> EKF_Update(accel + mag)
    |   \-- No  -> EKF_Update(accel only)
    |
    +-- Read EKF quaternion
    +-- Quaternion valid?
    |   +-- No -> EKF_Init() and continue
    |   \-- Yes
    |
    +-- Compute gravity_body
    +-- ComputeSpinCommands()
    +-- IntegrateQuat(object_q)
    +-- Compute motion magnitude
    +-- IsStableForReset()?
    |   +-- Yes
    |   |   +-- Accumulate still_time_s
    |   |   \-- If still_time_s >= 10 s -> ResetObjectOrientation()
    |   \-- No
    |       \-- Reset still_time_s to 0
    |
    +-- QuaternionToEulerDegrees()
    +-- Engine3D_SetCameraPose()
    +-- Engine3D_SetObjectRotation()
    +-- Engine3D_PrepareFrame()
    |   +-- Rotate vertices
    |   +-- Project points
    |   \-- Build visible line list
    |
    +-- GPU_Render(Engine3D_DrawScene)
    |   +-- Erase previous lines
    |   \-- Draw current lines
    |
    \-- DrawPoseOverlay()
```

## Module Relationships

```text
main.c
|
+-- sensor_helper.c
+-- kalman_filter.c
+-- engine3d.c
|   \-- gpu_driver.c
\-- gpu_driver.c (via sensor/UI drawing paths)
```

## Notes

- `main.c` owns system startup, the main loop, and the high-level application state.
- `sensor_helper.c` handles sensor detection, calibration, reference-frame capture, and live sensor reads.
- `kalman_filter.c` estimates device orientation from gyro, accelerometer, and optional magnetometer data.
- `engine3d.c` converts the current object orientation into projected wireframe line segments.
- `gpu_driver.c` wraps low-level LCD drawing primitives used by both the renderer and the startup/UI screens.
