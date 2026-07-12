/* 8c1ba1c8: undecompiled BSS block (section B) referenced from many units */
#ifndef _014F54_TEXT_PRE_DATA_H
#define _014F54_TEXT_PRE_DATA_H

#include <shinobi.h>
#include "011120_asset_queues.h"
#include "013ae8_route_load.h"
#include "014a9c_tasks.h"
#include "014b8c_backup.h"
#include "014f54_text.h"

/* =================
 * Type Declarations
 * =================
 */

// TODO:
typedef struct {
    int field_0x000;
    int field_0x004;
    int field_0x008;
    int field_0x00c;
    int field_0x010;
    int field_0x014;
    int field_0x018;
    int field_0x01c;
    int field_0x020;
    int field_0x024;
    int field_0x028;
    int field_0x02c;
    int field_0x030;
    int field_0x034;
    int field_0x038;
    int field_0x03c;
    int field_0x040;
    int field_0x044;
    int field_0x048;
    int field_0x04c;
    int field_0x050;
    int field_0x054;
    int field_0x058;
    int field_0x05c;
    int field_0x060;
    int field_0x064;
    int field_0x068;
    int field_0x06c;

    int distance_traveled_0x070;
    int ang_0x074;
    int acc_0x078;
    int ang_0x07c;
    int blinker_0x080;
    int field_0x084;

    int field_0x088;
    int field_0x08c;
    int field_0x090;
    int field_0x094;
    int field_0x098;
    int field_0x09c;
    int field_0x0a0;
    int field_0x0a4;
    int field_0x0a8;
    int field_0x0ac;
    int field_0x0b0;
    int field_0x0b4;
    int field_0x0b8;
    int field_0x0bc;
    int field_0x0c0;
    int field_0x0c4;
    int field_0x0c8;
    int field_0x0cc;
    int field_0x0d0;
    int field_0x0d4;
    int field_0x0d8;
    int field_0x0dc;
    int field_0x0e0;
    int field_0x0e4;
    int field_0x0e8;
    int field_0x0ec;
    int field_0x0f0;

    float field_0x0f4;

    int field_0x0f8;
    int field_0x0fc;

    float field_0x100;

    int field_0x104;
    int field_0x108;
    int field_0x10c;
    int field_0x110;
    int field_0x114;
    int field_0x118;
    int field_0x11c;
    int field_0x120;
    int field_0x124;
    int field_0x128;
    int field_0x12c;
    int field_0x130;
    int field_0x134;
    int field_0x138;
    int field_0x13c;
    int field_0x140;
    int field_0x144;
    int field_0x148;
    int field_0x14c;
    int field_0x150;
    int field_0x154;
    int field_0x158;
    int field_0x15c;
    int field_0x160;
    int field_0x164;
    int field_0x168;
    int field_0x16c;
    int field_0x170;
    int field_0x174;
    int field_0x178;
    int field_0x17c;
    int field_0x180;
    int field_0x184;
    int field_0x188;
    int field_0x18c;
    int field_0x190;
    int field_0x194;
    int field_0x198;
    int field_0x19c;
    int field_0x1a0;
    int field_0x1a4;
    int field_0x1a8;
    int field_0x1ac;
    int field_0x1b0;
    int field_0x1b4;
    int field_0x1b8;
    int field_0x1bc;
    int field_0x1c0;
    int field_0x1c4;
    int field_0x1c8;
    int field_0x1cc;
    int field_0x1d0;
    int field_0x1d4;
    int field_0x1d8;
    int field_0x1dc;
    int field_0x1e0;
    int field_0x1e4;
    int field_0x1e8;
    int field_0x1ec;
    int field_0x1f0;
    int field_0x1f4;
    int field_0x1f8;
    int field_0x1fc;
    int field_0x200;
    int field_0x204;
    int field_0x208;
    int field_0x20c;
    int field_0x210;
    int field_0x214;
    int field_0x218;
    int field_0x21c;
    int field_0x220;
    int field_0x224;
    int field_0x228;
    int field_0x22c;
    int field_0x230;
    int field_0x234;
    int field_0x238;
    int field_0x23c;
    int field_0x240;
    int field_0x244;
    int field_0x248;
    int field_0x24c;

    int ang_0x250;

    int field_0x254;

    int ang_0x258;

    int field_0x25c;
    int field_0x260;
    int field_0x264;

    int mirror_0x268;

    int field_0x26c;
    int field_0x270;
    int field_0x274;

    float field_0x278;
    float speed_0x27c;
    float acc_hist_0x280[4];

    int field_0x290;
    int field_0x294;
    int field_0x298;
    int field_0x29c;
    int field_0x2a0;
    int field_0x2a4;
    int field_0x2a8;
    int field_0x2ac;
    int field_0x2b0;

    int bus_state_0x2b4;

    int field_0x2b8;
    int field_0x2bc;
    int field_0x2c0;
    int field_0x2c4;
    int field_0x2c8;
    int field_0x2cc;
    int field_0x2d0;
    int field_0x2d4;
    int field_0x2d8;
    int field_0x2dc;
    int field_0x2e0;
    int field_0x2e4;
    int field_0x2e8;
    int field_0x2ec;
    int field_0x2f0;

    int gear_0x2f4;

    int field_0x2f8;
    int field_0x2fc;
    int field_0x300;
    int field_0x304;
    int field_0x308;
    int field_0x30c;
    int field_0x310;
    int field_0x314;
    int field_0x318;
    int field_0x31c;
    int field_0x320;
    int field_0x324;
    int field_0x328;
    int field_0x32c;
    int field_0x330;
    int field_0x334;
    int field_0x338;
    int field_0x33c;
    int field_0x340;
    int field_0x344;
    int field_0x348;
    int field_0x34c;
    int field_0x350;
    int field_0x354;
    int field_0x358;
    int field_0x35c;
    int field_0x360;
    int field_0x364;
    int field_0x368;
    int field_0x36c;
    int field_0x370;
    int field_0x374;
    int field_0x378;
    int field_0x37c;
    int field_0x380;
    int field_0x384;
    int field_0x388;
    int field_0x38c;
    int field_0x390;
    int field_0x394;
    int field_0x398;
    int field_0x39c;
    int field_0x3a0;
    int field_0x3a4;
    int field_0x3a8;
    int field_0x3ac;
    int field_0x3b0;
    int field_0x3b4;
    int field_0x3b8;
    int field_0x3bc;

    int bus_substate_0x3c0;

    int field_0x3c4;
} BusState;

