/* 8c012f44 */
#include <shinobi.h>
#include <njdef.h>
#include "includes.h"
#include "serial_debug.h"
#include "014a9c_tasks.h"
#include "015ab8_title.h"
#include "scif.h"
#include "011120_asset_queues.h"
#include "02fb50_sh4nlfzn_post_data.h"

#define TEX_NUM         3072
// #define CACHE_BUFSIZE   0x20000
// #define SHAPE_BUFSIZE   512
#define RENDER_X        256
#define RENDER_Y        512

struct loadedNj {
    void *field_0x00;
    int *field_0x04;
}
typedef loadedNj;

struct uknStruct2 {
    int field_0x00;
    int field_0x04;
    int field_0x08;
}
typedef uknStruct2;

struct s_8c18ad28 {
    int field_0x00;
    int field_0x04;
    Uint8 field_0x08;
    Uint8 field_0x09;
    Uint8 field_0x0a;
    Uint8 field_0x0b;
    float fogN_0x0c;
    float fogF_0x10;
}
typedef s_8c18ad28;
extern s_8c18ad28 *var_8c18ad28;

// TODO:
struct BusState {
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
}
typedef BusState;

extern NJS_TEXMEMLIST var_tex_8c157af8[TEX_NUM];
extern NJS_TEXNAME    var_texname_8c18acf8[1];
extern Sint8          var_soundMode_8c226070;
extern char           s_DUMMY_8c03346c[];
extern int            const_8c033474;

NJS_TEXLIST init_texlist_8c03bf44 = {var_texname_8c18acf8, 1};
int init_8c03bf48 = 1;
char init_8c03bf4c[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD4, 0x9B, 0x5E, 0x3F, 0x00, 0x00, 0x00, 0x7F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x43, 0xD4, 0x9B, 0x5E, 0x3F, 0x00, 0x00, 0x00, 0x7F,
    0x00, 0x00, 0x20, 0x44, 0x00, 0x00, 0x00, 0x00, 0xD4, 0x9B, 0x5E, 0x3F, 0x00, 0x00, 0x00, 0x7F,
    0x00, 0x00, 0x20, 0x44, 0x00, 0x00, 0xF0, 0x43, 0xD4, 0x9B, 0x5E, 0x3F, 0x00, 0x00, 0x00, 0x7F
};
// Forward declaration
int init_8c03bf8c[7];
int init_8c03bf8c[] = {
    0,
    (int) &s_DUMMY_8c03346c,
    1,
    (int) &init_8c03bf8c,
    (int) &const_8c033474,
    0,
    0,
};
int init_8c03bfa8 = 0;

extern Task var_tasks_8c1ba3c8[];
extern Task var_tasks_8c1ba5e8[];
extern Task var_tasks_8c1ba808[];
extern Task var_tasks_8c1bac28[];
extern Task var_tasks_8c1bb448[];

extern void* var_8c1bb86c;
extern void* var_8c1bc3ec;
extern void* var_8c1bc3f0;
extern void* var_8c1bc3f4;
extern void* var_8c1bc404;
extern void* var_8c226434;
extern void* var_8c226438;
extern void* var_8c228234;
extern void* var_8c227e20;
extern void* var_8c227e24;
extern void* var_8c2288f8;
extern void* var_8c1bc438;
extern void* var_resourceGroup_8c2263a8;
extern void* var_8c1ba2e0;
extern void* var_8c1ba348;
extern void* var_8c1ba344;
extern void* var_currentSysResGroupInfo_8c225fb0;
extern int* var_demoBuf_8c1ba3c4;
extern void* var_8c1bc454;
extern void* var_selectedVm_8c1ba34c;
extern BusState var_busState_8c1bb9d0;

extern void* var_8c1bbddc;
extern void* var_8c1bbfdc;

extern int var_8c1bb8c4;
extern int var_demoIndex_8c1bb8d8;

