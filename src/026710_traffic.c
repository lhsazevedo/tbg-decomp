/* @unit Traffic */

#include <shinobi.h>

#include "011120_asset_queues.h"
#include "013ae8_route.h"
#include "014a9c_tasks.h"
#include "022464_render.h"
#include "020914_ground_query.h"
#include "020b6c_ground_probe.h"
#include "025b98_traffic_drive.h"
#include "026710_traffic.h"
#include "028258_signal.h"
#include "02786c_vehicle_parts.h"
#include "02c884_stop.h"
#include "02df3c_traffic_lookahead.h"
#include "02e51c_attr_query.h"
#include "02f0c8_traffic_path_scan.h"
#include "02b464_grading.h"
#include "024b4c_bus_camera.h"
#include "1ba1c8_globals.h"
#include "includes.h"
#include "serial_debug.h"

/* ====================
 * Type Declarations
 * ====================
 */

typedef struct {
    TaskAction action;
    void *state;
    int counter_0x08; /* frames since the last placement */
    /* 1 until the first preset switch: placements with a nonzero progress
     * are skipped unless it is 1. */
    int presetState_0x0c;
    int field_0x10;
    int field_0x14;
    TrafficPlacement *queuedItem_0x18; /* next placement of the active preset */
    int field_0x1c;
} TrafficUpdateTask;

/* ====================
 * Non-initialized Globals
 * ====================
 */

int var_activeTrafficPreset_8c227e14;
/* Placement runs per preset, indexed by bits 8-15 of
 * var_busState_8c1bb9d0.scenePresetIds_0x3bc. */
STATIC Sint32 *var_trafficPresetTable_8c227e18;
STATIC PathRecord **var_cpuPathBlocks_8c227e1c;

/* ====================
 * Initialized Globals
 * ====================
 */

/* Script instruction lengths in words, indexed by opcode. */
STATIC Uint8 init_scriptOpWords_8c0460bc[] = {
    0x01, 0x03, 0x03, 0x03, 0x03, 0x02, 0x03, 0x02, 0x04, 0x01, 0x04, 0x00,
};

/* {width, height, groundOffset, length} per variant. */
STATIC float init_variantDims_8c0460c8[16][4] = {
    { 2.42f, 1.67f, 0.65f, 3.0500002f }, /* 2do0 */
    { 2.42f, 1.67f, 0.65f, 3.0500002f }, /* 2do1 */
    { 2.2f,  1.73f, 0.76f, 3.04f      }, /* 4wd  */
    { 2.8f,  1.8f,  0.9f,  3.8f       }, /* sed0 */
    { 2.8f,  1.8f,  0.9f,  3.8f       }, /* sed1 */
    { 2.8f,  1.8f,  0.9f,  3.8f       }, /* sed2 */
    { 2.8f,  1.7f,  0.7f,  3.9f       }, /* tax  */
    { 6.1f,  2.03f, 1.5f,  8.9f       }, /* tor0 */
    { 6.1f,  2.03f, 1.5f,  8.9f       }, /* tor1 */
    { 2.7f,  1.8f,  1.0f,  4.0f       }, /* kto  */
    { 4.0f,  2.07f, 2.0f,  5.3f       }, /* dan0 */
    { 4.0f,  2.07f, 2.0f,  5.3f       }, /* dan1 */
    { 2.6f,  1.53f, 0.9f,  3.5f       }, /* wag  */
    { 4.9f,  2.33f, 2.6f,  8.0f       }, /* bus0 */
    { 2.8f,  1.69f, 0.74f, 3.96f      }, /* pat  */
    { 2.6f,  1.68f, 1.0f,  3.8999999f }, /* kyu  */
};

/* Per-variant acceleration, per frame. */
STATIC float init_variantAccel_8c0461c8[] = {
    0.01f, 0.01f, 0.004f, 0.007f,
    0.007f, 0.007f, 0.007f, 0.002f,
    0.002f, 0.004f, 0.002f, 0.002f,
    0.004f, 0.001f, 0.01f, 0.01f,
};

STATIC Uint32 init_dayMasks_8c046208[3][3] = {
    { 0x00000000, 0x00008000, 0x01000000 },
    { 0x00004000, 0x00000000, 0x08000000 },
    { 0x00000000, 0x00000000, 0x00000200 },
};

/* Variant -> index of its shadow model in var_trafficModels_8c1bc3f4. */
STATIC Uint8 init_variantBodyType_8c04622c[] = {
    0x00, 0x00, 0x01, 0x02, 0x02, 0x02, 0x03, 0x04, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x09, 0x0A,
};

/* ====================
 * Functions
 * ====================
 */

