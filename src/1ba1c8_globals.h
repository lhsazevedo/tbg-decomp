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
    /* TrafficEntry's texlistSmall/modelSmall slots; the bus has no far LOD. */
    int field_0x008;
    int modelLarge_0x00c;
    int field_0x010;
    int shadowModel_0x014;
    /* Nodes of modelLarge_0x00c. Whether rearWheelB_0x028 and the turn lamps
     * exist depends on typeCode_0x000; the bus is type 0x1a. */
    NJS_OBJECT *bodyNode_0x018;
    NJS_OBJECT *frontWheelA_0x01c;
    NJS_OBJECT *frontWheelB_0x020;
    NJS_OBJECT *rearWheelA_0x024;
    NJS_OBJECT *rearWheelB_0x028;
    /* Shown from bits 0-4 of blinker_0x080. Bit 0 is the brake lamp; bits 3
     * and 4 are forced on for the night/evening running lights. */
    NJS_OBJECT *blinkerLights_0x02c[5];
    NJS_OBJECT *turnLampA_0x040;
    NJS_OBJECT *turnLampB_0x044;
    NJS_OBJECT *turnLampC_0x048;
    NJS_OBJECT *bodyModels_0x04c[6];
    /* Never read. */
    int field_0x064;
    int field_0x068;
    int field_0x06c;

    /* Animation angles, BAMS. distanceTraveled spins the wheels (one world
     * unit per revolution). Pitch comes off a 4-frame acceleration sum, roll
     * off yaw rate times speed; both stay within a few degrees. */
    int distanceTraveled_0x070;
    int steerAngle_0x074;
    int pitchAngle_0x078;
    int rollAngle_0x07c;
    int blinker_0x080;

    NJS_MATRIX worldMatrix_0x084;

    /* Copy of the course's primary directional light row,
     * var_sceneParams_8c18ad24->rec0_0x0c[0]. */
    float lightCoeffRow_0x0c4[5];
    int easyLightIntensityA_0x0d8;
    int easyLightIntensityB_0x0dc;
    int easyLightColorR_0x0e0;
    int easyLightColorG_0x0e4;
    int easyLightColorB_0x0e8;
    float laneTargetX_0x0ec;
    float laneTargetZ_0x0f0;

    /* Passed whole as one float[3]. */
    float posX_0x0f4;
    float posY_0x0f8;
    float posZ_0x0fc;

    /* placeBus_8c023310 seeds only [0] and [1]'s x/z (4.9m and 8.0m behind
     * the spawn stop); [2..11] keep stale x/z. */
    NJS_POINT3 posHistory_0x100[12];

    GroundQueryResult groundSamples_0x190[10];

    /* Unit vector (x,z). */
    float headingX_0x230;
    int field_0x234;  /* never used; the pair has no y */
    float headingZ_0x238;
    /* Constants set by placeBus_8c023310; field_0x240 is not among them. */
    float width_0x23c;
    int field_0x240;
    float height_0x244;
    /* Never read. */
    float halfHeight_0x248;
    float groundOffset_0x24c;

    int ang_0x250;

    int targetHeadingAngle_0x254;

    int ang_0x258;

    int signalSide_0x25c;
    int turnSignalBlinkCounter_0x260;
    /* Never used. */
    int field_0x264;

    MirrorSelect mirror_0x268;

    float pitchSin_0x26c;
    float pitchCos_0x270;
    /* Unit vector (x,z), seeded from the spawn stop area's direction. */
    float headingDirX_0x274;

    float headingDirZ_0x278;
    float speed_0x27c;
    float acc_hist_0x280[4];

    /* Never used by the bus; TrafficEntry's 0x290 is a float. */
    int field_0x290;
    int field_0x294;
    int field_0x298;

    /* Collision knockback direction (unit vector). dir_x2/dir_z2 hold a
     * copy; role unclear. */
    float dir_x_0x29c;
    float dir_z_0x2a0;

    int field_0x2a4;  /* never used */
    int field_0x2a8;  /* never used */
    float dir_x2_0x2ac;
    float dir_z2_0x2b0;

    /* 0 = stopped at a bus stop (doorState_0x3c0 runs here), 1 = driving,
     * 2 = knockback after a collision, 3 = halted for good, 4 = braking
     * down to 3. */
    int driveState_0x2b4;

    /* A LinePoint *; placeBus_8c023310 seeds it with the spawn
     * StopAreaRecord *. */
    int currentLinePointPtr_0x2b8;
    float lineSegmentRemaining_0x2bc;
    float lineSegmentProgress_0x2c0;
    float laneOffset_0x2c4;
    int groundProbeFn_0x2c8;
    int junctionQueryFnCpu_0x2cc;
    int junctionQueryFnRoute_0x2d0;
    int busAheadFlag_0x2d4;
    int lightFadeState_0x2d8;
    int lightFadeGate_0x2dc;
    /* 0 = off, 1 = starting (30-frame crank, idles at 500 rpm), 2 = running. */
    int engineState_0x2e0;
    /* Throttle ramp phase in BAM; njSin of it times 6000 gives
     * targetRpm_0x2e8. Wound up at the current gear's accelRate_0x00 and
     * clamped to the scaled trigger travel. */
    int rpmRampAngle_0x2e4;
    float targetRpm_0x2e8;
    int idleFrameCounter_0x2ec;
    /* Cleared when the engine starts running; never read. */
    int field_0x2f0;

    int gear_0x2f4;

    int hazardBlinkCounter_0x2f8;
    /* draw position */
    float posX_0x2fc;
    float posY_0x300;
    float posZ_0x304;
    /* Last move applied to posX_0x2fc/posZ_0x304, rotated toward
     * headingX_0x230/headingZ_0x238 first when the bearing change is large. */
    float moveDeltaX_0x308;
    float moveDeltaY_0x30c;
    float moveDeltaZ_0x310;
    float moveDeltaMagnitude_0x314;
    float mirrorWorldOffsetX_0x318;
    float mirrorWorldOffsetY_0x31c;
    float mirrorWorldOffsetZ_0x320;
    float mirrorDirX_0x324;
    int field_0x328;  /* never used, like field_0x234 */
    float mirrorDirZ_0x32c;
    float mirrorDist_0x330;
    /* Lane change. Side: 0/1 = look for a target on the left/right lane of
     * currentLineNodeIdx_0x33c, 2 = idle; armed by the turn-signal buttons.
     * Done: a target was found, until laneOffset_0x2c4 reaches 2.0. */
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
    /* Attribute words [1] and [2] of junctionCSlot_0x378's query; never read. */
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

    /* Passenger door: 0 = shut, 1 = opening, 2 = open, 3 = closing. Only
     * advances while driveState_0x2b4 is 0 (opening) or 1 (closing). */
    int doorState_0x3c0;

    /* Asks doorState_0x3c0 for its next step; cleared once taken. */
    int doorRequest_0x3c4;

    int cameraYawEase_0x3c8;
} BusState;

