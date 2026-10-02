/* 8c1ba1c8 - globals (data only, linked after 0149b0_sbinit) */
#ifndef _1BA1C8_GLOBALS_H
#define _1BA1C8_GLOBALS_H

#include <shinobi.h>
#include "011120_asset_queues.h" /* LoadedModel */
#include "013ae8_route.h" /* CurrentCourse, ModelSlot */
#include "014a9c_tasks.h" /* Task */
#include "015034_text.h" /* enum PLAY_MODE */
#include "015ab8_title.h" /* ResourceGroup */
#include "020914_ground_query.h" /* GroundQueryResult */
#include "022464_render.h" /* MirrorSelect */

/* =================
 * Type Declarations
 * =================
 */

/* The player bus, var_busState_8c1bb9d0. */
typedef struct {
    int typeCode_0x000;
    int texlistLarge_0x004;
    /* TrafficEntry's texlistSmall_0x08/modelSmall_0x10 sit here; the player
     * bus never sets or reads either, so it has no far/simple LOD. */
    int field_0x008;
    int modelLarge_0x00c;
    int field_0x010;
    int shadowModel_0x014;
    /* Sub-model nodes walked out of modelLarge_0x00c by VehiclePartsBind_8c02786c
     * and animated by BusDrawUpdateModels_8c027958 (027958_bus_draw). Which of
     * rearWheelB_0x028 and the three turn lamps get bound at all depends on
     * typeCode_0x000; the bus is type 0x1a. */
    NJS_OBJECT *bodyNode_0x018;
    NJS_OBJECT *frontWheelA_0x01c;
    NJS_OBJECT *frontWheelB_0x020;
    NJS_OBJECT *rearWheelA_0x024;
    NJS_OBJECT *rearWheelB_0x028;
    /* The five lamp nodes hanging off bodyNode_0x018, shown/hidden from bits
     * 0-4 of blinker_0x080. Bit 0 is the brake lamp (025b98 sets it while
     * decelerating or stopped); bits 3 and 4 are forced on for the
     * night/evening running lights. */
    NJS_OBJECT *blinkerLights_0x02c[5];
    NJS_OBJECT *turnLampA_0x040;
    NJS_OBJECT *turnLampB_0x044;
    NJS_OBJECT *turnLampC_0x048;
    NJS_OBJECT *bodyModels_0x04c[6];
    /* Zeroed by busInitPlaceBus_8c023310 and never read; TrafficEntry's
     * three at the same offsets are the same way. */
    int field_0x064;
    int field_0x068;
    int field_0x06c;

    /* Animation angles, BAMS. distanceTraveled spins the wheels (one world
     * unit per revolution), steerAngle turns the front pair, and pitch/roll
     * lean the body: pitch off a 4-frame acceleration sum, roll off yaw rate
     * times speed. Both lean angles stay within a few degrees. */
    int distanceTraveled_0x070;
    int steerAngle_0x074;
    int pitchAngle_0x078;
    int rollAngle_0x07c;
    int blinker_0x080;

    /* Bus's world transform matrix, applied by njMultiMatrix/njSetMatrix
     * before drawing and used by njCalcPoint to place the mirror camera. */
    NJS_MATRIX worldMatrix_0x084;

    /* Copy of var_sceneParams_8c18ad24->rec0_0x0c[0] (the primary directional
     * light's coefficient row), set by busInitPlaceBus_8c023310. */
    float lightCoeffRow_0x0c4[5];
    int easyLightIntensityA_0x0d8;
    int easyLightIntensityB_0x0dc;
    int easyLightColorR_0x0e0;
    int easyLightColorG_0x0e4;
    int easyLightColorB_0x0e8;
    /* Last confirmed lane-crossing point (x,z), read by GeomDistanceXZ_8c02081c
     * (022bdc) and written by BusDriveFindLaneTarget_8c023e7e (023938) -- real float fields
     * (FMOV.S stores), not int. */
    float laneTargetX_0x0ec;
    float laneTargetZ_0x0f0;

    /* Averaged with posX_0x2fc/posZ_0x304 by rowMaterialModelTask_8c02a27c to get a
     * distance-fade reference point. posY_0x0f8 (and posHistory_0x100 below)
     * is filled in by busInitPlaceBus_8c023310 via GroundProbeInterpolateHeight_8c020f7e --
     * it treats {posX_0x0f4, posY_0x0f8, posZ_0x0fc} as one contiguous float[3]. */
    float posX_0x0f4;
    float posY_0x0f8;
    float posZ_0x0fc;

    /* Waypoint-position history, 12 entries. busInitPlaceBus_8c023310 seeds
     * rec[0]/rec[1]'s x/z 4.9m/8.0m behind the spawn stop and every rec's y
     * from the just-computed ground height (posY_0x0f8), leaving rec[2..11]'s
     * x/z untouched (stale). */
    NJS_POINT3 posHistory_0x100[12];

    /* 10 ground-polygon samples around the bus, taken by BusDriveSampleGround_8c023938 at
     * corner/lookahead points derived from posHistory_0x100, then read back
     * by BusDriveApplyGround_8c023cba to average a lane-offset ground height. */
    GroundQueryResult groundSamples_0x190[10];

    /* Bus heading unit vector (x,z), dotted and crossed against the move
     * delta by positionCamera_8c024d6c to size/sign its turn. */
    float headingX_0x230;
    int field_0x234;  /* no use site; the pair has no y, so likely padding */
    float headingZ_0x238;
    /* Written as float literals (4.9/2.5/1.25/2.6) by busInitPlaceBus_8c023310;
     * field_0x240 is skipped by that write and its role is unclear. */
    float width_0x23c;
    int field_0x240;
    float height_0x244;
    /* Both written there and read nowhere in decompiled C. */
    float halfHeight_0x248;
    float groundOffset_0x24c;

    int ang_0x250;

    int targetHeadingAngle_0x254;

    int ang_0x258;

    int signalSide_0x25c;
    int turnSignalBlinkCounter_0x260;
    /* No use site. TrafficEntry's same-offset field is unused too. */
    int field_0x264;

    /* Which mirror view is up, a MirrorSelect (022464_render.h). Set by
     * BusInputUpdate_8c0246b2 from the turn-signal buttons (024280); read by
     * RenderDrawFrame_8c022560 and BusCameraUpdateMirror_8c025604. */
    MirrorSelect mirror_0x268;

    /* Divided by pitchCos_0x270 by BusCameraUpdate_8c025078 for the
     * mode-1 camera's Y bob -- a real float field (FMOV.S load), not int. */
    float pitchSin_0x26c;
    float pitchCos_0x270;
    /* dx component of the spawn stop area's direction (StopAreaRecord.dx_0x0c);
     * paired with headingDirZ_0x278 (dz). */
    float headingDirX_0x274;

    float headingDirZ_0x278;
    float speed_0x27c;
    float acc_hist_0x280[4];

    /* No use site. TrafficEntry uses 0x290 as a float in a speed calc
     * (025b98), but the bus never touches it. */
    int field_0x290;
    int field_0x294;
    int field_0x298;

    /* Collision knockback direction (unit vector), set by
     * handleBump_8c02b6d4 (02b464); field_0x2ac/0x2b0 mirror
     * 0x29c/0x2a0 (role of the duplicate pair unclear). */
    float dir_x_0x29c;
    float dir_z_0x2a0;

    int field_0x2a4;  /* no use site */
    int field_0x2a8;  /* no use site */
    float dir_x2_0x2ac;
    float dir_z2_0x2b0;

    /* 0 = stopped at a bus stop (the door sequence in doorState_0x3c0 runs
     * here), 1 = driving, 2 = knockback after a collision, 3 = halted for
     * good, 4 = braking down to 3. All five are dispatched by
     * BusTask_8c022bdc (022bdc). */
    int driveState_0x2b4;

    int currentLinePointPtr_0x2b8; /* the spawn stop area's StopAreaRecord*, cast to int */
    float lineSegmentRemaining_0x2bc;
    float lineSegmentProgress_0x2c0;
    float laneOffset_0x2c4;
    int groundProbeFn_0x2c8;
    int junctionQueryFnCpu_0x2cc;
    int junctionQueryFnRoute_0x2d0;
    int busAheadFlag_0x2d4;
    int lightFadeState_0x2d8;
    int lightFadeGate_0x2dc;
    /* 0 = off, 1 = starting (30-frame crank, idles at 500 rpm), 2 = running.
     * Driven by BusInputUpdate_8c0246b2 (024280); drawHud_8c01fbac (01fa78)
     * eases var_hudState_8c22643c.engineRpm_0x2c toward 0 / 500 /
     * targetRpm_0x2e8 accordingly. */
    int engineState_0x2e0;
    /* Throttle ramp phase in BAM; njSin of it times 6000 gives
     * targetRpm_0x2e8. Wound up at the current gear's accelRate_0x00 and
     * clamped to the scaled trigger travel. */
    int rpmRampAngle_0x2e4;
    /* A real float field -- the asm loads it with FMOV.S directly, no
     * int-to-float conversion (Ghidra's decompile shows a spurious int cast
     * here). */
    float targetRpm_0x2e8;
    int idleFrameCounter_0x2ec;
    /* Cleared on the transition into engineState_0x2e0 == 2 (024280) and read
     * nowhere. */
    int field_0x2f0;

    int gear_0x2f4;

    int hazardBlinkCounter_0x2f8;
    /* draw position, read by TileStreamDrawTile_8c021b34 */
    float posX_0x2fc;
    float posY_0x300;
    float posZ_0x304;
    /* Last move-delta vector (x,y,z) applied to posX_0x2fc/posZ_0x304 --
     * rotated toward headingX_0x230/headingZ_0x238 first when the raw bearing
     * change is large -- and its magnitude, all written by
     * positionCamera_8c024d6c. */
    float moveDeltaX_0x308;
    float moveDeltaY_0x30c;
    float moveDeltaZ_0x310;
    float moveDeltaMagnitude_0x314;
    /* Rear-view-mirror camera's local offset (x,y,z), picked by
     * mirror_0x268 and rotated by worldMatrix_0x084 into mirrorWorldOffsetX_0x318/0x32c world
     * offsets, all by BusCameraUpdateMirror_8c025604. */
    float mirrorWorldOffsetX_0x318;
    float mirrorWorldOffsetY_0x31c;
    float mirrorWorldOffsetZ_0x320;
    float mirrorDirX_0x324;
    int field_0x328;  /* no use site; padding, like field_0x234 */
    float mirrorDirZ_0x32c;
    float mirrorDist_0x330;
    /* Drive BusDriveFindLaneTarget_8c023e7e's search for a lane-change target point (0/1 =
     * search forward/backward from currentLineNodeIdx_0x33c, 2 = idle/done);
     * armed by the same two buttons that toggle signalSide_0x25c, in
     * mapped-route steering mode (024280.c). Despite the name, unrelated to
     * the O_FUMI_* railway level crossing (0289ac_objects.c) -- "crossing"
     * here means a route-line intersection, not the railway. */
    int laneTargetSearchDone_0x334;
    int laneTargetSearchSide_0x338;
    int currentLineNodeIdx_0x33c;
    int junctionASlot_0x340;
    int field_0x344;
    int field_0x348;
    int junctionARoadFlags_0x34c;
    int field_0x350;
    int field_0x354;
    int junctionARoadFlags2_0x358;
    int junctionBSlot_0x35c;
    int field_0x360;
    int field_0x364;
    int junctionBRoadFlags_0x368;
    int junctionBAttr1_0x36c;
    int field_0x370;
    int junctionBRoadFlags2_0x374;
    int junctionCSlot_0x378;
    int field_0x37c;
    int field_0x380;
    int junctionCRoadFlags_0x384;
    /* Attribute words [1] and [2] of junctionCSlot_0x378's query -- written
     * by 022bdc/023310, read nowhere. */
    int field_0x388;
    int field_0x38c;
    int junctionCRoadFlags2_0x390;
    int cpuAttrOutSlot_0x394;
    int field_0x398;
    int field_0x39c;
    int fallbackTaskMatchId_0x3a0;
    int markAttrOutSlot_0x3a4;
    int field_0x3a8;
    int field_0x3ac;
    int markDriveFlags_0x3b0;
    int markCueByte_0x3b4;
    int markAudioCue_0x3b8;
    int scenePresetIds_0x3bc;

    /* Passenger-door sequence: 0 = shut, 1 = opening, 2 = open, 3 = closing.
     * Only advances while driveState_0x2b4 is 0 (opening) or 1 (closing);
     * var_busDoorFrame_8c227db0 is the animation frame it drives. */
    int doorState_0x3c0;

    /* Asks doorState_0x3c0 for the next step in its cycle -- the A button
     * while stopped, the scripted arrival, or 02b464 after docking the
     * INSTR_DOOR_OPERATION penalty. Cleared once the step is taken. */
    int doorRequest_0x3c4;

    /* Zeroed by BusInitStart_8c023610. */
    int cameraYawEase_0x3c8;
} BusState;