void TrafficResolvePathBlocks_8c026710(TrafficEntry *entry, Uint16 *script)
{
    PathRecord **out;
    Uint16 *ip;

    out = entry->pathBlocks_0x304;
    for (ip = (Uint16 *)((Uint8 *)script + 2); *ip != 9; ip += init_scriptOpWords_8c0460bc[*ip]) {
        if (*ip == 1) {
            *out = var_cpuPathBlocks_8c227e1c[ip[1]];
            out++;
        }
    }
    *out = (PathRecord *)-1;
}

/* For opcode 0 (spawn on the path) or 10 (park at a fixed position). */
STATIC void initEntryState_8c026748(TrafficEntry *entry, int *scriptIp)
{
    TrafficEntry *e = entry;
    Uint16 type = *e->scriptBase_0x2f8;
    PathRecord *seg = NULL;
    int scriptCursor = 0;
    float dist = 0.0f;
    Sint32 segIdx;
    Sint32 variantIdx;
    const float *dims;
    void *junction;
    /* 4 bytes larger than GroundQueryResult; the extra word keeps the frame
     * layout the original build had. */
    Uint8 groundBuf[20];
    Sint32 i;
    Uint16 speedRandom;

    if (type != 10) {
        seg = e->pathBlocks_0x304[0];
        scriptCursor = *scriptIp + 2;
        e->blockIndex_0x300 = 0;
        dist = e->spawnProgress_0x2e8 + 2.0f;
        while (seg->length_0x00 <= dist) {
            dist -= seg->length_0x00;
            seg++;
            if (seg->length_0x00 == 0.0f) {
                segIdx = e->blockIndex_0x300 + 1;
                e->blockIndex_0x300 = segIdx;
                seg = e->pathBlocks_0x304[segIdx];
                scriptCursor += 6;
            }
        }
    }

    e->junctionVertexIds_0x408 = 0;
    e->junctionHitCount_0x40c = 0;
    e->junctionVertexIds2_0x504 = 0;
    e->junctionHitCount2_0x508 = 0;
    e->atGroundJunction_0x50c = 0;
    variantIdx = e->variantIndex_0x2e0;
    e->field_0x064 = 0;
    e->field_0x068 = 0;
    e->field_0x06c = 0;
    e->distanceTraveled_0x070 = 0;
    e->steerAngle_0x074 = 0;
    e->pitchAngle_0x078 = 0;
    e->rollAngle_0x07c = 0;
    e->blinker_0x080 = 0;

    if (var_timeOfDay_8c18ad20 == TIME_OF_DAY_NIGHT && type == 10 &&
        (junction = AttrQueryFindConvexPolygon_8c02e51c(e->posX_0xf4, e->posY_0xf8,
                                  e->posZ_0xfc, &e->junctionSlot_0x404)) != NULL &&
        *(Sint32 *)((Uint8 *)junction + 4) != 0) {
        e->simpleLightIntensity0_0x0c4 = var_nightLightIntensityOn_8c1bbdb0[0];
        e->simpleLightIntensity1_0x0c8 = var_nightLightIntensityOn_8c1bbdb0[1];
        e->simpleLightColorR_0x0cc = var_nightLightColorOn_8c1bbdd0[0];
        e->simpleLightColorG_0x0d0 = var_nightLightColorOn_8c1bbdd0[1];
        e->simpleLightColorB_0x0d4 = var_nightLightColorOn_8c1bbdd0[2];
    } else {
        e->easyLightIntensity0_0x0d8 = var_sceneParams_8c18ad24->rec0_0x0c[2][0];
        e->easyLightIntensity1_0x0dc = var_sceneParams_8c18ad24->rec0_0x0c[2][1];
        e->easyLightColorR_0x0e0 = var_sceneParams_8c18ad24->rec0_0x0c[2][2];
        e->easyLightColorG_0x0e4 = var_sceneParams_8c18ad24->rec0_0x0c[2][3];
        e->easyLightColorB_0x0e8 = var_sceneParams_8c18ad24->rec0_0x0c[2][4];
        e->simpleLightIntensity0_0x0c4 = var_sceneParams_8c18ad24->rec0_0x0c[1][0];
        e->simpleLightIntensity1_0x0c8 = var_sceneParams_8c18ad24->rec0_0x0c[1][1];
        e->simpleLightColorR_0x0cc = var_sceneParams_8c18ad24->rec0_0x0c[1][2];
        e->simpleLightColorG_0x0d0 = var_sceneParams_8c18ad24->rec0_0x0c[1][3];
        e->simpleLightColorB_0x0d4 = var_sceneParams_8c18ad24->rec0_0x0c[1][4];
    }

    if (type != 10) {
        e->posX_0xf4 = (dist - 2.0f) * seg->dirX_0x0c + seg->x_0x04 + e->originJitterX_0x2ec;
        e->posZ_0xfc = (dist - 2.0f) * seg->dirZ_0x10 + seg->z_0x08 + e->originJitterZ_0x2f0;
    }

    GroundQueryFindPolygon_8c020914(e->posX_0xf4, 0.0f, e->posZ_0xfc,
                                    (GroundQueryResult *)groundBuf);
    GroundProbeInterpolateHeight_8c020f7e((GroundQueryResult *)groundBuf, &e->posX_0xf4);

    dims = init_variantDims_8c0460c8[variantIdx];
    e->width_0x23c = dims[0];
    e->length_0x240 = dims[3];
    e->height_0x244 = dims[1];
    e->halfHeight_0x248 = dims[1] / 2.0f;
    e->groundOffset_0x24c = dims[2];
    e->headingDelta_0x258 = 0;
    e->mirrorVisible_0x268 = 0;
    e->headingCos_0x270 = 1.0f;
    e->headingSin_0x26c = 0;

    if (type == 10) {
        e->headingDirZ_0x278 = njCos(e->heading_0x250);
        e->headingDirX_0x274 = njSin(e->heading_0x250);
    } else {
        e->headingDirZ_0x278 = seg->dirZ_0x10;
        e->headingDirX_0x274 = seg->dirX_0x0c;
    }

    e->frontPointX_0x100 = e->posX_0xf4 - e->width_0x23c * e->headingDirX_0x274;
    e->frontPointZ_0x108 = e->posZ_0xfc - e->width_0x23c * e->headingDirZ_0x278;
    e->frontPointY_0x104 = e->posY_0xf8;
    e->probeSideAY_0x11c = e->posY_0xf8;
    e->probeSideBY_0x128 = e->posY_0xf8;
    e->groundProbe_0x190[0].vertexIds_0x08 = 0;
    e->groundProbe_0x190[0].count_0x0c = 0;
    e->groundProbe_0x190[1].vertexIds_0x08 = 0;
    e->groundProbe_0x190[1].count_0x0c = 0;
    e->groundProbe_0x190[2].vertexIds_0x08 = 0;
    e->groundProbe_0x190[2].count_0x0c = 0;
    e->groundProbe_0x190[3].vertexIds_0x08 = 0;
    e->groundProbe_0x190[3].count_0x0c = 0;
    e->driveState_0x2b4 = 0;
    e->pathRecord_0x2b8 = seg;
    e->pathDistanceCopy_0x2c0 = dist;
    e->pathDistance_0x2bc = dist;
    e->projectDistance_0x2c4 = 2.0f;
    e->busAheadFlag_0x2d4 = 0;
    e->lightFadeState_0x2d8 = 0;
    e->lightFadeTrigger_0x2dc = 0;

    if (type == 10) {
        e->speed_0x27c = 0;
    } else {
        e->cruiseSpeed_0x414 = (float)*(Uint16 *)(scriptCursor + 4) / 65536.0f;
        e->speed_0x27c = e->cruiseSpeed_0x414 / 2.0f;
        for (i = 1; i < 4; i++) {
            e->accelHistory_0x280[i] = 0;
        }
        e->accelRate_0x290 = init_variantAccel_8c0461c8[variantIdx];
        e->followSpeedCap_0x418 = 9999.0f;
        e->obstacleLimitActive_0x424 = 0;
        e->stopLimitActive_0x428 = 0;
        speedRandom = AsqGetRandomA_8c012166();
        e->lookaheadMargin_0x41c = (float)speedRandom / 65536.0f + 1.0f;
        e->turnState_0x42c = 0;
        e->signalWaitState_0x448 = 0;
        e->signalWaitId_0x450 = 0xffffffff;
        e->crossingWaitState_0x458 = 0;
        e->heldCrossingId_0x498 = 0;
        e->mergeWaitState_0x468 = 0;
        e->laneChangeState_0x474 = 0;
        *scriptIp = scriptCursor + 6;
        e->busDistance_0x490 = 9999.0f;
        e->lookaheadCacheLen_0x4ec = 0.0f;
        e->lookaheadCursor_0x4f4 = seg;
        e->lookaheadCursorBlock_0x4f8 = e->blockIndex_0x300;
        e->lookaheadCursorDist_0x4fc = dist + 2.5f;
        TrafficLookaheadInit_8c02df3c(entry);
    }
}

