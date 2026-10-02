/* @unit PracticeMenu */
#include <shinobi.h>

#include "01e27c_practice_menu.h"
#include "011120_asset_queues.h"
#include "0129cc_game.h"
#include "013ae8_route.h"
#include "015ab8_title.h"
#include "01614c_replay_menu.h"
#include "016c58_prompt.h"
#include "016d2c_course_menu.h"
#include "01f3c0_ending.h"
#include "02a9fc_message_box.h"
#include "0100bc_sound.h"
#include "02b464_grading.h"
#include "014f54_sprite.h"
#include "022464_render.h"
#include "014a9c_tasks.h"
#include "015034_text.h"
#include "1ba1c8_globals.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "strings.h"

/* ====================
 * Type Declarations
 * ====================
 */

enum STATE {
    STATE_INIT,                /* 0 */
    STATE_DESCRIPTION_FADE_IN, /* 1 */
    STATE_DESCRIPTION_VIEW,    /* 2 */
    STATE_PROMPT,              /* 3 */
    STATE_CONFIRM_FADE_OUT,    /* 4 */
    STATE_LOADING_FADE_IN,     /* 5 */
    STATE_LOADING_HOLD,        /* 6 */
    STATE_LOADING_START,       /* 7 */
    STATE_CANCEL_FADE_OUT      /* 8 */
};

/* lessonGuideTask_8c01e63c's states, in the same state_0x18. */
enum GUIDE_STATE {
    GUIDE_STATE_INIT,                    /* 0 */
    GUIDE_STATE_FADE_IN,                 /* 1 */
    GUIDE_STATE_VIEW,                    /* 2 */
    GUIDE_STATE_VIEW_END,                /* 3: scrolled past the last row of guide text */
    GUIDE_STATE_FADE_OUT_TO_DESCRIPTION, /* 4 */
    GUIDE_STATE_FADE_OUT_TO_MENU         /* 5 */
};

/* lessonMenuTask_8c01ebf2's states, in the same state_0x18. */
enum LESSON_STATE {
    LESSON_STATE_INIT,             /* 0 */
    LESSON_STATE_DIALOG_FADE_IN,   /* 1 */
    LESSON_STATE_DIALOG_QUEUE,     /* 2 */
    LESSON_STATE_MENU,             /* 3 */
    LESSON_STATE_QUIT_ROW,         /* 4: the row past the 11 lessons that leaves practice */
    LESSON_STATE_QUIT_PROMPT,      /* 5 */
    LESSON_STATE_SELECT_FADE_OUT,  /* 6 */
    LESSON_STATE_QUIT_FADE_OUT     /* 7 */
};

/* ====================
 * Non-initialized Globals
 * ====================
 */

int var_practiceLesson_8c22640c;
int var_practiceRules_8c226410;
STATIC int var_lessonDialogQueue_8c226414[6]; /* INSTR_* ids, -1 terminated */
/* Practice drives finished since the lesson list was opened; still 0 means the
 * player never drove, and the day does not advance. */
STATIC int var_lessonAttempts_8c22642c;

/* ====================
 * Initialized Globals
 * ====================
 */

/* Sized to include the original's trailing zero padding. */
STATIC const char const_confirmStartMsg_8c03896c[TEXT_SJIS_SIZE(24)] = MSG_CONFIRM_START_PRACTICE;
STATIC const char const_emptyMsg_8c038984[4] = "";
STATIC const char const_confirmQuitMsg_8c038988[TEXT_SJIS_SIZE(44)] = MSG_CONFIRM_QUIT_PRACTICE;

/* First description-page sprite of each lesson: lesson N's pages run
 * [starts[N], starts[N + 1]). */
STATIC char init_lessonPageStarts_8c0451b4[] = {
    0, 2, 3, 4, 6, 9, 12, 15, 17, 19, 24, 25
};
/* Per-lesson var_practiceRules_8c226410: lessons 0-6 are driving only, 7 keeps
 * the schedule and the stop sequence, 8-10 are full runs. */
STATIC Uint32 init_practiceRules_8c0451c0[] = { 0, 0, 0, 0, 0, 0, 0, 6, 0xf, 0xf, 0xf };
/* Rows of guide text per lesson. */
STATIC char init_lessonGuideRows_8c0451ec[] = { 0x10, 0x0a, 0x14, 0x11, 0x12, 0x14, 0x15, 0x13, 0x13, 0x1b, 0x09, 0x00 };
/* Guide text clip rect, floats (2, 6)-(11, 7) in 32-pixel tiles: x 64-352,
 * y 192-224. */
