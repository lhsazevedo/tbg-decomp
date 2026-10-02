/* @unit Traffic */

#include <shinobi.h>

#include "011120_asset_queues.h"
#include "013ae8_route.h"
#include "014a9c_tasks.h"
#include "022464_render.h"
#include "020914_ground_query.h" /* GroundQueryFindPolygon_8c020914, GroundQueryResult */
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
#include "includes.h" /* STATIC */
#include "serial_debug.h"

/* ====================
 * Type Declarations
 * ====================
 */

/* Task private state for updateTask_8c0275d4, set up by TrafficInit_8c02769e. */
typedef struct {
    TaskAction action;
    void *state;
    int counter_0x08; /* the current script record's threshold-comparison counter */
    /* a tri-state flag (0 = no preset armed, 1 = initial-arm suppresses
     * spawnEntry_8c0272b8, 2 = latched) round-tripped through Task's void*
     * field_0x0c */
    int presetState_0x0c;
    int field_0x10;
    int field_0x14;
    TrafficPlacement *queuedItem_0x18; /* the cursor into the active preset's script record array */
    int field_0x1c;
} TrafficUpdateTask;

/* ====================
 * Non-initialized Globals
 * ====================
 */

int var_activeTrafficPreset_8c227e14;
/* Traffic preset table: indexed by var_busState_8c1bb9d0.scenePresetIds_0x3bc's byte at
 * bits 8-15, yielding that preset's record run in the course's *_MAC_CPU1.DAT;
 * read by updateTask_8c0275d4. */
STATIC Sint32 *var_trafficPresetTable_8c227e18;
/* Course CPU path-block table (== CurrentCourse.lineCpu_0x1c), indexed by a
 * script argument to resolve a traffic entry's path. */
STATIC PathRecord **var_cpuPathBlocks_8c227e1c;

/* ====================
 * Initialized Globals
 * ====================
 */

/* Opcode word lengths for TrafficRunEntryScript_8c027012, indexed by opcode. */
STATIC Uint8 init_scriptOpWords_8c0460bc[] = {
    0x01, 0x03, 0x03, 0x03, 0x03, 0x02, 0x03, 0x02, 0x04, 0x01, 0x04, 0x00,
};

/* Per-variant body dimensions, indexed by the 0..15 variant index: one record
 * of {width, height, groundOffset, length}. Paired variants share dimensions.
 * Ghidra split the first record's leading three floats into separate symbols
 * (the .src still carries those labels, marked "from ghidra"); the table is one
 * contiguous 0x100 block ending exactly where init_variantAccel_8c0461c8
 * begins. Grouped by record here, unlike the .src's layout. */
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

/* Per-variant speed-up rate, the per-frame increment
 * TrafficDriveVehicleTask_8c025b98 adds while a vehicle closes on its lane limit. */
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

/* Variant index -> body type, which picks the vehicle's shadow model out of
 * var_trafficModels_8c1bc3f4. */
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

/* Runs one "spawn" or "place decoration" instruction of the entry's script
 * (opcodes 0 and 10 in the caller, TrafficRunEntryScript_8c027012): walks
 * the path to the entry's starting distance, resolves its position and
 * heading, fills in the vehicle's dimension/animation state from its
 * variant table, and spawns its driving task. entry+0x2f8 holds the
 * script's own base pointer, so
 * *entry->0x2f8 is the script's header word: 10 marks a fixed-angle static
 * decoration (traffic light, sign, ...) rather than a path-following
 * vehicle. entry+0x304 onward is itself an inline array of per-block path
 * pointers (the same region TrafficResolvePathBlocks_8c026710 resolves into
 * from the entry's script), indexed by the segment index at entry+0x300;
 * each block is a run of {len,x,y,dx,dy} records terminated by a len==0
 * sentinel record.
 */
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

