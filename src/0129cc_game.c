/* @unit Game */
#include <shinobi.h>
#include <njdef.h>
#include "0129cc_game.h"
#include "0100bc_sound.h"
#include "014f54_sprite.h"
#include "015034_text.h"
#include "01fa78_hud.h" /* HudReset_8c02018c, var_8c226434, var_8c226438 */
#include "01614c_replay_menu.h"
#include "028258_signal.h"
#include "0289ac_objects.h" /* ObjectsInitPedestrianGroups_8c0296d6 */
#include "02a9fc_message_box.h"
#include "024b4c_bus_camera.h"
#include "1ba1c8_globals.h"
#include "includes.h"
#include "serial_debug.h"
#include "014a9c_tasks.h"
#include "015ab8_title.h"
#include "scif.h"
#include "011120_asset_queues.h"
#include "013ae8_route.h"
#include "02b464_grading.h" /* GradeReset_8c02c46a */
#include "020214_drive_cue.h" /* DriveCueInit_8c020528 */
#include "014b8c_backup.h"
#include "018644_file_select.h"
#include "01a148_option.h"
#include "01bb48_vm_game.h"
#include "01c980_profile_file.h"
#include "235ca0_nj_buffers.h"
#include "02171c_tile_stream.h"
#include "023310_bus_init.h"
#include "025870_demo.h"
#include "02d968_stop_spawn.h"
#include "021b9c_tile_draw.h" /* TileDrawSpawnTask_8c0222dc */
#include "02c884_stop.h"
#include "016d2c_course_menu.h"
#include "01e27c_practice_menu.h"
#include "022464_render.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#define RENDER_X        256
#define RENDER_Y        512

/* marks from var_markTexlist_8c1bc418 */
#define MARK_BASE          0x74
#define MARK_CONTINUE      0x75
#define MARK_RETIRE        0x7a
#define MARK_CONFIRM_YES   0x76
#define MARK_CONFIRM_NO    0x77

#define MARK_Z_ARROW       -1.09f
#define MARK_Z_BASE        -1.1f

/* the stick deflection that counts as a dpad press */
#define STICK_THRESHOLD    0x40

#define RETIRE_PHASE_IDLE      0
#define RETIRE_PHASE_CONFIRM   1
#define RETIRE_PHASE_FADING    2

#define CONFIRM_YES        0
#define CONFIRM_NO         1

#ifdef SERIAL_DEBUG
char *DEBUG_retirePhaseNames[] = {
    "IDLE",
    "CONFIRM",
    "FADING",
};
#endif

#define CHANGE_RETIRE_PHASE(x)                                                 \
    var_retirePhase_8c18ad08 = x;                                              \
    LOG_DEBUG(("[PAUSE] Retire phase changed: %s\n", DEBUG_retirePhaseNames[x]))

/* Attract demo marks and phases */
#define MARK_DEMO          0x7b
#define MARK_DEMO_BLINK    0x7c

#define DEMO_END_PLAYING   0
#define DEMO_END_SKIPPED   1
#define DEMO_END_TIMED_OUT 2

#define DEMO_END_TIMEOUT   0x708 /* 1800 frames, 30s */

/* =================
 * Type Declarations
 * =================
 */

/* Task layout as seen by attractDemoTask_8c012d5a. */
typedef struct {
    TaskAction action;
    void *state;
    int phase_0x08;      /* DEMO_END_* */
    int counter_0x0c;    /* frames */
    int field_0x10;
    int field_0x14;
    void *queuedItem_0x18;
    int field_0x1c;
} AttractDemoTaskData;

/* ==========================
 * Non-initialized Globals
 * ==========================
 */

NJS_TEXMEMLIST var_tex_8c157af8[TEX_NUM];
STATIC NJS_FOG_TABLE var_fogTable_8c18aaf8;
STATIC NJS_TEXNAME var_renderTexname_8c18acf8[1];

STATIC int var_pauseSettle_8c18ad04;
STATIC int var_retirePhase_8c18ad08;
STATIC int var_confirmChoice_8c18ad0c;
STATIC int var_onRetire_8c18ad10;

STATIC int var_gdErr_8c18ad14;

/* ======================
 * Initialized Globals
 * ======================
 */

/* Offscreen render target for the mirror view. */
NJS_TEXLIST init_renderTexlist_8c03bf44 = {var_renderTexname_8c18acf8, 1};

