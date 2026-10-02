/* @unit CourseMenu */
#include <shinobi.h>
#include <sg_sd.h>
#include <njdef.h>
#include <sg_xpt.h>
#include "012324_input.h"
#include "0129cc_game.h"
#include "013ae8_route.h"
#include "015ab8_title.h"
#include "014a9c_tasks.h"
#include "011120_asset_queues.h"
#include "016c58_prompt.h"
#include "019e98_main_menu.h"
#include "016d2c_course_menu.h"
#include "0100bc_sound.h"
#include "01d290_album.h"
#include "01b19c_system_menu.h"
#include "01c980_profile_file.h"
#include "01e27c_practice_menu.h"
#include "02a9fc_message_box.h"
#include "014f54_sprite.h"
#include "022464_render.h"
#include "01614c_replay_menu.h"
#include "1ba1c8_globals.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "strings.h"

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

/* 3 rows x 5 columns; columns 0-1 are the side buttons, 2-4 the course grid. */
#define COURSE_BUTTON_COUNT 15
#define COURSE_COUNT 9

/* ====================
 * Type Declarations
 * ====================
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
    int state_0x00;
    InstructorLine *dialog_0x04;
    int charCount_0x08;
    int revealedCharCount_0x0c;
    int charRevealTimer_0x10;
    int bobAngle_0x14;
    /* Unused; the voice cues live in InstructorDialogTask.voiceCuePtr_0x18. */
    int *field_0x18;
} InstructorDialogState;

typedef struct {
    TaskAction action;
    void *state;
    int field_0x08;
    void* field_0x0c;
    int field_0x10;
    int field_0x14;
    int *voiceCuePtr_0x18;
    int field_0x1c;
} InstructorDialogTask;

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

/* ====================
 * Non-initialized Globals
 * ====================
 */

void* var_currentSysResGroupInfo_8c225fb0;
int var_instructorDialogActive_8c225fb4;
int var_menuTextboxCharLimit_8c225fb8;
int var_dialogQueue_8c225fbc[6];
STATIC Sint8 var_coursesToUnlock_8c225fd4[12];

/* ====================
 * Forward Declarations
 * ====================
 */

/* Defined after the functions so the shared "" literal is first seen in code
   and lands at the head of the constant pool. */
STATIC CourseMenuButton init_courseMenuButtons_8c04442c[COURSE_BUTTON_COUNT];
STATIC ResourceGroupInfo init_courseResourceGroup_8c044d40;

int CourseMenuRequestSysResgrp_8c018568(ResourceGroup* dds, ResourceGroupInfo* rg);
STATIC void courseMenuConfirmInit_8c0184cc(Task *task);
void CourseMenuFreeResourceGroup_8c0185c4(ResourceGroup *res_group);
STATIC void freeRunMenuTask_8c017ada(Task * task, void *state);
void CourseMenuRequestCommonResources_8c01852c(void);
InstructorLine *init_instructorDialogs_8c044c08[66];
Uint8 init_courseVariants_8c044d10[30];
Uint8 init_routeInfoTime_8c044d2e[3 * 3 * 2];

/* ====================
 * Functions
 * ====================
 */