/* Re-aims the body at its front point. The float parameter is unused. */
void TrafficUpdateHeading_8c026bc4(float unused, TrafficEntry *entry)
{
    TrafficEntry *e = entry;
    float dx = e->frontPointX_0x100 - e->posX_0xf4;
    float dy = e->frontPointZ_0x108 - e->posZ_0xfc;
    float dist = njSqrt(dx * dx + dy * dy);
    float halfDx, halfDy;
    float angle;
    int iAngle;

    dx = dx / dist;
    dy = dy / dist;
    e->headingDirX_0x274 = dx;
    e->headingDirZ_0x278 = dy;

    e->frontPointX_0x100 = dx * e->width_0x23c + e->posX_0xf4;
    e->frontPointZ_0x108 = dy * e->width_0x23c + e->posZ_0xfc;
    e->rearPointX_0x10c = dx * e->length_0x240 + e->posX_0xf4;
    e->rearPointZ_0x114 = dy * e->length_0x240 + e->posZ_0xfc;

    halfDy = e->halfHeight_0x248 * dy;
    halfDx = e->halfHeight_0x248 * dx;
    e->probeSideAX_0x118 = e->posX_0xf4 - halfDy;
    e->probeSideAZ_0x120 = e->posZ_0xfc + halfDx;
    e->probeSideBX_0x124 = e->posX_0xf4 + halfDy;
    e->probeSideBZ_0x12c = e->posZ_0xfc - halfDx;

    angle = acosf(dy);
    iAngle = (int)((angle * 65536.0) / 6.283184);
    if (dx < 0.0f) {
        iAngle = -iAngle;
    }
    e->heading_0x250 = iAngle;
}


