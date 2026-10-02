/* @unit FileSelect */
#include <shinobi.h>
#include "018644_file_select.h"
#include "011120_asset_queues.h"
#include "014a9c_tasks.h"
#include "014b8c_backup.h"
#include "016d2c_course_menu.h"
#include "0193c8_vm_select.h"
#include "019e98_main_menu.h"
#include "01a148_option.h"
#include "0100bc_sound.h"
#include "014f54_sprite.h"
#include "02a9fc_message_box.h"
#include "01bb48_vm_game.h"
#include "01b19c_system_menu.h"
#include "022464_render.h"
#include "1ba1c8_globals.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "strings.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#ifdef SERIAL_DEBUG
char *DEBUG_fileSelectStateNames[] = {
    "LOADING",
    "LOAD_ERROR",
    "ERROR_FADE_OUT",
    "READY",
    "CONFIRM",
    "CONFIRM_FADE_OUT",
    "UNMOUNT_TO_MAIN",
    "CANCEL_FADE_OUT",
    "UNMOUNT_TO_VM",
};
#endif

#define CHANGE_STATE(x) var_menuState_8c1bc7a8.state_0x18 = x; LOG_DEBUG(("[FILE_SELECT] State changed: %s\n", DEBUG_fileSelectStateNames[x]))

/* Card slots hold a VMU file index 0-9, or one of these. */
#define FILE_CARD_NEW   0xa
#define FILE_CARD_EMPTY 0xb

/* ====================
 * Type Declarations
 * ====================
 */

enum FILE_SELECT_STATE {
    FILE_SELECT_STATE_LOADING = 0,
    FILE_SELECT_STATE_LOAD_ERROR = 1,
    FILE_SELECT_STATE_ERROR_FADE_OUT = 2,
    FILE_SELECT_STATE_READY = 3,
    FILE_SELECT_STATE_CONFIRM = 4,
    FILE_SELECT_STATE_CONFIRM_FADE_OUT = 5,
    FILE_SELECT_STATE_UNMOUNT_TO_MAIN = 6,
    FILE_SELECT_STATE_CANCEL_FADE_OUT = 7,
    /* Nothing ever sets this one. */
    FILE_SELECT_STATE_UNMOUNT_TO_VM = 8,
};

enum SAVE_LOAD_RESULT {
    SAVE_LOAD_RUNNING = 0,
    SAVE_LOAD_DONE = 1,
    SAVE_LOAD_FAILED = 2,
};

typedef struct {
    TaskAction action;
    void *state;
    int phase_0x08;
    int fileIndex_0x0c;
    int field_0x10;
    int field_0x14;
    char **names_0x18;
    int field_0x1c;
} LoadFileTask;

/* ====================
 * Non-initialized Globals
 * ====================
 */

STATIC void *var_saveBufCursor_8c225fe0; /* steps 0x600 per save image */
STATIC int var_loadedSaveSlots_8c225fe4[10]; /* VMU file index of each loaded save, in load order */
STATIC int var_loadedSaveCount_8c22600c;
STATIC int var_saveLoadResult_8c226010; /* enum SAVE_LOAD_RESULT */
STATIC int var_fileCardCount_8c226014;
STATIC int var_fileCards_8c226018[12]; /* VMU file index, or FILE_CARD_NEW / FILE_CARD_EMPTY */

/* ====================
 * Functions
 * ====================
 */