/* Returns 1 once the cursor reaches its target. */
int CourseMenuInterpolateCursor_8c016d2c()
{
    var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x += var_menuState_8c1bc7a8.cursorVelocity_0x30.x;
    var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.y += var_menuState_8c1bc7a8.cursorVelocity_0x30.y;

    if (var_menuState_8c1bc7a8.cursorVelocity_0x30.x) {
        if (
            (var_menuState_8c1bc7a8.cursorVelocity_0x30.x >= 0)
            || (var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x > var_menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.x)
        ) {
            if (var_menuState_8c1bc7a8.cursorVelocity_0x30.x <= 0) {
                return 0;
            }

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

STATIC int startCursorMove_8c016dc6()
{
    int selected;
    float y;
    float x;

    selected = var_menuState_8c1bc7a8.cursorCol_0x3c + var_menuState_8c1bc7a8.cursorRow_0x40 * 5;
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
    MessageBoxSetText_8c02aefc("");
    return 1;
}

STATIC void drawInteger_8c016e6c(int value, float x, float y)
{
    do {
        SpriteDraw_8c014f54(
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

    days = var_progress_8c1ba1cc.days_0x00;
    if (days < 10) {
        x = 84.0;
    } else {
        x = 95.0;
    }
    drawInteger_8c016e6c(days, x, 82.0);

    /* The story runs through September; sprites 13 and 14 mark the two public
       holidays in it, in place of the weekday glyph. */
    if (days == 15) {
        sprite_id = 13;
    } else if (days == 23) {
        sprite_id = 14;
    } else {
        sprite_id = 6 + getWeekDayIndex_8c016ed2();
    }

    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00,
        sprite_id,
        112.0,
        82.0,
        -4.0
    );

    drawInteger_8c016e6c(var_progress_8c1ba1cc.exp_0x90, 534.0, 82.0);
}

STATIC void instructorDialogTask_8c016f98(InstructorDialogTask *task, InstructorDialogState *state)
{
    switch(state->state_0x00) {
        case 0: {
            int r;

            if (!*(state->dialog_0x04->text_0x00)) {
                var_instructorDialogActive_8c225fb4 = 0;
                TaskUninstall_8c014b66((void *) task);
                return;
            }

            if (task->voiceCuePtr_0x18 && *task->voiceCuePtr_0x18) {
                SndPlayAdx_8c010cd6(2, *task->voiceCuePtr_0x18);
                task->voiceCuePtr_0x18++;
            }

            state->charCount_0x08 = MessageBoxSetText_8c02aefc(state->dialog_0x04->text_0x00);
            var_menuState_8c1bc7a8.instructorSprite_0x60 = state->dialog_0x04->spriteNo_0x04;
            state->revealedCharCount_0x0c = 1;
            state->charRevealTimer_0x10 = 0;
            state->state_0x00 = 1;
            break;
        }

        case 1: {
            if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
                state->charRevealTimer_0x10 = 99;
                state->state_0x00 = 2;
                SndStopAdx_8c010ca6(1);
            }

            if (++state->charRevealTimer_0x10 < 3) {
                break;
            }

            if (++state->revealedCharCount_0x0c < state->charCount_0x08) {
                state->charRevealTimer_0x10 = 0;
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

            if ((state->revealedCharCount_0x0c += 2) < state->charCount_0x08) {
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

            state->bobAngle_0x14 += 0x1111;
            SpriteDraw_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupA_0x00,
                44,
                32.0,
                -16.0 + 8 * njCos(state->bobAngle_0x14),
                -3.0
            );

            break;
        }
    }

    var_menuTextboxCharLimit_8c225fb8 = state->revealedCharCount_0x0c;
}

void CourseMenuInstallDialog_8c0170c6(int dialog_index, int *voice_cues)
{
    InstructorDialogTask *task;
    InstructorDialogState *state;

    TaskInstall_8c014ae8(
        var_tasks_8c1ba3c8,
        &instructorDialogTask_8c016f98,
        &task,
        &state,
        0x18
    );

    task->voiceCuePtr_0x18 = voice_cues;
    state->state_0x00 = 0;
    state->dialog_0x04 = init_instructorDialogs_8c044c08[dialog_index];
    var_instructorDialogActive_8c225fb4 = 1;
}

STATIC void swapDialogMessageBox_8c017108(int sequence)
{
    var_menuTextboxCharLimit_8c225fb8 = MessageBoxSetText_8c02aefc(
        init_instructorDialogs_8c044c08[sequence]->text_0x00
    );
}

STATIC void handleCourseMenuInput_8c017126()
{
    if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
        if (
            init_courseMenuButtons_8c04442c[
                var_menuState_8c1bc7a8.cursorCol_0x3c
                + var_menuState_8c1bc7a8.cursorRow_0x40 * 5
            ]
            .unlocked_0x04 == 0
        ) {
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
            swapDialogMessageBox_8c017108(INSTR_COURSE_LOCKED);
        } else {
            SndStartAdxFadeOut_8c010bae(0);
            SndStartAdxFadeOut_8c010bae(1);
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            CHANGE_STATE(COURSE_MENU_STATE_COURSE_SELECTED);
            var_menuState_8c1bc7a8.timer_0x68 = 0;
        }
    }

    if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KU) {
        do {
            if (--var_menuState_8c1bc7a8.cursorRow_0x40 < 0) {
                var_menuState_8c1bc7a8.cursorRow_0x40 = 2;
            }
        } while (
            init_courseMenuButtons_8c04442c[
                var_menuState_8c1bc7a8.cursorRow_0x40 * 5 + var_menuState_8c1bc7a8.cursorCol_0x3c
            ].enabled_0x00 == 0
        );

        if (startCursorMove_8c016dc6()) {
            CHANGE_STATE(COURSE_MENU_STATE_ANIMATING);
        }
    } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KD) {
        do {
            if (++var_menuState_8c1bc7a8.cursorRow_0x40 > 2) {
                var_menuState_8c1bc7a8.cursorRow_0x40 = 0;
            }
        } while (
            init_courseMenuButtons_8c04442c[
                var_menuState_8c1bc7a8.cursorRow_0x40 * 5 + var_menuState_8c1bc7a8.cursorCol_0x3c
            ].enabled_0x00 == 0
        );

        if (startCursorMove_8c016dc6()) {
            CHANGE_STATE(COURSE_MENU_STATE_ANIMATING);
        }
    } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KL) {
        do {
            if (--var_menuState_8c1bc7a8.cursorCol_0x3c < 0) {
                var_menuState_8c1bc7a8.cursorCol_0x3c = 4;
            }
        } while (
            init_courseMenuButtons_8c04442c[
                var_menuState_8c1bc7a8.cursorRow_0x40 * 5 + var_menuState_8c1bc7a8.cursorCol_0x3c
            ].enabled_0x00 == 0
        );

        if (startCursorMove_8c016dc6()) {
            CHANGE_STATE(COURSE_MENU_STATE_ANIMATING);
        }
    } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KR) {
        do {
            if (++var_menuState_8c1bc7a8.cursorCol_0x3c > 4) {
                var_menuState_8c1bc7a8.cursorCol_0x3c = 0;
            }
        } while (
            init_courseMenuButtons_8c04442c[
                var_menuState_8c1bc7a8.cursorRow_0x40 * 5 + var_menuState_8c1bc7a8.cursorCol_0x3c
            ].enabled_0x00 == 0
        );

        if (startCursorMove_8c016dc6()) {
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

STATIC void buildStoryDialogQueue_8c017420(void)
{
    int cur = 0;

    if (var_runReportPending_8c1bb8b8 == 0) {
        var_dialogQueue_8c225fbc[cur++] = INSTR_STORY_CHOOSE_COURSE;
        var_dialogQueue_8c225fbc[cur]   = -1;
        return;
    }

    if (var_progress_8c1ba1cc.days_0x00 == 1) {
        var_dialogQueue_8c225fbc[cur++] = INSTR_STORY_INTRO;
        var_dialogQueue_8c225fbc[cur++] = INSTR_STORY_CHOOSE_COURSE;
        var_dialogQueue_8c225fbc[cur]   = -1;
        return;
    }

    if (var_runWasPractice_8c1bb8bc != 0) {
        var_dialogQueue_8c225fbc[cur++] = INSTR_GOOD_PRACTICE;
        var_dialogQueue_8c225fbc[cur++] = INSTR_STORY_CHOOSE_COURSE;
        var_dialogQueue_8c225fbc[cur]   = -1;
        return;
    }

    if (var_runSucceeded_8c1bb8dc == 0) {
        var_dialogQueue_8c225fbc[cur++] = INSTR_FAILURE_RETRY;
    } else {
        int award_seq = INSTR_SUCCESS;
        if      (var_award_8c1bb8f8 == AWARD_TIER_BRONZE) award_seq = INSTR_AWARD_BADGE_BRONZE;
        else if (var_award_8c1bb8f8 == AWARD_TIER_SILVER) award_seq = INSTR_AWARD_BADGE_SILVER;
        else if (var_award_8c1bb8f8 == AWARD_TIER_GOLD) award_seq = INSTR_AWARD_BADGE_GOLD;
        var_dialogQueue_8c225fbc[cur++] = award_seq;
    }

    if (CourseMenuBuildCourseUnlockList_8c0172dc() != 0) {
        var_dialogQueue_8c225fbc[cur++] = INSTR_COURSE_UNLOCKED;
    }

    // Every seventh day, one of the six letters at random -- and none at all if
    // that one is already held.
    if (((var_progress_8c1ba1cc.days_0x00 + 1) % 7) == 0) {
        int r = AsqGetRandomInRangeB_8c0121be(6);
        if (var_progress_8c1ba1cc.letters_0x2c[r] == 0) {
            var_progress_8c1ba1cc.letters_0x2c[r] = 1;
            var_dialogQueue_8c225fbc[cur++] = INSTR_PASSENGER_LETTER;
        }
    }

    var_dialogQueue_8c225fbc[cur++] = INSTR_STORY_CHOOSE_COURSE;

    var_dialogQueue_8c225fbc[cur] = -1;
}

STATIC void drawCourseButtons_8c017590()
{
    int i;

    if (var_menuState_8c1bc7a8.cursorVisible_0x48) {
        SpriteDraw_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
            0x18,
            var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x,
            var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.y,
            -3.0
        );
    }

    for (i = 0; i < COURSE_BUTTON_COUNT; i++) {
        CourseMenuButton *btn = &init_courseMenuButtons_8c04442c[i];

        if (btn->unlocked_0x04 == 0 || btn->spriteNo_0x10 == 0)
            continue;

        SpriteDraw_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
            btn->spriteNo_0x10,
            0.0,
            0.0,
            -4.0
        );
    }

    for (i = 0; i < COURSE_COUNT; i++) {
        char spriteNo = var_gameMode_8c1bb8fc == 0
            ? var_progress_8c1ba1cc.courses_0x44[i].storyAward_0x03
            : var_progress_8c1ba1cc.courses_0x44[i].freeRunAward_0x04;

        if (!spriteNo)
            continue;

        SpriteDraw_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
            0x18 - spriteNo,
            240.0 + (i % 3) * 93.0,
            106.0 + (i / 3) * 74.0,
            -3.5
        );
    }
}