/* Per-course badge tier, ratcheted from var_award_8c1bb8f8 (driver points)
 * by ResultsShowPassedRun_8c01e0b4. Doubles as a sprite index, with a
 * different base per screen: the course menu's badges sit at 0x18 - tier,
 * the results screen's at 0x1f - tier. */
enum {
    AWARD_TIER_NONE   = 0,
    AWARD_TIER_BRONZE = 1,
    AWARD_TIER_SILVER = 2,
    AWARD_TIER_GOLD   = 3
};

typedef struct {
    Uint8 unlocked_0x00;
    Uint8 new_0x01;
    Uint8 everPlayed_0x02;
    Uint8 storyAward_0x03;
    Uint8 freeRunAward_0x04;
    Uint8 reserved_0x05[3]; // Padding?
} CourseProgress;

typedef struct {
    int days_0x00;

    /* unlock-flag bitsets set together by setProgressFlag_8c02af78
     * and tested individually by hasEventProgressFlag_8c02afbe/EventHasProfileProgressFlag_8c02aff0 */
    int eventProgressFlags_0x04[5];
    int profileProgressFlags_0x18[5];

    int letters_0x2c[6];
    CourseProgress courses_0x44[9];
    int profileUnlockedCount_0x8c;
    int exp_0x90; // EXP, also shown on the VMU icon status line (see updateVmsComment_8c01b206)
    int field_0x94;
    int practiceLessonBestScores_0x98[11];

    /* The SETTING screen's five rows, in screen order; while that screen is up
     * var_settingValues_8c226074 walks them from here, wrapping each per
     * init_settingOptionCounts_8c044de0. */
    char difficulty_0xc4;
    char driveMode_0xc5;
    char defaultView_0xc6;
    char vibration_0xc7;  /* 0 is on -- driveCueTask_8c020214 ticks rumble only while it reads 0 */
    /* 0 is on; nonzero holds the cockpit and first-person views level,
     * ignoring the bus's roll and pitch (BusCameraUpdate_8c025078). */
    char screenRoll_0xc8;

    char reserved_0xc9[3];

    /* KEY CONFIGURE button assignment, one row per drive mode x controller
     * type; each selects a layout for the matching remap table in
     * AsqApplyButtonConfig_8c0121e8. */
    char btnConfigManual_0xcc;
    char btnConfigAuto_0xcd;
    char btnConfigWheelManual_0xce;
    char btnConfigWheelAuto_0xcf;

    /* KEY CONFIGURE ACCEL/BRAKE sensitivity: the trigger travel (0-0x80) below
     * which 024280_bus_input ignores the axis. */
    unsigned char accelSensitivity_0xd0;
    unsigned char brakeSensitivity_0xd1;
    /* Never touched by any .c; padding between the sensitivity bytes and
     * the volumes at 0xd4. */
    char field_0xd2;
    char field_0xd3;

    /* AUDIO MUSIC/SFX/VOICE volume (0-9, 10 steps); reset with the sound mode
     * (see FileSelectResetSoundDefaults_8c0188dc) */
    char musicVolume_0xd4;
    char sfxVolume_0xd5;
    char voiceVolume_0xd6;
    char reserved_0xd7; // padding

    /* mirrored to/from var_runReportPending_8c1bb8b8 / var_runWasPractice_8c1bb8bc
     * / var_runSucceeded_8c1bb8dc / var_award_8c1bb8f8 by 01b19c_system_menu on
     * load (SystemMenuApplyLoadedProgress_8c01b19c) and save
     * (SystemMenuWriteToVmu_8c01b26c) */
    int runReportPending_0xd8;
    int runWasPractice_0xdc;
    int runSucceeded_0xe0;
    char award_0xe4;
    char reserved_0xe5[3]; // padding
} PlayerProgress;