extern void* var_mark_parts_dat_8c1bc41c;
extern void* var_mark_dat_8c1bc420;
extern void* var_busstop_parts_dat_8c1bc428;
extern void* var_busstop_dat_8c1bc42c;
extern void* var_8c1bc3f8[3];
extern void* var_busFont_8c1ba1c8;
extern void* var_8c2260ac;
extern void* var_8c2260b8;
extern void* var_8c2260c4;
extern void* var_8c1bc440;
extern void* var_8c1bc444;
extern NJS_MOTION* var_loadedFooNjm_8c1bc448;
extern void* var_8c1bc410;
extern void* var_8c1bc414;

extern Uint32 var_vibport_8c1ba354;
extern int init_8c03bd80;
extern int init_8c03bd84;

extern setUknPvmBool_8c014330();
extern int var_gdErr_8c18ad14;

extern NJS_FOG_TABLE var_fogTable_8c18aaf8;

extern uknStruct2 *var_8c1bc824;
extern char init_8c0460b0[];

extern float var_8c1bc450;
extern int var_8c2260a8;
extern int var_demo_8c1bb8d0;
extern task_8c012cbc;
extern task_8c01677e;
extern var_8c1bb8d4;
extern task_8c012d06;
extern task_8c012d5a;
extern task_8c016bf4;
extern var_8c1bb8cc;
extern var_8c22847c;
extern int var_8c1bb868;
extern int var_8c228704;
extern int var_8c1bb8c8;
extern var_8c227dd4;
extern Uint32 var_8c227da0;
extern Uint32 var_8c1ba292;
extern Uint32 var_8c1ba291;
extern int var_8c227da8;
extern task_load_8c014338;
extern var_memblkSource_8c0fcd48;
extern var_memblkSource_8c0fcd4c;

extern var_8c18ad1c;
extern var_8c228708;
extern Bool var_8c22655c;

extern FUN_8c0144fc();
extern Sint8 getSoundMode_8c010924();
extern setSoundMode_8c0108c0(Bool);

/* Matched :) */
void task_8c012f44()
{
    if ((var_resetRequested_8c157a78 != 0) && (var_8c157a7c == 0)) {
        FUN_8c010ca6(0);
        sdMidiStopAll();
        if (var_vibport_8c1ba354 != -1) {
        pdVibMxStop(var_vibport_8c1ba354);
        }
        FUN_8c016182();
        if (var_8c1bb8c4 != 0) {
            init_8c03bd80 = 1;
            init_8c03bd84 = 0;
        } else {
            pushTitle_8c015fd6(1);
        }
    }
}

/* Matched :) */
void task_8c012f9c(Task *task, void* state) {
    Bool r7;
    Float speed_fr2;

    if (var_demo_8c1bb8d0 != 1 && var_8c18ad1c == 2 && var_8c228708 == 0) {
        r7 = TRUE;
    } else {
        r7 = FALSE;
    }

    if (var_busState_8c1bb9d0.speed_0x27c == 0) {
        switch (task->field_0x08) {
            case 0:
                if (var_busState_8c1bb9d0.bus_substate_0x3c0 == 0) {
                    var_8c22655c = 0;
                    if (r7 == FALSE) {
                        var_busState_8c1bb9d0.mirror_0x268 = 2;
                    } else {
                        var_busState_8c1bb9d0.mirror_0x268 = 0;
                    }

                    /* 8c012ff6 */
                    task->field_0x08 = 1;
                } else {
                    if (var_busState_8c1bb9d0.mirror_0x268 != 3) {
                        var_8c22655c = 0;
                        freeTask_8c014b66(task);
                    }
                }
                break;

            case 1:
            default:
                // Nothing...
                break;
        }
    } else {
        var_8c22655c = 0;
        if (r7 == FALSE) {
            var_busState_8c1bb9d0.field_0x25c = 0;
        }

        var_busState_8c1bb9d0.mirror_0x268 = 0;

        freeTask_8c014b66(task);
    }

    njControl3D(0);
}

