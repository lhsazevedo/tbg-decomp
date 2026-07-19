/* @unit DebugMenu */
#include <shinobi.h>
#include "01614c_debug_menu.h"
#include "0100bc_sound.h"
#include "010e90.h"
#include "011120_asset_queues.h"
#include "012504_input.h"
#include "012f44_game.h"
#include "013ae8_route_load.h"
#include "014a9c_tasks.h"
#include "014b8c_backup.h"
#include "014f54_text.h"
#include "016108.h"
#include "016d2c_course_menu.h"
#include "018784.h"
#include "02171c.h"
#include "028258.h"
#include "02c884.h"
#include "02f320.h"
#include "0193c8_vm_menu.h"
#include "sectionB.h"
#include "serial_debug.h"

/* ====================
 * Type Declarations
 * ====================
 */

/* one row of the debug menu listMenuTask_8c01666a lists/selects from */
typedef struct {
    const char *name_0x00;
    void (*func_0x04)(void);
    DebugMenuCourseSel courseSel_0x08;
} DebugMenuEntry;

/* ====================
 * Initialized Globals
 * ====================
 */

char const_replayFileName_8c036160[] = "BUS_REPLAY";

/* saveNames for VmMenuUpdateVmusStatus; scanned until a pointer to "" (terminator). */
char *init_replaySaveNames_8c044294[2] = { const_replayFileName_8c036160, "" };

/* ====================
 * Forward Declarations
 * ====================
 */

void DebugMenuOpen_8c01673a(void);

/* referenced by init_debugMenuEntries_8c04429c below; defined further down this file */
STATIC void FUN_8c0167c0(void);
STATIC void openSaveMenu_8c016636(void);
STATIC void startReplayLoad_8c016b4c(void);

/* ====================
 * Initialized Globals (continued: needs the forward decls above)
 * ====================
 */

/* debug menu listMenuTask_8c01666a lists/selects from; scanned until a "" (terminator) name */
DebugMenuEntry init_debugMenuEntries_8c04429c[] = {
    { "SHINJYUKU_EVENT",       FUN_8c02aa36,             {0,  0, 0} },
    { "WANGAN_EVENT",          FUN_8c02aa36,             {1,  0, 0} },
    { "OUME_EVENT",            FUN_8c02aa36,             {2,  0, 0} },
    { "WANGAN_DAY",            FUN_8c0167c0,             {0, 10, 0} },
    { "WANGAN_DAY_AUTO",       FUN_8c0167c0,             {0, 10, 1} },
    { "SHINJYUKU_DAY",         FUN_8c0167c0,             {9, 20, 0} },
    { "SHINJYUKU_EVENING",     FUN_8c0167c0,             {12, 0, 0} },
    { "SHINJYUKU_NIGHT",       FUN_8c0167c0,             {15, 0, 0} },
    { "SHINJYUKU_DAY_AUTO",    FUN_8c0167c0,             {9,  0, 1} },
    { "SHINJYUKU_EVENING",     FUN_8c0167c0,             {12, 0, 1} },
    { "SHINJYUKU_NIGHT_AUTO",  FUN_8c0167c0,             {15, 0, 1} },
    { "OUME_DAY",              FUN_8c0167c0,             {18, 10, 0} },
    { "OUME_DAY_AUTO",         FUN_8c0167c0,             {18, 0, 1} },
    { "WANGAN_NIGHT",          FUN_8c0167c0,             {6,  0, 0} },
    { "OUME_NIGHT",            FUN_8c0167c0,             {24, 17, 0} },
    { "WANGAN_NIGHT_AUTO",     FUN_8c0167c0,             {6,  0, 1} },
    { "OUME_NIGHT_AUTO",       FUN_8c0167c0,             {24, 0, 1} },
    { "REPLAY",                startReplayLoad_8c016b4c, {0,  0, 0} },
    { "VISUAL_MEMORY",         openSaveMenu_8c016636,    {0,  0, 0} },
    { "",                      NULL,                     {0,  0, 0} },
};

/* ====================
 * Functions
 * ====================
 */

void FUN_8c01614c(void)
{
    FUN_8c0297da();
    FUN_8c0288be();
    FUN_8c02ca96();
    TaskFreeGroup_8c014ab4(var_tasks_8c1bb448);
    TaskFreeGroup_8c014ab4(var_tasks_8c1bac28);
    TaskFreeGroup_8c014ab4(var_tasks_8c1ba808);
    TaskFreeGroup_8c014ab4(var_tasks_8c1ba5e8);
}