extern void* var_busFont_8c1ba1c8;
extern PlayerProgress var_progress_8c1ba1cc;
/* single-word bitset, set/tested by setRunEventFlag_8c02b022/hasRunEventFlag_8c02b030; role unclear */
extern int var_runEventFlags_8c1ba2b4;
/* PlayerProgress.eventProgressFlags_0x04 / profileProgressFlags_0x18 as they
 * stood when the drive started, so retiring from the pause menu can roll back
 * whatever the abandoned drive raised. */
extern int var_eventFlagsSnapshot_8c1ba2b8[5];
extern int var_profileFlagsSnapshot_8c1ba2cc[5];
/* Staging buffer for VMU save-file images: 018644_file_select allocates 16
 * 0x600-byte slots in it (one per VM file) and 01b19c one, for the file it is
 * about to write. -1 when unallocated. */
extern void* var_saveBuf_8c1ba2e0;
extern BUS_BACKUPFILEHEADER var_backupFileHeader_8c1ba2e4; // 018644: analyzed backup file header
extern void* var_vmuIconFileBuf_8c1ba344;
extern void* var_backupFileImageBuf_8c1ba348;
extern int var_selectedVm_8c1ba34c;
extern int var_saveSlot_8c1ba350;        // 018644: chosen VMU file index, i.e. into init_saveNames_8c044d50
extern Uint32 var_vibport_8c1ba354;
extern PDS_PERIPHERAL *var_peripheral_8c1ba358;
extern PDS_PERIPHERAL var_peripherals_8c1ba35c[2];
extern int* var_demoBuf_8c1ba3c4;
extern Task var_tasks_8c1ba3c8[];
extern Task var_tasks_8c1ba5e8[];
extern Task var_tasks_8c1ba808[];
extern Task var_tasks_8c1bac28[];
extern Task var_tasks_8c1bb448[];
extern CurrentCourse var_currentCourse_8c1bb868;
/*
 * The pair the course and practice menus read on arrival to pick their opening
 * dialog: pending says a day was consumed and there is something to announce,
 * practice says the day went on a lesson rather than a course run.
 * Course menu: nothing pending -> "choose a course"; pending + practice -> the
 * "good practice" line; pending + course -> the run's result. Practice menu:
 * pending + practice -> the lesson result; not pending + practice -> the
 * one-time tips, which the course menu arms by setting practice on the way out.
 */
