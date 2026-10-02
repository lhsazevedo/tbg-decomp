/* 8c026710 */
#ifndef _026710_TRAFFIC_H
#define _026710_TRAFFIC_H

#include <shinobi.h>

#include "020914_ground_query.h" /* GroundQueryResult */

/* Ground-probe callback stored in TrafficEntry.probeFn_0x2c8: same shape as
 * BusState's groundProbeFn_0x2c8 (023938_bus_drive.c), one of
 * GroundProbeTrackPolygon_8c020b6c/GroundProbeTrackPolygonAtHeight_8c021290. */
typedef void (*GroundProbeFn)(float x, float y, float z, GroundQueryResult *out);

/* Junction/collision query callback stored in TrafficEntry.junctionQueryFn_0x2cc:
 * same shape as BusState's junctionQueryFnCpu_0x2cc/junctionQueryFnRoute_0x2d0 (022bdc_bus.c), one of
 * AttrQueryFindConvexPolygon_8c02e51c/AttrQueryFindConvexPolygonAtHeight_8c02eab4. Distinct signature from GroundProbeFn above --
 * this one returns a hit pointer instead of writing through out. */
typedef void *(*JunctionQueryFn)(float x, float y, float z, void *out);

/* One record of a path block: a run of these terminated by length == 0.
 * TrafficAdvanceOnPath_8c026ca2 walks them to find the record containing the
 * entry's distance-along-path, then projects the world position from the
 * record's origin along its direction. */
typedef struct {
    float length_0x00;
    float x_0x04;
    float z_0x08;
    float dirX_0x0c;
    float dirZ_0x10;
} PathRecord;

/* One CPU-vehicle placement record from the course's *_MAC_CPU1.DAT, walked by
 * updateTask_8c0275d4. A run of these is terminated by script == NULL;
 * TrafficRelocatePlacementTable_8c026da4 relocates `script` from a self-relative offset to a pointer at
 * load time. `progress` is the entry's start distance along its path. */
typedef struct {
    Uint16 typeCode_0x00;
    Uint16 threshold_0x02;
    Uint16 *script_0x04;
    float progress_0x08;
} TrafficPlacement;

/* The 0x514-byte task state spawnEntry_8c0272b8 allocates for every CPU
 * vehicle and fixed decoration. Fields are named only where this unit,
 * 02786c_vehicle_parts and 02e400_collision establish a meaning; the rest stay
 * field_/padding_ -- the per-frame drivers TrafficDriveVehicleTask_8c025b98
 * (moving vehicle) and TrafficDriveParkedTask_8c02656a (decoration) own
 * most of it. */
