/* @unit BusInit */
#include <shinobi.h>
#include "includes.h" /* TWO_PI, STATIC */
#include "serial_debug.h"

#include "024b4c_bus_camera.h"
#include "014a9c_tasks.h"
#include "015034_text.h"
#include "1ba1c8_globals.h"
#include "01e27c_practice_menu.h" /* var_practiceRules_8c226410 */
#include "013ae8_route.h" /* CourseSceneParams, CourseSegment */
#include "02c884_stop.h"   /* StopAreaRecord, StopGetArea_8c02cd7a */
#include "020914_ground_query.h"
#include "020b6c_ground_probe.h"
#include "023938_bus_drive.h"
#include "023310_bus_init.h"
#include "024280_bus_input.h"
#include "02786c_vehicle_parts.h" /* VehiclePartsBind_8c02786c */
#include "02e51c_attr_query.h"               /* AttrQueryFindConvexPolygon_8c02e51c, AttrQueryFindConvexPolygonAtHeight_8c02eab4, AttrQueryFindPolygon_8c02e69c, AttrQueryFindPolygonAtHeight_8c02ec50 */
#include "020594_vehicle_model.h" /* VehicleModelPlace_8c020594 */
#include "022bdc_bus.h"               /* BusTask_8c022bdc */

/* ====================
 * Non-initialized Globals
 * ====================
 */

LineBusSegment *var_lineSegments_8c227d84;
LineBusNode *var_lineNodes_8c227d88;

/* =====================
 * Functions
 * =====================
 */

/* Positions the player's bus at the current segment's stop and resets its
 * driving-physics state. Called once by BusInitStart_8c023610 at the start of a run. */
