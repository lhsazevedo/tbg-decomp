/* @unit Ending */
#include <shinobi.h>

#include "01f3c0_ending.h"
#include "0100bc_sound.h"
#include "011120_asset_queues.h"
#include "012324_input.h"
#include "0129cc_game.h"
#include "013ae8_route.h"
#include "014a9c_tasks.h"
#include "014f54_sprite.h"
#include "015034_text.h"
#include "015ab8_title.h"
#include "01614c_replay_menu.h"
#include "016d2c_course_menu.h"
#include "01d7fc_results.h"
#include "022464_render.h"
#include "02a9fc_message_box.h"
#include "1ba1c8_globals.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "strings.h"

/* ====================
 * Type Declarations
 * ====================
 */

/* Ending-sequence phase, held in MenuState.state_0x18 (the generic state slot
 * every screen drives). */
enum ENDING_TASK_STATE {
    /* EndingStart_8c01f954 sets the pvm-ready flag before queueing the ending
     * assets and the load clears it, so this phase waits for a zero. */
    ENDING_TASK_STATE_WAIT_PVM = 0,
    ENDING_TASK_STATE_FADE_IN = 1,
    ENDING_TASK_STATE_INSTRUCTOR_DIALOG = 2,
    ENDING_TASK_STATE_FADE_OUT_TO_CREDITS = 3,
    ENDING_TASK_STATE_CREDITS_INTRO = 4,
    ENDING_TASK_STATE_CREDITS_SCROLL = 5,
    ENDING_TASK_STATE_CREDITS_HOLD = 6,
    ENDING_TASK_STATE_EXIT = 7,
};

/* ====================
 * Non-initialized Globals
 * ====================
 */

STATIC int *var_endingVoiceList_8c226430; /* selected ending voice-id list, set by selectEndingDialog_8c01f3c0 */

/* ====================
 * Initialized Globals
 * ====================
 */

/* One voice id per dialog page, walked by instructorDialogTask_8c016f98 via
 * voiceCuePtr_0x18 and terminated by 0. The four tiers line up with
 * init_instructorDialogs_8c044c08 entries INSTR_SUCCESS_PERFECT ..
 * INSTR_FAILURE_FINAL, which selectEndingDialog_8c01f3c0 picks in step. */
STATIC int init_endingVoicesPerfect_8c04522c[] = {
    0x51a, 0x51b, 0x51c, 0x51d, 0x51e, 0,
};

STATIC int init_endingVoicesHigh_8c045244[] = {
    0x51f, 0x520, 0x521, 0x522, 0x523, 0,
};

STATIC int init_endingVoicesNormal_8c04525c[] = {
    0x524, 0x525, 0x526, 0x527, 0x528, 0x529, 0,
};

STATIC int init_endingVoicesFailure_8c045278[] = {
    0x52a, 0x52b, 0x52c, 0x52d, 0,
};

/* Head and tail of one credit roll: laid out back to back in the original
 * binary (1 + 37 pointers, no gap), and scrollCreditsText_8c01f50e indexes
 * across the pair from the head as a single 38-entry table. */
STATIC const char *init_endingCreditsHead_8c04528c[] = {
    MSG_ENDING_CREDITS_01,
};

/* Terminated by an empty string (originally a pointer to a 4-byte zeroed
 * buffer), not a null pointer. */
