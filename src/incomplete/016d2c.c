#include <shinobi.h>
#include <sg_sd.h>
#include <njdef.h>
#include <sg_xpt.h>
#include "015ab8_title.h"
#include "014a9c_tasks.h"
#include "011120_asset_queues.h"

typedef struct {
    int field_0x00;
    int field_0x04;
    float x_0x08;
    float y_0x0c;
    int field_0x10;
    void *field_0x14;
    int field_0x18c;
} RunStruct;

typedef struct {
    char *text_0x00;
    int field_0x04;
} MenuDialog;

typedef struct {
    char field_0x00;
    char field_0x01;
    char field_0x02[6];
} Struct_1cc_Nested;

typedef struct {
    int field_0x00;
    char field_0x04[0x28];
    int field_0x2c[6]; // This size is probably wrong
    Struct_1cc_Nested field_0x44[8];
    int field_0x84;
    int field_0x88;
    int field_0x8c;
    int field_0x90;
} Struct_1cc;

extern void snd_8c010cd6(int p1, int p2);
extern void setUknPvmBool_8c014330();
extern int requestSysResgrp_8c018568(ResourceGroup* dds, ResourceGroupInfo* rg);
extern RunStruct init_runStruct_8c04442c[15];
extern void* const_8c03628c;
extern SDMIDI var_midiHandles_8c0fcd28[7];
extern Struct_1cc var_8c1ba1cc;
extern int var_score_8c1ba25c;
extern int var_8c225fb4;
extern int var_8c225fb8;
extern PDS_PERIPHERAL var_peripheral_8c1ba35c[2];
extern ResourceGroupInfo init_mainMenuResourceGroup_8c044264;

extern MenuDialog *init_8c044c08[];
extern int var_game_mode_8c1bb8fc;
extern int var_dialog_8c225fbc[4]; // TODO: Confirm length
extern Sint8 var_8c225fd4[];
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
    x = init_runStruct_8c04442c[selected].x_0x08;
    y = init_runStruct_8c04442c[selected].y_0x0c;
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

void FUN_8c016e6c(int sprite_id, float x, float y)
{
    do {
        drawSprite_8c014f54(
            &menuState_8c1bc7a8.resourceGroupA_0x00,
            15 + sprite_id % 10,
            x,
            y,
            -4.0
        );
        x -= 10.0;
    } while (sprite_id /= 10);
}

unsigned int FUN_8c016ed2()
{
    unsigned int r = var_8c1ba1cc.field_0x00 + 1;
    return r % 7;
}

void FUN_8c016ee6()
{
    float x;
    int a, sprite_id;

    a = var_8c1ba1cc.field_0x00;
    if (a < 10) {
        x = 84.0;
    } else {
        x = 95.0;
    }
    FUN_8c016e6c(a, x, 82.0);

    if (a == 15) {
        sprite_id = 13;
    } else if (a == 23) {
        sprite_id = 14;
    } else {
        sprite_id = FUN_8c016ed2() + 6;
    }

    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupA_0x00,
        sprite_id,
        112.0,
        82.0,
        -4.0
    );

    FUN_8c016e6c(var_8c1ba1cc.field_0x90, 534.0, 82.0);
}

typedef struct {
    int state_0x00;
    MenuDialog *dialog_0x04;
    int field_0x08;
    int field_0x0c;
    int field_0x10;
    int field_0x14;
    int *field_0x18;
} Task8c016f98State;

typedef struct {
    TaskAction action;
    void *state;
    int field_0x08;
    void* field_0x0c;
    int field_0x10;
    int field_0x14;
    int *field_0x18;
    int field_0x1c;
} Task8c016f98;