STATIC void storyMenuTask_8c017718(Task * task, void *state)
{
    switch (var_menuState_8c1bc7a8.state_0x18) {
        case COURSE_MENU_STATE_INIT: {
            if (RouteGetLatch_8c01432a())
                return;

            AsqFreeQueues_8c011f7e();
            CHANGE_STATE(COURSE_MENU_STATE_FADE_IN);
            SndStopBgm_8c010d8a();
            SndPlayAdx_8c010cd6(0, 15);
            RenderStartFadeIn_8c022a9c(10);
            return;
        }

        case COURSE_MENU_STATE_FADE_IN: {
            if (var_isFading_8c226568 == 0) {
                CourseMenuInstallDialog_8c0170c6(var_dialogQueue_8c225fbc[0], 0);
                CHANGE_STATE(COURSE_MENU_STATE_DIALOG);
            }
            break;
        }

        case COURSE_MENU_STATE_DIALOG: {
            if (var_instructorDialogActive_8c225fb4) break;

            if (var_dialogQueue_8c225fbc[task->field_0x08] == INSTR_COURSE_UNLOCKED) {
                int row;
                CourseMenuApplyUnlocks_8c0173e6();
                for (row = 0; row < 3; row++) {
                    int col;
                    for (col = 0; col < 3; col++) {
                        init_courseMenuButtons_8c04442c[2 + row * 5 + col].unlocked_0x04 =
                            var_progress_8c1ba1cc.courses_0x44[row * 3 + col].unlocked_0x00;
                    }
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[5], 1, 0x16, 0);
            }

            /* field_0x08 is this task's cursor into var_dialogQueue_8c225fbc. */
            task->field_0x08++;

            if (var_dialogQueue_8c225fbc[task->field_0x08] == -1) {
                CHANGE_STATE(COURSE_MENU_STATE_IDLE);
                MessageBoxSetText_8c02aefc("");
            }
            else {
                CourseMenuInstallDialog_8c0170c6(var_dialogQueue_8c225fbc[task->field_0x08], 0);
                if (var_dialogQueue_8c225fbc[task->field_0x08] == INSTR_COURSE_UNLOCKED) {
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
            if (++var_menuState_8c1bc7a8.timer_0x68 > 10) {
                CHANGE_STATE(COURSE_MENU_STATE_FADE_OUT);
                RenderStartFadeOut_8c022b60(10);
            }
            var_menuState_8c1bc7a8.cursorVisible_0x48 = var_menuState_8c1bc7a8.timer_0x68 & 1;
            break;
        }

        case COURSE_MENU_STATE_FADE_OUT: {
            int buttonIndex;

            if (var_isFading_8c226568) {
                var_menuState_8c1bc7a8.cursorVisible_0x48 = ++var_menuState_8c1bc7a8.timer_0x68 & 1;
                break;
            }

            if (init_adxPlaying_8c03bd80)
                return;

            if (var_menuState_8c1bc7a8.cursorCol_0x3c != 1 || var_menuState_8c1bc7a8.cursorRow_0x40 != 0) {
                CourseMenuFreeResourceGroup_8c0185c4(&var_menuState_8c1bc7a8.resourceGroupB_0x0c);
                var_currentSysResGroupInfo_8c225fb0 = (void *) -1;
            }

            var_menuState_8c1bc7a8.selected_0x38 = 0;
            buttonIndex = var_menuState_8c1bc7a8.cursorRow_0x40 * 5 + var_menuState_8c1bc7a8.cursorCol_0x3c;
            var_menuState_8c1bc7a8.courseId_0x50 =
                init_courseMenuButtons_8c04442c[buttonIndex].courseId_0x18;

            var_runSucceeded_8c1bb8dc = 1;
            var_runReportPending_8c1bb8b8 = 0;
            var_runWasPractice_8c1bb8bc = 1;

            init_courseMenuButtons_8c04442c[buttonIndex].onSelect_0x14(task);
            return;
        }

        case COURSE_MENU_STATE_FADE_OUT_TO_MAIN_MENU: {
            if (var_isFading_8c226568)
                break;

            if (init_adxPlaying_8c03bd80)
                return;

            var_runReportPending_8c1bb8b8 = 0;
            MainMenuEnter_8c01a09a(task, var_menuState_8c1bc7a8.subState_0x1c);
            return;
        }
    }

    CourseMenuDrawDateAndExp_8c016ee6();
    drawCourseButtons_8c017590();
    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c, 10, 0.0, 0.0, -5.0
    );
    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00, 0x2b, 0.0, 0.0, -4.0
    );
    if (MessageBoxDrawText_8c02af1c(var_menuTextboxCharLimit_8c225fb8) ) {
        SpriteDraw_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupA_0x00, 1, 0.0, 0.0, -5.0
        );
    }

    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_menuState_8c1bc7a8.instructorSprite_0x60,
        0.0,
        0.0,
        -6.0
    );

    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00, 0, 0.0, 0.0, -7.0
    );
    AsqGetRandomA_8c012166();
}

