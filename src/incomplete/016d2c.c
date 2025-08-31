#include <shinobi.h>
#include <sg_sd.h>
#include <njdef.h>
#include <sg_xpt.h>
#include "015ab8_title.h"
#include "014a9c_tasks.h"
#include "011120_asset_queues.h"
#include "019e98_main_menu.h"

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
    int field_0x04;
} MenuDialog;

typedef struct {
    Uint8 unlocked_0x00;
    Uint8 new_0x01;
    Uint8 field_0x02;
    Uint8 storySpriteNo_0x03;
    Uint8 freeRunSpriteNo_0x04;
    Uint8 field_0x05[3]; // Padding?
} CourseProgress;

typedef struct {
    int days_0x00;
    char field_0x04[0x28];
    int letters_0x2c[6];
    CourseProgress courses_0x44[8]; // Should this be 9?
    int field_0x84;
    int field_0x88;
    int field_0x8c;
    int field_0x90;
} PlayerProgress;

extern void snd_8c010cd6(int p1, int p2);
extern void setUknPvmBool_8c014330();
extern int requestSysResgrp_8c018568(ResourceGroup* dds, ResourceGroupInfo* rg);
extern CourseMenuButton init_courseMenuButtons_8c04442c[15];
extern void* const_8c03628c;
extern SDMIDI var_midiHandles_8c0fcd28[7];
extern PlayerProgress var_progress_8c1ba1cc;
extern int var_exp_8c1ba25c;
extern int var_8c225fb4;
extern int var_8c225fb8;
extern PDS_PERIPHERAL var_peripherals_8c1ba35c[2];
extern ResourceGroupInfo init_mainMenuResourceGroup_8c044264;

extern MenuDialog *init_8c044c08[];
extern int var_game_mode_8c1bb8fc;
extern int var_dialogQueue_8c225fbc[4]; // TODO: Confirm length
extern Sint8 var_coursesToUnlock_8c225fd4[];
extern int var_demo_8c1bb8d0;
extern void resetUknPvmBool_8c014322();
extern NJS_TEXMEMLIST var_tex_8c157af8[];

/**
 * Returns 1 if the cursor has reached its target position, 0 otherwise.
 */
int interpolateCursor_8c016d2c()
{
    menuState_8c1bc7a8.pos.cursor.cursor_0x20.x += menuState_8c1bc7a8.cursorVelocity_0x30.x;
    menuState_8c1bc7a8.pos.cursor.cursor_0x20.y += menuState_8c1bc7a8.cursorVelocity_0x30.y;

    if (menuState_8c1bc7a8.cursorVelocity_0x30.x) {
        if (
            (menuState_8c1bc7a8.cursorVelocity_0x30.x >= 0)
            || (menuState_8c1bc7a8.pos.cursor.cursor_0x20.x > menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.x)
        ) {
            // if (!(menuState_8c1bc7a8.cursorVelocity_0x30.x > 0)) {
            if (menuState_8c1bc7a8.cursorVelocity_0x30.x <= 0) {
                return 0;
            }

            // if (!(menuState_8c1bc7a8.pos.cursor.cursor_0x20.x > menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.x)) {
            if (menuState_8c1bc7a8.pos.cursor.cursor_0x20.x <= menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.x) {
                return 0;
            }

        }
        menuState_8c1bc7a8.pos.cursor.cursor_0x20 = menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28;
    } else if (menuState_8c1bc7a8.cursorVelocity_0x30.y) {
        if (
            (menuState_8c1bc7a8.cursorVelocity_0x30.y >= 0)
            || (menuState_8c1bc7a8.pos.cursor.cursor_0x20.y > menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.y)
        ) {
            // if (!(menuState_8c1bc7a8.cursorVelocity_0x30.y > 0)) {
            if (menuState_8c1bc7a8.cursorVelocity_0x30.y <= 0) {
                return 0;
            }

            if (!(menuState_8c1bc7a8.pos.cursor.cursor_0x20.y > menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.y)) {
                return 0;
            }

        }
        menuState_8c1bc7a8.pos.cursor.cursor_0x20 = menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28;
    }

    return 1;
}

