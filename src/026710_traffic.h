/* 8c026710 */
#ifndef _026710_TRAFFIC_H
#define _026710_TRAFFIC_H

#include <shinobi.h>

#include "020914_ground_query.h" /* GroundQueryResult */

typedef void (*GroundProbeFn)(float x, float y, float z, GroundQueryResult *out);

typedef void *(*JunctionQueryFn)(float x, float y, float z, void *out);

/* One straight piece of a path block; a block is a run of these ended by
 * length == 0. */
typedef struct {
    float length_0x00;
    float x_0x04;
    float z_0x08;
    float dirX_0x0c;
    float dirZ_0x10;
} PathRecord;

/* One CPU-vehicle placement record from the course's *_MAC_CPU1.DAT; a run
 * of these is ended by script == NULL. `script` is stored as an offset from
 * the file base until relocated. */
typedef struct {
    Uint16 typeCode_0x00;
    Uint16 threshold_0x02; /* frames after the previous record */
    Uint16 *script_0x04;
    float progress_0x08; /* start distance along the path */
} TrafficPlacement;

/* Task state of a CPU vehicle. Shares many offsets with BusState
 * (1ba1c8_globals.h). */
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
    /* Shown from bits 0-4 of blinker_0x080: 0 brake, 1/2 turn signals,
     * 3/4 running lights. */
    NJS_OBJECT *blinkerLights_0x02c[5];
    NJS_OBJECT *turnLampA_0x040;
    NJS_OBJECT *turnLampB_0x044;
    NJS_OBJECT *turnLampC_0x048;
    /* Filled only for typeCode 0x1a, and never read. */
    NJS_OBJECT *field_0x04c;
    NJS_OBJECT *field_0x050;
    NJS_OBJECT *field_0x054;
    NJS_OBJECT *field_0x058;
    NJS_OBJECT *field_0x05c;
    NJS_OBJECT *field_0x060;
    /* Zeroed, never read. */
    Uint32 field_0x064;
    Uint32 field_0x068;
    Uint32 field_0x06c;
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
    Uint32 field_0x110;
    float rearPointZ_0x114;
    float probeSideAX_0x118;
    float probeSideAY_0x11c;
    float probeSideAZ_0x120;
    float probeSideBX_0x124;
    float probeSideBY_0x128;
    float probeSideBZ_0x12c;
    Uint8 padding_0x130[0x60];
    /* Height samples under posY_0xf8, probeSideAY_0x11c and
     * probeSideBY_0x128; the 4th has no known reader. Clearing count_0x0c
     * forces a re-probe. */
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
    Uint32 field_0x25c; /* unused here; BusState's lane-change arrow state */
    Uint32 blinkCounter_0x260;
    Uint32 field_0x264;
    Uint32 mirrorVisible_0x268;
    Sint32 headingSin_0x26c;
    float headingCos_0x270;
    float headingDirX_0x274;
    float headingDirZ_0x278;
    float speed_0x27c;
    float accelHistory_0x280[4]; /* recent speed deltas, oldest first */
    float accelRate_0x290;
    Uint8 padding_0x294[0x8];
    /* Knockback direction. */
    float dirX_0x29c;
    float dirZ_0x2a0;
    Uint8 padding_0x2a4[0x10];
    /* 0 = driving, 1 = knockback, 2 = driving while easing back onto the
     * path after a knockback, 3 = waiting for the spawn box to clear after a
     * script reload. */
    Uint32 driveState_0x2b4;
    PathRecord *pathRecord_0x2b8;
    float pathDistance_0x2bc;
    float pathDistanceCopy_0x2c0;
    float projectDistance_0x2c4; /* body distance from its path point */
    GroundProbeFn probeFn_0x2c8;
    JunctionQueryFn junctionQueryFn_0x2cc;
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
    PathRecord *pathBlocks_0x304[64]; /* ended by -1 */
    Uint32 junctionSlot_0x404;
    Uint32 junctionVertexIds_0x408;
    Uint32 junctionHitCount_0x40c;
    Uint32 signalId_0x410;
    float cruiseSpeed_0x414;
    /* Speed of the vehicle ahead less a margin; 9999.0f = no cap. */
    float followSpeedCap_0x418;
    float lookaheadMargin_0x41c;
    Uint32 field_0x420;
    Uint32 obstacleLimitActive_0x424;
    Uint32 stopLimitActive_0x428;
    Uint32 turnState_0x42c;
    Sint32 turnDir_0x430; /* picks the turn-signal bit */
    Uint32 turnStartSignalId_0x434;
    Uint32 turnEndSignalId_0x438;
    Uint8 padding_0x43c[0xc];
    Uint32 signalWaitState_0x448;
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
    float busDistance_0x490;
    Uint32 groundAligned_0x494;
    Uint32 heldCrossingId_0x498;
    /* Upcoming path points (x, z) 5 units apart, ended by x == 9999.0. */
    float lookaheadPoints_0x49c[10][2];
    float lookaheadCacheLen_0x4ec;
    Uint32 field_0x4f0;
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