/* The float parameter is unused. Returns 0 once the path block runs out,
 * leaving the position alone. */
Sint32 TrafficAdvanceOnPath_8c026ca2(float unused, TrafficEntry *entry)
{
    TrafficEntry *e = entry;
    PathRecord *seg = e->pathRecord_0x2b8;
    float dist = e->pathDistance_0x2bc;
    float segX, segY;
    float dx, dy, len;
    Sint32 angle;

    (void)unused;

    while (dist >= seg->length_0x00) {
        dist -= seg->length_0x00;
        seg++;
        if (seg->length_0x00 == 0.0f) {
            e->pathRecord_0x2b8 = seg;
            e->pathDistance_0x2bc = dist;
            e->pathDistanceCopy_0x2c0 = dist;
            return 0;
        }
    }

    e->pathRecord_0x2b8 = seg;
    e->pathDistance_0x2bc = dist;

    segX = dist * seg->dirX_0x0c + seg->x_0x04 + e->originJitterX_0x2ec;
    segY = dist * seg->dirZ_0x10 + seg->z_0x08 + e->originJitterZ_0x2f0;
    e->pathPointX_0x0ec = segX;
    e->pathPointZ_0x0f0 = segY;

    dx = e->posX_0xf4 - segX;
    dy = e->posZ_0xfc - segY;
    len = njSqrt(dx * dx + dy * dy);
    dx /= len;
    dy /= len;

    e->posX_0xf4 = dx * e->projectDistance_0x2c4 + segX;
    e->posZ_0xfc = dy * e->projectDistance_0x2c4 + segY;

    angle = (Sint32)((acosf(dy) * 65536.0) / 6.283184);
    if (dx <= 0.0f) {
        angle = -angle;
    }
    e->headingAlt_0x254 = angle;

    return 1;
}

/* Turns a loaded *_MAC_CPU1.DAT's self-relative offsets into pointers. */
void TrafficRelocatePlacementTable_8c026da4(void *handle)
{
    Sint32 *outer;
    TrafficPlacement *rec;

    for (outer = (Sint32 *)handle; *outer != 0; outer++) {
        *outer += (Sint32)handle;
        rec = (TrafficPlacement *)*outer;
        for (; rec->script_0x04 != NULL; rec++) {
            rec->script_0x04 = (Uint16 *)((Uint8 *)rec->script_0x04 + (Sint32)handle);
        }
    }
}

/* Walks the TrafficPathScanNext_8c02f212 hits from `other` and returns the
 * speed `entry` may drive at without hitting one ahead of it (9999.0f = no
 * limit). Hits behind `entry` get their followSpeedCap_0x418 set instead.
 * The scan stops at the player bus. */
