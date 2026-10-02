/* @unit Bus */
#include <shinobi.h>
#include "includes.h" /* TWO_PI */

#include "015034_text.h"
#include "1ba1c8_globals.h"
#include "013ae8_route.h"    /* CurrentCourse */
#include "0100bc_sound.h"         /* var_midiHandles_8c0fcd28, SndUpdateEngine_8c010c6e */
#include "014a9c_tasks.h"         /* Task */
#include "020594_vehicle_model.h" /* VehicleModelPlace_8c020594 */
#include "023938_bus_drive.h"               /* BusDriveSampleGround_8c023938/023cba/023e7e */
#include "024280_bus_input.h"               /* BusInputUpdate_8c0246b2/024280 */
#include "02412c_bus_line.h"               /* BusLineAdvance_8c02412c */
#include "02081c_geom.h" /* GeomDistanceXZ_8c02081c */
#include "024b4c_bus_camera.h"               /* BusCameraUpdate_8c025078, BusCameraUpdateMirror_8c025604 */
#include "025870_cinematic.h"               /* CinematicUpdateCamera_8c025906 */
#include "027958_bus_draw.h"      /* BusDrawFadeLights_8c028022 */
#include "022bdc_bus.h"           /* BusTask_8c022bdc */
#include "02e51c_attr_query.h"
#include "02b464_grading.h"
#include "020214_drive_cue.h" /* var_driveCueState_8c2264b8 */
#include "020914_ground_query.h" /* var_activeGroundGrid_8c2264d4 */

/* =====================
 * Type Declarations
 * =====================
 */

/* Signature shared by the ground and attribute polygon queries. */
typedef void *(*GroundQueryFn)(float x, float y, float z, void *out);

/* =====================
 * Functions
 * =====================
 */

