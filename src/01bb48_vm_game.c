/* @unit VmGame */
#include <shinobi.h>
#include "includes.h" /* STATIC */
#include "01bb48_vm_game.h"
#include "011120_asset_queues.h"
#include "013ae8_route.h"
#include "014a9c_tasks.h"
#include "014b8c_backup.h"
#include "016c58_prompt.h"
#include "016d2c_course_menu.h"
#include "0193c8_vm_select.h"
#include "019e98_main_menu.h"
#include "01c980_profile_file.h"
#include "022464_render.h"
#include "0100bc_sound.h"
#include "02a9fc_message_box.h"
#include "235ca0_nj_buffers.h"
#include "014f54_sprite.h"
#include "1ba1c8_globals.h"
#include "strings.h"

/* =================
 * Type Declarations
 * =================
 */

enum State {
    STATE_INIT                = 0,
    STATE_MENU_FADE_IN        = 1,
    STATE_MENU                = 2,
    STATE_MENU_FADE_OUT       = 3,
    STATE_SELECT_ENTER        = 4,
    STATE_SELECT              = 5,
    STATE_SELECT_CURSOR_MOVE  = 6,
    STATE_DOWNLOAD            = 7,
    STATE_EXP_LOAD            = 8,
    STATE_NO_VM               = 9,
    STATE_OP_COMPLETE         = 11,
    STATE_RETURN_FADE_OUT     = 12,
    STATE_EXIT                = 13
};

enum MenuItem {
    MENU_DOWNLOAD = 0,
    MENU_EXP_LOAD = 1,
    MENU_EXIT     = 2
};

enum DownloadPhase {
    DOWNLOAD_PHASE_PROMPT = 0,
    DOWNLOAD_PHASE_SAVING = 1,
    DOWNLOAD_PHASE_DEFRAG_SAVING = 2
};

enum ExpPhase {
    EXP_PHASE_PROMPT    = 0,
    EXP_PHASE_LOADING   = 1,
    EXP_PHASE_REWRITING = 2
};

enum VmGameSaveResult {
    SAVE_STARTED      = 0,
    SAVE_ERR_OTHER    = 10,
    SAVE_ERR_EXISTS   = 11,
    SAVE_ERR_WRITE    = 12,
    SAVE_ERR_UNFORMAT = 13,
    SAVE_ERR_BUSY     = 14,
    SAVE_ERR_NO_CARD  = 15,
    SAVE_NEEDS_DEFRAG = 16,
    SAVE_ERR_FULL     = 17
};

typedef struct {
    TaskAction action;
    void *state;
    /* DOWNLOAD_PHASE_* or EXP_PHASE_*, by state */
    int phase_0x08;
    void *field_0x0c;
    int field_0x10;
    int field_0x14;
    void *queuedItem_0x18;
    int field_0x1c;
} VmGameTask;

/* ====================
 * Non-initialized Globals
 * ====================
 */

int var_lcdAnimActive_8c2260a8;
LcdAnim var_lcdAnimBus_8c2260ac;
LcdAnim var_lcdAnimDanger_8c2260b8;
LcdAnim var_lcdAnimLoading_8c2260c4;
STATIC enum VmGameBupPhase var_bupPhase_8c2260d0;
STATIC char var_lcdClearBuf_8c2260d4[0xc4]; // 48x32 1bpp
STATIC Uint32 var_lcdFrameCounter_8c226198;
STATIC LcdFrame *var_lcdFramePtr_8c22619c;
STATIC char var_defragBuf_8c2261a0[512];
STATIC int var_lcdAnimIdx_8c2263a0;

/* ===================
 * Initialized Globals
 * ===================
 */

LcdAnim *init_lcdAnimTable_8c044e38[] = {
    &var_lcdAnimBus_8c2260ac,
    &var_lcdAnimDanger_8c2260b8,
    &var_lcdAnimLoading_8c2260c4,
    NULL
};

char *init_vmuProbeNames_8c044e48[] = {
    "TOKYOBUS._VM",
    ""
};

/* 2 x 4 vm slot cursor targets */
NJS_POINT2 init_slotCursorTargets_8c044e50[8] = {
    {186.f,  98.f},
    {256.f,  98.f},
    {326.f,  98.f},
    {396.f,  98.f},

    {186.f, 194.f},
    {256.f, 194.f},
    {326.f, 194.f},
    {396.f, 194.f}
};