float TrafficComputeBlockedSpeed_8c026eaa(TrafficEntry *entry, TrafficEntry *other)
{
    TrafficEntry *e = entry;
    TrafficEntry *o = other;
    float result = 9999.0f;
    float margin = -0.18518518f;
    float dot;
    float candidate;

    for (; o != NULL; o = (TrafficEntry *)TrafficPathScanNext_8c02f212()) {
        if (o == (TrafficEntry *)var_playerBus_8c1bbd9c) {
            dot = (o->frontPointX_0x100 - var_busState_8c1bb9d0.posX_0x0f4) *
                      (o->frontPointX_0x100 - o->posX_0xf4) +
                  (o->frontPointZ_0x108 - o->posZ_0xfc) *
                      (o->frontPointZ_0x108 - var_busState_8c1bb9d0.posZ_0x0fc);
            if (dot < 0.0f) {
                candidate = var_busState_8c1bb9d0.speed_0x27c + margin;
                result = candidate;
                if (candidate < 0.0f) {
                    result = 0.0f;
                }
            }
            break;
        }

        if (o->pathDistanceCopy_0x2c0 <= e->pathDistanceCopy_0x2c0) {
            o->followSpeedCap_0x418 = e->speed_0x27c + margin;
            if (o->followSpeedCap_0x418 < 0.0f) {
                o->followSpeedCap_0x418 = 0.0f;
            }
        } else {
            candidate = o->speed_0x27c + margin;
            result = candidate;
            if (candidate < 0.0f) {
                result = 0.0f;
            }
        }
    }

    return result;
}

/* Sets var_trafficSignalFrames_8c227e24[id] to 1 for every signal id an
 * opcode-5 wait in the current segment's placement scripts names, 0 for the
 * rest; all ids up to maxId when the segment lists no scene object types. */
void TrafficMarkSignalIdsInUse_8c026dcc(int maxId)
{
    CourseSegment *seg;
    Uint8 *typeIds;
    void **table;
    TrafficPlacement *rec;
    Uint16 *ip;
    int i;

    seg = StopGetSegment_8c02cd6a(var_currentSegment_8c228708);
    typeIds = seg->sceneObjectTypeIds_0x14;

    if (typeIds == NULL) {
        for (i = 0; i <= maxId; i++) {
            var_trafficSignalFrames_8c227e24[i] = 1;
        }
        return;
    }

    for (i = 0; i < maxId; i++) {
        var_trafficSignalFrames_8c227e24[i] = 0;
    }

    table = (void **)var_currentCourse_8c1bb868.macCpu1_0x24;
    for (; *typeIds != 0xff; typeIds++) {
        for (rec = (TrafficPlacement *)table[*typeIds]; rec->script_0x04 != NULL; rec++) {
            for (ip = rec->script_0x04; *ip != 9; ip += init_scriptOpWords_8c0460bc[*ip]) {
                if (*ip == 5) {
                    var_trafficSignalFrames_8c227e24[ip[1]] = 1;
                }
            }
        }
    }
}

void TrafficBlinkTurnSignal_8c026f7e(TrafficEntry *entry)
{
    TrafficEntry *e = entry;
    Uint32 counter = e->blinkCounter_0x260;

    e->blinkCounter_0x260 = counter + 1;
    if ((counter & 0x10) == 0) {
        if (e->laneChangeDir_0x47c == 0) {
            e->blinker_0x080 = e->blinker_0x080 | 2;
        } else {
            e->blinker_0x080 = e->blinker_0x080 | 4;
        }
    }
}

/* Distance to the end of the current path block. */
float TrafficRemainingPathDistance_8c026fb0(TrafficEntry *entry)
{
    TrafficEntry *e = entry;
    PathRecord *seg = e->pathRecord_0x2b8;
    float remaining = -e->pathDistance_0x2bc;

    do {
        remaining += seg->length_0x00;
        seg++;
    } while (seg->length_0x00 != 0.0f);

    return remaining;
}

void TrafficSeekPathRecord_8c026fcc(TrafficEntry *entry, PathRecord *seg)
{
    TrafficEntry *e = entry;
    Sint32 segIdx;
    float dist = e->pathDistanceCopy_0x2c0;

    while (seg->length_0x00 <= dist) {
        dist -= seg->length_0x00;
        seg++;
        if (seg->length_0x00 == 0.0f) {
            segIdx = e->blockIndex_0x300 + 1;
            e->blockIndex_0x300 = segIdx;
            seg = e->pathBlocks_0x304[segIdx];
        }
    }

    e->pathBlocks_0x304[e->blockIndex_0x300] = seg;
    e->pathRecord_0x2b8 = seg;
    e->pathDistance_0x2bc = dist;
}


/* Runs the script up to the next halt. Returns 0 when it reaches opcode 9
 * before any opcode 0 or 1 on this call. */