STATIC char init_guideClipRect_8c0451f8[] = { 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0xc0, 0x40, 0x00, 0x00, 0x30, 0x41, 0x00, 0x00, 0xe0, 0x40 };
/* PENALTY_MSG_* -> INSTR_* dialog. Some penalties share a dialog; the INSTR_*
 * enum lists the ones this leaves never shown. */
STATIC char init_penaltyMsgSetInstr_8c045208[] = {
    INSTR_COLLISION_CAR_MINOR, INSTR_COLLISION_CAR_MEDIUM, INSTR_COLLISION_CAR_FATAL, INSTR_COLLISION_WALL_MINOR,
    INSTR_COLLISION_WALL_MEDIUM, INSTR_COLLISION_WALL_SEVERE, INSTR_OFF_COURSE_MINOR, INSTR_OFF_COURSE_MEDIUM,
    INSTR_OFF_COURSE_MEDIUM, INSTR_SPEEDING_MINOR, INSTR_SPEEDING_MAJOR, INSTR_WRONG_LANE,
    INSTR_WRONG_LANE, INSTR_LANE_STRADDLE, INSTR_LANE_STRADDLE, INSTR_NO_SIGNAL,
    INSTR_SIGNAL_VIOLATION, INSTR_BAD_STOP_LINE, INSTR_ILLEGAL_LANE_CHANGE, INSTR_BLOCK_INTERSECTION,
    INSTR_BLANK, INSTR_BLANK, INSTR_WRONG_WAY, INSTR_RAPID_ACCEL,
    INSTR_HARD_BRAKE, INSTR_SWERVING, INSTR_MISSED_STOP, INSTR_BAD_STOP_POSITION_1,
    INSTR_BAD_STOP_POSITION_1, INSTR_TIME_MANAGEMENT, INSTR_BLANK, INSTR_NEAR_MISS_PEDESTRIAN,
    INSTR_ANNOUNCEMENT, INSTR_DOOR_OPERATION, 0, 0,
};

/* ====================
 * Forward Declarations
 * ====================
 */

STATIC void enterGuide_8c01e920(Task *task);

/* ====================
 * Functions
 * ====================
 */