/* Half-black full-screen quad behind the pause marks. */
STATIC NJS_POLYGON_VTX init_pauseDimQuad_8c03bf4c[4] = {
    {   0.0f,   0.0f, 0.8695652484893799f, ARGB(0x7f, 0x00, 0x00, 0x00) },
    {   0.0f, 480.0f, 0.8695652484893799f, ARGB(0x7f, 0x00, 0x00, 0x00) },
    { 640.0f,   0.0f, 0.8695652484893799f, ARGB(0x7f, 0x00, 0x00, 0x00) },
    { 640.0f, 480.0f, 0.8695652484893799f, ARGB(0x7f, 0x00, 0x00, 0x00) },
};

/* Nothing reads it, and it points at itself. Probably a library stub the
 * linker pulled in; kept so section D keeps its shape. */
STATIC int init_8c03bf8c[7];
STATIC int init_8c03bf8c[] = {
    0,
    (int) &"DUMMY",
    1,
    (int) &init_8c03bf8c,
    (int) &"",
    0,
    0,
};

/* A gdFsReqDrvStat request is in flight. */
STATIC int init_drvStatPending_8c03bfa8 = 0;

/* ==========
 * Functions
 * ==========
 */

/* CONTINUE / RETIRE, RETIRE guarded by a YES/NO confirm. Returns nonzero
 * while closed. */