STATIC void placeBus_8c023310(void)
{
    StopAreaRecord *stopArea = StopGetArea_8c02cd7a(var_currentSegment_8c228708);
    GroundQueryResult groundResult;
    float angle;
    int ang;
    int i;

    var_busState_8c1bb9d0.field_0x064 = 0;
    var_busState_8c1bb9d0.field_0x068 = 0;
    var_busState_8c1bb9d0.field_0x06c = 0;
    var_busState_8c1bb9d0.distanceTraveled_0x070 = 0;
    var_busState_8c1bb9d0.steerAngle_0x074 = 0;
    var_busState_8c1bb9d0.pitchAngle_0x078 = 0;
    var_busState_8c1bb9d0.rollAngle_0x07c = 0;
    var_busState_8c1bb9d0.blinker_0x080 = 0;

    for (i = 0; i < 5; i++) {
        var_busState_8c1bb9d0.lightCoeffRow_0x0c4[i] = var_sceneParams_8c18ad24->rec0_0x0c[0][i];
    }

    var_busState_8c1bb9d0.posX_0x0f4 = stopArea->x_0x04;
    var_busState_8c1bb9d0.posZ_0x0fc = stopArea->z_0x08;

    GroundQueryFindPolygon_8c020914(var_busState_8c1bb9d0.posX_0x0f4, 0.0f,
                                     var_busState_8c1bb9d0.posZ_0x0fc, &groundResult);
    GroundProbeInterpolateHeight_8c020f7e(&groundResult, &var_busState_8c1bb9d0.posX_0x0f4);

    /* The bus's own body dimensions, in the same four fields TrafficEntry
     * fills from its dims[] table. BusDriveSampleGround_8c023938 hardcodes
     * the same 4.9/1.25/2.6 rather than reading them back. */
    var_busState_8c1bb9d0.width_0x23c = 4.9f;
    var_busState_8c1bb9d0.height_0x244 = 2.5f;
    var_busState_8c1bb9d0.halfHeight_0x248 = 1.25f;
    var_busState_8c1bb9d0.groundOffset_0x24c = 2.6f;
    var_busState_8c1bb9d0.ang_0x258 = 0;

    if (var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE && (var_practiceRules_8c226410 & 8) != 8) {
        var_busState_8c1bb9d0.signalSide_0x25c = 2;
        var_busState_8c1bb9d0.mirror_0x268 = 2;
    } else {
        var_busState_8c1bb9d0.signalSide_0x25c = 0;
        var_busState_8c1bb9d0.mirror_0x268 = 3;
    }

    var_busState_8c1bb9d0.pitchCos_0x270 = 1.0f;
    var_busState_8c1bb9d0.pitchSin_0x26c = 0;

    var_busState_8c1bb9d0.headingDirZ_0x278 = stopArea->dz_0x10;
    var_busState_8c1bb9d0.headingDirX_0x274 = stopArea->dx_0x0c;

    var_busState_8c1bb9d0.posHistory_0x100[0].x =
        var_busState_8c1bb9d0.posX_0x0f4 - var_busState_8c1bb9d0.headingDirX_0x274 * 4.9f;
    var_busState_8c1bb9d0.posHistory_0x100[0].z =
        var_busState_8c1bb9d0.posZ_0x0fc - var_busState_8c1bb9d0.headingDirZ_0x278 * 4.9f;
    var_busState_8c1bb9d0.posHistory_0x100[0].y = var_busState_8c1bb9d0.posY_0x0f8;

    var_busState_8c1bb9d0.posHistory_0x100[1].x =
        var_busState_8c1bb9d0.posX_0x0f4 - var_busState_8c1bb9d0.headingDirX_0x274 * 8.0f;
    var_busState_8c1bb9d0.posHistory_0x100[1].z =
        var_busState_8c1bb9d0.posZ_0x0fc - var_busState_8c1bb9d0.headingDirZ_0x278 * 8.0f;
    var_busState_8c1bb9d0.posHistory_0x100[1].y = var_busState_8c1bb9d0.posY_0x0f8;

    for (i = 2; i < 12; i++) {
        var_busState_8c1bb9d0.posHistory_0x100[i].y = var_busState_8c1bb9d0.posY_0x0f8;
    }

    /* Seed all 10 corner ground-samples (023938_bus_drive.h) as misses,
     * matching GroundQueryFindPolygon_8c020914's own miss path -- attr_0x00/
     * polyIdSlot_0x04 are left stale/uninitialized. */
    for (i = 0; i < 10; i++) {
        var_busState_8c1bb9d0.groundSamples_0x190[i].vertexIds_0x08 = 0;
        var_busState_8c1bb9d0.groundSamples_0x190[i].count_0x0c = 0;
    }

    var_busState_8c1bb9d0.speed_0x27c = 0.0f;
    var_busState_8c1bb9d0.acc_hist_0x280[1] = 0.0f;
    var_busState_8c1bb9d0.acc_hist_0x280[2] = 0.0f;
    var_busState_8c1bb9d0.acc_hist_0x280[3] = 0.0f;

    var_busState_8c1bb9d0.driveState_0x2b4 = 0;
    var_busState_8c1bb9d0.currentLinePointPtr_0x2b8 = (int)stopArea;
    var_busState_8c1bb9d0.lineSegmentProgress_0x2c0 = 2.0f;
    var_busState_8c1bb9d0.lineSegmentRemaining_0x2bc = 2.0f;
    var_busState_8c1bb9d0.laneOffset_0x2c4 = 2.0f;
    var_busState_8c1bb9d0.busAheadFlag_0x2d4 = 1;

    BusDriveSampleGround_8c023938();

    angle = acosf(var_busState_8c1bb9d0.headingDirZ_0x278);
    ang = (int)(angle * 65536.0f / TWO_PI);
    if (var_busState_8c1bb9d0.posX_0x0f4 > var_busState_8c1bb9d0.posHistory_0x100[0].x) {
        ang = -ang;
    }
    var_busState_8c1bb9d0.ang_0x250 = ang;
    var_busState_8c1bb9d0.targetHeadingAngle_0x254 = ang;

    if (var_playMode_8c1bb8d0 == PLAY_MODE_DEMO) {
        var_cameraMode_8c227d9c = 5;
    } else {
        var_cameraMode_8c227d9c = 0;
        var_cameraCueBusy_8c227dac = 0;
    }
}

/* Places the player's bus for the start of a run and arms its per-frame
 * task. BusTask_8c022bdc's ground-query family is picked per route/segment:
 * the Wangan route's segment 10 (an elevated-road overlap) gets the
 * *AtHeight variants, every other case gets the plain ones. */
