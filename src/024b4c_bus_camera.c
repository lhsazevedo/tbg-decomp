/* @unit BusCamera */

#include <shinobi.h>
#include "includes.h" /* TWO_PI */

#include "serial_debug.h"
#include "013ae8_route.h"
#include "1ba1c8_globals.h"
#include "027958_bus_draw.h"
#include "022464_render.h"
#include "024b4c_bus_camera.h"

/* ====================
 * Non-initialized Globals
 * ====================
 */

float var_fixedCameraTarget_8c227d90[3];
int var_cameraMode_8c227d9c;
Uint32 var_savedCameraMode_8c227da0;
int var_cameraCueState_8c227da4;
int var_savedCameraCueState_8c227da8;
int var_cameraCueBusy_8c227dac;
float var_busDoorFrame_8c227db0;
float var_busDoorLastFrame_8c227db4;
float var_busSimpleLightDir_8c227db8[3];
float var_mirrorLightDir_8c227dc4[3];
float var_farClipDepth_8c227dd0;
int var_demoShotId_8c227dd4;

STATIC float var_cameraHeightFrom_8c227dd8;
STATIC float var_savedCameraHeightFrom_8c227ddc;
STATIC float var_cameraHeightTo_8c227de0;
STATIC float var_savedCameraHeightTo_8c227de4;
STATIC float var_cameraHeightDelta_8c227de8;
STATIC float var_savedCameraHeightDelta_8c227dec;
/* Chase camera height above the bus: 5.0 near / 18.0 far by default. */
STATIC float var_cameraHeight_8c227df0;
STATIC float var_savedCameraHeight_8c227df4;
/* Quarter-sine ease angle, BAM 0..0x4000. */
STATIC Sint32 var_cameraHeightPhase_8c227df8;
STATIC Sint32 var_savedCameraHeightPhase_8c227dfc;

/* ====================
 * Forward Declarations
 * ====================
 */

STATIC void positionCamera_8c024d6c(float dist, float dyOffset, float interestDyOffset);

/* ====================
 * Functions
 * ====================
 */

void BusCameraSaveState_8c024b4c(void)
{
    var_savedCameraMode_8c227da0 = var_cameraMode_8c227d9c;
    var_savedCameraCueState_8c227da8 = var_cameraCueState_8c227da4;
    var_savedCameraHeightFrom_8c227ddc = var_cameraHeightFrom_8c227dd8;
    var_savedCameraHeightTo_8c227de4 = var_cameraHeightTo_8c227de0;
    var_savedCameraHeightDelta_8c227dec = var_cameraHeightDelta_8c227de8;
    var_savedCameraHeight_8c227df4 = var_cameraHeight_8c227df0;
    var_savedCameraHeightPhase_8c227dfc = var_cameraHeightPhase_8c227df8;
}

void BusCameraRestoreState_8c024b86(void)
{
    var_cameraMode_8c227d9c = var_savedCameraMode_8c227da0;
    var_cameraCueState_8c227da4 = var_savedCameraCueState_8c227da8;
    var_cameraHeightFrom_8c227dd8 = var_savedCameraHeightFrom_8c227ddc;
    var_cameraHeightTo_8c227de0 = var_savedCameraHeightTo_8c227de4;
    var_cameraHeightDelta_8c227de8 = var_savedCameraHeightDelta_8c227dec;
    var_cameraHeight_8c227df0 = var_savedCameraHeight_8c227df4;

    BusCameraApplyMode_8c024f32();
}

