/*
 * File: gpu_driver.h
 * Purpose: Public interface for the LCD drawing helpers used by both the 3D
 * renderer and the startup/status UI screens.
 *
 * Functions in this file:
 * - GPU_Init: initializes the LCD subsystem.
 * - GPU_FillScreen: fills the entire display with one color.
 * - GPU_FillRect: fills a clipped rectangle on the display.
 * - GPU_Render: runs one draw callback.
 * - GPU_DrawLine: draws a line directly to the LCD.
 * - GPU_DrawPixel: draws a single pixel.
 * - GPU_DrawChar: draws a single built-in bitmap glyph.
 * - GPU_DrawText: draws a string using the built-in bitmap font.
 */
#ifndef GPU_DRIVER_H
#define GPU_DRIVER_H

#include <stdint.h>

/* Physical LCD size in landscape orientation. */
#define SCR_WIDTH  480
#define SCR_HEIGHT 320

/* Common RGB565 colors used by the simple UI and renderer. */
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_YELLOW  0xFFE0

void GPU_Init(void);
void GPU_FillScreen(uint16_t color);
void GPU_FillRect(int x, int y, int w, int h, uint16_t color);
void GPU_Render(void (*draw_callback)(void));
void GPU_DrawLine(int x1, int y1, int x2, int y2, uint16_t color);
void GPU_DrawPixel(int x, int y, uint16_t color);
void GPU_DrawChar(int x, int y, char c, uint16_t color, int scale);
void GPU_DrawText(int x, int y, const char *text, uint16_t color, int scale);

#endif // GPU_DRIVER_H