extern int var_runReportPending_8c1bb8b8;
extern int var_runWasPractice_8c1bb8bc;
extern int var_shouldShowFreeRunIntro_8c1bb8c0;
/* The title screen is the screen on top. GameSoftResetTask_8c012f44's soft reset
 * re-spawns the title when it is clear, and quits to the BIOS when it is set. */
extern int var_titleActive_8c1bb8c4;
/* The run's DRIVE MODE: 0 manual, 1 auto. Set from
 * PlayerProgress.driveMode_0xc5 for a retail start, or from the debug row's
 * ReplayMenuCourseSel.driveMode_0x08 (1 on every *_AUTO row). Auto skips most
 * of 02b464's driver-points grading. */
extern int var_driveMode_8c1bb8c8;
extern int var_pauseActive_8c1bb8cc;
extern enum PLAY_MODE var_playMode_8c1bb8d0;
/* Which kind of PLAY_MODE_DEMO is running: 1 = the attract loop
 * (TxtStartAttractDemo_8c0159ac), 0 = a VMU replay
 * (startReplayLoad_8c016b4c). */
extern int var_isAttractDemo_8c1bb8d4;
extern int var_demoIndex_8c1bb8d8;
extern int var_runSucceeded_8c1bb8dc;
extern int var_firstClearOfCourse_8c1bb8e0; // course was unlocked
extern int var_passengerCount_8c1bb8e4;
extern int var_eventCount_8c1bb8e8;
extern int var_worstPenaltyMsgSet_8c1bb8ec;
extern int var_worstPenaltyDelta_8c1bb8f0;
extern int var_penaltyCount_8c1bb8f4;
extern Uint8 var_award_8c1bb8f8;
extern int var_gameMode_8c1bb8fc;
extern int var_cutsceneActive_8c1bb900;
extern NJS_CAMERA var_camera_8c1bb904; // 021b9c_tile_draw
extern NJS_CAMERA var_mirrorCamera_8c1bb944; // 021b9c_tile_draw
/* Aimed from the driver's eye down the aisle by DemoBoardingCamera_8c025870
 * (025870); RenderDrawFrame_8c022560's arrival variant 1 renders it into the inset
 * over the mirror view. */