int cursorOffTarget_8c016dc6()
{
    int selected;
    float y;
    float x;

    selected = menuState_8c1bc7a8.field_0x3c + menuState_8c1bc7a8.field_0x40 * 5;
    x = init_courseMenuButtons_8c04442c[selected].x_0x08;
    y = init_courseMenuButtons_8c04442c[selected].y_0x0c;
    if (
        (menuState_8c1bc7a8.pos.cursor.cursor_0x20.x == x)
        && (menuState_8c1bc7a8.pos.cursor.cursor_0x20.y == y)
    ) {
        return 0;
    }
    menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.x = x;
    menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.y = y;
    menuState_8c1bc7a8.cursorVelocity_0x30.x = (x - menuState_8c1bc7a8.pos.cursor.cursor_0x20.x) / 6.0;
    menuState_8c1bc7a8.cursorVelocity_0x30.y = (y - menuState_8c1bc7a8.pos.cursor.cursor_0x20.y) / 6.0;
    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
    swapMessageBoxFor_8c02aefc(&const_8c03628c);
    return 1;
}

void drawInteger_8c016e6c(int value, float x, float y)
{
    do {
        drawSprite_8c014f54(
            &menuState_8c1bc7a8.resourceGroupA_0x00,
            15 + value % 10,
            x,
            y,
            -4.0
        );
        x -= 10.0;
    } while (value /= 10);
}

unsigned int getWeekDayIndex_8c016ed2()
{
    unsigned int r = var_progress_8c1ba1cc.days_0x00 + 1;
    return r % 7;
}

void drawDateAndExp_8c016ee6()
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

    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupA_0x00,
        sprite_id,
        112.0,
        82.0,
        -4.0
    );

    drawInteger_8c016e6c(var_progress_8c1ba1cc.field_0x90, 534.0, 82.0);
}

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

void dialogSequenceTask_8c016f98(DialogSequenceTask *task, DialogSequenceTaskState *state)
{
    switch(state->state_0x00) {
        case 0: {
            int r;

            if (!*(state->dialog_0x04->text_0x00)) {
                var_8c225fb4 = 0;
                freeTask_8c014b66((void *) task);
                return;
            }

            if (task->field_0x18 && *task->field_0x18) {
                snd_8c010cd6(2, *task->field_0x18);
                task->field_0x18++;
            }

            state->field_0x08 = swapMessageBoxFor_8c02aefc(state->dialog_0x04->text_0x00);
            menuState_8c1bc7a8.field_0x60 = state->dialog_0x04->field_0x04;
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
            drawSprite_8c014f54(
                &menuState_8c1bc7a8.resourceGroupA_0x00,
                44,
                32.0,
                -16.0 + 8 * njCos(state->field_0x14),
                -3.0
            );

            break;
        }
    }

    var_8c225fb8 = state->field_0x0c;
}

void pushDialogTask_8c0170c6(int dialog_index, int *p2)
{
    DialogSequenceTask *task;
    DialogSequenceTaskState *state;

    pushTask_8c014ae8(
        var_tasks_8c1ba3c8,
        &dialogSequenceTask_8c016f98,
        &task,
        &state,
        0x18
    );

    task->field_0x18 = p2;
    state->state_0x00 = 0;
    state->dialog_0x04 = init_8c044c08[dialog_index];
    var_8c225fb4 = 1;
}

void FUN_swapDialogMessageBox_8c017108(int dialog_index)
{
    var_8c225fb8 = swapMessageBoxFor_8c02aefc(init_8c044c08[dialog_index]->text_0x00);
}