ResourceGroupInfo init_vmGameResgrp_8c044e90 = {
    /* parts     */  "vms_parts.dat",
    /* dat       */  "vms.dat",
    /* pvm       */  "vms.pvm",
    /* tex_count */  4
};

/* ====================
 * Functions
 * ====================
 */

STATIC void advanceLcdAnim_8c01bb48(LcdAnim *anim, int slot)
{
    unsigned char frameCount;
    int idx;

    switch (slot) {
        case 0: slot = PDD_PORT_A1; break;
        case 1: slot = PDD_PORT_A2; break;
        case 2: slot = PDD_PORT_B1; break;
        case 3: slot = PDD_PORT_B2; break;
        case 4: slot = PDD_PORT_C1; break;
        case 5: slot = PDD_PORT_C2; break;
        case 6: slot = PDD_PORT_D1; break;
        case 7: slot = PDD_PORT_D2; break;
    }

    if (!pdVmsLcdIsReady(slot))
        return;

    /* Pass NULL to clear the LCD. */
    if (anim == NULL) {
        memset(var_lcdClearBuf_8c2260d4, 0, 0xc0);
        pdVmsLcdWrite1(slot, var_lcdClearBuf_8c2260d4);
        return;
    }

    if (!var_lcdAnimActive_8c2260a8)
        return;

    frameCount = anim->frames->count;
    var_lcdFramePtr_8c22619c = &anim->frames->data[anim->frameIdx];
    pdVmsLcdWrite(slot, var_lcdFramePtr_8c22619c, 3);

    if (var_lcdFrameCounter_8c226198 < anim->delay) {
        var_lcdFrameCounter_8c226198++;
        return;
    }

    var_lcdFrameCounter_8c226198 = 0;

    if (++anim->frameIdx >= frameCount) {
        anim->frameIdx = 0;
    }
}

/* anim is not used. */
STATIC int pollBupOp_8c01bc44(LcdAnim *anim, int drive)
{
    int stat;

    while (1) {
        int op;
        const BACKUPINFO *info = BupGetInfo_8c014bba(drive);
        stat = buStat(drive);

        if (stat != BUD_STAT_BUSY)
            break;

        op = info->Operation;

        if (op == BUD_OP_NOP)
            continue;

        switch (op) {
            case BUD_OP_SAVEEXECFILE:
                var_bupPhase_8c2260d0 = VMGAME_BUP_SAVE_BUSY;
                break;
            case BUD_OP_DEFRAGDISK:
                var_bupPhase_8c2260d0 = VMGAME_BUP_DEFRAG_BUSY;
                break;
            case BUD_OP_LOADFILEEX:
                var_bupPhase_8c2260d0 = VMGAME_BUP_LOAD_BUSY;
                break;
            case BUD_OP_REWRITEEXECFILE:
                var_bupPhase_8c2260d0 = VMGAME_BUP_REWRITE_BUSY;
                break;
        }
        return var_bupPhase_8c2260d0;
    }

    if (stat != BUD_STAT_READY || var_bupPhase_8c2260d0 == VMGAME_BUP_ERROR) {
        return var_bupPhase_8c2260d0 = VMGAME_BUP_IDLE;
    }

    switch (var_bupPhase_8c2260d0) {
        case VMGAME_BUP_SAVE_BUSY:
            if (buGetLastError(drive) != BUD_ERR_OK)
                return var_bupPhase_8c2260d0 = VMGAME_BUP_ERROR;

            return var_bupPhase_8c2260d0 = VMGAME_BUP_SAVE_DONE;

        case VMGAME_BUP_DEFRAG_BUSY:
            if (buGetLastError(drive) != BUD_ERR_OK)
                return var_bupPhase_8c2260d0 = VMGAME_BUP_ERROR;

            return var_bupPhase_8c2260d0 = VMGAME_BUP_DEFRAG_DONE;

        case VMGAME_BUP_LOAD_BUSY:
            if (buGetLastError(drive) != BUD_ERR_OK)
                return var_bupPhase_8c2260d0 = VMGAME_BUP_ERROR;

            return var_bupPhase_8c2260d0 = VMGAME_BUP_LOAD_DONE;

        case VMGAME_BUP_REWRITE_BUSY:
            if (buGetLastError(drive) != BUD_ERR_OK)
                return var_bupPhase_8c2260d0 = VMGAME_BUP_ERROR;

            return var_bupPhase_8c2260d0 = VMGAME_BUP_REWRITE_DONE;

        case VMGAME_BUP_SAVE_DONE:
        case VMGAME_BUP_DEFRAG_DONE:
        case VMGAME_BUP_LOAD_DONE:
        case VMGAME_BUP_REWRITE_DONE:
            return var_bupPhase_8c2260d0 = VMGAME_BUP_IDLE;
    }

    return var_bupPhase_8c2260d0;
}