extern NJS_CAMERA var_cabinCamera_8c1bb984;
extern char var_8c1bb9c4[12]; /* unreferenced; declared here to hold its place in B */
extern BusState var_busState_8c1bb9d0;
/* Points at the player's own BusState (presumably &var_busState_8c1bb9d0).
 * Used in 026710_traffic.c only as a sentinel "entry" marking the player's
 * bus in traffic-avoidance code that otherwise walks a list of real traffic
 * entries (see TrafficComputeBlockedSpeed_8c026eaa) -- identity-compared,
 * never dereferenced there. 02b464 does dereference it, for the player's
 * side of a collision response. */
extern BusState *var_playerBus_8c1bbd9c;
/* The two ends of BusDrawFadeLights_8c028022's (027958_bus_draw) crossfade and
 * the per-frame step between them, cached by TrafficInit_8c02769e from
 * CourseSceneParams.rec0_0x0c rows 1 and 2 -- but only when timeOfDay is
 * TIME_OF_DAY_NIGHT, so by day these hold whatever the last night run left.
 * A row is {intensity0, intensity1, r, g, b}, split 2+3 here exactly as
 * BusState.lightCoeffRow_0x0c4 consumes it. "Off" and "On" are the states of
 * lightFadeGate_0x2dc, a road-polygon attribute word. */