enum {
    STORY_MENU_STATE_INIT = 0,
    STORY_MENU_STATE_FADE_IN = 1,
    STORY_MENU_STATE_DIALOG = 2,
    STORY_MENU_STATE_IDLE = 3,
    STORY_MENU_STATE_ANIMATING = 4,
    STORY_MENU_STATE_COURSE_SELECTED = 5,
    STORY_MENU_STATE_FADE_OUT = 6,
    STORY_MENU_STATE_FADE_OUT_TO_MAIN_MENU = 7
};

void handleCourseMenuInput_8c017126()
{
    if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
        if (
            init_courseMenuButtons_8c04442c[
                menuState_8c1bc7a8.field_0x3c
                + menuState_8c1bc7a8.field_0x40 * 5
            ]
            .unlocked_0x04 == 0
        ) {
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
            // écîOÇæÇ™ÇªÇÃÉRÅ[ÉXÇÕÇ‹ÇæëñÇÍÇ»Ç¢ÇÊ<E>Ç‡Ç¡Ç∆åoå±ÇêœÇÒÇ≈Ç©ÇÁÇæÇ»
            // Sorry, but you canÅft drive that course yet.<E>YouÅfll need more experience first.
            FUN_swapDialogMessageBox_8c017108(15);
        } else {
            FUN_8c010bae(0);
            FUN_8c010bae(1);
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            menuState_8c1bc7a8.state_0x18 = 5;
            menuState_8c1bc7a8.logo_timer_0x68 = 0;
        }
    }

    if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KU) {
        do {
            if (--menuState_8c1bc7a8.field_0x40 < 0) {
                menuState_8c1bc7a8.field_0x40 = 2;
            }
        } while (
            init_courseMenuButtons_8c04442c[
                menuState_8c1bc7a8.field_0x40 * 5 + menuState_8c1bc7a8.field_0x3c
            ].enabled_0x00 == 0
        );

        if (cursorOffTarget_8c016dc6()) {
            menuState_8c1bc7a8.state_0x18 = 4;
        }
    } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KD) {
        do {
            if (++menuState_8c1bc7a8.field_0x40 > 2) {
                menuState_8c1bc7a8.field_0x40 = 0;
            }
        } while (
            init_courseMenuButtons_8c04442c[
                menuState_8c1bc7a8.field_0x40 * 5 + menuState_8c1bc7a8.field_0x3c
            ].enabled_0x00 == 0
        );

        if (cursorOffTarget_8c016dc6()) {
            menuState_8c1bc7a8.state_0x18 = 4;
        }
    } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KL) {
        do {
            if (--menuState_8c1bc7a8.field_0x3c < 0) {
                menuState_8c1bc7a8.field_0x3c = 4;
            }
        } while (
            init_courseMenuButtons_8c04442c[
                menuState_8c1bc7a8.field_0x40 * 5 + menuState_8c1bc7a8.field_0x3c
            ].enabled_0x00 == 0
        );

        if (cursorOffTarget_8c016dc6()) {
            menuState_8c1bc7a8.state_0x18 = 4;
        }
    } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KR) {
        do {
            if (++menuState_8c1bc7a8.field_0x3c > 4) {
                menuState_8c1bc7a8.field_0x3c = 0;
            }
        } while (
            init_courseMenuButtons_8c04442c[
                menuState_8c1bc7a8.field_0x40 * 5 + menuState_8c1bc7a8.field_0x3c
            ].enabled_0x00 == 0
        );

        if (cursorOffTarget_8c016dc6()) {
            menuState_8c1bc7a8.state_0x18 = 4;
        }
    }
}