STATIC void lessonDescriptionTask_8c01e27c(Task *task)
{
    switch (var_menuState_8c1bc7a8.state_0x18) {
        case STATE_INIT:
            if (RouteGetLatch_8c01432a()) break;

            AsqFreeQueues_8c011f7e();
            var_menuState_8c1bc7a8.state_0x18 = STATE_DESCRIPTION_FADE_IN;
            var_menuState_8c1bc7a8.field_0x54 = init_lessonPageStarts_8c0451b4[var_practiceLesson_8c22640c];
            var_menuState_8c1bc7a8.field_0x5c = var_menuState_8c1bc7a8.field_0x54;
            var_menuState_8c1bc7a8.field_0x58 = init_lessonPageStarts_8c0451b4[var_practiceLesson_8c22640c + 1] - 1;
            SndPlayAdx_8c010cd6(0, 0xd);
            RenderStartFadeIn_8c022a9c(10);
            break;

        case STATE_DESCRIPTION_FADE_IN:
            if (var_isFading_8c226568 == 0)
                var_menuState_8c1bc7a8.state_0x18 = STATE_DESCRIPTION_VIEW;

            SpriteDraw_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                var_menuState_8c1bc7a8.field_0x5c,
                0.0f, 0.0f, -4.5f
            );
            break;

        case STATE_DESCRIPTION_VIEW:
            if (var_peripherals_8c1ba35c[0].press & 0x4) {
                if (var_menuState_8c1bc7a8.field_0x5c < var_menuState_8c1bc7a8.field_0x58) {
                    var_menuState_8c1bc7a8.field_0x5c++;
                } else {
                    var_menuState_8c1bc7a8.state_0x18 = STATE_PROMPT;
                    var_menuState_8c1bc7a8.selected_0x38 = 0;
                    MessageBoxSetText_8c02aefc(const_confirmStartMsg_8c03896c);
                }

                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            }

            SpriteDraw_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                var_menuState_8c1bc7a8.field_0x5c,
                0.0f, 0.0f, -4.5f
            );
            break;

        case STATE_PROMPT: {
            int result = PromptHandleBinary_8c016caa(&var_menuState_8c1bc7a8.selected_0x38);

            if (result == 1) {
                var_menuState_8c1bc7a8.state_0x18 = STATE_CONFIRM_FADE_OUT;
                SndStartAdxFadeOut_8c010bae(0);
                SndStartAdxFadeOut_8c010bae(1);
                RenderStartFadeOut_8c022b60(10);
            } else if (result == 2) {
                var_menuState_8c1bc7a8.state_0x18 = STATE_CANCEL_FADE_OUT;
                SndStartAdxFadeOut_8c010bae(0);
                SndStartAdxFadeOut_8c010bae(1);
                RenderStartFadeOut_8c022b60(10);
            }

            SpriteDraw_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                var_menuState_8c1bc7a8.selected_0x38 + 2,
                224.0f, 300.0f, -4.0f
            );

            if (MessageBoxDrawText_8c02af1c(0xff))
                SpriteDraw_8c014f54(
                    &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                    1,
                    0.0f, 0.0f, -5.0f
                );

            SpriteDraw_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                0,
                0.0f, 0.0f, -8.0f
            );
            break;
        }

        case STATE_CONFIRM_FADE_OUT:
            if (var_isFading_8c226568 == 0) {
                var_menuState_8c1bc7a8.state_0x18 = STATE_LOADING_FADE_IN;
                var_menuState_8c1bc7a8.timer_0x68 = 0;
                njSetBackColor(0, 0, 0);
                RenderStartFadeIn_8c022a9c(0x14);
                break;
            }

            SpriteDraw_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                var_menuState_8c1bc7a8.selected_0x38 + 2,
                224.0f, 300.0f, -4.0f
            );

            if (MessageBoxDrawText_8c02af1c(0xff))
                SpriteDraw_8c014f54(
                    &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                    1,
                    0.0f, 0.0f, -5.0f
                );

            SpriteDraw_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                0,
                0.0f, 0.0f, -8.0f
            );
            break;

        case STATE_LOADING_FADE_IN:
            if (var_isFading_8c226568 == 0)
                var_menuState_8c1bc7a8.state_0x18 = STATE_LOADING_HOLD;

            SpriteDraw_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                var_practiceLesson_8c22640c + 0x19,
                0.0f, 0.0f, -4.5f
            );
            break;

        case STATE_LOADING_HOLD:
            if (++var_menuState_8c1bc7a8.timer_0x68 > 10) {
                var_menuState_8c1bc7a8.state_0x18 = STATE_LOADING_START;
                RenderStartFadeOut_8c022b60(0x14);
            }

            SpriteDraw_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                var_practiceLesson_8c22640c + 0x19,
                0.0f, 0.0f, -4.5f
            );
            break;

        case STATE_LOADING_START:
            if (var_isFading_8c226568 == 0) {
                if (init_adxPlaying_8c03bd80 != 0) break;

                ReplayMenuFreeSessionAssets_8c016182();
                var_worstPenaltyDelta_8c1bb8f0 = 0;
                var_worstPenaltyMsgSet_8c1bb8ec = 0x1d;
                var_penaltyCount_8c1bb8f4 = 0;
                var_practiceRules_8c226410 = init_practiceRules_8c0451c0[var_practiceLesson_8c22640c];
                GameStartCourse_8c013310(var_practiceLesson_8c22640c + 0x1b);
                break;
            }

            SpriteDraw_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                var_practiceLesson_8c22640c + 0x19,
                0.0f, 0.0f, -4.5f
            );
            break;

        case STATE_CANCEL_FADE_OUT:
            if (var_isFading_8c226568 == 0) {
                if (init_adxPlaying_8c03bd80 != 0) break;

                enterGuide_8c01e920(task);
                break;
            }

            SpriteDraw_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                var_menuState_8c1bc7a8.selected_0x38 + 2,
                224.0f, 300.0f, -4.0f
            );

            if (MessageBoxDrawText_8c02af1c(0xff))
                SpriteDraw_8c014f54(
                    &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                    1,
                    0.0f, 0.0f, -4.5f
                );
            break;
    }
}

STATIC void enterDescription_8c01e576(Task *task)
{
    TaskReinstall_8c014b3e(task, lessonDescriptionTask_8c01e27c);
    var_menuState_8c1bc7a8.state_0x18 = STATE_INIT;

    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();
    CourseMenuRequestSysResgrp_8c018568(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, &init_practice02ResourceGroup_8c044284);
    RouteSetLatch_8c014330();
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteClearLatch_8c014322);
}