extern float var_nightLightIntensityStep_8c1bbda0[2];
extern float var_nightLightIntensityOff_8c1bbda8[2];
extern float var_nightLightIntensityOn_8c1bbdb0[2];
extern float var_nightLightColorStep_8c1bbdb8[3];
extern float var_nightLightColorOff_8c1bbdc4[3];
extern float var_nightLightColorOn_8c1bbdd0[3];
/* [26] is the player's bus (3s_bus0_l.njd/3t_bus0_l.pvm); BusInitStart_8c023610
 * takes its texlist/nj for texlistLarge_0x004/modelLarge_0x00c. */
extern ModelSlot var_routeModelSlots_8c1bbddc[0x20];
extern ModelSlot var_pedestrianAssets_8c1bbfdc[0x41];
extern void* var_routeModels_8c1bc3ec;
extern LoadedModel *var_segmentModels_8c1bc3f0;
extern LoadedModel *var_trafficModels_8c1bc3f4;
extern ResourceGroup var_loadingResourceGroup_8c1bc3f8;
extern void* var_messageTextBoxA_8c1bc404;
extern void* var_messageTextBoxB_8c1bc408; /* second half of the double-buffered message textbox pair */
extern int var_messageTextBoxIndex_8c1bc40c;   /* active index (0/1) into (&var_messageTextBoxA_8c1bc404)[idx] */
extern NJS_MOTION* var_busDoorMotion_8c1bc410;
extern void* var_busDoorShape_8c1bc414;
/* These three are one ResourceGroup (015ab8_title.h) at 8c1bc418: mark.pvm,
 * mark_parts.dat and mark.dat, the same triple 0129cc_game.c loads into the
 * declared var_loadingResourceGroup_8c1bc3f8's tlist/tanim/contents. Every
 * SpriteDraw_8c014f54 caller reaches them by casting
 * &var_markTexlist_8c1bc418; 8c1bc424 is the same shape for the bus stop.
 * Naming them per-field predates the struct -- folding them into one is a
 * move-data job. */