void BusInitStart_8c023610(void)
{
    Task *created_task;
    void *created_state;
    int i;
    NJS_OBJECT **selected;
    CourseSegment *segment;
    void *result;

    var_activeGroundGrid_8c2264d4 = var_currentCourse_8c1bb868.atariBus_0x04;
    var_activeAttrGrid_8c228b3c = var_currentCourse_8c1bb868.attrBus_0x10;
    var_lineSegments_8c227d84 = var_currentCourse_8c1bb868.lineBus_0x08;
    var_lineNodes_8c227d88 = var_currentCourse_8c1bb868.lineNodes_0x0c;

    var_busDoorLastFrame_8c227db4 = (float)var_busDoorMotion_8c1bc410->nbFrame - 1.0f;

    TaskSpawn_8c014ae8(var_tasks_8c1ba5e8, &BusTask_8c022bdc, &created_task, &created_state, 0);

    var_playerBus_8c1bbd9c = &var_busState_8c1bb9d0;
    var_busState_8c1bb9d0.texlistLarge_0x004 = (int)var_routeModelSlots_8c1bbddc[26].texlist_0x08;
    var_busState_8c1bb9d0.modelLarge_0x00c = (int)var_routeModelSlots_8c1bbddc[26].nj_0x0c;
    var_busState_8c1bb9d0.shadowModel_0x014 = *(int *)((char *)var_trafficModels_8c1bc3f4 + 0x44);
    VehiclePartsBind_8c02786c(&var_busState_8c1bb9d0, 0x1a);

    /* bodyModels_0x04c[0..5] default to 0x3f, then the one for this run's
     * time-of-day/route is overridden to 0x37. */
    for (i = 0; i < 6; i++) {
        var_busState_8c1bb9d0.bodyModels_0x04c[i]->evalflags = 0x3f;
    }

    if (var_timeOfDay_8c18ad20 == TIME_OF_DAY_DAY || var_timeOfDay_8c18ad20 == TIME_OF_DAY_EVENING) {
        selected = &var_busState_8c1bb9d0.bodyModels_0x04c[0];
    } else {
        /* var_timeOfDay_8c18ad20 only ever holds the three TIME_OF_DAY_*
         * values, so this covers TIME_OF_DAY_NIGHT -- the original leaves
         * the register unset for any other value. */
        selected = &var_busState_8c1bb9d0.bodyModels_0x04c[3];
    }
    selected[var_route_8c18ad1c]->evalflags = 0x37;

    if (var_route_8c18ad1c == ROUTE_WANGAN && var_currentSegment_8c228708 == 10) {
        var_busState_8c1bb9d0.groundProbeFn_0x2c8 = (int)GroundProbeFindPolygonAtHeight_8c020fe4;
        var_busState_8c1bb9d0.junctionQueryFnCpu_0x2cc = (int)AttrQueryFindConvexPolygonAtHeight_8c02eab4;
        var_busState_8c1bb9d0.junctionQueryFnRoute_0x2d0 = (int)AttrQueryFindPolygonAtHeight_8c02ec50;
    } else {
        var_busState_8c1bb9d0.groundProbeFn_0x2c8 = (int)GroundQueryFindPolygon_8c020914;
        var_busState_8c1bb9d0.junctionQueryFnCpu_0x2cc = (int)AttrQueryFindConvexPolygon_8c02e51c;
        var_busState_8c1bb9d0.junctionQueryFnRoute_0x2d0 = (int)AttrQueryFindPolygon_8c02e69c;
    }

    placeBus_8c023310();

    var_busState_8c1bb9d0.engineState_0x2e0 = 0;
    var_busState_8c1bb9d0.idleFrameCounter_0x2ec = 0;
    var_busState_8c1bb9d0.gear_0x2f4 = 0;
    var_busState_8c1bb9d0.laneTargetSearchDone_0x334 = 0;

    segment = StopGetSegment_8c02cd6a(var_currentSegment_8c228708);
    var_busState_8c1bb9d0.currentLineNodeIdx_0x33c = segment->stopAreaId_0x02;

    var_busState_8c1bb9d0.field_0x344 = 0;
    var_busState_8c1bb9d0.field_0x348 = 0;
    var_busState_8c1bb9d0.field_0x360 = 0;
    var_busState_8c1bb9d0.field_0x364 = 0;
    var_busState_8c1bb9d0.field_0x37c = 0;
    var_busState_8c1bb9d0.field_0x380 = 0;
    var_busState_8c1bb9d0.field_0x398 = 0;
    var_busState_8c1bb9d0.field_0x39c = 0;
    var_busState_8c1bb9d0.field_0x3a8 = 0;
    var_busState_8c1bb9d0.field_0x3ac = 0;

    BusDriveSampleGround_8c023938();
    BusDriveApplyGround_8c023cba();

    VehicleModelPlace_8c020594(&var_busState_8c1bb9d0.worldMatrix_0x084, var_playerBus_8c1bbd9c);

    result = AttrQueryFindPolygon_8c02e69c(var_busState_8c1bb9d0.posHistory_0x100[2].x,
                           var_busState_8c1bb9d0.posHistory_0x100[2].y,
                           var_busState_8c1bb9d0.posHistory_0x100[2].z,
                           &var_busState_8c1bb9d0.junctionASlot_0x340);
    if (result != NULL) {
        var_busState_8c1bb9d0.junctionARoadFlags_0x34c = ((int *)result)[0];
        var_busState_8c1bb9d0.field_0x350 = ((int *)result)[1];
        var_busState_8c1bb9d0.field_0x354 = ((int *)result)[2];
        var_busState_8c1bb9d0.junctionARoadFlags2_0x358 = ((int *)result)[3];
    } else {
        var_busState_8c1bb9d0.junctionARoadFlags_0x34c = 0;
        var_busState_8c1bb9d0.field_0x350 = 0;
        var_busState_8c1bb9d0.field_0x354 = 0;
        var_busState_8c1bb9d0.junctionARoadFlags2_0x358 = 0;
    }

    result = AttrQueryFindPolygon_8c02e69c(var_busState_8c1bb9d0.posHistory_0x100[3].x,
                           var_busState_8c1bb9d0.posHistory_0x100[3].y,
                           var_busState_8c1bb9d0.posHistory_0x100[3].z,
                           &var_busState_8c1bb9d0.junctionBSlot_0x35c);
    if (result != NULL) {
        var_busState_8c1bb9d0.junctionBRoadFlags_0x368 = ((int *)result)[0];
        var_busState_8c1bb9d0.junctionBAttr1_0x36c = ((int *)result)[1];
        var_busState_8c1bb9d0.field_0x370 = ((int *)result)[2];
        var_busState_8c1bb9d0.junctionBRoadFlags2_0x374 = ((int *)result)[3];
    } else {
        var_busState_8c1bb9d0.junctionBRoadFlags_0x368 = 0;
        var_busState_8c1bb9d0.junctionBAttr1_0x36c = 0;
        var_busState_8c1bb9d0.field_0x370 = 0;
        var_busState_8c1bb9d0.junctionBRoadFlags2_0x374 = 0;
    }

    /* Same posHistory[3] point as above, second out-buffer. */
    result = AttrQueryFindPolygon_8c02e69c(var_busState_8c1bb9d0.posHistory_0x100[3].x,
                           var_busState_8c1bb9d0.posHistory_0x100[3].y,
                           var_busState_8c1bb9d0.posHistory_0x100[3].z,
                           &var_busState_8c1bb9d0.junctionCSlot_0x378);
    if (result != NULL) {
        var_busState_8c1bb9d0.junctionCRoadFlags_0x384 = ((int *)result)[0];
        var_busState_8c1bb9d0.field_0x388 = ((int *)result)[1];
        var_busState_8c1bb9d0.field_0x38c = ((int *)result)[2];
        var_busState_8c1bb9d0.junctionCRoadFlags2_0x390 = ((int *)result)[3];
    } else {
        var_busState_8c1bb9d0.junctionCRoadFlags_0x384 = 0;
        var_busState_8c1bb9d0.field_0x388 = 0;
        var_busState_8c1bb9d0.field_0x38c = 0;
        var_busState_8c1bb9d0.junctionCRoadFlags2_0x390 = 0;
    }

    var_busState_8c1bb9d0.fallbackTaskMatchId_0x3a0 = 0;
    var_busState_8c1bb9d0.markDriveFlags_0x3b0 = 0;
    var_busState_8c1bb9d0.markCueByte_0x3b4 = 0;
    var_busState_8c1bb9d0.markAudioCue_0x3b8 = 0;
    var_busState_8c1bb9d0.scenePresetIds_0x3bc = 0;
    var_busState_8c1bb9d0.doorState_0x3c0 = 0;

    if (var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE && (var_practiceRules_8c226410 & 8) != 8) {
        var_busState_8c1bb9d0.doorRequest_0x3c4 = 0;
    } else {
        var_busState_8c1bb9d0.doorRequest_0x3c4 = 1;
    }

    var_brakePressPeak_8c227d8c = 0;
    var_busState_8c1bb9d0.rpmRampAngle_0x2e4 = 0;
    var_busState_8c1bb9d0.targetRpm_0x2e8 = 0.0f;
    var_busState_8c1bb9d0.cameraYawEase_0x3c8 = 0;
}