typedef struct {
    Uint8 unlocked_0x00;
    Uint8 new_0x01;
    Uint8 field_0x02;
    Uint8 storySpriteNo_0x03;
    Uint8 freeRunSpriteNo_0x04;
    Uint8 field_0x05[3]; // Padding?
} CourseProgress;

typedef struct {
    int days_0x00;

    /* unlock-flag bitsets set together by FUN_8c02af78
     * and tested individually by FUN_8c02afbe/FUN_8c02aff0 */
    int field_0x04[5];
    int field_0x18[5];

    int letters_0x2c[6];
    CourseProgress courses_0x44[9];
    int field_0x8c;
    int exp_0x90;
    char field_0x94[4];
    int field_0x98[11];
    signed char field_0xc4;
    char field_0xc5;
    char field_0xc6;

    /* covers the bytes 011120_asset_queues.c indexes at 0xcc-0xcf */
    char field_0xc7[9];

    /* 0xd0/0xd1 look like saved input deadzone
     * thresholds (see FUN_8c024320/FUN_8c024606) */
    char field_0xd0;
    char field_0xd1;
} PlayerProgress;

/* single-word bitset, set/tested by FUN_8c02b022/FUN_8c02b030; role unclear */
extern int var_8c1ba2b4;

extern int var_8c1ba2b8[5]; // Maybe progress backup
extern int var_8c1ba2cc[5]; // Maybe progress backup
extern void* var_8c1ba2e0;
extern void* var_8c1ba344;
extern void* var_8c1ba348;
extern void* var_8c1bb86c;
extern int var_8c1bb8b8; // Maybe courseMenuHasResult or courseMenuHasDialog
extern int var_8c1bb8bc;
extern int var_8c1bb8c4;
extern int var_8c1bb8cc;
extern int var_8c1bb8d4;
extern int var_8c1bb8dc;
extern int var_8c1bb8e0; // course was unlocked
extern int var_8c1bb8e4;
extern int var_8c1bb8e8;
extern int var_8c1bb8ec;
extern int var_8c1bb8f0;
extern int var_8c1bb8f4;
extern float var_8c1bbc4c;
extern int var_8c1bbc84;
extern Uint32 var_8c1bbcb0;
extern int var_8c1bbcc4;
extern void* var_8c1bc404;
extern void* var_8c1bc410;
extern void* var_8c1bc414;
extern void* var_8c1bc440;
extern void* var_8c1bc444;
extern float var_8c1bc450;
extern void* var_8c1bc454;
extern int var_award_8c1bb8f8;
extern void* var_busFont_8c1ba1c8;
extern BusState var_busState_8c1bb9d0;
extern void* var_busstopDat_8c1bc42c;
extern void* var_busstopPartsDat_8c1bc428;
extern NJS_TEXLIST *var_busStopTexlist_8c1bc424;
extern CurrentCourse var_currentCourse_8c1bb868;
extern int var_cutsceneActive_8c1bb900;
extern int* var_demoBuf_8c1ba3c4;
extern int var_demoIndex_8c1bb8d8;
extern void *var_frontNj_8c1bc434;
extern NJS_TEXLIST *var_frontTexlist_8c1bc430;
extern int var_gameMode_8c1bb8fc;
extern BACKUPINFO var_gBupInfo_8c1bc4ac[8];
extern int var_inputMapSel_8c1bb8c8;
extern void *var_interiorNj_8c1bc43c;
extern NJS_TEXLIST *var_interiorTexlist_8c1bc438;
extern NJS_MOTION* var_loadedFooNjm_8c1bc448;
extern ResourceGroup var_loadingResourceGroup_8c1bc3f8;
extern void* var_markDat_8c1bc420;
extern void* var_markPartsDat_8c1bc41c;
extern NJS_TEXLIST *var_markTexlist_8c1bc418;
extern ModelSlot var_pedestrianAssets_8c1bbfdc[0x41];
extern PDS_PERIPHERAL *var_peripheral_8c1ba358;
extern PDS_PERIPHERAL var_peripherals_8c1ba35c[2];
extern enum PLAY_MODE var_playMode_8c1bb8d0;
extern PlayerProgress var_progress_8c1ba1cc;
extern void* var_routeModels_8c1bc3ec;
extern ModelSlot var_routeModelSlots_8c1bbddc[0x20];
extern LoadedModel *var_segmentModels_8c1bc3f0;
extern int var_selectedVm_8c1ba34c;
extern int var_shouldShowFreeRunIntro_8c1bb8c0;
extern Task var_tasks_8c1ba3c8[];
extern Task var_tasks_8c1ba5e8[];
extern Task var_tasks_8c1ba808[];
extern Task var_tasks_8c1bac28[];
extern Task var_tasks_8c1bb448[];
extern LoadedModel *var_trafficModels_8c1bc3f4;
extern Uint32 var_vibport_8c1ba354;

#endif // _014F54_TEXT_PRE_DATA_H
