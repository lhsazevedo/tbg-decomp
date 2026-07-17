/* @unit CourseMenu */
#include <shinobi.h>
#include <sg_sd.h>
#include <njdef.h>
#include <sg_xpt.h>
#include "012504_input.h"
#include "012f44_game.h"
#include "013ae8_route_load.h"
#include "015ab8_title.h"
#include "014a9c_tasks.h"
#include "011120_asset_queues.h"
#include "016c58_prompt.h"
#include "019e98_main_menu.h"
#include "016d2c_course_menu.h"
#include "0100bc_sound.h"
#include "01d290_album.h"
#include "01b19c.h"
#include "01c980.h"
#include "01e27c.h"
#include "028258.h"
#include "014f54_text_pre_data.h"
#include "0fcd20_sectionB.h"
#include "serial_debug.h"
#include "serial_debug.h"

// TODO:
// - Review comments

/* ====================
 * Compiler Definitions
 * ====================
 */

#ifdef SERIAL_DEBUG
char *DEBUG_courseMenuStateNames[] = {
    "INIT",
    "FADE_IN",
    "DIALOG",
    "IDLE",
    "ANIMATING",
    "COURSE_SELECTED",
    "FADE_OUT",
    "FADE_OUT_TO_MAIN_MENU"
};

char *DEBUG_courseConfirmStateNames[] = {
    "INIT",
    "FADE_IN",
    "PROMPT",
    "FADE_OUT",
    "ROUTE_INFO_FADE_IN",
    "ROUTE_INFO_DISPLAY",
    "START_LOADING",
    "FADE_OUT_TO_COURSE_MENU"
};
#endif

#ifdef SERIAL_DEBUG
#define CHANGE_STATE(x) var_menuState_8c1bc7a8.state_0x18 = x; LOG_DEBUG(("[COURSE_MENU] State changed: %s\n", DEBUG_courseMenuStateNames[x]))
#define CHANGE_CONFIRM_STATE(x) var_menuState_8c1bc7a8.state_0x18 = x; LOG_DEBUG(("[COURSE_MENU] Confirm state changed: %s\n", DEBUG_courseConfirmStateNames[x]))
#else
#define CHANGE_STATE(x) var_menuState_8c1bc7a8.state_0x18 = x
#define CHANGE_CONFIRM_STATE(x) var_menuState_8c1bc7a8.state_0x18 = x
#endif

/* =================
 * Type Declarations
 * =================
 */

typedef struct {
    int enabled_0x00;
    int unlocked_0x04;
    float x_0x08;
    float y_0x0c;
    int spriteNo_0x10;
    void (*onSelect_0x14)(Task *task);
    int courseId_0x18;
} CourseMenuButton;

typedef struct {
    char *text_0x00;
    int instructorSpriteNo_0x04;
} MenuDialog;

typedef struct {
    int state_0x00;
    MenuDialog *dialog_0x04;
    int field_0x08;
    int field_0x0c;
    int field_0x10;
    int field_0x14;
    int *field_0x18;
} DialogSequenceTaskState;

typedef struct {
    TaskAction action;
    void *state;
    int field_0x08;
    void* field_0x0c;
    int field_0x10;
    int field_0x14;
    int *field_0x18;
    int field_0x1c;
} DialogSequenceTask;

enum {
    // --- Story / Training ---
    SEQ_STORY_INTRO           = 0,
    // SEQ_SUCCESS_PERFECT       = 1,
    // SEQ_SUCCESS_HIGH          = 2,
    // SEQ_SUCCESS_NORMAL        = 3,
    // SEQ_FAILURE_FINAL         = 4,
    // SEQ_FREE_RUN_INTRO        = 5,
    SEQ_STORY_CHOOSE_COURSE   = 6,
    SEQ_GOOD_PRACTICE         = 7,
    SEQ_SUCCESS               = 8,

    // --- Awards / Unlocks ---
    SEQ_AWARD_BADGE_GOLD      = 9,
    SEQ_AWARD_BADGE_SILVER    = 10,
    SEQ_AWARD_BADGE_BRONZE    = 11,
    SEQ_FAILURE_RETRY         = 12,
    SEQ_COURSE_UNLOCKED       = 13,
    SEQ_PASSENGER_LETTER      = 14,
    SEQ_COURSE_LOCKED         = 15,
    // SEQ_FORCE_PRACTICE        = 16,
    // SEQ_FINAL_DAY             = 17,

    // --- Lesson Mode ---
    // SEQ_LESSON_INTRO          = 18,
    // SEQ_LESSON_COMPLETE       = 19,
    // SEQ_LESSON_NEXT           = 20,
    // SEQ_LESSON_RETRY          = 21,
    // SEQ_LESSON_TIPS           = 22,
    // SEQ_LESSON_WARNING        = 23,
    // SEQ_LESSON_CHOOSE         = 24,
    // SEQ_SCORE_RECORD          = 25,
    // SEQ_LESSON_FINAL_DAY      = 26,
    // SEQ_LESSON_PERFECT        = 27,
    // SEQ_LESSON_GOOD           = 28,
    // SEQ_LESSON_PASS           = 29,
    // SEQ_LESSON_FAIL_MINOR     = 30,
    // SEQ_LESSON_FAIL_MAJOR     = 31,

    // --- Driving Mistakes / Penalties ---
    // SEQ_COLLISION_CAR_MINOR   = 32,
    // SEQ_COLLISION_CAR_MEDIUM  = 33,
    // SEQ_COLLISION_CAR_SEVERE  = 34,
    // SEQ_COLLISION_CAR_FATAL   = 35,
    // SEQ_COLLISION_WALL_MINOR  = 36,
    // SEQ_COLLISION_WALL_MEDIUM = 37,
    // SEQ_COLLISION_WALL_SEVERE = 38,
    // SEQ_NEAR_MISS_PEDESTRIAN  = 39,
    // SEQ_OFF_COURSE_MINOR      = 40,
    // SEQ_OFF_COURSE_MEDIUM     = 41,
    // SEQ_OFF_COURSE_MAJOR      = 42,
    // SEQ_SPEEDING_MINOR        = 43,
    // SEQ_SPEEDING_MAJOR        = 44,
    // SEQ_WRONG_LANE            = 45,
    // SEQ_LANE_STRADDLE         = 46,
    // SEQ_NO_SIGNAL             = 47,
    // SEQ_NO_SIGNAL_TURN        = 48,
    // SEQ_UKN_49                 = 49,
    // SEQ_SIGNAL_VIOLATION      = 50,
    // SEQ_BAD_STOP_LINE         = 51,
    // SEQ_ILLEGAL_LANE_CHANGE   = 52,
    // SEQ_BLOCK_INTERSECTION    = 53,
    // SEQ_WRONG_WAY             = 54,
    // SEQ_RAPID_ACCEL           = 55,
    // SEQ_HARD_BRAKE            = 56,
    // SEQ_SWERVING              = 57,
    // SEQ_MISSED_STOP           = 58,
    // SEQ_BAD_STOP_POSITION_1   = 59,
    // SEQ_BAD_STOP_POSITION_2   = 60,
    // SEQ_TIME_MANAGEMENT       = 61,
    // SEQ_ANNOUNCEMENT          = 62,
    // SEQ_DOOR_OPERATION        = 63,

    // --- Free Run Mode ---
    SEQ_FREE_RUN_INTRO_2       = 64,
    SEQ_FREE_RUN_CHOOSE_COURSE = 65,
};

enum {
    COURSE_MENU_STATE_INIT = 0,
    COURSE_MENU_STATE_FADE_IN = 1,
    COURSE_MENU_STATE_DIALOG = 2,
    COURSE_MENU_STATE_IDLE = 3,
    COURSE_MENU_STATE_ANIMATING = 4,
    COURSE_MENU_STATE_COURSE_SELECTED = 5,
    COURSE_MENU_STATE_FADE_OUT = 6,
    COURSE_MENU_STATE_FADE_OUT_TO_MAIN_MENU = 7
};

enum {
    COURSE_CONFIRM_STATE_INIT = 0,
    COURSE_CONFIRM_STATE_FADE_IN = 1,
    COURSE_CONFIRM_STATE_PROMPT = 2,
    COURSE_CONFIRM_STATE_FADE_OUT = 3,
    COURSE_CONFIRM_STATE_ROUTE_INFO_FADE_IN = 4,
    COURSE_CONFIRM_STATE_ROUTE_INFO_DISPLAY = 5,
    COURSE_CONFIRM_STATE_START_LOADING = 6,
    COURSE_CONFIRM_STATE_FADE_OUT_TO_COURSE_MENU = 7
};

/* =======================
 * Non-initialized Globals
 * =======================
 */

/* Data defined after the functions so the shared "" literal is first seen in code
   (swapMessageBoxFor("")) and lands at the head of the constant pool. */
STATIC CourseMenuButton init_courseMenuButtons_8c04442c[15];
STATIC ResourceGroupInfo init_courseResourceGroup_8c044d40;

/* ====================
 * Forward Declarations
 * ====================
 */