STATIC void loadFileTask_8c018644(LoadFileTask *task)
{
    switch (task->phase_0x08) {
        case 0: {
            char **name;

            for (name = task->names_0x18; **name != '\0'; name++) {
                int err = buIsExistFile(var_selectedVm_8c1ba34c, *name);

                if (err != BUD_ERR_OK) {
                    if (err == BUD_ERR_FILE_NOT_FOUND) {
                        task->fileIndex_0x0c++;
                        continue;
                    }

                    LOG_WARN(("[FILE_SELECT] enumeration failed for \"%s\" (err=%d)\n", *name, err));
                    TaskUninstall_8c014b66((Task *)task);
                    var_saveLoadResult_8c226010 = SAVE_LOAD_FAILED;
                    return;
                }

                LOG_DEBUG(("[FILE_SELECT] requesting load for \"%s\"\n", *name));
                BupLoad_8c014bc6(var_selectedVm_8c1ba34c, *name, var_saveBufCursor_8c225fe0);
                var_loadedSaveSlots_8c225fe4[var_loadedSaveCount_8c22600c] = task->fileIndex_0x0c;
                var_loadedSaveCount_8c22600c++;
                task->names_0x18 = ++name;
                task->phase_0x08 = 1;
                task->fileIndex_0x0c++;
                return;
            }

            LOG_DEBUG(("[FILE_SELECT] all files loaded (%d)\n", var_loadedSaveCount_8c22600c));
            TaskUninstall_8c014b66((Task *)task);
            var_saveLoadResult_8c226010 = SAVE_LOAD_DONE;

            return;
        }

        case 1: {
            if (buStat(var_selectedVm_8c1ba34c) != BUD_STAT_READY) {
                return;
            }

            if (buGetLastError(var_selectedVm_8c1ba34c) != BUD_ERR_OK) {
                LOG_WARN(("[FILE_SELECT] load failed\n"));
                TaskUninstall_8c014b66((Task *)task);
                var_saveLoadResult_8c226010 = SAVE_LOAD_FAILED;
                return;
            }

            // Shrink the raw backup image down to its payload; the bounce
            // buffer is needed because source and destination overlap.
            var_backupFileImageBuf_8c1ba348 = syMalloc(sizeof(PlayerProgress));
            buAnalyzeBackupFileImage(&var_backupFileHeader_8c1ba2e4, var_saveBufCursor_8c225fe0);
            njMemCopy(var_backupFileImageBuf_8c1ba348, var_backupFileHeader_8c1ba2e4.save_data, sizeof(PlayerProgress));
            njMemCopy(var_saveBufCursor_8c225fe0, var_backupFileImageBuf_8c1ba348, sizeof(PlayerProgress));
            syFree(var_backupFileImageBuf_8c1ba348);
            var_backupFileImageBuf_8c1ba348 = (void *) -1;

            var_saveBufCursor_8c225fe0 = (char *)var_saveBufCursor_8c225fe0 + 0x600;
            task->phase_0x08 = 0;

            return;
        }
    }
}

STATIC void startVmLoad_8c018784(void)
{
    LoadFileTask *task;
    void *state;

    LOG_DEBUG(("[FILE_SELECT] startVmLoad_8c018784: starting VMU load\n"));

    TaskInstall_8c014ae8(var_tasks_8c1ba3c8, (void *)loadFileTask_8c018644, (Task **)&task, &state, 0);
    var_vmBusy_8c157a7c = 1;
    task->phase_0x08 = 0;
    task->fileIndex_0x0c = 0;
    task->names_0x18 = init_saveNames_8c044d50;
    var_saveBuf_8c1ba2e0 = syMalloc(0x3c00);
    var_saveBufCursor_8c225fe0 = var_saveBuf_8c1ba2e0;
    var_saveLoadResult_8c226010 = SAVE_LOAD_RUNNING;
}

void FileSelectFreeBuffers_8c0187d0(void)
{
    LOG_DEBUG(("[FILE_SELECT] FileSelectFreeBuffers_8c0187d0: freeing buffers\n"));

    // -1 marks a buffer as already freed.
    if (var_saveBuf_8c1ba2e0 != (void *)-1) {
        syFree(var_saveBuf_8c1ba2e0);
        var_saveBuf_8c1ba2e0 = (void *)-1;
    }
    if (var_vmuIconFileBuf_8c1ba344 != (void *)-1) {
        syFree(var_vmuIconFileBuf_8c1ba344);
        var_vmuIconFileBuf_8c1ba344 = (void *)-1;
    }
}

/* save is a PlayerProgress read as int[]: save[0] is days_0x00, save + 0x11
 * the courses_0x44 records, save[0x24] exp_0x90. */
