/* @unit ReplayMenu */
#include <shinobi.h>
#include "01614c_replay_menu.h"
#include "0100bc_sound.h"
#include "010e90_vibration.h"
#include "011120_asset_queues.h"
#include "012324_input.h"
#include "0129cc_game.h"
#include "013ae8_route.h"
#include "014a9c_tasks.h"
#include "014b8c_backup.h"
#include "015034_text.h"
#include "016108_resgrp_free.h"
#include "016d2c_course_menu.h"
#include "018644_file_select.h"
#include "02171c_tile_stream.h"
#include "028258_signal.h"
#include "0289ac_objects.h"
#include "02a9fc_message_box.h"
#include "02c884_stop.h"
#include "02f320_replay_codec.h"
#include "0193c8_vm_select.h"
#include "1ba1c8_globals.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"

/* ====================
 * Type Declarations
 * ====================
 */

typedef struct {
    const char *name_0x00;
    void (*func_0x04)(void);
    ReplayMenuCourseSel courseSel_0x08;
} ReplayMenuEntry;

/* Task layout for replaySaveTask_8c0167ca. Clearing active_0x08 ends it on
 * the next tick. */
typedef struct {
    TaskAction action;
    void *state;
    int active_0x08;
    int phase_0x0c;
    int field_0x10;
    int field_0x14;
    void *queuedItem_0x18;
    int field_0x1c;
} ReplaySaveTask;

/* Task layout for replayLoadTask_8c0169bc. */
typedef struct {
    TaskAction action;
    void *state;
    int phase_0x08;
    void *field_0x0c;
    int field_0x10;
    int field_0x14;
    void *queuedItem_0x18;
    int field_0x1c;
} ReplayLoadTask;

/* ====================
 * Non-initialized Globals
 * ====================
 */

ReplayMenuCourseSel *var_replayMenuCourseSel_8c1bc824;
ReplayInput var_demoBuffer_8c1bc828[REPLAY_BUFFER_CAPACITY];
STATIC ReplayInput *var_demoCursor_8c225fa8;
STATIC Uint32 var_demoPrevOn_8c225fac;

/* ====================
 * Initialized Globals
 * ====================
 */

/* Scanned until "". */
char *init_replaySaveNames_8c044294[2] = { "BUS_REPLAY", "" };

/* ====================
 * Forward Declarations
 * ====================
 */

STATIC void startCourse_8c0167c0(void);
STATIC void openSaveMenu_8c016636(void);
STATIC void startReplayLoad_8c016b4c(void);

/* ====================
 * Initialized Globals (continued)
 * ====================
 */

/* Scanned until a "" name. */
ReplayMenuEntry init_replayMenuEntries_8c04429c[] = {
    { "SHINJYUKU_EVENT",      MessageBoxRequestAssets_8c02aa36, { 0,  0, 0} },
    { "WANGAN_EVENT",         MessageBoxRequestAssets_8c02aa36, { 1,  0, 0} },
    { "OUME_EVENT",           MessageBoxRequestAssets_8c02aa36, { 2,  0, 0} },
    { "WANGAN_DAY",           startCourse_8c0167c0,                 { 0, 10, 0} },
    { "WANGAN_DAY_AUTO",      startCourse_8c0167c0,                 { 0, 10, 1} },
    { "SHINJYUKU_DAY",        startCourse_8c0167c0,                 { 9, 20, 0} },
    { "SHINJYUKU_EVENING",    startCourse_8c0167c0,                 {12,  0, 0} },
    { "SHINJYUKU_NIGHT",      startCourse_8c0167c0,                 {15,  0, 0} },
    { "SHINJYUKU_DAY_AUTO",   startCourse_8c0167c0,                 { 9,  0, 1} },
    /* label is a copy of entry 6's; by its values the row is SHINJYUKU_EVENING_AUTO */
    { "SHINJYUKU_EVENING",    startCourse_8c0167c0,                 {12,  0, 1} },
    { "SHINJYUKU_NIGHT_AUTO", startCourse_8c0167c0,                 {15,  0, 1} },
    { "OUME_DAY",             startCourse_8c0167c0,                 {18, 10, 0} },
    { "OUME_DAY_AUTO",        startCourse_8c0167c0,                 {18,  0, 1} },
    { "WANGAN_NIGHT",         startCourse_8c0167c0,                 { 6,  0, 0} },
    { "OUME_NIGHT",           startCourse_8c0167c0,                 {24, 17, 0} },
    { "WANGAN_NIGHT_AUTO",    startCourse_8c0167c0,                 { 6,  0, 1} },
    { "OUME_NIGHT_AUTO",      startCourse_8c0167c0,                 {24,  0, 1} },
    { "REPLAY",               startReplayLoad_8c016b4c,             { 0,  0, 0} },
    { "VISUAL_MEMORY",        openSaveMenu_8c016636,                { 0,  0, 0} },
    { "",                     NULL,                                 { 0,  0, 0} },
};