/* Per-course badge tier, only ever raised. Doubles as a sprite index:
 * 0x18 - tier on the course menu, 0x1f - tier on the results screen. */
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
    Uint8 reserved_0x05[3];
} CourseProgress;

typedef struct {
    int days_0x00;

    int eventProgressFlags_0x04[5];
    int profileProgressFlags_0x18[5];

    int letters_0x2c[6];
    CourseProgress courses_0x44[9];
    int profileUnlockedCount_0x8c;
    int exp_0x90; // also shown in the VMU file comment
    int field_0x94;
    int practiceLessonBestScores_0x98[11];

    /* The SETTING screen's five rows, in screen order. */
    char difficulty_0xc4;
    char driveMode_0xc5;
    char defaultView_0xc6;
    char vibration_0xc7;  /* 0 is on */
    /* 0 is on; nonzero holds the cockpit and first-person views level. */
    char screenRoll_0xc8;

    char reserved_0xc9[3];

    /* KEY CONFIGURE layout, per drive mode x controller type. */
    char btnConfigManual_0xcc;
    char btnConfigAuto_0xcd;
    char btnConfigWheelManual_0xce;
    char btnConfigWheelAuto_0xcf;

    /* KEY CONFIGURE ACCEL/BRAKE: the trigger travel (0-0x80) below which the
     * axis is ignored. */
    unsigned char accelSensitivity_0xd0;
    unsigned char brakeSensitivity_0xd1;
    /* padding */
    char field_0xd2;
    char field_0xd3;

    /* AUDIO MUSIC/SFX/VOICE volume, 0-9 */
    char musicVolume_0xd4;
    char sfxVolume_0xd5;
    char voiceVolume_0xd6;
    char reserved_0xd7; // padding

    /* Saved copies of the same-named var_run* globals and var_award_8c1bb8f8. */
    int runReportPending_0xd8;
    int runWasPractice_0xdc;
    int runSucceeded_0xe0;
    char award_0xe4;
    char reserved_0xe5[3]; // padding
} PlayerProgress;

extern void* var_busFont_8c1ba1c8;
extern PlayerProgress var_progress_8c1ba1cc;
/* bitset; role unclear */
extern int var_runEventFlags_8c1ba2b4;
/* PlayerProgress's event/profile flags as they stood when the drive
 * started, so retiring from the pause menu can roll back. */
extern int var_eventFlagsSnapshot_8c1ba2b8[5];
extern int var_profileFlagsSnapshot_8c1ba2cc[5];
/* VMU save-file images: 16 0x600-byte slots on FILE SELECT, one when
 * saving. -1 when unallocated. */