/*
 * Free-run twin of storyMenuTask_8c017718. Differences: no run/dialog
 * flags are set on selection, the main-menu row is fixed at 1 rather than taken
 * from subState_0x1c, and the header (date/EXP and sprite 0x2b) is not drawn --
 * the panel sprite is 9 instead of 10.
 */
STATIC void freeRunMenuTask_8c017ada(Task * task, void *state)
{
    switch (var_menuState_8c1bc7a8.state_0x18) {
        case COURSE_MENU_STATE_INIT: {
            if (RouteGetLatch_8c01432a())
                return;

            AsqFreeQueues_8c011f7e();
            CHANGE_STATE(COURSE_MENU_STATE_FADE_IN);
            SndStopBgm_8c010d8a();
            SndPlayAdx_8c010cd6(0, 15);
            RenderStartFadeIn_8c022a9c(10);
            return;
        }

        case COURSE_MENU_STATE_FADE_IN: {
            if (var_isFading_8c226568 == 0) {
                CourseMenuInstallDialog_8c0170c6(var_dialogQueue_8c225fbc[0], 0);
                CHANGE_STATE(COURSE_MENU_STATE_DIALOG);
            }
            break;
        }

        case COURSE_MENU_STATE_DIALOG: {
            if (var_instructorDialogActive_8c225fb4) break;

            if (var_dialogQueue_8c225fbc[task->field_0x08] == INSTR_COURSE_UNLOCKED) {
                int row;
                CourseMenuApplyUnlocks_8c0173e6();
                for (row = 0; row < 3; row++) {
                    int col;
                    for (col = 0; col < 3; col++) {
                        init_courseMenuButtons_8c04442c[2 + row * 5 + col].unlocked_0x04 =
                            var_progress_8c1ba1cc.courses_0x44[row * 3 + col].unlocked_0x00;
                    }
                }
                sdMidiPlay(var_midiHandles_8c0fcd28[5], 1, 0x16, 0);
            }

            /* field_0x08 is this task's cursor into var_dialogQueue_8c225fbc. */
            task->field_0x08++;

            if (var_dialogQueue_8c225fbc[task->field_0x08] == -1) {
                CHANGE_STATE(COURSE_MENU_STATE_IDLE);
                MessageBoxSetText_8c02aefc("");
            }
            else {
                CourseMenuInstallDialog_8c0170c6(var_dialogQueue_8c225fbc[task->field_0x08], 0);
                if (var_dialogQueue_8c225fbc[task->field_0x08] == INSTR_COURSE_UNLOCKED) {
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
            if (++var_menuState_8c1bc7a8.timer_0x68 > 10) {
                CHANGE_STATE(COURSE_MENU_STATE_FADE_OUT);
                RenderStartFadeOut_8c022b60(10);
            }
            var_menuState_8c1bc7a8.cursorVisible_0x48 = var_menuState_8c1bc7a8.timer_0x68 & 1;
            break;
        }

        case COURSE_MENU_STATE_FADE_OUT: {
            int buttonIndex;

            if (var_isFading_8c226568) {
                var_menuState_8c1bc7a8.cursorVisible_0x48 = ++var_menuState_8c1bc7a8.timer_0x68 & 1;
                break;
            }

            if (init_adxPlaying_8c03bd80)
                return;

            if (var_menuState_8c1bc7a8.cursorCol_0x3c != 1 || var_menuState_8c1bc7a8.cursorRow_0x40 != 0) {
                CourseMenuFreeResourceGroup_8c0185c4(&var_menuState_8c1bc7a8.resourceGroupB_0x0c);
                var_currentSysResGroupInfo_8c225fb0 = (void *) -1;
            }

            var_menuState_8c1bc7a8.selected_0x38 = 0;
            buttonIndex = var_menuState_8c1bc7a8.cursorRow_0x40 * 5 + var_menuState_8c1bc7a8.cursorCol_0x3c;
            var_menuState_8c1bc7a8.courseId_0x50 =
                init_courseMenuButtons_8c04442c[buttonIndex].courseId_0x18;

            init_courseMenuButtons_8c04442c[buttonIndex].onSelect_0x14(task);
            return;
        }

        case COURSE_MENU_STATE_FADE_OUT_TO_MAIN_MENU: {
            if (var_isFading_8c226568)
                break;

            if (init_adxPlaying_8c03bd80)
                return;

            MainMenuEnter_8c01a09a(task, 1);
            return;
        }
    }

    drawCourseButtons_8c017590();
    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c, 9, 0.0, 0.0, -5.0
    );
    if (MessageBoxDrawText_8c02af1c(var_menuTextboxCharLimit_8c225fb8) ) {
        SpriteDraw_8c014f54(
            &var_menuState_8c1bc7a8.resourceGroupA_0x00, 1, 0.0, 0.0, -5.0
        );
    }

    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_menuState_8c1bc7a8.instructorSprite_0x60,
        0.0,
        0.0,
        -6.0
    );

    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00, 0, 0.0, 0.0, -7.0
    );
    AsqGetRandomA_8c012166();
}

STATIC void buildFreeRunDialogQueue_8c017a20(void)
{
    int idx = 0;

    if (var_shouldShowFreeRunIntro_8c1bb8c0) {
        var_dialogQueue_8c225fbc[idx++] = INSTR_FREE_RUN_INTRO_2;
    }

    var_dialogQueue_8c225fbc[idx++] = INSTR_FREE_RUN_CHOOSE_COURSE;
    var_dialogQueue_8c225fbc[idx]   = -1;

    var_shouldShowFreeRunIntro_8c1bb8c0 = 0;
}