STATIC const char *init_endingCreditsTail_8c045290[] = {
    MSG_ENDING_CREDITS_02, MSG_ENDING_CREDITS_03, MSG_ENDING_CREDITS_04, MSG_ENDING_CREDITS_05,
    MSG_ENDING_CREDITS_06, MSG_ENDING_CREDITS_07, MSG_ENDING_CREDITS_08, MSG_ENDING_CREDITS_09,
    MSG_ENDING_CREDITS_10, MSG_ENDING_CREDITS_11, MSG_ENDING_CREDITS_12, MSG_ENDING_CREDITS_13,
    MSG_ENDING_CREDITS_14, MSG_ENDING_CREDITS_15, MSG_ENDING_CREDITS_16, MSG_ENDING_CREDITS_17,
    MSG_ENDING_CREDITS_18, MSG_ENDING_CREDITS_19, MSG_ENDING_CREDITS_20, MSG_ENDING_CREDITS_21,
    MSG_ENDING_CREDITS_22, MSG_ENDING_CREDITS_23, MSG_ENDING_CREDITS_24, MSG_ENDING_CREDITS_25,
    MSG_ENDING_CREDITS_26, MSG_ENDING_CREDITS_27, MSG_ENDING_CREDITS_28, MSG_ENDING_CREDITS_29,
    MSG_ENDING_CREDITS_30, MSG_ENDING_CREDITS_31, MSG_ENDING_CREDITS_32, MSG_ENDING_CREDITS_33,
    MSG_ENDING_CREDITS_34, MSG_ENDING_CREDITS_35, MSG_ENDING_CREDITS_36, MSG_ENDING_CREDITS_37,
    "",
};

STATIC ResourceGroupInfo init_endingResourceGroup_8c045324 = {
    "ending_parts.dat",
    "ending.dat",
    "ending.pvm",
    4,
};

/* ====================
 * Forward Declarations
 * ====================
 */

STATIC void selectEndingDialog_8c01f3c0(void);
STATIC void updateEndingOverlay_8c01f42c(void);
STATIC int scrollCreditsText_8c01f50e(void);
STATIC void creditsTask_8c01f658(void);

/* ====================
 * Functions
 * ====================
 */

STATIC void selectEndingDialog_8c01f3c0(void)
{
    int i;
    int perfectCount;
    int attemptedCount;
    Uint8 state;

    perfectCount = 0;
    attemptedCount = 0;
    for (i = 0; i < 9; i++) {
        state = var_progress_8c1ba1cc.courses_0x44[i].storyAward_0x03;
        if (state == AWARD_TIER_GOLD) {
            perfectCount++;
            attemptedCount++;
        } else if (state == AWARD_TIER_SILVER || state == AWARD_TIER_BRONZE) {
            attemptedCount++;
        }
    }

    if (perfectCount >= 9) {
        var_dialogQueue_8c225fbc[0] = INSTR_SUCCESS_PERFECT;
        var_endingVoiceList_8c226430 = init_endingVoicesPerfect_8c04522c;
    } else if (attemptedCount >= 9) {
        var_dialogQueue_8c225fbc[0] = INSTR_SUCCESS_HIGH;
        var_endingVoiceList_8c226430 = init_endingVoicesHigh_8c045244;
    } else if (attemptedCount >= 5) {
        var_dialogQueue_8c225fbc[0] = INSTR_SUCCESS_NORMAL;
        var_endingVoiceList_8c226430 = init_endingVoicesNormal_8c04525c;
    } else {
        var_dialogQueue_8c225fbc[0] = INSTR_FAILURE_FINAL;
        var_endingVoiceList_8c226430 = init_endingVoicesFailure_8c045278;
    }
}

/* Per-frame update and draw of the dialog scene, before the credits start.
 * Sprite 4 of the ending resource group bounces behind the instructor
 * portrait, borrowing MenuState.pos.title (busX/flagY) as its x/y and
 * cursorVelocity_0x30 as its velocity: subState_0x1c 0 falls under a constant
 * 0.1/frame gravity until y passes 300, 1 rises until the velocity flips, and
 * x turns around at 0 and 90. */