STATIC enum VmGameSaveResult saveExecFile_8c01bd30(
    void *buf, const char *fname, int nblock, int drive
)
{
    int result;
    char findbuf[16];

    var_bupPhase_8c2260d0 = VMGAME_BUP_IDLE;
    result = buFindExecFile(drive, findbuf);

    while (1) {
        SYS_RTC_DATE rtc;
        Sint32 flag = BUD_FLAG_VERIFY | BUD_FLAG_COPYDISABLE;

        if (result == BUD_ERR_OK)
            return SAVE_ERR_EXISTS;
        if (result != BUD_ERR_FILE_NOT_FOUND)
            break;

        result = buGetDiskFree(drive, 1);
        if (result < 0)
            continue;

        if (result < nblock) {
            if (buGetDiskFree(drive, 0) < nblock) {
                return SAVE_ERR_FULL;
            }
            return SAVE_NEEDS_DEFRAG;
        }

        syRtcGetDate(&rtc);
        result = buSaveExecFile(
            drive, fname, buf, nblock, (BUS_TIME *)&rtc, flag
        );
        if (result != BUD_ERR_OK)
            return SAVE_ERR_WRITE;

        return SAVE_STARTED;
    }

    switch (result) {
        case BUD_ERR_UNFORMAT:
            return SAVE_ERR_UNFORMAT;
        case BUD_ERR_BUSY:
            return SAVE_ERR_BUSY;
        case BUD_ERR_NO_DISK:
            return SAVE_ERR_NO_CARD;
    }

    return SAVE_ERR_OTHER;
}

STATIC int defragDisk_8c01bde4(int drive)
{
    var_bupPhase_8c2260d0 = VMGAME_BUP_IDLE;

    if (buDefragDisk(drive, &var_defragBuf_8c2261a0) != BUD_ERR_OK)
        return -1;
    return 0;
}

STATIC int loadFileEx_8c01be30(
    int drive, const char *fname, void *buf, Uint32 start, Uint32 nblock
)
{
    var_bupPhase_8c2260d0 = VMGAME_BUP_IDLE;
    if (buLoadFileEx(drive, fname, buf, start, nblock) != BUD_ERR_OK)
        return -1;

    return 0;
}

STATIC int rewriteExecFile_8c01be60(
    int drive, const char *fname, const void *buf, Uint32 start, Uint32 nblock
)
{
    var_bupPhase_8c2260d0 = VMGAME_BUP_IDLE;
    if (buRewriteExecFile(drive, fname, buf, start, nblock) != BUD_ERR_OK)
        return -1;

    return 0;
}

STATIC void drawSelectScreen_8c01be90(void)
{
    MenuState *m = &var_menuState_8c1bc7a8;
    int i;

    if (m->state_0x18 >= STATE_SELECT
        && m->state_0x18 != STATE_NO_VM
    ) {
        SpriteDraw_8c014f54(
            &var_resourceGroup_8c2263a8, 9,
            m->pos.vmSelect.cursor_0x20.x,
            m->pos.vmSelect.cursor_0x20.y, -4.0f
        );
    }

    for (i = 0; i < 8; i++) {
        if (var_vmuStatus_8c226048[i] == VMU_STATUS_NOT_CONNECTED)
            continue;

        SpriteDraw_8c014f54(&var_resourceGroup_8c2263a8, i + 1, 0.0f, 0.0f, -5.0f);
    }

    SpriteDraw_8c014f54(&var_resourceGroup_8c2263a8, 0, 0.0f, 0.0f, -6.0f);
    SpriteDraw_8c014f54(&m->resourceGroupA_0x00, 1, 0.0f, 0.0f, -4.3f);
    SpriteDraw_8c014f54(&m->resourceGroupA_0x00, 0, 0.0f, 0.0f, -7.0f);
}