int FileSelectIsSaveValid_8c018804(int *save)
{
    unsigned char *base;
    unsigned char *rec;
    int i;

    if (0 < save[0] && save[0] < 0x1f) {
        base = (unsigned char *)(save + 0x11);
        for (i = 0; i < 9; i++) {
            rec = base + i * 8;
            if (rec[0] > 1) return 0;
            if (rec[1] > 1) return 0;
            if (rec[2] > 1) return 0;
            if (rec[3] > 3) return 0;
            if (rec[4] > 3) return 0;
        }
        if (save[0x24] <= 99999) {
            return 1;
        }
    }
    return 0;
}

void FileSelectResetSettingDefaults_8c018862(void)
{
    LOG_DEBUG(("[FILE_SELECT] FileSelectResetSettingDefaults_8c018862\n"));

    var_progress_8c1ba1cc.difficulty_0xc4 = 1;
    var_progress_8c1ba1cc.driveMode_0xc5 = 0;
    var_progress_8c1ba1cc.defaultView_0xc6 = 2;
    var_progress_8c1ba1cc.vibration_0xc7 = 0;
    var_progress_8c1ba1cc.screenRoll_0xc8 = 0;
}

void FileSelectResetKeyConfigDefaults_8c0188bc(void)
{
    LOG_DEBUG(("[FILE_SELECT] FileSelectResetKeyConfigDefaults_8c0188bc\n"));

    var_progress_8c1ba1cc.btnConfigManual_0xcc = 0;
    var_progress_8c1ba1cc.btnConfigAuto_0xcd = 0;
    var_progress_8c1ba1cc.btnConfigWheelManual_0xce = 0;
    var_progress_8c1ba1cc.btnConfigWheelAuto_0xcf = 0;
    var_progress_8c1ba1cc.accelSensitivity_0xd0 = 0x10;
    var_progress_8c1ba1cc.brakeSensitivity_0xd1 = 0x10;
}

void FileSelectResetSoundDefaults_8c0188dc(void)
{
    LOG_DEBUG(("[FILE_SELECT] FileSelectResetSoundDefaults_8c0188dc\n"));

    var_soundMode_8c226070 = SndGetSoundMode_8c010924();
    if (var_soundMode_8c226070 < 0) {
        var_soundMode_8c226070 = 0;
    }
    var_progress_8c1ba1cc.musicVolume_0xd4 = 9;
    var_progress_8c1ba1cc.sfxVolume_0xd5 = 5;
    var_progress_8c1ba1cc.voiceVolume_0xd6 = 9;
}

void FileSelectResetProgress_8c01890a(void)
{
    int i;

    LOG_DEBUG(("[FILE_SELECT] FileSelectResetProgress_8c01890a\n"));

    var_progress_8c1ba1cc.days_0x00 = 1;
    for (i = 0; i < 5; i++) {
        var_progress_8c1ba1cc.eventProgressFlags_0x04[i] = 0;
    }
    for (i = 0; i < 9; i++) {
        var_progress_8c1ba1cc.courses_0x44[i].unlocked_0x00 = 0;
        var_progress_8c1ba1cc.courses_0x44[i].everPlayed_0x02 = 0;
        var_progress_8c1ba1cc.courses_0x44[i].storyAward_0x03 = 0;
    }
    var_progress_8c1ba1cc.courses_0x44[0].unlocked_0x00 = 1;
    var_progress_8c1ba1cc.courses_0x44[6].unlocked_0x00 = 1;
    var_progress_8c1ba1cc.exp_0x90 = 0;
    var_runReportPending_8c1bb8b8 = 1;
    var_runWasPractice_8c1bb8bc = 0;
}

/* FileSelectResetProgress_8c01890a, plus the fields only a new game clears. */
void FileSelectResetNewGame_8c01895e(void)
{
    int i;

    LOG_DEBUG(("[FILE_SELECT] FileSelectResetNewGame_8c01895e\n"));

    FileSelectResetProgress_8c01890a();
    for (i = 0; i < 5; i++) {
        var_progress_8c1ba1cc.profileProgressFlags_0x18[i] = 0;
    }
    for (i = 0; i < 6; i++) {
        var_progress_8c1ba1cc.letters_0x2c[i] = 0;
    }
    for (i = 0; i < 9; i++) {
        var_progress_8c1ba1cc.courses_0x44[i].new_0x01 = 0;
        var_progress_8c1ba1cc.courses_0x44[i].freeRunAward_0x04 = 0;
    }
    var_progress_8c1ba1cc.courses_0x44[0].new_0x01 = 1;
    var_progress_8c1ba1cc.courses_0x44[6].new_0x01 = 1;
    var_progress_8c1ba1cc.profileUnlockedCount_0x8c = 0;
    var_progress_8c1ba1cc.field_0x94 = 0;
    for (i = 0; i < 11; i++) {
        var_progress_8c1ba1cc.practiceLessonBestScores_0x98[i] = 0;
    }
}