STATIC void updateEndingOverlay_8c01f42c(void)
{
    const float step = 0.1f;

    if (var_menuState_8c1bc7a8.subState_0x1c == 0) {
        var_menuState_8c1bc7a8.cursorVelocity_0x30.y += step;
        var_menuState_8c1bc7a8.pos.title.flagY_0x24 += var_menuState_8c1bc7a8.cursorVelocity_0x30.y;
        if (var_menuState_8c1bc7a8.pos.title.flagY_0x24 > 300.0f) {
            var_menuState_8c1bc7a8.subState_0x1c = 1;
        }
    } else if (var_menuState_8c1bc7a8.subState_0x1c == 1) {
        var_menuState_8c1bc7a8.pos.title.flagY_0x24 -= var_menuState_8c1bc7a8.cursorVelocity_0x30.y;
        var_menuState_8c1bc7a8.cursorVelocity_0x30.y -= step;
        if (0.0f > var_menuState_8c1bc7a8.cursorVelocity_0x30.y) {
            var_menuState_8c1bc7a8.subState_0x1c = 0;
        }
    }

    var_menuState_8c1bc7a8.pos.title.busX_0x20 += var_menuState_8c1bc7a8.cursorVelocity_0x30.x;
    if (var_menuState_8c1bc7a8.pos.title.busX_0x20 < 0.0f
        || var_menuState_8c1bc7a8.pos.title.busX_0x20 > 90.0f) {
        var_menuState_8c1bc7a8.cursorVelocity_0x30.x = -var_menuState_8c1bc7a8.cursorVelocity_0x30.x;
    }

    SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 4,
        var_menuState_8c1bc7a8.pos.title.busX_0x20, var_menuState_8c1bc7a8.pos.title.flagY_0x24, -8.5f);

    if (MessageBoxMenuTextboxText_8c02af1c(var_menuTextboxCharLimit_8c225fb8) != 0) {
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 1, 0.0f, 0.0f, -7.0f);
    }

    SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_menuState_8c1bc7a8.instructorSprite_0x60, 0.0f, 0.0f, -8.0f);

    njSetBackColor(0x5CA3D9, 0x5CA3D9, 0x5CA3D9);
}

/* Scrolls the two credit textboxes (var_messageTextBoxA/B) up 2px/frame,
 * recycling whichever one field_0x5c points at: once it is fully off the top
 * (y < -480) it drops back a screen below (y += 960) and takes the next credit
 * page. TxtPrepareTextBoxLayout_8c01543a returning 0 means the "" terminator,
 * and subState_0x1c latches that for creditsTask_8c01f658. field_0x54/0x58 are
 * box A's and B's character-reveal caps, one more character per frame up to
 * 0xff, reset when that box takes a new page. */
STATIC int scrollCreditsText_8c01f50e(void)
{
    TextBox *box;

    if (var_menuState_8c1bc7a8.subState_0x1c == 0) {
        int idx = var_menuState_8c1bc7a8.field_0x5c;
        int other = idx ^ 1;

        ((TextBox *) (&var_messageTextBoxA_8c1bc404)[idx])->y_0x04 -= 2;
        ((TextBox *) (&var_messageTextBoxA_8c1bc404)[other])->y_0x04 -= 2;

        box = (TextBox *) (&var_messageTextBoxA_8c1bc404)[idx];
        if ((float) box->y_0x04 < -480.0f) {
            box->y_0x04 += 960;

            if (TxtPrepareTextBoxLayout_8c01543a(box, (&init_endingCreditsHead_8c04528c[0])[var_menuState_8c1bc7a8.counter_0x64]) != 0) {
                if (idx != 0) {
                    var_menuState_8c1bc7a8.field_0x58 = 0;
                } else {
                    var_menuState_8c1bc7a8.field_0x54 = 0;
                }
                var_menuState_8c1bc7a8.field_0x5c = idx ^ 1;
                var_menuState_8c1bc7a8.counter_0x64++;
            } else {
                var_menuState_8c1bc7a8.subState_0x1c = 1;
            }
        }
    }

    if (++var_menuState_8c1bc7a8.field_0x54 > 0xFF) {
        var_menuState_8c1bc7a8.field_0x54 = 0xFF;
    }
    if (++var_menuState_8c1bc7a8.field_0x58 > 0xFF) {
        var_menuState_8c1bc7a8.field_0x58 = 0xFF;
    }

    TxtDrawTextbox_8c0155e0((TextBox *) var_messageTextBoxA_8c1bc404, var_menuState_8c1bc7a8.field_0x54);
    return TxtDrawTextbox_8c0155e0((TextBox *) var_messageTextBoxB_8c1bc408, var_menuState_8c1bc7a8.field_0x58);
}

