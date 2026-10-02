/* @unit VmSelect */
#include <shinobi.h>
#include <sg_sd.h>
#include "0100bc_sound.h"
#include "0193c8_vm_select.h"
#include "018644_file_select.h"
#include "019e98_main_menu.h"
#include "sg_xpt.h"
#include "014a9c_tasks.h"
#include "015ab8_title.h"
#include "014b8c_backup.h"
#include "014f54_sprite.h"
#include "022464_render.h"
#include "02a9fc_message_box.h"
#include "1ba1c8_globals.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "strings.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#ifdef SERIAL_DEBUG
char *DEBUG_vmSelectStateNames[] = {
    "INIT",
    "FADE_IN",
    "IDLE",
    "CURSOR_ANIMATING",
    "CONFIRM",
    "PROCEED_WITHOUT_SAVING",
    "VM_WARNING_FADE_IN",
    "VM_WARNING",
    "FADE_OUT_TO_FILE_SELECT",
    "FADE_OUT_TO_MAIN_MENU",
    "FADE_IN_FROM_VM_WARNING"
};

char *DEBUG_vmuStatusNames[] = {
    "NOT_CONNECTED",
    "NOT_AVAILABLE",
    "NOT_ENOUGH_SPACE",
    "PROCEED_WITHOUT_SAVING",
    "SAVING_POSSIBLE",
    "SAVE_EXISTS_NO_SPACE",
    "SAVE_EXISTS"
};

int intArrayChecksum(int *arr, int size) {
    int checksum = 0;
    int i;
    for (i = 0; i < size; ++i) {
        checksum ^= arr[i];
    }
    return checksum;
}
#endif

#define CHANGE_STATE(x) var_menuState_8c1bc7a8.state_0x18 = x; LOG_DEBUG(("[VM_SELECT] State changed: %s\n", DEBUG_vmSelectStateNames[x]))


/* =================
 * Type Declarations
 * =================
 */

enum VM_SELECT_STATE {
    VM_SELECT_STATE_INIT = 0,
    VM_SELECT_STATE_FADE_IN = 1,
    VM_SELECT_STATE_IDLE = 2,
    VM_SELECT_STATE_CURSOR_ANIMATING = 3,
    VM_SELECT_STATE_CONFIRM = 4,
    VM_SELECT_STATE_PROCEED_WITHOUT_SAVING = 5,
    VM_SELECT_STATE_VM_WARNING_FADE_IN = 6,
    VM_SELECT_STATE_VM_WARNING = 7,
    VM_SELECT_STATE_FADE_OUT_TO_FILE_SELECT = 8,
    VM_SELECT_STATE_FADE_OUT_TO_MAIN_MENU = 9,
    VM_SELECT_STATE_FADE_IN_FROM_VM_WARNING = 10
};

/* ====================
 * Non-initialized Globals
 * ====================
 */

int var_vmuStatus_8c226048[9]; /* enum VMU_STATUS */
int var_vmMountBusy_8c22606c;


/* ===================
 * Initialized Globals
 * ===================
 */

/* Scanned in order; the empty string ends the list. */
char* init_saveNames_8c044d50[11] = {
    "TOKYOBUS.001",
    "TOKYOBUS.002",
    "TOKYOBUS.003",
    "TOKYOBUS.004",
    "TOKYOBUS.005",
    "TOKYOBUS.006",
    "TOKYOBUS.007",
    "TOKYOBUS.008",
    "TOKYOBUS.009",
    "TOKYOBUS.010",
    ""
};

/* Cursor rest positions: drives 0-3 on the top row, 4-7 on the bottom
 * alongside slot 8, the PROCEED WITHOUT SAVING option. */
NJS_POINT2 init_cursorPositions_8c044d7c[9] = {
    {185.0f, 98.0f},
    {255.0f, 98.0f},
    {325.0f, 98.0f},
    {395.0f, 98.0f},
    {150.0f, 194.0f},
    {220.0f, 194.0f},
    {290.0f, 194.0f},
    {360.0f, 194.0f},
    {430.0f, 194.0f}
};

/* Textbox line for the highlighted slot, indexed by VMU_STATUS. */
char* init_vmuStatusMessages_8c044dc4[7] = {
    NULL,
    MSG_CANNOT_USE,
    MSG_SAVE_NEED_3_BLOCKS,
    MSG_START_NO_FILE,
    MSG_SAVE_OK,
    MSG_SAVE_EXISTS,
    MSG_SAVE_EXISTS
};