int buildCourseUnlockList_8c0172dc()
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
                if (var_progress_8c1ba1cc.days_0x00 < 8 || var_exp_8c1ba25c < 4000)
                    continue;
                break;

            case 2:
                if (var_progress_8c1ba1cc.days_0x00 < 9 || var_exp_8c1ba25c < 5500)
                    continue;
                break;

            case 3:
                if (var_progress_8c1ba1cc.days_0x00 < 5 || var_exp_8c1ba25c < 2000)
                    continue;
                break;

            case 4:
                if (var_progress_8c1ba1cc.days_0x00 < 11 || var_exp_8c1ba25c < 8000)
                    continue;
                break;

            case 5:
                if (var_progress_8c1ba1cc.days_0x00 < 13 || var_exp_8c1ba25c < 12000)
                    continue;
                break;

            case 6:
                continue;

            case 7:
                if (var_progress_8c1ba1cc.days_0x00 < 3 || var_exp_8c1ba25c < 500)
                    continue;
                break;

            case 8:
                if (var_progress_8c1ba1cc.days_0x00 < 6 || var_exp_8c1ba25c < 3000)
                    continue;
                break;
        }

        var_coursesToUnlock_8c225fd4[j] = i;
        j++;
    }

    var_coursesToUnlock_8c225fd4[j] = -1;
    return j;
}

void applyUnlocks_8c0173e6(void)
{
    int i;
    for (i = 0; var_coursesToUnlock_8c225fd4[i] != -1; i++) {
        int j = var_coursesToUnlock_8c225fd4[i];
        var_progress_8c1ba1cc.courses_0x44[j].unlocked_0x00 = 1;
        var_progress_8c1ba1cc.courses_0x44[j].new_0x01 = 1;
    }
}

extern int var_8c1bb8b8;
extern int var_8c1bb8bc;
extern int var_8c1bb8dc;
extern int var_award_8c1bb8f8;

enum {
    SEQ_INTRO_BRIEFING            = 0,
    SEQ_CHOOSE_COURSE             = 6,
    SEQ_SUCCESS_2                 = 7,
    SEQ_SUCCESS                   = 8,
    SEQ_AWARD_BADGE_GOLD          = 9,
    SEQ_AWARD_BADGE_SILVER        = 10,
    SEQ_AWARD_BADGE_BRONZE        = 11,
    SEQ_FAILURE                   = 12,
    SEQ_COURSE_UNLOCKED           = 13,
    SEQ_PASSENGER_LETTER_RECEIVED = 14
};

// This function has been refactored.
void buildCourseMenuDialogFlow_8c017420(void)
{
    int cur = 0;

    // Default choose course
    if (var_8c1bb8b8 == 0) {
        var_dialogQueue_8c225fbc[cur++] = SEQ_CHOOSE_COURSE;
        var_dialogQueue_8c225fbc[cur]   = -1;
        return;
    }

    // On the first day, show the intro briefing
    if (var_progress_8c1ba1cc.days_0x00 == 1) {
        var_dialogQueue_8c225fbc[cur++] = SEQ_INTRO_BRIEFING;
        var_dialogQueue_8c225fbc[cur++] = SEQ_CHOOSE_COURSE;
        var_dialogQueue_8c225fbc[cur]   = -1;
        return;
    }

    // Special Success
    if (var_8c1bb8bc != 0) {
        var_dialogQueue_8c225fbc[cur++] = SEQ_SUCCESS_2;
        var_dialogQueue_8c225fbc[cur++] = SEQ_CHOOSE_COURSE;
        var_dialogQueue_8c225fbc[cur]   = -1;
        return;
    }

    // Result
    if (var_8c1bb8dc == 0) {
        var_dialogQueue_8c225fbc[cur++] = SEQ_FAILURE;
    } else {
        int award_seq = SEQ_SUCCESS;
        if      (var_award_8c1bb8f8 == 1) award_seq = SEQ_AWARD_BADGE_BRONZE;
        else if (var_award_8c1bb8f8 == 2) award_seq = SEQ_AWARD_BADGE_SILVER;
        else if (var_award_8c1bb8f8 == 3) award_seq = SEQ_AWARD_BADGE_GOLD;
        var_dialogQueue_8c225fbc[cur++] = award_seq;
    }

    // Course unlocked
    if (buildCourseUnlockList_8c0172dc() != 0) {
        var_dialogQueue_8c225fbc[cur++] = SEQ_COURSE_UNLOCKED;
    }

    // Passenger letter received
    if (((var_progress_8c1ba1cc.days_0x00 + 1) % 7) == 0) {
        int r = AsqGetRandomInRangeB_121be(6);
        if (var_progress_8c1ba1cc.letters_0x2c[r] == 0) {
            var_progress_8c1ba1cc.letters_0x2c[r] = 1;
            var_dialogQueue_8c225fbc[cur++] = SEQ_PASSENGER_LETTER_RECEIVED;
        }
    }

    var_dialogQueue_8c225fbc[cur++] = SEQ_CHOOSE_COURSE;

    var_dialogQueue_8c225fbc[cur] = -1;
}