void BusCameraApplyMode_8c024f32(void)
{
    float dx = 0.0f;
    float dz = 0.0f;

    switch (var_cameraMode_8c227d9c) {
    case BUS_CAMERA_COCKPIT:
        var_busState_8c1bb9d0.cameraYawEase_0x3c8 = 0;
        return;
    case BUS_CAMERA_FIRST_PERSON:
    case BUS_CAMERA_FIXED_TARGET:
        return;
    case BUS_CAMERA_THIRD_PERSON_NEAR:
        dx = var_busState_8c1bb9d0.headingDirX_0x274 * 18.0f;
        dz = var_busState_8c1bb9d0.headingDirZ_0x278 * 18.0f;
        var_cameraHeight_8c227df0 = (var_cameraCueState_8c227da4 != 0) ? var_cameraHeightTo_8c227de0 : 5.0f;
        break;
    case BUS_CAMERA_THIRD_PERSON_FAR:
        dx = var_busState_8c1bb9d0.headingDirX_0x274 * 30.0f;
        dz = var_busState_8c1bb9d0.headingDirZ_0x278 * 30.0f;
        var_cameraHeight_8c227df0 = (var_cameraCueState_8c227da4 != 0) ? var_cameraHeightTo_8c227de0 : 18.0f;
        break;
    }

    var_busState_8c1bb9d0.posX_0x2fc = var_busState_8c1bb9d0.posX_0x0f4 + dx;
    var_busState_8c1bb9d0.posZ_0x304 = var_busState_8c1bb9d0.posZ_0x0fc + dz;
}

void BusCameraDrawBusModel_8c024bb8(void *altLight)
{
    float *dir = altLight ? var_mirrorLightDir_8c227dc4 : var_busSimpleLightDir_8c227db8;
    float frame = (var_busState_8c1bb9d0.doorState_0x3c0 != 0) ? var_busDoorFrame_8c227db0 : 0.0f;

    njCnkSetSimpleLight(dir[0], dir[1], dir[2]);
    njCnkSetSimpleLightIntensity(var_busState_8c1bb9d0.lightCoeffRow_0x0c4[0],
                                  var_busState_8c1bb9d0.lightCoeffRow_0x0c4[1]);
    njCnkSetSimpleLightColor(var_busState_8c1bb9d0.lightCoeffRow_0x0c4[2],
                              var_busState_8c1bb9d0.lightCoeffRow_0x0c4[3],
                              var_busState_8c1bb9d0.lightCoeffRow_0x0c4[4]);
    BusDrawUpdateModels_8c027958(&var_busState_8c1bb9d0);
    njMultiMatrix(NULL, &var_busState_8c1bb9d0.worldMatrix_0x084);
    njSetTexture((NJS_TEXLIST *)var_busState_8c1bb9d0.texlistLarge_0x004);
    njCnkSimpleDrawShapeMotion((NJS_CNK_OBJECT *)var_busState_8c1bb9d0.modelLarge_0x00c,
                                var_busDoorMotion_8c1bc410, var_busDoorShape_8c1bc414, frame);

    njControl3D(NJD_CONTROL_3D_MODEL_CLIP | NJD_CONTROL_3D_SHADOW | NJD_CONTROL_3D_TRANS_MODIFIER);
    njCnkModDrawObject((NJS_CNK_OBJECT *)var_busState_8c1bb9d0.shadowModel_0x014);
    njControl3D(NJD_CONTROL_3D_MODEL_CLIP);
}

STATIC void drawFrontBusModel_8c024cc8(void)
{
    njCnkSetSimpleLight(var_busSimpleLightDir_8c227db8[0],
                         var_busSimpleLightDir_8c227db8[1],
                         var_busSimpleLightDir_8c227db8[2]);
    njCnkSetSimpleLightIntensity(var_busState_8c1bb9d0.lightCoeffRow_0x0c4[0],
                                  var_busState_8c1bb9d0.lightCoeffRow_0x0c4[1]);
    njCnkSetSimpleLightColor(var_busState_8c1bb9d0.lightCoeffRow_0x0c4[2],
                              var_busState_8c1bb9d0.lightCoeffRow_0x0c4[3],
                              var_busState_8c1bb9d0.lightCoeffRow_0x0c4[4]);
    njMultiMatrix(NULL, &var_busState_8c1bb9d0.worldMatrix_0x084);
    njSetTexture(var_frontTexlist_8c1bc430);
    njCnkSimpleDrawObject((NJS_CNK_OBJECT *)var_frontNj_8c1bc434);
}

