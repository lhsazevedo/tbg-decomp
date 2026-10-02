#ifndef _024B4C_BUS_CAMERA_H
#define _024B4C_BUS_CAMERA_H

#include <shinobi.h>

/* Values of var_cameraMode_8c227d9c. 0-4 are the gameplay camera
 * (BusCameraUpdate_8c025078); the Y button cycles only 0-3
 * (BUS_CAMERA_FIXED_TARGET is never assigned anywhere in the decompiled
 * code -- traced as dead in the shipped game, not merely hard to reach).
 * 5-7 are demo-playback-only (DemoUpdateCamera_8c025906, 025870_demo.c),
 * selected per DemoShot.cameraKind_0x00 by demoShotTask_8c0259e8. */
enum {
    BUS_CAMERA_COCKPIT = 0,          /* at the bus origin; dashboard model
                                       * drawn; eased yaw sway with steering */
    BUS_CAMERA_FIRST_PERSON = 1,     /* offset ahead of the bus; no bus model
                                       * drawn; gentle heading-based bob */
    BUS_CAMERA_THIRD_PERSON_NEAR = 2, /* chase via positionCamera_8c024d6c, dist 18 */
    BUS_CAMERA_THIRD_PERSON_FAR = 3,  /* chase via positionCamera_8c024d6c, dist 30 */
    BUS_CAMERA_FIXED_TARGET = 4,      /* fixed on var_fixedCameraTarget_8c227d90; unreachable */
    BUS_CAMERA_DEMO_WORLD_POINT = 5,
    BUS_CAMERA_DEMO_LOCAL_POINT = 6,
    BUS_CAMERA_DEMO_CHASE = 7,
};

/* =======================
 * Non-initialized Globals
 * =======================
 */

/* Fixed camera-interest point for BusCameraUpdate_8c025078's
 * var_cameraMode_8c227d9c==4 mode. */
extern float var_fixedCameraTarget_8c227d90[3];
extern int var_cameraMode_8c227d9c;
extern Uint32 var_savedCameraMode_8c227da0;
extern int var_cameraCueState_8c227da4; /* shifted into var_savedCameraCueState_8c227da8 each frame alongside var_cameraMode_8c227d9c->var_savedCameraMode_8c227da0 */
extern int var_savedCameraCueState_8c227da8;
extern int var_cameraCueBusy_8c227dac; /* zeroed alongside var_cameraMode_8c227d9c by placeBus_8c023310 for a normal run */
/* Door-timer counter driven by BusTask_8c022bdc (022bdc): counts up by 0.5/frame
 * while boarding (doorState_0x3c0==1), capped at var_busDoorLastFrame_8c227db4, then counts
 * back down by 0.5/frame once departing (doorState_0x3c0==3) until it hits 0. */
extern float var_busDoorFrame_8c227db0;
/* var_busDoorMotion_8c1bc410->nbFrame - 1.0, set by BusInitStart_8c023610, read by BusTask_8c022bdc
 * (022bdc). */
extern float var_busDoorLastFrame_8c227db4;
extern float var_busSimpleLightDir_8c227db8[3]; // 028258: light direction (x, y, z), written by BusCameraUpdate_8c025078
extern float var_mirrorLightDir_8c227dc4[3];
extern float var_farClipDepth_8c227dd0;
/* Which attract-mode shot is showing, so demoShotTask_8c0259e8 only cuts on a
 * change. 0129cc_game.c pre-seeds it with the course's opening shot from
 * init_demoFirstShot_8c0460b0 (025870_demo.h). */
extern int var_demoShotId_8c227dd4;

/* =========
 * Functions
 * =========
 */

/* Copies the camera mode and the cue-ramp state into their var_saved*
 * counterparts, around the stop scene. Saved by 02b464 on arrival,
 * restored by 02d19c on departure. */
void BusCameraSaveState_8c024b4c(void);

/* Restores what BusCameraSaveState_8c024b4c saved, except the ramp
 * phase (var_savedCameraHeightPhase_8c227dfc), then tail-calls
 * BusCameraApplyMode_8c024f32. */
void BusCameraRestoreState_8c024b86(void);

/* Re-seats busState's camera draw position (posX_0x2fc/posZ_0x304) behind
 * the bus at the current mode's chase distance, and resets that mode's
 * default camera height. Call after assigning var_cameraMode_8c227d9c. */
void BusCameraApplyMode_8c024f32(void);

/* Lights, textures and draws the third-person bus model (with door/etc
 * shape motion, always busState.modelLarge_0x00c); altLight only selects the
 * light direction (non-NULL -> var_mirrorLightDir_8c227dc4). Called by 025870_demo. */
void BusCameraDrawBusModel_8c024bb8(void *altLight);

/* Updates the gameplay camera to follow the player's bus, outside demo
 * playback (var_playMode_8c1bb8d0 != PLAY_MODE_DEMO). */
void BusCameraUpdate_8c025078(void);

/* No-op unless busState.mirror_0x268 is nonzero. Otherwise picks a
 * local mirror-camera offset/interest by mirror_0x268 (1/2/3, else stale),
 * rotates the offset into world space by the bus's world matrix
 * (worldMatrix_0x084), positions the separate mirror camera (var_mirrorCamera_8c1bb944)
 * there, points its interest at the same-rotated per-mode interest
 * vector, rolls it by recent Y waypoint history, activates it, and queues
 * BusCameraDrawBusModel_8c024bb8 on draw layer 1 with the alt light direction
 * (var_mirrorLightDir_8c227dc4, recomputed here from the course's primary light). */
void BusCameraUpdateMirror_8c025604(void);

#endif // _024B4C_BUS_CAMERA_H