STATIC int pauseUpdate_8c0129cc(void)
{
    PDS_PERIPHERAL *p;
    int i;

    /* Start opens the menu -- so does losing the controller. A message box on
       screen blocks it. */
    if (!var_pauseActive_8c1bb8cc) {
        if (((var_peripheral_8c1ba358->press & PDD_DGT_ST) != 0 || var_activeCtrlType_8c157a70 == -1)
            && var_messageBoxActive_8c22847c == 0) {
            var_pauseActive_8c1bb8cc = TRUE;
            var_pauseSettle_8c18ad04 = 0;
            var_onRetire_8c18ad10 = 0;
            SndSetPaused_8c0107d2(1);
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            if (var_vibport_8c1ba354 != -1) {
                pdVibMxStop(var_vibport_8c1ba354);
            }
            LOG_DEBUG(("[PAUSE] pauseUpdate_8c0129cc: menu opened\n"));
        }
        return 1;
    }

    /* Dead: nothing in the image ever stores a nonzero here. */
    if (var_pauseSettle_8c18ad04 != 0) {
        if (var_pauseSettle_8c18ad04 == 1) {
            var_pauseSettle_8c18ad04 = 2;
        }
        return 0;
    }

    RenderDrawFrame_8c022560();
    p = var_peripheral_8c1ba358;

    if ((p->press & PDD_DGT_ST) != 0) {
        var_pauseActive_8c1bb8cc = 0;
        SndSetPaused_8c0107d2(0);
        LOG_DEBUG(("[PAUSE] pauseUpdate_8c0129cc: menu closed (Start)\n"));
    } else if (var_onRetire_8c18ad10 == 0) {
        if ((p->press & PDD_DGT_TA) != 0) {
            var_pauseActive_8c1bb8cc = 0;
            SndSetPaused_8c0107d2(0);
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            LOG_DEBUG(("[PAUSE] pauseUpdate_8c0129cc: resumed (A)\n"));
        } else if ((p->press & PDD_DGT_KD) != 0 || p->y1 > STICK_THRESHOLD) {
            var_onRetire_8c18ad10 = 1;
            var_retirePhase_8c18ad08 = RETIRE_PHASE_IDLE;
            var_confirmChoice_8c18ad0c = CONFIRM_NO;
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            LOG_DEBUG(("[PAUSE] pauseUpdate_8c0129cc: cursor -> RETIRE\n"));
        }
        SpriteDraw_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418, MARK_CONTINUE, 0.0f, 0.0f, MARK_Z_ARROW);
    } else {
        switch (var_retirePhase_8c18ad08) {
        case RETIRE_PHASE_IDLE:
            if ((p->press & PDD_DGT_TA) != 0) {
                CHANGE_RETIRE_PHASE(RETIRE_PHASE_CONFIRM);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            } else if ((p->press & PDD_DGT_KU) != 0 || p->y1 < -STICK_THRESHOLD) {
                var_onRetire_8c18ad10 = 0;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                LOG_DEBUG(("[PAUSE] pauseUpdate_8c0129cc: cursor -> CONTINUE\n"));
            }
            SpriteDraw_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418, MARK_RETIRE, 0.0f, 0.0f, MARK_Z_ARROW);
            break;

        case RETIRE_PHASE_CONFIRM:
            if ((p->press & PDD_DGT_TB) != 0) {
                CHANGE_RETIRE_PHASE(RETIRE_PHASE_IDLE);
                SpriteDraw_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418, MARK_RETIRE, 0.0f, 0.0f, MARK_Z_ARROW);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
                break;
            }
            if (var_confirmChoice_8c18ad0c == CONFIRM_YES) {
                /* No mark this frame. */
                if ((p->press & PDD_DGT_TA) != 0) {
                    CHANGE_RETIRE_PHASE(RETIRE_PHASE_FADING);
                    RenderStartFadeOut_8c022b60(10);
                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                    LOG_DEBUG(("[PAUSE] pauseUpdate_8c0129cc: retire committed, fading out\n"));
                    break;
                }
                if ((p->press & PDD_DGT_KR) != 0 || p->x1 > STICK_THRESHOLD) {
                    var_confirmChoice_8c18ad0c = CONFIRM_NO;
                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                    LOG_DEBUG(("[PAUSE] pauseUpdate_8c0129cc: confirm choice -> NO\n"));
                }
                SpriteDraw_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418, MARK_CONFIRM_YES, 0.0f, 0.0f, MARK_Z_ARROW);
                break;
            }
            if ((p->press & PDD_DGT_TA) != 0) {
                CHANGE_RETIRE_PHASE(RETIRE_PHASE_IDLE);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            } else if ((p->press & PDD_DGT_KL) != 0 || p->x1 < -STICK_THRESHOLD) {
                var_confirmChoice_8c18ad0c = CONFIRM_YES;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                LOG_DEBUG(("[PAUSE] pauseUpdate_8c0129cc: confirm choice -> YES\n"));
            }
            SpriteDraw_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418, MARK_CONFIRM_NO, 0.0f, 0.0f, MARK_Z_ARROW);
            break;

        case RETIRE_PHASE_FADING:
            if (var_isFading_8c226568 != 0) {
                SpriteDraw_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418, MARK_CONFIRM_YES, 0.0f, 0.0f, MARK_Z_ARROW);
                njDrawPolygon(init_pauseDimQuad_8c03bf4c, 4, 1);
                return 0;
            }
            var_runSucceeded_8c1bb8dc = 0;
            var_runReportPending_8c1bb8b8 = 0;
            var_runWasPractice_8c1bb8bc = 0;
            ReplayMenuFreeSessionAssets_8c016182();
            if (var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE) {
                var_menuState_8c1bc7a8.selected_0x38 = var_practiceLesson_8c22640c;
                PracticeMenuReturn_8c01f21c();
                LOG_INFO(("[PAUSE] pauseUpdate_8c0129cc: retire complete, leaving drive (practice)\n"));
                return 0;
            }
            for (i = 0; i < 5; i++) {
                var_progress_8c1ba1cc.eventProgressFlags_0x04[i] = var_eventFlagsSnapshot_8c1ba2b8[i];
                var_progress_8c1ba1cc.profileProgressFlags_0x18[i] = var_profileFlagsSnapshot_8c1ba2cc[i];
            }
            CourseMenuReturn_8c017ef2();
            LOG_INFO(("[PAUSE] pauseUpdate_8c0129cc: retire complete, leaving drive (course menu)\n"));
            return 0;
        }
    }

    SpriteDraw_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418, MARK_BASE, 0.0f, 0.0f, MARK_Z_BASE);
    njDrawPolygon(init_pauseDimQuad_8c03bf4c, 4, 1);
    return 0;
}

/* Root task of a normal drive. */
STATIC void pauseTask_8c012cbc()
{
    LOG_TRACE(("[PAUSE] pauseTask_8c012cbc\n"));

    if (var_resetRequested_8c157a78 != 0 && var_vmBusy_8c157a7c == 0
        && var_queuesAreInitialized_8c157a60 == 0) {
        ReplayMenuFreeSessionAssets_8c016182();
        TitleStart_8c015fd6(1);
        LOG_DEBUG(("[PAUSE] pauseTask_8c012cbc: reset requested, returning to title\n"));
        return;
    }

    if (pauseUpdate_8c0129cc() != 0) {
        RenderResetQueues_8c02239c();
        TaskRunGroup_8c014b42(var_tasks_8c1ba5e8);
        RenderDrawFrame_8c022560();
    }
}