void BusCameraUpdate_8c025078(void)
{
    /* Scripted height cue: 9 eases back to the mode's default height, any
     * other nonzero value eases to that height. */
    Sint32 cue = var_busState_8c1bb9d0.markAudioCue_0x3b8 & 0x0F000000;
    float pitchOffset;
    Sint32 ang;

    if (var_cameraCueState_8c227da4 == 0) {
        if (cue != 0 && cue != 0x09000000) {
            var_cameraHeightTo_8c227de0 = (float)(Sint8)(cue >> 24);
            if (var_cameraMode_8c227d9c == BUS_CAMERA_COCKPIT || var_cameraMode_8c227d9c == BUS_CAMERA_FIRST_PERSON) {
                var_cameraCueState_8c227da4 = 2;
            } else if (var_cameraMode_8c227d9c != BUS_CAMERA_THIRD_PERSON_NEAR || var_cameraHeightTo_8c227de0 < 5.0f) {
                var_cameraHeightFrom_8c227dd8 = var_cameraHeight_8c227df0;
                var_cameraHeightDelta_8c227de8 = var_cameraHeight_8c227df0 - var_cameraHeightTo_8c227de0;
                var_cameraHeightPhase_8c227df8 = 0;
                var_cameraCueBusy_8c227dac = 1;
                var_cameraCueState_8c227da4 = 1;
            } else {
                var_cameraCueState_8c227da4 = 2;
            }
        }
    } else if (var_cameraCueState_8c227da4 == 1) {
        var_cameraHeightPhase_8c227df8 += 0x222;
        if (var_cameraHeightPhase_8c227df8 < 0x4000) {
            var_cameraHeight_8c227df0 = var_cameraHeightFrom_8c227dd8 - njSin(var_cameraHeightPhase_8c227df8) * var_cameraHeightDelta_8c227de8;
        } else {
            var_cameraHeight_8c227df0 = var_cameraHeightTo_8c227de0;
            var_cameraCueBusy_8c227dac = 0;
            var_cameraCueState_8c227da4 = 2;
        }
    } else if (var_cameraCueState_8c227da4 == 2) {
        if (cue == 0x09000000) {
            if (var_cameraMode_8c227d9c == BUS_CAMERA_COCKPIT || var_cameraMode_8c227d9c == BUS_CAMERA_FIRST_PERSON) {
                var_cameraCueState_8c227da4 = 0;
            } else if (var_cameraMode_8c227d9c == BUS_CAMERA_THIRD_PERSON_NEAR && var_cameraHeight_8c227df0 >= 5.0f) {
                var_cameraCueState_8c227da4 = 0;
            } else {
                var_cameraHeightTo_8c227de0 = (var_cameraMode_8c227d9c == BUS_CAMERA_THIRD_PERSON_NEAR) ? 5.0f : 18.0f;
                var_cameraHeightFrom_8c227dd8 = var_cameraHeight_8c227df0;
                var_cameraHeightDelta_8c227de8 = var_cameraHeightTo_8c227de0 - var_cameraHeight_8c227df0;
                var_cameraHeightPhase_8c227df8 = 0;
                var_cameraCueBusy_8c227dac = 1;
                var_cameraCueState_8c227da4 = 3;
            }
        }
    } else /* var_cameraCueState_8c227da4 == 3 */ {
        var_cameraHeightPhase_8c227df8 += 0x222;
        if (var_cameraHeightPhase_8c227df8 < 0x4000) {
            var_cameraHeight_8c227df0 = var_cameraHeightFrom_8c227dd8 + njSin(var_cameraHeightPhase_8c227df8) * var_cameraHeightDelta_8c227de8;
        } else {
            var_cameraHeight_8c227df0 = var_cameraHeightTo_8c227de0;
            var_cameraCueBusy_8c227dac = 0;
            var_cameraCueState_8c227da4 = 0;
        }
    }

    if ((var_peripherals_8c1ba35c[0].press & PDD_DGT_TY)
        && var_busState_8c1bb9d0.driveState_0x2b4 == 1
        && (var_cameraCueBusy_8c227dac == 0 || var_cameraMode_8c227d9c < BUS_CAMERA_THIRD_PERSON_NEAR)) {
        var_cameraMode_8c227d9c++;
        if (var_cameraMode_8c227d9c > BUS_CAMERA_THIRD_PERSON_FAR) {
            var_cameraMode_8c227d9c = BUS_CAMERA_COCKPIT;
        }
        BusCameraApplyMode_8c024f32();
    }

    njInitCamera(&var_camera_8c1bb904);
    njSetCameraAngle(&var_camera_8c1bb904, 10194);
    njSetCameraDepth(&var_camera_8c1bb904, -1.0f, var_farClipDepth_8c227dd0);

    pitchOffset = (float)var_busState_8c1bb9d0.pitchAngle_0x078 * 360.0f / 65536.0f / -8.0f;
    ang = var_busState_8c1bb9d0.rollAngle_0x07c;

    switch (var_cameraMode_8c227d9c) {
    case BUS_CAMERA_COCKPIT: {
        Sint32 target;

        if (var_progress_8c1ba1cc.screenRoll_0xc8 != 0) {
            ang = 0;
            pitchOffset = 0.0f;
        }

        var_busState_8c1bb9d0.posX_0x2fc = var_busState_8c1bb9d0.posX_0x0f4;
        var_busState_8c1bb9d0.posZ_0x304 = var_busState_8c1bb9d0.posZ_0x0fc;
        var_busState_8c1bb9d0.posY_0x300 =
            var_busState_8c1bb9d0.posY_0x0f8 + pitchOffset + 2.0f;

        target = var_busState_8c1bb9d0.ang_0x258 / 3;
        if (var_busState_8c1bb9d0.cameraYawEase_0x3c8 != 0 || target < -728 || target > 728) {
            if (var_busState_8c1bb9d0.cameraYawEase_0x3c8 < target) {
                var_busState_8c1bb9d0.cameraYawEase_0x3c8 += 60;
                if (var_busState_8c1bb9d0.cameraYawEase_0x3c8 > target) {
                    var_busState_8c1bb9d0.cameraYawEase_0x3c8 = target;
                }
            } else {
                var_busState_8c1bb9d0.cameraYawEase_0x3c8 -= 60;
                if (var_busState_8c1bb9d0.cameraYawEase_0x3c8 < target) {
                    var_busState_8c1bb9d0.cameraYawEase_0x3c8 = target;
                }
            }
        }

        njSetMatrix(&var_scratchMatrix_8c1bc46c, &var_busState_8c1bb9d0.worldMatrix_0x084);
        njRotateY(&var_scratchMatrix_8c1bc46c, var_busState_8c1bb9d0.cameraYawEase_0x3c8);

        var_groundQueryPoint_8c1bc460.x = 0.0f;
        var_groundQueryPoint_8c1bc460.y = 2.0f;
        var_groundQueryPoint_8c1bc460.z = -1.0f;
        njCalcPoint(&var_scratchMatrix_8c1bc46c, &var_groundQueryPoint_8c1bc460, &var_groundQueryPoint_8c1bc460);

        var_busState_8c1bb9d0.moveDeltaX_0x308 =
            var_busState_8c1bb9d0.posX_0x0f4 - var_groundQueryPoint_8c1bc460.x;
        var_busState_8c1bb9d0.moveDeltaY_0x30c =
            var_busState_8c1bb9d0.posY_0x0f8 - var_groundQueryPoint_8c1bc460.y;
        var_busState_8c1bb9d0.moveDeltaZ_0x310 =
            var_busState_8c1bb9d0.posZ_0x0fc - var_groundQueryPoint_8c1bc460.z;
        var_busState_8c1bb9d0.moveDeltaMagnitude_0x314 = 1.0f;

        njTranslateCameraPosition(&var_camera_8c1bb904,
                                   var_busState_8c1bb9d0.posX_0x2fc,
                                   var_busState_8c1bb9d0.posY_0x300,
                                   var_busState_8c1bb9d0.posZ_0x304);

        njPointCameraInterest(&var_camera_8c1bb904,
                               var_groundQueryPoint_8c1bc460.x,
                               var_groundQueryPoint_8c1bc460.y,
                               var_groundQueryPoint_8c1bc460.z);
        break;
    }
    case BUS_CAMERA_FIRST_PERSON:
        if (var_progress_8c1ba1cc.screenRoll_0xc8 != 0) {
            ang = 0;
            pitchOffset = 0.0f;
        }

        var_busState_8c1bb9d0.moveDeltaX_0x308 = var_busState_8c1bb9d0.headingDirX_0x274 * 1.0f;
        var_busState_8c1bb9d0.moveDeltaZ_0x310 = var_busState_8c1bb9d0.headingDirZ_0x278 * 1.0f;
        var_busState_8c1bb9d0.moveDeltaMagnitude_0x314 = 2.0f;
        var_busState_8c1bb9d0.posX_0x2fc =
            var_busState_8c1bb9d0.posX_0x0f4 + var_busState_8c1bb9d0.moveDeltaX_0x308;
        var_busState_8c1bb9d0.posZ_0x304 =
            var_busState_8c1bb9d0.posZ_0x0fc + var_busState_8c1bb9d0.moveDeltaZ_0x310;
        var_busState_8c1bb9d0.posY_0x300 =
            2.0f * (var_busState_8c1bb9d0.pitchSin_0x26c / var_busState_8c1bb9d0.pitchCos_0x270)
            + var_busState_8c1bb9d0.posY_0x0f8 + pitchOffset + 2.0f;

        njTranslateCameraPosition(&var_camera_8c1bb904,
                                   var_busState_8c1bb9d0.posX_0x2fc,
                                   var_busState_8c1bb9d0.posY_0x300,
                                   var_busState_8c1bb9d0.posZ_0x304);

        njPointCameraInterest(&var_camera_8c1bb904,
                               var_busState_8c1bb9d0.posX_0x0f4,
                               var_busState_8c1bb9d0.posY_0x0f8 + 2.0f,
                               var_busState_8c1bb9d0.posZ_0x0fc);
        break;
    case BUS_CAMERA_THIRD_PERSON_NEAR:
        positionCamera_8c024d6c(18.0f, var_cameraHeight_8c227df0, 0.5f);
        break;
    case BUS_CAMERA_THIRD_PERSON_FAR:
        positionCamera_8c024d6c(30.0f, var_cameraHeight_8c227df0, 2.0f);
        break;
    case BUS_CAMERA_FIXED_TARGET:
        var_busState_8c1bb9d0.moveDeltaX_0x308 =
            var_busState_8c1bb9d0.posX_0x2fc - var_fixedCameraTarget_8c227d90[0];
        var_busState_8c1bb9d0.moveDeltaZ_0x310 =
            var_busState_8c1bb9d0.posZ_0x304 - var_fixedCameraTarget_8c227d90[2];

        njTranslateCameraPosition(&var_camera_8c1bb904,
                                   var_busState_8c1bb9d0.posX_0x2fc,
                                   var_busState_8c1bb9d0.posY_0x300,
                                   var_busState_8c1bb9d0.posZ_0x304);
        njPointCameraInterest(&var_camera_8c1bb904, var_fixedCameraTarget_8c227d90[0], var_fixedCameraTarget_8c227d90[1], var_fixedCameraTarget_8c227d90[2]);
        break;
    default:
        break;
    }

    /* Roll by the road's pitch. */
    if (var_cameraMode_8c227d9c == BUS_CAMERA_COCKPIT || var_cameraMode_8c227d9c == BUS_CAMERA_FIRST_PERSON) {
        float roll = atan2f(var_busState_8c1bb9d0.posHistory_0x100[2].y
                             - var_busState_8c1bb9d0.posHistory_0x100[3].y,
                             2.33f);
        njRollCameraInterest(&var_camera_8c1bb904, (Sint32)(roll * 65536.0f / TWO_PI) + ang);
    }

    njSetCamera(&var_camera_8c1bb904);

    var_busSimpleLightDir_8c227db8[0] = var_sceneParams_8c18ad24->dir0_0x00[0];
    var_busSimpleLightDir_8c227db8[1] = var_sceneParams_8c18ad24->dir0_0x00[1];
    var_busSimpleLightDir_8c227db8[2] = var_sceneParams_8c18ad24->dir0_0x00[2];
    njCalcVector(NULL, (NJS_VECTOR *)var_busSimpleLightDir_8c227db8,
                 (NJS_VECTOR *)var_busSimpleLightDir_8c227db8);

    if (var_cameraMode_8c227d9c == BUS_CAMERA_COCKPIT) {
        RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, (DrawFn)drawFrontBusModel_8c024cc8, 0);
    } else if (var_cameraMode_8c227d9c == BUS_CAMERA_THIRD_PERSON_NEAR || var_cameraMode_8c227d9c == BUS_CAMERA_THIRD_PERSON_FAR) {
        RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, (DrawFn)BusCameraDrawBusModel_8c024bb8, 0);
    }
}