void FUN_8c016182(void)
{
    int i;

    FUN_8c010c7c();
    sdMidiStopAll();
    if (var_vibport_8c1ba354 != -1) {
        pdVibMxStop(var_vibport_8c1ba354);
    }
    VibClear_8c010fbe();
    FUN_8c01614c();
    TaskFreeGroup_8c014ab4(var_tasks_8c1ba3c8);
    FUN_8c02adee();
    FUN_8c029cfe();
    RouteLoadFreePedestrianAssets_8c013ee4();
    RouteLoadFreeAllRouteModels_8c013dae();
    AsqFreeModels_8c0120fe((LoadedModel **)&var_routeModels_8c1bc3ec);
    AsqFreeModels_8c0120fe(&var_segmentModels_8c1bc3f0);
    AsqFreeModels_8c0120fe(&var_trafficModels_8c1bc3f4);
    FUN_8c021724();
    RouteLoadFreeVehicleAssets_8c013b5a();

    if (var_currentCourse_8c1bb868.slots_0x04[0] != (void *)-1) {
        for (i = 0; i < 19; i++) {
            if (i != 2) { /* slot 2 is not owned here */
                syFree(var_currentCourse_8c1bb868.slots_0x04[i]);
            }
        }
        var_currentCourse_8c1bb868.slots_0x04[0] = (void *)-1;
    }
    if (var_demoBuf_8c1ba3c4 != (int *)-1) {
        syFree(var_demoBuf_8c1ba3c4);
        var_demoBuf_8c1ba3c4 = (int *)-1;
    }
    if (var_8c1bc454 != (void *)-1) {
        syFree(var_8c1bc454);
        var_8c1bc454 = (void *)-1;
    }
    RgFreeResourceGroups_8c016108();
    FUN_8c0187d0();
    VmMenuFreeAndClear_8c019504();
}