/* This is the original code for the function above. (with some gotos to be removed)
void buildCourseMenuDialogFlow_8c017420()
{
    bool bVar1_done;
    int iVar2;
    int iVar3_index;
    int iVar4_curDialog;

    iVar4_curDialog = 0;
    bVar1_done = false;
    iVar3_index = 0;
    do
    {
        if ((4 < iVar3_index) || (bVar1_done))
        {
            var_dialogQueue_8c225fbc[iVar4_curDialog] = -1;
            return;
        }
        if (iVar3_index == 0)
        {
            // This path always marks and done,
            // except if the loop is explicitly continued
            iVar2 = iVar4_curDialog;
            if (var_8c1bb8b8 != 0)
            {
                if (var_progress_8c1ba1cc.field_0x00 == 1)
                {
                    iVar2 = iVar4_curDialog + 1;
                    // SEQ_INTRO_BRIEFING
                    var_dialogQueue_8c225fbc[iVar4_curDialog] = 0;
                }
                else
                {

                    if (var_8c1bb8bc == 0)
                    {
                        // This is the only path that can lead to a loop continue
                        if (var_8c1bb8dc == 0)
                        {
                            // SEQ_FAILURE
                            var_dialogQueue_8c225fbc[iVar4_curDialog] = 12;
                            iVar4_curDialog = iVar4_curDialog + 1;
                        }
                        else
                        {
                            if (var_award_8c1bb8f8 == 0)
                            {
                                // SEQ_SUCCESS
                                iVar2 = 8;
                            }
                            else if (var_award_8c1bb8f8 == 1)
                            {
                                // SEQ_AWARD_BADGE_BRONZE
                                iVar2 = 11;
                            }
                            else if (var_award_8c1bb8f8 == 2)
                            {
                                // SEQ_AWARD_BADGE_SILVER
                                iVar2 = 10;
                            }
                            else
                            {
                                if (var_award_8c1bb8f8 != 3)
                                    goto LAB_8c01754c;
                                // SEQ_AWARD_BADGE_GOLD
                                iVar2 = 9;
                            }
                            var_dialogQueue_8c225fbc[iVar4_curDialog] = iVar2;
                            iVar4_curDialog = iVar4_curDialog + 1;
                        }
                        goto LAB_8c01754c;
                    }
                    // SEQ_SUCCESS_2
                    var_dialogQueue_8c225fbc[iVar4_curDialog] = 7;
                    iVar2 = iVar4_curDialog + 1;
                }
            }
            // SEQ_CHOOSE_COURSE
            var_dialogQueue_8c225fbc[iVar2] = 6;
            bVar1_done = true;
            iVar4_curDialog = iVar2 + 1;
        }
        else if (iVar3_index != 1)
        {
            if (iVar3_index == 2)
            {
                iVar2 = buildCourseUnlockList_8c0172dc();
                if (iVar2 != 0)
                {
                    // SEQ_COURSE_UNLOCKED
                    var_dialogQueue_8c225fbc[iVar4_curDialog] = 13;
                    iVar4_curDialog = iVar4_curDialog + 1;
                }
            }
            else if (iVar3_index == 3)
            {
                iVar2 = (var_progress_8c1ba1cc.field_0x00 + 1) % 7;
                if (iVar2 == 0)
                {
                    iVar2 = AsqGetRandomInRangeB_121be(6);
                    if (var_progress_8c1ba1cc.field_0x2c[iVar2] == 0)
                    {
                        var_progress_8c1ba1cc.field_0x2c[iVar2] = 1;
                        // SEQ_PASSENGER_LETTER_RECEIVED
                        var_dialogQueue_8c225fbc[iVar4_curDialog] = 14;
                        iVar4_curDialog = iVar4_curDialog + 1;
                    }
                }
            }
            else if (iVar3_index == 4)
            {
                // SEQ_CHOOSE_COURSE
                var_dialogQueue_8c225fbc[iVar4_curDialog] = 6;
                iVar4_curDialog = iVar4_curDialog + 1;
            }
        }
    LAB_8c01754c:
        iVar3_index = iVar3_index + 1;
    } while (true);
} */

