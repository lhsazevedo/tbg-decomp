/* @unit Game */
/* 8c012f44 */
#include <shinobi.h>
#include <njdef.h>
#include "012f44_game.h"
#include "0100bc_sound.h"
#include "014f54_text.h"
#include "016bf4_demo_input.h"
#include "0129cc_pause.h"
#include "01614c.h"
#include "sectionD.h"
#include "014f54_text_pre_data.h"
#include "sectionB.h"
#include "includes.h"
#include "serial_debug.h"
#include "014a9c_tasks.h"
#include "015ab8_title.h"
#include "scif.h"
#include "011120_asset_queues.h"
#include "013ae8_route_load.h"
#include "014b8c_backup.h"
#include "02fb50_sh4nlfzn_post_data.h"

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

NJS_TEXMEMLIST var_tex_8c157af8[TEX_NUM];
STATIC NJS_TEXNAME    var_texname_8c18acf8[1];

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
    (int) &"DUMMY",
    1,
    (int) &init_8c03bf8c,
    (int) &"",
    0,
    0,
};
int init_8c03bfa8 = 0;

STATIC int var_gdErr_8c18ad14;

STATIC NJS_FOG_TABLE var_fogTable_8c18aaf8;

int var_pauseSettle_8c18ad04;
int var_retirePhase_8c18ad08;
int var_confirmChoice_8c18ad0c;
int var_onRetire_8c18ad10;

/* Matched :) */
void GameTask_8c012f44()
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
            TitlePushTitle_8c015fd6(1);
        }
    }
}

/* Matched :) */
STATIC void task_8c012f9c(Task *task, void* state) {
    Bool r7;
    Float speed_fr2;

    if (var_playMode_8c1bb8d0 != 1 && var_route_8c18ad1c == 2 && var_currentSegment_8c228708 == 0) {
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
                        TaskFree_8c014b66(task);
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

        TaskFree_8c014b66(task);
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
        var_fogParams_8c18ad28->field_0x0b,
        var_fogParams_8c18ad28->field_0x0a,
        var_fogParams_8c18ad28->field_0x09,
        var_fogParams_8c18ad28->field_0x08
    ));

    njGenerateFogTable3(var_fogTable_8c18aaf8, var_fogParams_8c18ad28->fogN_0x0c, var_fogParams_8c18ad28->fogF_0x10);
    njFogEnable();
    kmSetCheapShadowMode(0x80);
    kmSetFogTable(var_fogTable_8c18aaf8);

    TaskClear_8c014a9c(var_tasks_8c1ba5e8, 0x10);
    TaskClear_8c014a9c(var_tasks_8c1ba808, 0x20);
    TaskClear_8c014a9c(var_tasks_8c1bac28, 0x40);
    TaskClear_8c014a9c(var_tasks_8c1bb448, 0x20);

    njRandomSeed(var_seed_8c157a64);
    AsqSetSeedA_8c012160(var_seed_8c157a64);
    AsqSetSeedB_8c0121a2(var_seed_8c157a64);

    InputPushTask_8c0128cc(1);

    if (var_playMode_8c1bb8d0 != 2) {
        TaskPush_8c014ae8(var_tasks_8c1ba3c8, &PauseTask_8c012cbc, &created_task, &created_state, 0);
        TaskPush_8c014ae8(var_tasks_8c1ba5e8, &task_8c01677e, &created_task, &created_state, 0);
    } else {
        if (var_8c1bb8d4 == 0) {
            TaskPush_8c014ae8(var_tasks_8c1ba3c8, &PauseToggleTask_8c012d06, &created_task, &created_state, 0);
        } else {
            TaskPush_8c014ae8(var_tasks_8c1ba3c8, &PauseDemoEndTask_8c012d5a, &created_task, &created_state, 0);
            created_task->field_0x08 = 0;
            created_task->field_0x0c = (void*) 0;
        }
        TaskPush_8c014ae8(var_tasks_8c1ba5e8, &DemoInputTask_8c016bf4, &created_task, &created_state, 0);
        FUN_8c025af4();
    }

    var_pauseActive_8c1bb8cc = 0;
    var_8c22847c = 0;

    FUN_8c023610();
    FUN_8c02845a();

    if (var_playMode_8c1bb8d0 != 2) {
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
    TaskPush_8c014ae8(var_tasks_8c1ba5e8, &task_8c012f9c, &created_task, &created_state, 0);
    created_task->field_0x08 = 0;
    FUN_8c0228a2();
}