/* Re-derives heading and the vehicle's 4 body-corner points after the entry
 * reaches a new waypoint (e+0x100/0x108): direction is (target - current)
 * normalized by its own length (njHypot, not the incoming float argument --
 * test confirms the first parameter is unused), matching the sin/dx=e+0x274,
 * cos/dy=e+0x278 convention from initEntryState_8c026748. e+0x23c/0x240 place
 * the front/rear reference points along the new heading; e+0x248 (half-width)
 * offsets those into the 4 corners at e+0x118/0x120/0x124/0x12c. The heading
 * angle (e+0x250) is njArcCos(dy) with its sign flipped when dx < 0 --
 * confirmed via test that acosf's argument is the original normalized dy, not
 * the half-width-scaled value that overwrites the same Ghidra SSA variable
 * just before the call.
 */
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


/* Advances the entry's path-segment cursor (entry+0x2b8 record pointer,
 * entry+0x2bc distance already accumulated into the current record) to the
 * {len,x,y,dx,dy} record containing that distance, then re-derives the
 * vehicle's world position by projecting a fixed distance (entry+0x2c4) from
 * the record's point (entry+0xec/0xf0) towards the current position, along
 * the direction from that record point back to the current position
 * (njHypot, not the incoming float argument -- confirmed unused via probing,
 * same as TrafficUpdateHeading_8c026bc4). Also refreshes the secondary
 * heading angle at entry+0x254 (acosf(dy), sign-flipped when the normalized
 * dx <= 0). Returns 1 when a containing record was found; 0 when the
 * segment run is exhausted (len==0 sentinel), in which case only the
 * segment pointer/distance fields are updated.
 */
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

/* Relocation fixup for a freshly-loaded blob (called generically from
 * routeLoadTask_8c014338/RouteUnusedTask_8c014784 after loading course
 * data): an array of self-relative offsets from the blob's own base
 * (handle), terminated by 0, each pointing to a record run. Each record is
 * 0xc bytes; the offset array's own slot is overwritten with the record's
 * absolute address, and the record's dword at +4 is likewise turned from a
 * self-relative offset into an absolute pointer, walking 0xc-byte strides
 * until a record's +4 field is 0. Same shape as the two
 * ObjectsRelocatePedGroup* fixups in 0289ac_objects.c. */
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

/* Scans the global list of traffic entries starting at "other" (advanced via
 * TrafficPathScanNext_8c02f212, an accessor with no arguments -- the list
 * cursor is maintained elsewhere), looking for one whose path projection is
 * ahead of "entry", to derive the speed limit "entry" must obey to avoid it.
 * var_playerBus_8c1bbd9c is a sentinel pointer standing in for the player's bus in this
 * list; when "other" equals it, the check reads the bus's own waypoint fields
 * directly instead (bus has no ordinary entry struct), and the scan always
 * stops there. Returns 9999.0f (no constraint) unless a blocking entry lowers
 * it. As a side effect, an entry already ahead of "entry" (0x2c0 <= entry's
 * own 0x2c0) gets its 0x418 field refreshed with "entry"'s current speed
 * margin -- use unclear; initEntryState_8c026748 initializes the same field
 * to 9999.0f.
 */
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

/* Rebuilds the "traffic signal frame in use" table (var_trafficSignalFrames_8c227e24,
 * 0..maxId) ahead of loading a new segment's decoration scripts, so a
 * newly-spawned signal's initEntryState_8c026748 init path can pick an id
 * nothing else already owns. When the segment has no scene-object-type list at all
 * (sceneObjectTypeIds_0x14 == NULL), every id up to maxId is conservatively
 * marked in-use. Otherwise every id starts free, then every already-placed
 * decoration script (opcode 5 = fixed id, same opcode table as
 * TrafficResolvePathBlocks_8c026710) across every scene-object type listed for
 * this segment marks its id in-use -- one type's placed-object list is a run
 * of TrafficRelocatePlacementTable_8c026da4-fixed-up 0xc-byte records, each holding its own script
 * pointer at +4, terminated by a 0 there. The per-type table base is
 * var_currentCourse_8c1bb868.macCpu1_0x24. */
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

/* Sums the {len,...} record lengths from the entry's current path record
 * (entry+0x2b8) to the end of the current path block (len==0 terminator),
 * minus the distance already consumed into the current record
 * (entry+0x2bc). Read-only: unlike TrafficAdvanceOnPath_8c026ca2, it does
 * not update the entry's segment pointer or distance fields.
 */
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