/* Root task of VMU replay playback: no menu, Start only freezes the replay. */
STATIC void pauseToggleTask_8c012d06()
{
    LOG_TRACE(("[PAUSE] pauseToggleTask_8c012d06\n"));

    if (var_resetRequested_8c157a78 != 0 && var_vmBusy_8c157a7c == 0
        && var_queuesAreInitialized_8c157a60 == 0) {
        ReplayMenuFreeSessionAssets_8c016182();
        TitleStart_8c015fd6(1);
        LOG_DEBUG(("[PAUSE] pauseToggleTask_8c012d06: reset requested, returning to title\n"));
        return;
    }

    if ((var_peripheral_8c1ba358->press & PDD_DGT_ST) != 0) {
        var_pauseActive_8c1bb8cc ^= 1;
    }

    if (var_pauseActive_8c1bb8cc == 0) {
        RenderResetQueues_8c02239c();
        TaskRunGroup_8c014b42(var_tasks_8c1ba5e8);
    }

    RenderDrawFrameMainOnly_8c022910();
}

/* Root task of the attract demo. Back on the title, Start skips the FortyFive
 * logo; a timeout replays it. */
STATIC void attractDemoTask_8c012d5a(AttractDemoTaskData *task)
{
    LOG_TRACE(("[PAUSE] attractDemoTask_8c012d5a\n"));

    if (var_resetRequested_8c157a78 != 0 && var_vmBusy_8c157a7c == 0
        && var_queuesAreInitialized_8c157a60 == 0) {
        ReplayMenuFreeSessionAssets_8c016182();
        init_adxPlaying_8c03bd80 = 1;
        init_soundOk_8c03bd84 = 0;
        LOG_DEBUG(("[PAUSE] attractDemoTask_8c012d5a: reset requested\n"));
        return;
    }

    SndUpdateAdxVolFade_8c010a40();

    switch (task->phase_0x08) {
    case DEMO_END_PLAYING:
        if ((var_peripherals_8c1ba35c[0].press & PDD_DGT_ST) != 0) {
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            task->phase_0x08 = DEMO_END_SKIPPED;
            LOG_DEBUG(("[PAUSE] attractDemoTask_8c012d5a: demo skipped (Start), fading out\n"));
        } else {
            task->counter_0x0c++;
            if (task->counter_0x0c <= DEMO_END_TIMEOUT) {
                break;
            }
            task->phase_0x08 = DEMO_END_TIMED_OUT;
            LOG_DEBUG(("[PAUSE] attractDemoTask_8c012d5a: demo timed out, fading out\n"));
        }
        SndStartAdxFadeOut_8c010bae(0);
        SndStartAdxFadeOut_8c010bae(1);
        RenderStartFadeOut_8c022b60(0x1e);
        break;

    case DEMO_END_SKIPPED:
    case DEMO_END_TIMED_OUT:
        if (var_isFading_8c226568 != 0) {
            task->counter_0x0c++;
            break;
        }
        if (init_adxPlaying_8c03bd80 != 0) {
            return;
        }
        ReplayMenuFreeSessionAssets_8c016182();
        TitleStart_8c015fd6(task->phase_0x08 == DEMO_END_SKIPPED ? 1 : 0);
        LOG_INFO(("[PAUSE] attractDemoTask_8c012d5a: demo ended, returning to title\n"));
        return;
    }

    RenderResetQueues_8c02239c();
    TaskRunGroup_8c014b42(var_tasks_8c1ba5e8);
    RenderDrawFrameMainOnly_8c022910();
    SpriteDraw_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418, MARK_DEMO, 0.0f, 0.0f, MARK_Z_BASE);
    if ((task->counter_0x0c & 0x18) != 0) {
        SpriteDraw_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418, MARK_DEMO_BLINK, 0.0f, 0.0f, MARK_Z_BASE);
    }
}


void GameSoftResetTask_8c012f44()
{
    if ((var_resetRequested_8c157a78 != 0) && (var_vmBusy_8c157a7c == 0)) {
        SndStopAdx_8c010ca6(0);
        sdMidiStopAll();
        if (var_vibport_8c1ba354 != -1) {
            pdVibMxStop(var_vibport_8c1ba354);
        }
        ReplayMenuFreeSessionAssets_8c016182();
        /* Already on the title: quit instead, by arming the pair
         * GameMain_8c01392e reads as "return -1". */
        if (var_titleActive_8c1bb8c4 != 0) {
            init_adxPlaying_8c03bd80 = 1;
            init_soundOk_8c03bd84 = 0;
        } else {
            TitleStart_8c015fd6(1);
        }
    }
}