/* In EXP load mode, also says whether the slot holds the quiz. */
STATIC void selectSlot_8c01bf2a(int slot)
{
    MenuState *m = &var_menuState_8c1bc7a8;
    NJS_POINT2 *cursorTarget = &m->pos.vmSelect.cursorTarget_0x28;

    cursorTarget->x = init_slotCursorTargets_8c044e50[slot].x;
    cursorTarget->y = init_slotCursorTargets_8c044e50[slot].y;

    m->cursorVelocity_0x30.x =
    (cursorTarget->x - m->pos.vmSelect.cursor_0x20.x) / 6.0f;
    m->cursorVelocity_0x30.y =
    (cursorTarget->y - m->pos.vmSelect.cursor_0x20.y) / 6.0f;

    if (m->subState_0x1c != MENU_DOWNLOAD) {
        switch (var_vmuStatus_8c226048[slot]) {
            case VMU_STATUS_NOT_AVAILABLE:
            case VMU_STATUS_NOT_ENOUGH_SPACE:
            case VMU_STATUS_SAVING_POSSIBLE:
                MessageBoxSetText_8c02aefc(MSG_QUIZ_NONE);
                break;
            case VMU_STATUS_SAVE_EXISTS_NO_SPACE:
            case VMU_STATUS_SAVE_EXISTS:
                MessageBoxSetText_8c02aefc(MSG_QUIZ_EXISTS);
                break;
            default:
                MessageBoxSetText_8c02aefc("");
                break;
        }
    }
}