void FileSelectResetOptionDefaults_8c0189d2(void)
{
    LOG_DEBUG(("[FILE_SELECT] FileSelectResetOptionDefaults_8c0189d2\n"));

    FileSelectResetSettingDefaults_8c018862();
    FileSelectResetKeyConfigDefaults_8c0188bc();
    FileSelectResetSoundDefaults_8c0188dc();
}

void FileSelectApplySoundSettings_8c0189fc(void)
{
    LOG_DEBUG(("[FILE_SELECT] FileSelectApplySoundSettings_8c0189fc\n"));

    SndSetAdxVol_8c010972(var_progress_8c1ba1cc.musicVolume_0xd4, 0);
    SndSetMidiVol_8c0109f4(var_progress_8c1ba1cc.sfxVolume_0xd5);
    SndSetAdxVol_8c010972(var_progress_8c1ba1cc.voiceVolume_0xd6, 1);
}

STATIC void buildFileList_8c018a22(void)
{
    int dst;
    int i;

    var_fileCardCount_8c226014 = 0;
    if (var_vmuStatus_8c226048[var_selectedVm_8c1ba34c] == VMU_STATUS_SAVING_POSSIBLE ||
        (var_vmuStatus_8c226048[var_selectedVm_8c1ba34c] == VMU_STATUS_SAVE_EXISTS &&
         var_loadedSaveCount_8c22600c < 10)) {
        var_fileCards_8c226018[0] = FILE_CARD_NEW;
        var_fileCardCount_8c226014 = 1;
    }
    dst = var_fileCardCount_8c226014;
    for (i = 0; i < var_loadedSaveCount_8c22600c; i++) {
        var_fileCards_8c226018[dst] = var_loadedSaveSlots_8c225fe4[i];
        dst++;
    }
    for (i = dst; i < 12; i++) {
        var_fileCards_8c226018[i] = FILE_CARD_EMPTY;
    }
    var_fileCardCount_8c226014 += var_loadedSaveCount_8c22600c;

    LOG_DEBUG(("[FILE_SELECT] buildFileList_8c018a22: built file list (%d cards)\n", var_fileCardCount_8c226014));
}

/* Draws value right to left from (x, y). */
STATIC void drawNumber_8c018aa2(int value, float x, float y)
{
    do {
        SpriteDraw_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupA_0x00,
            15 + value % 10, // digit glyphs start at 15
            x,
            y,
            -4.0
        );
        x -= 10.0;
    } while (value /= 10);
}

/*
 * Draws the save at var_saveBufCursor_8c225fe0, then advances it to the next
 * image. The award grid below is buggy in the original: every counter is
 * decremented even on the branch that did not match, so the first silver or
 * bronze sends the gold count negative and the rest of the grid floods with
 * gold marks.
 */