/* =========
 * Functions
 * =========
 */

/* Tested */
STATIC void mountWaitTask_8c0193c8(Task *task)
{
    int drive;

    for (drive = 0; drive < 8; drive++) {
        const BACKUPINFO* bupInfo = BupGetInfo_8c014bba(drive);
        if (bupInfo->Connect && !bupInfo->Ready)
            return;
    }

    TaskKill_8c014b66(task);
    var_vmMountBusy_8c22606c = 0;
    return;
}

/* Tested */
void VmSelectMountAll_8c01940e()
{
    int drive;
    Task *createdTask;
    void *createdState;
    for (drive = 0; drive < 8; drive++) {
        const BACKUPINFO *bupInfo = BupGetInfo_8c014bba(drive);
        if (bupInfo->Connect && !bupInfo->Work)
            BupMount_8c014c00(drive);
    }

    TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, &mountWaitTask_8c0193c8, &createdTask, &createdState, 0);
    var_vmMountBusy_8c22606c = 1;
}

/* Tested */
STATIC void unmountTask_8c01946a(Task *task, void *state)
{
    int drive;
    Bool isBusy = FALSE;
    for (drive = 0; drive < 8; drive++) {
        const BACKUPINFO *bupInfo = BupGetInfo_8c014bba(drive);
        if (bupInfo->Connect) {
            int stat = buStat(drive);
            if (stat == BUD_STAT_BUSY) {
                isBusy = TRUE;
                continue;
            }

            if (bupInfo->Work)
                BupUnmount_8c014c46(drive);
        }
    }

    if (!isBusy) {
        TaskKill_8c014b66(task);
        var_vmMountBusy_8c22606c = 0;
    }
}

/* Tested */
void VmSelectUnmountAll_8c0194de()
{
    Task *createdTask;
    void *createdState;

    TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, &unmountTask_8c01946a, &createdTask, &createdState, 0);
    var_vmMountBusy_8c22606c = 1;
}

/* Tested */
void VmSelectFreeAndClear_8c019504(void)
{
    int drive;

    for (drive = 0; drive < 8; drive++) {
        if (var_gBupInfo_8c1bc4ac[drive].Work) {
            syFree(var_gBupInfo_8c1bc4ac[drive].Work);
            BupClearInfo_8c014c8a(drive);
        }
    }
}

/* Tested */
/* Refresh every drive's VMU_STATUS against saveNames and a `blocks` budget,
 * returning how many drives could be saved to or loaded from. */
int VmSelectUpdateAllStatus_8c019550(char **saveNames, Uint16 blocks)
{
    int drive;
    int count = 0;
#ifdef SERIAL_DEBUG
    int checksum = intArrayChecksum(var_vmuStatus_8c226048, 9);
#endif
    for (drive = 0; drive < 8; drive++) {
        int i;
        const BACKUPINFO *bupInfo = BupGetInfo_8c014bba(drive);

        if (!bupInfo->Connect) {
            var_vmuStatus_8c226048[drive] = VMU_STATUS_NOT_CONNECTED;
            continue;
        }

        if (!bupInfo->Work) {
            BupMount_8c014c00(drive);
            continue;
        }

        if (!bupInfo->Ready) {
            continue;
        }

        for (i = 0; *saveNames[i]; i++) {
            switch (buIsExistFile(drive, saveNames[i])) {
                case BUD_ERR_OK:
                    var_vmuStatus_8c226048[drive] = (bupInfo->DiskInfo.free_user_blocks < blocks)
                        ? VMU_STATUS_SAVE_EXISTS_NO_SPACE
                        : VMU_STATUS_SAVE_EXISTS;
                    count++;
                    break;

                case BUD_ERR_UNFORMAT:
                    var_vmuStatus_8c226048[drive] = VMU_STATUS_NOT_AVAILABLE;
                    // Missing break statement in original code

                case BUD_ERR_BUSY:
                    var_vmuStatus_8c226048[drive] = VMU_STATUS_NOT_AVAILABLE;
                    break;

                default:
                    // File not found, continue to next file
                    continue;
            }

            // If this is reached, the vmu status was set
            break;
        }

        if (!*saveNames[i]) {
            if (bupInfo->DiskInfo.free_user_blocks < blocks) {
                var_vmuStatus_8c226048[drive] = VMU_STATUS_NOT_ENOUGH_SPACE;
            } else {
                var_vmuStatus_8c226048[drive] = VMU_STATUS_SAVING_POSSIBLE;
                count++;
            }
        }
    }

    var_vmuStatus_8c226048[8] = VMU_STATUS_PROCEED_WITHOUT_SAVING;

#ifdef SERIAL_DEBUG
    if (checksum != intArrayChecksum(var_vmuStatus_8c226048, 9)) {
        int i;
        LOG_INFO(("[VM_SELECT] VMU status changed:\n"));
        for (i = 0; i < 9; i++) {
            LOG_INFO(("          Slot %d: %s\n", i, DEBUG_vmuStatusNames[var_vmuStatus_8c226048[i]]));
        }
    }
#endif

    return count;
}

