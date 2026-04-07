/*----------------------------------------------------------------------------------------*/
/*
 * File:    gpu_driver.c
 * Purpose: Very small LCD drawing layer that wraps the board's LCD driver and
 *          provides direct drawing primitives plus a tiny bitmap font for UI overlays.
 *
 * Functions contained in this file:
 * - GPU_GetGlyph: Returns the 5x7 bitmap for a supported character.
 * - GPU_Init: Initializes the LCD and clears the screen.
 * - GPU_FillScreen: Fills the full display with one color.
 * - GPU_FillRect: Fills a clipped rectangle.
 * - GPU_DrawPixel: Draws one pixel if it is on-screen.
 * - GPU_DrawLine: Draws a line with Bresenham rasterization.
 * - GPU_DrawChar: Draws one bitmap-font character.
 * - GPU_DrawText: Draws a string using the bitmap font.
 * - GPU_Render: Runs the caller-provided scene draw callback.
 */
/*----------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------*/
// INCLUDES
/*----------------------------------------------------------------------------------------*/
#include "gpu_driver.h"
#include "Lcd/lcd.h"
#include <stdlib.h>


/*----------------------------------------------------------------------------------------*/
// GLOBAL VARIABLES
/*----------------------------------------------------------------------------------------*/
extern LCD_DrvTypeDef *lcd_drv;


/*----------------------------------------------------------------------------------------*/
/* Function: GPU_GetGlyph
 * Purpose:  Return the 5x7 bitmap for a supported character. Only a small subset of ASCII
 *           is needed because the project only displays simple status and numeric text.
 * Returns:  A pointer to the glyph bitmap for the requested character.
 */
/*----------------------------------------------------------------------------------------*/
static const uint8_t *GPU_GetGlyph(char c) {
    static const uint8_t glyph_space[7] = {0, 0, 0, 0, 0, 0, 0};
    static const uint8_t glyph_0[7]     = {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E};
    static const uint8_t glyph_1[7]     = {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E};
    static const uint8_t glyph_2[7]     = {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F};
    static const uint8_t glyph_3[7]     = {0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E};
    static const uint8_t glyph_4[7]     = {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02};
    static const uint8_t glyph_5[7]     = {0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E};
    static const uint8_t glyph_6[7]     = {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E};
    static const uint8_t glyph_7[7]     = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08};
    static const uint8_t glyph_8[7]     = {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E};
    static const uint8_t glyph_9[7]     = {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C};
    static const uint8_t glyph_A[7]     = {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
    static const uint8_t glyph_B[7]     = {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E};
    static const uint8_t glyph_C[7]     = {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E};
    static const uint8_t glyph_D[7]     = {0x1E, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1E};
    static const uint8_t glyph_E[7]     = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F};
    static const uint8_t glyph_F[7]     = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10};
    static const uint8_t glyph_G[7]     = {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F};
    static const uint8_t glyph_H[7]     = {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
    static const uint8_t glyph_I[7]     = {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E};
    static const uint8_t glyph_K[7]     = {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11};
    static const uint8_t glyph_L[7]     = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F};
    static const uint8_t glyph_M[7]     = {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11};
    static const uint8_t glyph_N[7]     = {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11};
    static const uint8_t glyph_O[7]     = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    static const uint8_t glyph_P[7]     = {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10};
    static const uint8_t glyph_R[7]     = {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11};
    static const uint8_t glyph_S[7]     = {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E};
    static const uint8_t glyph_T[7]     = {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
    static const uint8_t glyph_U[7]     = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    static const uint8_t glyph_W[7]     = {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A};
    static const uint8_t glyph_Y[7]     = {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04};
    static const uint8_t glyph_dash[7]  = {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00};

    switch (c) {
    case '0': return glyph_0;     // Digit 0.
    case '1': return glyph_1;     // Digit 1.
    case '2': return glyph_2;     // Digit 2.
    case '3': return glyph_3;     // Digit 3.
    case '4': return glyph_4;     // Digit 4.
    case '5': return glyph_5;     // Digit 5.
    case '6': return glyph_6;     // Digit 6.
    case '7': return glyph_7;     // Digit 7.
    case '8': return glyph_8;     // Digit 8.
    case '9': return glyph_9;     // Digit 9.
    case 'A': return glyph_A;     // Letter A.
    case 'B': return glyph_B;     // Letter B.
    case 'C': return glyph_C;     // Letter C.
    case 'D': return glyph_D;     // Letter D.
    case 'E': return glyph_E;     // Letter E.
    case 'F': return glyph_F;     // Letter F.
    case 'G': return glyph_G;     // Letter G.
    case 'H': return glyph_H;     // Letter H.
    case 'I': return glyph_I;     // Letter I.
    case 'K': return glyph_K;     // Letter K.
    case 'L': return glyph_L;     // Letter L.
    case 'M': return glyph_M;     // Letter M.
    case 'N': return glyph_N;     // Letter N.
    case 'O': return glyph_O;     // Letter O.
    case 'P': return glyph_P;     // Letter P.
    case 'R': return glyph_R;     // Letter R.
    case 'S': return glyph_S;     // Letter S.
    case 'T': return glyph_T;     // Letter T.
    case 'U': return glyph_U;     // Letter U.
    case 'W': return glyph_W;     // Letter W.
    case 'Y': return glyph_Y;     // Letter Y.
    case '-': return glyph_dash;  // Dash character.
    case ' ': return glyph_space; // Space character.
    default:  return glyph_space; // Fallback for unsupported characters.
    }
}