typedef struct {
    Uint32 typeCode_0x000;
    NJS_TEXLIST *texlistLarge_0x04;
    NJS_TEXLIST *texlistSmall_0x08;
    NJS_OBJECT *modelLarge_0x0c;
    void *modelSmall_0x10;
    void *shadowModel_0x14;
    NJS_OBJECT *bodyNode_0x018;
    NJS_OBJECT *frontWheelA_0x01c;
    NJS_OBJECT *frontWheelB_0x020;
    NJS_OBJECT *rearWheelA_0x024;
    NJS_OBJECT *rearWheelB_0x028;
    /* The five lamp nodes hanging off bodyNode_0x018, shown/hidden from
     * bits 0-4 of blinker_0x080. Bit 0 is the brake lamp (025b98 sets it
     * while decelerating or stopped); bits 3 and 4 are forced on for the
     * night/evening running lights. */
    NJS_OBJECT *blinkerLights_0x02c[5];
    NJS_OBJECT *turnLampA_0x040;
    NJS_OBJECT *turnLampB_0x044;
    NJS_OBJECT *turnLampC_0x048;
    /* BusState's bodyModels_0x04c[6] at the same offsets (1ba1c8_globals.h), but
     * VehiclePartsBind_8c02786c fills them only for typeCode 0x1a and nothing on
     * the traffic side reads them back. */
    NJS_OBJECT *field_0x04c;
    NJS_OBJECT *field_0x050;
    NJS_OBJECT *field_0x054;
    NJS_OBJECT *field_0x058;
    NJS_OBJECT *field_0x05c;
    NJS_OBJECT *field_0x060;
    /* Zeroed by initEntryState_8c026748 and never read; BusState's three at
     * the same offsets are the same way. */
    Uint32 field_0x064;
    Uint32 field_0x068;
    Uint32 field_0x06c;
    /* Same animation angles as BusState's at these offsets (1ba1c8_globals.h). */
    /* Same animation angles as BusState's at these offsets (1ba1c8_globals.h). */
    Uint32 distanceTraveled_0x070;
    Uint32 steerAngle_0x074;
    Uint32 pitchAngle_0x078;
    Uint32 rollAngle_0x07c;
    Uint32 blinker_0x080;
    NJS_MATRIX worldMatrix_0x84;
    float simpleLightIntensity0_0x0c4;
    float simpleLightIntensity1_0x0c8;
    float simpleLightColorR_0x0cc;
    float simpleLightColorG_0x0d0;
    float simpleLightColorB_0x0d4;
    float easyLightIntensity0_0x0d8;
    float easyLightIntensity1_0x0dc;
    float easyLightColorR_0x0e0;
    float easyLightColorG_0x0e4;
    float easyLightColorB_0x0e8;
    float pathPointX_0x0ec;
    float pathPointZ_0x0f0;
    float posX_0xf4;
    float posY_0xf8;
    float posZ_0xfc;
    float frontPointX_0x100;
    float frontPointY_0x104;
    float frontPointZ_0x108;
    float rearPointX_0x10c;
    /* Unreferenced. The front point at 0x100 has a y; the rear point never
     * gets one, so this is likely leftover from a symmetric layout. */
    Uint32 field_0x110;
    float rearPointZ_0x114;
    float probeSideAX_0x118;
    float probeSideAY_0x11c;
    float probeSideAZ_0x120;
    float probeSideBX_0x124;
    float probeSideBY_0x128;
    float probeSideBZ_0x12c;
    Uint8 padding_0x130[0x60];
    /* 4 scratch ground-probe results. spawnEntry_8c0272b8 clears each one's
     * vertexIds_0x08/count_0x0c (leaving attr_0x00/polyIdSlot_0x04
     * untouched -- real asm behavior). Only the first 3 (0x190/0x1a0/0x1b0)
     * have confirmed consumers: per 027958_bus_draw.h/BusDrawPlaceEntity_8c027c3c, which fills them
     * through probeFn_0x2c8's probe callback and interpolates posY_0xf8/
     * probeSideAY_0x11c/probeSideBY_0x128 from them; their .attr_0x00 words are read as
     * a "probe already valid" gate by TrafficDriveParkedTask_8c02656a,
     * which clears their .count_0x0c to force a re-probe. The 4th
     * (0x1c0)'s reader is not yet identified. */
    GroundQueryResult groundProbe_0x190[4];
    Uint8 padding_0x1d0[0x6c];
    float width_0x23c;
    float length_0x240;
    float height_0x244;
    float halfHeight_0x248;
    float groundOffset_0x24c;
    Sint32 heading_0x250;
    Sint32 headingAlt_0x254;
    Uint32 headingDelta_0x258;
    /* Every hit for this offset is on var_busState_8c1bb9d0 (lane-change
     * arrow state); nothing reaches it through a TrafficEntry. */
    Uint32 field_0x25c;
    Uint32 blinkCounter_0x260;
    Uint32 field_0x264;  /* no use site anywhere in src/ */
    Uint32 mirrorVisible_0x268;
    Sint32 headingSin_0x26c;
    float headingCos_0x270;
    float headingDirX_0x274;
    float headingDirZ_0x278;
    /* Same offset/role as BusState.speed_0x27c: a ramp value
     * TrafficDriveParkedTask_8c02656a nudges by +-0.1 per frame and
     * projects along dirX_0x29c/dirZ_0x2a0 to slide the entity. */
    float speed_0x27c;
    /* 4-slot ring buffer, shifted down by one slot (0x284->0x280,
     * 0x288->0x284, 0x28c->0x288, 0x28c unchanged) every frame by
     * TrafficDriveVehicleTask_8c025b98's shared tail, which sums the 3 shifted-
     * out slots plus this frame's speed delta into the heading value passed
     * to BusDrawPlaceEntity_8c027c3c. All 4 slots are float; previously typed
     * as 2 Uint32 fields plus 8 bytes of padding -- see that function. */
    float accelHistory_0x280[4];
    float accelRate_0x290;
    Uint8 padding_0x294[0x8];
    /* Same offset/role as BusState.dir_x_0x29c/dir_z_0x2a0 (its collision
     * knockback direction): applied to posX_0xf4/frontPointX_0x100 (front) and
     * posZ_0xfc/frontPointZ_0x108 (rear) by TrafficDriveParkedTask_8c02656a. */
    float dirX_0x29c;
    float dirZ_0x2a0;
    Uint8 padding_0x2a4[0x10];
    /* Same offset as BusState.driveState_0x2b4. The top-level state driving
     * both per-frame drivers: 0/2 = normal driving (TrafficDriveVehicleTask_8c025b98's
     * main body; 2 additionally blends the ground-snapped height back down
     * to normal, then reverts to 0), 1 = being pushed by
     * speed_0x27c/dirX_0x29c/dirZ_0x2a0 (collision knockback for a moving
     * vehicle, or the whole story for a fixed decoration -- see
     * TrafficDriveParkedTask_8c02656a), 3 = waiting for the entity's own
     * spawn box to clear after a script reload before resuming. Unrelated
     * to laneChangeState_0x474's own 1-4 sequencing for the junction-wait script
     * opcodes (5/6/7/8). */
    Uint32 driveState_0x2b4;
    PathRecord *pathRecord_0x2b8;
    float pathDistance_0x2bc;
    float pathDistanceCopy_0x2c0;
    float projectDistance_0x2c4;
    GroundProbeFn probeFn_0x2c8;
    JunctionQueryFn junctionQueryFn_0x2cc;
    /* BusState holds a second junction-query fn pointer here; the traffic
     * code never touches the offset. */
    Uint32 field_0x2d0;
    Uint32 busAheadFlag_0x2d4;
    Uint32 lightFadeState_0x2d8;
    Uint32 lightFadeTrigger_0x2dc;
    Sint32 variantIndex_0x2e0;
    Sint32 isParked_0x2e4;
    float spawnProgress_0x2e8;
    float originJitterX_0x2ec;
    float originJitterZ_0x2f0;
    Sint32 spawnPresetId_0x2f4;
    Uint16 *scriptBase_0x2f8;
    Uint16 *scriptCursor_0x2fc;
    Sint32 blockIndex_0x300;
    /* inline array of per-path-block record pointers, indexed by
     * blockIndex_0x300; TrafficResolvePathBlocks_8c026710 fills it from the
     * script's opcode-1 path ids and terminates it with -1 */
    PathRecord *pathBlocks_0x304[64];
    Uint32 junctionSlot_0x404;
    Uint32 junctionVertexIds_0x408;
    Uint32 junctionHitCount_0x40c;
    Uint32 signalId_0x410;
    float cruiseSpeed_0x414;
    /* Speed this entry must not exceed because of whatever is in front of
     * it: the vehicle ahead's speed plus a margin, clamped at 0. Written by
     * TrafficComputeBlockedSpeed_8c026eaa (into the *other* entry's copy) and
     * by BusInputCapMirrorTraffic_8c024280 (024280) for the bus; reset to
     * 9999.0f each frame, then fed to the per-frame speed-limit min()
     * (025b98). */
    float followSpeedCap_0x418;
    float lookaheadMargin_0x41c;
    Uint32 field_0x420;  /* no use site anywhere in src/ */
    Uint32 obstacleLimitActive_0x424;
    Uint32 stopLimitActive_0x428;
    Uint32 turnState_0x42c;
    Sint32 turnDir_0x430;
    Uint32 turnStartSignalId_0x434;
    Uint32 turnEndSignalId_0x438;
    Uint8 padding_0x43c[0xc];
    Uint32 signalWaitState_0x448;
    /* Unreferenced. init_scriptOpWords_8c0460bc gives the 0x448 family one arg word where
     * the 0x458/0x468 families get two, so this is probably just matching
     * their width. */
    Uint32 field_0x44c;
    Uint32 signalWaitId_0x450;
    Uint32 signalWaitArmedBlock_0x454;
    Uint32 crossingWaitState_0x458;
    Uint32 crossingId_0x45c;
    Uint32 crossingExitSignalId_0x460;
    Uint32 crossingArmedBlock_0x464;
    Uint32 mergeWaitState_0x468;
    Uint32 mergeWaitSignalId_0x46c;
    Uint32 mergeWaitArmedBlock_0x470;
    Uint32 laneChangeState_0x474;
    Uint32 laneChangeSignalId_0x478;
    Sint32 laneChangeDir_0x47c;
    Uint32 laneChangeArmedBlock_0x480;
    PathRecord *laneChangePath_0x484;
    Uint32 laneChangeTimer_0x488;
    Sint32 animKind_0x48c;
    /* Distance to the player bus, refreshed per frame by 025b98. */
    float busDistance_0x490;
    Uint32 groundAligned_0x494;
    Uint32 heldCrossingId_0x498;
    /* Cache of upcoming path positions (x,z pairs) at 5-unit intervals,
     * built by TrafficLookaheadInit_8c02df3c/TrafficLookaheadScan_8c02dfca (02df3c) from lookaheadCursor_0x4f4/0x4fc/
     * 0x4f8's path-walk cursor. Terminated by a 9999.0 sentinel in the
     * next unwritten slot's x; up to 10 pairs fit. */
    float lookaheadPoints_0x49c[10][2];
    /* float, not Sint32 -- TrafficDriveVehicleTask_8c025b98 decrements it by
     * the frame's speed like an odometer. */
    float lookaheadCacheLen_0x4ec;
    Uint32 field_0x4f0;  /* unreferenced; likely alignment */
    PathRecord *lookaheadCursor_0x4f4;
    Uint32 lookaheadCursorBlock_0x4f8;
    float lookaheadCursorDist_0x4fc;
    Uint32 junctionSlot2_0x500;
    Uint32 junctionVertexIds2_0x504;
    Uint32 junctionHitCount2_0x508;
    Uint32 atGroundJunction_0x50c;
    Uint32 extraLightFlags_0x510;
} TrafficEntry;

extern int var_activeTrafficPreset_8c227e14;

void TrafficResolvePathBlocks_8c026710(TrafficEntry *entry, Uint16 *script);
void TrafficUpdateHeading_8c026bc4(float unused, TrafficEntry *entry);
Sint32 TrafficAdvanceOnPath_8c026ca2(float unused, TrafficEntry *entry);
void TrafficRelocatePlacementTable_8c026da4(void *handle);
void TrafficMarkSignalIdsInUse_8c026dcc(int maxId);
float TrafficComputeBlockedSpeed_8c026eaa(TrafficEntry *entry, TrafficEntry *other);
void TrafficBlinkTurnSignal_8c026f7e(TrafficEntry *entry);
float TrafficRemainingPathDistance_8c026fb0(TrafficEntry *entry);
void TrafficSeekPathRecord_8c026fcc(TrafficEntry *entry, PathRecord *seg);
Sint32 TrafficRunEntryScript_8c027012(TrafficEntry *entry);
void TrafficInit_8c02769e(void);

#endif // _026710_TRAFFIC_H