int CourseMenuRequestSysResgrp_8c018568(ResourceGroup* dds, ResourceGroupInfo* rg);
STATIC void courseMenuConfirmInit_8c0184cc(Task *task);
void CourseMenuFreeResourceGroup_8c0185c4(ResourceGroup *res_group);
STATIC void courseMenuFreeRunMenuTask_8c017ada(Task * task, void *state);
void CourseMenuRequestCommonResources_8c01852c(void);
MenuDialog *init_dialogSequences_8c044c08[66];
Uint8 init_courseVariants_8c044d10[30];
Uint8 init_routeInfoTime_8c044d2e[3 * 3 * 2];

/* =========
 * Functions
 * =========
 */

/**
 * Returns 1 if the cursor has reached its target position, 0 otherwise.
 */
int CourseMenuInterpolateCursor_8c016d2c()
{
    var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x += var_menuState_8c1bc7a8.cursorVelocity_0x30.x;
    var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.y += var_menuState_8c1bc7a8.cursorVelocity_0x30.y;

    if (var_menuState_8c1bc7a8.cursorVelocity_0x30.x) {
        if (
            (var_menuState_8c1bc7a8.cursorVelocity_0x30.x >= 0)
            || (var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x > var_menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.x)
        ) {
            // if (!(var_menuState_8c1bc7a8.cursorVelocity_0x30.x > 0)) {
            if (var_menuState_8c1bc7a8.cursorVelocity_0x30.x <= 0) {
                return 0;
            }

            // if (!(var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x > var_menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.x)) {
            if (var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x <= var_menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.x) {
                return 0;
            }

        }
        var_menuState_8c1bc7a8.pos.cursor.cursor_0x20 = var_menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28;
    } else if (var_menuState_8c1bc7a8.cursorVelocity_0x30.y) {
        if (
            (var_menuState_8c1bc7a8.cursorVelocity_0x30.y >= 0)
            || (var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.y > var_menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.y)
        ) {
            // if (!(var_menuState_8c1bc7a8.cursorVelocity_0x30.y > 0)) {
            if (var_menuState_8c1bc7a8.cursorVelocity_0x30.y <= 0) {
                return 0;
            }

            if (!(var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.y > var_menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.y)) {
                return 0;
            }

        }
        var_menuState_8c1bc7a8.pos.cursor.cursor_0x20 = var_menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28;
    }

    return 1;
}

STATIC int cursorOffTarget_8c016dc6()
{
    int selected;
    float y;
    float x;

    selected = var_menuState_8c1bc7a8.field_0x3c + var_menuState_8c1bc7a8.field_0x40 * 5;
    x = init_courseMenuButtons_8c04442c[selected].x_0x08;
    y = init_courseMenuButtons_8c04442c[selected].y_0x0c;
    if (
        (var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x == x)
        && (var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.y == y)
    ) {
        return 0;
    }
    var_menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.x = x;
    var_menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.y = y;
    var_menuState_8c1bc7a8.cursorVelocity_0x30.x = (x - var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x) / 6.0;
    var_menuState_8c1bc7a8.cursorVelocity_0x30.y = (y - var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.y) / 6.0;
    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
    swapMessageBoxFor_8c02aefc("");
    return 1;
}

STATIC void drawInteger_8c016e6c(int value, float x, float y)
{
    do {
        TxtDrawSprite_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupA_0x00,
            15 + value % 10,
            x,
            y,
            -4.0
        );
        x -= 10.0;
    } while (value /= 10);
}

STATIC unsigned int getWeekDayIndex_8c016ed2()
{
    unsigned int r = var_progress_8c1ba1cc.days_0x00 + 1;
    return r % 7;
}

void CourseMenuDrawDateAndExp_8c016ee6()
{
    float x;
    int days, sprite_id;

    // Draw date
    days = var_progress_8c1ba1cc.days_0x00;
    if (days < 10) {
        x = 84.0;
    } else {
        x = 95.0;
    }
    drawInteger_8c016e6c(days, x, 82.0);

    // Hmm...
    if (days == 15) {
        sprite_id = 13;
    } else if (days == 23) {
        sprite_id = 14;
    } else {
        sprite_id = 6 + getWeekDayIndex_8c016ed2();
    }

    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00,
        sprite_id,
        112.0,
        82.0,
        -4.0
    );

    drawInteger_8c016e6c(var_progress_8c1ba1cc.exp_0x90, 534.0, 82.0);
}

STATIC void dialogSequenceTask_8c016f98(DialogSequenceTask *task, DialogSequenceTaskState *state)
{
    switch(state->state_0x00) {
        case 0: {
            int r;

            if (!*(state->dialog_0x04->text_0x00)) {
                var_dialogSequenceIsActive_8c225fb4 = 0;
                TaskFree_8c014b66((void *) task);
                return;
            }

            if (task->field_0x18 && *task->field_0x18) {
                SndProc_8c010cd6(2, *task->field_0x18);
                task->field_0x18++;
            }

            state->field_0x08 = swapMessageBoxFor_8c02aefc(state->dialog_0x04->text_0x00);
            var_menuState_8c1bc7a8.instructorSprite_0x60 = state->dialog_0x04->instructorSpriteNo_0x04;
            state->field_0x0c = 1;
            state->field_0x10 = 0;
            state->state_0x00 = 1;
            break;
        }

        case 1: {
            if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
                state->field_0x10 = 99;
                state->state_0x00 = 2;
                FUN_8c010ca6(1);
            }

            if (++state->field_0x10 < 3) {
                break;
            }

            if (++state->field_0x0c < state->field_0x08) {
                state->field_0x10 = 0;
            } else {
                state->state_0x00 = 3;
            }

            break;
        }

        case 2: {
            if (!(var_peripherals_8c1ba35c[0].on & PDD_DGT_TA)) {
                state->state_0x00 = 1;
                break;
            }

            if ((state->field_0x0c += 2) < state->field_0x08) {
                break;
            }

            state->state_0x00 = 3;
            break;
        }

        case 3: {
            if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
                state->dialog_0x04++;
                state->state_0x00 = 0;
            }

            state->field_0x14 += 0x1111;
            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                44,
                32.0,
                -16.0 + 8 * njCos(state->field_0x14),
                -3.0
            );

            break;
        }
    }

    var_menuTextboxCharLimit_8c225fb8 = state->field_0x0c;
}

void CourseMenuPushDialogTask_8c0170c6(int dialog_index, int *p2)
{
    DialogSequenceTask *task;
    DialogSequenceTaskState *state;

    TaskPush_8c014ae8(
        var_tasks_8c1ba3c8,
        &dialogSequenceTask_8c016f98,
        &task,
        &state,
        0x18
    );

    task->field_0x18 = p2;
    state->state_0x00 = 0;
    state->dialog_0x04 = init_dialogSequences_8c044c08[dialog_index];
    var_dialogSequenceIsActive_8c225fb4 = 1;
}

STATIC void swapDialogMessageBox_8c017108(int sequence)
{
    var_menuTextboxCharLimit_8c225fb8 = swapMessageBoxFor_8c02aefc(
        init_dialogSequences_8c044c08[sequence]->text_0x00
    );
}

STATIC void handleCourseMenuInput_8c017126()
{
    if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
        if (
            init_courseMenuButtons_8c04442c[
                var_menuState_8c1bc7a8.field_0x3c
                + var_menuState_8c1bc7a8.field_0x40 * 5
            ]
            .unlocked_0x04 == 0
        ) {
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
            swapDialogMessageBox_8c017108(SEQ_COURSE_LOCKED);
        } else {
            SndStartAdxFadeOut_8c010bae(0);
            SndStartAdxFadeOut_8c010bae(1);
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            CHANGE_STATE(COURSE_MENU_STATE_COURSE_SELECTED);
            var_menuState_8c1bc7a8.logo_timer_0x68 = 0;
        }
    }

    if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KU) {
        do {
            if (--var_menuState_8c1bc7a8.field_0x40 < 0) {
                var_menuState_8c1bc7a8.field_0x40 = 2;
            }
        } while (
            init_courseMenuButtons_8c04442c[
                var_menuState_8c1bc7a8.field_0x40 * 5 + var_menuState_8c1bc7a8.field_0x3c
            ].enabled_0x00 == 0
        );

        if (cursorOffTarget_8c016dc6()) {
            CHANGE_STATE(COURSE_MENU_STATE_ANIMATING);
        }
    } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KD) {
        do {
            if (++var_menuState_8c1bc7a8.field_0x40 > 2) {
                var_menuState_8c1bc7a8.field_0x40 = 0;
            }
        } while (
            init_courseMenuButtons_8c04442c[
                var_menuState_8c1bc7a8.field_0x40 * 5 + var_menuState_8c1bc7a8.field_0x3c
            ].enabled_0x00 == 0
        );

        if (cursorOffTarget_8c016dc6()) {
            CHANGE_STATE(COURSE_MENU_STATE_ANIMATING);
        }
    } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KL) {
        do {
            if (--var_menuState_8c1bc7a8.field_0x3c < 0) {
                var_menuState_8c1bc7a8.field_0x3c = 4;
            }
        } while (
            init_courseMenuButtons_8c04442c[
                var_menuState_8c1bc7a8.field_0x40 * 5 + var_menuState_8c1bc7a8.field_0x3c
            ].enabled_0x00 == 0
        );

        if (cursorOffTarget_8c016dc6()) {
            CHANGE_STATE(COURSE_MENU_STATE_ANIMATING);
        }
    } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KR) {
        do {
            if (++var_menuState_8c1bc7a8.field_0x3c > 4) {
                var_menuState_8c1bc7a8.field_0x3c = 0;
            }
        } while (
            init_courseMenuButtons_8c04442c[
                var_menuState_8c1bc7a8.field_0x40 * 5 + var_menuState_8c1bc7a8.field_0x3c
            ].enabled_0x00 == 0
        );

        if (cursorOffTarget_8c016dc6()) {
            CHANGE_STATE(COURSE_MENU_STATE_ANIMATING);
        }
    }
}