/* Matched :) */
void FUN_8c01328c() {
    Task *created_task;
    void* created_state;
  
    if (var_playMode_8c1bb8d0 == 0) {
        var_currentCourse_8c1bb868.courseId_0x00 = ((uknStruct2*)var_8c1bc824)->field_0x00;
        var_8c228704 = ((uknStruct2*)var_8c1bc824)->field_0x04;
        var_inputMapSel_8c1bb8c8 = ((uknStruct2*)var_8c1bc824)->field_0x08;
        var_seed_8c157a64 = AsqGetRandomA_8c012166();
    } else if ((var_playMode_8c1bb8d0 == 2) && (var_8c1bb8d4 != 0)) {
        var_8c227dd4 = init_8c0460b0[var_currentCourse_8c1bb868.courseId_0x00 - 0x26];
        FUN_8c01895e();
    } else {
        var_8c227dd4 = 0;
    }

    njRandomSeed(var_seed_8c157a64);
    AsqSetSeedA_8c012160(var_seed_8c157a64);
    AsqSetSeedB_8c0121a2(var_seed_8c157a64);
    AsqApplyButtonConfig_8c0121e8();
    var_8c227da0 = var_progress_8c1ba1cc.field_0xc6;
    var_8c227da8 = 0;

    RouteLoadPushTask_8c0144fc();
}

/* Matched :) */
void GamePushLoadingTask_8c013310(int p1) {
    Task *created_task;
    void* created_state;
  
    if (var_playMode_8c1bb8d0 != 2) {
        var_currentCourse_8c1bb868.courseId_0x00 = p1;
        var_8c228704 = 0;
        var_inputMapSel_8c1bb8c8 = var_progress_8c1ba1cc.field_0xc5;
        var_seed_8c157a64 = AsqGetRandomA_8c012166();
    } else if (var_playMode_8c1bb8d0 == 2 && var_8c1bb8d4 != 0) {
        var_8c227dd4 = init_8c0460b0[var_currentCourse_8c1bb868.courseId_0x00 - 0x26];
    } else {
        var_8c227dd4 = 0;
    }

    njRandomSeed(var_seed_8c157a64);
    AsqSetSeedA_8c012160(var_seed_8c157a64);
    AsqSetSeedB_8c0121a2(var_seed_8c157a64);
    AsqApplyButtonConfig_8c0121e8();
    var_8c227da0 = var_progress_8c1ba1cc.field_0xc6;
    var_8c227da8 = 0;

    RouteLoadPushTask_8c0144fc();
}

/** Tested */
STATIC void task_8c013388(Task *task, void *state) {
    switch (task->field_0x08) {
        case 0: {
            /* 8c013440 */
            Bool b = RouteLoadIsPvmReady_8c01432a();
            if (b) {
                task->field_0x08++;
                var_8c1bc450 = (Float) var_loadedFooNjm_8c1bc448->nbFrame - 1;

                AsqResetQueues_8c011f6c();
                AsqRequestDat_8c011182("\\SOUND", "manatee.drv", &var_memblkSource_8c0fcd48);
                AsqRequestDat_8c011182("\\SOUND", "bus.mlt", &var_memblkSource_8c0fcd4c);
                RouteLoadResetPvmReady_8c014322();
                AsqProcessQueues_8c011fe0(&AsqNop_8c011120, 0, 0, 0, &RouteLoadSetPvmReady_8c014330);
            }
            break;
        }
        case 1: {
            /* 8c0133a0, 8c0134ce */
            if (RouteLoadIsPvmReady_8c01432a() != 0) {
                AsqFreeQueues_8c011f7e();
                TaskFree_8c014b66(task);
                SndInitSoundMidiAdx_8c010e18("\\SOUND");
                var_8c2260a8 = 1;
                TitlePushTitle_8c015fd6(0);
            }
            break;
        }
        default:
            /* 8c0134a0 */
            break;
    }
}