/* ====================
 * Functions
 * ====================
 */

void ReplayMenuUninstallDrive_8c01614c(void)
{
    ObjectsUninstallPedestrianGroups_8c0297da();
    SignalFree_8c0288be();
    StopUninstallGroup_8c02ca96();
    TaskUninstallGroup_8c014ab4(var_tasks_8c1bb448);
    TaskUninstallGroup_8c014ab4(var_tasks_8c1bac28);
    TaskUninstallGroup_8c014ab4(var_tasks_8c1ba808);
    TaskUninstallGroup_8c014ab4(var_tasks_8c1ba5e8);
}

void ReplayMenuFreeSessionAssets_8c016182(void)
{
    int i;

    SndStopAllAdx_8c010c7c();
    sdMidiStopAll();
    if (var_vibport_8c1ba354 != -1) {
        pdVibMxStop(var_vibport_8c1ba354);
    }
    VibClear_8c010fbe();
    ReplayMenuUninstallDrive_8c01614c();
    TaskUninstallGroup_8c014ab4(var_tasks_8c1ba3c8);
    MessageBoxFreeAssets_8c02adee();
    ObjectsFreeAssetRequests_8c029cfe();
    RouteFreePedestrianAssets_8c013ee4();
    RouteFreeAllModels_8c013dae();
    AsqFreeModels_8c0120fe((LoadedModel **)&var_routeModels_8c1bc3ec);
    AsqFreeModels_8c0120fe(&var_segmentModels_8c1bc3f0);
    AsqFreeModels_8c0120fe(&var_trafficModels_8c1bc3f4);
    TileStreamTeardown_8c021724();
    RouteFreeVehicleAssets_8c013b5a();

    if (var_currentCourse_8c1bb868.atariBus_0x04 != (void *)-1) {
        CurrentCourse *course = &var_currentCourse_8c1bb868;

        syFree(course->atariBus_0x04);
        syFree(course->lineBus_0x08);
        /* lineNodes_0x0c is borrowed from the course config, not owned */
        syFree(course->attrBus_0x10);
        syFree(course->attrMark_0x14);
        syFree(course->atariCpu_0x18);
        syFree(course->lineCpu_0x1c);
        syFree(course->attrCpu_0x20);
        syFree(course->macCpu1_0x24);
        syFree(course->atariHum_0x28);
        syFree(course->lineHum_0x2c);
        syFree(course->macHumG0_0x30);
        syFree(course->macHumM0_0x34);
        syFree(course->macSignal_0x38);
        for (i = 0; i < 5; i++) {
            syFree(course->tileLayers_0x3c[i]);
        }
        course->atariBus_0x04 = (void *)-1;
    }
    if (var_demoBuf_8c1ba3c4 != (int *)-1) {
        syFree(var_demoBuf_8c1ba3c4);
        var_demoBuf_8c1ba3c4 = (int *)-1;
    }
    if (var_vmGameBuf_8c1bc454 != (void *)-1) {
        syFree(var_vmGameBuf_8c1bc454);
        var_vmGameBuf_8c1bc454 = (void *)-1;
    }
    ResGroupFreeAll_8c016108();
    FileSelectFreeBuffers_8c0187d0();
    VmSelectFreeAndClear_8c019504();
}