/* Trails the camera dist behind the bus, dyOffset above it. Past ~5 degrees
 * off the bus heading it swings round at a limited rate. */
STATIC void positionCamera_8c024d6c(float dist, float dyOffset, float interestDyOffset)
{
    NJS_POINT3 *groundPt = &var_groundQueryPoint_8c1bc460;
    float posX0 = var_busState_8c1bb9d0.posX_0x0f4;
    float posZ0 = var_busState_8c1bb9d0.posZ_0x0fc;
    float dx, dz, travel, moveX, moveZ;
    Sint32 angleInt;

    var_busState_8c1bb9d0.moveDeltaMagnitude_0x314 = dist;

    dx = var_busState_8c1bb9d0.posX_0x2fc - posX0;
    dz = var_busState_8c1bb9d0.posZ_0x304 - posZ0;
    travel = njSqrt(dx * dx + dz * dz);

    var_busState_8c1bb9d0.posX_0x2fc = posX0 + dist * (dx / travel);
    var_busState_8c1bb9d0.posY_0x300 = var_busState_8c1bb9d0.posY_0x0f8 + dyOffset;
    var_busState_8c1bb9d0.posZ_0x304 = posZ0 + dist * (dz / travel);

    moveX = var_busState_8c1bb9d0.posX_0x2fc - posX0;
    moveZ = var_busState_8c1bb9d0.posZ_0x304 - posZ0;

    angleInt = (Sint32)(acosf((moveX * var_busState_8c1bb9d0.headingX_0x230
                               + moveZ * var_busState_8c1bb9d0.headingZ_0x238)
                              / (dist * 4.9f))
                        * 65536.0f / TWO_PI);

    if (angleInt > 910) {
        Sint32 angleAfterTurn;
        Sint32 turnStep;
        float cross;

        if (angleInt > 5461) {
            angleAfterTurn = 5461;
        } else {
            Sint32 turnLimit = angleInt / 30 / 2;
            if (turnLimit < 45) {
                turnLimit = 45;
            }
            angleAfterTurn = angleInt - turnLimit;
            if (angleAfterTurn < 0) {
                angleAfterTurn = 0;
            }
        }

        turnStep = angleInt - angleAfterTurn;
        cross = moveZ * var_busState_8c1bb9d0.headingX_0x230
              - moveX * var_busState_8c1bb9d0.headingZ_0x238;
        if (cross < 0.0f) {
            turnStep = -turnStep;
        }

        groundPt->x = moveX;
        groundPt->z = moveZ;
        njUnitMatrix(&var_scratchMatrix_8c1bc46c);
        njRotateY(&var_scratchMatrix_8c1bc46c, turnStep);
        njCalcPoint(&var_scratchMatrix_8c1bc46c, groundPt,
                    (NJS_POINT3 *)&var_busState_8c1bb9d0.moveDeltaX_0x308);

        var_busState_8c1bb9d0.posX_0x2fc = posX0 + var_busState_8c1bb9d0.moveDeltaX_0x308;
        var_busState_8c1bb9d0.posZ_0x304 = posZ0 + var_busState_8c1bb9d0.moveDeltaZ_0x310;
    } else {
        var_busState_8c1bb9d0.moveDeltaX_0x308 = moveX;
        var_busState_8c1bb9d0.moveDeltaZ_0x310 = moveZ;
    }

    njTranslateCameraPosition(&var_camera_8c1bb904,
                               var_busState_8c1bb9d0.posX_0x2fc,
                               var_busState_8c1bb9d0.posY_0x300,
                               var_busState_8c1bb9d0.posZ_0x304);
    njPointCameraInterest(&var_camera_8c1bb904, posX0,
                           var_busState_8c1bb9d0.posY_0x0f8 + interestDyOffset,
                           posZ0);
}