/** Tested */
void FUN_8c01306e(void)
{
    Task *created_task;
    void* created_state;
    Task *tasks;

    njInitMatrix(var_matrix_8c2f8ca0, 16, 0);
    njSetBackColor(0,0,0);
    njSetFogColor(ARGB(
        var_8c18ad28->field_0x0b,
        var_8c18ad28->field_0x0a,
        var_8c18ad28->field_0x09,
        var_8c18ad28->field_0x08
    ));

    njGenerateFogTable3(var_fogTable_8c18aaf8, var_8c18ad28->fogN_0x0c, var_8c18ad28->fogF_0x10);
    njFogEnable();
    kmSetCheapShadowMode(0x80);
    kmSetFogTable(var_fogTable_8c18aaf8);

    clearTasks_8c014a9c(var_tasks_8c1ba5e8, 0x10);
    clearTasks_8c014a9c(var_tasks_8c1ba808, 0x20);
    clearTasks_8c014a9c(var_tasks_8c1bac28, 0x40);
    clearTasks_8c014a9c(var_tasks_8c1bb448, 0x20);

    njRandomSeed(var_seed_8c157a64);
    AsqSetSeedA_12160(var_seed_8c157a64);
    AsqSetSeedB_121a2(var_seed_8c157a64);

    FUN_8c0128cc(1);

    if (var_demo_8c1bb8d0 != 2) {
        pushTask_8c014ae8(var_tasks_8c1ba3c8, &task_8c012cbc, &created_task, &created_state, 0);
        pushTask_8c014ae8(var_tasks_8c1ba5e8, &task_8c01677e, &created_task, &created_state, 0);
    } else {
        if (var_8c1bb8d4 == 0) {
            pushTask_8c014ae8(var_tasks_8c1ba3c8, &task_8c012d06, &created_task, &created_state, 0);
        } else {
            pushTask_8c014ae8(var_tasks_8c1ba3c8, &task_8c012d5a, &created_task, &created_state, 0);
            created_task->field_0x08 = 0;
            created_task->field_0x0c = (void*) 0;
        }
        pushTask_8c014ae8(var_tasks_8c1ba5e8, &task_8c016bf4, &created_task, &created_state, 0);
        FUN_8c025af4();
    }

    var_8c1bb8cc = 0;
    var_8c22847c = 0;

    FUN_8c023610();
    FUN_8c02845a();

    if (var_demo_8c1bb8d0 != 2) {
        FUN_8c029920();
    }

    FUN_8c0296d6();
    FUN_8c02769e();
    FUN_8c0222dc();
    FUN_8c02a6ac();
    FUN_8c02c46a();
    FUN_8c02018c();
    FUN_8c02d968();
    FUN_8c020528();
    pushTask_8c014ae8(var_tasks_8c1ba5e8, &task_8c012f9c, &created_task, &created_state, 0);
    created_task->field_0x08 = 0;
    FUN_8c0228a2();
}

/* Matched :) */
void FUN_8c01328c() {
    Task *created_task;
    void* created_state;
  
    if (var_demo_8c1bb8d0 == 0) {
        var_8c1bb868 = var_8c1bc824->field_0x00;
        var_8c228704 = var_8c1bc824->field_0x04;
        var_8c1bb8c8 = var_8c1bc824->field_0x08;
        var_seed_8c157a64 = AsqGetRandomA_12166();
    } else if ((var_demo_8c1bb8d0 == 2) && (var_8c1bb8d4 != 0)) {
        var_8c227dd4 = init_8c0460b0[var_8c1bb868 - 0x26];
        FUN_8c01895e();
    } else {
        var_8c227dd4 = 0;
    }

    njRandomSeed(var_seed_8c157a64);
    AsqSetSeedA_12160(var_seed_8c157a64);
    AsqSetSeedB_121a2(var_seed_8c157a64);
    AsqFUN_121e8();
    var_8c227da0 = (char) var_8c1ba292;
    var_8c227da8 = 0;

    FUN_8c0144fc();
}