/*----------------------------------------------------------------------------------------*/
/* Function: GPU_Init
 * Purpose:  Initialize the LCD controller and clear the display.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void GPU_Init(void) {
    lcd_drv->Init(); // Initialize the hardware LCD driver.
    GPU_FillScreen(COLOR_BLACK); // Start from a blank black screen.
}


/*----------------------------------------------------------------------------------------*/
/* Function: GPU_FillScreen
 * Purpose:  Fill the full LCD with a solid RGB565 color.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void GPU_FillScreen(uint16_t color) {
    lcd_drv->FillRect(0, 0, SCR_WIDTH, SCR_HEIGHT, color); // Fill the full display area.
}


/*----------------------------------------------------------------------------------------*/
/* Function: GPU_FillRect
 * Purpose:  Fill a rectangle after clipping it against the physical screen bounds.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void GPU_FillRect(int x, int y, int w, int h, uint16_t color) {
    if (w <= 0 || h <= 0) {
        return; // Ignore empty rectangles.
    }

    if (x < 0) {
        w += x; // Shrink width by the amount clipped off-screen.
        x = 0;  // Snap X to the left edge of the display.
    }
    if (y < 0) {
        h += y; // Shrink height by the amount clipped off-screen.
        y = 0;  // Snap Y to the top edge of the display.
    }
    if (x + w > SCR_WIDTH) {
        w = SCR_WIDTH - x; // Clip width at the right edge.
    }
    if (y + h > SCR_HEIGHT) {
        h = SCR_HEIGHT - y; // Clip height at the bottom edge.
    }
    if (w > 0 && h > 0) {
        lcd_drv->FillRect(x, y, w, h, color); // Draw only if a visible region remains.
    }
}


/*----------------------------------------------------------------------------------------*/
/* Function: GPU_DrawPixel
 * Purpose:  Plot a single pixel if it falls inside the visible display area.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void GPU_DrawPixel(int x, int y, uint16_t color) {
    if (x >= 0 && x < SCR_WIDTH && y >= 0 && y < SCR_HEIGHT) {
        lcd_drv->WritePixel(x, y, color); // Send one visible pixel to the LCD.
    }
}


/*----------------------------------------------------------------------------------------*/
/* Function: GPU_DrawLine
 * Purpose:  Draw a line directly to the display using a CPU-side Bresenham algorithm,
 *           which is sufficient for the current wireframe scene and UI overlays.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void GPU_DrawLine(int x1, int y1, int x2, int y2, uint16_t color) {
    int dx = abs(x2 - x1);       // Horizontal distance magnitude.
    int sx = x1 < x2 ? 1 : -1;   // Direction to step in X.
    int dy = -abs(y2 - y1);      // Negative vertical distance for Bresenham math.
    int sy = y1 < y2 ? 1 : -1;   // Direction to step in Y.
    int err = dx + dy;           // Bresenham accumulated error term.
    int e2;                      // Doubled error used for branch decisions.

    while (1) {
        GPU_DrawPixel(x1, y1, color); // Plot the current line point.
        if (x1 == x2 && y1 == y2) break; // Stop once the endpoint is reached.
        e2 = 2 * err; // Double the error to test which axis to step along.
        if (e2 >= dy) {
            err += dy; // Update accumulated error after an X step.
            x1 += sx;  // Move one pixel toward the endpoint in X.
        }
        if (e2 <= dx) {
            err += dx; // Update accumulated error after a Y step.
            y1 += sy;  // Move one pixel toward the endpoint in Y.
        }
    }
}


/*----------------------------------------------------------------------------------------*/
/* Function: GPU_DrawChar
 * Purpose:  Draw a single character from the built-in bitmap font.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void GPU_DrawChar(int x, int y, char c, uint16_t color, int scale) {
    int row;                // Current glyph row.
    int col;                // Current glyph column.
    const uint8_t *glyph;   // Pointer to the selected 5x7 bitmap.

    if (scale < 1) {
        scale = 1; // Prevent invalid scale values.
    }

    glyph = GPU_GetGlyph(c); // Look up the glyph bitmap for this character.
    for (row = 0; row < 7; row++) {
        for (col = 0; col < 5; col++) {
            if (glyph[row] & (1U << (4 - col))) {
                // Draw one scaled pixel block.
                GPU_FillRect(x + (col * scale), y + (row * scale), scale, scale, color);
            }
        }
    }
}


/*----------------------------------------------------------------------------------------*/
/* Function: GPU_DrawText
 * Purpose:  Draw a text string with uniform glyph spacing.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void GPU_DrawText(int x, int y, const char *text, uint16_t color, int scale) {
    int cursor_x = x; // Current X position for the next glyph.

    if (scale < 1) {
        scale = 1; // Prevent invalid scale values.
    }

    while (*text != '\0') {
        GPU_DrawChar(cursor_x, y, *text, color, scale); // Draw the current character.
        cursor_x += 6 * scale; // Advance by glyph width plus one pixel of spacing.
        text++; // Move to the next character in the string.
    }
}


/*----------------------------------------------------------------------------------------*/
/* Function: GPU_Render
 * Purpose:  Execute the caller-provided scene render callback.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void GPU_Render(void (*draw_callback)(void)) {
    if (draw_callback) {
        draw_callback(); // Execute the caller's drawing routine if one was provided.
    }
}