/* Guide text in a 4-row window. A goes on to the description pages; B, or A
 * past the last row, goes back to the lesson list. */
STATIC void lessonGuideTask_8c01e63c(Task *task)
{
    switch (var_menuState_8c1bc7a8.state_0x18) {
        case GUIDE_STATE_INIT:
            if (RouteGetLatch_8c01432a()) return;

            AsqFreeQueues_8c011f7e();
            var_menuState_8c1bc7a8.state_0x18 = GUIDE_STATE_FADE_IN;
            SndPlayAdx_8c010cd6(0, 0xd);
            RenderStartFadeIn_8c022a9c(10);
            return;

        case GUIDE_STATE_FADE_IN:
            if (var_isFading_8c226568 == 0) {
                var_menuState_8c1bc7a8.state_0x18 = GUIDE_STATE_VIEW;
            }
            break;

        case GUIDE_STATE_VIEW:
            if (var_peripherals_8c1ba35c[0].press & 0x4) {
                var_menuState_8c1bc7a8.state_0x18 = GUIDE_STATE_FADE_OUT_TO_DESCRIPTION;
                SndStartAdxFadeOut_8c010bae(0);
                SndStartAdxFadeOut_8c010bae(1);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                RenderStartFadeOut_8c022b60(10);
            } else if (var_peripherals_8c1ba35c[0].press & 0x2) {
                var_menuState_8c1bc7a8.state_0x18 = GUIDE_STATE_FADE_OUT_TO_MENU;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
                RenderStartFadeOut_8c022b60(10);
            } else if (!(var_peripherals_8c1ba35c[0].press & 0x10)) {
                if (var_peripherals_8c1ba35c[0].press & 0x20) {
                    if (var_menuState_8c1bc7a8.cursorRow_0x40 + 3 < (signed char)init_lessonGuideRows_8c0451ec[var_practiceLesson_8c22640c])
                        var_menuState_8c1bc7a8.cursorRow_0x40++;
                    else
                        var_menuState_8c1bc7a8.state_0x18 = GUIDE_STATE_VIEW_END;

                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                }
            } else if (var_menuState_8c1bc7a8.cursorRow_0x40 >= 1) {
                var_menuState_8c1bc7a8.cursorRow_0x40--;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            }
            break;

        case GUIDE_STATE_VIEW_END:
            if (var_peripherals_8c1ba35c[0].press & 0x4) {
                var_menuState_8c1bc7a8.state_0x18 = GUIDE_STATE_FADE_OUT_TO_MENU;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
                RenderStartFadeOut_8c022b60(10);
            } else if (var_peripherals_8c1ba35c[0].press & 0x10) {
                var_menuState_8c1bc7a8.state_0x18 = GUIDE_STATE_VIEW;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            }
            break;

        case GUIDE_STATE_FADE_OUT_TO_DESCRIPTION:
            if (var_isFading_8c226568 == 0) {
                if (init_adxPlaying_8c03bd80 != 0) return;
                enterDescription_8c01e576(task);
                return;
            }
            break;

        case GUIDE_STATE_FADE_OUT_TO_MENU:
            if (var_isFading_8c226568 == 0) {
                var_menuState_8c1bc7a8.selected_0x38 = var_practiceLesson_8c22640c;
                PracticeMenuEnter_8c01f114(task);
                return;
            }
            break;
    }

    if (var_gameMode_8c1bb8fc == 0)
        CourseMenuDrawDateAndExp_8c016ee6();

    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_practiceLesson_8c22640c + 0x1c,
        0.0f, 0.0f, -4.0f
    );

    njUserClipping(2, init_guideClipRect_8c0451f8);
    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_practiceLesson_8c22640c + 0x27,
        65.0f, 195.0f - (float)var_menuState_8c1bc7a8.cursorRow_0x40 * 24.0f, -4.0f
    );
    njUserClipping(0, init_guideClipRect_8c0451f8);

    if (var_menuState_8c1bc7a8.cursorRow_0x40 > 0)
        SpriteDraw_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
            0x18,
            0.0f, 0.0f, -4.5f
        );

    if (var_menuState_8c1bc7a8.cursorRow_0x40 + 3 != (signed char)init_lessonGuideRows_8c0451ec[var_practiceLesson_8c22640c])
        SpriteDraw_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
            0x19,
            0.0f, 0.0f, -4.5f
        );

    if (var_menuState_8c1bc7a8.state_0x18 == GUIDE_STATE_VIEW_END)
        SpriteDraw_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
            0x1b,
            0.0f, 0.0f, -4.5f
        );

    if (MessageBoxDrawText_8c02af1c(0xff))
        SpriteDraw_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupA_0x00,
            1,
            0.0f, 0.0f, -5.0f
        );

    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        0x32,
        0.0f, 0.0f, -6.0f
    );

    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_gameMode_8c1bb8fc == 0 ? 0x17 : 0x41,
        0.0f, 0.0f, -7.0f
    );

    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00,
        0,
        0.0f, 0.0f, -8.0f
    );
}

