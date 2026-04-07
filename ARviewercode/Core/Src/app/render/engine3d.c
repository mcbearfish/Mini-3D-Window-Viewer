/*----------------------------------------------------------------------------------------*/
/*
 * File:    engine3d.c
 * Purpose: Lightweight direct-draw 3D wireframe renderer for the hammer demo.
 *          It stores the scene geometry, applies object rotation, projects vertices,
 *          and redraws the LCD each frame.
 *
 * Functions contained in this file:
 * - Engine3D_Init: Initializes the renderer and LCD subsystem.
 * - Engine3D_SetCameraPose: Stores the camera position for the next frame.
 * - Engine3D_SetObjectRotation: Stores the object quaternion for the next frame.
 * - quat_to_matrix: Converts a quaternion to a 3x3 rotation matrix.
 * - mount_matrix: Returns the fixed display-mount alignment matrix.
 * - mat3_mul: Multiplies two 3x3 matrices.
 * - RotateObjectVertex: Rotates one model-space vertex into display-aligned space.
 * - TransformToCameraSpace: Moves a world vertex into camera space.
 * - ProjectPoint: Perspective-projects a 3D point to the LCD.
 * - Engine3D_PrepareFrame: Builds the projected line list for the current frame.
 * - Engine3D_DrawScene: Erases the old frame and draws the new frame.
 */
/*----------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------*/
// INCLUDES
/*----------------------------------------------------------------------------------------*/
#include "engine3d.h"


/*----------------------------------------------------------------------------------------*/
// CONSTANTS
/*----------------------------------------------------------------------------------------*/
/* Capacity for the wireframe line list used each frame. */
#define MAX_SCENE_LINES 100
#define CAMERA_BASE_HEIGHT_IN 3.8f
#define CAMERA_MIN_HEIGHT_IN 2.4f
#define CAMERA_MAX_HEIGHT_IN 8.0f


/*----------------------------------------------------------------------------------------*/
// GLOBAL VARIABLES
/*----------------------------------------------------------------------------------------*/
/* A 2D line that has already been projected to screen space. */
typedef struct {
    Point2D p1;
    Point2D p2;
    uint16_t color;
} ProjectedLine;

/* Current frame line list plus the previous frame used for erase/redraw. */
static ProjectedLine projected_lines[MAX_SCENE_LINES];
static ProjectedLine previous_lines[MAX_SCENE_LINES];
static int line_count = 0;
static int previous_line_count = 0;

/* Camera state in world space and the current object orientation. */
static float cam_x=0, cam_y=0, cam_z=CAMERA_BASE_HEIGHT_IN;
static float obj_q0=1, obj_q1=0, obj_q2=0, obj_q3=0;

/* Colored line segment stored by vertex index. */
typedef struct {
    uint8_t a;
    uint8_t b;
    uint16_t color;
} SceneEdge;

/* Hammer wireframe vertices in object-local space. */
static Vec3 scene_verts[] = {
    {-0.28f, -1.55f, -0.12f}, { 0.08f, -1.55f, -0.12f}, { 0.08f,  0.55f, -0.12f}, {-0.28f,  0.55f, -0.12f},
    {-0.28f, -1.55f,  0.12f}, { 0.08f, -1.55f,  0.12f}, { 0.08f,  0.55f,  0.12f}, {-0.28f,  0.55f,  0.12f},
    {-0.95f,  0.45f, -0.28f}, { 0.75f,  0.45f, -0.28f}, { 0.75f,  0.90f, -0.28f}, {-0.95f,  0.90f, -0.28f},
    {-0.95f,  0.45f,  0.28f}, { 0.75f,  0.45f,  0.28f}, { 0.75f,  0.90f,  0.28f}, {-0.95f,  0.90f,  0.28f},
    {-1.28f,  0.58f, -0.16f}, {-0.95f,  0.45f, -0.28f}, {-0.95f,  0.90f, -0.28f},
    {-1.28f,  0.58f,  0.16f}, {-0.95f,  0.45f,  0.28f}, {-0.95f,  0.90f,  0.28f}
};