Sint32 TrafficRunEntryScript_8c027012(TrafficEntry *entry)
{
    TrafficEntry *e = entry;
    Uint16 *ip = e->scriptCursor_0x2fc;
    Uint16 *cur;
    Uint16 op;
    Sint32 spawned = 0;
    Sint32 blockIdx;

    for (;;) {
        cur = ip;
        op = *ip;

        if (op == 0) {
            initEntryState_8c026748(entry, (int *)&ip);
            e->spawnProgress_0x2e8 = 0;
            spawned = 1;
            continue;
        }

        if (op == 1) {
            if (spawned) {
                /* Halts without consuming itself: it runs on the next call. */
                break;
            }
            spawned = 1;
            blockIdx = e->blockIndex_0x300 + 1;
            e->blockIndex_0x300 = blockIdx;
            e->pathRecord_0x2b8 = e->pathBlocks_0x304[blockIdx];
            ip = cur + 3;
            e->cruiseSpeed_0x414 = (float)cur[2] / 65536.0f;
            continue;
        }

        if (op == 2 || op == 3) {
            e->turnState_0x42c = 1;
            e->turnDir_0x430 = (op == 3) ? 1 : 0;
            e->turnStartSignalId_0x434 = cur[1];
            e->turnEndSignalId_0x438 = cur[2];
            ip = cur + 3;
            continue;
        } else if (op == 4) {
            ip = cur + 3;
            continue;
        } else if (op == 5) {
            e->signalWaitState_0x448 = 2;
            e->signalWaitId_0x450 = cur[1];
            e->signalWaitArmedBlock_0x454 = e->blockIndex_0x300;
            ip = cur + 2;
            continue;
        } else if (op == 6) {
            e->crossingWaitState_0x458 = 2;
            e->crossingId_0x45c = cur[1];
            e->crossingExitSignalId_0x460 = cur[2];
            e->crossingArmedBlock_0x464 = e->blockIndex_0x300;
            ip = cur + 3;
            continue;
        } else if (op == 7) {
            e->mergeWaitState_0x468 = 2;
            e->mergeWaitSignalId_0x46c = cur[1];
            e->mergeWaitArmedBlock_0x470 = e->blockIndex_0x300;
            ip = cur + 2;
            continue;
        } else if (op == 8) {
            e->laneChangeState_0x474 = 1;
            e->laneChangeSignalId_0x478 = cur[1];
            e->laneChangeDir_0x47c = cur[2];
            e->laneChangePath_0x484 = var_cpuPathBlocks_8c227e1c[cur[3]];
            e->laneChangeArmedBlock_0x480 = e->blockIndex_0x300;
            ip = cur + 4;
            continue;
        } else if (op == 9) {
            if (!spawned) {
                return 0;
            }
            break;
        } else if (op == 10) {
            e->posX_0xf4 = (float)cur[1] / 10.0f;
            e->posZ_0xfc = (float)cur[2] / 10.0f;
            e->heading_0x250 = cur[3];
            e->headingAlt_0x254 = e->heading_0x250 + 0x8000;
            ip = cur + 3;
            initEntryState_8c026748(entry, (int *)&ip);
            break;
        } else {
            /* An unknown opcode hangs, as in the original. */
            continue;
        }
    }

    e->scriptCursor_0x2fc = ip;
    return 1;
}

/* Returns 0 only when the task can't be allocated or the spawn box is
 * occupied. Typecodes 0x1c/0x1e only run on the days in init_dayMasks_8c046208. */
