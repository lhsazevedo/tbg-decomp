#include <shinobi.h>
#include <sg_sd.h>
#include "0193c8_vm_menu.h"
#include "sg_xpt.h"
#include "014a9c_tasks.h"
#include "015ab8_title.h"
#include "014b8c_backup.h"
#include "serial_debug.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#ifdef SERIAL_DEBUG
char *DEBUG_vmMenuStateNames[] = {
    "INIT",
    "FADE_IN",
    "IDLE",
    "CURSOR_ANIMATING",
    "CONFIRM",
    "PROCEED_WITHOUT_SAVING",
    "VM_WARNING_FADE_IN",
    "VM_WARNING",
    "CONFIRM_FADE_OUT_TO_MAIN_MENU",
    "FADE_OUT",
    "FADE_IN_FROM_VM_WARNING"
};

char *DEBUG_vmuStatusNames[] = {
    "NOT_CONNECTED",
    "NOT_AVAILABLE",
    "NOT_ENOUGH_SPACE",
    "PROCEED_WITHOUT_SAVING",
    "SAVING_POSSIBLE",
    "SAVE_EXISTS",
    "SAVE_EXISTS_NO_SPACE"
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

#define CHANGE_STATE(x) menuState_8c1bc7a8.state_0x18 = x; LOG_DEBUG(("[VM_MENU] State changed: %s\n", DEBUG_vmMenuStateNames[x]))


/* =================
 * Type Declarations
 * =================
 */

enum VM_MENU_STATE {
    VM_MENU_STATE_INIT = 0,
    VM_MENU_STATE_FADE_IN = 1,
    VM_MENU_STATE_IDLE = 2,
    VM_MENU_STATE_CURSOR_ANIMATING = 3,
    VM_MENU_STATE_CONFIRM = 4,
    VM_MENU_STATE_PROCEED_WITHOUT_SAVING = 5,
    VM_MENU_STATE_VM_WARNING_FADE_IN = 6,
    VM_MENU_STATE_VM_WARNING = 7,
    VM_MENU_STATE_CONFIRM_FADE_OUT_TO_MAIN_MENU = 8,
    VM_MENU_STATE_FADE_OUT = 9,
    VM_MENU_STATE_FADE_IN_FROM_VM_WARNING = 10
};

enum VMU_STATUS {
    VMU_STATUS_NOT_CONNECTED = 0,
    VMU_STATUS_NOT_AVAILABLE = 1,
    VMU_STATUS_NOT_ENOUGH_SPACE = 2,
    VMU_STATUS_PROCEED_WITHOUT_SAVING = 3,
    VMU_STATUS_SAVING_POSSIBLE = 4,
    VMU_STATUS_SAVE_EXISTS = 5,
    VMU_STATUS_SAVE_EXISTS_NO_SPACE = 6
};


/* =====================
 * External Declarations
   =====================
 */

extern MainMenuSwitchFromTask_8c01a09a(Task *task);

extern int init_8c03bd80;
extern char* init_saveNames_8c044d50[11];
extern char* init_vmuStatusMessages_8c044dc4[7];
extern NJS_POINT2 init_vmIconsPositions_8c044d7c[8];

extern SDMIDI var_midiHandles_8c0fcd28[7];

extern int var_selectedVm_8c1ba34c;
extern PDS_PERIPHERAL var_peripheral_8c1ba35c[2];
extern Task var_tasks_8c1ba3c8[];

extern BACKUPINFO gBupInfo_8c1bc4ac[8];

extern int var_vmuStatus_8c226048[9];
extern int var_8c22606c;
extern Bool isFading_8c226568;


/* =======================
 * Non-initialized Globals
 * =======================
 */


/* ===================
 * Initialized Globals
   ===================
 */


/* =========
 * Functions
   =========
 */

/* Tested */
STATIC void TaskWaitForVmsReady_193c8(Task *task)
{
    int drive;

    for (drive = 0; drive < 8; drive++) {
        const BACKUPINFO* bupInfo = BupGetInfo_8c014bba(drive);
        if (bupInfo->Connect && !bupInfo->Ready)
            return;
    }

    freeTask_8c014b66(task);
    var_8c22606c = 0;
    return;
}

/* Tested */
void VmMenuMountVms_1940e()
{
    int drive;
    Task *createdTask;
    void *createdState;
    for (drive = 0; drive < 8; drive++) {
        const BACKUPINFO *bupInfo = BupGetInfo_8c014bba(drive);
        if (bupInfo->Connect && !bupInfo->Work)
            BupMount_8c014c00(drive);
    }

    pushTask_8c014ae8(var_tasks_8c1ba3c8, &TaskWaitForVmsReady_193c8, &createdTask, &createdState, 0);
    var_8c22606c = 1;
}

/* Tested */
STATIC void TaskUnmountVms_1946a(Task *task, void *state)
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
        freeTask_8c014b66(task);
        var_8c22606c = 0;
    }
}