STATIC void creditsTask_8c01f658(void)
{
    switch (var_menuState_8c1bc7a8.state_0x18) {
    case ENDING_TASK_STATE_WAIT_PVM:
        if (RouteGetLatch_8c01432a() != 0) {
            return;
        }
        AsqFreeQueues_8c011f7e();
        var_menuState_8c1bc7a8.state_0x18 = ENDING_TASK_STATE_FADE_IN;
        var_menuState_8c1bc7a8.subState_0x1c = 0;
        var_menuState_8c1bc7a8.pos.title.busX_0x20 = 45.0f;
        var_menuState_8c1bc7a8.pos.title.flagY_0x24 = 100.0f;
        var_menuState_8c1bc7a8.cursorVelocity_0x30.x = 1.0f;
        var_menuState_8c1bc7a8.cursorVelocity_0x30.y = 0.0f;
        SndPlayAdx_8c010cd6(0, 1);
        RenderStartFadeIn_8c022a9c(10);
        return;

    case ENDING_TASK_STATE_FADE_IN:
        if (var_isFading_8c226568 == 0) {
            CourseMenuSpawnDialogTask_8c0170c6(var_dialogQueue_8c225fbc[0], var_endingVoiceList_8c226430);
            var_menuState_8c1bc7a8.state_0x18 = ENDING_TASK_STATE_INSTRUCTOR_DIALOG;
        }
        updateEndingOverlay_8c01f42c();
        return;

    case ENDING_TASK_STATE_INSTRUCTOR_DIALOG:
        if (var_instructorDialogActive_8c225fb4 == 0) {
            var_menuState_8c1bc7a8.state_0x18 = ENDING_TASK_STATE_FADE_OUT_TO_CREDITS;
            SndStartAdxFadeOut_8c010bae(0);
            SndStartAdxFadeOut_8c010bae(1);
            RenderStartFadeOut_8c022b60(30);
        }
        updateEndingOverlay_8c01f42c();
        return;

    case ENDING_TASK_STATE_FADE_OUT_TO_CREDITS:
        if (var_isFading_8c226568 != 0) {
            updateEndingOverlay_8c01f42c();
            return;
        }
        if (init_adxPlaying_8c03bd80 != 0) {
            return;
        }

        var_menuState_8c1bc7a8.state_0x18 = ENDING_TASK_STATE_CREDITS_INTRO;
        var_menuState_8c1bc7a8.timer_0x68 = 0;
        MessageBoxFreeTextboxes_8c02af32();
        TxtInit_8c01524c();
        var_messageTextBoxA_8c1bc404 = TxtCreateTextBox_8c0152fc(0, 480, -5.0f, 640, 480, 0, 0, -1);
        var_messageTextBoxB_8c1bc408 = TxtCreateTextBox_8c0152fc(0, 960, -5.0f, 640, 480, 0, 0, -1);
        TxtPrepareTextBoxLayout_8c01543a((TextBox *) var_messageTextBoxA_8c1bc404, init_endingCreditsHead_8c04528c[0]);
        TxtPrepareTextBoxLayout_8c01543a((TextBox *) var_messageTextBoxB_8c1bc408, init_endingCreditsTail_8c045290[0]);
        var_menuState_8c1bc7a8.subState_0x1c = 0;
        var_menuState_8c1bc7a8.field_0x5c = 0;
        var_menuState_8c1bc7a8.counter_0x64 = 2;
        var_menuState_8c1bc7a8.field_0x54 = 0;
        var_menuState_8c1bc7a8.field_0x58 = 0;
        var_menuState_8c1bc7a8.timer_0x68 = 3240;
        SndPlayAdx_8c010cd6(0, 12);
        RenderStartFadeIn_8c022a9c(30);
        return;

    case ENDING_TASK_STATE_CREDITS_INTRO:
        if (var_isFading_8c226568 == 0) {
            var_menuState_8c1bc7a8.state_0x18 = ENDING_TASK_STATE_CREDITS_SCROLL;
        }
        scrollCreditsText_8c01f50e();
        var_menuState_8c1bc7a8.timer_0x68--;
        return;

    case ENDING_TASK_STATE_CREDITS_SCROLL:
        var_menuState_8c1bc7a8.timer_0x68--;
        if (var_menuState_8c1bc7a8.timer_0x68 < 0) {
            SndPlayAdx_8c010cd6(0, 11);
            var_menuState_8c1bc7a8.timer_0x68 = 2700;
        }
        if (var_menuState_8c1bc7a8.subState_0x1c != 0) {
            var_menuState_8c1bc7a8.state_0x18 = ENDING_TASK_STATE_CREDITS_HOLD;
            var_menuState_8c1bc7a8.timer_0x68 = 0;
        }
        scrollCreditsText_8c01f50e();
        return;

    case ENDING_TASK_STATE_CREDITS_HOLD:
        var_menuState_8c1bc7a8.timer_0x68++;
        if (var_menuState_8c1bc7a8.timer_0x68 > 150) {
            var_menuState_8c1bc7a8.state_0x18 = ENDING_TASK_STATE_EXIT;
            SndStartAdxFadeOut_8c010bae(0);
            SndStartAdxFadeOut_8c010bae(1);
            RenderStartFadeOut_8c022b60(60);
        }
        scrollCreditsText_8c01f50e();
        return;

    case ENDING_TASK_STATE_EXIT:
        if (var_isFading_8c226568 != 0) {
            scrollCreditsText_8c01f50e();
            return;
        }
        if (init_adxPlaying_8c03bd80 != 0) {
            return;
        }
        ReplayMenuFreeSessionAssets_8c016182();
        if (var_selectedVm_8c1ba34c == -1) {
            TitleStart_8c015fd6(0);
        } else {
            ResultsShowFailedRun_8c01e24e();
        }
        return;
    }
}