/* Per-frame(ish) script interpreter for one traffic entry: walks opcodes
 * starting at the entry's saved cursor (entry+0x2fc) until it hits a
 * stopping condition, then writes the cursor back. Opcode 0 (spawn) is
 * consumed transparently in a loop of its own -- initEntryState_8c026748
 * is called for every run of opcode-0 words, and the cursor it leaves behind
 * (via its scriptIp out-param) is re-read for the next opcode immediately,
 * without yielding.
 *
 * Returns 1 for every halt except an opcode-9 with no prior spawn, which
 * returns 0.
 */
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
                /* Once already spawned (whether by an earlier opcode 0 or by
                 * this same opcode on a previous call), this halts the
                 * interpreter *without* consuming itself, so the same
                 * instruction is reprocessed on the next call. */
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
            /* resolves its id argument through var_cpuPathBlocks_8c227e1c the
             * same way TrafficResolvePathBlocks_8c026710 does */
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
            /* Unreachable with well-formed scripts: init_scriptOpWords_8c0460bc only
             * defines lengths for opcodes 0-10. Preserved as real asm
             * behavior -- an unrecognized opcode leaves the cursor
             * untouched and loops forever. */
            continue;
        }
    }

    e->scriptCursor_0x2fc = ip;
    return 1;
}