STATIC void usrGdErrFunc_8c0134d6(void *obj, Sint32 errcode) {
  if (errcode == GDD_ERR_TRAYOPEND || errcode == GDD_ERR_UNITATTENT) {
    var_gdErr_8c18ad14 = 1;
  }
}

/* Tested */
void GameInit_8c0134ec() {
    NJS_TEXINFO info;
    Task *created_task;
    void *created_state;

#ifdef SERIAL_DEBUG
    scif_init(57600);
#endif

    /* 8c0134fc */
    njSetTextureMemorySize(0x100000);

    if (syCblCheckCable() == SYE_CBL_CABLE_VGA) {
        SbInitSystem_8c0149b0(NJD_RESOLUTION_VGA, NJD_FRAMEBUFFER_MODE_RGB565, 2);
    } else {
        /* TODO: Test this block */
        SbInitSystem_8c0149b0(NJD_RESOLUTION_640x480_NTSCNI, NJD_FRAMEBUFFER_MODE_RGB565, 2);
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

    var_soundMode_8c226070 = SndGetSoundMode_8c010924();
    if (var_soundMode_8c226070 >= 0) {
        SndSetSoundMode_8c0108c0(var_soundMode_8c226070);
    } else {
        SndSetSoundMode_8c0108c0(SYD_CFG_STEREO);
    }

    VibClear_8c010fbe();
    BupInit_8c014b8c();

    njSetTextureInfo(&info, (Uint16 *) var_texbuf_8c277ca0, NJD_TEXFMT_STRIDE | NJD_TEXFMT_RGB_565, RENDER_X, RENDER_Y);

    njSetTextureName(&var_texname_8c18acf8[0], &info, 999, NJD_TEXATTR_TYPE_MEMORY|NJD_TEXATTR_GLOBALINDEX);
    njSetRenderWidth(256);
    njLoadTexture(&init_texlist_8c03bf44);

    TaskClear_8c014a9c(var_tasks_8c1ba3c8, 0x10);
    TaskClear_8c014a9c(var_tasks_8c1ba5e8, 0x10);
    TaskClear_8c014a9c(var_tasks_8c1ba808, 0x20);
    TaskClear_8c014a9c(var_tasks_8c1bac28, 0x40);
    TaskClear_8c014a9c(var_tasks_8c1bb448, 0x20);

    var_8c1bb86c = (void *) -1;

    RouteLoadClearModelSlots_8c013bbc(var_routeModelSlots_8c1bbddc, 0x20);
    RouteLoadClearModelSlots_8c013bbc(var_pedestrianAssets_8c1bbfdc, 0x41);

    var_routeModels_8c1bc3ec = (void *) -1;
    var_segmentModels_8c1bc3f0 = (LoadedModel *) -1;
    var_trafficModels_8c1bc3f4 = (LoadedModel *) -1;

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
    var_interiorTexlist_8c1bc438 = (NJS_TEXLIST *) -1;
    var_menuState_8c1bc7a8.resourceGroupA_0x00.tlist_0x00 = (void*) -1;
    var_menuState_8c1bc7a8.resourceGroupB_0x0c.tlist_0x00 = (void*) -1;
    var_resourceGroup_8c2263a8 = (ResourceGroup *) -1;
    var_8c1ba2e0 = (void *) -1;
    var_8c1ba348 = (void *) -1;
    var_8c1ba344 = (void *) -1;
    var_currentSysResGroupInfo_8c225fb0 = (void *) -1;
    var_demoBuf_8c1ba3c4 = (void *) -1;
    var_8c1bc454 = (void *) -1;
    var_selectedVm_8c1ba34c = -1;

    var_8c1bb8c4 = 0;

    // Set high index to trigger loop and ensure first demo runs
    var_demoIndex_8c1bb8d8 = 100;
    var_loadScreenActive_8c157a6c = 0;

    FUN_8c01c8dc();
    FUN_8c0189d2();
    njSetBorderColor(0);
    vmsLcd_8c01c8fc(3);
    vmsLcd_8c01c910();

    TaskPush_8c014ae8(var_tasks_8c1ba3c8, &task_8c013388, &created_task, &created_state, 0);
    created_task->field_0x08 = 0;

    AsqInitQueues_8c011f36(16, 8, 0, 8);
    AsqResetQueues_8c011f6c();

    AsqRequestDat_8c011182("\\SYSTEM", "mark_parts.dat", &var_markPartsDat_8c1bc41c);
    AsqRequestDat_8c011182("\\SYSTEM", "mark.dat", &var_markDat_8c1bc420);
    AsqRequestDat_8c011182("\\SYSTEM", "busstop_parts.dat", &var_busstopPartsDat_8c1bc428);
    AsqRequestDat_8c011182("\\SYSTEM", "busstop.dat", &var_busstopDat_8c1bc42c);

    AsqRequestPvm_8c011ac0("\\SYSTEM", "loading.pvm", &var_loadingResourceGroup_8c1bc3f8.tlist_0x00, 1, 0x80000000);
    AsqRequestDat_8c011182("\\SYSTEM", "load_parts.dat", &var_loadingResourceGroup_8c1bc3f8.tanim_0x04);
    AsqRequestDat_8c011182("\\SYSTEM", "loading.dat", &var_loadingResourceGroup_8c1bc3f8.contents_0x08);

    AsqRequestDat_8c011182("\\SYSTEM", "bus_font.fff", &var_busFont_8c1ba1c8);
    AsqRequestDat_8c011182("\\SYSTEM", "vm_bus.lcd", &var_8c2260ac);
    AsqRequestDat_8c011182("\\SYSTEM", "vm_danger.lcd", &var_8c2260b8);
    AsqRequestDat_8c011182("\\SYSTEM", "now_loading.lcd", &var_8c2260c4);

    AsqRequestPvm_8c011ac0("\\SYSTEM", "fuu.pvm", &var_8c1bc440, 1, 0);
    AsqRequestNj_8c011492("\\SYSTEM", "fuu.njd", &var_8c1bc444, 0);
    AsqRequestNj_8c011492("\\SYSTEM", "fuu.njm", &var_loadedFooNjm_8c1bc448, 0);

    AsqRequestNj_8c011492("\\SD_COMMON","3s_bus_m2.njm", &var_8c1bc410, 0);
    AsqRequestNj_8c011492("\\SD_COMMON","3s_bus_m2.njs", &var_8c1bc414, 0);

    RouteLoadResetPvmReady_8c014322();
    AsqProcessQueues_8c011fe0(&AsqNop_8c011120, 0, 0, 0, &RouteLoadSetPvmReady_8c014330);
    var_gdErr_8c18ad14 = 0;
    gdFsEntryErrFuncAll(&usrGdErrFunc_8c0134d6, (void *) 0);
}

/* TODO: Test */
int GameMain_8c01392e(void) {
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

        TaskExecGroup_8c014b42(var_tasks_8c1ba3c8);
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

    TaskExecGroup_8c014b42(var_tasks_8c1ba3c8);
    return 0;
}

void GameExit_8c0139d4(void) {
  njExitTexture();
  SbExitSystem_8c014a24();
  syBtExit();
}