extern void* var_saveBuf_8c1ba2e0;
extern BUS_BACKUPFILEHEADER var_backupFileHeader_8c1ba2e4;
extern void* var_vmuIconFileBuf_8c1ba344;
extern void* var_backupFileImageBuf_8c1ba348;
extern int var_selectedVm_8c1ba34c;
extern int var_saveSlot_8c1ba350;        // index into init_saveNames_8c044d50
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
/* The run's DRIVE MODE: 0 manual, 1 auto. Auto skips most of the
 * driver-points grading. */
extern int var_driveMode_8c1bb8c8;
extern int var_pauseActive_8c1bb8cc;
extern enum PLAY_MODE var_playMode_8c1bb8d0;
/* Which PLAY_MODE_DEMO: 1 = the attract loop, 0 = a VMU replay. */
extern int var_isAttractDemo_8c1bb8d4;
extern int var_demoIndex_8c1bb8d8;
extern int var_runSucceeded_8c1bb8dc;
extern int var_firstClearOfCourse_8c1bb8e0; // the course had never been played
extern int var_passengerCount_8c1bb8e4;
extern int var_eventCount_8c1bb8e8;
extern int var_worstPenaltyMsgSet_8c1bb8ec;
extern int var_worstPenaltyDelta_8c1bb8f0;
extern int var_penaltyCount_8c1bb8f4;
extern Uint8 var_award_8c1bb8f8;
extern int var_gameMode_8c1bb8fc;
extern int var_cutsceneActive_8c1bb900;
extern NJS_CAMERA var_camera_8c1bb904;
extern NJS_CAMERA var_mirrorCamera_8c1bb944;
/* Driver's eye down the aisle, shown in an inset over the mirror view
 * during an arrival. */
extern NJS_CAMERA var_cabinCamera_8c1bb984;
extern char var_8c1bb9c4[12]; /* unreferenced; declared here to hold its place in B */
extern BusState var_busState_8c1bb9d0;
/* Presumably &var_busState_8c1bb9d0. Traffic code only identity-compares
 * it, as the player's entry among the traffic entries. */
extern BusState *var_playerBus_8c1bbd9c;
/* The ends of the bus lights' crossfade and the per-frame step, from
 * CourseSceneParams.rec0_0x0c rows 1 and 2. Only refreshed at
 * TIME_OF_DAY_NIGHT, so by day these hold the last night run's. A row is
 * {intensity0, intensity1, r, g, b}, split 2+3. "Off" and "On" are the
 * states of lightFadeGate_0x2dc. */
extern float var_nightLightIntensityStep_8c1bbda0[2];
extern float var_nightLightIntensityOff_8c1bbda8[2];
extern float var_nightLightIntensityOn_8c1bbdb0[2];
extern float var_nightLightColorStep_8c1bbdb8[3];
extern float var_nightLightColorOff_8c1bbdc4[3];
extern float var_nightLightColorOn_8c1bbdd0[3];
/* [26] is the player's bus (3s_bus0_l.njd/3t_bus0_l.pvm). */
extern ModelSlot var_routeModelSlots_8c1bbddc[0x20];
extern ModelSlot var_pedestrianAssets_8c1bbfdc[0x41];
extern void* var_routeModels_8c1bc3ec;
extern LoadedModel *var_segmentModels_8c1bc3f0;
extern LoadedModel *var_trafficModels_8c1bc3f4;
extern ResourceGroup var_loadingResourceGroup_8c1bc3f8;
extern void* var_messageTextBoxA_8c1bc404;
extern void* var_messageTextBoxB_8c1bc408;
extern int var_messageTextBoxIndex_8c1bc40c;   /* indexes (&var_messageTextBoxA_8c1bc404)[idx] */
extern NJS_MOTION* var_busDoorMotion_8c1bc410;
extern void* var_busDoorShape_8c1bc414;
/* These three are one ResourceGroup (mark.pvm, mark_parts.dat, mark.dat),
 * reached by casting &var_markTexlist_8c1bc418; 8c1bc424 is the same for the
 * bus stop. Folding each into one struct is a move-data job. */
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
/* fuu.pvm/.njd/.njm: the animated stop marker */
extern void* var_fuuTexlist_8c1bc440;
extern void* var_fuuNj_8c1bc444;
extern NJS_MOTION* var_fuuNjm_8c1bc448;
/* Counts up while the bus approaches a stop, wrapping at
 * var_fuuLastFrame_8c1bc450. */
extern float var_fuuFrame_8c1bc44c;
extern float var_fuuLastFrame_8c1bc450;
extern void* var_vmGameBuf_8c1bc454;
/* IntersectSegments_8c0206f0's output, an XZ pair: the halves must stay
 * adjacent, since callers pass &var_crossingIntersectPoint_8c1bc458 whole. */
extern float var_crossingIntersectPoint_8c1bc458;
extern float var_crossingIntersectPointZ_8c1bc45c;
extern NJS_POINT3 var_groundQueryPoint_8c1bc460; // scratch point for ground-height queries
/* Shared; each user rebuilds it before reading. */
extern NJS_MATRIX var_scratchMatrix_8c1bc46c;

#endif // _1BA1C8_GLOBALS_H