/* Also snaps the cursor onto the current button. */
STATIC void refreshCourseGrid_8c017d54(void)
{
    int enabled;
    int row;
    int game_mode = var_gameMode_8c1bb8fc;

    var_menuState_8c1bc7a8.cursorVisible_0x48 = 1;

    startCursorMove_8c016dc6();

    var_menuState_8c1bc7a8.pos.cursor.cursor_0x20 = var_menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28;

    // PROFILE FILE and ALBUM are story-mode only
    enabled = game_mode == 0 ? 1 : 0;
    init_courseMenuButtons_8c04442c[5].enabled_0x00 = enabled;
    init_courseMenuButtons_8c04442c[6].enabled_0x00 = enabled;

    /* Free run lights a course by new_0x01 rather than unlocked_0x00. */
    for (row = 0; row < 3; row++) {
        int col;
        for (col = 0; col < 3; col++) {
            int courseIdx = row * 3 + col;
            int buttonIdx = 2 + row * 5 + col;
            init_courseMenuButtons_8c04442c[buttonIdx].unlocked_0x04 =
                game_mode == 0
                    ? var_progress_8c1ba1cc.courses_0x44[courseIdx].unlocked_0x00
                    : var_progress_8c1ba1cc.courses_0x44[courseIdx].new_0x01;
        }
    }
}

void CourseMenuEnter_8c017e18(Task *task)
{
    LOG_INFO(("[COURSE_MENU] Initializing course menu (mode=%d)\n", var_gameMode_8c1bb8fc));

    if (var_gameMode_8c1bb8fc == 0) {
        TaskReinstall_8c014b3e(task, storyMenuTask_8c017718);
        buildStoryDialogQueue_8c017420();
    } else {
        TaskReinstall_8c014b3e(task, freeRunMenuTask_8c017ada);
        buildFreeRunDialogQueue_8c017a20();
    }

    var_menuState_8c1bc7a8.instructorSprite_0x60 =
        init_instructorDialogs_8c044c08[
            var_dialogQueue_8c225fbc[0]
        ]->spriteNo_0x04;
    task->field_0x08 = 0;
    var_menuTextboxCharLimit_8c225fb8 = 0;
    var_playMode_8c1bb8d0 = 0;
    refreshCourseGrid_8c017d54();
    njGarbageTexture(var_tex_8c157af8, 0xc00);
    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();

    if (!CourseMenuRequestSysResgrp_8c018568(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        &init_mainMenuResourceGroup_8c044264
    )) {
        AsqFreeQueues_8c011f7e();
        CHANGE_STATE(COURSE_MENU_STATE_FADE_IN);
        RenderStartFadeIn_8c022a9c(10);
        SndPlayAdx_8c010cd6(0, 15);
        return;
    }

    RouteSetLatch_8c014330();
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteClearLatch_8c014322);
    CHANGE_STATE(COURSE_MENU_STATE_INIT);
}

/* Installs the menu's tasks from scratch, where CourseMenuEnter_8c017e18
 * reinstalls an existing task. */
void CourseMenuReturn_8c017ef2(void)
{
    Task *installedTask;
    void *installedState;

    LOG_INFO(("[COURSE_MENU] Setting up story course menu\n"));

    InputInstall_8c0128cc(0);

    TaskInstall_8c014ae8(
        var_tasks_8c1ba3c8,
        &GameSoftResetTask_8c012f44,
        &installedTask,
        &installedState,
        0
    );

    if (var_gameMode_8c1bb8fc == 0) {
        TaskInstall_8c014ae8(
            var_tasks_8c1ba3c8,
            &storyMenuTask_8c017718,
            &installedTask,
            &installedState,
            0
        );
        buildStoryDialogQueue_8c017420();
    } else {
        TaskInstall_8c014ae8(
            var_tasks_8c1ba3c8,
            &freeRunMenuTask_8c017ada,
            &installedTask,
            &installedState,
            0
        );
        buildFreeRunDialogQueue_8c017a20();
    }

    var_menuState_8c1bc7a8.instructorSprite_0x60 =
        init_instructorDialogs_8c044c08[
            var_dialogQueue_8c225fbc[0]
        ]->spriteNo_0x04;

    installedTask->field_0x08 = 0;

    var_menuTextboxCharLimit_8c225fb8 = 0;

    njGarbageTexture(var_tex_8c157af8, 0xc00);
    MessageBoxOpenTextbox_8c02ae3e(0x20, 0x180, -2.0, 0x240, 0x40, 0, 0, -1);
    MessageBoxSetText_8c02aefc("");
    var_playMode_8c1bb8d0 = 0;

    refreshCourseGrid_8c017d54();
    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();

    CourseMenuRequestSysResgrp_8c018568(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        &init_mainMenuResourceGroup_8c044264
    );
    CourseMenuRequestCommonResources_8c01852c();
    RouteSetLatch_8c014330();
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteClearLatch_8c014322);

    CHANGE_STATE(COURSE_MENU_STATE_INIT);
}