STATIC void vmGameTask_8c01bfec(VmGameTask *task)
{
    MenuState *m = &var_menuState_8c1bc7a8;
    int slot;

    VmSelectUpdateAllStatus_8c019550(init_vmuProbeNames_8c044e48, 45);
    slot = m->selected_0x38;

    switch (m->state_0x18) {
    case STATE_INIT:
        if (RouteGetLatch_8c01432a())
            return;

        m->state_0x18 = STATE_MENU_FADE_IN;
        AsqFreeQueues_8c011f7e();
        RenderStartFadeIn_8c022a9c(10);
        return;

    case STATE_MENU_FADE_IN:
        if (!var_isFading_8c226568) {
            m->state_0x18 = STATE_MENU;
        }
        SpriteDraw_8c014f54(
            &m->resourceGroupB_0x0c, 0x6c, 0.0f, 0.0f, -5.0f
        );
        SpriteDraw_8c014f54(&m->resourceGroupA_0x00, 0, 0.0f, 0.0f, -7.0f);
        break;

    case STATE_MENU:
        PromptHandleMultiple_8c016c58(&slot, 3);
        if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
            switch (slot) {
                case MENU_DOWNLOAD:
                    m->state_0x18 = STATE_MENU_FADE_OUT;
                    m->subState_0x1c = MENU_DOWNLOAD;
                    RenderStartFadeOut_8c022b60(10);
                    break;
                case MENU_EXP_LOAD:
                    m->state_0x18 = STATE_MENU_FADE_OUT;
                    m->subState_0x1c = MENU_EXP_LOAD;
                    RenderStartFadeOut_8c022b60(10);
                    break;
                case MENU_EXIT:
                    m->state_0x18 = STATE_EXIT;
                    VmSelectUnmountAll_8c0194de();
                    RenderStartFadeOut_8c022b60(10);
                    break;
            }
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
        } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TB) {
            m->state_0x18 = STATE_EXIT;
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
            VmSelectUnmountAll_8c0194de();
            RenderStartFadeOut_8c022b60(10);
        }
        SpriteDraw_8c014f54(
            &m->resourceGroupB_0x0c, slot + 109, 0.0f, 0.0f, -5.0f
        );
        SpriteDraw_8c014f54(&m->resourceGroupA_0x00, 0, 0.0f, 0.0f, -7.0f);
        break;

    case STATE_MENU_FADE_OUT:
        if (!var_isFading_8c226568) {
            m->state_0x18 = STATE_SELECT_ENTER;
            RenderStartFadeIn_8c022a9c(10);
            return;
        }
        SpriteDraw_8c014f54(
            &m->resourceGroupB_0x0c, slot + 109, 0.0f, 0.0f, -5.0f
        );
        SpriteDraw_8c014f54(&m->resourceGroupA_0x00, 0, 0.0f, 0.0f, -7.0f);
        break;

    case STATE_SELECT_ENTER:
        if (!var_isFading_8c226568) {
            slot = 0;
            while (var_vmuStatus_8c226048[slot] == VMU_STATUS_NOT_CONNECTED) {
                slot++;
            }

            // Reached the no-save slot: no VM is connected.
            if (var_vmuStatus_8c226048[slot] ==
                VMU_STATUS_PROCEED_WITHOUT_SAVING
            ) {
                m->state_0x18 = STATE_NO_VM;
                MessageBoxSetText_8c02aefc(MSG_VM_SET_PLEASE);
            } else {
                selectSlot_8c01bf2a(slot);
                m->pos.vmSelect.cursor_0x20 = m->pos.vmSelect.cursorTarget_0x28;
                m->state_0x18 = STATE_SELECT;
            }
        }
        drawSelectScreen_8c01be90();
        break;

    case STATE_SELECT: {
        int i;

        VmGameSetLcdAnim_8c01c8fc(0);

        i = 0;
        while (var_vmuStatus_8c226048[i] == VMU_STATUS_NOT_CONNECTED) {
            i++;
        }

        if (var_vmuStatus_8c226048[i] == VMU_STATUS_PROCEED_WITHOUT_SAVING) {
            m->state_0x18 = STATE_NO_VM;
            MessageBoxSetText_8c02aefc(MSG_VM_SET_PLEASE);
            drawSelectScreen_8c01be90();
            break;
        }

        if (var_vmuStatus_8c226048[slot] == VMU_STATUS_NOT_CONNECTED) {
            // Selected VM was unplugged.
            slot = 0;
            while (var_vmuStatus_8c226048[slot] == VMU_STATUS_NOT_CONNECTED) {
                slot++;
            }
        } else {
            // First row
            if (slot < 4) {
                if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KL) {
                    do {
                        if (--slot < 0) slot = 3;
                    } while (
                        var_vmuStatus_8c226048[slot] == VMU_STATUS_NOT_CONNECTED
                    );
                } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KR) {
                    do {
                        if (++slot > 3) slot = 0;
                    } while (
                        var_vmuStatus_8c226048[slot] == VMU_STATUS_NOT_CONNECTED
                    );
                } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KD) {
                    for (i = 4; i < 8; i++) {
                        if (var_vmuStatus_8c226048[i] == VMU_STATUS_NOT_CONNECTED)
                            continue;

                        slot += 4;
                        while (
                            var_vmuStatus_8c226048[slot] == VMU_STATUS_NOT_CONNECTED
                        ) {
                            if (++slot > 7) slot = 4;
                        }
                        break;
                    }
                }
            }
            // Second row
            else {
                if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KL) {
                    do {
                        if (--slot < 4) slot = 7;
                    } while (
                        var_vmuStatus_8c226048[slot] == VMU_STATUS_NOT_CONNECTED
                    );
                } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KR) {
                    do {
                        if (++slot > 7) slot = 4;
                    } while (
                        var_vmuStatus_8c226048[slot] == VMU_STATUS_NOT_CONNECTED
                    );
                } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KU) {
                    for (i = 0; i < 4; i++) {
                        if (var_vmuStatus_8c226048[i] == VMU_STATUS_NOT_CONNECTED)
                            continue;

                        slot -= 4;
                        while (
                            var_vmuStatus_8c226048[slot] == VMU_STATUS_NOT_CONNECTED
                        ) {
                            if (--slot < 0) slot = 3;
                        }
                        break;
                    }
                }
            }

            while (var_vmuStatus_8c226048[slot] == VMU_STATUS_NOT_CONNECTED) {
                // Unreachable
                // coverage:ignore-next-line
                if (++slot > 7) slot = 0;
            }

            if (slot == m->selected_0x38) {
                if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
                    if (m->subState_0x1c == MENU_DOWNLOAD) {
                        m->state_0x18 = STATE_DOWNLOAD;
                        MessageBoxSetText_8c02aefc(MSG_CONFIRM_DL_QUIZ);
                        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                    } else if (
                        var_vmuStatus_8c226048[slot] == VMU_STATUS_SAVE_EXISTS_NO_SPACE
                        || var_vmuStatus_8c226048[slot] ==
                            VMU_STATUS_SAVE_EXISTS
                    ) {
                        m->state_0x18 = STATE_EXP_LOAD;
                        MessageBoxSetText_8c02aefc(MSG_CONFIRM_ADD_POINTS);
                        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                    } else {
                        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                    }
                    m->selectedVmuSlot_0x6c = m->selected_0x38;
                    m->cursorCol_0x3c = 0;
                    task->phase_0x08 = 0;
                } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TB) {
                    m->state_0x18 = STATE_RETURN_FADE_OUT;
                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
                    RenderStartFadeOut_8c022b60(10);
                }
                drawSelectScreen_8c01be90();
                break;
            }
        }

        selectSlot_8c01bf2a(slot);
        m->state_0x18 = STATE_SELECT_CURSOR_MOVE;
        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
        drawSelectScreen_8c01be90();
        break;
    }

    case STATE_SELECT_CURSOR_MOVE:
        if (CourseMenuInterpolateCursor_8c016d2c() != 0) {
            m->state_0x18 = STATE_SELECT;
        }
        drawSelectScreen_8c01be90();
        break;

    case STATE_DOWNLOAD: {
        switch (task->phase_0x08) {
            case DOWNLOAD_PHASE_PROMPT: {
                switch (PromptHandleBinary_8c016caa(&m->cursorCol_0x3c)) {
                    case 1: {
                        int save = saveExecFile_8c01bd30(
                            var_vmGameBuf_8c1bc454,
                            "TOKYOBUS._VM",
                            45,
                            m->selectedVmuSlot_0x6c
                        );

                        switch (save) {
                            case SAVE_STARTED: {
                                VmGameSetLcdAnim_8c01c8fc(1);
                                task->phase_0x08 = DOWNLOAD_PHASE_SAVING;
                                MessageBoxSetText_8c02aefc(
                                    MSG_DOWNLOADING_NO_REMOVE
                                );
                                var_vmBusy_8c157a7c = 1;
                                break;
                            }

                            // TODO: Merge SAVE_* and
                            // VMGAME_BUP_* into a single enum
                            case -1:
                            case SAVE_ERR_OTHER:
                            case SAVE_ERR_WRITE:
                            case SAVE_ERR_UNFORMAT:
                            case SAVE_ERR_BUSY:
                            case SAVE_ERR_NO_CARD: {
                                m->state_0x18 = STATE_SELECT;
                                MessageBoxSetText_8c02aefc(MSG_DL_FAIL);
                                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                                break;
                            }

                            case SAVE_NEEDS_DEFRAG: {
                                if (defragDisk_8c01bde4(m->selectedVmuSlot_0x6c) != 0) {
                                    m->state_0x18 = STATE_SELECT;
                                    MessageBoxSetText_8c02aefc(MSG_DL_FAIL);
                                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                                    break;
                                }

                                task->phase_0x08 = DOWNLOAD_PHASE_DEFRAG_SAVING;
                                MessageBoxSetText_8c02aefc(
                                    MSG_DOWNLOADING_NO_REMOVE
                                );
                                var_vmBusy_8c157a7c = 1;
                                break;
                            }

                            case SAVE_ERR_FULL: {
                                m->state_0x18 = STATE_SELECT;
                                MessageBoxSetText_8c02aefc(
                                    MSG_DL_NEED_45_BLOCKS
                                );
                                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                                break;
                            }

                            case SAVE_ERR_EXISTS: {
                                m->state_0x18 = STATE_SELECT;
                                MessageBoxSetText_8c02aefc(MSG_EXE_EXISTS);
                                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
                                break;
                            }
                        }

                        break;
                    }

                    case 2: {
                        MessageBoxSetText_8c02aefc("");
                        m->state_0x18 = STATE_SELECT;
                        break;
                    }
                }
                SpriteDraw_8c014f54(&m->resourceGroupA_0x00, m->cursorCol_0x3c + 2, 228.0f, 300.0f, -5.0f);
                break;
            }

            case DOWNLOAD_PHASE_SAVING: {
                int poll = pollBupOp_8c01bc44(
                    &var_lcdAnimDanger_8c2260b8, m->selectedVmuSlot_0x6c
                );
                switch (poll) {
                    case VMGAME_BUP_IDLE:
                    case VMGAME_BUP_SAVE_DONE:
                        m->state_0x18 = STATE_OP_COMPLETE;
                        MessageBoxSetText_8c02aefc(MSG_DL_DONE);
                        var_vmBusy_8c157a7c = 0;
                        break;
                    case VMGAME_BUP_ERROR:
                        m->state_0x18 = STATE_SELECT;
                        MessageBoxSetText_8c02aefc(MSG_DL_FAIL);
                        var_vmBusy_8c157a7c = 0;
                        break;
                }
                break;
            }

            case DOWNLOAD_PHASE_DEFRAG_SAVING: {
                int poll = pollBupOp_8c01bc44(&var_lcdAnimDanger_8c2260b8, m->selectedVmuSlot_0x6c);
                switch (poll) {
                    case VMGAME_BUP_IDLE:
                        saveExecFile_8c01bd30(
                            var_vmGameBuf_8c1bc454,
                            "TOKYOBUS._VM",
                            45,
                            m->selectedVmuSlot_0x6c
                        );
                        task->phase_0x08 = DOWNLOAD_PHASE_SAVING;
                        break;
                    case VMGAME_BUP_ERROR:
                        m->state_0x18 = STATE_SELECT;
                        MessageBoxSetText_8c02aefc(MSG_DL_FAIL);
                        var_vmBusy_8c157a7c = 0;
                        break;
                }
                break;
            }
        }
        drawSelectScreen_8c01be90();
        break;
    }

    case STATE_EXP_LOAD: {
        switch (task->phase_0x08) {
            case EXP_PHASE_PROMPT: {
                switch (PromptHandleBinary_8c016caa(&m->cursorCol_0x3c)) {
                    case 1: {
                        int load = loadFileEx_8c01be30(
                            m->selectedVmuSlot_0x6c, "TOKYOBUS._VM", var_texbuf_8c277ca0, 0, 6
                        );

                        if (load == -1) {
                            m->state_0x18 = STATE_SELECT;
                            MessageBoxSetText_8c02aefc(MSG_DL_FAIL);
                            break;
                        }

                        task->phase_0x08 = EXP_PHASE_LOADING;
                        MessageBoxSetText_8c02aefc(MSG_VM_NO_REMOVE);
                        VmGameSetLcdAnim_8c01c8fc(1);
                        var_vmBusy_8c157a7c = 1;
                        break;
                    }

                    case 2: {
                        MessageBoxSetText_8c02aefc("");
                        m->state_0x18 = STATE_SELECT;
                        break;
                    }
                }
                SpriteDraw_8c014f54(&m->resourceGroupA_0x00, m->cursorCol_0x3c + 2, 228.0f, 300.0f, -5.0f);
                break;
            }

            case EXP_PHASE_LOADING: {
                int poll = pollBupOp_8c01bc44(
                    &var_lcdAnimDanger_8c2260b8, m->selectedVmuSlot_0x6c
                );
                switch (poll) {
                    case VMGAME_BUP_IDLE:
                    case VMGAME_BUP_LOAD_DONE: {
                        int rewrite;

                        var_progress_8c1ba1cc.exp_0x90 +=
                            (short)var_texbuf_8c277ca0[0x900] * 100
                            + var_texbuf_8c277ca0[0x901];

                        if (var_progress_8c1ba1cc.exp_0x90 > 99999) {
                            var_progress_8c1ba1cc.exp_0x90 = 99999;
                        }

                        var_texbuf_8c277ca0[0x900] = 0;
                        var_texbuf_8c277ca0[0x901] = 0;
                        var_texbuf_8c277ca0[0x905] =
                            var_texbuf_8c277ca0[0x902]
                            ^ var_texbuf_8c277ca0[0x903]
                            ^ var_texbuf_8c277ca0[0x904];

                        rewrite = rewriteExecFile_8c01be60(
                            m->selectedVmuSlot_0x6c, "TOKYOBUS._VM", var_texbuf_8c277ca0, 0, 6
                        );

                        if (rewrite == -1) {
                            m->state_0x18 = STATE_SELECT;
                            MessageBoxSetText_8c02aefc(MSG_WRITE_FAIL);
                            var_vmBusy_8c157a7c = 0;
                            break;
                        }

                        task->phase_0x08 = EXP_PHASE_REWRITING;
                        break;
                    }

                    case VMGAME_BUP_ERROR: {
                        m->state_0x18 = STATE_SELECT;
                        MessageBoxSetText_8c02aefc(MSG_READ_FAIL);
                        var_vmBusy_8c157a7c = 0;
                        break;
                    }
                }
                break;
            }

            case EXP_PHASE_REWRITING: {
                int poll = pollBupOp_8c01bc44(
                    &var_lcdAnimDanger_8c2260b8, m->selectedVmuSlot_0x6c
                );
                switch (poll) {
                    case VMGAME_BUP_IDLE:
                    case VMGAME_BUP_SAVE_DONE:
                        m->state_0x18 = STATE_OP_COMPLETE;
                        MessageBoxSetText_8c02aefc(MSG_POINTS_ADDED);
                        var_vmBusy_8c157a7c = 0;
                        break;
                    case VMGAME_BUP_ERROR:
                        m->state_0x18 = STATE_SELECT;
                        MessageBoxSetText_8c02aefc(MSG_WRITE_FAIL);
                        var_vmBusy_8c157a7c = 0;
                        break;
                }
                break;
            }
        }
        drawSelectScreen_8c01be90();
        break;
    }

    case STATE_NO_VM: {
        slot = 0;
        while (var_vmuStatus_8c226048[slot] == VMU_STATUS_NOT_CONNECTED) {
            slot++;
        }

        if (var_vmuStatus_8c226048[slot] == VMU_STATUS_PROCEED_WITHOUT_SAVING) {
            if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
                m->state_0x18 = STATE_RETURN_FADE_OUT;
                MessageBoxSetText_8c02aefc("");
                RenderStartFadeOut_8c022b60(10);
            }
        } else {
            selectSlot_8c01bf2a(slot);
            m->pos.vmSelect.cursor_0x20 = m->pos.vmSelect.cursorTarget_0x28;
            m->state_0x18 = STATE_SELECT;
            MessageBoxSetText_8c02aefc("");
        }

        drawSelectScreen_8c01be90();
        break;
    }

    case STATE_OP_COMPLETE: {
        if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
            m->state_0x18 = STATE_RETURN_FADE_OUT;
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            RenderStartFadeOut_8c022b60(10);
        }

        VmGameSetLcdAnim_8c01c8fc(0);
        drawSelectScreen_8c01be90();
        break;
    }

    case STATE_RETURN_FADE_OUT: {
        if (var_isFading_8c226568 != 0)
            break;

        m->state_0x18 = STATE_MENU_FADE_IN;
        m->selected_0x38 = m->subState_0x1c;
        MessageBoxSetText_8c02aefc("");
        RenderStartFadeIn_8c022a9c(10);
        return;
    }

    case STATE_EXIT: {
        if (var_isFading_8c226568 != 0)
            break;
        if (var_vmMountBusy_8c22606c != 0)
            return;

        CourseMenuFreeResourceGroup_8c0185c4(&var_resourceGroup_8c2263a8);
        syFree(var_vmGameBuf_8c1bc454);
        var_vmGameBuf_8c1bc454 = (void *)0xffffffff;
        MainMenuEnter_8c01a09a(task, 3);
        return;
    }

    }

    MessageBoxDrawText_8c02af1c(0xff);
    SpriteDraw_8c014f54(&m->resourceGroupA_0x00, 1, 0.0f, 0.0f, -4.0f);
    m->selected_0x38 = slot;
}