STATIC void drawFileCard_8c018b4c(int kind, float x)
{
    PlayerProgress *save;
    int cnt1;
    int cnt2;
    int cnt3;
    int i;
    float y;
    float xoff;

    if (kind == FILE_CARD_NEW) {
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0x13, x, 0.0, -4.0);
        return;
    }

    save = var_saveBufCursor_8c225fe0;

    drawNumber_8c018aa2(save->days_0x00, x + (save->days_0x00 >= 10 ? 63.0 : 52.0), 122.0);
    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00,
        6 + (save->days_0x00 + 1) % 7,
        x + 84.0, 122.0, -4.0);

    cnt1 = 0;
    cnt2 = 0;
    cnt3 = 0;
    for (i = 0; i < 9; i++) {
        if (save->courses_0x44[i].unlocked_0x00 != 0) {
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0x26 + i, x, 0.0, -4.0);
        }
        switch (save->courses_0x44[i].storyAward_0x03) {
        case AWARD_TIER_GOLD:   cnt3++; break;
        case AWARD_TIER_SILVER: cnt2++; break;
        case AWARD_TIER_BRONZE: cnt1++; break;
        }
    }

    drawNumber_8c018aa2(save->profileUnlockedCount_0x8c, x + 77.0, 244.0);
    drawNumber_8c018aa2(save->exp_0x90, x + 77.0, 264.0);

    y = 279.0;
    while (y <= 303.0) {
        xoff = 13.0;
        while (1) {
            int glyph;
            if (cnt3-- != 0) {
                glyph = 0x23;
            } else if (cnt2-- != 0) {
                glyph = 0x24;
            } else if (cnt1-- != 0) {
                glyph = 0x25;
            } else {
                y = 65536.0;
                break;
            }
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, glyph, x + xoff, y, -4.5);
            xoff += 24.0;
            if (xoff > 109.0) {
                break;
            }
        }
        y += 24.0;
    }

    SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0x12, x, 0.0, -4.5);
    var_saveBufCursor_8c225fe0 = (char *)var_saveBufCursor_8c225fe0 + 0x600;
}

/* A page shows three cards; cursorCol_0x3c is the index of the leftmost one. */
STATIC void drawFileSelect_8c018d46(void)
{
    int i;
    float x;
    void *dst;

    if (var_fileCards_8c226018[0] == FILE_CARD_NEW) {
        dst = var_saveBuf_8c1ba2e0;
        if (var_menuState_8c1bc7a8.cursorCol_0x3c != 0) {
            dst = (char *)var_saveBuf_8c1ba2e0 + (var_menuState_8c1bc7a8.cursorCol_0x3c - 1) * 0x600;
        }
    } else {
        dst = (char *)var_saveBuf_8c1ba2e0 + var_menuState_8c1bc7a8.cursorCol_0x3c * 0x600;
    }
    var_saveBufCursor_8c225fe0 = dst;

    x = 55.0;
    for (i = var_menuState_8c1bc7a8.cursorCol_0x3c;
         x <= 419.0 && var_fileCards_8c226018[i] != FILE_CARD_EMPTY;
         i++) {
        drawFileCard_8c018b4c(var_fileCards_8c226018[i], x);
        x += 182.0;
    }

    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0x2f,
        182.0 * (float)var_menuState_8c1bc7a8.selected_0x38 + 45.0, 0.0, -3.0);

    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_menuState_8c1bc7a8.cursorCol_0x3c != 0 ? 0x16 : 0x15,
        0.0, 0.0, -3.0);

    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_fileCards_8c226018[var_menuState_8c1bc7a8.cursorCol_0x3c + 3] == FILE_CARD_EMPTY ? 0x17 : 0x18,
        0.0, 0.0, -3.0);

    SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0x14, 0.0, 0.0, -4.0);
    SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 1, 0.0, 0.0, -4.3);
    SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 0, 0.0, 0.0, -5.0);
}