int CourseMenuBuildCourseUnlockList_8c0172dc()
{
    int i = 0;
    int j = 0;
    for (; i < 9; i++) {
        if (var_progress_8c1ba1cc.courses_0x44[i].unlocked_0x00)
            continue;

        switch (i) {
            case 0:
                continue;

            case 1:
                if (
                    var_progress_8c1ba1cc.days_0x00 < 8 ||
                    var_progress_8c1ba1cc.exp_0x90 < 4000
                )
                    continue;
                break;

            case 2:
                if (
                    var_progress_8c1ba1cc.days_0x00 < 9 ||
                    var_progress_8c1ba1cc.exp_0x90 < 5500
                )
                    continue;
                break;

            case 3:
                if (
                    var_progress_8c1ba1cc.days_0x00 < 5 ||
                    var_progress_8c1ba1cc.exp_0x90 < 2000
                )
                    continue;
                break;

            case 4:
                if (
                    var_progress_8c1ba1cc.days_0x00 < 11 ||
                    var_progress_8c1ba1cc.exp_0x90 < 8000
                )
                    continue;
                break;

            case 5:
                if (
                    var_progress_8c1ba1cc.days_0x00 < 13 ||
                    var_progress_8c1ba1cc.exp_0x90 < 12000
                )
                    continue;
                break;

            case 6:
                continue;

            case 7:
                if (
                    var_progress_8c1ba1cc.days_0x00 < 3 ||
                    var_progress_8c1ba1cc.exp_0x90 < 500
                )
                    continue;
                break;

            case 8:
                if (
                    var_progress_8c1ba1cc.days_0x00 < 6 ||
                    var_progress_8c1ba1cc.exp_0x90 < 3000
                )
                    continue;
                break;
        }

        var_coursesToUnlock_8c225fd4[j] = i;
        j++;
    }

    var_coursesToUnlock_8c225fd4[j] = -1;
    return j;
}

void CourseMenuApplyUnlocks_8c0173e6(void)
{
    int i;
    for (i = 0; var_coursesToUnlock_8c225fd4[i] != -1; i++) {
        int j = var_coursesToUnlock_8c225fd4[i];
        var_progress_8c1ba1cc.courses_0x44[j].unlocked_0x00 = 1;
        var_progress_8c1ba1cc.courses_0x44[j].new_0x01 = 1;
    }
}

STATIC void buildCourseMenuDialogFlow_8c017420(void)
{
    int cur = 0;

    // Default choose course
    if (var_8c1bb8b8 == 0) {
        var_dialogQueue_8c225fbc[cur++] = SEQ_STORY_CHOOSE_COURSE;
        var_dialogQueue_8c225fbc[cur]   = -1;
        return;
    }

    // On the first day, show the intro briefing
    if (var_progress_8c1ba1cc.days_0x00 == 1) {
        var_dialogQueue_8c225fbc[cur++] = SEQ_STORY_INTRO;
        var_dialogQueue_8c225fbc[cur++] = SEQ_STORY_CHOOSE_COURSE;
        var_dialogQueue_8c225fbc[cur]   = -1;
        return;
    }

    // Special Success
    if (var_8c1bb8bc != 0) {
        var_dialogQueue_8c225fbc[cur++] = SEQ_GOOD_PRACTICE;
        var_dialogQueue_8c225fbc[cur++] = SEQ_STORY_CHOOSE_COURSE;
        var_dialogQueue_8c225fbc[cur]   = -1;
        return;
    }

    // Result
    if (var_8c1bb8dc == 0) {
        var_dialogQueue_8c225fbc[cur++] = SEQ_FAILURE_RETRY;
    } else {
        int award_seq = SEQ_SUCCESS;
        if      (var_award_8c1bb8f8 == 1) award_seq = SEQ_AWARD_BADGE_BRONZE;
        else if (var_award_8c1bb8f8 == 2) award_seq = SEQ_AWARD_BADGE_SILVER;
        else if (var_award_8c1bb8f8 == 3) award_seq = SEQ_AWARD_BADGE_GOLD;
        var_dialogQueue_8c225fbc[cur++] = award_seq;
    }

    // Course unlocked
    if (CourseMenuBuildCourseUnlockList_8c0172dc() != 0) {
        var_dialogQueue_8c225fbc[cur++] = SEQ_COURSE_UNLOCKED;
    }

    // Passenger letter received
    if (((var_progress_8c1ba1cc.days_0x00 + 1) % 7) == 0) {
        int r = AsqGetRandomInRangeB_8c0121be(6);
        if (var_progress_8c1ba1cc.letters_0x2c[r] == 0) {
            var_progress_8c1ba1cc.letters_0x2c[r] = 1;
            var_dialogQueue_8c225fbc[cur++] = SEQ_PASSENGER_LETTER;
        }
    }

    var_dialogQueue_8c225fbc[cur++] = SEQ_STORY_CHOOSE_COURSE;

    var_dialogQueue_8c225fbc[cur] = -1;
}

STATIC void drawCourseButtons_8c017590()
{
    int i;

    if (var_menuState_8c1bc7a8.field_0x48) {
        TxtDrawSprite_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
            0x18,
            var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x,
            var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.y,
            -3.0
        );
    }

    // TODO: Extract length constant
    for (i = 0; i < 15; i++) {
        CourseMenuButton *btn = &init_courseMenuButtons_8c04442c[i];

        if (btn->unlocked_0x04 == 0 || btn->spriteNo_0x10 == 0)
            continue;

        TxtDrawSprite_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
            btn->spriteNo_0x10,
            0.0,
            0.0,
            -4.0
        );
    }

    // TODO: Extract length constant
    for (i = 0; i < 9; i++) {
        char spriteNo = var_gameMode_8c1bb8fc == 0
            ? var_progress_8c1ba1cc.courses_0x44[i].storySpriteNo_0x03
            : var_progress_8c1ba1cc.courses_0x44[i].freeRunSpriteNo_0x04;

        if (!spriteNo)
            continue;

        TxtDrawSprite_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
            0x18 - spriteNo,
            240.0 + (i % 3) * 93.0,
            106.0 + (i / 3) * 74.0,
            -3.5
        );
    }
}