/* CourseMenuRequestSysResgrp_8c018568 returns 0 when practice01 is still
 * resident. */
STATIC void enterGuide_8c01e920(Task *task)
{
    TaskReinstall_8c014b3e(task, lessonGuideTask_8c01e63c);
    var_menuState_8c1bc7a8.cursorRow_0x40 = 0;
    MessageBoxSetText_8c02aefc(const_emptyMsg_8c038984);

    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();

    if (CourseMenuRequestSysResgrp_8c018568(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, &init_practice01ResourceGroup_8c044274)) {
        RouteSetLatch_8c014330();
        AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteClearLatch_8c014322);
        var_menuState_8c1bc7a8.state_0x18 = GUIDE_STATE_INIT;
        return;
    }

    AsqFreeQueues_8c011f7e();
    var_menuState_8c1bc7a8.state_0x18 = GUIDE_STATE_FADE_IN;
    RenderStartFadeIn_8c022a9c(10);
    // Unreachable: compiled as a tail call, never returns here.
    // coverage:ignore-next-line
}

STATIC void buildDialogQueue_8c01e992(void)
{
    int i = 0;

    /* Only a finished practice drive outside free run reports a result.
     * Arriving from the course menu gets the tips, or from day 30 the
     * final-day preamble. */
    if (var_gameMode_8c1bb8fc == 1 || var_runReportPending_8c1bb8b8 == 0 || var_runWasPractice_8c1bb8bc == 0) {
        if (var_gameMode_8c1bb8fc != 1 && var_runReportPending_8c1bb8b8 == 0 && var_runWasPractice_8c1bb8bc != 0) {
            if (var_progress_8c1ba1cc.days_0x00 < 0x1e) {
                var_lessonDialogQueue_8c226414[i++] = INSTR_LESSON_TIPS;
                if (var_gameMode_8c1bb8fc == 0) var_lessonDialogQueue_8c226414[i++] = INSTR_LESSON_WARNING;
            } else {
                var_lessonDialogQueue_8c226414[i++] = INSTR_LESSON_FINAL_DAY;
            }

            var_lessonDialogQueue_8c226414[i++] = INSTR_LESSON_CHOOSE;
            var_runReportPending_8c1bb8b8 = 1;
        } else {
            var_lessonDialogQueue_8c226414[i++] = INSTR_LESSON_CHOOSE;
        }

        var_lessonDialogQueue_8c226414[i] = -1;
        return;
    }

    if (var_penaltyCount_8c1bb8f4 == 0) {
        var_lessonDialogQueue_8c226414[i++] = INSTR_LESSON_PERFECT;
    } else {
        var_lessonDialogQueue_8c226414[i++] = (unsigned char)init_penaltyMsgSetInstr_8c045208[var_worstPenaltyMsgSet_8c1bb8ec];

        if (var_runSucceeded_8c1bb8dc == 0)
            var_lessonDialogQueue_8c226414[i++] = (var_penaltyCount_8c1bb8f4 == 1) ? INSTR_LESSON_FAIL_MINOR : INSTR_LESSON_FAIL_MAJOR;
        else
            var_lessonDialogQueue_8c226414[i++] = (var_penaltyCount_8c1bb8f4 == 1) ? INSTR_LESSON_GOOD : INSTR_LESSON_PASS;
    }

    if (var_award_8c1bb8f8 != 0) var_lessonDialogQueue_8c226414[i++] = INSTR_SCORE_RECORD;

    var_lessonDialogQueue_8c226414[i++] = INSTR_LESSON_CHOOSE;
    var_lessonDialogQueue_8c226414[i] = -1;
}

STATIC void drawDigits_8c01ead8(int value, float y)
{
    float x = 362.0f;

    do {
        SpriteDraw_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
            value % 10 + 0xc,
            x, y + 6.0f, -4.3f
        );

        value /= 10;
        x -= 18.0f;
    } while (value != 0);
}