STATIC void drawFixedInteger_8c01803e(float x, float y, int value, int digits)
{
    float tracking = 19.0;
    do {
        do {
            SpriteDraw_8c014f54(
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
    int index = var_menuState_8c1bc7a8.cursorRow_0x40 * 6 + (var_menuState_8c1bc7a8.cursorCol_0x3c - 2) * 2;

    drawFixedInteger_8c01803e(219.0, 108.0, var_progress_8c1ba1cc.days_0x00, 0);

    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        getWeekDayIndex_8c016ed2() + 0x16,
        281.0,
        110.0,
        -4.0
    );

    drawFixedInteger_8c01803e(421.0, 108.0, init_routeInfoTime_8c044d2e[index], 2);
    drawFixedInteger_8c01803e(471.0, 108.0, init_routeInfoTime_8c044d2e[index + 1], 2);

    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_menuState_8c1bc7a8.cursorRow_0x40 + 9,
        0.0,
        0.0,
        -7.0
    );
}

STATIC void courseConfirmMenuTask_8c0181b6(Task * task, void *state)
{
    switch (var_menuState_8c1bc7a8.state_0x18) {
        case COURSE_CONFIRM_STATE_INIT: {
            if (RouteGetLatch_8c01432a())
                return;

            AsqFreeQueues_8c011f7e();
            CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_FADE_IN);
            RenderStartFadeIn_8c022a9c(10);
            SndPlayAdx_8c010cd6(0, 15);
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
                RenderStartFadeOut_8c022b60(10);
            } else if (r == 2) {
                CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_FADE_OUT_TO_COURSE_MENU);
                SndStartAdxFadeOut_8c010bae(0);
                SndStartAdxFadeOut_8c010bae(1);
                RenderStartFadeOut_8c022b60(10);
            }
            break;
        }

        case COURSE_CONFIRM_STATE_FADE_OUT: {
            if (var_isFading_8c226568 == 0) {
                CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_ROUTE_INFO_FADE_IN);
                RenderStartFadeIn_8c022a9c(0x14);
            }
            break;
        }

        case COURSE_CONFIRM_STATE_ROUTE_INFO_FADE_IN: {
            if (var_isFading_8c226568 == 0) {
                CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_ROUTE_INFO_DISPLAY);
                var_menuState_8c1bc7a8.timer_0x68 = 0;
            }
            drawRouteInfo_8c018118();
            return;
        }

        case COURSE_CONFIRM_STATE_ROUTE_INFO_DISPLAY: {
            var_menuState_8c1bc7a8.timer_0x68++;
            if (var_menuState_8c1bc7a8.timer_0x68 > 30) {
                CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_START_LOADING);
                SndStartAdxFadeOut_8c010bae(0);
                SndStartAdxFadeOut_8c010bae(1);
                RenderStartFadeOut_8c022b60(20);
            }
            drawRouteInfo_8c018118();
            return;
        }

        case COURSE_CONFIRM_STATE_START_LOADING: {
            if (var_isFading_8c226568 == 0) {
                int i = 0;
                int courseIndex = var_menuState_8c1bc7a8.courseId_0x50 / 3;

                /* Hold until both ADX streams have finished fading out. */
                if (init_adxPlaying_8c03bd80 != 0) {
                    return;
                }
                ReplayMenuFreeSessionAssets_8c016182();

                if (var_progress_8c1ba1cc.courses_0x44[courseIndex].everPlayed_0x02 == 0) {
                    var_firstRunOfCourse_8c1bb8e0 = 1;
                    var_progress_8c1ba1cc.courses_0x44[courseIndex].everPlayed_0x02 = 1;
                } else {
                    var_firstRunOfCourse_8c1bb8e0 = 0;
                }

                var_eventCount_8c1bb8e8 = 0;
                var_passengerCount_8c1bb8e4 = 0;
                var_worstPenaltyDelta_8c1bb8f0 = 0;
                var_worstPenaltyMsgSet_8c1bb8ec = 0x1d;
                var_penaltyCount_8c1bb8f4 = 0;

                /* Snapshot the progress flag words; retiring from the pause
                   menu restores them. */
                for (i = 0; i < 5; i++) {
                    var_eventFlagsSnapshot_8c1ba2b8[i] = ((int*)(&var_progress_8c1ba1cc.eventProgressFlags_0x04))[i];
                    var_profileFlagsSnapshot_8c1ba2cc[i] = ((int*)(&var_progress_8c1ba1cc.eventProgressFlags_0x04))[i + 5];
                }

                var_menuState_8c1bc7a8.courseId_0x50 += 
                    init_courseVariants_8c044d10[var_progress_8c1ba1cc.days_0x00 - 1];

                GameStartCourse_8c013310(var_menuState_8c1bc7a8.courseId_0x50);
                return;
            }
            drawRouteInfo_8c018118();
            return;
        }

        case COURSE_CONFIRM_STATE_FADE_OUT_TO_COURSE_MENU: {
            if (var_isFading_8c226568 == 0) {
                if (init_adxPlaying_8c03bd80 != 0) {
                    return;
                }

                CourseMenuFreeResourceGroup_8c0185c4(&var_menuState_8c1bc7a8.resourceGroupB_0x0c);
                var_currentSysResGroupInfo_8c225fb0 = (void *) -1;
                CourseMenuEnter_8c017e18(task);
                return;
            }
            break;
        }
    }

    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        var_menuState_8c1bc7a8.courseId_0x50 / 3,
        0.0,
        0.0,
        -4.0
    );

    // Confirm/cancel prompt
    SpriteDraw_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00,
        var_menuState_8c1bc7a8.selected_0x38 + 2,
        376.0,
        378.0,
        -4.0
    );

    // Foreground overlay
    SpriteDraw_8c014f54(
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
    TaskReinstall_8c014b3e(task, courseConfirmMenuTask_8c0181b6);
    CHANGE_CONFIRM_STATE(COURSE_CONFIRM_STATE_INIT);
    var_menuState_8c1bc7a8.selected_0x38 = 0;
    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();
    CourseMenuRequestSysResgrp_8c018568(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        &init_courseResourceGroup_8c044d40
    );
    RouteSetLatch_8c014330();
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteClearLatch_8c014322);
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

 /*
  * Cursor index is cursorCol_0x3c + cursorRow_0x40 * 5.
  * Columns 2-4 are the course grid, one row per route, one column per departure;
  * courseId is 3 * the courses_0x44 index.
  *
  *   0 LESSON   1 SYSTEM    2  3  4
  *   5 PROFILE  6 ALBUM     7  8  9
  *  10 --      11 --       12 13 14
  */