/* Matched :) */
void pushLoadingTask_8c013310(int p1) {
    Task *created_task;
    void* created_state;
  
    if (var_demo_8c1bb8d0 != 2) {
        var_8c1bb868 = p1;
        var_8c228704 = 0;
        var_8c1bb8c8 = (char) var_8c1ba291;
        var_seed_8c157a64 = AsqGetRandomA_12166();
    } else if (var_demo_8c1bb8d0 == 2 && var_8c1bb8d4 != 0) {
        var_8c227dd4 = init_8c0460b0[var_8c1bb868 - 0x26];
    } else {
        var_8c227dd4 = 0;
    }

    njRandomSeed(var_seed_8c157a64);
    AsqSetSeedA_12160(var_seed_8c157a64);
    AsqSetSeedB_121a2(var_seed_8c157a64);
    AsqFUN_121e8();
    var_8c227da0 = (char) var_8c1ba292;
    var_8c227da8 = 0;

    FUN_8c0144fc();
}

/** Tested */
void task_8c013388(Task *task, void *state) {
    switch (task->field_0x08) {
        case 0: {
            /* 8c013440 */
            Bool b = getUknPvmBool_8c01432a();
            if (b) {
                task->field_0x08++;
                var_8c1bc450 = (Float) var_loadedFooNjm_8c1bc448->nbFrame - 1;

                AsqResetQueues_11f6c();
                AsqRequestDat_11182("\\SOUND", "manatee.drv", &var_memblkSource_8c0fcd48);
                AsqRequestDat_11182("\\SOUND", "bus.mlt", &var_memblkSource_8c0fcd4c);
                resetUknPvmBool_8c014322();
                AsqProcessQueues_11fe0(&AsqNop_11120, 0, 0, 0, &setUknPvmBool_8c014330);
            }
            break;
        }
        case 1: {
            /* 8c0133a0, 8c0134ce */
            if (getUknPvmBool_8c01432a() != 0) {
                AsqFreeQueues_11f7e();
                freeTask_8c014b66(task);
                initSoundMidiAdx_8c010e18();
                var_8c2260a8 = 1;
                pushTitle_8c015fd6(0);
            }
            break;
        }
        default:
            /* 8c0134a0 */
            break;
    }
}

void usrGdErrFunc_8c0134d6(void *obj, Sint32 errcode) {
  if (errcode == GDD_ERR_TRAYOPEND || errcode == GDD_ERR_UNITATTENT) {
    var_gdErr_8c18ad14 = 1;
  }
}