/* Drops the view a run starts in (MIRROR_DOOR, arrival overlay up) once the
 * doors shut or the bus pulls away. */
STATIC void runStartViewTask_8c012f9c(Task *task, void* state) {
    Bool omeStart;

    if (var_playMode_8c1bb8d0 != PLAY_MODE_PRACTICE && var_route_8c18ad1c == ROUTE_OME && var_currentSegment_8c228708 == 0) {
        omeStart = TRUE;
    } else {
        omeStart = FALSE;
    }

    if (var_busState_8c1bb9d0.speed_0x27c == 0) {
        switch (task->field_0x08) {
            case 0:
                if (var_busState_8c1bb9d0.doorState_0x3c0 == 0) {
                    var_arrivalOverlayVariant_8c22655c = 0;
                    if (omeStart == FALSE) {
                        var_busState_8c1bb9d0.mirror_0x268 = MIRROR_RIGHT;
                    } else {
                        var_busState_8c1bb9d0.mirror_0x268 = MIRROR_NONE;
                    }

                    task->field_0x08 = 1;
                } else {
                    if (var_busState_8c1bb9d0.mirror_0x268 != MIRROR_DOOR) {
                        var_arrivalOverlayVariant_8c22655c = 0;
                        TaskKill_8c014b66(task);
                    }
                }
                break;

            case 1:
            default:
                break;
        }
    } else {
        var_arrivalOverlayVariant_8c22655c = 0;
        if (omeStart == FALSE) {
            var_busState_8c1bb9d0.signalSide_0x25c = 0;
        }

        var_busState_8c1bb9d0.mirror_0x268 = MIRROR_NONE;

        TaskKill_8c014b66(task);
    }

    njControl3D(0);
}

void GameEnterDrive_8c01306e(void)
{
    Task *created_task;
    void* created_state;

    njInitMatrix(var_matrix_8c2f8ca0, 16, 0);
    njSetBackColor(0,0,0);
    njSetFogColor(ARGB(
        var_viewParams_8c18ad28->fogAlpha_0x0b,
        var_viewParams_8c18ad28->fogRed_0x0a,
        var_viewParams_8c18ad28->fogGreen_0x09,
        var_viewParams_8c18ad28->fogBlue_0x08
    ));

    njGenerateFogTable3(var_fogTable_8c18aaf8, var_viewParams_8c18ad28->fogN_0x0c, var_viewParams_8c18ad28->fogF_0x10);
    njFogEnable();
    njSetCheapShadowMode(0x80);
    njSetFogTable(var_fogTable_8c18aaf8);

    TaskInitGroup_8c014a9c(var_tasks_8c1ba5e8, 0x10);
    TaskInitGroup_8c014a9c(var_tasks_8c1ba808, 0x20);
    TaskInitGroup_8c014a9c(var_tasks_8c1bac28, 0x40);
    TaskInitGroup_8c014a9c(var_tasks_8c1bb448, 0x20);

    njRandomSeed(var_seed_8c157a64);
    AsqSetSeedA_8c012160(var_seed_8c157a64);
    AsqSetSeedB_8c0121a2(var_seed_8c157a64);

    InputSpawnTask_8c0128cc(1);

    if (var_playMode_8c1bb8d0 != PLAY_MODE_DEMO) {
        TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, &pauseTask_8c012cbc, &created_task, &created_state, 0);
        TaskSpawn_8c014ae8(var_tasks_8c1ba5e8, &ReplayMenuDemoRecordTask_8c01677e, &created_task, &created_state, 0);
    } else {
        if (var_isAttractDemo_8c1bb8d4 == 0) {
            TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, &pauseToggleTask_8c012d06, &created_task, &created_state, 0);
        } else {
            TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, &attractDemoTask_8c012d5a, &created_task, &created_state, 0);
            created_task->field_0x08 = 0;
            created_task->field_0x0c = (void*) 0;
        }
        TaskSpawn_8c014ae8(var_tasks_8c1ba5e8, &ReplayMenuDemoPlayTask_8c016bf4, &created_task, &created_state, 0);
        DemoStartTour_8c025af4();
    }

    var_pauseActive_8c1bb8cc = 0;
    var_messageBoxActive_8c22847c = 0;

    BusInitStart_8c023610();
    SignalInit_8c02845a();

    if (var_playMode_8c1bb8d0 != PLAY_MODE_DEMO) {
        ObjectsInitBlinkers_8c029920();
    }

    ObjectsInitPedestrianGroups_8c0296d6();
    TrafficInit_8c02769e();
    TileDrawSpawnTask_8c0222dc();
    ObjectsSpawnTasks_8c02a6ac();
    GradeReset_8c02c46a();
    HudReset_8c02018c();
    StopSpawnInit_8c02d968();
    DriveCueInit_8c020528();
    TaskSpawn_8c014ae8(var_tasks_8c1ba5e8, &runStartViewTask_8c012f9c, &created_task, &created_state, 0);
    created_task->field_0x08 = 0;
    RenderStartRunFade_8c0228a2();
}