STATIC CourseMenuButton init_courseMenuButtons_8c04442c[COURSE_BUTTON_COUNT] = {
    {   /* [0] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 45.0f, 109.0f,
        /* spriteNo */ 0,
        /* onSelect */ PracticeMenuEnter_8c01f114,
        /* courseId */ 0,
    },
    {   /* [1] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 110.0f, 109.0f,
        /* spriteNo */ 0,
        /* onSelect */ SystemMenuEnter_8c01ba64,
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
        /* onSelect */ ProfileFileEnter_8c01d1c4,
        /* courseId */ 0,
    },
    {   /* [6] */
        /* enabled  */ 1,
        /* unlocked */ 1,
        /* x, y     */ 109.0f, 182.0f,
        /* spriteNo */ 0,
        /* onSelect */ AlbumEnter_8c01d6e2,
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

STATIC InstructorLine init_seqStoryIntro_8c0445d0[] = {
    { MSG_SEQ_STORY_INTRO_01, 1 },
    { MSG_SEQ_STORY_INTRO_02, 0 },
    { MSG_SEQ_STORY_INTRO_03, 0 },
    { MSG_SEQ_STORY_INTRO_04, 0 },
    { MSG_SEQ_STORY_INTRO_05, 0 },
    { MSG_SEQ_STORY_INTRO_06, 0 },
    { MSG_SEQ_STORY_INTRO_07, 0 },
    { MSG_SEQ_STORY_INTRO_08, 0 },
    { MSG_SEQ_STORY_INTRO_09, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqSuccessPerfect_8c044620[] = {
    { MSG_SEQ_SUCCESS_PERFECT_01, 0 },
    { MSG_SEQ_SUCCESS_PERFECT_02, 0 },
    { MSG_SEQ_SUCCESS_PERFECT_03, 2 },
    { MSG_SEQ_SUCCESS_PERFECT_04, 2 },
    { MSG_SEQ_SUCCESS_PERFECT_05, 3 },
    { "", 0 },
};

STATIC InstructorLine init_seqSuccessHigh_8c044650[] = {
    { MSG_SEQ_SUCCESS_HIGH_01, 0 },
    { MSG_SEQ_SUCCESS_HIGH_02, 0 },
    { MSG_SEQ_SUCCESS_HIGH_03, 0 },
    { MSG_SEQ_SUCCESS_HIGH_04, 2 },
    { MSG_SEQ_SUCCESS_HIGH_05, 3 },
    { "", 0 },
};

STATIC InstructorLine init_seqSuccessNormal_8c044680[] = {
    { MSG_SEQ_SUCCESS_HIGH_01, 0 },
    { MSG_SEQ_SUCCESS_NORMAL_02, 0 },
    { MSG_SEQ_SUCCESS_NORMAL_03, 0 },
    { MSG_SEQ_SUCCESS_NORMAL_04, 0 },
    { MSG_SEQ_SUCCESS_NORMAL_05, 2 },
    { MSG_SEQ_SUCCESS_NORMAL_06, 3 },
    { "", 0 },
};

STATIC InstructorLine init_seqFailureFinal_8c0446b8[] = {
    { MSG_SEQ_FAILURE_FINAL_01, 0 },
    { MSG_SEQ_FAILURE_FINAL_02, 1 },
    { MSG_SEQ_FAILURE_FINAL_03, 1 },
    { MSG_SEQ_FAILURE_FINAL_04, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqFreeRunIntro_8c0446e0[] = {
    { MSG_SEQ_FREE_RUN_INTRO_01, 0 },
    { MSG_SEQ_FREE_RUN_INTRO_02, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqStoryChooseCourse_8c0446f8[] = {
    { MSG_SEQ_STORY_CHOOSE_COURSE_01, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqGoodPractice_8c044708[] = {
    { MSG_SEQ_GOOD_PRACTICE_01, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqSuccess_8c044718[] = {
    { MSG_SEQ_SUCCESS_01, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqAwardBadgeGold_8c044728[] = {
    { MSG_SEQ_AWARD_BADGE_GOLD_01, 0 },
    { MSG_SEQ_AWARD_BADGE_GOLD_02, 0 },
    { MSG_SEQ_AWARD_BADGE_GOLD_03, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqAwardBadgeSilver_8c044748[] = {
    { MSG_SEQ_AWARD_BADGE_SILVER_01, 1 },
    { MSG_SEQ_AWARD_BADGE_SILVER_02, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqAwardBadgeBronze_8c044760[] = {
    { MSG_SEQ_AWARD_BADGE_BRONZE_01, 1 },
    { MSG_SEQ_AWARD_BADGE_BRONZE_02, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqFailureRetry_8c044778[] = {
    { MSG_SEQ_FAILURE_RETRY_01, 0 },
    { MSG_SEQ_FAILURE_RETRY_02, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqCourseUnlocked_8c044790[] = {
    { MSG_SEQ_COURSE_UNLOCKED_01, 0 },
    { MSG_SEQ_COURSE_UNLOCKED_02, 0 },
    { MSG_SEQ_COURSE_UNLOCKED_03, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqPassengerLetter_8c0447b0[] = {
    { MSG_SEQ_PASSENGER_LETTER_01, 0 },
    { MSG_SEQ_PASSENGER_LETTER_02, 0 },
    { MSG_SEQ_PASSENGER_LETTER_03, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqCourseLocked_8c0447d0[] = {
    { MSG_SEQ_COURSE_LOCKED_01, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqForcePractice_8c0447e0[] = {
    { MSG_SEQ_FORCE_PRACTICE_01, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqFinalDay_8c0447f0[] = {
    { MSG_SEQ_FINAL_DAY_01, 0 },
    { MSG_SEQ_FINAL_DAY_02, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqLessonIntro_8c044808[] = {
    { MSG_SEQ_LESSON_INTRO_01, 0 },
    { MSG_SEQ_LESSON_INTRO_02, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqLessonComplete_8c044820[] = {
    { MSG_SEQ_LESSON_COMPLETE_01, 0 },
    { MSG_SEQ_LESSON_COMPLETE_02, 0 },
    { MSG_SEQ_LESSON_COMPLETE_03, 0 },
    { MSG_SEQ_LESSON_COMPLETE_04, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqLessonNext_8c044848[] = {
    { MSG_SEQ_LESSON_NEXT_01, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqLessonRetry_8c044858[] = {
    { MSG_SEQ_LESSON_RETRY_01, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqLessonTips_8c044868[] = {
    { MSG_SEQ_LESSON_TIPS_01, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqLessonWarning_8c044878[] = {
    { MSG_SEQ_LESSON_WARNING_01, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqLessonChoose_8c044888[] = {
    { MSG_SEQ_LESSON_CHOOSE_01, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqScoreRecord_8c044898[] = {
    { MSG_SEQ_SCORE_RECORD_01, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqLessonFinalDay_8c0448a8[] = {
    { MSG_SEQ_LESSON_FINAL_DAY_01, 0 },
    { MSG_SEQ_LESSON_FINAL_DAY_02, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqLessonPerfect_8c0448c0[] = {
    { MSG_SEQ_LESSON_PERFECT_01, 0 },
    { MSG_SEQ_LESSON_PERFECT_02, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqLessonGood_8c0448d8[] = {
    { MSG_SEQ_LESSON_GOOD_01, 0 },
    { MSG_SEQ_LESSON_GOOD_02, 0 },
    { MSG_SEQ_LESSON_GOOD_03, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqLessonPass_8c0448f8[] = {
    { MSG_SEQ_LESSON_PASS_01, 0 },
    { MSG_SEQ_LESSON_PASS_02, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqLessonFailMinor_8c044910[] = {
    { MSG_SEQ_LESSON_FAIL_MINOR_01, 1 },
    { MSG_SEQ_LESSON_FAIL_MINOR_02, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqLessonFailMajor_8c044928[] = {
    { MSG_SEQ_LESSON_FAIL_MAJOR_01, 1 },
    { MSG_SEQ_LESSON_FAIL_MAJOR_02, 1 },
    { MSG_SEQ_LESSON_FAIL_MAJOR_03, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqCollisionCarMinor_8c044948[] = {
    { MSG_SEQ_COLLISION_CAR_MINOR_01, 1 },
    { MSG_SEQ_COLLISION_CAR_MINOR_02, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqCollisionCarMedium_8c044960[] = {
    { MSG_SEQ_COLLISION_CAR_MEDIUM_01, 1 },
    { MSG_SEQ_COLLISION_CAR_MEDIUM_02, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqCollisionCarSevere_8c044978[] = {
    { MSG_SEQ_COLLISION_CAR_SEVERE_01, 1 },
    { MSG_SEQ_COLLISION_CAR_MEDIUM_02, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqCollisionCarFatal_8c044990[] = {
    { MSG_SEQ_COLLISION_CAR_FATAL_01, 1 },
    { MSG_SEQ_COLLISION_CAR_FATAL_02, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqCollisionWallMinor_8c0449a8[] = {
    { MSG_SEQ_COLLISION_WALL_MINOR_01, 1 },
    { MSG_SEQ_COLLISION_CAR_MEDIUM_02, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqCollisionWallMedium_8c0449c0[] = {
    { MSG_SEQ_COLLISION_WALL_MEDIUM_01, 1 },
    { MSG_SEQ_COLLISION_CAR_MEDIUM_02, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqCollisionWallSevere_8c0449d8[] = {
    { MSG_SEQ_COLLISION_WALL_SEVERE_01, 1 },
    { MSG_SEQ_COLLISION_CAR_FATAL_02, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqNearMissPedestrian_8c0449f0[] = {
    { MSG_SEQ_NEAR_MISS_PEDESTRIAN_01, 1 },
    { MSG_SEQ_NEAR_MISS_PEDESTRIAN_02, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqOffCourseMinor_8c044a08[] = {
    { MSG_SEQ_OFF_COURSE_MINOR_01, 1 },
    { MSG_SEQ_OFF_COURSE_MINOR_02, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqOffCourseMedium_8c044a20[] = {
    { MSG_SEQ_OFF_COURSE_MINOR_01, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqOffCourseMajor_8c044a30[] = {
    { MSG_SEQ_OFF_COURSE_MINOR_01, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqSpeedingMinor_8c044a40[] = {
    { MSG_SEQ_SPEEDING_MINOR_01, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqSpeedingMajor_8c044a50[] = {
    { MSG_SEQ_SPEEDING_MAJOR_01, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqWrongLane_8c044a60[] = {
    { MSG_SEQ_WRONG_LANE_01, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqLaneStraddle_8c044a70[] = {
    { MSG_SEQ_LANE_STRADDLE_01, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqNoSignal_8c044a80[] = {
    { MSG_SEQ_NO_SIGNAL_01, 1 },
    { MSG_SEQ_NO_SIGNAL_02, 1 },
    { MSG_SEQ_NO_SIGNAL_03, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqNoSignalTurn_8c044aa0[] = {
    { MSG_SEQ_NO_SIGNAL_TURN_01, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqBlank_8c044ab0[] = {
    { "", 0 },
};

STATIC InstructorLine init_seqSignalViolation_8c044ab8[] = {
    { MSG_SEQ_SIGNAL_VIOLATION_01, 1 },
    { MSG_SEQ_SIGNAL_VIOLATION_02, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqBadStopLine_8c044ad0[] = {
    { MSG_SEQ_BAD_STOP_LINE_01, 1 },
    { MSG_SEQ_BAD_STOP_LINE_02, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqIllegalLaneChange_8c044ae8[] = {
    { MSG_SEQ_ILLEGAL_LANE_CHANGE_01, 1 },
    { MSG_SEQ_ILLEGAL_LANE_CHANGE_02, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqBlockIntersection_8c044b00[] = {
    { MSG_SEQ_BLOCK_INTERSECTION_01, 1 },
    { MSG_SEQ_BLOCK_INTERSECTION_02, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqWrongWay_8c044b18[] = {
    { MSG_SEQ_WRONG_WAY_01, 1 },
    { MSG_SEQ_WRONG_WAY_02, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqRapidAccel_8c044b30[] = {
    { MSG_SEQ_RAPID_ACCEL_01, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqHardBrake_8c044b40[] = {
    { MSG_SEQ_HARD_BRAKE_01, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqSwerving_8c044b50[] = {
    { MSG_SEQ_SWERVING_01, 1 },
    { MSG_SEQ_SWERVING_02, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqMissedStop_8c044b68[] = {
    { MSG_SEQ_MISSED_STOP_01, 1 },
    { MSG_SEQ_MISSED_STOP_02, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqBadStopPosition1_8c044b80[] = {
    { MSG_SEQ_BAD_STOP_POSITION1_01, 1 },
    { MSG_SEQ_BAD_STOP_POSITION1_02, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqBadStopPosition2_8c044b98[] = {
    { MSG_SEQ_BAD_STOP_POSITION1_01, 1 },
    { MSG_SEQ_BAD_STOP_POSITION1_02, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqTimeManagement_8c044bb0[] = {
    { MSG_SEQ_TIME_MANAGEMENT_01, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqAnnouncement_8c044bc0[] = {
    { MSG_SEQ_ANNOUNCEMENT_01, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqDoorOperation_8c044bd0[] = {
    { MSG_SEQ_DOOR_OPERATION_01, 1 },
    { "", 0 },
};

STATIC InstructorLine init_seqFreeRunIntro2_8c044be0[] = {
    { MSG_SEQ_FREE_RUN_INTRO2_01, 0 },
    { MSG_SEQ_FREE_RUN_INTRO2_02, 0 },
    { "", 0 },
};

STATIC InstructorLine init_seqFreeRunChooseCourse_8c044bf8[] = {
    { MSG_SEQ_FREE_RUN_CHOOSE_COURSE_01, 0 },
    { "", 0 },
};

InstructorLine *init_instructorDialogs_8c044c08[] = {
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
    init_seqBlank_8c044ab0,
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

// 3 routes -> 3 departure slots -> hh, mm
Uint8 init_routeInfoTime_8c044d2e[3 * 3 * 2] = {
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