void BusTask_8c022bdc(Task *task, void *state)
{
    float prevSpeed;
    int ang;
    int i;

    (void)task;
    (void)state;

    prevSpeed = var_busState_8c1bb9d0.speed_0x27c;

    var_activeGroundGrid_8c2264d4 = var_currentCourse_8c1bb868.atariBus_0x04;
    var_wallHitBits_8c228660 = 0;
    var_busState_8c1bb9d0.blinker_0x080 = 0;

    /* acos only gives the magnitude; the sign comes from which way X is
     * moving. */
    ang = (int)((acosf(var_busState_8c1bb9d0.headingDirZ_0x278) * 65536.0f) / TWO_PI);
    if (var_busState_8c1bb9d0.posX_0x0f4 > var_busState_8c1bb9d0.posHistory_0x100[0].x) {
        ang = -ang;
    }
    var_busState_8c1bb9d0.ang_0x250 = ang;

    if (var_busState_8c1bb9d0.driveState_0x2b4 == 0) {
        /* At a stop: open the doors on request; 02c884_stop takes over once
         * they are open. */
        if (var_busState_8c1bb9d0.doorState_0x3c0 == 0) {
            if (var_busState_8c1bb9d0.doorRequest_0x3c4 != 0) {
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0x1d, 0);
                var_busDoorFrame_8c227db0 = 0.0f;
                var_busState_8c1bb9d0.doorState_0x3c0 = 1;
            }
        } else if (var_busState_8c1bb9d0.doorState_0x3c0 == 1) {
            var_busDoorFrame_8c227db0 += 0.5f;
            if (var_busDoorLastFrame_8c227db4 < var_busDoorFrame_8c227db0) {
                var_busDoorFrame_8c227db0 = var_busDoorLastFrame_8c227db4;
                var_busState_8c1bb9d0.doorState_0x3c0 = 2;
                var_busState_8c1bb9d0.doorRequest_0x3c4 = 0;
            }
        }
    } else if (var_busState_8c1bb9d0.driveState_0x2b4 == 1) {
        /* A, or a scripted request, closes the doors. */
        if (var_busState_8c1bb9d0.doorState_0x3c0 == 2) {
            if ((var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) != 0) {
                var_busState_8c1bb9d0.doorRequest_0x3c4 = 1;
            }
            if (var_busState_8c1bb9d0.doorRequest_0x3c4 != 0) {
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0x1e, 0);
                var_busState_8c1bb9d0.doorState_0x3c0 = 3;
            }
        } else if (var_busState_8c1bb9d0.doorState_0x3c0 == 3) {
            var_busDoorFrame_8c227db0 -= 0.5f;
            if (var_busDoorFrame_8c227db0 < 0.0f) {
                var_busState_8c1bb9d0.doorState_0x3c0 = 0;
                var_busState_8c1bb9d0.doorRequest_0x3c4 = 0;
            }
        }

        BusInputUpdate_8c0246b2();

        /* The first A press of a drive requests the stop announcement. */
        if (var_driveCueState_8c2264b8.announceRequested_0x0c == 0 &&
            (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) != 0) {
            var_driveCueState_8c2264b8.announceRequested_0x0c = 1;
        }

        if (var_driveMode_8c1bb8c8 == 0) {
            int steerAng = var_busState_8c1bb9d0.ang_0x258 + ang;
            float sinA = njSin(steerAng);
            float cosA = njCos(steerAng);

            var_busState_8c1bb9d0.posX_0x0f4 -= var_busState_8c1bb9d0.speed_0x27c * sinA;
            var_busState_8c1bb9d0.posZ_0x0fc -= var_busState_8c1bb9d0.speed_0x27c * cosA;
        } else {
            /* Line following; a laneOffset_0x2c4 of 2.0 is centred. */
            if (var_busState_8c1bb9d0.laneOffset_0x2c4 != 2.0f &&
                var_busState_8c1bb9d0.speed_0x27c != 0.0f) {
                if (var_busState_8c1bb9d0.laneOffset_0x2c4 <= 2.0f) {
                    var_busState_8c1bb9d0.laneOffset_0x2c4 += prevSpeed;
                    if (var_busState_8c1bb9d0.laneOffset_0x2c4 > 2.0f) {
                        var_busState_8c1bb9d0.laneOffset_0x2c4 = 2.0f;
                    }
                } else {
                    var_busState_8c1bb9d0.laneOffset_0x2c4 -= prevSpeed;
                    if (var_busState_8c1bb9d0.laneOffset_0x2c4 < 2.0f) {
                        var_busState_8c1bb9d0.laneOffset_0x2c4 = 2.0f;
                    }
                }
            }

            BusDriveFindLaneTarget_8c023e7e();

            if (var_busState_8c1bb9d0.laneTargetSearchDone_0x334 == 0) {
                var_busState_8c1bb9d0.lineSegmentRemaining_0x2bc += var_busState_8c1bb9d0.speed_0x27c;
                var_busState_8c1bb9d0.lineSegmentProgress_0x2c0 += var_busState_8c1bb9d0.speed_0x27c;
            }

            BusLineAdvance_8c02412c();
        }

        if (var_busState_8c1bb9d0.mirror_0x268 != 0) {
            BusInputCapMirrorTraffic_8c024280();
        }
    } else if (var_busState_8c1bb9d0.driveState_0x2b4 == 2) {
        /* Knockback from a collision. */
        var_busState_8c1bb9d0.posX_0x0f4 += prevSpeed * var_busState_8c1bb9d0.dir_x_0x29c;
        var_busState_8c1bb9d0.posZ_0x0fc += prevSpeed * var_busState_8c1bb9d0.dir_z_0x2a0;
        var_busState_8c1bb9d0.posHistory_0x100[0].x += prevSpeed * var_busState_8c1bb9d0.dir_x2_0x2ac;
        var_busState_8c1bb9d0.posHistory_0x100[0].z += prevSpeed * var_busState_8c1bb9d0.dir_z2_0x2b0;
        var_busState_8c1bb9d0.speed_0x27c -= 0.1f;
        if (var_busState_8c1bb9d0.speed_0x27c <= 0.0f) {
            if (var_driveMode_8c1bb8c8 != 0) {
                var_busState_8c1bb9d0.laneOffset_0x2c4 =
                    GeomDistanceXZ_8c02081c(&var_busState_8c1bb9d0.posX_0x0f4, &var_busState_8c1bb9d0.laneTargetX_0x0ec);
            }
            /* From run phase 3 on, the run is over. */
            var_busState_8c1bb9d0.driveState_0x2b4 = (var_runState_8c2285c4.runPhase_0x00 < 3) ? 1 : 3;
            var_busState_8c1bb9d0.speed_0x27c = 0.0f;
        }
    } else if (var_busState_8c1bb9d0.driveState_0x2b4 == 4) {
        /* Braking to a stop once the run is over. */
        if (var_busState_8c1bb9d0.speed_0x27c <= 0.0f) {
            var_busState_8c1bb9d0.speed_0x27c += 0.02f;
            if (var_busState_8c1bb9d0.speed_0x27c > 0.0f) {
                var_busState_8c1bb9d0.speed_0x27c = 0.0f;
                var_busState_8c1bb9d0.driveState_0x2b4 = 3;
            }
        } else {
            var_busState_8c1bb9d0.speed_0x27c -= 0.02f;
            if (var_busState_8c1bb9d0.speed_0x27c < 0.0f) {
                var_busState_8c1bb9d0.speed_0x27c = 0.0f;
                var_busState_8c1bb9d0.driveState_0x2b4 = 3;
            }
        }
        var_busState_8c1bb9d0.posX_0x0f4 -= var_busState_8c1bb9d0.speed_0x27c * var_busState_8c1bb9d0.headingDirX_0x274;
        var_busState_8c1bb9d0.posZ_0x0fc -= var_busState_8c1bb9d0.speed_0x27c * var_busState_8c1bb9d0.headingDirZ_0x278;
    }

    SndUpdateEngine_8c010c6e();

    if (var_busState_8c1bb9d0.speed_0x27c != 0.0f) {
        int *result;

        BusDriveSampleGround_8c023938();
        BusDriveApplyGround_8c023cba();

        var_activeAttrGrid_8c228b3c = var_currentCourse_8c1bb868.attrBus_0x10;
        result = ((GroundQueryFn)var_busState_8c1bb9d0.junctionQueryFnRoute_0x2d0)(
            var_busState_8c1bb9d0.posHistory_0x100[2].x, var_busState_8c1bb9d0.posHistory_0x100[2].y,
            var_busState_8c1bb9d0.posHistory_0x100[2].z, &var_busState_8c1bb9d0.junctionASlot_0x340);
        if (result != NULL) {
            var_busState_8c1bb9d0.junctionARoadFlags_0x34c = result[0];
            var_busState_8c1bb9d0.field_0x350 = result[1];
            var_busState_8c1bb9d0.field_0x354 = result[2];
            var_busState_8c1bb9d0.junctionARoadFlags2_0x358 = result[3];
        }

        result = ((GroundQueryFn)var_busState_8c1bb9d0.junctionQueryFnRoute_0x2d0)(
            var_busState_8c1bb9d0.posHistory_0x100[3].x, var_busState_8c1bb9d0.posHistory_0x100[3].y,
            var_busState_8c1bb9d0.posHistory_0x100[3].z, &var_busState_8c1bb9d0.junctionBSlot_0x35c);
        if (result != NULL) {
            var_busState_8c1bb9d0.junctionBRoadFlags_0x368 = result[0];
            var_busState_8c1bb9d0.junctionBAttr1_0x36c = result[1];
            var_busState_8c1bb9d0.field_0x370 = result[2];
            var_busState_8c1bb9d0.junctionBRoadFlags2_0x374 = result[3];
        }

        result = ((GroundQueryFn)var_busState_8c1bb9d0.junctionQueryFnRoute_0x2d0)(
            var_busState_8c1bb9d0.posHistory_0x100[0].x, var_busState_8c1bb9d0.posHistory_0x100[0].y,
            var_busState_8c1bb9d0.posHistory_0x100[0].z, &var_busState_8c1bb9d0.junctionCSlot_0x378);
        if (result != NULL) {
            var_busState_8c1bb9d0.junctionCRoadFlags_0x384 = result[0];
            var_busState_8c1bb9d0.field_0x388 = result[1];
            var_busState_8c1bb9d0.field_0x38c = result[2];
            var_busState_8c1bb9d0.junctionCRoadFlags2_0x390 = result[3];
        }

        var_activeAttrGrid_8c228b3c = var_currentCourse_8c1bb868.attrCpu_0x20;
        result = ((GroundQueryFn)var_busState_8c1bb9d0.junctionQueryFnCpu_0x2cc)(
            var_busState_8c1bb9d0.posX_0x0f4, var_busState_8c1bb9d0.posY_0x0f8,
            var_busState_8c1bb9d0.posZ_0x0fc, &var_busState_8c1bb9d0.cpuAttrOutSlot_0x394);
        if (result == NULL) {
            var_busState_8c1bb9d0.fallbackTaskMatchId_0x3a0 = -1;
            var_busState_8c1bb9d0.lightFadeGate_0x2dc = 0;
        } else {
            var_busState_8c1bb9d0.fallbackTaskMatchId_0x3a0 = result[0];
            var_busState_8c1bb9d0.lightFadeGate_0x2dc = result[1];
        }

        var_activeAttrGrid_8c228b3c = var_currentCourse_8c1bb868.attrMark_0x14;
        result = ((GroundQueryFn)var_busState_8c1bb9d0.junctionQueryFnCpu_0x2cc)(
            var_busState_8c1bb9d0.posX_0x0f4, var_busState_8c1bb9d0.posY_0x0f8,
            var_busState_8c1bb9d0.posZ_0x0fc, &var_busState_8c1bb9d0.markAttrOutSlot_0x3a4);
        if (result == NULL) {
            var_busState_8c1bb9d0.markDriveFlags_0x3b0 = 0;
            var_busState_8c1bb9d0.markCueByte_0x3b4 = 0;
            var_busState_8c1bb9d0.markAudioCue_0x3b8 = 0;
            var_busState_8c1bb9d0.scenePresetIds_0x3bc = 0;
        } else {
            var_busState_8c1bb9d0.markDriveFlags_0x3b0 = result[0];
            var_busState_8c1bb9d0.markCueByte_0x3b4 = result[1];
            var_busState_8c1bb9d0.markAudioCue_0x3b8 = result[2];
            var_busState_8c1bb9d0.scenePresetIds_0x3bc = result[3];
        }
    }

    if (var_driveMode_8c1bb8c8 != 0) {
        /* At most 1 degree (0xb6) per frame, within +-60 degrees (0x2aaa). */
        int target = var_busState_8c1bb9d0.targetHeadingAngle_0x254 - var_busState_8c1bb9d0.ang_0x250;

        if (var_busState_8c1bb9d0.ang_0x258 < target) {
            if (var_busState_8c1bb9d0.ang_0x258 - target < -0xb6) {
                target = var_busState_8c1bb9d0.ang_0x258 + 0xb6;
            }
        } else if (var_busState_8c1bb9d0.ang_0x258 - target > 0xb6) {
            target = var_busState_8c1bb9d0.ang_0x258 - 0xb6;
        }

        if (target > 0x2aaa) {
            var_busState_8c1bb9d0.ang_0x258 = 0x2aaa;
        } else if (target < -0x2aaa) {
            var_busState_8c1bb9d0.ang_0x258 = -0x2aaa;
        } else {
            var_busState_8c1bb9d0.ang_0x258 = target;
        }
    }

    var_busState_8c1bb9d0.distanceTraveled_0x070 -= (int)(var_busState_8c1bb9d0.speed_0x27c * 65536.0f);
    var_busState_8c1bb9d0.steerAngle_0x074 = var_busState_8c1bb9d0.ang_0x258;

    {
        float accel = 0.0f;

        for (i = 0; i < 3; i++) {
            accel += var_busState_8c1bb9d0.acc_hist_0x280[i + 1];
            var_busState_8c1bb9d0.acc_hist_0x280[i] = var_busState_8c1bb9d0.acc_hist_0x280[i + 1];
        }
        var_busState_8c1bb9d0.acc_hist_0x280[3] = var_busState_8c1bb9d0.speed_0x27c - prevSpeed;
        accel += var_busState_8c1bb9d0.acc_hist_0x280[3];
        var_busState_8c1bb9d0.pitchAngle_0x078 = (int)(accel * 1024.0f);
    }
    if (var_busState_8c1bb9d0.pitchAngle_0x078 < -0x5b) {
        var_busState_8c1bb9d0.pitchAngle_0x078 = -0x5b;
    } else if (var_busState_8c1bb9d0.pitchAngle_0x078 > 0x5b) {
        var_busState_8c1bb9d0.pitchAngle_0x078 = 0x5b;
    }

    var_busState_8c1bb9d0.rollAngle_0x07c =
        (int)-(var_busState_8c1bb9d0.speed_0x27c * (float)var_busState_8c1bb9d0.ang_0x258 * 0.2f);
    if (var_busState_8c1bb9d0.rollAngle_0x07c < -0x2d8) {
        var_busState_8c1bb9d0.rollAngle_0x07c = -0x2d8;
    } else if (var_busState_8c1bb9d0.rollAngle_0x07c > 0x2d8) {
        var_busState_8c1bb9d0.rollAngle_0x07c = 0x2d8;
    }

    if (var_timeOfDay_8c18ad20 == TIME_OF_DAY_EVENING) {
        var_busState_8c1bb9d0.blinker_0x080 |= 0x10;
    } else if (var_timeOfDay_8c18ad20 == TIME_OF_DAY_NIGHT) {
        BusDrawFadeLights_8c028022(var_playerBus_8c1bbd9c);
    }

    if (var_busState_8c1bb9d0.gear_0x2f4 == 5) {
        /* Reverse gear: hazard lights. */
        int wasSet = var_busState_8c1bb9d0.hazardBlinkCounter_0x2f8 & 0x10;

        var_busState_8c1bb9d0.hazardBlinkCounter_0x2f8 += 1;
        if (wasSet != 0) {
            var_busState_8c1bb9d0.blinker_0x080 |= 6;
        }
    } else if (var_busState_8c1bb9d0.signalSide_0x25c == 0) {
        var_busState_8c1bb9d0.turnSignalBlinkCounter_0x260 = 0;
        sdMidiStop(var_midiHandles_8c0fcd28[1]);
    } else {
        int wasSet;

        if (var_busState_8c1bb9d0.turnSignalBlinkCounter_0x260 == 0) {
            sdMidiPlay(var_midiHandles_8c0fcd28[1], 1, 0x17, 0);
        }
        wasSet = var_busState_8c1bb9d0.turnSignalBlinkCounter_0x260 & 0x10;
        var_busState_8c1bb9d0.turnSignalBlinkCounter_0x260 += 1;
        if (wasSet == 0) {
            if (var_busState_8c1bb9d0.signalSide_0x25c == 1) {
                var_busState_8c1bb9d0.blinker_0x080 |= 2;
            } else {
                var_busState_8c1bb9d0.blinker_0x080 |= 4;
            }
        }
    }

    VehicleModelPlace_8c020594(&var_busState_8c1bb9d0.worldMatrix_0x084, var_playerBus_8c1bbd9c);

    if (var_playMode_8c1bb8d0 == PLAY_MODE_DEMO) {
        CinematicUpdateCamera_8c025906();
    } else {
        BusCameraUpdate_8c025078();
    }

    BusCameraUpdateMirror_8c025604();
}