STATIC void saveMenuTask_8c01628c(Task *task, SaveMenuState *state)
{
    int selectedVmu;
    int counter;
    int i;

    selectedVmu = state->selectedVmu_0x04;
    switch (state->state_0x00) {
    case DEBUG_SAVE_MENU_INIT:
        counter = state->frameCounter_0x0c;
        state->frameCounter_0x0c = counter + 1;
        if ((unsigned int)(counter + 1) >= 0xb) {
            if (VmMenuUpdateVmusStatus_8c019550(init_replaySaveNames_8c044294, 0x1e) == 0) {
                state->state_0x00 = DEBUG_SAVE_MENU_NO_VMU;
            } else {
                state->state_0x00 = DEBUG_SAVE_MENU_SELECT;
                state->selectedVmu_0x04 = 0;
            }
        }
        break;
    case DEBUG_SAVE_MENU_SELECT:
        VmMenuUpdateVmusStatus_8c019550(init_replaySaveNames_8c044294, 0x1e);
        for (i = 0; i < 8; i++) {
            if (var_vmuStatus_8c226048[i] != 0) {
                njPrintD(NJM_LOCATION(15, i * 2 + 8), i, 1);
            }
        }
        njPrintC(NJM_LOCATION(15, 24), "NO SAVING");
        if (var_vmuStatus_8c226048[selectedVmu] == 0) {
            /* selected slot vanished: snap forward to first present slot */
            for (selectedVmu = 0; var_vmuStatus_8c226048[selectedVmu] == 0; selectedVmu++) {
            }
        } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KU) {
            do {                                 /* prev present slot, wrapping */
                selectedVmu--;
                if (var_vmuStatus_8c226048[selectedVmu] != 0) break;
            } while (selectedVmu > -1);
            if (selectedVmu < 0) {
                for (selectedVmu = 8; var_vmuStatus_8c226048[selectedVmu] == 0; selectedVmu--) {
                }
            }
        } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KD) {
            do {                                 /* next present slot, wrapping */
                selectedVmu++;
                if (var_vmuStatus_8c226048[selectedVmu] != 0) break;
            } while (selectedVmu < 9);
            if (selectedVmu > 8) {
                for (selectedVmu = 0; var_vmuStatus_8c226048[selectedVmu] == 0; selectedVmu++) {
                }
            }
        } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
            if (selectedVmu == 8) {
                state->state_0x00 = DEBUG_SAVE_MENU_NO_SAVING; /* the "NO SAVING" slot */
            } else {
                state->port_0x08 = selectedVmu;
                state->bupInfo_0x10 = BupGetInfo_8c014bba(selectedVmu);
                if (state->bupInfo_0x10->Work == NULL) {
                    BupMount_8c014c00(selectedVmu);
                }
                state->state_0x00 = DEBUG_SAVE_MENU_CHECK;
            }
        }
        njPrintC(NJM_LOCATION(10, selectedVmu * 2 + 8), "-");
        break;
    case DEBUG_SAVE_MENU_CHECK:
        if (state->bupInfo_0x10->Ready == 0) {
            njPrint(NJM_LOCATION(10, 10), "CHECKING...");
        } else if (state->bupInfo_0x10->IsFormat == 0) {
            state->state_0x00 = DEBUG_SAVE_MENU_UNFORMATTED;
        } else if (buIsExistFile(state->port_0x08, "BUS_REPLAY") != BUD_ERR_FILE_NOT_FOUND
                   || state->bupInfo_0x10->DiskInfo.free_user_blocks > 0x1d) {
            state->state_0x00 = DEBUG_SAVE_MENU_CONFIRM; /* file exists (overwrite) or room for a new one */
        } else {
            state->state_0x00 = DEBUG_SAVE_MENU_NO_SPACE;
        }
        break;
    case DEBUG_SAVE_MENU_CONFIRM:
        if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
            state->state_0x00 = DEBUG_SAVE_MENU_EXIT;  /* A: exit the save flow */
            var_selectedVm_8c1ba34c = state->port_0x08;
        } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TB) {
            state->state_0x00 = DEBUG_SAVE_MENU_SELECT; /* B: back to VMU selection */
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
    case DEBUG_SAVE_MENU_NO_SAVING:
        if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
            state->state_0x00 = DEBUG_SAVE_MENU_EXIT;  /* A: confirm no save, exit */
            var_selectedVm_8c1ba34c = -1;        /* -1: nothing saved */
        } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TB) {
            state->state_0x00 = DEBUG_SAVE_MENU_SELECT; /* B: back to VMU selection */
            state->selectedVmu_0x04 = 0;
        }
        njPrintC(NJM_LOCATION(10, 10), "NO SAVING OK?");
        break;
    case DEBUG_SAVE_MENU_UNFORMATTED:
        if (var_peripherals_8c1ba35c[0].press & (PDD_DGT_TA | PDD_DGT_TB)) {
            state->state_0x00 = DEBUG_SAVE_MENU_SELECT; /* A or B: back to VMU selection */
            state->selectedVmu_0x04 = 0;
        }
        njPrintC(NJM_LOCATION(10, 10), "MEMORY_CARD IS UNFORMAT");
        break;
    case DEBUG_SAVE_MENU_NO_SPACE:
        if (var_peripherals_8c1ba35c[0].press & (PDD_DGT_TA | PDD_DGT_TB)) {
            state->state_0x00 = DEBUG_SAVE_MENU_SELECT; /* A or B: back to VMU selection */
            state->selectedVmu_0x04 = 0;
        }
        njPrintC(NJM_LOCATION(2, 10), "MEMORY_CARD IS NOT ENOUGH FREE AREA");
        break;
    case DEBUG_SAVE_MENU_NO_VMU:
        if (VmMenuUpdateVmusStatus_8c019550(init_replaySaveNames_8c044294, 0x1e) == 0) {
            njPrintC(NJM_LOCATION(10, 10), "NO_MEMORY_CARD");
        } else {
            state->state_0x00 = DEBUG_SAVE_MENU_SELECT;
            state->selectedVmu_0x04 = 0;
        }
        break;
    case DEBUG_SAVE_MENU_EXIT:
        TaskFree_8c014b66(task);
        DebugMenuOpen_8c01673a();
        return;               /* no epilogue write -- task is freed */
    }
    state->selectedVmu_0x04 = selectedVmu;
}