// This function has been refactored.
void drawCourseButtons_8c017590()
{
    int i;

    if (menuState_8c1bc7a8.field_0x48) {
        drawSprite_8c014f54(
            &menuState_8c1bc7a8.resourceGroupB_0x0c,
            0x18,
            menuState_8c1bc7a8.pos.cursor.cursor_0x20.x,
            menuState_8c1bc7a8.pos.cursor.cursor_0x20.y,
            -3.0
        );
    }

    // TODO: Extract length constant
    for (i = 0; i < 15; i++) {
        CourseMenuButton *btn = &init_courseMenuButtons_8c04442c[i];

        if (btn->enabled_0x00 == 0 || btn->spriteNo_0x10 == 0)
            continue;

        drawSprite_8c014f54(
            &menuState_8c1bc7a8.resourceGroupB_0x0c,
            btn->spriteNo_0x10,
            0.0,
            0.0,
            -4.0
        );
    }

    // TODO: Extract length constant
    for (i = 0; i < 9; i++) {
        char spriteNo = var_game_mode_8c1bb8fc == 0
            ? var_progress_8c1ba1cc.courses_0x44[i].storySpriteNo_0x03
            : var_progress_8c1ba1cc.courses_0x44[i].freeRunSpriteNo_0x04;

        if (!spriteNo)
            continue;

        drawSprite_8c014f54(
            &menuState_8c1bc7a8.resourceGroupB_0x0c,
            0x18 - spriteNo,
            240.0 + (i % 3) * 93.0,
            106.0 + (i / 3) * 74.0,
            -3.5
        );
    }
}

/* This is the original code for the function above.
void drawCourseButtons_8c017590()
{
    int iVar1;
    int iVar2;
    CourseMenuButton *pRVar3;
    CourseMenuButton *pRVar4;
    int iVar5;
    float x;
    float fVar6;
    float y;
    float fVar7;
    float priority;

    if (menuState_8c1bc7a8.field_0x48 != 0) {
        drawSprite_8c014f54(
            &menuState_8c1bc7a8.resourceGroupB_0x0c,
            0x18,
            menuState_8c1bc7a8.pos.cursor.cursor_0x20.x,
            menuState_8c1bc7a8.pos.cursor.cursor_0x20.y,
            -3.0
        );
    }
    pRVar3 = init_courseMenuButtons_8c04442c;
    fVar7 = -4.0;
    fVar6 = 0.0;
    do {
        iVar2 = 0;
        pRVar4 = pRVar3;
        do {
            if ((pRVar4->field_0x04 != 0) && (pRVar4->field_0x10 != 0)) {
                drawSprite_8c014f54(
                    &menuState_8c1bc7a8.resourceGroupB_0x0c,
                    pRVar4->field_0x10,
                    fVar6,
                    fVar6,
                    fVar7
                );
            }
            iVar2 = iVar2 + 1;
            pRVar4 = pRVar4 + 1;
        } while (iVar2 < 5);
        pRVar3 = pRVar3 + 5;
    } while (pRVar3 < &init_courseMenuButtons_8c04442c[15]); // TODO: Extract length constant
    fVar7 = 240.0;
    fVar6 = 93.0;
    priority = -3.5;
    iVar2 = 0;
    do {
        iVar1 = iVar2 * 3;
        iVar5 = 0;
        y = (float)iVar2 * 74.0 + 106.0;
        do {
            x = fVar6 * (float)iVar5 + fVar7;
            if (var_game_mode_8c1bb8fc == 0) { // TODO: Extract constant
                if (var_progress_8c1ba1cc.courses_0x44[iVar1 + iVar5].field_0x03 != 0) {
                    drawSprite_8c014f54(
                        &menuState_8c1bc7a8.resourceGroupB_0x0c,
                        0x18 - var_progress_8c1ba1cc.courses_0x44[iVar1 + iVar5].field_0x03,
                        x,
                        y,
                        priority
                    );
                }
            }
            else if (var_progress_8c1ba1cc.courses_0x44[iVar1 + iVar5].field_0x04[0] != 0) {
                drawSprite_8c014f54(
                    &menuState_8c1bc7a8.resourceGroupB_0x0c,
                    0x18 - var_progress_8c1ba1cc.courses_0x44[iVar1 + iVar5].field_0x04[0],
                    x,
                    y,
                    priority
                );
            }
            iVar5++;
        } while (iVar5 < 3);
        iVar2++;
    } while (iVar2 < 3);
    return;
}
*/