void VmGameEnter_8c01c880(Task *task)
{
    TaskSwitch_8c014b3e(task, vmGameTask_8c01bfec);
    var_menuState_8c1bc7a8.state_0x18 = 0;
    var_menuState_8c1bc7a8.selected_0x38 = 0;

    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();

    CourseMenuRequestSysResgrp_8c018568(
        &var_resourceGroup_8c2263a8, &init_vmGameResgrp_8c044e90
    );
    AsqRequestDat_8c011182("\\SYSTEM", "PDAQUIZ.bin", &var_vmGameBuf_8c1bc454);
    RouteSetLatch_8c014330();
    AsqProcessQueues_8c011fe0(
        AsqNop_8c011120, 0, 0, 0, RouteClearLatch_8c014322
    );

    MessageBoxSetText_8c02aefc("");
}

void VmGameResetLcdAnims_8c01c8dc(void)
{
    var_lcdAnimBus_8c2260ac.delay = 10;
    var_lcdAnimBus_8c2260ac.frameIdx = 0;

    var_lcdAnimDanger_8c2260b8.delay = 10;
    var_lcdAnimDanger_8c2260b8.frameIdx = 0;

    var_lcdAnimLoading_8c2260c4.delay = 10;
    var_lcdAnimLoading_8c2260c4.frameIdx = 0;

    var_lcdAnimIdx_8c2263a0 = 0;
    var_lcdAnimActive_8c2260a8 = 0;
}

void VmGameSetLcdAnim_8c01c8fc(int idx)
{
    var_lcdAnimIdx_8c2263a0 = idx;
    /* -1 clears the LCD. */
    if (idx == -1) {
        advanceLcdAnim_8c01bb48(0, 0);
    }
}

void VmGameUpdateLcd_8c01c910(void)
{
    advanceLcdAnim_8c01bb48(
        init_lcdAnimTable_8c044e38[var_lcdAnimIdx_8c2263a0], 0
    );
}