STATIC void courseMenuStoryMenuTask_8c017718(Task * task, void *state)
{
    switch (var_menuState_8c1bc7a8.state_0x18) {
        case COURSE_MENU_STATE_INIT: {
            if (RouteLoadIsPvmReady_8c01432a())
                return;

            AsqFreeQueues_8c011f7e();
            CHANGE_STATE(COURSE_MENU_STATE_FADE_IN);
            FUN_8c010d8a();
            SndProc_8c010cd6(0, 15);
            push_fadein_8c022a9c(10);
            return;
        }

        case COURSE_MENU_STATE_FADE_IN: {
            if (var_isFading_8c226568 == 0) {
                CourseMenuPushDialogTask_8c0170c6(var_dialogQueue_8c225fbc[0], 0);
                CHANGE_STATE(COURSE_MENU_STATE_DIALOG);
            }
            break;
        }

        case COURSE_MENU_STATE_DIALOG: {
            // Dialog still running
            if (var_dialogSequenceIsActive_8c225fb4) break;

            if (var_dialogQueue_8c225fbc[task->field_0x08] == SEQ_COURSE_UNLOCKED) {
                int row;
                CourseMenuApplyUnlocks_8c0173e6();
                for (row = 0; row < 3; row++) {
                    int col;
                    for (col = 0; col < 3; col++) {
                        // We offset by 2 because the first two entries
                        // of each row are not courses buttons.
                        init_courseMenuButtons_8c04442c[2 + row * 5 + col].unlocked_0x04 =
                            var_progress_8c1ba1cc.courses_0x44[row * 3 + col].unlocked_0x00;
                    }
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[5], 1, 0x16, 0);
            }

            // TODO: Rename to dialogSequenceIndex
            task->field_0x08++;

            // If we finished the last dialog sequence
            if (var_dialogQueue_8c225fbc[task->field_0x08] == -1) {
                CHANGE_STATE(COURSE_MENU_STATE_IDLE);
                swapMessageBoxFor_8c02aefc("");
            }
            // Otherwise, start the next dialog sequence
            else {
                CourseMenuPushDialogTask_8c0170c6(var_dialogQueue_8c225fbc[task->field_0x08], 0);
                if (var_dialogQueue_8c225fbc[task->field_0x08] == SEQ_COURSE_UNLOCKED) {
                    SndMidiResetFxAndPlay_8c010846(0, 0);
                }
            }
            break;
        }

        case COURSE_MENU_STATE_IDLE: {
            handleCourseMenuInput_8c017126();
            break;
        }

        case COURSE_MENU_STATE_ANIMATING: {
            if (!CourseMenuInterpolateCursor_8c016d2c())
                break;

            CHANGE_STATE(COURSE_MENU_STATE_IDLE);
            break;
        }

        case COURSE_MENU_STATE_COURSE_SELECTED: {
            if (++var_menuState_8c1bc7a8.logo_timer_0x68 > 10) {
                CHANGE_STATE(COURSE_MENU_STATE_FADE_OUT);
                push_fadeout_8c022b60(10);
            }
            var_menuState_8c1bc7a8.field_0x48 = var_menuState_8c1bc7a8.logo_timer_0x68 & 1;
            break;
        }

        case COURSE_MENU_STATE_FADE_OUT: {
            int buttonIndex;

            if (var_isFading_8c226568) {
                var_menuState_8c1bc7a8.field_0x48 = ++var_menuState_8c1bc7a8.logo_timer_0x68 & 1;
                break;
            }

            if (init_8c03bd80)
                return;

            if (var_menuState_8c1bc7a8.field_0x3c != 1 || var_menuState_8c1bc7a8.field_0x40 != 0) {
                CourseMenuFreeResourceGroup_8c0185c4(&var_menuState_8c1bc7a8.resourceGroupB_0x0c);
                var_currentSysResGroupInfo_8c225fb0 = (void *) -1;
            }

            var_menuState_8c1bc7a8.selected_0x38 = 0;
            buttonIndex = var_menuState_8c1bc7a8.field_0x40 * 5 + var_menuState_8c1bc7a8.field_0x3c;
            var_menuState_8c1bc7a8.courseId_0x50 =
                init_courseMenuButtons_8c04442c[buttonIndex].courseId_0x18;

            var_8c1bb8dc = 1;
            var_8c1bb8b8 = 0;
            var_8c1bb8bc = 1;

            init_courseMenuButtons_8c04442c[buttonIndex].onSelect_0x14(task);
            return;
        }

        case COURSE_MENU_STATE_FADE_OUT_TO_MAIN_MENU: {
            if (var_isFading_8c226568)
                break;

            if (init_8c03bd80)
                return;

            var_8c1bb8b8 = 0;
            MainMenuSwitchFromTask_8c01a09a(task);
            return;
        }
    }

    CourseMenuDrawDateAndExp_8c016ee6();
    drawCourseButtons_8c017590();
    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c, 10, 0.0, 0.0, -5.0
    );
    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00, 0x2b, 0.0, 0.0, -4.0
    );
    if (menuTextboxText_8c02af1c(var_menuTextboxCharLimit_8c225fb8) ) {
        TxtDrawSprite_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupA_0x00, 1, 0.0, 0.0, -5.0
        );
    }

    // Draw instructor
    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_menuState_8c1bc7a8.instructorSprite_0x60,
        0.0,
        0.0,
        -6.0
    );

    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00, 0, 0.0, 0.0, -7.0
    );
    AsqGetRandomA_8c012166();
}

STATIC void courseMenuFreeRunMenuTask_8c017ada(Task * task, void *state)
{
    switch (var_menuState_8c1bc7a8.state_0x18) {
        case COURSE_MENU_STATE_INIT: {
            if (RouteLoadIsPvmReady_8c01432a())
                return;

            AsqFreeQueues_8c011f7e();
            CHANGE_STATE(COURSE_MENU_STATE_FADE_IN);
            FUN_8c010d8a();
            SndProc_8c010cd6(0, 15);
            push_fadein_8c022a9c(10);
            return;
        }

        case COURSE_MENU_STATE_FADE_IN: {
            if (var_isFading_8c226568 == 0) {
                CourseMenuPushDialogTask_8c0170c6(var_dialogQueue_8c225fbc[0], 0);
                CHANGE_STATE(COURSE_MENU_STATE_DIALOG);
            }
            break;
        }

        case COURSE_MENU_STATE_DIALOG: {
            // Dialog still running
            if (var_dialogSequenceIsActive_8c225fb4) break;

            if (var_dialogQueue_8c225fbc[task->field_0x08] == SEQ_COURSE_UNLOCKED) {
                int row;
                CourseMenuApplyUnlocks_8c0173e6();
                for (row = 0; row < 3; row++) {
                    int col;
                    for (col = 0; col < 3; col++) {
                        // We offset by 2 because the first two entries
                        // of each row are not courses buttons.
                        init_courseMenuButtons_8c04442c[2 + row * 5 + col].unlocked_0x04 =
                            var_progress_8c1ba1cc.courses_0x44[row * 3 + col].unlocked_0x00;
                    }
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[5], 1, 0x16, 0);
            }

            // TODO: Rename to dialogSequenceIndex
            task->field_0x08++;

            // If we finished the last dialog sequence
            if (var_dialogQueue_8c225fbc[task->field_0x08] == -1) {
                CHANGE_STATE(COURSE_MENU_STATE_IDLE);
                swapMessageBoxFor_8c02aefc("");
            }
            // Otherwise, start the next dialog sequence
            else {
                CourseMenuPushDialogTask_8c0170c6(var_dialogQueue_8c225fbc[task->field_0x08], 0);
                if (var_dialogQueue_8c225fbc[task->field_0x08] == SEQ_COURSE_UNLOCKED) {
                    SndMidiResetFxAndPlay_8c010846(0, 0);
                }
            }
            break;
        }

        case COURSE_MENU_STATE_IDLE: {
            handleCourseMenuInput_8c017126();
            break;
        }

        case COURSE_MENU_STATE_ANIMATING: {
            if (!CourseMenuInterpolateCursor_8c016d2c())
                break;

            CHANGE_STATE(COURSE_MENU_STATE_IDLE);
            break;
        }

        case COURSE_MENU_STATE_COURSE_SELECTED: {
            if (++var_menuState_8c1bc7a8.logo_timer_0x68 > 10) {
                CHANGE_STATE(COURSE_MENU_STATE_FADE_OUT);
                push_fadeout_8c022b60(10);
            }
            var_menuState_8c1bc7a8.field_0x48 = var_menuState_8c1bc7a8.logo_timer_0x68 & 1;
            break;
        }

        case COURSE_MENU_STATE_FADE_OUT: {
            int buttonIndex;

            if (var_isFading_8c226568) {
                var_menuState_8c1bc7a8.field_0x48 = ++var_menuState_8c1bc7a8.logo_timer_0x68 & 1;
                break;
            }

            if (init_8c03bd80)
                return;

            if (var_menuState_8c1bc7a8.field_0x3c != 1 || var_menuState_8c1bc7a8.field_0x40 != 0) {
                CourseMenuFreeResourceGroup_8c0185c4(&var_menuState_8c1bc7a8.resourceGroupB_0x0c);
                var_currentSysResGroupInfo_8c225fb0 = (void *) -1;
            }

            var_menuState_8c1bc7a8.selected_0x38 = 0;
            buttonIndex = var_menuState_8c1bc7a8.field_0x40 * 5 + var_menuState_8c1bc7a8.field_0x3c;
            var_menuState_8c1bc7a8.courseId_0x50 =
                init_courseMenuButtons_8c04442c[buttonIndex].courseId_0x18;

            // var_8c1bb8dc = 1;
            // var_8c1bb8b8 = 0;
            // var_8c1bb8bc = 1;

            init_courseMenuButtons_8c04442c[buttonIndex].onSelect_0x14(task);
            return;
        }

        case COURSE_MENU_STATE_FADE_OUT_TO_MAIN_MENU: {
            if (var_isFading_8c226568)
                break;

            if (init_8c03bd80)
                return;

            // var_8c1bb8b8 = 0;
            MainMenuSwitchFromTask_8c01a09a(task);
            return;
        }
    }

    // CourseMenuDrawDateAndExp_8c016ee6();
    drawCourseButtons_8c017590();
    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c, 9, 0.0, 0.0, -5.0
    );
    // TxtDrawSprite_8c014f54(
    //     &var_menuState_8c1bc7a8.resourceGroupA_0x00, 0x2b, 0.0, 0.0, -4.0
    // );
    if (menuTextboxText_8c02af1c(var_menuTextboxCharLimit_8c225fb8) ) {
        TxtDrawSprite_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupA_0x00, 1, 0.0, 0.0, -5.0
        );
    }

    // Draw instructor
    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_menuState_8c1bc7a8.instructorSprite_0x60,
        0.0,
        0.0,
        -6.0
    );

    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00, 0, 0.0, 0.0, -7.0
    );
    AsqGetRandomA_8c012166();
}

STATIC void buildFreeRunMenuDialogFlow_8c017a20(void)
{
    int idx = 0;

    if (var_shouldShowFreeRunIntro_8c1bb8c0) {
        var_dialogQueue_8c225fbc[idx++] = SEQ_FREE_RUN_INTRO_2;
    }

    var_dialogQueue_8c225fbc[idx++] = SEQ_FREE_RUN_CHOOSE_COURSE;
    var_dialogQueue_8c225fbc[idx]   = -1;

    var_shouldShowFreeRunIntro_8c1bb8c0 = 0;
}