void GameStartSelectedCourse_8c01328c() {
    Task *created_task;
    void* created_state;
  
    if (var_playMode_8c1bb8d0 == PLAY_MODE_NORMAL) {
        var_currentCourse_8c1bb868.courseId_0x00 = var_replayMenuCourseSel_8c1bc824->courseId_0x00;
        var_startStopIndex_8c228704 = var_replayMenuCourseSel_8c1bc824->startStopIndex_0x04;
        var_driveMode_8c1bb8c8 = var_replayMenuCourseSel_8c1bc824->driveMode_0x08;
        var_seed_8c157a64 = AsqGetRandomA_8c012166();
    } else if ((var_playMode_8c1bb8d0 == PLAY_MODE_DEMO) && (var_isAttractDemo_8c1bb8d4 != 0)) {
        var_demoShotId_8c227dd4 = init_demoFirstShot_8c0460b0[var_currentCourse_8c1bb868.courseId_0x00 - 0x26];
        FileSelectResetNewGame_8c01895e();
    } else {
        var_demoShotId_8c227dd4 = 0;
    }

    njRandomSeed(var_seed_8c157a64);
    AsqSetSeedA_8c012160(var_seed_8c157a64);
    AsqSetSeedB_8c0121a2(var_seed_8c157a64);
    AsqApplyButtonConfig_8c0121e8();
    var_savedCameraMode_8c227da0 = var_progress_8c1ba1cc.defaultView_0xc6;
    var_savedCameraCueState_8c227da8 = 0;

    RouteSpawnLoadTask_8c0144fc();
}

void GameStartCourse_8c013310(int courseId) {
    Task *created_task;
    void* created_state;
  
    if (var_playMode_8c1bb8d0 != PLAY_MODE_DEMO) {
        var_currentCourse_8c1bb868.courseId_0x00 = courseId;
        var_startStopIndex_8c228704 = 0;
        var_driveMode_8c1bb8c8 = var_progress_8c1ba1cc.driveMode_0xc5;
        var_seed_8c157a64 = AsqGetRandomA_8c012166();
    } else if (var_playMode_8c1bb8d0 == PLAY_MODE_DEMO && var_isAttractDemo_8c1bb8d4 != 0) {
        var_demoShotId_8c227dd4 = init_demoFirstShot_8c0460b0[var_currentCourse_8c1bb868.courseId_0x00 - 0x26];
    } else {
        var_demoShotId_8c227dd4 = 0;
    }

    njRandomSeed(var_seed_8c157a64);
    AsqSetSeedA_8c012160(var_seed_8c157a64);
    AsqSetSeedB_8c0121a2(var_seed_8c157a64);
    AsqApplyButtonConfig_8c0121e8();
    var_savedCameraMode_8c227da0 = var_progress_8c1ba1cc.defaultView_0xc6;
    var_savedCameraCueState_8c227da8 = 0;

    RouteSpawnLoadTask_8c0144fc();
}

/* Waits for GameInit_8c0134ec's asset load, loads the sound driver, then
 * hands over to the title. */
STATIC void bootTask_8c013388(Task *task, void *state) {
    switch (task->field_0x08) {
        case 0: {
            Bool b = RouteGetLatch_8c01432a();
            if (b) {
                task->field_0x08++;
                var_fuuLastFrame_8c1bc450 = (Float) var_fuuNjm_8c1bc448->nbFrame - 1;

                AsqResetQueues_8c011f6c();
                AsqRequestDat_8c011182("\\SOUND", "manatee.drv", &var_sndDrvData_8c0fcd48);
                AsqRequestDat_8c011182("\\SOUND", "bus.mlt", &var_sndBankData_8c0fcd4c);
                RouteClearLatch_8c014322();
                AsqProcessQueues_8c011fe0(&AsqNop_8c011120, 0, 0, 0, &RouteSetLatch_8c014330);
            }
            break;
        }
        case 1: {
            if (RouteGetLatch_8c01432a() != 0) {
                AsqFreeQueues_8c011f7e();
                TaskKill_8c014b66(task);
                SndInit_8c010e18("\\SOUND");
                var_lcdAnimActive_8c2260a8 = 1;
#ifdef DEBUG_MENU
                /* The debug menu, compiled out of retail. Course entries
                 * work; *_EVENT / REPLAY / VISUAL_MEMORY hang on the loading
                 * screen. */
                ReplayMenuOpen_8c01673a();
#else
                TitleStart_8c015fd6(0);
#endif
            }
            break;
        }
        default:
            break;
    }
}