void FUN_dialog_8c016f98(Task8c016f98 *task, Task8c016f98State *state)
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
            if (var_peripheral_8c1ba35c[0].press & PDD_DGT_TA) {
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
            if (!(var_peripheral_8c1ba35c[0].on & PDD_DGT_TA)) {
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
            if (var_peripheral_8c1ba35c[0].press & PDD_DGT_TA) {
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

void FUN_pushDialogTask_8c0170c6(int dialog_index, int *p2)
{
    Task8c016f98 *task;
    Task8c016f98State *state;

    pushTask_8c014ae8(
        var_tasks_8c1ba3c8,
        &FUN_dialog_8c016f98,
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

void FUN_handleCourseMenuCursor_8c017126()
{
    if (var_peripheral_8c1ba35c[0].press & PDD_DGT_TA) {
        if (
            init_runStruct_8c04442c[
                menuState_8c1bc7a8.field_0x3c
                + menuState_8c1bc7a8.field_0x40 * 5
            ]
            .field_0x04 == 0
        ) {
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
            FUN_swapDialogMessageBox_8c017108(15);
        } else {
            FUN_8c010bae(0);
            FUN_8c010bae(1);
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            menuState_8c1bc7a8.state_0x18 = 5;
            menuState_8c1bc7a8.logo_timer_0x68 = 0;
        }
    }

    if (var_peripheral_8c1ba35c[0].press & PDD_DGT_KU) {
        do {
            if (--menuState_8c1bc7a8.field_0x40 < 0) {
                menuState_8c1bc7a8.field_0x40 = 2;
            }
        } while (
            init_runStruct_8c04442c[
                menuState_8c1bc7a8.field_0x40 * 5 + menuState_8c1bc7a8.field_0x3c
            ].field_0x00 == 0
        );

        if (cursorOffTarget_8c016dc6()) {
            menuState_8c1bc7a8.state_0x18 = 4;
        }
    } else if (var_peripheral_8c1ba35c[0].press & PDD_DGT_KD) {
        do {
            if (++menuState_8c1bc7a8.field_0x40 > 2) {
                menuState_8c1bc7a8.field_0x40 = 0;
            }
        } while (
            init_runStruct_8c04442c[
                menuState_8c1bc7a8.field_0x40 * 5 + menuState_8c1bc7a8.field_0x3c
            ].field_0x00 == 0
        );

        if (cursorOffTarget_8c016dc6()) {
            menuState_8c1bc7a8.state_0x18 = 4;
        }
    } else if (var_peripheral_8c1ba35c[0].press & PDD_DGT_KL) {
        do {
            if (--menuState_8c1bc7a8.field_0x3c < 0) {
                menuState_8c1bc7a8.field_0x3c = 4;
            }
        } while (
            init_runStruct_8c04442c[
                menuState_8c1bc7a8.field_0x40 * 5 + menuState_8c1bc7a8.field_0x3c
            ].field_0x00 == 0
        );

        if (cursorOffTarget_8c016dc6()) {
            menuState_8c1bc7a8.state_0x18 = 4;
        }
    } else if (var_peripheral_8c1ba35c[0].press & PDD_DGT_KR) {
        do {
            if (++menuState_8c1bc7a8.field_0x3c > 4) {
                menuState_8c1bc7a8.field_0x3c = 0;
            }
        } while (
            init_runStruct_8c04442c[
                menuState_8c1bc7a8.field_0x40 * 5 + menuState_8c1bc7a8.field_0x3c
            ].field_0x00 == 0
        );

        if (cursorOffTarget_8c016dc6()) {
            menuState_8c1bc7a8.state_0x18 = 4;
        }
    }
}

int FUN_checkCourses_8c0172dc()
{
    int i = 0;
    int j = 0;
    for (; i < 9; i++) {
        if (var_8c1ba1cc.field_0x44[i].field_0x00)
            continue;

        switch (i) {
            case 0:
                continue;

            case 1:
                if (var_8c1ba1cc.field_0x00 < 8 || var_score_8c1ba25c < 4000)
                    continue;
                break;

            case 2:
                if (var_8c1ba1cc.field_0x00 < 9 || var_score_8c1ba25c < 5500)
                    continue;
                break;

            case 3:
                if (var_8c1ba1cc.field_0x00 < 5 || var_score_8c1ba25c < 2000)
                    continue;
                break;

            case 4:
                if (var_8c1ba1cc.field_0x00 < 11 || var_score_8c1ba25c < 8000)
                    continue;
                break;

            case 5:
                if (var_8c1ba1cc.field_0x00 < 13 || var_score_8c1ba25c < 12000)
                    continue;
                break;

            case 6:
                continue;

            case 7:
                if (var_8c1ba1cc.field_0x00 < 3 || var_score_8c1ba25c < 500)
                    continue;
                break;

            case 8:
                if (var_8c1ba1cc.field_0x00 < 6 || var_score_8c1ba25c < 3000)
                    continue;
                break;
        }

        var_8c225fd4[j] = i;
        j++;
    }

    var_8c225fd4[j] = -1;
    return j;
}

void FUN_8c0173e6(void)
{
    int i = 0;
    for (; var_8c225fd4[i] != -1; i++) {
        int j = var_8c225fd4[i];
        var_8c1ba1cc.field_0x44[j].field_0x00 = 1;
        var_8c1ba1cc.field_0x44[j].field_0x01 = 1;
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
        var_dialog_8c225fbc[cur++] = SEQ_CHOOSE_COURSE;
        var_dialog_8c225fbc[cur]   = -1;
        return;
    }

    // Intro first, then choose course, then finish
    if (var_8c1ba1cc.field_0x00 == 1) {
        var_dialog_8c225fbc[cur++] = SEQ_INTRO_BRIEFING;
        var_dialog_8c225fbc[cur++] = SEQ_CHOOSE_COURSE;
        var_dialog_8c225fbc[cur]   = -1;
        return;
    }

    // Special Success
    if (var_8c1bb8bc != 0) {
        var_dialog_8c225fbc[cur++] = SEQ_SUCCESS_2;
        var_dialog_8c225fbc[cur++] = SEQ_CHOOSE_COURSE;
        var_dialog_8c225fbc[cur]   = -1;
        return;
    }

    // Result
    if (var_8c1bb8dc == 0) {
        var_dialog_8c225fbc[cur++] = SEQ_FAILURE;
    } else {
        int award_seq = SEQ_SUCCESS;
        if      (var_award_8c1bb8f8 == 1) award_seq = SEQ_AWARD_BADGE_BRONZE;
        else if (var_award_8c1bb8f8 == 2) award_seq = SEQ_AWARD_BADGE_SILVER;
        else if (var_award_8c1bb8f8 == 3) award_seq = SEQ_AWARD_BADGE_GOLD;
        var_dialog_8c225fbc[cur++] = award_seq;
    }

    // Course unlocked
    if (FUN_checkCourses_8c0172dc() != 0) {
        var_dialog_8c225fbc[cur++] = SEQ_COURSE_UNLOCKED;
    }

    // Passenger letter received
    if (((var_8c1ba1cc.field_0x00 + 1) % 7) == 0) {
        int r = AsqGetRandomInRangeB_121be(6);
        if (var_8c1ba1cc.field_0x2c[r] == 0) {
            var_8c1ba1cc.field_0x2c[r] = 1;
            var_dialog_8c225fbc[cur++] = SEQ_PASSENGER_LETTER_RECEIVED;
        }
    }

    var_dialog_8c225fbc[cur++] = SEQ_CHOOSE_COURSE;

    var_dialog_8c225fbc[cur] = -1;
}


/*
This is the original code for the function above. 
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
            var_dialog_8c225fbc[iVar4_curDialog] = -1;
            return;
        }
        if (iVar3_index == 0)
        {
            // This path always marks and done,
            // except if the loop is explicitly continued
            iVar2 = iVar4_curDialog;
            if (var_8c1bb8b8 != 0)
            {
                if (var_8c1ba1cc.field_0x00 == 1)
                {
                    iVar2 = iVar4_curDialog + 1;
                    // SEQ_INTRO_BRIEFING
                    var_dialog_8c225fbc[iVar4_curDialog] = 0;
                }
                else
                {

                    if (var_8c1bb8bc == 0)
                    {
                        // This is the only path that can lead to a loop continue
                        if (var_8c1bb8dc == 0)
                        {
                            // SEQ_FAILURE
                            var_dialog_8c225fbc[iVar4_curDialog] = 12;
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
                            var_dialog_8c225fbc[iVar4_curDialog] = iVar2;
                            iVar4_curDialog = iVar4_curDialog + 1;
                        }
                        goto LAB_8c01754c;
                    }
                    // SEQ_SUCCESS_2
                    var_dialog_8c225fbc[iVar4_curDialog] = 7;
                    iVar2 = iVar4_curDialog + 1;
                }
            }
            // SEQ_CHOOSE_COURSE
            var_dialog_8c225fbc[iVar2] = 6;
            bVar1_done = true;
            iVar4_curDialog = iVar2 + 1;
        }
        else if (iVar3_index != 1)
        {
            if (iVar3_index == 2)
            {
                iVar2 = FUN_checkCourses_8c0172dc();
                if (iVar2 != 0)
                {
                    // SEQ_COURSE_UNLOCKED
                    var_dialog_8c225fbc[iVar4_curDialog] = 13;
                    iVar4_curDialog = iVar4_curDialog + 1;
                }
            }
            else if (iVar3_index == 3)
            {
                iVar2 = (var_8c1ba1cc.field_0x00 + 1) % 7;
                if (iVar2 == 0)
                {
                    iVar2 = AsqGetRandomInRangeB_121be(6);
                    if (var_8c1ba1cc.field_0x2c[iVar2] == 0)
                    {
                        var_8c1ba1cc.field_0x2c[iVar2] = 1;
                        // SEQ_PASSENGER_LETTER_RECEIVED
                        var_dialog_8c225fbc[iVar4_curDialog] = 14;
                        iVar4_curDialog = iVar4_curDialog + 1;
                    }
                }
            }
            else if (iVar3_index == 4)
            {
                // SEQ_CHOOSE_COURSE
                var_dialog_8c225fbc[iVar4_curDialog] = 6;
                iVar4_curDialog = iVar4_curDialog + 1;
            }
        }
    LAB_8c01754c:
        iVar3_index = iVar3_index + 1;
    } while (true);
} */

void FUN_8c017d54();
void buildCourseMenuDialogFlow_8c017420();
void StoryMenuTask_8c017718();
void FUN_8c017a20();
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

    menuState_8c1bc7a8.field_0x60 = init_8c044c08[var_dialog_8c225fbc[0]]->field_0x04;
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