/* Spawns one traffic entry for the script's opcode-0 header word (typeCode),
 * script pointer, and route progress (only ever forwarded to
 * TrafficAdvanceOnPath_8c026ca2/TrafficUpdateHeading_8c026bc4, both of which
 * ignore it -- see their own comments).
 *
 * typeCode's low byte is first checked against a per-(route,timeOfDay) day
 * bitmask (init_dayMasks_8c046208): for typeCode 0x1c/0x1e, a clear bit means "not
 * today" and the entry is skipped entirely (still returns 1, as if spawned).
 *
 * The script's header word (*script) selects the driving task: 10 marks a
 * fixed decoration (TrafficDriveParkedTask_8c02656a, entry+0x2e4=1), anything else a
 * path-following vehicle (TrafficDriveVehicleTask_8c025b98, entry+0x2e4=0). typeCode's 0x8000/
 * 0x4000 bits pick the animation kind (entry+0x48c) and the ground/junction
 * query callback pair stored at entry+0x2c8/0x2cc (elevated-road vs. normal).
 *
 * typeCode's low 12 bits then index a pair of consecutive
 * var_routeModelSlots_8c1bbddc entries (front/rear texlist+model, entry+4/
 * 0x8/0xc/0x10); halved, they give the variant index at entry+0x2e0, used to
 * resolve the variant's body-type animation data (init_variantBodyType_8c04622c ->
 * var_trafficModels_8c1bc3f4, entry+0x14). An unloaded model slot
 * (texlist_0x08 == -1) frees the task and skips the rest of the setup below
 * -- real asm behavior: it still returns 1, same as a completed spawn.
 *
 * For a moving vehicle (*script != 10), the script's block-relative args are
 * resolved (TrafficResolvePathBlocks_8c026710), then TrafficPathScanBuild_8c02f0c8 gets a chance
 * to reject the entry outright (TaskKill, return 0 -- this is the only path
 * that actually reports failure after allocation); on acceptance, the
 * entry's start progress and a small random per-entry path-origin jitter
 * (entry+0x2ec/0x2f0) are recorded.
 *
 * Finally the entry's script cursor is armed (entry+0x2f8/0x2fc), VehiclePartsBind_8c02786c
 * runs any remaining per-type setup, and the entry's script is run once
 * (TrafficRunEntryScript_8c027012) before its path/heading are derived for
 * the first frame. Returns 1 on every completed spawn (day-mask skip and
 * unloaded model slot included), 0 only when TaskSpawn itself fails or
 * TrafficPathScanBuild_8c02f0c8 rejects the entry.
 */
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
        /* Real asm quirk: an unloaded model slot still frees the just-
         * allocated task, but falls through to the same "success" return
         * as a completed spawn -- it does not report failure here. */
        TaskKill_8c014b66(task);
    } else {
        e->extraLightFlags_0x510 = 0;
        if ((typeCode == 0x14 || typeCode == 0x16) && (AsqGetRandomA_8c012166() & 1) != 0) {
            e->extraLightFlags_0x510 |= 0x40;
        }

        if (*script != 10) {
            TrafficResolvePathBlocks_8c026710(e, script);

            if (TrafficPathScanBuild_8c02f0c8(task, e, e->pathBlocks_0x304[0], 0, progress, 8.0f) != 0) {
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

/* Sets Chunk simple/easy light direction from one of two fixed light-dir
 * vectors, selected by flag. Installed as a DrawFn (arg passed
 * through as flag) by spawnEntry_8c0272b8's caller. */
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

/* Per-frame TaskAction driving the traffic subsystem: spawned once (with no
 * extra state -- its own TrafficUpdateTask struct doubles as the state, see
 * counter_0x08/presetState_0x0c/queuedItem_0x18 below) into var_tasks_8c1ba5e8.
 * No-ops until the run is under way.
 *
 * Selects the CPU-vehicle collision/attribute meshes as the active
 * ground-query grid for the GroundQueryFindPolygon_8c020914/GroundProbeInterpolateHeight_8c020f7e queries run while
 * spawning/updating entries below (var_activeGroundGrid_8c2264d4 <-
 * .atariCpu_0x18, var_activeAttrGrid_8c228b3c <- .attrCpu_0x20), and resets var_occupiedGroup_8c228b44
 * (its other consumer, TrafficPathScanJunctionOccupied_8c02f28a in 02f0c8_traffic_path_scan, treats
 * -1 as "not cached yet").
 *
 * Tracks a traffic-preset switch via var_busState_8c1bb9d0.scenePresetIds_0x3bc's
 * bits 8-15 (a different byte lane than pedestriansTask_8c0293f6's own use
 * of the same packed word): the first time through with presetState_0x0c == 0
 * and no preset selected, arms presetState_0x0c = 2; once a *different*
 * preset id appears, it's latched into var_activeTrafficPreset_8c227e14 and
 * the task's script cursor (queuedItem_0x18) is refreshed from
 * var_trafficPresetTable_8c227e18[presetId], with presetState_0x0c reset to 0.
 *
 * Then processes the current script record pointed to by queuedItem_0x18
 * (an inline array of 0xc-byte {typeCode, threshold, script, progress}
 * records): while the record has a script (its dword at +4 != 0), a
 * counter (counter_0x08) advances every call (compared against the
 * record's threshold, its Uint16 at +2, *before* the increment); once the
 * threshold is exceeded, the record is "consumed" -- unless
 * presetState_0x0c is not 1 and the record's own progress float is
 * nonzero, this spawns the entry via
 * spawnEntry_8c0272b8(typeCode, progress, script). Real asm quirk:
 * when the (presetState_0x0c != 1 && progress != 0.0f) branch is taken
 * instead, spawnEntry_8c0272b8 is never called, but the record is
 * still advanced as if it had succeeded. Either way "succeeding" advances
 * the cursor to the next record (+0xc) and resets the counter.
 *
 * Finally re-applies the two fixed light directions (applyLighting_8c02756a,
 * one RenderQueueDraw per draw layer) and runs every vehicle task spawned
 * above to completion (TaskRunGroup_8c014b42 on var_tasks_8c1bac28).
 */
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

/* Reading this needs var_trafficPresetTable_8c227e18 typed as the
 * already-known CurrentCourse.macCpu1_0x24 field value instead of the
 * invented PTR_PTR_8c1bb88c global -- its address is
 * var_currentCourse_8c1bb868 + 0x24 -- and var_signalGroups_8c228b40 as Sint32*.
 *
 * Spawns updateTask_8c0275d4 into var_tasks_8c1ba5e8,
 * caching two per-course table pointers (route path array, per-preset script
 * table) and, when time of day is night, a copy of two adjacent
 * CourseSceneParams.rec0_0x0c rows plus their per-20-frame deltas -- consumed
 * by initEntryState_8c026748's junction-light path and (rows only) by
 * BusDrawFadeLights_8c028022 in 027958_bus_draw.
 */
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
