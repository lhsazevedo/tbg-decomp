#include <shinobi.h>
#include <sg_sd.h>
#include <njdef.h>
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

extern void snd_8c010cd6(int p1, int p2);
extern void setUknPvmBool_8c014330();
extern int requestSysResgrp_8c018568(ResourceGroup* dds, ResourceGroupInfo* rg);
extern RunStruct init_8c04442c[15];
extern void* const_8c03628c;
extern SDMIDI var_midiHandles_8c0fcd28[7];
extern char var_8c1ba1cc[];
extern int var_8c1ba25c;
extern int var_8c225fb4;
extern int var_8c225fb8;
extern PDS_PERIPHERAL var_peripheral_8c1ba35c[2];
extern ResourceGroupInfo init_mainMenuResourceGroup_8c044264;

extern MenuDialog *init_8c044c08[];
extern int var_game_mode_8c1bb8fc;
extern int var_dialog_8c225fbc;
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

int FUN_8c016dc6()
{
    int selected;
    float y;
    float x;

    selected = menuState_8c1bc7a8.field_0x3c + menuState_8c1bc7a8.field_0x40 * 5;
    x = init_8c04442c[selected].x_0x08;
    y = init_8c04442c[selected].y_0x0c;
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
    unsigned int r = *((int *) var_8c1ba1cc) + 1;
    return r % 7;
}

void FUN_8c016ee6()
{
    float x;
    int a, sprite_id;

    a = *((int *) var_8c1ba1cc);
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

    FUN_8c016e6c(((int*) var_8c1ba1cc)[0x24], 534.0, 82.0);
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

void FUN_8c017d54();
void FUN_8c017420();
void StoryMenuTask_8c017718();
void FUN_8c017a20();
void FreeRunMenuTask_8c017ada();

void CourseMenuSwitchFromTask_8c017e18(Task *task)
{
    if (var_game_mode_8c1bb8fc == 0) {
        setTaskAction_8c014b3e(task, StoryMenuTask_8c017718);
        FUN_8c017420();
    } else {
        setTaskAction_8c014b3e(task, FreeRunMenuTask_8c017ada);
        FUN_8c017a20();
    }

    menuState_8c1bc7a8.field_0x60 = init_8c044c08[var_dialog_8c225fbc]->field_0x04;
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