/* Tested */
void VmSelectUpdateStatus_8c01967c(Sint32 drive, char* saveName, Uint16 blocks)
{
    const BACKUPINFO *bupInfo = BupGetInfo_8c014bba(drive);

    if (!bupInfo->Connect) {
        var_vmuStatus_8c226048[drive] = VMU_STATUS_NOT_CONNECTED;
        return;
    }

    if (!bupInfo->Work) {
        BupMount_8c014c00(drive);
    }

    if (!bupInfo->Ready) {
        var_vmuStatus_8c226048[drive] = VMU_STATUS_NOT_CONNECTED;
        return;
    }

    switch (buIsExistFile(drive, saveName)) {
        case BUD_ERR_OK:
            var_vmuStatus_8c226048[drive] = (bupInfo->DiskInfo.free_user_blocks < blocks)
                ? VMU_STATUS_SAVE_EXISTS_NO_SPACE
                : VMU_STATUS_SAVE_EXISTS;
            break;

        case BUD_ERR_UNFORMAT:
            var_vmuStatus_8c226048[drive] = VMU_STATUS_NOT_AVAILABLE;
            // Missing break statement in original code

        case BUD_ERR_BUSY:
            var_vmuStatus_8c226048[drive] = VMU_STATUS_NOT_AVAILABLE;
            break;

        default:
            if (bupInfo->DiskInfo.free_user_blocks < blocks) {
                var_vmuStatus_8c226048[drive] = VMU_STATUS_NOT_ENOUGH_SPACE;
            } else {
                var_vmuStatus_8c226048[drive] = VMU_STATUS_SAVING_POSSIBLE;
            }
            break;
    }
}

/* Tested, unused */
STATIC int saveFileExists_8c019730(Sint32 drive, char* saveName)
{
    const BACKUPINFO *bupInfo = BupGetInfo_8c014bba(drive);
    if (!bupInfo->Connect
        || !bupInfo->Work
        || !bupInfo->Ready
        || buIsExistFile(drive, saveName) != BUD_ERR_OK)
    {
        return 0;
    }

    return 1;
}

/* Tested */
STATIC void initCursorLerp_8c019788(int drive)
{
    var_menuState_8c1bc7a8.pos.vmSelect.cursorTarget_0x28.x = init_cursorPositions_8c044d7c[drive].x;
    var_menuState_8c1bc7a8.pos.vmSelect.cursorTarget_0x28.y = init_cursorPositions_8c044d7c[drive].y;
    var_menuState_8c1bc7a8.cursorVelocity_0x30.x =
        (init_cursorPositions_8c044d7c[drive].x - var_menuState_8c1bc7a8.pos.vmSelect.cursor_0x20.x) / 6;
    var_menuState_8c1bc7a8.cursorVelocity_0x30.y =
        (init_cursorPositions_8c044d7c[drive].y - var_menuState_8c1bc7a8.pos.vmSelect.cursor_0x20.y) / 6;
}

/* Tested */
STATIC void drawVmMenu_8c0197c0()
{
    int drive;
    int textureId = 8;

    // Draw cursor
    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        0x10,
        var_menuState_8c1bc7a8.pos.vmSelect.cursor_0x20.x,
        var_menuState_8c1bc7a8.pos.vmSelect.cursor_0x20.y,
        -4.0
    );

    // Draw connected VMUs
    for (drive = 0; drive < 8; drive++) {
        if (var_vmuStatus_8c226048[drive]) {
            SpriteDraw_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                textureId, 0.0, 0.0, -5.0
            );
        }
        textureId++;
    }

    // Draw default VMU icons
    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        7, 0.0, 0.0, -6.0
    );

    // Draw textbox background
    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00,
        1, 0.0, 0.0, -4.3
    );

    // Draw background
    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00,
        0, 0.0, 0.0, -7.0
    );
}