STATIC Sint32 spawnEntry_8c0272b8(Uint32 typeCode, float progress, Uint16 *script)
{
    Task *task;
    void *entryVoid;
    TrafficEntry *e;
    Sint32 dayShift;
    Uint32 dayMask;
    Uint32 tableWord;
    Sint32 variantIdx;
    Uint8 bodyTypeIdx;
    Uint16 rnd;

    if ((typeCode & 0xff) == 0x1c || (typeCode & 0xff) == 0x1e) {
        dayShift = var_progress_8c1ba1cc.days_0x00 - 1;
        dayMask = (dayShift < 0) ? (1u >> ((Uint32)(-dayShift) & 0x1f))
                                  : (1u << ((Uint32)dayShift & 0x1f));
        tableWord = init_dayMasks_8c046208[var_route_8c18ad1c][var_timeOfDay_8c18ad20];
        if ((dayMask & tableWord) == 0) {
            return 1;
        }
    }

    if (*script == 10) {
        if (!TaskSpawn_8c014ae8(var_tasks_8c1bac28, TrafficDriveParkedTask_8c02656a, &task, &entryVoid, 0x514)) {
            return 0;
        }
        e = (TrafficEntry *)entryVoid;
        e->isParked_0x2e4 = 1;
    } else {
        if (!TaskSpawn_8c014ae8(var_tasks_8c1bac28, TrafficDriveVehicleTask_8c025b98, &task, &entryVoid, 0x514)) {
            return 0;
        }
        e = (TrafficEntry *)entryVoid;
        e->isParked_0x2e4 = 0;
    }

    e->animKind_0x48c = (typeCode & 0x8000) ? 4 : 3;

    if (typeCode & 0x4000) {
        e->probeFn_0x2c8 = GroundProbeTrackPolygonAtHeight_8c021290;
        e->junctionQueryFn_0x2cc = AttrQueryFindConvexPolygonAtHeight_8c02eab4;
    } else {
        e->probeFn_0x2c8 = GroundProbeTrackPolygon_8c020b6c;
        e->junctionQueryFn_0x2cc = AttrQueryFindConvexPolygon_8c02e51c;
    }

    typeCode &= 0xfff;
    variantIdx = (Sint32)typeCode >> 1;
    e->variantIndex_0x2e0 = variantIdx;

    if (var_routeModelSlots_8c1bbddc[typeCode].texlist_0x08 == (NJS_TEXLIST *)-1) {
        /* Reported as spawned, as in the original. */
        TaskKill_8c014b66(task);
    } else {
        e->extraLightFlags_0x510 = 0;
        if ((typeCode == 0x14 || typeCode == 0x16) && (AsqGetRandomA_8c012166() & 1) != 0) {
            e->extraLightFlags_0x510 |= 0x40;
        }

        if (*script != 10) {
            TrafficResolvePathBlocks_8c026710(e, script);

            if (TrafficPathScanFirst_8c02f0c8(task, e, e->pathBlocks_0x304[0], 0, progress, 8.0f) != 0) {
                TaskKill_8c014b66(task);
                return 0;
            }

            e->spawnProgress_0x2e8 = progress;
            rnd = AsqGetRandomA_8c012166();
            e->originJitterX_0x2ec = (float)rnd / 65536.0f - 0.5f;
            rnd = AsqGetRandomA_8c012166();
            e->originJitterZ_0x2f0 = (float)rnd / 65536.0f - 0.5f;
        }

        e->spawnPresetId_0x2f4 = var_activeTrafficPreset_8c227e14;
        e->scriptBase_0x2f8 = script;
        e->scriptCursor_0x2fc = script;

        e->texlistLarge_0x04 = var_routeModelSlots_8c1bbddc[typeCode].texlist_0x08;
        e->modelLarge_0x0c = var_routeModelSlots_8c1bbddc[typeCode].nj_0x0c;
        e->texlistSmall_0x08 = var_routeModelSlots_8c1bbddc[typeCode + 1].texlist_0x08;
        e->modelSmall_0x10 = var_routeModelSlots_8c1bbddc[typeCode + 1].nj_0x0c;

        bodyTypeIdx = init_variantBodyType_8c04622c[variantIdx];
        e->shadowModel_0x14 = var_trafficModels_8c1bc3f4[bodyTypeIdx].njDest;

        VehiclePartsBind_8c02786c(e, typeCode);
        TrafficRunEntryScript_8c027012(e);

        if (*script != 10) {
            TrafficAdvanceOnPath_8c026ca2(progress, e);
        }
        TrafficUpdateHeading_8c026bc4(progress, e);
    }

    return 1;
}

/* DrawFn; flag selects the mirror view's light direction. */
STATIC void applyLighting_8c02756a(int flag)
{
    float *dir;

    if (flag != 0) {
        dir = var_mirrorLightDir_8c227dc4;
    } else {
        dir = var_busSimpleLightDir_8c227db8;
    }

    njCnkSetSimpleLight(dir[0], dir[1], dir[2]);
    njCnkSetEasyLight(dir[0], dir[1], dir[2]);
}

/* Spawns the active preset's placements in turn and runs the traffic tasks. */
STATIC void updateTask_8c0275d4(TrafficUpdateTask *task, void *state)
{
    TrafficPlacement *rec;
    Uint32 presetMask;
    Sint32 presetId;
    Sint32 counter;

    (void)state;

    if (var_runState_8c2285c4.runPhase_0x00 == 0) {
        return;
    }

    var_activeGroundGrid_8c2264d4 = var_currentCourse_8c1bb868.atariCpu_0x18;
    var_activeAttrGrid_8c228b3c = var_currentCourse_8c1bb868.attrCpu_0x20;
    SignalClearCrossingOccupied_8c028958();
    var_occupiedGroup_8c228b44 = (Sint32 *)-1;

    presetMask = var_busState_8c1bb9d0.scenePresetIds_0x3bc & 0xff00;
    if (task->presetState_0x0c == 0) {
        if (presetMask == 0) {
            task->presetState_0x0c = 2;
        }
    } else if (presetMask != 0) {
        presetId = (Sint32)presetMask >> 8;
        if (var_activeTrafficPreset_8c227e14 != presetId) {
            var_activeTrafficPreset_8c227e14 = presetId;
            task->queuedItem_0x18 = (TrafficPlacement *)var_trafficPresetTable_8c227e18[presetId];
            task->presetState_0x0c = 0;
        }
    }

    rec = task->queuedItem_0x18;
    if (rec->script_0x04 != NULL) {
        counter = task->counter_0x08;
        task->counter_0x08 = counter + 1;
        if ((Sint32)rec->threshold_0x02 < counter) {
            if ((task->presetState_0x0c != 1 && rec->progress_0x08 != 0.0f) ||
                spawnEntry_8c0272b8(rec->typeCode_0x00, rec->progress_0x08,
                                    rec->script_0x04) != 0) {
                task->queuedItem_0x18 = rec + 1;
                task->counter_0x08 = 0;
            }
        }
    }

    RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, applyLighting_8c02756a, 0);
    RenderQueueDraw_8c0223ea(RENDER_LAYER_MIRROR, applyLighting_8c02756a, 1);
    TaskRunGroup_8c014b42(var_tasks_8c1bac28);
}