extern Bool isFading_8c226568;
extern int init_8c03bd80;
extern void *var_8c225fb0;

void StoryMenuTask_8c017718(Task * task, void *state)
{
    switch (menuState_8c1bc7a8.state_0x18) {
        case STORY_MENU_STATE_INIT: {
            if (getUknPvmBool_8c01432a())
                return;

            AsqFreeQueues_11f7e();
            menuState_8c1bc7a8.state_0x18 = STORY_MENU_STATE_FADE_IN;
            FUN_8c010d8a();
            snd_8c010cd6(0, 15);
            push_fadein_8c022a9c(10);
            return;
        }

        case STORY_MENU_STATE_FADE_IN: {
            if (isFading_8c226568 == 0) {
                pushDialogTask_8c0170c6(var_dialogQueue_8c225fbc[0], 0);
                menuState_8c1bc7a8.state_0x18 = 2;
            }
            break;
        }

        case STORY_MENU_STATE_DIALOG: {
            // Dialog still running
            if (var_8c225fb4) break;

            if (var_dialogQueue_8c225fbc[task->field_0x08] == SEQ_COURSE_UNLOCKED) {
                int row;
                applyUnlocks_8c0173e6();
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
                menuState_8c1bc7a8.state_0x18 = 3;
                // This is probably a empty string literal
                swapMessageBoxFor_8c02aefc(&const_8c03628c);
            }
            // Otherwise, start the next dialog sequence
            else {
                pushDialogTask_8c0170c6(var_dialogQueue_8c225fbc[task->field_0x08], 0);
                if (var_dialogQueue_8c225fbc[task->field_0x08] == SEQ_COURSE_UNLOCKED) {
                    midiResetFxAndPlay_8c010846(0, 0);
                }
            }
            break;
        }

        case STORY_MENU_STATE_IDLE: {
            handleCourseMenuInput_8c017126();
            break;
        }

        case STORY_MENU_STATE_ANIMATING: {
            if (!interpolateCursor_8c016d2c())
                break;

            menuState_8c1bc7a8.state_0x18 = STORY_MENU_STATE_IDLE;
            break;
        }

        case STORY_MENU_STATE_COURSE_SELECTED: {
            if (++menuState_8c1bc7a8.logo_timer_0x68 > 10) {
                menuState_8c1bc7a8.state_0x18 = STORY_MENU_STATE_FADE_OUT;
                push_fadeout_8c022b60(10);
            }
            menuState_8c1bc7a8.field_0x48 = menuState_8c1bc7a8.logo_timer_0x68 & 1;
            break;
        }

        case STORY_MENU_STATE_FADE_OUT: {
            int buttonIndex;

            if (isFading_8c226568) {
                menuState_8c1bc7a8.field_0x48 = ++menuState_8c1bc7a8.logo_timer_0x68 & 1;
                break;
            }

            if (init_8c03bd80)
                return;

            if (menuState_8c1bc7a8.field_0x3c != 1 || menuState_8c1bc7a8.field_0x40 != 0) {
                freeResourceGroup_8c0185c4(&menuState_8c1bc7a8.resourceGroupB_0x0c);
                var_8c225fb0 = (void *) -1;
            }

            menuState_8c1bc7a8.selected_0x38 = 0;
            buttonIndex = menuState_8c1bc7a8.field_0x40 * 5 + menuState_8c1bc7a8.field_0x3c;
            menuState_8c1bc7a8.field_0x50 =
                init_courseMenuButtons_8c04442c[buttonIndex].courseId_0x18;

            var_8c1bb8dc = 1;
            var_8c1bb8b8 = 0;
            var_8c1bb8bc = 1;

            init_courseMenuButtons_8c04442c[buttonIndex].onSelect_0x14(task);
            return;
        }

        case STORY_MENU_STATE_FADE_OUT_TO_MAIN_MENU: {
            if (isFading_8c226568)
                break;

            if (init_8c03bd80)
                return;

            var_8c1bb8b8 = 0;
            MainMenuSwitchFromTask_8c01a09a(task);
            return;
        }
    }

    drawDateAndExp_8c016ee6();
    drawCourseButtons_8c017590();
    drawSprite_8c014f54(&menuState_8c1bc7a8.resourceGroupB_0x0c,10,0.0,0.0,-5.0);
    drawSprite_8c014f54(&menuState_8c1bc7a8.resourceGroupA_0x00,0x2b,0.0,0.0,-4.0);
    if (menuTextboxText_8c02af1c(var_8c225fb8) ) {
        drawSprite_8c014f54(&menuState_8c1bc7a8.resourceGroupA_0x00,1,0.0,0.0,-5.0);
    }
    drawSprite_8c014f54(&menuState_8c1bc7a8.resourceGroupB_0x0c,menuState_8c1bc7a8.field_0x60,0.0,0.0,-6.0);
    drawSprite_8c014f54(&menuState_8c1bc7a8.resourceGroupA_0x00, 0, 0.0, 0.0, -7.0);
    AsqGetRandomA_12166();
}