/* Tested */
STATIC void drawVmWarning_8c019852()
{
    SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0x11, 0.0, 0.0, -5.0);
    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00,
        var_menuState_8c1bc7a8.cursorCol_0x3c + 2,
        228.0, 304.0, -5.0
    );
    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00, 0,
        0.0, 0.0, -7.0
    );
}

/* Tested */
STATIC void vmMenuTask_8c0198a0(Task* task, void *actionState)
{
    int slot = var_menuState_8c1bc7a8.selected_0x38;
    switch (var_menuState_8c1bc7a8.state_0x18)
    {
        /* Init */
        case VM_SELECT_STATE_INIT: {
            var_menuState_8c1bc7a8.timer_0x68++;
            if (var_menuState_8c1bc7a8.timer_0x68 < 5) {
                return;
            }

            LOG_INFO(("[VM_SELECT] Initializing VM Select menu\n"));

            if (VmSelectUpdateAllStatus_8c019550(init_saveNames_8c044d50, 3)) {
                CHANGE_STATE(VM_SELECT_STATE_FADE_IN);

                // Start on the first connected drive
                for (slot = 0; var_vmuStatus_8c226048[slot] == VMU_STATUS_NOT_CONNECTED; slot++);
                initCursorLerp_8c019788(slot);

                var_menuState_8c1bc7a8.pos.vmSelect.cursor_0x20 = var_menuState_8c1bc7a8.pos.vmSelect.cursorTarget_0x28;
                MessageBoxSwapFor_8c02aefc(init_vmuStatusMessages_8c044dc4[var_vmuStatus_8c226048[slot]]);
                SndStopBgm_8c010d8a();
                SndPlayAdx_8c010cd6(0,0xe);
            } else {
                CHANGE_STATE(VM_SELECT_STATE_VM_WARNING_FADE_IN);
                task->field_0x08 = 0;
                var_menuState_8c1bc7a8.cursorCol_0x3c = 0;
            }

            RenderStartFadeIn_8c022a9c(10);
            return;
        }

        // Fade In
        case VM_SELECT_STATE_FADE_IN: {
            if (!var_isFading_8c226568) {
                CHANGE_STATE(VM_SELECT_STATE_IDLE);
            }

            drawVmMenu_8c0197c0();
            break;
        }

        // Idle
        case VM_SELECT_STATE_IDLE: {
            VmSelectUpdateAllStatus_8c019550(init_saveNames_8c044d50, 3);

            if (!var_vmuStatus_8c226048[slot]) {
                for (slot = 0; !var_vmuStatus_8c226048[slot]; slot++);
            } else {
                // If on upper row
                if (slot < 4) {
                    // Move left
                    if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KL) {
                        do {
                            if (--slot < 0) slot = 3;
                        } while (!var_vmuStatus_8c226048[slot]);
                    }
                    // Move right
                    else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KR) {
                        do {
                            if (++slot > 3) slot = 0;
                        } while (!var_vmuStatus_8c226048[slot]);
                    }
                    // Move down
                    else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KD) {
                        for (slot += 4; !var_vmuStatus_8c226048[slot]; slot++);
                    }
                }
                // Else, on lower row
                else {
                    // Move left
                    if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KL) {
                        do {
                            if (--slot < 4) slot = 8; 
                        } while (!var_vmuStatus_8c226048[slot]);
                    }
                    // Move right
                    else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KR) {
                        do {
                            if (++slot > 8) slot = 8;
                        } while (!var_vmuStatus_8c226048[slot]);
                    }
                    // Move up
                    else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KU) {
                        int i;
                        for (i = 0; !var_vmuStatus_8c226048[i]; i++);

                        if (i < 4) {
                            if (slot == 8) slot = 3;
                            else slot -= 4;

                            for (; !var_vmuStatus_8c226048[slot]; ) {
                                slot--;
                                if (slot < 0) slot = 3;
                            }

                            // The original emits the search twice; this pass never runs.
                            while (!var_vmuStatus_8c226048[slot]) {
                                if (--slot < 0) slot = 3;
                            }
                        }
                    }
                }

                // If slot didn't change and A was pressed
                if (slot == var_menuState_8c1bc7a8.selected_0x38
                    && (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA))
                {
                    int status = var_vmuStatus_8c226048[slot];
                    if (
                        status == VMU_STATUS_SAVING_POSSIBLE
                        || status == VMU_STATUS_SAVE_EXISTS_NO_SPACE
                        || status == VMU_STATUS_SAVE_EXISTS)
                    {
                        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                        MessageBoxSwapFor_8c02aefc(MSG_CONFIRM);
                        var_menuState_8c1bc7a8.selectedVmuSlot_0x6c = slot;
                        var_menuState_8c1bc7a8.bupInfo_0x78 = BupGetInfo_8c014bba(slot);
                        CHANGE_STATE(VM_SELECT_STATE_CONFIRM);
                    }
                    else if (status == VMU_STATUS_PROCEED_WITHOUT_SAVING) {
                        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                        MessageBoxSwapFor_8c02aefc(
                            MSG_SAVE_NO_FILE
                            MSG_CONFIRM_START_NO_FILE
                        );
                        CHANGE_STATE(VM_SELECT_STATE_PROCEED_WITHOUT_SAVING);
                    }
                    else {
                        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                    }

                    var_menuState_8c1bc7a8.cursorCol_0x3c = 0;
                }
            }

            // If selection changed
            if (slot != var_menuState_8c1bc7a8.selected_0x38) {
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                initCursorLerp_8c019788(slot);
                CHANGE_STATE(VM_SELECT_STATE_CURSOR_ANIMATING);
                MessageBoxSwapFor_8c02aefc(init_vmuStatusMessages_8c044dc4[var_vmuStatus_8c226048[slot]]);
            }

            drawVmMenu_8c0197c0();
            MessageBoxMenuTextboxText_8c02af1c(0x20);
            break;
        }

        /* Cursor animating */
        case VM_SELECT_STATE_CURSOR_ANIMATING: {
            if (CourseMenuInterpolateCursor_8c016d2c()) {
                CHANGE_STATE(VM_SELECT_STATE_IDLE);
                MessageBoxSwapFor_8c02aefc(init_vmuStatusMessages_8c044dc4[var_vmuStatus_8c226048[slot]]);
            }

            drawVmMenu_8c0197c0();
            MessageBoxMenuTextboxText_8c02af1c(0x20);
            break;
        }

        /* Confirm */
        case VM_SELECT_STATE_CONFIRM: {
            int promptResult = PromptHandleBinary_8c016caa(&var_menuState_8c1bc7a8.cursorCol_0x3c);
            if (promptResult == 1) {
                var_selectedVm_8c1ba34c = var_menuState_8c1bc7a8.selectedVmuSlot_0x6c;
                CHANGE_STATE(VM_SELECT_STATE_FADE_OUT_TO_FILE_SELECT);
                RenderStartFadeOut_8c022b60(10);
            } else if (promptResult == 2) {
                MessageBoxSwapFor_8c02aefc(init_vmuStatusMessages_8c044dc4[var_vmuStatus_8c226048[slot]]);
                CHANGE_STATE(VM_SELECT_STATE_IDLE);
            }

            SpriteDraw_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                var_menuState_8c1bc7a8.cursorCol_0x3c + 2,
                228.0,
                304.0,
                -5.0
            );
            drawVmMenu_8c0197c0();
            MessageBoxMenuTextboxText_8c02af1c(0xff);
            break;
        }

        // Proceed without saving?
        case VM_SELECT_STATE_PROCEED_WITHOUT_SAVING: {
            int promptResult = PromptHandleBinary_8c016caa(&var_menuState_8c1bc7a8.cursorCol_0x3c);
            if (promptResult == 1) {
                var_selectedVm_8c1ba34c = -1;
                FileSelectResetNewGame_8c01895e();
                CHANGE_STATE(VM_SELECT_STATE_FADE_OUT_TO_MAIN_MENU);
                SndStartAdxFadeOut_8c010bae(0);
                SndStartAdxFadeOut_8c010bae(1);
                RenderStartFadeOut_8c022b60(10);
            } else if (promptResult == 2) {
                MessageBoxSwapFor_8c02aefc(init_vmuStatusMessages_8c044dc4[var_vmuStatus_8c226048[slot]]);
                slot = 8;
                CHANGE_STATE(VM_SELECT_STATE_IDLE);
            }

            SpriteDraw_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                var_menuState_8c1bc7a8.cursorCol_0x3c + 2,
                228.0,
                304.0,
                -5.0
            );
            drawVmMenu_8c0197c0();
            MessageBoxMenuTextboxText_8c02af1c(0xff);
            break;
        }

        // VM Warning Fade In
        case VM_SELECT_STATE_VM_WARNING_FADE_IN: {
            if (!var_isFading_8c226568) {
                CHANGE_STATE(VM_SELECT_STATE_VM_WARNING);
            }
            drawVmWarning_8c019852();
            break;
        }

        // VM Warning
        case VM_SELECT_STATE_VM_WARNING: {
            // task->field_0x08 is this screen's own sub-phase.
            switch (task->field_0x08) {
                // Idle
                case 0: {
                    if (!VmSelectUpdateAllStatus_8c019550(init_saveNames_8c044d50, 3)) {
                        int promptResult = PromptHandleBinary_8c016caa(&var_menuState_8c1bc7a8.cursorCol_0x3c);
                        if (promptResult == 1) {
                            var_selectedVm_8c1ba34c = -1;
                            FileSelectResetNewGame_8c01895e();
                            task->field_0x08 = 2;
                            RenderStartFadeOut_8c022b60(10);
                        } else if (promptResult == 2) {
                            for (slot = 0; !var_vmuStatus_8c226048[slot]; slot++);
                            initCursorLerp_8c019788(slot);
                            var_menuState_8c1bc7a8.pos.vmSelect.cursor_0x20 = var_menuState_8c1bc7a8.pos.vmSelect.cursorTarget_0x28;
                            MessageBoxSwapFor_8c02aefc(init_vmuStatusMessages_8c044dc4[var_vmuStatus_8c226048[slot]]);
                            task->field_0x08 = 3;
                            RenderStartFadeOut_8c022b60(10);
                        }
                    }
                    // VM inserted
                    else {
                        task->field_0x08 = 1;
                        RenderStartFadeOut_8c022b60(10);
                    }

                    break;
                }

                // VM Inserted Fade out
                case 1: {
                    if (!var_isFading_8c226568) {
                        var_menuState_8c1bc7a8.timer_0x68 = 10;
                        CHANGE_STATE(VM_SELECT_STATE_INIT);
                        return;
                    }
                    break;
                }

                // VM Warning Fade out to Main Menu
                case 2: {
                    if (!var_isFading_8c226568) {
                        MainMenuEnter_8c01a09a(task, 0);
                        return;
                    }
                    break;
                }

                // VM Warning Fade out to VM Select
                case 3: {
                    if (!var_isFading_8c226568) {
                        CHANGE_STATE(VM_SELECT_STATE_FADE_IN_FROM_VM_WARNING);
                        return;
                    }
                    break;
                }
            }

            drawVmWarning_8c019852();
            return;
        }

        // Confirm Fade Out to Main Menu
        case VM_SELECT_STATE_FADE_OUT_TO_FILE_SELECT: {
            if (!var_isFading_8c226568) {
                FileSelectEnter_8c019334(task);
                return;
            }

            SpriteDraw_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                var_menuState_8c1bc7a8.cursorCol_0x3c + 2,
                228.0,
                304.0,
                -5.0
            );

            drawVmMenu_8c0197c0();
            break;
        }

        // Fade out
        case VM_SELECT_STATE_FADE_OUT_TO_MAIN_MENU: {
            if (var_isFading_8c226568) {
                break;
            }
            // Bits 0 and 4 stay set while the two ADX streams are still
            // playing; hold the screen until both fade-outs have finished.
            if (init_adxPlaying_8c03bd80) return;
            MainMenuEnter_8c01a09a(task, 0);
            return;
        }

        // VM Warning Fade In to VM Select
        case VM_SELECT_STATE_FADE_IN_FROM_VM_WARNING: {
            if (!var_isFading_8c226568) {
                CHANGE_STATE(VM_SELECT_STATE_IDLE);
            }
            drawVmMenu_8c0197c0();
        }
    }

    var_menuState_8c1bc7a8.selected_0x38 = slot;
}

/* Tested */
void VmSelectEnter_8c019e44(Task *task)
{
    TaskSwitch_8c014b3e(task, vmMenuTask_8c0198a0);
    var_menuState_8c1bc7a8.state_0x18 = VM_SELECT_STATE_INIT;
    var_menuState_8c1bc7a8.selected_0x38 = 0;
    var_menuState_8c1bc7a8.timer_0x68 = 0;
}