extern NJS_TEXLIST *var_markTexlist_8c1bc418;
extern void* var_markPartsDat_8c1bc41c;
extern void* var_markDat_8c1bc420;
extern NJS_TEXLIST *var_busStopTexlist_8c1bc424;
extern void* var_busstopPartsDat_8c1bc428;
extern void* var_busstopDat_8c1bc42c;
extern NJS_TEXLIST *var_frontTexlist_8c1bc430;
extern void *var_frontNj_8c1bc434;
extern NJS_TEXLIST *var_interiorTexlist_8c1bc438;
extern void *var_interiorNj_8c1bc43c;
/* fuu.pvm/.njd/.njm -- the animated stop marker drawn by
 * drawStopMarker_8c02cd92 (02c884). Loaded by GameInit_8c0134ec. */
extern void* var_fuuTexlist_8c1bc440;
extern void* var_fuuNj_8c1bc444;
extern NJS_MOTION* var_fuuNjm_8c1bc448;
/* Current "fuu" stop-marker animation frame, driven by
 * StopUpdateArrival_8c02ce48 (02c884): counts up by 1.0 per frame while
 * the bus approaches a stop, wrapping to 0 at var_fuuLastFrame_8c1bc450. */
extern float var_fuuFrame_8c1bc44c;
extern float var_fuuLastFrame_8c1bc450;
extern void* var_vmGameBuf_8c1bc454;
/* IntersectSegments_8c0206f0's intersection-point output, an XZ pair. The two
 * halves are exported separately but must stay adjacent -- callers pass
 * &var_crossingIntersectPoint_8c1bc458 as the whole point. */
extern float var_crossingIntersectPoint_8c1bc458;
extern float var_crossingIntersectPointZ_8c1bc45c;
extern NJS_POINT3 var_groundQueryPoint_8c1bc460; // scratch world point for ground-height queries, e.g. snapPointToGround_8c02840c
/* Shared scratch matrix, rebuilt by each user before it reads it back
 * (01fa78, 023938, 024b4c, 02c884). */
extern NJS_MATRIX var_scratchMatrix_8c1bc46c;

#endif // _1BA1C8_GLOBALS_H
