/* @unit SystemMenu */
#include <shinobi.h>
#include "01b19c_system_menu.h"
#include "014a9c_tasks.h"
#include "1ba1c8_globals.h"
#include "013ae8_route.h"
#include "01c980_profile_file.h"
#include "0193c8_vm_select.h"
#include "014f54_sprite.h"
#include "014b8c_backup.h"
#include "011120_asset_queues.h"
#include "02a9fc_message_box.h"
#include "022464_render.h"
#include "016c58_prompt.h"
#include "016d2c_course_menu.h"
#include "018644_file_select.h"
#include "01bb48_vm_game.h"
#include "015ab8_title.h"
#include "01614c_replay_menu.h"
#include "0100bc_sound.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "strings.h"

/* ====================
 * Type Declarations
 * ====================
 */

typedef enum SaveState {
    SAVE_STATE_LOAD_ICON      = 0, /* wait for bus_mem.VMI */
    SAVE_STATE_FADE_IN        = 1,
    SAVE_STATE_TOP_MENU       = 2,
    SAVE_STATE_LOAD           = 3,
    SAVE_STATE_LOAD_FAILED    = 4, /* corrupt save; A returns to title */
    SAVE_STATE_SAVE           = 5,
    SAVE_STATE_QUIT_CONFIRM   = 6,
    SAVE_STATE_EXIT_TO_COURSE = 7,
    SAVE_STATE_EXIT_TO_TITLE  = 8
} SaveState;

typedef enum SaveMenuItem {
    SAVE_MENU_LOAD = 0,
    SAVE_MENU_SAVE = 1,
    SAVE_MENU_BACK = 2,
    SAVE_MENU_QUIT = 3
} SaveMenuItem;

typedef enum SavePhase {
    SAVE_PHASE_CONFIRM     = 0,
    SAVE_PHASE_IN_PROGRESS = 1
} SavePhase;

/* ====================
 * Non-initialized Globals
 * ====================
 */

/* VMS file comment, "9/<day> EXP <points>" */
STATIC char var_vmsComment_8c226098[16];

/* ====================
 * Forward Declarations
 * ====================
 */

STATIC void writeDecimalDigits_8c01b1c0(char *dst, int value);
STATIC void updateVmsComment_8c01b206(void);

/* ====================
 * Functions
 * ====================
 */

void SystemMenuApplyLoadedProgress_8c01b19c(void)
{
    var_runReportPending_8c1bb8b8 = var_progress_8c1ba1cc.runReportPending_0xd8;
    var_runWasPractice_8c1bb8bc = var_progress_8c1ba1cc.runWasPractice_0xdc;
    var_runSucceeded_8c1bb8dc = var_progress_8c1ba1cc.runSucceeded_0xe0;
    var_award_8c1bb8f8 = var_progress_8c1ba1cc.award_0xe4;
}

/* No terminator. */
STATIC void writeDecimalDigits_8c01b1c0(char *dst, int value)
{
    char buf[8];
    char *p;
    int count;

    p = &buf[7];
    count = 0;
    do {
        *p = (char)(value % 10) + '0';
        p--;
        count++;
        value /= 10;
    } while (value != 0);

    for (; count > 0; count--) {
        p++;
        *dst = *p;
        dst++;
    }
}

/* Space-padded, unterminated. */
STATIC void updateVmsComment_8c01b206(void)
{
    int i;

    for (i = 0; i < 0x10; i++) {
        var_vmsComment_8c226098[i] = ' ';
    }
    strcpy(var_vmsComment_8c226098, "9/   EXP ");
    writeDecimalDigits_8c01b1c0(&var_vmsComment_8c226098[2], var_progress_8c1ba1cc.days_0x00);
    writeDecimalDigits_8c01b1c0(&var_vmsComment_8c226098[9], var_progress_8c1ba1cc.exp_0x90);
}