/* 5-row window over the 11 lessons; 6 is the last full page. */
STATIC void scrollTowardSelection_8c01ebc8(void)
{
    if (var_menuState_8c1bc7a8.scrollTopRow_0x44 < var_menuState_8c1bc7a8.selected_0x38 - 4) {
        var_menuState_8c1bc7a8.scrollTopRow_0x44 = var_menuState_8c1bc7a8.selected_0x38 - 4;
        if (var_menuState_8c1bc7a8.scrollTopRow_0x44 > 6)
            var_menuState_8c1bc7a8.scrollTopRow_0x44 = 6;
    } else if (var_menuState_8c1bc7a8.selected_0x38 < var_menuState_8c1bc7a8.scrollTopRow_0x44) {
        var_menuState_8c1bc7a8.scrollTopRow_0x44 = var_menuState_8c1bc7a8.selected_0x38;
    }
}

/* Plays out var_lessonDialogQueue_8c226414, then the lesson list. */
STATIC void lessonMenuTask_8c01ebf2(Task *task, void *state)
{
    int row, i, spriteId, modeIcon;
    float y;

    switch (var_menuState_8c1bc7a8.state_0x18) {
        case LESSON_STATE_INIT:
            if (RouteGetLatch_8c01432a()) return;

            AsqFreeQueues_8c011f7e();
            var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_DIALOG_FADE_IN;
            SndStopBgm_8c010d8a();
            SndPlayAdx_8c010cd6(0, 0xd);
            RenderStartFadeIn_8c022a9c(10);
            return;

        case LESSON_STATE_DIALOG_FADE_IN:
            if (var_isFading_8c226568 == 0) {
                CourseMenuInstallDialog_8c0170c6(var_lessonDialogQueue_8c226414[0], 0);
                var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_DIALOG_QUEUE;
            }
            break;

        case LESSON_STATE_DIALOG_QUEUE:
            if (var_instructorDialogActive_8c225fb4 == 0) {
                task->field_0x08++;
                if (var_lessonDialogQueue_8c226414[task->field_0x08] == -1) {
                    var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_MENU;
                    MessageBoxSetText_8c02aefc(const_emptyMsg_8c038984);
                } else {
                    CourseMenuInstallDialog_8c0170c6(var_lessonDialogQueue_8c226414[task->field_0x08], 0);
                }
            }
            break;

        case LESSON_STATE_MENU:
            if ((var_peripherals_8c1ba35c[0].press & 4) != 0) {
                var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_SELECT_FADE_OUT;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                RenderStartFadeOut_8c022b60(10);
            }
            else if ((var_peripherals_8c1ba35c[0].press & 2) != 0) {
                var_menuState_8c1bc7a8.subState_0x1c = var_menuState_8c1bc7a8.state_0x18;
                var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_QUIT_PROMPT;
                var_menuState_8c1bc7a8.cursorCol_0x3c = 0;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
                var_menuTextboxCharLimit_8c225fb8 = MessageBoxSetText_8c02aefc(const_confirmQuitMsg_8c038988);
            }
            else if ((var_peripherals_8c1ba35c[0].press & 0x10) != 0) {
                if (var_menuState_8c1bc7a8.selected_0x38 != 0) {
                    var_menuState_8c1bc7a8.selected_0x38--;
                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                    MessageBoxSetText_8c02aefc(const_emptyMsg_8c038984);
                }
            }
            else if ((var_peripherals_8c1ba35c[0].press & 0x20) != 0) {
                var_menuState_8c1bc7a8.selected_0x38++;
                MessageBoxSetText_8c02aefc(const_emptyMsg_8c038984);
                if (var_menuState_8c1bc7a8.selected_0x38 > 10) {
                    var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_QUIT_ROW;
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            }
            scrollTowardSelection_8c01ebc8();
            break;

        case LESSON_STATE_QUIT_ROW:
            if ((var_peripherals_8c1ba35c[0].press & 4) != 0) {
                var_menuState_8c1bc7a8.subState_0x1c = var_menuState_8c1bc7a8.state_0x18;
                var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_QUIT_PROMPT;
                var_menuState_8c1bc7a8.cursorCol_0x3c = 0;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                var_menuTextboxCharLimit_8c225fb8 = MessageBoxSetText_8c02aefc(const_confirmQuitMsg_8c038988);
            }
            else if ((var_peripherals_8c1ba35c[0].press & 0x10) != 0) {
                var_menuState_8c1bc7a8.selected_0x38--;
                var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_MENU;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            }
            break;

        case LESSON_STATE_QUIT_PROMPT: {
            int result = PromptHandleBinary_8c016caa(&var_menuState_8c1bc7a8.cursorCol_0x3c);
            if (result == 1) {
                var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_QUIT_FADE_OUT;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                SndStartAdxFadeOut_8c010bae(0);
                SndStartAdxFadeOut_8c010bae(1);
                RenderStartFadeOut_8c022b60(10);
            }
            else if (result == 2) {
                var_menuState_8c1bc7a8.state_0x18 = var_menuState_8c1bc7a8.subState_0x1c;
                MessageBoxSetText_8c02aefc(const_emptyMsg_8c038984);
                var_menuTextboxCharLimit_8c225fb8 = 0;
            }

            SpriteDraw_8c014f54(
                &var_menuState_8c1bc7a8,
                var_menuState_8c1bc7a8.cursorCol_0x3c + 2,
                228.0f, 320.0f, -5.0f
            );
            break;
        }

        case LESSON_STATE_SELECT_FADE_OUT:
            if (var_isFading_8c226568 == 0) {
                var_practiceLesson_8c22640c = var_menuState_8c1bc7a8.selected_0x38;
                var_runWasPractice_8c1bb8bc = 0;
                enterGuide_8c01e920(task);
                return;
            }
            break;

        case LESSON_STATE_QUIT_FADE_OUT:
            if (var_isFading_8c226568 == 0) {
                if (init_adxPlaying_8c03bd80 != 0) return;

                if (var_gameMode_8c1bb8fc == 0) {
                    if (var_lessonAttempts_8c22642c == 0) {
                        var_runReportPending_8c1bb8b8 = 0;
                    } else {
                        var_progress_8c1ba1cc.days_0x00++;
                        var_progress_8c1ba1cc.exp_0x90 += 0x32;
                        if (var_progress_8c1ba1cc.exp_0x90 > 99999) {
                            var_progress_8c1ba1cc.exp_0x90 = 99999;
                        }
                        var_runReportPending_8c1bb8b8 = 1;
                    }
                    var_runWasPractice_8c1bb8bc = 1;
                }

                var_menuState_8c1bc7a8.cursorCol_0x3c = 0;
                var_menuState_8c1bc7a8.cursorRow_0x40 = 0;
                /* Offset 0x24, not scrollTopRow_0x44: as in the original. */
                var_menuState_8c1bc7a8.pos.title.flagY_0x24 = 0.0f;
                ReplayMenuFreeSessionAssets_8c016182();

                if (var_progress_8c1ba1cc.days_0x00 > 0x1e && var_gameMode_8c1bb8fc != 1) {
                    CourseMenuBuildCourseUnlockList_8c0172dc();
                    CourseMenuApplyUnlocks_8c0173e6();
                    EndingStart_8c01f954();
                    return;
                }

                CourseMenuReturn_8c017ef2();
                return;
            }

            SpriteDraw_8c014f54(
                &var_menuState_8c1bc7a8,
                var_menuState_8c1bc7a8.cursorCol_0x3c + 2,
                228.0f, 320.0f, -5.0f
            );
            break;
    }

    if (var_gameMode_8c1bb8fc == 0) {
        CourseMenuDrawDateAndExp_8c016ee6();
    }

    y = 108.0f;
    row = var_menuState_8c1bc7a8.scrollTopRow_0x44;
    for (i = 0; i < 5; i++) {
        if (row == var_menuState_8c1bc7a8.selected_0x38
            && var_menuState_8c1bc7a8.state_0x18 != LESSON_STATE_QUIT_ROW) {
            spriteId = row + 0x34;
        } else {
            spriteId = row + 1;
        }
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, spriteId, 48.0f, y, -4.5f);
        drawDigits_8c01ead8(var_progress_8c1ba1cc.practiceLessonBestScores_0x98[row], y);
        row++;
        y += 33.0f;
    }

    if (var_menuState_8c1bc7a8.selected_0x38 > 10) {
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0x1b, 0.0f, 0.0f, -2.0f);
    }
    if (var_menuState_8c1bc7a8.scrollTopRow_0x44 > 0) {
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0x1a, 0.0f, 0.0f, -2.0f);
    }
    if (var_menuState_8c1bc7a8.scrollTopRow_0x44 < 6) {
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0x19, 0.0f, 0.0f, -2.0f);
    }

    if (MessageBoxDrawText_8c02af1c(var_menuTextboxCharLimit_8c225fb8)) {
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8, 1, 0.0f, 0.0f, -5.0f);
    }

    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_menuState_8c1bc7a8.instructorSprite_0x60 + 0x32, 0.0f, 0.0f, -6.0f);

    if (var_gameMode_8c1bb8fc == 0) {
        modeIcon = 0;
    } else {
        modeIcon = 0x40;
    }
    SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, modeIcon, 0.0f, 0.0f, -3.0f);

    SpriteDraw_8c014f54(&var_menuState_8c1bc7a8, 0, 0.0f, 0.0f, -8.0f);
}