/* Tested */
void VmMenuUnmountVms_194de()
{
    Task *createdTask;
    void *createdState;

    pushTask_8c014ae8(var_tasks_8c1ba3c8, &TaskUnmountVms_1946a, &createdTask, &createdState, 0);
    var_8c22606c = 1;
}

/* Tested */
void VmMenuFreeAndClear_19504(void)
{
    int drive;

    for (drive = 0; drive < 8; drive++) {
        if (gBupInfo_8c1bc4ac[drive].Work) {
            syFree(gBupInfo_8c1bc4ac[drive].Work);
            ClearInfo_8c014c8a(drive);
        }
    }
}

/* Tested */
int VmMenuUpdateVmusStatus_19550(char **saveNames, Uint16 blocks)
{
    int drive;
    int count = 0;
#ifdef SERIAL_DEBUG
    int checksum = intArrayChecksum(var_vmuStatus_8c226048, 9);
#endif
    for (drive = 0; drive < 8; drive++) {
        int i;
        char** saveName;
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
                        ? VMU_STATUS_SAVE_EXISTS
                        : VMU_STATUS_SAVE_EXISTS_NO_SPACE;
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

    var_vmuStatus_8c226048[8] = VMU_STATUS_PROCEED_WITHOUT_SAVING; // Proceed without saving

#ifdef SERIAL_DEBUG
    if (checksum != intArrayChecksum(var_vmuStatus_8c226048, 9)) {
        int i;
        LOG_INFO(("[VM_MENU] VMU status changed:\n"));
        for (i = 0; i < 9; i++) {
            LOG_INFO(("          Slot %d: %s\n", i, DEBUG_vmuStatusNames[var_vmuStatus_8c226048[i]]));
        }
    }
#endif

    return count;
}

/* Tested */
void VmMenuUpdateVmuStatus_1967c(Sint32 drive, char* saveName, Uint16 blocks)
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
                ? VMU_STATUS_SAVE_EXISTS
                : VMU_STATUS_SAVE_EXISTS_NO_SPACE;
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
STATIC int saveFileExists_19730(Sint32 drive, char* saveName)
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
STATIC void initCursorLerp_19788(int drive)
{
    menuState_8c1bc7a8.pos.vmSelect.cursorTarget_0x28.x = init_vmIconsPositions_8c044d7c[drive].x;
    menuState_8c1bc7a8.pos.vmSelect.cursorTarget_0x28.y = init_vmIconsPositions_8c044d7c[drive].y;
    menuState_8c1bc7a8.cursorVelocity_0x30.x =
        (init_vmIconsPositions_8c044d7c[drive].x - menuState_8c1bc7a8.pos.vmSelect.cursor_0x20.x) / 6;
    menuState_8c1bc7a8.cursorVelocity_0x30.y =
        (init_vmIconsPositions_8c044d7c[drive].y - menuState_8c1bc7a8.pos.vmSelect.cursor_0x20.y) / 6;
}

/* Tested */
STATIC void drawVmMenu_197c0()
{
    int drive;
    int textureId = 8;

    // Draw cursor
    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupB_0x0c,
        0x10,
        menuState_8c1bc7a8.pos.vmSelect.cursor_0x20.x,
        menuState_8c1bc7a8.pos.vmSelect.cursor_0x20.y,
        -4.0
    );

    // Draw connected VMUs
    for (drive = 0; drive < 8; drive++) {
        if (var_vmuStatus_8c226048[drive]) {
            drawSprite_8c014f54(
                &menuState_8c1bc7a8.resourceGroupB_0x0c,
                textureId, 0.0, 0.0, -5.0
            );
        }
        textureId++;
    }

    // Draw default VMU icons
    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupB_0x0c,
        7, 0.0, 0.0, -6.0
    );

    // Draw textbox background
    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupA_0x00,
        1, 0.0, 0.0, -4.3
    );

    // Draw background
    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupA_0x00,
        0, 0.0, 0.0, -7.0
    );
}

