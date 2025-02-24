#include <sg_sd.h>
#include "015ab8_title.h"

extern int var_8c1bc7cc;

typedef struct {
    int field_0x00;
    int field_0x04;
    float x_0x08;
    float y_0x0c;
    int field_0x10;
    void *field_0x14;
    int field_0x18c;
} RunStruct;

extern RunStruct init_8c04442c[15];
extern void* const_8c03628c;
extern SDMIDI var_midiHandles_8c0fcd28[7];
extern char var_8c1ba1cc[];
extern int var_8c1ba25c;

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