/* Hammer wireframe edges referencing the vertex list above. */
static SceneEdge scene_edges[] = {
    {0,1, COLOR_YELLOW}, {1,2, COLOR_YELLOW}, {2,3, COLOR_YELLOW}, {3,0, COLOR_YELLOW},
    {4,5, COLOR_YELLOW}, {5,6, COLOR_YELLOW}, {6,7, COLOR_YELLOW}, {7,4, COLOR_YELLOW},
    {0,4, COLOR_YELLOW}, {1,5, COLOR_YELLOW}, {2,6, COLOR_YELLOW}, {3,7, COLOR_YELLOW},
    {8,9, COLOR_WHITE}, {9,10, COLOR_WHITE}, {10,11, COLOR_WHITE}, {11,8, COLOR_WHITE},
    {12,13, COLOR_WHITE}, {13,14, COLOR_WHITE}, {14,15, COLOR_WHITE}, {15,12, COLOR_WHITE},
    {8,12, COLOR_WHITE}, {9,13, COLOR_WHITE}, {10,14, COLOR_WHITE}, {11,15, COLOR_WHITE},
    {16,17, COLOR_WHITE}, {16,18, COLOR_WHITE}, {19,20, COLOR_WHITE}, {19,21, COLOR_WHITE},
    {16,19, COLOR_WHITE}, {17,20, COLOR_WHITE}, {18,21, COLOR_WHITE}
};

static void quat_to_matrix(float q0, float q1, float q2, float q3, float R[3][3]);
static void mount_matrix(float M[3][3]);
static void mat3_mul(float out[3][3], float A[3][3], float B[3][3]);
static Vec3 RotateObjectVertex(Vec3 v);
static Vec3 TransformToCameraSpace(Vec3 v);
static int ProjectPoint(Vec3 v, Point2D *out);


/*----------------------------------------------------------------------------------------*/
/* Function: Engine3D_Init
 * Purpose:  Initialize the renderer and LCD subsystem.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void Engine3D_Init(void) {
    GPU_Init(); // Bring up the LCD and clear the display.
}


/*----------------------------------------------------------------------------------------*/
/* Function: Engine3D_SetCameraPose
 * Purpose:  Update the camera position used by the next projection pass.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void Engine3D_SetCameraPose(float x, float y, float z) {
    cam_x = x; // Store camera X position.
    cam_y = y; // Store camera Y position.
    cam_z = z; // Store camera Z position.

    if (cam_z < CAMERA_MIN_HEIGHT_IN) {
        cam_z = CAMERA_MIN_HEIGHT_IN; // Clamp camera so it cannot move unrealistically close.
    }
    if (cam_z > CAMERA_MAX_HEIGHT_IN) {
        cam_z = CAMERA_MAX_HEIGHT_IN; // Clamp camera so it cannot move unrealistically far away.
    }
}


/*----------------------------------------------------------------------------------------*/
/* Function: Engine3D_SetObjectRotation
 * Purpose:  Update the object's orientation quaternion.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void Engine3D_SetObjectRotation(float q0, float q1, float q2, float q3) {
    obj_q0 = q0; // Store quaternion scalar term.
    obj_q1 = q1; // Store quaternion X term.
    obj_q2 = q2; // Store quaternion Y term.
    obj_q3 = q3; // Store quaternion Z term.
}


/*----------------------------------------------------------------------------------------*/
/* Function: quat_to_matrix
 * Purpose:  Convert a quaternion into a 3x3 rotation matrix.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
static void quat_to_matrix(float q0, float q1, float q2, float q3, float R[3][3]) {
    R[0][0] = 1.0f - 2.0f * (q2*q2 + q3*q3); // Row 0, column 0.
    R[0][1] = 2.0f * (q1*q2 - q0*q3);        // Row 0, column 1.
    R[0][2] = 2.0f * (q1*q3 + q0*q2);        // Row 0, column 2.

    R[1][0] = 2.0f * (q1*q2 + q0*q3);        // Row 1, column 0.
    R[1][1] = 1.0f - 2.0f * (q1*q1 + q3*q3); // Row 1, column 1.
    R[1][2] = 2.0f * (q2*q3 - q0*q1);        // Row 1, column 2.

    R[2][0] = 2.0f * (q1*q3 - q0*q2);        // Row 2, column 0.
    R[2][1] = 2.0f * (q2*q3 + q0*q1);        // Row 2, column 1.
    R[2][2] = 1.0f - 2.0f * (q1*q1 + q2*q2); // Row 2, column 2.
}


/*----------------------------------------------------------------------------------------*/
/* Function: mount_matrix
 * Purpose:  Return the fixed transform that maps the physical display mount into the
 *           renderer's expected coordinate system.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
static void mount_matrix(float M[3][3]) {
    M[0][0] = 1.0f;  M[0][1] = 0.0f;  M[0][2] = 0.0f;  // X axis maps directly.
    M[1][0] = 0.0f;  M[1][1] = 1.0f;  M[1][2] = 0.0f;  // Y axis maps directly.
    M[2][0] = 0.0f;  M[2][1] = 0.0f;  M[2][2] = -1.0f; // Flip Z so display space matches the intended view.
}


/*----------------------------------------------------------------------------------------*/
/* Function: mat3_mul
 * Purpose:  Multiply two 3x3 matrices.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
static void mat3_mul(float out[3][3], float A[3][3], float B[3][3]) {
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            out[r][c] = A[r][0] * B[0][c] + // Row r of A times column c of B, first term.
                        A[r][1] * B[1][c] + // Second multiply-accumulate term.
                        A[r][2] * B[2][c];  // Third multiply-accumulate term.
        }
    }
}


/*----------------------------------------------------------------------------------------*/
/* Function: RotateObjectVertex
 * Purpose:  Rotate a model-space vertex into the display-aligned object pose before
 *           projection to the screen.
 * Returns:  The rotated vertex in display-aligned space.
 */