STATIC void saveMenuTask_8c01628c(Task *task, SaveMenuState *state)
{
    int selectedVmu;
    int counter;
    int i;

    selectedVmu = state->selectedVmu_0x04;
    switch (state->state_0x00) {
    case REPLAY_SAVE_MENU_INIT:
        counter = state->frameCounter_0x0c;
        state->frameCounter_0x0c = counter + 1;
        if ((unsigned int)(counter + 1) >= 0xb) {
            if (VmSelectUpdateAllStatus_8c019550(init_replaySaveNames_8c044294, 0x1e) == 0) {
                state->state_0x00 = REPLAY_SAVE_MENU_NO_VMU;
            } else {
                state->state_0x00 = REPLAY_SAVE_MENU_SELECT;
                state->selectedVmu_0x04 = 0;
            }
        }
        break;
    case REPLAY_SAVE_MENU_SELECT:
        VmSelectUpdateAllStatus_8c019550(init_replaySaveNames_8c044294, 0x1e);
        for (i = 0; i < 8; i++) {
            if (var_vmuStatus_8c226048[i] != VMU_STATUS_NOT_CONNECTED) {
                njPrintD(NJM_LOCATION(15, i * 2 + 8), i, 1);
            }
        }
        njPrintC(NJM_LOCATION(15, 24), "NO SAVING");
        if (var_vmuStatus_8c226048[selectedVmu] == VMU_STATUS_NOT_CONNECTED) {
            /* selected slot vanished: snap forward to first present slot */
            for (selectedVmu = 0; var_vmuStatus_8c226048[selectedVmu] == VMU_STATUS_NOT_CONNECTED; selectedVmu++) {
            }
        } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KU) {
            do {                                 /* prev present slot, wrapping */
                selectedVmu--;
                if (var_vmuStatus_8c226048[selectedVmu] != VMU_STATUS_NOT_CONNECTED) break;
            } while (selectedVmu > -1);
            if (selectedVmu < 0) {
                for (selectedVmu = 8; var_vmuStatus_8c226048[selectedVmu] == VMU_STATUS_NOT_CONNECTED; selectedVmu--) {
                }
            }
        } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KD) {
            do {                                 /* next present slot, wrapping */
                selectedVmu++;
                if (var_vmuStatus_8c226048[selectedVmu] != VMU_STATUS_NOT_CONNECTED) break;
            } while (selectedVmu < 9);
            if (selectedVmu > 8) {
                for (selectedVmu = 0; var_vmuStatus_8c226048[selectedVmu] == VMU_STATUS_NOT_CONNECTED; selectedVmu++) {
                }
            }
        } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
            if (selectedVmu == 8) {
                state->state_0x00 = REPLAY_SAVE_MENU_NO_SAVING; /* the "NO SAVING" slot */
            } else {
                state->port_0x08 = selectedVmu;
                state->bupInfo_0x10 = BupGetInfo_8c014bba(selectedVmu);
                if (state->bupInfo_0x10->Work == NULL) {
                    BupMount_8c014c00(selectedVmu);
                }
                state->state_0x00 = REPLAY_SAVE_MENU_CHECK;
            }
        }
        njPrintC(NJM_LOCATION(10, selectedVmu * 2 + 8), "-");
        break;
    case REPLAY_SAVE_MENU_CHECK:
        if (state->bupInfo_0x10->Ready == 0) {
            njPrint(NJM_LOCATION(10, 10), "CHECKING...");
        } else if (state->bupInfo_0x10->IsFormat == 0) {
            state->state_0x00 = REPLAY_SAVE_MENU_UNFORMATTED;
        } else if (buIsExistFile(state->port_0x08, "BUS_REPLAY") != BUD_ERR_FILE_NOT_FOUND
                   || state->bupInfo_0x10->DiskInfo.free_user_blocks > 0x1d) {
            state->state_0x00 = REPLAY_SAVE_MENU_CONFIRM; /* file exists (overwrite) or room for a new one */
        } else {
            state->state_0x00 = REPLAY_SAVE_MENU_NO_SPACE;
        }
        break;
    case REPLAY_SAVE_MENU_CONFIRM:
        if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
            state->state_0x00 = REPLAY_SAVE_MENU_EXIT;
            var_selectedVm_8c1ba34c = state->port_0x08;
        } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TB) {
            state->state_0x00 = REPLAY_SAVE_MENU_SELECT;
            state->selectedVmu_0x04 = 0;
            if (state->bupInfo_0x10->Work != NULL) {
                BupUnmount_8c014c46(state->port_0x08);
            }
        } else {
            njPrint(NJM_LOCATION(10, 10), "%04d/%04d BLOCKS",
                    state->bupInfo_0x10->DiskInfo.free_user_blocks,
                    state->bupInfo_0x10->DiskInfo.total_user_blocks);
        }
        break;
    case REPLAY_SAVE_MENU_NO_SAVING:
        if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
            state->state_0x00 = REPLAY_SAVE_MENU_EXIT;
            var_selectedVm_8c1ba34c = -1;        /* -1: no drive */
        } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TB) {
            state->state_0x00 = REPLAY_SAVE_MENU_SELECT;
            state->selectedVmu_0x04 = 0;
        }
        njPrintC(NJM_LOCATION(10, 10), "NO SAVING OK?");
        break;
    case REPLAY_SAVE_MENU_UNFORMATTED:
        if (var_peripherals_8c1ba35c[0].press & (PDD_DGT_TA | PDD_DGT_TB)) {
            state->state_0x00 = REPLAY_SAVE_MENU_SELECT;
            state->selectedVmu_0x04 = 0;
        }
        njPrintC(NJM_LOCATION(10, 10), "MEMORY_CARD IS UNFORMAT");
        break;
    case REPLAY_SAVE_MENU_NO_SPACE:
        if (var_peripherals_8c1ba35c[0].press & (PDD_DGT_TA | PDD_DGT_TB)) {
            state->state_0x00 = REPLAY_SAVE_MENU_SELECT;
            state->selectedVmu_0x04 = 0;
        }
        njPrintC(NJM_LOCATION(2, 10), "MEMORY_CARD IS NOT ENOUGH FREE AREA");
        break;
    case REPLAY_SAVE_MENU_NO_VMU:
        if (VmSelectUpdateAllStatus_8c019550(init_replaySaveNames_8c044294, 0x1e) == 0) {
            njPrintC(NJM_LOCATION(10, 10), "NO_MEMORY_CARD");
        } else {
            state->state_0x00 = REPLAY_SAVE_MENU_SELECT;
            state->selectedVmu_0x04 = 0;
        }
        break;
    case REPLAY_SAVE_MENU_EXIT:
        TaskUninstall_8c014b66(task);
        ReplayMenuOpen_8c01673a();
        return;               /* no epilogue write -- task is freed */
    }
    state->selectedVmu_0x04 = selectedVmu;
}