/* Tested */
void njUserInit_8c0134ec() {
    NJS_TEXINFO info;
    Task *created_task;
    void *created_state;

#ifdef SERIAL_DEBUG
    scif_init(57600);
#endif

    /* 8c0134fc */
    njSetTextureMemorySize(0x100000);

    if (syCblCheckCable() == SYE_CBL_CABLE_VGA) {
        sbInitSystem_8c0149b0(NJD_RESOLUTION_VGA, NJD_FRAMEBUFFER_MODE_RGB565, 2);
    } else {
        /* TODO: Test this block */
        sbInitSystem_8c0149b0(NJD_RESOLUTION_640x480_NTSCNI, NJD_FRAMEBUFFER_MODE_RGB565, 2);
        njSetAspect(1, 0.91);
    }

    njInitMatrix(var_matrix_8c2f8ca0, 16, 0);
    njInit3D(var_vbuf_8c255ca0, 2048);
    njInitVertexBuffer(800000, 320000, 320000, 320000, 20000);
    njInitTextureBuffer(var_texbuf_8c277ca0, TEX_BUFSIZE);
    njInitTexture(var_tex_8c157af8, TEX_NUM);
    njInitCacheTextureBuffer(var_cachebuf_8c235ca0, CACHE_BUFSIZE);
    njInitShape(var_shapebuf_8c2f84a0);
    syRtcInit();

    var_soundMode_8c226070 = getSoundMode_8c010924();
    if (var_soundMode_8c226070 >= 0) {
        setSoundMode_8c0108c0(var_soundMode_8c226070);
    } else {
        setSoundMode_8c0108c0(SYD_CFG_STEREO);
    }

    vibClear_8c010fbe();
    bupInit_8c014b8c();

    njSetTextureInfo(&info, (Uint16 *) var_texbuf_8c277ca0, NJD_TEXFMT_STRIDE | NJD_TEXFMT_RGB_565, RENDER_X, RENDER_Y);

    njSetTextureName(&var_texname_8c18acf8[0], &info, 999, NJD_TEXATTR_TYPE_MEMORY|NJD_TEXATTR_GLOBALINDEX);
    njSetRenderWidth(256);
    njLoadTexture(&init_texlist_8c03bf44);

    clearTasks_8c014a9c(var_tasks_8c1ba3c8, 0x10);
    clearTasks_8c014a9c(var_tasks_8c1ba5e8, 0x10);
    clearTasks_8c014a9c(var_tasks_8c1ba808, 0x20);
    clearTasks_8c014a9c(var_tasks_8c1bac28, 0x40);
    clearTasks_8c014a9c(var_tasks_8c1bb448, 0x20);

    var_8c1bb86c = (void *) -1;

    clearUnknowArray_8c013bbc(&var_8c1bbddc, 0x20);
    clearUnknowArray_8c013bbc(&var_8c1bbfdc, 0x41);

    var_8c1bc3ec = (void *) -1;
    var_8c1bc3f0 = (void *) -1;
    var_8c1bc3f4 = (void *) -1;

    clearUnknownVar_8c02171c();
    clearUnknownVar_8c029acc();
    clearUnknownVars_8c02aa28();

    var_8c1bc404 = (void *) -1;
    var_8c226434 = (void *) -1;
    var_8c226438 = (void *) -1;
    var_8c228234 = (void *) -1;
    var_8c227e20 = (void *) -1;
    var_8c227e24 = (void *) -1;
    var_8c2288f8 = (void *) -1;
    var_8c1bc438 = (void *) -1;
    menuState_8c1bc7a8.resourceGroupA_0x00.tlist_0x00 = (void*) -1;
    menuState_8c1bc7a8.resourceGroupB_0x0c.tlist_0x00 = (void*) -1;
    var_resourceGroup_8c2263a8 = (void *) -1;
    var_8c1ba2e0 = (void *) -1;
    var_8c1ba348 = (void *) -1;
    var_8c1ba344 = (void *) -1;
    var_currentSysResGroupInfo_8c225fb0 = (void *) -1;
    var_demoBuf_8c1ba3c4 = (void *) -1;
    var_8c1bc454 = (void *) -1;
    var_selectedVm_8c1ba34c = (void *) -1;

    var_8c1bb8c4 = 0;

    // Set high index to trigger loop and ensure first demo runs
    var_demoIndex_8c1bb8d8 = 100;
    var_8c157a6c = 0;

    FUN_8c01c8dc();
    FUN_8c0189d2();
    njSetBorderColor(0);
    vmsLcd_8c01c8fc(3);
    vmsLcd_8c01c910();

    pushTask_8c014ae8(var_tasks_8c1ba3c8, &task_8c013388, &created_task, &created_state, 0);
    created_task->field_0x08 = 0;

    AsqInitQueues_11f36(16, 8, 0, 8);
    AsqResetQueues_11f6c();

    AsqRequestDat_11182("\\SYSTEM", "mark_parts.dat", &var_mark_parts_dat_8c1bc41c);
    AsqRequestDat_11182("\\SYSTEM", "mark.dat", &var_mark_dat_8c1bc420);
    AsqRequestDat_11182("\\SYSTEM", "busstop_parts.dat", &var_busstop_parts_dat_8c1bc428);
    AsqRequestDat_11182("\\SYSTEM", "busstop.dat", &var_busstop_dat_8c1bc42c);

    /*  TODO: Fix var_8c1bc3f8 type */ 
    AsqRequestPvm_11ac0("\\SYSTEM", "loading.pvm", &var_8c1bc3f8[0], 1, 0x80000000);
    AsqRequestDat_11182("\\SYSTEM", "load_parts.dat", &var_8c1bc3f8[1]);
    AsqRequestDat_11182("\\SYSTEM", "loading.dat", &var_8c1bc3f8[2]);

    AsqRequestDat_11182("\\SYSTEM", "bus_font.fff", &var_busFont_8c1ba1c8);
    AsqRequestDat_11182("\\SYSTEM", "vm_bus.lcd", &var_8c2260ac);
    AsqRequestDat_11182("\\SYSTEM", "vm_danger.lcd", &var_8c2260b8);
    AsqRequestDat_11182("\\SYSTEM", "now_loading.lcd", &var_8c2260c4);

    AsqRequestPvm_11ac0("\\SYSTEM", "fuu.pvm", &var_8c1bc440, 1, 0);
    AsqRequestNj_11492("\\SYSTEM", "fuu.njd", &var_8c1bc444, 0);
    AsqRequestNj_11492("\\SYSTEM", "fuu.njm", &var_loadedFooNjm_8c1bc448, 0);

    AsqRequestNj_11492("\\SD_COMMON","3s_bus_m2.njm", &var_8c1bc410, 0);
    AsqRequestNj_11492("\\SD_COMMON","3s_bus_m2.njs", &var_8c1bc414, 0);

    resetUknPvmBool_8c014322();
    AsqProcessQueues_11fe0(&AsqNop_11120, 0, 0, 0, &setUknPvmBool_8c014330);
    var_gdErr_8c18ad14 = 0;
    gdFsEntryErrFuncAll(&usrGdErrFunc_8c0134d6, (void *) 0);
}