STATIC void FUN_8c017d54(void)
{
    int enabled;
    int row;
    int game_mode = var_gameMode_8c1bb8fc;

    // Enable cursor
    var_menuState_8c1bc7a8.field_0x48 = 1;

    // Update cursor target/velocity if off-target
    cursorOffTarget_8c016dc6();

    // Snap current cursor position to its target
    var_menuState_8c1bc7a8.pos.cursor.cursor_0x20 = var_menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28;

    // Event and Album buttons: enabled in Story Mode, disabled in Free Run
    enabled = game_mode == 0 ? 1 : 0;
    init_courseMenuButtons_8c04442c[5].enabled_0x00 = enabled;
    init_courseMenuButtons_8c04442c[6].enabled_0x00 = enabled;

    // Refresh the 3x3 grid of course buttons from PlayerProgress
    for (row = 0; row < 3; row++) {
        int col;
        for (col = 0; col < 3; col++) {
            int courseIdx = row * 3 + col;
            int buttonIdx = 2 + row * 5 + col; // offset by 2 each row
            init_courseMenuButtons_8c04442c[buttonIdx].unlocked_0x04 =
                game_mode == 0
                    ? var_progress_8c1ba1cc.courses_0x44[courseIdx].unlocked_0x00
                    : var_progress_8c1ba1cc.courses_0x44[courseIdx].new_0x01;
        }
    }
}

void CourseMenuSwitchFromTask_8c017e18(Task *task)
{
    LOG_INFO(("[COURSE_MENU] Initializing course menu (mode=%d)\n", var_gameMode_8c1bb8fc));

    if (var_gameMode_8c1bb8fc == 0) {
        TaskSetAction_8c014b3e(task, courseMenuStoryMenuTask_8c017718);
        buildCourseMenuDialogFlow_8c017420();
    } else {
        TaskSetAction_8c014b3e(task, courseMenuFreeRunMenuTask_8c017ada);
        buildFreeRunMenuDialogFlow_8c017a20();
    }

    // Get instructor sprite from the first dialog entry
    var_menuState_8c1bc7a8.instructorSprite_0x60 =
        init_dialogSequences_8c044c08[
            var_dialogQueue_8c225fbc[0]
        ]->instructorSpriteNo_0x04;
    task->field_0x08 = 0;
    var_menuTextboxCharLimit_8c225fb8 = 0;
    var_playMode_8c1bb8d0 = 0;
    FUN_8c017d54();
    njGarbageTexture(var_tex_8c157af8, 0xc00);
    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();

    if (!CourseMenuRequestSysResgrp_8c018568(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        &init_mainMenuResourceGroup_8c044264
    )) {
        AsqFreeQueues_8c011f7e();
        CHANGE_STATE(COURSE_MENU_STATE_FADE_IN);
        push_fadein_8c022a9c(10);
        SndProc_8c010cd6(0, 15);
        return;
    }

    RouteLoadSetPvmReady_8c014330();
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteLoadResetPvmReady_8c014322);
    CHANGE_STATE(COURSE_MENU_STATE_INIT);
}

void CourseMenuFUN_8c017ef2(void)
{
    Task *createdTask;
    void *createdState;

    LOG_INFO(("[COURSE_MENU] Setting up story course menu\n"));

    InputPushTask_8c0128cc(0);

    TaskPush_8c014ae8(
        var_tasks_8c1ba3c8,
        &GameTask_8c012f44,
        &createdTask,
        &createdState,
        0
    );

    if (var_gameMode_8c1bb8fc == 0) {
        TaskPush_8c014ae8(
            var_tasks_8c1ba3c8,
            &courseMenuStoryMenuTask_8c017718,
            &createdTask,
            &createdState,
            0
        );
        buildCourseMenuDialogFlow_8c017420();
    } else {
        TaskPush_8c014ae8(
            var_tasks_8c1ba3c8,
            &courseMenuFreeRunMenuTask_8c017ada,
            &createdTask,
            &createdState,
            0
        );
        buildFreeRunMenuDialogFlow_8c017a20();
    }

    var_menuState_8c1bc7a8.instructorSprite_0x60 =
        init_dialogSequences_8c044c08[
            var_dialogQueue_8c225fbc[0]
        ]->instructorSpriteNo_0x04;

    createdTask->field_0x08 = 0;

    var_menuTextboxCharLimit_8c225fb8 = 0;

    njGarbageTexture(var_tex_8c157af8, 0xc00);
    FUN_8c02ae3e(0x20, 0x180, -2.0, 0x240, 0x40, 0, 0, -1);
    swapMessageBoxFor_8c02aefc("");
    var_playMode_8c1bb8d0 = 0;

    FUN_8c017d54();
    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();

    CourseMenuRequestSysResgrp_8c018568(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        &init_mainMenuResourceGroup_8c044264
    );
    CourseMenuRequestCommonResources_8c01852c();
    RouteLoadSetPvmReady_8c014330();
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteLoadResetPvmReady_8c014322);

    CHANGE_STATE(COURSE_MENU_STATE_INIT);
}

STATIC void drawFixedInteger_8c01803e(float x, float y, int value, int digits)
{
    float tracking = 19.0;
    do {
        do {
            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                12 + value % 10,
                x,
                y,
                -4.0
            );
            x -= tracking;
            digits--;
        } while (value /= 10);
    } while (digits > 0);
}

STATIC void drawRouteInfo_8c018118(void)
{
    int index = var_menuState_8c1bc7a8.field_0x40 * 6 + (var_menuState_8c1bc7a8.field_0x3c - 2) * 2;

    // Draw day
    drawFixedInteger_8c01803e(219.0, 108.0, var_progress_8c1ba1cc.days_0x00, 0);

    // Draw weekday sprite
    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        getWeekDayIndex_8c016ed2() + 0x16,
        281.0,
        110.0,
        -4.0
    );

    // Draw hour and minute
    drawFixedInteger_8c01803e(421.0, 108.0, init_routeInfoTime_8c044d2e[index], 2);
    drawFixedInteger_8c01803e(471.0, 108.0, init_routeInfoTime_8c044d2e[index + 1], 2);

    // Draw route info
    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_menuState_8c1bc7a8.field_0x40 + 9,
        0.0,
        0.0,
        -7.0
    );
}

STATIC void courseConfirmMenuTask_8c0181b6(Task * task, void *state)
{
    switch (var_menuState_8c1bc7a8.state_0x18) {
        case COURSE_CONFIRM_STATE_INIT: {
            if (RouteLoadIsPvmReady_8c01432a())
                return;

            AsqFreeQueues_8c011f7e();
            CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_FADE_IN);
            push_fadein_8c022a9c(10);
            SndProc_8c010cd6(0, 15);
            return;
        }

        case COURSE_CONFIRM_STATE_FADE_IN: {
            if (var_isFading_8c226568 == 0) {
                CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_PROMPT);
            }
            break;
        }

        case COURSE_CONFIRM_STATE_PROMPT: {
            int r = PromptHandleBinary_8c016caa(&var_menuState_8c1bc7a8.selected_0x38);
            if (r == 1) {
                CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_FADE_OUT);
                push_fadeout_8c022b60(10);
            } else if (r == 2) {
                CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_FADE_OUT_TO_COURSE_MENU);
                SndStartAdxFadeOut_8c010bae(0);
                SndStartAdxFadeOut_8c010bae(1);
                push_fadeout_8c022b60(10);
            }
            break;
        }

        case COURSE_CONFIRM_STATE_FADE_OUT: {
            if (var_isFading_8c226568 == 0) {
                CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_ROUTE_INFO_FADE_IN);
                push_fadein_8c022a9c(0x14);
            }
            break;
        }

        case COURSE_CONFIRM_STATE_ROUTE_INFO_FADE_IN: {
            if (var_isFading_8c226568 == 0) {
                CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_ROUTE_INFO_DISPLAY);
                var_menuState_8c1bc7a8.logo_timer_0x68 = 0;
            }
            // State 4 uses drawRouteInfo instead of epilogue rendering
            drawRouteInfo_8c018118();
            return;
        }

        case COURSE_CONFIRM_STATE_ROUTE_INFO_DISPLAY: {
            var_menuState_8c1bc7a8.logo_timer_0x68++;
            if (var_menuState_8c1bc7a8.logo_timer_0x68 > 30) {
                CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_START_LOADING);
                SndStartAdxFadeOut_8c010bae(0);
                SndStartAdxFadeOut_8c010bae(1);
                push_fadeout_8c022b60(20);
            }
            // State 5 uses drawRouteInfo instead of epilogue rendering
            drawRouteInfo_8c018118();
            return;
        }

        case COURSE_CONFIRM_STATE_START_LOADING: {
            if (var_isFading_8c226568 == 0) {
                int i = 0;
                int courseIndex = var_menuState_8c1bc7a8.courseId_0x50 / 3;

                if (init_8c03bd80 != 0) {
                    // init is busy, just return early
                    return;
                }
                // Step 1: Initialize game systems
                FUN_8c016182();

                // Step 2: Get course index and check if unlocked
                if (var_progress_8c1ba1cc.courses_0x44[courseIndex].field_0x02 == 0) {
                    // Course not unlocked, mark it
                    var_8c1bb8e0 = 1;
                    var_progress_8c1ba1cc.courses_0x44[courseIndex].field_0x02 = 1;
                } else {
                    var_8c1bb8e0 = 0;
                }

                // Step 3: Initialize various game state variables
                var_8c1bb8e8 = 0;
                var_8c1bb8e4 = 0;
                var_8c1bb8f0 = 0;
                var_8c1bb8ec = 0x1d;
                var_8c1bb8f4 = 0;

                // Step 4: Copy progress data to two arrays (5 uint32 values each)
                for (i = 0; i < 5; i++) {
                    var_8c1ba2b8[i] = ((int*)(&var_progress_8c1ba1cc.field_0x04))[i];
                    var_8c1ba2cc[i] = ((int*)(&var_progress_8c1ba1cc.field_0x04))[i + 5];
                }

                // Step 5: Update courseId_0x50 by adding day-based lookup value
                var_menuState_8c1bc7a8.courseId_0x50 += 
                    init_courseVariants_8c044d10[var_progress_8c1ba1cc.days_0x00 - 1];

                // Step 6: Initialize game and push loading task
                GamePushLoadingTask_8c013310(var_menuState_8c1bc7a8.courseId_0x50);
                return;
            }
            // State 6 uses drawRouteInfo instead of epilogue rendering
            drawRouteInfo_8c018118();
            return;
        }

        case COURSE_CONFIRM_STATE_FADE_OUT_TO_COURSE_MENU: {
            if (var_isFading_8c226568 == 0) {
                if (init_8c03bd80 != 0) {
                    // init is busy, just return early
                    return;
                }

                CourseMenuFreeResourceGroup_8c0185c4(&var_menuState_8c1bc7a8.resourceGroupB_0x0c);
                var_currentSysResGroupInfo_8c225fb0 = (void *) -1;
                CourseMenuSwitchFromTask_8c017e18(task);
                return;
            }
            break; // State 7 uses normal epilogue rendering
        }
    }

    // Epilogue rendering that runs every frame for this task
    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_menuState_8c1bc7a8.courseId_0x50 / 3,
        0.0,
        0.0,
        -4.0
    );

    // 2) Draw confirm/cancel prompt (sprite id = field_0x38 + 2)
    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00,
        var_menuState_8c1bc7a8.selected_0x38 + 2,
        376.0,
        378.0,
        -4.0
    );

    // 3) Foreground overlay
    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00,
        0,
        0.0,
        0.0,
        -7.0
    );
}