/* At night, also caches the scene's light on/off levels and their 20-frame
 * fade steps. */
void TrafficInit_8c02769e(void)
{
    TrafficUpdateTask *task;
    void *state;

    var_cpuPathBlocks_8c227e1c = var_currentCourse_8c1bb868.lineCpu_0x1c;
    var_trafficPresetTable_8c227e18 = (Sint32 *)var_currentCourse_8c1bb868.macCpu1_0x24;

    if (var_route_8c18ad1c == ROUTE_WANGAN) {
        var_signalGroups_8c228b40 = init_signalGroupsWangan_8c04c980;
    } else if (var_route_8c18ad1c == ROUTE_SHINJUKU) {
        var_signalGroups_8c228b40 = init_signalGroupsShinjuku_8c04caec;
    } else if (var_route_8c18ad1c == ROUTE_OME) {
        var_signalGroups_8c228b40 = init_signalGroupsOme_8c04cd38;
    }

    if (var_timeOfDay_8c18ad20 == TIME_OF_DAY_NIGHT) {
        var_nightLightIntensityOn_8c1bbdb0[0] = var_sceneParams_8c18ad24->rec0_0x0c[2][0];
        var_nightLightIntensityOn_8c1bbdb0[1] = var_sceneParams_8c18ad24->rec0_0x0c[2][1];
        var_nightLightColorOn_8c1bbdd0[0] = var_sceneParams_8c18ad24->rec0_0x0c[2][2];
        var_nightLightColorOn_8c1bbdd0[1] = var_sceneParams_8c18ad24->rec0_0x0c[2][3];
        var_nightLightColorOn_8c1bbdd0[2] = var_sceneParams_8c18ad24->rec0_0x0c[2][4];
        var_nightLightIntensityOff_8c1bbda8[0] = var_sceneParams_8c18ad24->rec0_0x0c[1][0];
        var_nightLightIntensityOff_8c1bbda8[1] = var_sceneParams_8c18ad24->rec0_0x0c[1][1];
        var_nightLightColorOff_8c1bbdc4[0] = var_sceneParams_8c18ad24->rec0_0x0c[1][2];
        var_nightLightColorOff_8c1bbdc4[1] = var_sceneParams_8c18ad24->rec0_0x0c[1][3];
        var_nightLightColorOff_8c1bbdc4[2] = var_sceneParams_8c18ad24->rec0_0x0c[1][4];

        var_nightLightIntensityStep_8c1bbda0[0] = (var_nightLightIntensityOn_8c1bbdb0[0] - var_nightLightIntensityOff_8c1bbda8[0]) / 20.0f;
        var_nightLightIntensityStep_8c1bbda0[1] = (var_nightLightIntensityOn_8c1bbdb0[1] - var_nightLightIntensityOff_8c1bbda8[1]) / 20.0f;
        var_nightLightColorStep_8c1bbdb8[0] = (var_nightLightColorOn_8c1bbdd0[0] - var_nightLightColorOff_8c1bbdc4[0]) / 20.0f;
        var_nightLightColorStep_8c1bbdb8[1] = (var_nightLightColorOn_8c1bbdd0[1] - var_nightLightColorOff_8c1bbdc4[1]) / 20.0f;
        var_nightLightColorStep_8c1bbdb8[2] = (var_nightLightColorOn_8c1bbdd0[2] - var_nightLightColorOff_8c1bbdc4[2]) / 20.0f;
    }

    TaskSpawn_8c014ae8(var_tasks_8c1ba5e8, updateTask_8c0275d4, (Task **)&task, &state, 0);
    task->queuedItem_0x18 = (TrafficPlacement *)var_trafficPresetTable_8c227e18[var_activeTrafficPreset_8c227e14];
    task->counter_0x08 = 0;
    task->presetState_0x0c = 1;

    SignalClearCrossingOccupied_8c028958();
}