STATIC void openSaveMenu_8c016636(void)
{
    Task *task;
    SaveMenuState *state;

    njSetBackColor(0, 0, 0xc060);
    TaskPush_8c014ae8(var_tasks_8c1ba3c8, saveMenuTask_8c01628c, &task, (void **)&state, 0x14);
    state->state_0x00 = DEBUG_SAVE_MENU_INIT;
    state->frameCounter_0x0c = 0;
}

STATIC void listMenuTask_8c01666a(Task *task)
{
    int cursor;
    int count;

    cursor = task->field_0x08;

    if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
        TaskFree_8c014b66(task);
        var_debugMenuCourseSel_8c1bc824 = &init_debugMenuEntries_8c04429c[cursor].courseSel_0x08;
        init_debugMenuEntries_8c04429c[cursor].func_0x04();
        return;
    }

    for (count = 0; init_debugMenuEntries_8c04429c[count].name_0x00[0] != '\0'; count++) {
        njPrintC(NJM_LOCATION(0xc, 8 + count), init_debugMenuEntries_8c04429c[count].name_0x00);
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

/* Kept public though only saveMenuTask_8c01628c calls it today: a debug-menu entry
 * point, not a private helper. Do NOT make STATIC -- see KEEP_PUBLIC in
 * scripts/check_private_decls.py. */
void DebugMenuOpen_8c01673a(void)
{
    Task *task;
    void *state;

    njSetBackColor(0, 0, 0);
    InputPushTask_8c0128cc(0);
    TaskPush_8c014ae8(var_tasks_8c1ba3c8, listMenuTask_8c01666a, &task, &state, 0);
    task->field_0x08 = 0;
}

void FUN_8c016770(void)
{
    var_demoCursor_8c225fa8 = var_demoBuffer_8c1bc828;
    var_demoPrevOn_8c225fac = 0;
}

/* record counterpart of DemoInputTask_8c016bf4's playback */
void DebugMenuDemoRecordTask_8c01677e(Task *task, void *state)
{
    if (var_8c1bbc84 > 0 && var_demoCursor_8c225fa8 < &var_demoBuffer_8c1bc828[REPLAY_BUFFER_CAPACITY]) {
        var_demoCursor_8c225fa8->on = var_peripherals_8c1ba35c[0].on != 0;
        var_demoCursor_8c225fa8->x1 = (Sint8)var_peripherals_8c1ba35c[0].x1;
        var_demoCursor_8c225fa8->r = (Uint8)var_peripherals_8c1ba35c[0].r;
        var_demoCursor_8c225fa8->l = (Uint8)var_peripherals_8c1ba35c[0].l;
        var_demoCursor_8c225fa8++;
    }
}

STATIC void FUN_8c0167c0(void)
{
    var_playMode_8c1bb8d0 = PLAY_MODE_NORMAL;
    FUN_8c01328c();
}

/* replay-save task installed by startReplaySave_8c016924; state machine driving the BupSave of the
 * recorded demo buffer for the selected VMU. */
STATIC void replaySaveTask_8c0167ca(Task *task, void *state)
{
    const BACKUPINFO *info;
    int taskState;

    if (task->field_0x08 == 0) {
        TaskFree_8c014b66(task);
        CourseMenuFUN_8c017ef2();
        return;
    }

    taskState = (int)task->field_0x0c;
    switch (taskState) {
    case 0:
        info = BupGetInfo_8c014bba(var_selectedVm_8c1ba34c);
        if (info->Connect == 0) {
            task->field_0x08 = 0;
            return;
        }
        if (info->Work == 0) {
            BupMount_8c014c00(var_selectedVm_8c1ba34c);
            task->field_0x0c = (void *)1;
            return;
        }
        task->field_0x0c = (void *)2;
        return;
    case 1:
        info = BupGetInfo_8c014bba(var_selectedVm_8c1ba34c);
        if (info->Ready == 0) {
            return;
        }
        task->field_0x0c = (void *)2;
        return;
    case 2: {
        Uint32 nblock;

        nblock = ((var_8c228ba4 + 0x10) >> 9) + 1;
        BupSave_8c014bcc(var_selectedVm_8c1ba34c, "BUS_REPLAY", var_demoBuf_8c1ba3c4, nblock);
        task->field_0x0c = (void *)3;
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
        TaskFree_8c014b66(task);
        CourseMenuFUN_8c017ef2();
        return;
    }
    default:
        return;
    }
}

/* installs replaySaveTask_8c0167ca; packs the recorded demo buffer (course/seed/inputs) into a
 * freshly malloc'd blob for BupSave, or skips saving if there's nothing to save. */
STATIC void startReplaySave_8c016924(void)
{
    Task *task;
    void *state;
    Uint32 recordedBytes;
    Uint32 size;
    int *buf;
    void *dest;

    TaskPush_8c014ae8(var_tasks_8c1ba3c8, replaySaveTask_8c0167ca, &task, &state, 0);

    if (var_selectedVm_8c1ba34c == -1 ||
        var_demoCursor_8c225fa8 >= &var_demoBuffer_8c1bc828[REPLAY_BUFFER_CAPACITY]) {
        task->field_0x08 = 0;
        return;
    }

    recordedBytes = (Uint32)((char *)var_demoCursor_8c225fa8 - (char *)var_demoBuffer_8c1bc828);
    size = recordedBytes + 0x10;

    buf = syMalloc(size);
    var_demoBuf_8c1ba3c4 = buf;
    dest = (char *)buf + 0x10;

    FUN_8c02f320();
    FUN_8c02f934(var_demoBuffer_8c1bc828, &dest, size);

    buf[0] = var_8c228ba4;
    buf[1] = var_currentCourse_8c1bb868.courseId_0x00;
    buf[2] = var_inputMapSel_8c1bb8c8;
    buf[3] = var_seed_8c157a64;

    task->field_0x08 = 1;
    task->field_0x0c = 0;
}

/* replay-load task installed by startReplayLoad_8c016b4c; state machine driving the BupLoad of a
 * recorded demo buffer from the selected VMU, then unpacking it for playback. */
STATIC void replayLoadTask_8c0169bc(Task *task, void *state)
{
    const BACKUPINFO *info;
    int taskState;

    if (var_selectedVm_8c1ba34c == -1) {
        TaskFree_8c014b66(task);
        FUN_8c01328c();
        return;
    }

    taskState = task->field_0x08;
    switch (taskState) {
    case 0:
        info = BupGetInfo_8c014bba(var_selectedVm_8c1ba34c);
        if (info->Connect == 0) {
            var_selectedVm_8c1ba34c = -1;
            var_playMode_8c1bb8d0 = PLAY_MODE_NORMAL;
            return;
        }
        if (info->Work != 0) {
            task->field_0x08 = 2;
            return;
        }
        BupMount_8c014c00(var_selectedVm_8c1ba34c);
        task->field_0x08 = 1;
        return;
    case 1:
        info = BupGetInfo_8c014bba(var_selectedVm_8c1ba34c);
        if (info->Ready == 0) {
            return;
        }
        task->field_0x08 = 2;
        return;
    case 2: {
        int *buf;

        buf = syMalloc(0x4000);
        var_demoBuf_8c1ba3c4 = buf;
        BupLoad_8c014bc6(var_selectedVm_8c1ba34c, "BUS_REPLAY", buf);
        task->field_0x08 = 3;
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
        var_inputMapSel_8c1bb8c8 = var_demoBuf_8c1ba3c4[2];
        var_seed_8c157a64 = var_demoBuf_8c1ba3c4[3];
        dest = var_demoBuffer_8c1bc828;
        FUN_8c02f320();
        FUN_readDemo_8c02fa14(&var_demoBuf_8c1ba3c4[4], &dest, var_demoBuf_8c1ba3c4[0]);
        syFree(var_demoBuf_8c1ba3c4);
        var_demoBuf_8c1ba3c4 = (int *)-1;
        BupUnmount_8c014c46(var_selectedVm_8c1ba34c);
        TaskFree_8c014b66(task);
        FUN_8c01328c();
        return;
    }
    default:
        return;
    }
}

/* installs replayLoadTask_8c0169bc to load and start replaying a demo recorded on the selected VMU. */
STATIC void startReplayLoad_8c016b4c(void)
{
    Task *task;
    void *state;

    if (var_selectedVm_8c1ba34c == -1) {
        var_playMode_8c1bb8d0 = PLAY_MODE_NORMAL;
        return;
    }

    var_playMode_8c1bb8d0 = PLAY_MODE_DEMO;
    var_8c1bb8d4 = 0;

    TaskPush_8c014ae8(var_tasks_8c1ba3c8, replayLoadTask_8c0169bc, &task, &state, 0);
    task->field_0x08 = 0;
}