STATIC void courseMenuConfirmInit_8c0184cc(Task *task)
{
    LOG_INFO(("[COURSE_MENU] Initializing course confirmation menu\n"));

    njGarbageTexture(var_tex_8c157af8, 0xc00);
    TaskSetAction_8c014b3e(task, courseConfirmMenuTask_8c0181b6);
    CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_INIT);
    var_menuState_8c1bc7a8.selected_0x38 = 0;
    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();
    CourseMenuRequestSysResgrp_8c018568(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        &init_courseResourceGroup_8c044d40
    );
    RouteLoadSetPvmReady_8c014330();
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteLoadResetPvmReady_8c014322);
    CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_INIT);
    return;
}

void CourseMenuRequestCommonResources_8c01852c(void)
{
    AsqRequestDat_8c011182(
        "\\SYSTEM",
        "common_parts.dat",
        &var_menuState_8c1bc7a8.resourceGroupA_0x00.tanim_0x04
    );
    AsqRequestDat_8c011182(
        "\\SYSTEM",
        "common.dat",
        &var_menuState_8c1bc7a8.resourceGroupA_0x00.contents_0x08
    );
    AsqRequestPvm_8c011ac0("\\SYSTEM", "common.pvm", &var_menuState_8c1bc7a8, 1, 0);
    return;
}

int CourseMenuRequestSysResgrp_8c018568(ResourceGroup *res_group, ResourceGroupInfo *res_group_info)
{
    if (var_currentSysResGroupInfo_8c225fb0 == res_group_info) {
        return 0;
    }

    var_currentSysResGroupInfo_8c225fb0 = res_group_info;

    if (res_group->tlist_0x00 != (void *) -1) {
        CourseMenuFreeResourceGroup_8c0185c4(res_group);
    }

    AsqRequestDat_8c011182(
        "\\SYSTEM", res_group_info->parts, &res_group->tanim_0x04
    );
    AsqRequestDat_8c011182(
        "\\SYSTEM", res_group_info->dat, &res_group->contents_0x08
    );
    AsqRequestPvm_8c011ac0(
        "\\SYSTEM",
        res_group_info->pvm, 
        res_group,
        res_group_info->tex_count,
        0
    );

    return 1;
}

void CourseMenuFreeResourceGroup_8c0185c4(ResourceGroup *res_group)
{
    if (res_group->tlist_0x00 == (void *) -1) {
        return;
    }
    AsqReleaseAndFreeTexlist_8c011e3c(res_group->tlist_0x00);
    syFree(res_group->contents_0x08);
    syFree(res_group->tanim_0x04);
    res_group->tlist_0x00 = (void *) -1;
}


/* ===================
 * Initialized Globals
 * ===================
 */

 /*  0  1    2  3  4
  *  5  6    7  8  9
  * 10 11   12 13 14  */
STATIC CourseMenuButton init_courseMenuButtons_8c04442c[15] = {
    {   /* [0] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 45.0f, 109.0f,
        /* spriteNo */ 0,
        /* onSelect */ FUN_8c01f114,
        /* courseId */ 0,
    },
    {   /* [1] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 110.0f, 109.0f,
        /* spriteNo */ 0,
        /* onSelect */ FUN_8c01ba64,
        /* courseId */ 0,
    },
    {   /* [2] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 190.0f, 109.0f,
        /* spriteNo */ 11,
        /* onSelect */ courseMenuConfirmInit_8c0184cc,
        /* courseId */ 0,
    },
    {   /* [3] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 284.0f, 109.0f,
        /* spriteNo */ 12,
        /* onSelect */ courseMenuConfirmInit_8c0184cc,
        /* courseId */ 3,
    },
    {   /* [4] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 377.0f, 109.0f,
        /* spriteNo */ 13,
        /* onSelect */ courseMenuConfirmInit_8c0184cc,
        /* courseId */ 6,
    },
    {   /* [5] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 45.0f, 182.0f,
        /* spriteNo */ 0,
        /* onSelect */ FUN_8c01d1c4,
        /* courseId */ 0,
    },
    {   /* [6] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 109.0f, 182.0f,
        /* spriteNo */ 0,
        /* onSelect */ AlbumSwitchFromTask_8c01d6e2,
        /* courseId */ 0,
    },
    {   /* [7] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 190.0f, 182.0f,
        /* spriteNo */ 14,
        /* onSelect */ courseMenuConfirmInit_8c0184cc,
        /* courseId */ 9,
    },
    {   /* [8] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 285.0f, 182.0f,
        /* spriteNo */ 15,
        /* onSelect */ courseMenuConfirmInit_8c0184cc,
        /* courseId */ 12,
    },
    {   /* [9] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 378.0f, 182.0f,
        /* spriteNo */ 16,
        /* onSelect */ courseMenuConfirmInit_8c0184cc,
        /* courseId */ 15,
    },
    {   /* [10] */
        /* enabled  */ 0,
        /* unlocked */ 0,
        /* x, y     */ 44.0f, 256.0f,
        /* spriteNo */ 0,
        /* onSelect */ NULL,
        /* courseId */ 0,
    },
    {   /* [11] */
        /* enabled  */ 0,
        /* unlocked */ 0,
        /* x, y     */ 108.0f, 256.0f,
        /* spriteNo */ 0,
        /* onSelect */ NULL,
        /* courseId */ 0,
    },
    {   /* [12] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 191.0f, 256.0f,
        /* spriteNo */ 17,
        /* onSelect */ courseMenuConfirmInit_8c0184cc,
        /* courseId */ 18,
    },
    {   /* [13] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 285.0f, 256.0f,
        /* spriteNo */ 18,
        /* onSelect */ courseMenuConfirmInit_8c0184cc,
        /* courseId */ 21,
    },
    {   /* [14] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 378.0f, 256.0f,
        /* spriteNo */ 19,
        /* onSelect */ courseMenuConfirmInit_8c0184cc,
        /* courseId */ 24,
    },
};