/*----------------------------------------------------------------------------------------*/
static Vec3 RotateObjectVertex(Vec3 v) {
    float R_world_to_body[3][3]; // Rotation from world/object orientation into body space.
    float R_mount[3][3];         // Fixed display mount correction.
    float R_display[3][3];       // Final rotation used for rendering.
    Vec3 res;                    // Rotated output vertex.

    // Convert object quaternion to matrix form.
    quat_to_matrix(obj_q0, obj_q1, obj_q2, obj_q3, R_world_to_body);
    // Load fixed display alignment matrix.
    mount_matrix(R_mount);
    // Combine mount alignment with current object rotation.
    mat3_mul(R_display, R_mount, R_world_to_body);

    // Use the transposed basis so the object appears to tumble with the box.
    res.x = R_display[0][0] * v.x + R_display[1][0] * v.y + R_display[2][0] * v.z; // Rotated X coordinate.
    res.y = R_display[0][1] * v.x + R_display[1][1] * v.y + R_display[2][1] * v.z; // Rotated Y coordinate.
    res.z = R_display[0][2] * v.x + R_display[1][2] * v.y + R_display[2][2] * v.z; // Rotated Z coordinate.
    return res; // Return rotated model-space vertex.
}


/*----------------------------------------------------------------------------------------*/
/* Function: TransformToCameraSpace
 * Purpose:  Move a world-space vertex into the camera's local coordinate system.
 * Returns:  The vertex expressed in camera space.
 */
/*----------------------------------------------------------------------------------------*/
static Vec3 TransformToCameraSpace(Vec3 v) {
    float tx = v.x - cam_x; // Translate vertex relative to camera X position.
    float ty = v.y - cam_y; // Translate vertex relative to camera Y position.
    float tz = v.z - cam_z; // Translate vertex relative to camera Z position.

    Vec3 res;       // Camera-space result.
    res.x = tx;     // Camera X points the same way as world X.
    res.y = ty;     // Camera Y points the same way as world Y.
    res.z = -tz;    // Flip Z so points in front of the camera have positive depth.
    return res;     // Return translated camera-space point.
}