STATIC void openSaveMenu_8c016636(void)
{
    Task *task;
    SaveMenuState *state;

    njSetBackColor(0, 0, 0xc060);
    TaskInstall_8c014ae8(var_tasks_8c1ba3c8, saveMenuTask_8c01628c, &task, (void **)&state, 0x14);
    state->state_0x00 = REPLAY_SAVE_MENU_INIT;
    state->frameCounter_0x0c = 0;
}

STATIC void listMenuTask_8c01666a(Task *task)
{
    int cursor;
    int count;

    cursor = task->field_0x08;

    if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
        TaskUninstall_8c014b66(task);
        var_replayMenuCourseSel_8c1bc824 = &init_replayMenuEntries_8c04429c[cursor].courseSel_0x08;
        init_replayMenuEntries_8c04429c[cursor].func_0x04();
        return;
    }

    for (count = 0; init_replayMenuEntries_8c04429c[count].name_0x00[0] != '\0'; count++) {
        njPrintC(NJM_LOCATION(0xc, 8 + count), init_replayMenuEntries_8c04429c[count].name_0x00);
    }

    if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KU) {
        cursor--;
        if (cursor < 0) {
            cursor = count - 1;
        }
    } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KD) {
        cursor++;
        if (cursor >= count) {
            cursor = 0;
        }
    }

    njPrintC(NJM_LOCATION(0xa, 8 + cursor), "-");
    task->field_0x08 = cursor;
    AsqGetRandomA_8c012166();
}

/* Public on purpose: KEEP_PUBLIC in check_private_decls.py. */
void ReplayMenuOpen_8c01673a(void)
{
    Task *task;
    void *state;

    njSetBackColor(0, 0, 0);
    InputInstall_8c0128cc(0);
    TaskInstall_8c014ae8(var_tasks_8c1ba3c8, listMenuTask_8c01666a, &task, &state, 0);
    task->field_0x08 = 0;
}

void ReplayMenuResetDemoCursor_8c016770(void)
{
    var_demoCursor_8c225fa8 = var_demoBuffer_8c1bc828;
    var_demoPrevOn_8c225fac = 0;
}

/* Original bug: records only whether any button is held, which playback
 * restores as the button mask. */