/* Tested */
STATIC void DrawVmWarning_19852()
{
    drawSprite_8c014f54(&menuState_8c1bc7a8.resourceGroupB_0x0c, 0x11, 0.0, 0.0, -5.0);
    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupA_0x00,
        menuState_8c1bc7a8.field_0x3c + 2,
        228.0, 304.0, -5.0
    );
    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupA_0x00, 0,
        0.0, 0.0, -7.0
    );
}

/* Tested */
STATIC void VmMenuTask_198a0(Task* task, void *actionState)
{
    int slot = menuState_8c1bc7a8.selected_0x38;
    switch (menuState_8c1bc7a8.state_0x18)
    {
        /* Init */
        case VM_MENU_STATE_INIT: {
            menuState_8c1bc7a8.logo_timer_0x68++;
            if (menuState_8c1bc7a8.logo_timer_0x68 < 5) {
                return;
            }

            LOG_INFO(("[VM_MENU] Initializing VM Select menu\n"));

            if (VmMenuUpdateVmusStatus_19550(init_saveNames_8c044d50, 3)) {
                CHANGE_STATE(VM_MENU_STATE_FADE_IN);

                // Skip empty slots
                for (slot = 0; var_vmuStatus_8c226048[slot] == VMU_STATUS_NOT_CONNECTED; slot++);
                initCursorLerp_19788(slot);

                menuState_8c1bc7a8.pos.vmSelect.cursor_0x20 = menuState_8c1bc7a8.pos.vmSelect.cursorTarget_0x28;
                swapMessageBoxFor_8c02aefc(init_vmuStatusMessages_8c044dc4[var_vmuStatus_8c226048[slot]]);
                FUN_8c010d8a();
                snd_8c010cd6(0,0xe);
            } else {
                CHANGE_STATE(VM_MENU_STATE_VM_WARNING_FADE_IN);
                task->field_0x08 = 0;
                menuState_8c1bc7a8.field_0x3c = 0;
            }

            push_fadein_8c022a9c(10);
            return;
        }

        // Fade In
        case VM_MENU_STATE_FADE_IN: {
            if (!isFading_8c226568) {
                CHANGE_STATE(VM_MENU_STATE_IDLE);
            }

            drawVmMenu_197c0();
            break;
        }

        // Idle
        case VM_MENU_STATE_IDLE: {
            VmMenuUpdateVmusStatus_19550(init_saveNames_8c044d50, 3);

            if (!var_vmuStatus_8c226048[slot]) {
                for (slot = 0; !var_vmuStatus_8c226048[slot]; slot++);
            } else {
                // If on upper row
                if (slot < 4) {
                    // Move left
                    if (var_peripheral_8c1ba35c[0].press & PDD_DGT_KL) {
                        do {
                            if (--slot < 0) slot = 3;
                        } while (!var_vmuStatus_8c226048[slot]);
                    }
                    // Move right
                    else if (var_peripheral_8c1ba35c[0].press & PDD_DGT_KR) {
                        do {
                            if (++slot > 3) slot = 0;
                        } while (!var_vmuStatus_8c226048[slot]);
                    }
                    // Move down
                    else if (var_peripheral_8c1ba35c[0].press & PDD_DGT_KD) {
                        for (slot += 4; !var_vmuStatus_8c226048[slot]; slot++);
                    }
                }
                // Else, on lower row
                else {
                    // Move left
                    if (var_peripheral_8c1ba35c[0].press & PDD_DGT_KL) {
                        do {
                            if (--slot < 4) slot = 8; 
                        } while (!var_vmuStatus_8c226048[slot]);
                    }
                    // Move right
                    else if (var_peripheral_8c1ba35c[0].press & PDD_DGT_KR) {
                        do {
                            if (++slot > 8) slot = 8;
                        } while (!var_vmuStatus_8c226048[slot]);
                    }
                    // Move up
                    else if (var_peripheral_8c1ba35c[0].press & PDD_DGT_KU) {
                        int i;
                        for (i = 0; !var_vmuStatus_8c226048[i]; i++);

                        if (i < 4) {
                            if (slot == 8) slot = 3;
                            else slot -= 4;

                            for (; !var_vmuStatus_8c226048[slot]; ) {
                                slot--;
                                if (slot < 0) slot = 3;
                            }

                            while (!var_vmuStatus_8c226048[slot]) {
                                if (--slot < 0) slot = 3;
                            }
                        }
                    }
                }

                // If slot didn't change and A was pressed
                if (slot == menuState_8c1bc7a8.selected_0x38
                    && (var_peripheral_8c1ba35c[0].press & PDD_DGT_TA))
                {
                    int status = var_vmuStatus_8c226048[slot];
                    if (
                        status == VMU_STATUS_SAVING_POSSIBLE
                        || status == VMU_STATUS_SAVE_EXISTS
                        || status == VMU_STATUS_SAVE_EXISTS_NO_SPACE)
                    {
                        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                        // Is this okay?
                        swapMessageBoxFor_8c02aefc("よろしいですか？");
                        menuState_8c1bc7a8.selectedVmuSlot_0x6c = slot;
                        menuState_8c1bc7a8.bupInfo_0x78 = BupGetInfo_8c014bba(slot);
                        CHANGE_STATE(VM_MENU_STATE_CONFIRM);
                    }
                    else if (status == VMU_STATUS_PROCEED_WITHOUT_SAVING) {
                        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                        // You cannot save without setting a file.
                        // Proceed without saving?
                        swapMessageBoxFor_8c02aefc(
                            "ファイルを設定しないとセーブできません<E>"
                            "このままゲームを開始してもよろしいですか？"
                        );
                        CHANGE_STATE(VM_MENU_STATE_PROCEED_WITHOUT_SAVING);
                    }
                    else {
                        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                    }

                    menuState_8c1bc7a8.field_0x3c = 0;
                }
            }

            // If selection changed
            if (slot != menuState_8c1bc7a8.selected_0x38) {
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                initCursorLerp_19788(slot);
                CHANGE_STATE(VM_MENU_STATE_CURSOR_ANIMATING);
                swapMessageBoxFor_8c02aefc(init_vmuStatusMessages_8c044dc4[var_vmuStatus_8c226048[slot]]);
            }

            drawVmMenu_197c0();
            menuTextboxText_8c02af1c(0x20);
            break;
        }

        /* Cursor animating */
        case VM_MENU_STATE_CURSOR_ANIMATING: {
            if (interpolateCursor_8c016d2c()) {
                CHANGE_STATE(VM_MENU_STATE_IDLE);
                swapMessageBoxFor_8c02aefc(init_vmuStatusMessages_8c044dc4[var_vmuStatus_8c226048[slot]]);
            }

            drawVmMenu_197c0();
            menuTextboxText_8c02af1c(0x20);
            break;
        }

        /* Confirm */
        case VM_MENU_STATE_CONFIRM: {
            int promptResult = promptHandleBinary_16caa(&menuState_8c1bc7a8.field_0x3c);
            if (promptResult == 1) {
                var_selectedVm_8c1ba34c = menuState_8c1bc7a8.selectedVmuSlot_0x6c;
                CHANGE_STATE(VM_MENU_STATE_CONFIRM_FADE_OUT_TO_MAIN_MENU);
                push_fadeout_8c022b60(10);
            } else if (promptResult == 2) {
                swapMessageBoxFor_8c02aefc(init_vmuStatusMessages_8c044dc4[var_vmuStatus_8c226048[slot]]);
                CHANGE_STATE(VM_MENU_STATE_IDLE);
            }

            drawSprite_8c014f54(
                &menuState_8c1bc7a8.resourceGroupA_0x00,
                menuState_8c1bc7a8.field_0x3c + 2,
                228.0,
                304.0,
                -5.0
            );
            drawVmMenu_197c0();
            menuTextboxText_8c02af1c(0xff);
            break;
        }

        // Proceed without saving?
        case VM_MENU_STATE_PROCEED_WITHOUT_SAVING: {
            int promptResult = promptHandleBinary_16caa(&menuState_8c1bc7a8.field_0x3c);
            if (promptResult == 1) {
                var_selectedVm_8c1ba34c = -1;
                FUN_8c01895e();
                CHANGE_STATE(VM_MENU_STATE_FADE_OUT);
                FUN_8c010bae(0);
                FUN_8c010bae(1);
                push_fadeout_8c022b60(10);
            } else if (promptResult == 2) {
                swapMessageBoxFor_8c02aefc(init_vmuStatusMessages_8c044dc4[var_vmuStatus_8c226048[slot]]);
                slot = 8;
                CHANGE_STATE(VM_MENU_STATE_IDLE);
            }

            drawSprite_8c014f54(
                &menuState_8c1bc7a8.resourceGroupA_0x00,
                menuState_8c1bc7a8.field_0x3c + 2,
                228.0,
                304.0,
                -5.0
            );
            drawVmMenu_197c0();
            menuTextboxText_8c02af1c(0xff);
            break;
        }

        // VM Warning Fade In
        case VM_MENU_STATE_VM_WARNING_FADE_IN: {
            if (!isFading_8c226568) {
                CHANGE_STATE(VM_MENU_STATE_VM_WARNING);
            }
            DrawVmWarning_19852();
            break;
        }

        // VM Warning
        case VM_MENU_STATE_VM_WARNING: {
            int substate = task->field_0x08;
            switch (task->field_0x08) {
                // Idle
                case 0: {
                    if (!VmMenuUpdateVmusStatus_19550(init_saveNames_8c044d50, 3)) {
                        int promptResult = promptHandleBinary_16caa(&menuState_8c1bc7a8.field_0x3c);
                        if (promptResult == 1) {
                            var_selectedVm_8c1ba34c = -1;
                            FUN_8c01895e();
                            task->field_0x08 = 2;
                            push_fadeout_8c022b60(10);
                        } else if (promptResult == 2) {
                            for (slot = 0; !var_vmuStatus_8c226048[slot]; slot++);
                            initCursorLerp_19788(slot);
                            menuState_8c1bc7a8.pos.vmSelect.cursor_0x20 = menuState_8c1bc7a8.pos.vmSelect.cursorTarget_0x28;
                            swapMessageBoxFor_8c02aefc(init_vmuStatusMessages_8c044dc4[var_vmuStatus_8c226048[slot]]);
                            task->field_0x08 = 3;
                            push_fadeout_8c022b60(10);
                        }
                    }
                    // VM inserted
                    else {
                        task->field_0x08 = 1;
                        push_fadeout_8c022b60(10);
                    }

                    break;
                }

                // VM Inserted Fade out
                case 1: {
                    if (!isFading_8c226568) {
                        menuState_8c1bc7a8.logo_timer_0x68 = 10;
                        CHANGE_STATE(VM_MENU_STATE_INIT);
                        return;
                    } 
                    break;
                }

                // VM Warning Fade out to Main Menu
                case 2: {
                    if (!isFading_8c226568) {
                        MainMenuSwitchFromTask_8c01a09a(task);
                        return;
                    }
                    break;
                }

                // VM Warning Fade out to VM Select
                case 3: {
                    if (!isFading_8c226568) {
                        CHANGE_STATE(VM_MENU_STATE_FADE_IN_FROM_VM_WARNING);
                        return;
                    }
                    break;
                }
            }

            DrawVmWarning_19852();
            return;
        }

        // Confirm Fade Out to Main Menu
        case VM_MENU_STATE_CONFIRM_FADE_OUT_TO_MAIN_MENU: {
            if (!isFading_8c226568) {
                FUN_8c019334(task);
                return;
            }

            drawSprite_8c014f54(
                &menuState_8c1bc7a8.resourceGroupA_0x00,
                menuState_8c1bc7a8.field_0x3c + 2,
                228.0,
                304.0,
                -5.0
            );

            drawVmMenu_197c0();
            break;
        }

        // Fade out
        case VM_MENU_STATE_FADE_OUT: {
            if (isFading_8c226568) {
                break;
            }
            if (init_8c03bd80) return;
            MainMenuSwitchFromTask_8c01a09a(task);
            return;
        }

        // VM Warning Fade In to VM Select
        case VM_MENU_STATE_FADE_IN_FROM_VM_WARNING: {
            if (!isFading_8c226568) {
                CHANGE_STATE(VM_MENU_STATE_IDLE);
            }
            drawVmMenu_197c0();
        }
    }

    menuState_8c1bc7a8.selected_0x38 = slot;
}

/* Tested */
void VmMenuSwitchFromTask_19e44(Task *task)
{
    setTaskAction_8c014b3e(task, VmMenuTask_198a0);
    menuState_8c1bc7a8.state_0x18 = VM_MENU_STATE_INIT;
    menuState_8c1bc7a8.selected_0x38 = 0;
    menuState_8c1bc7a8.logo_timer_0x68 = 0;
}