void FUN_8c017a20();
void FUN_8c017d54();
void FreeRunMenuTask_8c017ada();

void CourseMenuSwitchFromTask_8c017e18(Task *task)
{
    if (var_game_mode_8c1bb8fc == 0) {
        setTaskAction_8c014b3e(task, StoryMenuTask_8c017718);
        buildCourseMenuDialogFlow_8c017420();
    } else {
        setTaskAction_8c014b3e(task, FreeRunMenuTask_8c017ada);
        FUN_8c017a20();
    }

    menuState_8c1bc7a8.field_0x60 = init_8c044c08[var_dialogQueue_8c225fbc[0]]->field_0x04;
    task->field_0x08 = 0;
    var_8c225fb8 = 0;
    var_demo_8c1bb8d0 = 0;
    FUN_8c017d54();
    njGarbageTexture(&var_tex_8c157af8, 0xc00);
    AsqInitQueues_11f36(8, 0, 0, 8);
    AsqResetQueues_11f6c();

    if (!requestSysResgrp_8c018568(
        &menuState_8c1bc7a8.resourceGroupB_0x0c,
        &init_mainMenuResourceGroup_8c044264
    )) {
        AsqFreeQueues_11f7e();
        menuState_8c1bc7a8.state_0x18 = 1;
        push_fadein_8c022a9c(10);
        snd_8c010cd6(0, 15);
        return;
    }

    setUknPvmBool_8c014330();
    AsqProcessQueues_11fe0(AsqNop_11120, 0, 0, 0, resetUknPvmBool_8c014322);
    menuState_8c1bc7a8.state_0x18 = 0;
}