STATIC void fileSelectTask_8c018e7e(Task *task)
{
    unsigned int press = var_peripherals_8c1ba35c[0].press;
    int i;

    switch (var_menuState_8c1bc7a8.state_0x18) {
    case FILE_SELECT_STATE_LOADING:
        if (var_saveLoadResult_8c226010 == SAVE_LOAD_RUNNING) {
            MessageBoxDrawText_8c02af1c(0xff);
        } else if (var_saveLoadResult_8c226010 == SAVE_LOAD_DONE) {
            var_vmBusy_8c157a7c = 0;
            VmGameSetLcdAnim_8c01c8fc(0);
            var_saveBufCursor_8c225fe0 = var_saveBuf_8c1ba2e0;
            for (i = 0; i < var_loadedSaveCount_8c22600c; i++) {
                if (!FileSelectIsSaveValid_8c018804((int *)var_saveBufCursor_8c225fe0)) {
                    LOG_WARN(("[FILE_SELECT] fileSelectTask_8c018e7e: save %d failed validation\n", i));
                    FileSelectFreeBuffers_8c0187d0();
                    MessageBoxSetText_8c02aefc(MSG_LOAD_FAIL);
                    CHANGE_STATE(FILE_SELECT_STATE_LOAD_ERROR);
                    break;
                }
                var_saveBufCursor_8c225fe0 = (char *)var_saveBufCursor_8c225fe0 + 0x600;
            }
            if (i >= var_loadedSaveCount_8c22600c) {
                buildFileList_8c018a22();
                CHANGE_STATE(FILE_SELECT_STATE_READY);
            }
        } else if (var_saveLoadResult_8c226010 == SAVE_LOAD_FAILED) {
            LOG_WARN(("[FILE_SELECT] fileSelectTask_8c018e7e: file load failed\n"));
            var_vmBusy_8c157a7c = 0;
            FileSelectFreeBuffers_8c0187d0();
            MessageBoxSetText_8c02aefc(MSG_LOAD_FAIL);
            CHANGE_STATE(FILE_SELECT_STATE_LOAD_ERROR);
            VmGameSetLcdAnim_8c01c8fc(0);
        }
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 1, 0.0, 0.0, -4.3);
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 0, 0.0, 0.0, -5.0);
        break;

    case FILE_SELECT_STATE_LOAD_ERROR:
        if (press & PDD_DGT_TA) {
            CHANGE_STATE(FILE_SELECT_STATE_ERROR_FADE_OUT);
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            RenderStartFadeOut_8c022b60(10);
            return;
        }
        MessageBoxDrawText_8c02af1c(0xff);
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 1, 0.0, 0.0, -4.3);
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 0, 0.0, 0.0, -5.0);
        break;

    case FILE_SELECT_STATE_ERROR_FADE_OUT:
        if (var_isFading_8c226568 == 0) {
            VmSelectEnter_8c019e44(task);
            return;
        }
        MessageBoxDrawText_8c02af1c(0xff);
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 1, 0.0, 0.0, -4.3);
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 0, 0.0, 0.0, -5.0);
        break;

    case FILE_SELECT_STATE_READY:
        if (press & PDD_DGT_KL) {
            if (var_menuState_8c1bc7a8.selected_0x38 != 0) {
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                var_menuState_8c1bc7a8.selected_0x38--;
            } else if (var_menuState_8c1bc7a8.cursorCol_0x3c > 0) {
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                var_menuState_8c1bc7a8.cursorCol_0x3c--;
            }
        } else if (press & PDD_DGT_KR) {
            if (var_menuState_8c1bc7a8.selected_0x38 >= 2) {
                if (var_fileCards_8c226018[var_menuState_8c1bc7a8.cursorCol_0x3c + 3] != FILE_CARD_EMPTY) {
                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                    var_menuState_8c1bc7a8.cursorCol_0x3c++;
                }
            } else if (var_fileCards_8c226018[var_menuState_8c1bc7a8.cursorCol_0x3c +
                                    var_menuState_8c1bc7a8.selected_0x38 + 1] != FILE_CARD_EMPTY) {
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                var_menuState_8c1bc7a8.selected_0x38++;
            }
        } else if (press & PDD_DGT_TB) {
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
            CHANGE_STATE(FILE_SELECT_STATE_CANCEL_FADE_OUT);
            RenderStartFadeOut_8c022b60(10);
        } else if (press & PDD_DGT_TA) {
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            if (var_fileCards_8c226018[var_menuState_8c1bc7a8.cursorCol_0x3c +
                             var_menuState_8c1bc7a8.selected_0x38] == FILE_CARD_NEW) {
                MessageBoxSetText_8c02aefc(MSG_CONFIRM_NEW_FILE);
            } else {
                MessageBoxSetText_8c02aefc(MSG_CONFIRM_FILE);
            }
            var_menuState_8c1bc7a8.cursorRow_0x40 = 0;
            CHANGE_STATE(FILE_SELECT_STATE_CONFIRM);
        }
        drawFileSelect_8c018d46();
        break;

    case FILE_SELECT_STATE_CONFIRM:
        if (press & PDD_DGT_TA) {
            int cardValue;

            CHANGE_STATE(FILE_SELECT_STATE_CONFIRM_FADE_OUT);
            cardValue = var_fileCards_8c226018[var_menuState_8c1bc7a8.cursorCol_0x3c +
                                     var_menuState_8c1bc7a8.selected_0x38];
            if (cardValue == FILE_CARD_NEW) {
                int slot = 0;
                int idx = 1;

                LOG_DEBUG(("[FILE_SELECT] fileSelectTask_8c018e7e: creating new file\n"));
                FileSelectResetNewGame_8c01895e();
                /* The cards after the NEW FILE one are the VMU file indices in
                 * order, so the first gap is the lowest free index. */
                while (idx < var_fileCardCount_8c226014 && var_fileCards_8c226018[idx] == slot) {
                    slot++;
                    idx++;
                }
                var_saveSlot_8c1ba350 = slot;
            } else {
                int idx = 0;

                LOG_DEBUG(("[FILE_SELECT] fileSelectTask_8c018e7e: loading file (slot %d)\n", cardValue));
                while (var_loadedSaveSlots_8c225fe4[idx] != cardValue) {
                    idx++;
                }
                njMemCopy(&var_progress_8c1ba1cc, (char *)var_saveBuf_8c1ba2e0 + idx * 0x600, sizeof(PlayerProgress));
                SystemMenuApplyLoadedProgress_8c01b19c();
                var_saveSlot_8c1ba350 = cardValue;
                FileSelectApplySoundSettings_8c0189fc();
            }
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            SndStartAdxFadeOut_8c010bae(0);
            SndStartAdxFadeOut_8c010bae(1);
            RenderStartFadeOut_8c022b60(10);
        } else if (press & PDD_DGT_TB) {
            CHANGE_STATE(FILE_SELECT_STATE_READY);
            MessageBoxSetText_8c02aefc("");
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
        }
        drawFileSelect_8c018d46();
        MessageBoxDrawText_8c02af1c(0xff);
        break;

    case FILE_SELECT_STATE_CONFIRM_FADE_OUT:
        if (var_isFading_8c226568 != 0) {
            drawFileSelect_8c018d46();
            break;
        }
        FileSelectFreeBuffers_8c0187d0();
        VmSelectUnmountAll_8c0194de();
        CHANGE_STATE(FILE_SELECT_STATE_UNMOUNT_TO_MAIN);
        break;

    case FILE_SELECT_STATE_UNMOUNT_TO_MAIN:
        if (var_vmMountBusy_8c22606c != 0 || init_adxPlaying_8c03bd80 != 0) {
            break;
        }
        var_currentSysResGroupInfo_8c225fb0 = (void *)-1;
        MainMenuEnter_8c01a09a(task, 0);
        return;

    case FILE_SELECT_STATE_CANCEL_FADE_OUT:
        if (var_isFading_8c226568 != 0) {
            drawFileSelect_8c018d46();
            break;
        }
        FileSelectFreeBuffers_8c0187d0();
        VmSelectEnter_8c019e44(task);
        return;

    case FILE_SELECT_STATE_UNMOUNT_TO_VM:
        if (var_vmMountBusy_8c22606c != 0) {
            break;
        }
        VmSelectEnter_8c019e44(task);
        return;
    }
}

void FileSelectEnter_8c019334(Task *task)
{
    TaskReinstall_8c014b3e(task, fileSelectTask_8c018e7e);
    var_loadedSaveCount_8c22600c = 0;
    if (var_vmuStatus_8c226048[var_selectedVm_8c1ba34c] == VMU_STATUS_SAVE_EXISTS_NO_SPACE ||
        var_vmuStatus_8c226048[var_selectedVm_8c1ba34c] == VMU_STATUS_SAVE_EXISTS) {
        CHANGE_STATE(FILE_SELECT_STATE_LOADING);
        MessageBoxSetText_8c02aefc(MSG_LOADING_NO_POWER_OFF);
        VmGameSetLcdAnim_8c01c8fc(1);
        startVmLoad_8c018784();
        var_vmBusy_8c157a7c = 1;
    } else {
        buildFileList_8c018a22();
        CHANGE_STATE(FILE_SELECT_STATE_READY);
    }
    var_menuState_8c1bc7a8.cursorCol_0x3c = 0;
    var_menuState_8c1bc7a8.selected_0x38 = 0;
    RenderStartFadeIn_8c022a9c(10);
}