void BusCameraUpdateMirror_8c025604(void)
{
    float offsetX, offsetY, offsetZ;
    NJS_POINT3 interest;
    float dx, dz;
    float roll;

    if (var_busState_8c1bb9d0.mirror_0x268 == MIRROR_NONE) {
        return;
    }

    njInitCamera(&var_mirrorCamera_8c1bb944);
    njSetCameraAngle(&var_mirrorCamera_8c1bb944, 9102);

    /* An out-of-range mirror_0x268 leaves offsetX and interest uninitialized,
     * as in the original. */
    offsetY = 4.2f;
    offsetZ = -14.2f;

    switch (var_busState_8c1bb9d0.mirror_0x268) {
    case MIRROR_LEFT:
        offsetX = -1.5f;
        interest.x = -2.55f;
        interest.y = 0.2f;
        interest.z = 4.0f;
        break;
    case MIRROR_RIGHT:
        offsetX = 1.5f;
        interest.x = 2.55f;
        interest.y = 0.2f;
        interest.z = 4.0f;
        break;
    case MIRROR_DOOR:
        offsetX = -2.0f;
        offsetY = 2.2f;
        offsetZ = -5.2f;
        interest.x = -0.75f;
        interest.y = 2.0f;
        interest.z = 7.6f;
        break;
    }

    var_busState_8c1bb9d0.mirrorWorldOffsetX_0x318 = offsetX;
    var_busState_8c1bb9d0.mirrorWorldOffsetY_0x31c = offsetY;
    var_busState_8c1bb9d0.mirrorWorldOffsetZ_0x320 = offsetZ;

    njSetCameraDepth(&var_mirrorCamera_8c1bb944, -1.0f, -50.0f);

    njCalcPoint(&var_busState_8c1bb9d0.worldMatrix_0x084,
                (NJS_POINT3 *)&var_busState_8c1bb9d0.mirrorWorldOffsetX_0x318,
                (NJS_POINT3 *)&var_busState_8c1bb9d0.mirrorWorldOffsetX_0x318);
    njCalcPoint(&var_busState_8c1bb9d0.worldMatrix_0x084, &interest, &interest);

    njTranslateCameraPosition(&var_mirrorCamera_8c1bb944,
                               var_busState_8c1bb9d0.mirrorWorldOffsetX_0x318,
                               var_busState_8c1bb9d0.mirrorWorldOffsetY_0x31c,
                               var_busState_8c1bb9d0.mirrorWorldOffsetZ_0x320);
    njPointCameraInterest(&var_mirrorCamera_8c1bb944, interest.x, interest.y, interest.z);

    roll = atan2f(var_busState_8c1bb9d0.posHistory_0x100[3].y
                   - var_busState_8c1bb9d0.posHistory_0x100[2].y,
                   2.33f);
    njRollCameraInterest(&var_mirrorCamera_8c1bb944, (Sint32)(roll * 65536.0f / TWO_PI));

    dx = interest.x - var_busState_8c1bb9d0.mirrorWorldOffsetX_0x318;
    dz = interest.z - var_busState_8c1bb9d0.mirrorWorldOffsetZ_0x320;
    var_busState_8c1bb9d0.mirrorDirX_0x324 = dx;
    var_busState_8c1bb9d0.mirrorDirZ_0x32c = dz;
    var_busState_8c1bb9d0.mirrorDist_0x330 = njSqrt(dx * dx + dz * dz);

    njSetCamera(&var_mirrorCamera_8c1bb944);

    var_mirrorLightDir_8c227dc4[0] = var_sceneParams_8c18ad24->dir0_0x00[0];
    var_mirrorLightDir_8c227dc4[1] = var_sceneParams_8c18ad24->dir0_0x00[1];
    var_mirrorLightDir_8c227dc4[2] = var_sceneParams_8c18ad24->dir0_0x00[2];
    njCalcVector(NULL, (NJS_VECTOR *)var_mirrorLightDir_8c227dc4, (NJS_VECTOR *)var_mirrorLightDir_8c227dc4);

    RenderQueueDraw_8c0223ea(RENDER_LAYER_MIRROR, (DrawFn)BusCameraDrawBusModel_8c024bb8, 1);
}