void ReplayMenuDemoRecordTask_8c01677e(Task *task, void *state)
{
    if (var_busState_8c1bb9d0.driveState_0x2b4 > 0 && var_demoCursor_8c225fa8 < &var_demoBuffer_8c1bc828[REPLAY_BUFFER_CAPACITY]) {
        var_demoCursor_8c225fa8->on = var_peripherals_8c1ba35c[0].on != 0;
        var_demoCursor_8c225fa8->x1 = (Sint8)var_peripherals_8c1ba35c[0].x1;
        var_demoCursor_8c225fa8->r = (Uint8)var_peripherals_8c1ba35c[0].r;
        var_demoCursor_8c225fa8->l = (Uint8)var_peripherals_8c1ba35c[0].l;
        var_demoCursor_8c225fa8++;
    }
}

STATIC void startCourse_8c0167c0(void)
{
    var_playMode_8c1bb8d0 = PLAY_MODE_NORMAL;
    GameStartSelectedCourse_8c01328c();
}

STATIC void replaySaveTask_8c0167ca(ReplaySaveTask *task, void *state)
{
    const BACKUPINFO *info;

    if (task->active_0x08 == 0) {
        TaskUninstall_8c014b66((Task *)task);
        CourseMenuReturn_8c017ef2();
        return;
    }

    switch (task->phase_0x0c) {
    case 0:
        info = BupGetInfo_8c014bba(var_selectedVm_8c1ba34c);
        if (info->Connect == 0) {
            task->active_0x08 = 0;
            return;
        }
        if (info->Work == 0) {
            BupMount_8c014c00(var_selectedVm_8c1ba34c);
            task->phase_0x0c = 1;
            return;
        }
        task->phase_0x0c = 2;
        return;
    case 1:
        info = BupGetInfo_8c014bba(var_selectedVm_8c1ba34c);
        if (info->Ready == 0) {
            return;
        }
        task->phase_0x0c = 2;
        return;
    case 2: {
        Uint32 nblock;

        nblock = ((var_replayPackedSize_8c228ba4 + 0x10) >> 9) + 1;   /* 512-byte blocks, header included */
        BupSave_8c014bcc(var_selectedVm_8c1ba34c, "BUS_REPLAY", var_demoBuf_8c1ba3c4, nblock);
        task->phase_0x0c = 3;
        /* fallthrough */
    }
    case 3: {
        Uint32 percent;

        info = BupGetInfo_8c014bba(var_selectedVm_8c1ba34c);
        percent = (info->ProgressCount * 100) / info->ProgressMax;
        njPrint(NJM_LOCATION(4, 8), "NOW SAVING...(%03d%%)", percent);
        if (buStat(var_selectedVm_8c1ba34c) != 0) {
            return;
        }
        syFree(var_demoBuf_8c1ba3c4);
        var_demoBuf_8c1ba3c4 = (int *)-1;
        BupUnmount_8c014c46(var_selectedVm_8c1ba34c);
        TaskUninstall_8c014b66((Task *)task);
        CourseMenuReturn_8c017ef2();
        return;
    }
    default:
        return;
    }
}

/* Saved layout: a 0x10-byte header (packed size, courseId, driveMode, seed),
 * then the LZW-packed inputs. Unreachable in the shipped build. */
STATIC void startReplaySave_8c016924(void)
{
    ReplaySaveTask *task;
    void *state;
    Uint32 recordedBytes;
    Uint32 size;
    int *buf;
    void *dest;

    TaskInstall_8c014ae8(var_tasks_8c1ba3c8, replaySaveTask_8c0167ca, (Task **)&task, &state, 0);

    /* no drive picked, or the recording ran the buffer out */
    if (var_selectedVm_8c1ba34c == -1 ||
        var_demoCursor_8c225fa8 >= &var_demoBuffer_8c1bc828[REPLAY_BUFFER_CAPACITY]) {
        task->active_0x08 = 0;
        return;
    }

    recordedBytes = (Uint32)((char *)var_demoCursor_8c225fa8 - (char *)var_demoBuffer_8c1bc828);
    /* the header is counted into the source length too, so the pack reads 0x10
     * bytes past the recording */
    size = recordedBytes + 0x10;

    buf = syMalloc(size);
    var_demoBuf_8c1ba3c4 = buf;
    dest = (char *)buf + 0x10;

    ReplayCodecInit_8c02f320();
    ReplayCodecPack_8c02f934(var_demoBuffer_8c1bc828, &dest, size);

    buf[0] = var_replayPackedSize_8c228ba4;
    buf[1] = var_currentCourse_8c1bb868.courseId_0x00;
    buf[2] = var_driveMode_8c1bb8c8;
    buf[3] = var_seed_8c157a64;

    task->active_0x08 = 1;
    task->phase_0x0c = 0;
}