STATIC void usrGdErrFunc_8c0134d6(void *obj, Sint32 errcode) {
  if (errcode == GDD_ERR_TRAYOPEND || errcode == GDD_ERR_UNITATTENT) {
    var_gdErr_8c18ad14 = 1;
  }
}

void GameInit_8c0134ec() {
    NJS_TEXINFO info;
    Task *created_task;
    void *created_state;

#ifdef SERIAL_DEBUG
    scif_init(57600);
#endif

    njSetTextureMemorySize(0x100000);

    if (syCblCheckCable() == SYE_CBL_CABLE_VGA) {
        SbInitSystem_8c0149b0(NJD_RESOLUTION_VGA, NJD_FRAMEBUFFER_MODE_RGB565, 2);
    } else {
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

#ifdef DEBUG_MENU
    /* Retail never inits njPrint. */
    njInitPrint(NULL, 0, 0);
#endif

    var_soundMode_8c226070 = SndGetSoundMode_8c010924();
    if (var_soundMode_8c226070 >= 0) {
        SndSetSoundMode_8c0108c0(var_soundMode_8c226070);
    } else {
        SndSetSoundMode_8c0108c0(SYD_CFG_STEREO);
    }

    VibClear_8c010fbe();
    BupInit_8c014b8c();

    njSetTextureInfo(&info, (Uint16 *) var_texbuf_8c277ca0, NJD_TEXFMT_STRIDE | NJD_TEXFMT_RGB_565, RENDER_X, RENDER_Y);

    njSetTextureName(&var_renderTexname_8c18acf8[0], &info, 999, NJD_TEXATTR_TYPE_MEMORY|NJD_TEXATTR_GLOBALINDEX);
    njSetRenderWidth(256);
    njLoadTexture(&init_renderTexlist_8c03bf44);

    TaskInitGroup_8c014a9c(var_tasks_8c1ba3c8, 0x10);
    TaskInitGroup_8c014a9c(var_tasks_8c1ba5e8, 0x10);
    TaskInitGroup_8c014a9c(var_tasks_8c1ba808, 0x20);
    TaskInitGroup_8c014a9c(var_tasks_8c1bac28, 0x40);
    TaskInitGroup_8c014a9c(var_tasks_8c1bb448, 0x20);

    var_currentCourse_8c1bb868.atariBus_0x04 = (void *) -1;

    RouteClearModelSlots_8c013bbc(var_routeModelSlots_8c1bbddc, 0x20);
    RouteClearModelSlots_8c013bbc(var_pedestrianAssets_8c1bbfdc, 0x41);

    var_routeModels_8c1bc3ec = (void *) -1;
    var_segmentModels_8c1bc3f0 = (LoadedModel *) -1;
    var_trafficModels_8c1bc3f4 = (LoadedModel *) -1;

    TileStreamClearSlots_8c02171c();
    ObjectsClearAssetRequestTable_8c029acc();
    MessageBoxClearAssets_8c02aa28();

    var_messageTextBoxA_8c1bc404 = (void *) -1;
    var_8c226434 = (void *) -1;
    var_8c226438 = (void *) -1;
    var_pedGroupCount_8c228234 = -1;
    var_trafficSignalTasks_8c227e20 = (void *) -1;
    var_trafficSignalFrames_8c227e24 = (void *) -1;
    var_stopTaskGroup_8c2288f8 = (void *) -1;
    var_interiorTexlist_8c1bc438 = (NJS_TEXLIST *) -1;
    var_menuState_8c1bc7a8.resourceGroupA_0x00.tlist_0x00 = (void*) -1;
    var_menuState_8c1bc7a8.resourceGroupB_0x0c.tlist_0x00 = (void*) -1;
    var_resourceGroup_8c2263a8.tlist_0x00 = (void *) -1;
    var_saveBuf_8c1ba2e0 = (void *) -1;
    var_backupFileImageBuf_8c1ba348 = (void *) -1;
    var_vmuIconFileBuf_8c1ba344 = (void *) -1;
    var_currentSysResGroupInfo_8c225fb0 = (void *) -1;
    var_demoBuf_8c1ba3c4 = (void *) -1;
    var_vmGameBuf_8c1bc454 = (void *) -1;
    var_selectedVm_8c1ba34c = -1;

    var_titleActive_8c1bb8c4 = 0;

    /* Past the end of init_demos_8c044154, so the first
     * TxtStartAttractDemo_8c0159ac wraps to demo 0. */
    var_demoIndex_8c1bb8d8 = 100;
    var_loadScreenActive_8c157a6c = 0;

    VmGameResetLcdAnims_8c01c8dc();
    FileSelectResetOptionDefaults_8c0189d2();
    njSetBorderColor(0);
    VmGameSetLcdAnim_8c01c8fc(3);
    VmGameUpdateLcd_8c01c910();

    TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, &bootTask_8c013388, &created_task, &created_state, 0);
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
    AsqRequestDat_8c011182("\\SYSTEM", "vm_bus.lcd", &var_lcdAnimBus_8c2260ac);
    AsqRequestDat_8c011182("\\SYSTEM", "vm_danger.lcd", &var_lcdAnimDanger_8c2260b8);
    AsqRequestDat_8c011182("\\SYSTEM", "now_loading.lcd", &var_lcdAnimLoading_8c2260c4);

    AsqRequestPvm_8c011ac0("\\SYSTEM", "fuu.pvm", &var_fuuTexlist_8c1bc440, 1, 0);
    AsqRequestNj_8c011492("\\SYSTEM", "fuu.njd", &var_fuuNj_8c1bc444, 0);
    AsqRequestNj_8c011492("\\SYSTEM", "fuu.njm", &var_fuuNjm_8c1bc448, 0);

    AsqRequestNj_8c011492("\\SD_COMMON","3s_bus_m2.njm", &var_busDoorMotion_8c1bc410, 0);
    AsqRequestNj_8c011492("\\SD_COMMON","3s_bus_m2.njs", &var_busDoorShape_8c1bc414, 0);

    RouteClearLatch_8c014322();
    AsqProcessQueues_8c011fe0(&AsqNop_8c011120, 0, 0, 0, &RouteSetLatch_8c014330);
    var_gdErr_8c18ad14 = 0;
    gdFsEntryErrFuncAll(&usrGdErrFunc_8c0134d6, (void *) 0);
}