void PracticeMenuEnter_8c01f114(Task *task)
{
    var_playMode_8c1bb8d0 = PLAY_MODE_PRACTICE;
    TaskReinstall_8c014b3e(task, lessonMenuTask_8c01ebf2);
    var_menuState_8c1bc7a8.scrollTopRow_0x44 = 0;
    scrollTowardSelection_8c01ebc8();
    var_lessonAttempts_8c22642c = 0;
    var_award_8c1bb8f8 = 0;
    buildDialogQueue_8c01e992();
    task->field_0x08 = 0;
    var_menuState_8c1bc7a8.instructorSprite_0x60 =
        init_instructorDialogs_8c044c08[var_lessonDialogQueue_8c226414[0]]->spriteNo_0x04;
    njSetBackColor(0, 0, 0);
    njGarbageTexture(var_tex_8c157af8, 0xc00);

    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();

    if (CourseMenuRequestSysResgrp_8c018568(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, &init_practice01ResourceGroup_8c044274)) {
        RouteSetLatch_8c014330();
        AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteClearLatch_8c014322);
        var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_INIT;
        return;
    }

    AsqFreeQueues_8c011f7e();
    var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_DIALOG_FADE_IN;
    RenderStartFadeIn_8c022a9c(10);
}

/* Re-entry after a practice drive: records a new best score and installs the
 * lesson list's tasks from scratch. */