void SystemMenuWriteToVmu_8c01b26c(void)
{
    int size;

    var_progress_8c1ba1cc.runReportPending_0xd8 = var_runReportPending_8c1bb8b8;
    var_progress_8c1ba1cc.runWasPractice_0xdc = var_runWasPractice_8c1bb8bc;
    var_progress_8c1ba1cc.runSucceeded_0xe0 = var_runSucceeded_8c1bb8dc;
    var_progress_8c1ba1cc.award_0xe4 = var_award_8c1bb8f8;
    ProfileFileUpdateUnlocks_8c01c980();
    var_progress_8c1ba1cc.profileUnlockedCount_0x8c = var_profileUnlockedCount_8c2263a4;
    updateVmsComment_8c01b206();

    memset(&var_backupFileHeader_8c1ba2e4, 0, 0x60);
    /* lands in vms_comment[18]; its last two bytes stay zeroed */
    njMemCopy(&var_backupFileHeader_8c1ba2e4, var_vmsComment_8c226098, 0x10);
    /* "Tokyo Bus Guide data" */
    strcpy(var_backupFileHeader_8c1ba2e4.btr_comment, STR_TITLE_DATA);
    njMemCopy(var_backupFileHeader_8c1ba2e4.game_name, init_bupGameName_8c04410c, 0x10);
    var_backupFileHeader_8c1ba2e4.icon_palette = var_vmuIconFileBuf_8c1ba344;
    var_backupFileHeader_8c1ba2e4.icon_data = (char *)var_vmuIconFileBuf_8c1ba344 + 0x20;
    var_backupFileHeader_8c1ba2e4.icon_num = 1;
    var_backupFileHeader_8c1ba2e4.icon_speed = 1;
    var_backupFileHeader_8c1ba2e4.save_data = &var_progress_8c1ba1cc;
    var_backupFileHeader_8c1ba2e4.save_size = 0xe8;

    size = buCalcBackupFileSize(var_backupFileHeader_8c1ba2e4.icon_num, var_backupFileHeader_8c1ba2e4.visual_type,
                                var_backupFileHeader_8c1ba2e4.save_size);
    var_backupFileImageBuf_8c1ba348 = syMalloc(size << 9);
    buMakeBackupFileImage(var_backupFileImageBuf_8c1ba348, &var_backupFileHeader_8c1ba2e4);
    BupSave_8c014bcc(var_selectedVm_8c1ba34c,
                     init_saveNames_8c044d50[var_saveSlot_8c1ba350], var_backupFileImageBuf_8c1ba348, 3);
    var_vmBusy_8c157a7c = 1;
}