int GameMain_8c01392e(void) {
    GDFS gdfs;
    Sint32 stat;

    if (init_adxPlaying_8c03bd80 != 0) {
        if (init_soundOk_8c03bd84 == 0) {
            if (var_vibport_8c1ba354 != -1) {
                pdVibMxStop(var_vibport_8c1ba354);
            }

            return -1;
        }

        TaskRunGroup_8c014b42(var_tasks_8c1ba3c8);
        return 0;
    }

    if (var_queuesAreInitialized_8c157a60 == 0) {
        if (init_drvStatPending_8c03bfa8 == 0) {
            if (!gdFsReqDrvStat()) {
                init_drvStatPending_8c03bfa8 = 1;
            }
        } else {
            gdfs = gdFsGetSysHn();
            stat = gdFsGetStat(gdfs);
            if (stat != GDD_STAT_BUSY) {
                init_drvStatPending_8c03bfa8 = 0;
            }
        }
    }

    stat = gdFsGetDrvStat();
    if (stat == GDD_DRVSTAT_OPEN) {
        if (var_vibport_8c1ba354 != -1) {
            pdVibMxStop(var_vibport_8c1ba354);
        }

        return -1;
    }

    stat = gdFsGetDrvStat();
    if ((stat == GDD_DRVSTAT_OPEN) || (stat == GDD_DRVSTAT_BUSY)) {
        gdFsReqDrvStat();
    }

    if (var_gdErr_8c18ad14 != 0) {
        if (var_vibport_8c1ba354 != -1) {
            pdVibMxStop(var_vibport_8c1ba354);
        }

        return -1;
    };

    TaskRunGroup_8c014b42(var_tasks_8c1ba3c8);
    return 0;
}

void GameExit_8c0139d4(void) {
  njExitTexture();
  SbExitSystem_8c014a24();
  syBtExit();
}