/* TODO: Test */
int njUserMain_8c01392e(void) {
    GDFS gdfs;
    Sint32 stat;

    if (init_8c03bd80 != 0) {
        /* 8c01393e */
        /* Hit on title screen, after fadein */
        if (init_8c03bd84 == 0) {
            if (var_vibport_8c1ba354 != -1) {
                pdVibMxStop(var_vibport_8c1ba354);
            }

            /* 8c0139be (shared) */
            return -1;
        }

        execTasks_8c014b42(var_tasks_8c1ba3c8);
        return 0;
    }

    /* 8c013956 */
    /* Hit just before logo/menu */
    if (var_queuesAreInitialized_8c157a60 == 0) {
        /* 8c01395e */
        if (init_8c03bfa8 == 0) {
            /* 8c013966 */
            if (!gdFsReqDrvStat()) {
                init_8c03bfa8 = 1;
            }
        } else {
            gdfs = gdFsGetSysHn();
            stat = gdFsGetStat(gdfs);
            if (stat != GDD_STAT_BUSY) {
                init_8c03bfa8 = 0;
            }
        }
    }

    /* 8c01398a */
    stat = gdFsGetDrvStat();
    if (stat == GDD_DRVSTAT_OPEN) {
        /* 8c0139b2 */
        if (var_vibport_8c1ba354 != -1) {
            pdVibMxStop(var_vibport_8c1ba354);
        }

        return -1;
    }

    /* 8c013994 */
    stat = gdFsGetDrvStat();
    if ((stat == GDD_DRVSTAT_OPEN) || (stat == GDD_DRVSTAT_BUSY)) {
        /* 8c0139a4 */
        gdFsReqDrvStat();
    }

    /* 8c0139aa */
    if (var_gdErr_8c18ad14 != 0) {
        /* 8c0139b2 */
        if (var_vibport_8c1ba354 != -1) {
            pdVibMxStop(var_vibport_8c1ba354);
        }

        return -1;
    };

    execTasks_8c014b42(var_tasks_8c1ba3c8);
    return 0;
}

void njUserExit_8c0139d4(void) {
  njExitTexture();
  sbExitSystem_8c014a24();
  syBtExit();
}
