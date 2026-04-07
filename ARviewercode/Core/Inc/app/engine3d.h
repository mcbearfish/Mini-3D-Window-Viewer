/*
 * File: engine3d.h
 * Purpose: Public interface for the lightweight 3D renderer used by the demo.
 * It exposes the camera/object pose setters and the render entry points.
 *
 * Functions in this file:
 * - Engine3D_Init: initializes the renderer and LCD subsystem.
 * - Engine3D_SetCameraPose: sets the camera position for the next frame.
 * - Engine3D_SetObjectRotation: sets the object quaternion for the next frame.
 * - Engine3D_PrepareFrame: projects the 3D scene into 2D lines.
 * - Engine3D_DrawScene: draws the projected line list.
 */
#ifndef ENGINE3D_H
#define ENGINE3D_H

#include "gpu_driver.h"

/* Basic 3D point used by the lightweight renderer. */
typedef struct {
    float x, y, z;
} Vec3;

/* 2D screen-space point used after projection. */
typedef struct {
    int16_t x, y;
} Point2D;

void Engine3D_Init(void);
void Engine3D_SetCameraPose(float x, float y, float z);
void Engine3D_SetObjectRotation(float q0, float q1, float q2, float q3);
void Engine3D_PrepareFrame(void);
void Engine3D_DrawScene(void);

#endif // ENGINE3D_H