/* Entry point, reached from the results and lesson screens once
 * var_progress_8c1ba1cc.days_0x00 passes 30 -- the career is one month long.
 * Picks the dialog tier, seeds the instructor sprite from it, then sets the
 * screen up the way every other one does (peripheral-support task plus
 * GameSoftResetTask_8c012f44's soft-reset watchdog) before spawning the ending's own
 * creditsTask_8c01f658. */
void EndingStart_8c01f954(void)
{
    Task *gameTask;
    void *gameTaskState;
    Task *creditsTaskHandle;
    void *creditsTaskState;

    selectEndingDialog_8c01f3c0();

    var_menuState_8c1bc7a8.instructorSprite_0x60 =
        init_instructorDialogs_8c044c08[var_dialogQueue_8c225fbc[0]]->spriteNo_0x04;

    InputSpawnTask_8c0128cc(0);

    TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, GameSoftResetTask_8c012f44, &gameTask, &gameTaskState, 0);
    TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, (void *) creditsTask_8c01f658, &creditsTaskHandle, &creditsTaskState, 0);

    var_menuState_8c1bc7a8.state_0x18 = 0;
    njGarbageTexture(var_tex_8c157af8, 3072);

    MessageBoxOpenTextbox_8c02ae3e(32, 384, -2.0f, 576, 64, 0, 0, -1);
    MessageBoxSwapFor_8c02aefc("");

    var_menuTextboxCharLimit_8c225fb8 = 0;
    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();

    CourseMenuRequestSysResgrp_8c018568(&var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        &init_endingResourceGroup_8c045324);
    CourseMenuRequestCommonResources_8c01852c();

    RouteSetLatch_8c014330();
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteClearLatch_8c014322);
}