STATIC MenuDialog init_seqStoryIntro_8c0445d0[] = {
    { "やぁ、よく来たね<E>私がここの所長だ、以後よろしく", 1 },
    { "今日から一ヶ月間、キミには<E>ここで実地研修を受けてもらう", 0 },
    { "この研修に合格すると<E>キミは晴れて都営バスの運転手だ", 0 },
    { "最後の研修だが<E>気を抜かずに頑張ってくれよ", 0 },
    { "まずは比較的簡単な〈梅７６〉と<E>〈虹０１〉の２コースで研修しよう", 0 },
    { "視界も広く、交通量も多くない時間帯を<E>選んでいるので、研修にはちょうどいいだろう", 0 },
    { "慣れてきたら、少しづつ難しいコースも<E>走れるようになるからな", 0 },
    { "いきなりコースを走るのが不安なら<E>練習することもできるぞ", 0 },
    { "練習したい場合は画面左上の<E>ＰＲＡＣＴＩＣＥを選ぶといい", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqSuccessPerfect_8c044620[] = {
    { "キミの成績には、正直、びっくりしたよ<E>実に素晴らしい！", 0 },
    { "どのコースをとっても<E>まさにパーフェクト！", 0 },
    { "おめでとう<E>実地研修は満点で合格だ！", 2 },
    { "キミならどこに配属されても大丈夫<E>研修を担当した私も鼻が高いよ", 2 },
    { "都営バスの未来を担う一員として<E>キミには期待しているぞ", 3 },
    { "", 0 },
};

STATIC MenuDialog init_seqSuccessHigh_8c044650[] = {
    { "昨日で無事、実地研修が終了した<E>本当によく頑張ったな", 0 },
    { "キミの成績を見せてもらったが<E>申し分のない成績だったよ", 0 },
    { "全コースとも高得点をあげている<E>これは並大抵の事ではない", 0 },
    { "おめでとう<E>実地研修はもちろん合格だ！", 2 },
    { "今日からキミは都営バスの立派な運転手だ<E>これからもよろしく頼むぞ！", 3 },
    { "", 0 },
};

STATIC MenuDialog init_seqSuccessNormal_8c044680[] = {
    { "昨日で無事、実地研修が終了した<E>本当によく頑張ったな", 0 },
    { "キミの成績を見せてもらったが<E>まぁ、十分に合格点をあげられるだろう", 0 },
    { "各コース、致命的なミスはなく<E>成績も悪くない", 0 },
    { "これからも注意を怠ることなく<E>この調子でますます努力・精進してくれ", 0 },
    { "おめでとう<E>実地研修は合格だ！", 2 },
    { "明日からも気を抜くことなく、<E>研修の延長線だと思って、業務に取り組んでくれ", 3 },
    { "", 0 },
};

STATIC MenuDialog init_seqFailureFinal_8c0446b8[] = {
    { "１ヶ月間本当によく頑張ったな", 0 },
    { "…と言いたい所なんだが<E>合格点には少し及ばなかったようだ", 1 },
    { "運転手は、乗客の命を預かっているんだ<E>軽率な行動やミスは許されない", 1 },
    { "キミはもう少し研修を続けた方が<E>良さそうだな、頑張ってくれよ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqFreeRunIntro_8c0446e0[] = {
    { "ここでは、好きなコースを<E>何度でも走る事ができるぞ", 0 },
    { "但しストーリーモードで走ったコース限る", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqStoryChooseCourse_8c0446f8[] = {
    { "さぁ、今日の研修を選んでくれ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqGoodPractice_8c044708[] = {
    { "昨日は充実した練習が出来たようだね", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqSuccess_8c044718[] = {
    { "昨日はなんとかうまく走れたようだね<E>その調子で頑張ってくれよ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqAwardBadgeGold_8c044728[] = {
    { "昨日は素晴らしい成績だったな", 0 },
    { "キミには優秀ドライバーの証し<E>ゴールドバッジを与えよう", 0 },
    { "この調子で頑張ってくれよ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqAwardBadgeSilver_8c044748[] = {
    { "キミには優秀ドライバーの証し<E>シルバーバッジを与えよう", 1 },
    { "もっと高得点が出れば、さらに上の<E>ゴールドバッジもあるからな", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqAwardBadgeBronze_8c044760[] = {
    { "キミには優秀ドライバーの証し<E>ブロンズバッジを与えよう", 1 },
    { "もっと高得点が出れば、さらに上の<E>シルバーバッジやゴールドバッジもあるぞ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqFailureRetry_8c044778[] = {
    { "昨日は残念な結果になってしまったな<E>まぁ、誰にだって失敗はある", 0 },
    { "特に慣れないうちは仕方ないさ<E>気分を改めて今日も頑張ってくれよ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqCourseUnlocked_8c044790[] = {
    { "それから…経験も積んだようなので<E>そろそろ違うコースで研修してみよう", 0 },
    { "今までより、少し難しくなっていると思うが<E>キミなら大丈夫だ", 0 },
    { "勇気を持ってチャレンジしてくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqPassengerLetter_8c0447b0[] = {
    { "そうそう、今日はお客さんから手紙が届いたよ", 0 },
    { "掲示板に張っておいたから、<E>見るときはＡＬＢＵＭを選んでくれ", 0 },
    { "お客さんってありがたいもんだね<E>運転手やってて本当に良かったって実感するよ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqCourseLocked_8c0447d0[] = {
    { "残念だがそのコースはまだ走れないよ<E>もっと経験を積んでからだな", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqForcePractice_8c0447e0[] = {
    { "はやる気持ちも分からんでもないが、<E>今日は練習してもらうよ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqFinalDay_8c0447f0[] = {
    { "長かった研修も今日が最後だな", 0 },
    { "さぁ、最後の研修を選んでくれ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonIntro_8c044808[] = {
    { "今日は初日なので、全てのＬＥＳＳＯＮを<E>順番にやってもらうよ", 0 },
};

// Unused?
STATIC MenuDialog init_8c044810[] = {
    { "合格ラインは７０点。決して難しい事では<E>ないので、落ち着いて慎重に進めていこう", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonComplete_8c044820[] = {
    { "おめでとう<E>これで全てのＬＥＳＳＯＮは終了だ", 0 },
    { "明日からは実際の街中にでて<E>実地研修を行うとしよう", 0 },
    { "もっとも、運行に何か不安があったら<E>ここに来て練習すればいい", 0 },
    { "次からは好きなＬＥＳＳＯＮが選べるから<E>有効に活用してくれ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonNext_8c044848[] = {
    { "よーし、次のＬＥＳＳＯＮに進もう", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonRetry_8c044858[] = {
    { "誰だって最初はうまくできないものさ<E>気を落とさないで、もう一度やってみよう", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonTips_8c044868[] = {
    { "ここでは運行中の諸注意を<E>ポイント毎に練習できるぞ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonWarning_8c044878[] = {
    { "一度ＬＥＳＳＯＮを選ぶと、今日はコースに<E>出られなくなるので、気を付けるように", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonChoose_8c044888[] = {
    { "練習したいＬＥＳＳＯＮを選んでくれ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqScoreRecord_8c044898[] = {
    { "そうそう、最高得点が更新されたようだ<E>この調子で頑張ってくれよ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonFinalDay_8c0448a8[] = {
    { "今日は研修の最終日だぞ<E>もう練習しているより、実戦あるのみ", 0 },
    { "まぁ、どうしてもと言うのなら止めはしないが", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonPerfect_8c0448c0[] = {
    { "素晴らしい！<E>パーフェクトで合格だ", 0 },
    { "この調子で他のＬＥＳＳＯＮはもちろん<E>コースの方も頑張ってくれよ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonGood_8c0448d8[] = {
    { "しかし、ミスはその１カ所だけのようだな", 0 },
    { "他はパーフェクトだっただけに<E>悔やまれるが、十分に合格点だ！", 0 },
    { "今度走る時はパーフェクトを<E>目指してみてはどうかな？", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonPass_8c0448f8[] = {
    { "まぁ、他にもミスはあったが<E>致命的なものではないようだな", 0 },
    { "少し不安は残るが、一応は合格点だ<E>次はもっと高得点を目指してくれよ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonFailMinor_8c044910[] = {
    { "ミスはその１カ所だけのようだが<E>状況によっては大事故になる危険なミスだぞ", 1 },
    { "残念だが今回は不合格だな<E>気を取り直して、次は頑張ってくれよ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqLessonFailMajor_8c044928[] = {
    { "…その他にもミスが目立つようだ<E>とても合格点はあげられないな", 1 },
    { "乗客を乗せているということは、<E>命を預かっているのと同じことなんだ", 1 },
    { "我々の不注意が、多くの人々を危険にさらして<E>しまう事を、よく肝に銘じて練習に励んでくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqCollisionCarMinor_8c044948[] = {
    { "他の車に接触してしまったようだな…", 1 },
    { "車間距離を十分に取り、<E>早めにブレーキをかけるようにしてくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqCollisionCarMedium_8c044960[] = {
    { "スピードは出ていなかったようだが<E>停車車両に接触してしまったな…", 1 },
    { "一日も早く、車両感覚を身につけてくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqCollisionCarSevere_8c044978[] = {
    { "停車車両に接触してしまったようだな…<E>しかもスピードが大分出ていたな", 1 },
    { "一日も早く、車両感覚を身につけてくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqCollisionCarFatal_8c044990[] = {
    { "停車車両に接触してしまったようだな…<E>しかも凄いスピードで…", 1 },
    { "まず速度はむやみに出さないこと！<E>そして車両感覚をしっかり身につけてくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqCollisionWallMinor_8c0449a8[] = {
    { "スピードは出ていなかったようだが<E>壁に接触してしまったな…", 1 },
    { "一日も早く、車両感覚を身につけてくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqCollisionWallMedium_8c0449c0[] = {
    { "壁に接触してしまったようだな…<E>しかもスピードが大分出ていたな", 1 },
    { "一日も早く、車両感覚を身につけてくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqCollisionWallSevere_8c0449d8[] = {
    { "壁に接触してしまったようだな…<E>しかも凄いスピードで…", 1 },
    { "まず速度はむやみに出さないこと！<E>そして車両感覚をしっかり身につけてくれ", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqNearMissPedestrian_8c0449f0[] = {
    { "ふー、危機一髪だったな", 1 },
    { "おいおい、通行人の安全は<E>何に替えても守ってくれよ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqOffCourseMinor_8c044a08[] = {
    { "コースを外れてしまったようだな…<E>コースは常に正確に走ってくれないと困るぞ", 1 },
    { "コースから外れかけても、慌てずに速度を落とし<E>正しいコースに戻ればいい", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqOffCourseMedium_8c044a20[] = {
    { "コースを外れてしまったようだな…<E>コースは常に正確に走ってくれないと困るぞ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqOffCourseMajor_8c044a30[] = {
    { "コースを外れてしまったようだな…<E>コースは常に正確に走ってくれないと困るぞ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqSpeedingMinor_8c044a40[] = {
    { "少しスピードオーバーしたみたいだな…<E>制限速度には常に注意を払ってくれよ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqSpeedingMajor_8c044a50[] = {
    { "大分スピードオーバーしたようだな…<E>制限速度を守るのは安全運転の基本だぞ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqWrongLane_8c044a60[] = {
    { "対向車線を越えるとは言語道断！<E>大事故の元なので、以後、絶対にないように", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqLaneStraddle_8c044a70[] = {
    { "車体が大きいので大変なのは分かるが<E>車線をまたがって走ってはいかんぞ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqNoSignal_8c044a80[] = {
    { "ウインカーを忘れていたぞ<E>車線変更や交差点での右左折", 1 },
    { "そして停留所に停車する時は<E>必ずウインカーを出すようにな", 1 },
    { "大切なのは、自分の行動を<E>周りの車にちゃんと知らせることだ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqNoSignalTurn_8c044aa0[] = {
    { "右左折する時のウインカーを忘れていたぞ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqUkn49_8c044ab0[] = {
    { "", 0 },
};

STATIC MenuDialog init_seqSignalViolation_8c044ab8[] = {
    { "おいおい、大事故になる所だったぞ", 1 },
    { "信号はしっかりと確認してくれよ<E>交通法規の基本中の基本だぞ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqBadStopLine_8c044ad0[] = {
    { "停止時にちょっと失敗したな", 1 },
    { "停止線をきちんと確認して、<E>正しい位置に停止するようにしてくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqIllegalLaneChange_8c044ae8[] = {
    { "してはいけない所で車線変更をしたようだな…", 1 },
    { "交差点の手前などでは禁止されているので<E>気を付けてくれよ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqBlockIntersection_8c044b00[] = {
    { "交差点での立ち往生は<E>絶対にしてはいけないぞ", 1 },
    { "事故や渋滞を引き起こす危険があるからな…<E>交差点は素早く通り抜けることだ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqWrongWay_8c044b18[] = {
    { "コースを正しく走る<E>これが運転の基本だ", 1 },
    { "落ち着いて走れば逆走などするわけがない<E>大事故になるので絶対にやめてくれよ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqRapidAccel_8c044b30[] = {
    { "基本的には、お客さんあってのバスだからね<E>急発進などしないように気を付けよう", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqHardBrake_8c044b40[] = {
    { "急ブレーキがあったようだな<E>今度はお客さんの身になって運転してみるといい", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqSwerving_8c044b50[] = {
    { "荒っぽい運転をすると<E>立っているお客さんは大変だぞ", 1 },
    { "もっと安全運転を心がけて<E>急ハンドルなんてしないようにな", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqMissedStop_8c044b68[] = {
    { "バスが停留所を通り過ぎてどうするんだ", 1 },
    { "きちんと停留所情報を確認して<E>必要な停留所では停車してくれよ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqBadStopPosition1_8c044b80[] = {
    { "停留所の停車位置が良くなかったようだな", 1 },
    { "今度からは、お客さんが乗り降り<E>しやすいように、正確に停車してくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqBadStopPosition2_8c044b98[] = {
    { "停留所の停車位置が良くなかったようだな", 1 },
    { "今度からは、お客さんが乗り降り<E>しやすいように、正確に停車してくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqTimeManagement_8c044bb0[] = {
    { "運転技術はもちろんのこと<E>時間配分にも気を配ってくれ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqAnnouncement_8c044bc0[] = {
    { "バスの運転手がお客さんにしなければならない<E>こととして、車内アナウンスは重要だぞ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqDoorOperation_8c044bd0[] = {
    { "ドアの開閉の確認は基本事項だぞ", 1 },
    { "", 0 },
};

STATIC MenuDialog init_seqFreeRunIntro2_8c044be0[] = {
    { "ここでは、好きなコースを<E>何度でも走る事ができるぞ", 0 },
    { "但しストーリーモードで<E>走ったコースに限るがな", 0 },
    { "", 0 },
};

STATIC MenuDialog init_seqFreeRunChooseCourse_8c044bf8[] = {
    { "さぁ、コースを選んでくれ", 0 },
    { "", 0 },
};

MenuDialog *init_dialogSequences_8c044c08[] = {
    init_seqStoryIntro_8c0445d0,
    init_seqSuccessPerfect_8c044620,
    init_seqSuccessHigh_8c044650,
    init_seqSuccessNormal_8c044680,
    init_seqFailureFinal_8c0446b8,
    init_seqFreeRunIntro_8c0446e0,
    init_seqStoryChooseCourse_8c0446f8,
    init_seqGoodPractice_8c044708,
    init_seqSuccess_8c044718,
    init_seqAwardBadgeGold_8c044728,
    init_seqAwardBadgeSilver_8c044748,
    init_seqAwardBadgeBronze_8c044760,
    init_seqFailureRetry_8c044778,
    init_seqCourseUnlocked_8c044790,
    init_seqPassengerLetter_8c0447b0,
    init_seqCourseLocked_8c0447d0,
    init_seqForcePractice_8c0447e0,
    init_seqFinalDay_8c0447f0,
    init_seqLessonIntro_8c044808,
    init_seqLessonComplete_8c044820,
    init_seqLessonNext_8c044848,
    init_seqLessonRetry_8c044858,
    init_seqLessonTips_8c044868,
    init_seqLessonWarning_8c044878,
    init_seqLessonChoose_8c044888,
    init_seqScoreRecord_8c044898,
    init_seqLessonFinalDay_8c0448a8,
    init_seqLessonPerfect_8c0448c0,
    init_seqLessonGood_8c0448d8,
    init_seqLessonPass_8c0448f8,
    init_seqLessonFailMinor_8c044910,
    init_seqLessonFailMajor_8c044928,
    init_seqCollisionCarMinor_8c044948,
    init_seqCollisionCarMedium_8c044960,
    init_seqCollisionCarSevere_8c044978,
    init_seqCollisionCarFatal_8c044990,
    init_seqCollisionWallMinor_8c0449a8,
    init_seqCollisionWallMedium_8c0449c0,
    init_seqCollisionWallSevere_8c0449d8,
    init_seqNearMissPedestrian_8c0449f0,
    init_seqOffCourseMinor_8c044a08,
    init_seqOffCourseMedium_8c044a20,
    init_seqOffCourseMajor_8c044a30,
    init_seqSpeedingMinor_8c044a40,
    init_seqSpeedingMajor_8c044a50,
    init_seqWrongLane_8c044a60,
    init_seqLaneStraddle_8c044a70,
    init_seqNoSignal_8c044a80,
    init_seqNoSignalTurn_8c044aa0,
    init_seqUkn49_8c044ab0,
    init_seqSignalViolation_8c044ab8,
    init_seqBadStopLine_8c044ad0,
    init_seqIllegalLaneChange_8c044ae8,
    init_seqBlockIntersection_8c044b00,
    init_seqWrongWay_8c044b18,
    init_seqRapidAccel_8c044b30,
    init_seqHardBrake_8c044b40,
    init_seqSwerving_8c044b50,
    init_seqMissedStop_8c044b68,
    init_seqBadStopPosition1_8c044b80,
    init_seqBadStopPosition2_8c044b98,
    init_seqTimeManagement_8c044bb0,
    init_seqAnnouncement_8c044bc0,
    init_seqDoorOperation_8c044bd0,
    init_seqFreeRunIntro2_8c044be0,
    init_seqFreeRunChooseCourse_8c044bf8
};

// 30 days -> course variant index (0-2)
Uint8 init_courseVariants_8c044d10[30] = {
    0, 0, 1, 2, 2, 0, 0,
    1, 1, 0, 2, 2, 0, 1,
    2, 1, 0, 2, 2, 0, 1,
    1, 2, 0, 2, 2, 0, 1,
    1, 0
};

// 3 courses -> 3 shifts -> hh, mm
Uint8 init_routeInfoTime_8c044d2e[3 * 3 * 2] = {
    // hh, mm
    12, 28,
    16, 52,
    20, 36,

    12, 05,
    16, 44,
    20, 48,

    12, 46,
    17, 04,
    19, 22
};

STATIC ResourceGroupInfo init_courseResourceGroup_8c044d40 = {
    "corse_parts.dat",
    "course.dat",
    "corse.pvm",
    4
};