STATIC void menuTask_8c01b3ac(Task *task, void *state)
{
    int vmStatus;
    int result;
    int drawPrompt = 0;

    if (var_menuState_8c1bc7a8.state_0x18 < SAVE_STATE_EXIT_TO_COURSE) {
        if (var_selectedVm_8c1ba34c == -1) {
            vmStatus = VMU_STATUS_PROCEED_WITHOUT_SAVING;
        } else {
            VmSelectUpdateStatus_8c01967c(var_selectedVm_8c1ba34c,
                                           init_saveNames_8c044d50[var_saveSlot_8c1ba350], 3);
            vmStatus = var_vmuStatus_8c226048[var_selectedVm_8c1ba34c];
        }
    }

    switch (var_menuState_8c1bc7a8.state_0x18) {
    case SAVE_STATE_LOAD_ICON:
        if (RouteGetLatch_8c01432a() != 0) {
            return;
        }
        AsqFreeQueues_8c011f7e();
        var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_FADE_IN;
        return;

    case SAVE_STATE_FADE_IN:
        if (var_isFading_8c226568 == 0) {
            var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_TOP_MENU;
        }
        break;

    case SAVE_STATE_TOP_MENU:
        PromptHandleMultiple_8c016c58(&var_menuState_8c1bc7a8.selected_0x38, 4);
        if ((var_peripherals_8c1ba35c[0].press & (PDD_DGT_KR | PDD_DGT_KL)) != 0) {
            MessageBoxSetText_8c02aefc("");
        }
        if ((var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) == 0) {
            if ((var_peripherals_8c1ba35c[0].press & PDD_DGT_TB) == 0) {
                break;
            }
            var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_EXIT_TO_COURSE;
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
            VmSelectUnmountAll_8c0194de();
            RenderStartFadeOut_8c022b60(10);
            break;
        }
        var_menuState_8c1bc7a8.cursorCol_0x3c = 0;
        var_menuState_8c1bc7a8.subState_0x1c = SAVE_PHASE_CONFIRM;
        switch (var_menuState_8c1bc7a8.selected_0x38) {
        case SAVE_MENU_LOAD:
            switch (vmStatus) {
            case VMU_STATUS_SAVE_EXISTS_NO_SPACE:
            case VMU_STATUS_SAVE_EXISTS:
                var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_LOAD;
                MessageBoxSetText_8c02aefc(MSG_CONFIRM_LOAD);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                break;
            case VMU_STATUS_NOT_ENOUGH_SPACE:
            case VMU_STATUS_SAVING_POSSIBLE:
                MessageBoxSetText_8c02aefc(MSG_LOAD_NO_FILE);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                break;
            case VMU_STATUS_NOT_CONNECTED:
                MessageBoxSetText_8c02aefc(MSG_VM_NOT_CONNECTED);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                break;
            case VMU_STATUS_NOT_AVAILABLE:
                MessageBoxSetText_8c02aefc(MSG_VM_CANT_LOAD);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                break;
            case VMU_STATUS_PROCEED_WITHOUT_SAVING:
                MessageBoxSetText_8c02aefc(MSG_LOAD_NO_FILE_SET);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                break;
            default:
                break;
            }
            break;
        case SAVE_MENU_SAVE:
            switch (vmStatus) {
            case VMU_STATUS_SAVING_POSSIBLE:
                MessageBoxSetText_8c02aefc(MSG_CONFIRM_CREATE_FILE);
                var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_SAVE;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                break;
            case VMU_STATUS_SAVE_EXISTS_NO_SPACE:
            case VMU_STATUS_SAVE_EXISTS:
                MessageBoxSetText_8c02aefc(MSG_CONFIRM_OVERWRITE);
                var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_SAVE;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                break;
            case VMU_STATUS_NOT_ENOUGH_SPACE:
                MessageBoxSetText_8c02aefc(MSG_SAVE_NEED_3_BLOCKS);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                break;
            case VMU_STATUS_NOT_AVAILABLE:
                MessageBoxSetText_8c02aefc(MSG_VM_CANT_SAVE);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                break;
            case VMU_STATUS_NOT_CONNECTED:
                MessageBoxSetText_8c02aefc(MSG_VM_NOT_CONN_SAVE);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                break;
            case VMU_STATUS_PROCEED_WITHOUT_SAVING:
                MessageBoxSetText_8c02aefc(MSG_SAVE_NO_FILE_SET);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                break;
            default:
                break;
            }
            break;
        case SAVE_MENU_BACK:
            var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_EXIT_TO_COURSE;
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            VmSelectUnmountAll_8c0194de();
            RenderStartFadeOut_8c022b60(10);
            break;
        case SAVE_MENU_QUIT:
            var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_QUIT_CONFIRM;
            if (var_gameMode_8c1bb8fc == 0) {
                MessageBoxSetText_8c02aefc(MSG_CONFIRM_QUIT_STORY);
            } else {
                MessageBoxSetText_8c02aefc(MSG_CONFIRM_QUIT_FREERUN);
            }
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            break;
        default:
            break;
        }
        break;

    case SAVE_STATE_LOAD:
        switch (var_menuState_8c1bc7a8.subState_0x1c) {
        case SAVE_PHASE_CONFIRM:
            result = PromptHandleBinary_8c016caa(&var_menuState_8c1bc7a8.cursorCol_0x3c);
            if (result == 1) {
                /* yes */
                BupLoad_8c014bc6(var_selectedVm_8c1ba34c,
                                 init_saveNames_8c044d50[var_saveSlot_8c1ba350], var_saveBuf_8c1ba2e0);
                var_vmBusy_8c157a7c = 1;
                var_menuState_8c1bc7a8.subState_0x1c = SAVE_PHASE_IN_PROGRESS;
                MessageBoxSetText_8c02aefc(MSG_LOADING_NO_POWER_OFF);
                VmGameSetLcdAnim_8c01c8fc(1);
            } else if (result == 2) {
                /* no */
                var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_TOP_MENU;
                MessageBoxSetText_8c02aefc("");
            } else if (vmStatus != VMU_STATUS_SAVE_EXISTS_NO_SPACE &&
                       vmStatus != VMU_STATUS_SAVE_EXISTS) {
                MessageBoxSetText_8c02aefc(MSG_LOAD_NO_TARGET);
                var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_TOP_MENU;
            }
            break;
        case SAVE_PHASE_IN_PROGRESS:
            if (buStat(var_selectedVm_8c1ba34c) != 0) {
                break;
            }
            if (buGetLastError(var_selectedVm_8c1ba34c) == 0) {
                buAnalyzeBackupFileImage(&var_backupFileHeader_8c1ba2e4, var_saveBuf_8c1ba2e0);
                njMemCopy(&var_progress_8c1ba1cc, var_backupFileHeader_8c1ba2e4.save_data, 0xe8);
                if (FileSelectIsSaveValid_8c018804((int *)&var_progress_8c1ba1cc) != 0) {
                    SystemMenuApplyLoadedProgress_8c01b19c();
                    FileSelectApplySoundSettings_8c0189fc();
                    MessageBoxSetText_8c02aefc(MSG_LOAD_DONE);
                } else {
                        MessageBoxSetText_8c02aefc(MSG_LOAD_FAIL_ABORT);
                    var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_LOAD_FAILED;
                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                    break;
                }
            } else {
                MessageBoxSetText_8c02aefc(MSG_LOAD_FAILED);
            }
            var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_TOP_MENU;
            var_vmBusy_8c157a7c = 0;
            VmGameSetLcdAnim_8c01c8fc(0);
            break;
        default:
            break;
        }
        drawPrompt = 1;
        break;

    case SAVE_STATE_LOAD_FAILED:
        if ((var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) != 0) {
            ReplayMenuFreeSessionAssets_8c016182();
            TitleStart_8c015fd6(0);
            return;
        }
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                               4, 0.0f, 0.0f, -5.0f);
        if (MessageBoxDrawText_8c02af1c(0xff) != 0) {
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00,
                                   1, 0.0f, 0.0f, -5.0f);
        }
        return;

    case SAVE_STATE_SAVE:
        switch (var_menuState_8c1bc7a8.subState_0x1c) {
        case SAVE_PHASE_CONFIRM:
            result = PromptHandleBinary_8c016caa(&var_menuState_8c1bc7a8.cursorCol_0x3c);
            if (result == 1) {
                /* yes */
                SystemMenuWriteToVmu_8c01b26c();
                var_menuState_8c1bc7a8.subState_0x1c = SAVE_PHASE_IN_PROGRESS;
                MessageBoxSetText_8c02aefc(MSG_SAVING_NO_POWER_OFF);
                VmGameSetLcdAnim_8c01c8fc(1);
                SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00,
                                       var_menuState_8c1bc7a8.cursorCol_0x3c + 2,
                                       228.0f, 266.0f, -4.0f);
            } else if (result == 2) {
                /* no */
                var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_TOP_MENU;
                MessageBoxSetText_8c02aefc("");
                SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00,
                                       var_menuState_8c1bc7a8.cursorCol_0x3c + 2,
                                       228.0f, 266.0f, -4.0f);
            } else if (vmStatus != VMU_STATUS_SAVING_POSSIBLE &&
                       vmStatus != VMU_STATUS_SAVE_EXISTS_NO_SPACE &&
                       vmStatus != VMU_STATUS_SAVE_EXISTS) {
                MessageBoxSetText_8c02aefc(MSG_SAVE_CANT);
                var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_TOP_MENU;
            }
            break;
        case SAVE_PHASE_IN_PROGRESS:
            BupGetInfo_8c014bba(var_selectedVm_8c1ba34c);
            if (buStat(var_selectedVm_8c1ba34c) != 0) {
                break;
            }
            if (buGetLastError(var_selectedVm_8c1ba34c) == 0) {
                MessageBoxSetText_8c02aefc(MSG_SAVE_DONE);
            } else {
                MessageBoxSetText_8c02aefc(MSG_SAVE_FAILED);
            }
            var_vmBusy_8c157a7c = 0;
            syFree(var_backupFileImageBuf_8c1ba348);
            var_backupFileImageBuf_8c1ba348 = (void *)-1;
            var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_TOP_MENU;
            VmGameSetLcdAnim_8c01c8fc(0);
            break;
        default:
            break;
        }
        drawPrompt = 1;
        break;

    case SAVE_STATE_QUIT_CONFIRM:
        result = PromptHandleBinary_8c016caa(&var_menuState_8c1bc7a8.cursorCol_0x3c);
        if (result == 1) {
            var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_EXIT_TO_TITLE;
            VmSelectUnmountAll_8c0194de();
            RenderStartFadeOut_8c022b60(10);
        } else if (result == 2) {
            var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_TOP_MENU;
            MessageBoxSetText_8c02aefc("");
        }
        drawPrompt = 1;
        break;

    case SAVE_STATE_EXIT_TO_COURSE:
        if (var_isFading_8c226568 != 0 || var_vmMountBusy_8c22606c != 0) {
            break;
        }
        var_menuState_8c1bc7a8.cursorCol_0x3c = 1;
        var_menuState_8c1bc7a8.cursorRow_0x40 = 0;
        FileSelectFreeBuffers_8c0187d0();
        CourseMenuEnter_8c017e18(task);
        return;

    case SAVE_STATE_EXIT_TO_TITLE:
        if (var_isFading_8c226568 != 0 || var_vmMountBusy_8c22606c != 0) {
            break;
        }
        ReplayMenuFreeSessionAssets_8c016182();
        TitleStart_8c015fd6(1);
        return;

    default:
        break;
    }

    if (drawPrompt) {
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00,
                               var_menuState_8c1bc7a8.cursorCol_0x3c + 2,
                               228.0f, 266.0f, -4.0f);
    }

    SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                           var_menuState_8c1bc7a8.selected_0x38 + 5, 0.0f, 0.0f, -4.0f);
    SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                           4, 0.0f, 0.0f, -5.0f);
    if (MessageBoxDrawText_8c02af1c(0xff)) {
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00,
                               1, 0.0f, 0.0f, -5.0f);
    }
    SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00,
                           0, 0.0f, 0.0f, -7.0f);
}

void SystemMenuEnter_8c01ba64(Task *task)
{
    TaskReinstall_8c014b3e(task, menuTask_8c01b3ac);
    var_menuState_8c1bc7a8.state_0x18 = SAVE_STATE_LOAD_ICON;
    var_menuState_8c1bc7a8.selected_0x38 = SAVE_MENU_LOAD;
    var_saveBuf_8c1ba2e0 = syMalloc(0x600);
    VmSelectUpdateStatus_8c01967c(var_selectedVm_8c1ba34c,
                                   init_saveNames_8c044d50[var_saveSlot_8c1ba350], 3);
    MessageBoxSetText_8c02aefc("");
    RenderStartFadeIn_8c022a9c(10);
    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();
    AsqRequestDat_8c011182("\\SYSTEM", "bus_mem.VMI", &var_vmuIconFileBuf_8c1ba344);
    RouteSetLatch_8c014330();
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteClearLatch_8c014322);
}