/*----------------------------------------------------------------------------------------*/
/* Function: ProjectPoint
 * Purpose:  Perspective-project a 3D point onto the LCD.
 * Returns:  1 if the point projects successfully, otherwise 0.
 */
/*----------------------------------------------------------------------------------------*/
static int ProjectPoint(Vec3 v, Point2D *out) {
    Vec3 cam_v = TransformToCameraSpace(v); // Move point into camera coordinates first.
    if (cam_v.z < 0.1f) return 0;           // Reject points behind or too close to the near plane.

    float fov = 280.0f;           // Projection scale factor controlling apparent zoom.
    int cx = SCR_WIDTH / 2;       // Screen center X.
    int cy = SCR_HEIGHT / 2;      // Screen center Y.
    float inv_z = 1.0f / cam_v.z; // Inverse depth used for perspective division.

    out->x = cx + (int)(cam_v.x * fov * inv_z); // Project X from camera space into screen space.
    out->y = cy - (int)(cam_v.y * fov * inv_z); // Project Y and flip it for screen coordinates.
    return 1; // Projection succeeded.
}


/*----------------------------------------------------------------------------------------*/
/* Function: Engine3D_PrepareFrame
 * Purpose:  Build the list of projected line segments for the current frame.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void Engine3D_PrepareFrame(void) {
    for (int i = 0; i < line_count; i++) {
        // Save current frame so it can be erased on the next draw.
        previous_lines[i] = projected_lines[i];
    }
    previous_line_count = line_count; // Record how many lines belong to the old frame.
    line_count = 0; // Start building a fresh frame line list.

    // Projected screen positions for each vertex.
    Point2D proj_verts[sizeof(scene_verts) / sizeof(scene_verts[0])];
    // Flags telling whether each vertex projected successfully.
    uint8_t valid_verts[sizeof(scene_verts) / sizeof(scene_verts[0])];

    for(int i=0; i<(int)(sizeof(scene_verts) / sizeof(scene_verts[0])); i++) {
        // Rotate model vertex into its current orientation.
        Vec3 v = RotateObjectVertex(scene_verts[i]);
        // Project the rotated vertex to screen space.
        valid_verts[i] = ProjectPoint(v, &proj_verts[i]);
    }

    for(int i=0; i<(int)(sizeof(scene_edges) / sizeof(scene_edges[0])); i++) {
        if (valid_verts[scene_edges[i].a] && valid_verts[scene_edges[i].b]) {
            if (line_count < MAX_SCENE_LINES) {
                projected_lines[line_count].p1 = proj_verts[scene_edges[i].a]; // First endpoint of projected edge.
                projected_lines[line_count].p2 = proj_verts[scene_edges[i].b]; // Second endpoint of projected edge.
                projected_lines[line_count].color = scene_edges[i].color;      // Store the edge color.
                line_count++; // One more line is now ready to draw.
            }
        }
    }
}


/*----------------------------------------------------------------------------------------*/
/* Function: Engine3D_DrawScene
 * Purpose:  Erase the previous frame and draw the new frame. The renderer uses a simple
 *           direct-draw strategy rather than a framebuffer.
 * Returns:  none
 */
/*----------------------------------------------------------------------------------------*/
void Engine3D_DrawScene(void) {
    for(int i=0; i<previous_line_count; i++) {
        // Erase the old line by redrawing it in black.
        GPU_DrawLine(previous_lines[i].p1.x, previous_lines[i].p1.y,
                     previous_lines[i].p2.x, previous_lines[i].p2.y,
                     COLOR_BLACK);
    }

    for(int i=0; i<line_count; i++) {
        // Draw the new line in its assigned color.
        GPU_DrawLine(projected_lines[i].p1.x, projected_lines[i].p1.y,
                     projected_lines[i].p2.x, projected_lines[i].p2.y,
                     projected_lines[i].color);
    }
}