STATIC void replayLoadTask_8c0169bc(ReplayLoadTask *task, void *state)
{
    const BACKUPINFO *info;

    if (var_selectedVm_8c1ba34c == -1) {
        TaskUninstall_8c014b66((Task *)task);
        GameStartSelectedCourse_8c01328c();
        return;
    }

    switch (task->phase_0x08) {
    case 0:
        info = BupGetInfo_8c014bba(var_selectedVm_8c1ba34c);
        if (info->Connect == 0) {
            var_selectedVm_8c1ba34c = -1;
            var_playMode_8c1bb8d0 = PLAY_MODE_NORMAL;
            return;
        }
        if (info->Work != 0) {
            task->phase_0x08 = 2;
            return;
        }
        BupMount_8c014c00(var_selectedVm_8c1ba34c);
        task->phase_0x08 = 1;
        return;
    case 1:
        info = BupGetInfo_8c014bba(var_selectedVm_8c1ba34c);
        if (info->Ready == 0) {
            return;
        }
        task->phase_0x08 = 2;
        return;
    case 2: {
        int *buf;

        buf = syMalloc(0x4000);
        var_demoBuf_8c1ba3c4 = buf;
        BupLoad_8c014bc6(var_selectedVm_8c1ba34c, "BUS_REPLAY", buf);
        task->phase_0x08 = 3;
        /* fallthrough */
    }
    case 3: {
        Uint32 percent;
        void *dest;

        info = BupGetInfo_8c014bba(var_selectedVm_8c1ba34c);
        percent = (info->ProgressCount * 100) / info->ProgressMax;
        njPrint(NJM_LOCATION(4, 8), "NOW LOADING...(%03d%%)", percent);
        if (buStat(var_selectedVm_8c1ba34c) != 0) {
            return;
        }
        var_currentCourse_8c1bb868.courseId_0x00 = var_demoBuf_8c1ba3c4[1];
        var_driveMode_8c1bb8c8 = var_demoBuf_8c1ba3c4[2];
        var_seed_8c157a64 = var_demoBuf_8c1ba3c4[3];
        dest = var_demoBuffer_8c1bc828;
        ReplayCodecInit_8c02f320();
        ReplayCodecUnpack_8c02fa14(&var_demoBuf_8c1ba3c4[4], &dest, var_demoBuf_8c1ba3c4[0]);
        syFree(var_demoBuf_8c1ba3c4);
        var_demoBuf_8c1ba3c4 = (int *)-1;
        BupUnmount_8c014c46(var_selectedVm_8c1ba34c);
        TaskUninstall_8c014b66((Task *)task);
        GameStartSelectedCourse_8c01328c();
        return;
    }
    default:
        return;
    }
}

STATIC void startReplayLoad_8c016b4c(void)
{
    ReplayLoadTask *task;
    void *state;

    if (var_selectedVm_8c1ba34c == -1) {
        var_playMode_8c1bb8d0 = PLAY_MODE_NORMAL;
        return;
    }

    var_playMode_8c1bb8d0 = PLAY_MODE_DEMO;
    var_isAttractDemo_8c1bb8d4 = 0;

    TaskInstall_8c014ae8(var_tasks_8c1ba3c8, replayLoadTask_8c0169bc, (Task **)&task, &state, 0);
    task->phase_0x08 = 0;
}

void ReplayMenuDemoPlayTask_8c016bf4()
{
    Uint32 on;

    if ((var_busState_8c1bb9d0.driveState_0x2b4 > 0) && (var_demoCursor_8c225fa8 < &var_demoBuffer_8c1bc828[REPLAY_BUFFER_CAPACITY])) {
        on = var_demoCursor_8c225fa8->on;
        var_peripherals_8c1ba35c[0].on = on;
        var_peripherals_8c1ba35c[0].press = on & (var_demoPrevOn_8c225fac ^ on);
        var_demoPrevOn_8c225fac = var_peripherals_8c1ba35c[0].on;
        var_peripherals_8c1ba35c[0].x1 = var_demoCursor_8c225fa8->x1;
        var_peripherals_8c1ba35c[0].r = var_demoCursor_8c225fa8->r;
        var_peripherals_8c1ba35c[0].l = var_demoCursor_8c225fa8->l;
        var_demoCursor_8c225fa8++;
    }
}