void PracticeMenuReturn_8c01f21c(void)
{
    Task *installed_task;
    void *installed_state;

    var_menuState_8c1bc7a8.scrollTopRow_0x44 = 0;
    scrollTowardSelection_8c01ebc8();
    var_award_8c1bb8f8 = 0;

    if (var_runWasPractice_8c1bb8bc == 0) {
        var_lessonDialogQueue_8c226414[0] = INSTR_LESSON_CHOOSE;
        var_lessonDialogQueue_8c226414[1] = -1;
    } else {
        if (var_runSucceeded_8c1bb8dc != 0 && var_progress_8c1ba1cc.practiceLessonBestScores_0x98[var_practiceLesson_8c22640c] < var_runState_8c2285c4.driverPoints_0x0c) {
            var_progress_8c1ba1cc.practiceLessonBestScores_0x98[var_practiceLesson_8c22640c] = var_runState_8c2285c4.driverPoints_0x0c;
            var_award_8c1bb8f8 = 1;
        }

        var_lessonAttempts_8c22642c++;
        buildDialogQueue_8c01e992();
    }

    var_playMode_8c1bb8d0 = PLAY_MODE_PRACTICE;
    InputInstall_8c0128cc(0);
    TaskInstall_8c014ae8(var_tasks_8c1ba3c8, GameSoftResetTask_8c012f44, &installed_task, &installed_state, 0);
    TaskInstall_8c014ae8(var_tasks_8c1ba3c8, lessonMenuTask_8c01ebf2, &installed_task, &installed_state, 0);

    var_menuState_8c1bc7a8.state_0x18 = LESSON_STATE_INIT;
    var_menuState_8c1bc7a8.instructorSprite_0x60 =
        init_instructorDialogs_8c044c08[var_lessonDialogQueue_8c226414[0]]->spriteNo_0x04;
    installed_task->field_0x08 = 0;

    njGarbageTexture(var_tex_8c157af8, 0xc00);
    MessageBoxOpenTextbox_8c02ae3e(0x20, 0x180, -2.0f, 0x240, 0x40, 0, 0, -1);
    MessageBoxSetText_8c02aefc(const_emptyMsg_8c038984);

    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();
    var_currentSysResGroupInfo_8c225fb0 = (void *) -1;
    CourseMenuRequestSysResgrp_8c018568(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, &init_practice01ResourceGroup_8c044274);
    CourseMenuRequestCommonResources_8c01852c();
    RouteSetLatch_8c014330();
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteClearLatch_8c014322);
}
