#ifndef _024B4C_BUS_CAMERA_H
#define _024B4C_BUS_CAMERA_H

#include <shinobi.h>

/* Values of var_cameraMode_8c227d9c. The Y button cycles 0-3; 5-7 are
 * attract-mode only. */
enum {
    BUS_CAMERA_COCKPIT = 0,           /* yaw sways with steering */
    BUS_CAMERA_FIRST_PERSON = 1,      /* no dashboard */
    BUS_CAMERA_THIRD_PERSON_NEAR = 2,
    BUS_CAMERA_THIRD_PERSON_FAR = 3,
    BUS_CAMERA_FIXED_TARGET = 4,      /* never assigned */
    BUS_CAMERA_DEMO_WORLD_POINT = 5,
    BUS_CAMERA_DEMO_LOCAL_POINT = 6,
    BUS_CAMERA_DEMO_CHASE = 7,
};

/* =======================
 * Non-initialized Globals
 * =======================
 */

extern float var_fixedCameraTarget_8c227d90[3];
extern int var_cameraMode_8c227d9c;
extern Uint32 var_savedCameraMode_8c227da0;
extern int var_cameraCueState_8c227da4;
extern int var_savedCameraCueState_8c227da8;
extern int var_cameraCueBusy_8c227dac; /* blocks the outside views' Y cycle */
/* Door animation frame: opens at 0.5/frame, closes back to 0. */
extern float var_busDoorFrame_8c227db0;
extern float var_busDoorLastFrame_8c227db4;
extern float var_busSimpleLightDir_8c227db8[3];
extern float var_mirrorLightDir_8c227dc4[3];
extern float var_farClipDepth_8c227dd0;
extern int var_demoShotId_8c227dd4;

/* =========
 * Functions
 * =========
 */

/* Kept across the stop scene. */
void BusCameraSaveState_8c024b4c(void);

/* Everything but the ramp phase. */
void BusCameraRestoreState_8c024b86(void);

/* Call after assigning var_cameraMode_8c227d9c. */
void BusCameraApplyMode_8c024f32(void);

/* Non-NULL altLight selects the mirror light direction. */
void BusCameraDrawBusModel_8c024bb8(void *altLight);

void BusCameraUpdate_8c025078(void);

void BusCameraUpdateMirror_8c025604(void);

#endif // _024B4C_BUS_CAMERA_H
