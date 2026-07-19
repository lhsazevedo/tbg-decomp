/* @unit Album */
#include <shinobi.h>
#include <sg_sd.h>
#include "012f44_game.h"
#include "013ae8_route_load.h"
#include "015ab8_title.h"
#include "014a9c_tasks.h"
#include "014f54_text.h"
#include "016d2c_course_menu.h"
#include "011120_asset_queues.h"
#include "0100bc_sound.h"
#include "serial_debug.h"
#include "01614c.h"
#include "022464.h"
#include "sectionD.h"
#include "014f54_text_pre_data.h"
#include "sectionB.h"
#include "01d290_album.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#ifdef SERIAL_DEBUG
char *DEBUG_albumStateNames[] = {
    "INIT",
    "FADE_IN",
    "DIALOG",
    "IDLE",
    "ANIMATING",
    "VIEWING",
    "FADE_OUT",
};
#endif

#define CHANGE_STATE(x)                                                        \
    var_menuState_8c1bc7a8.state_0x18 = x;                                         \
    LOG_DEBUG(("[ALBUM] State changed: %s\n", DEBUG_albumStateNames[x]))

/* =================
 * Type Declarations
 * =================
 */

enum ALBUM_STATE {
    ALBUM_STATE_INIT     = 0,
    ALBUM_STATE_FADE_IN  = 1,
    ALBUM_STATE_DIALOG   = 2,
    ALBUM_STATE_IDLE     = 3,
    ALBUM_STATE_ANIMATING = 4,
    ALBUM_STATE_VIEWING  = 5,
    ALBUM_STATE_FADE_OUT = 6
};

/* ===================
 * Initialized Globals
 * ===================
 */

STATIC ResourceGroupInfo init_albumResourceGroup_8c045160 = {
    "album_parts.dat",
    "album.dat",
    "album.pvm",
    6
};

STATIC NJS_POINT2 init_8c045170[6] = {
    { 147.0, 166.0 },
    { 303.0, 166.0 },
    { 455.0, 166.0 },
    { 147.0, 328.0 },
    { 303.0, 328.0 },
    { 455.0, 328.0 }
};


/* ====================
 * Forward Declarations
 * ====================
 */

STATIC void albumDrawGrid_8c01d290(void);

/* =========
 * Functions
 * =========
 */

/* Draws the received-letter icons over the album grid, then the grid frame. */
STATIC void albumDrawGrid_8c01d290(void)
{
    int i;
    int spriteNo = 1;

    /* State 5 (viewing a letter) hides the grid. */
    if (var_menuState_8c1bc7a8.state_0x18 == ALBUM_STATE_VIEWING) {
        return;
    }

    for (i = 0; i < 6; i++, spriteNo++) {
        if (var_progress_8c1ba1cc.letters_0x2c[i]) {
            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                spriteNo,
                0.0, 0.0, -4.0
            );
        }
    }

    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        0,
        0.0, 0.0, -5.0
    );
}

STATIC void albumMenuTask_8c01d300(Task *task, void *state)
{
    int slot = var_menuState_8c1bc7a8.selected_0x38;
    int press = var_peripherals_8c1ba35c[0].press;

    switch (var_menuState_8c1bc7a8.state_0x18) {
        case ALBUM_STATE_INIT: {
            if (RouteLoadIsPvmReady_8c01432a()) {
                return;
            }
            AsqFreeQueues_8c011f7e();
            CHANGE_STATE(ALBUM_STATE_FADE_IN);
            SndProc_8c010cd6(0, 0x10);
            push_fadein_8c022a9c(10);
            return;
        }

        case ALBUM_STATE_FADE_IN: {
            if (!var_isFading_8c226568) {
                if (task->field_0x08) {
                    CHANGE_STATE(ALBUM_STATE_IDLE);
                } else {
                    CHANGE_STATE(ALBUM_STATE_DIALOG);
                }
            }
            break;
        }

        case ALBUM_STATE_DIALOG: {
            if (press & PDD_DGT_TA) {
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
            } else if (press & PDD_DGT_TB) {
                CHANGE_STATE(ALBUM_STATE_FADE_OUT);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
                SndStartAdxFadeOut_8c010bae(0);
                SndStartAdxFadeOut_8c010bae(1);
                push_fadeout_8c022b60(10);
            }
            break;
        }

        case ALBUM_STATE_IDLE: {
            if (slot < 3) {
                /* Top row (slots 0..2) */
                if (press & PDD_DGT_KD) {
                    /* Move to the received letter in the bottom row whose
                     * column is closest to the one below us. */
                    int target = slot + 3;
                    int i;
                    for (i = 3; i < 6; i++) {
                        if (var_progress_8c1ba1cc.letters_0x2c[i]) break;
                    }
                    if (i < 6) {
                        int best = 9999;
                        for (i = 3; i < 6; i++) {
                            int dist;
                            if (!var_progress_8c1ba1cc.letters_0x2c[i]) continue;
                            dist = i - target;
                            if (dist < 0) dist = -dist;
                            if (best > dist) {
                                slot = i;
                                best = dist;
                            }
                        }
                    }
                } else if (press & PDD_DGT_KL) {
                    do {
                        if (--slot < 0) slot = 2;
                    } while (!var_progress_8c1ba1cc.letters_0x2c[slot]);
                } else if (press & PDD_DGT_KR) {
                    do {
                        if (++slot > 2) slot = 0;
                    } while (!var_progress_8c1ba1cc.letters_0x2c[slot]);
                }
            } else {
                /* Bottom row (slots 3..5) */
                if (press & PDD_DGT_KU) {
                    /* Move to the received letter in the top row whose column
                     * is closest to the one above us. */
                    int target = slot - 3;
                    int i;
                    for (i = 0; i < 3; i++) {
                        if (var_progress_8c1ba1cc.letters_0x2c[i]) break;
                    }
                    if (i < 3) {
                        int best = 9999;
                        for (i = 0; i < 3; i++) {
                            int dist;
                            if (!var_progress_8c1ba1cc.letters_0x2c[i]) continue;
                            dist = i - target;
                            if (dist < 0) dist = -dist;
                            if (best > dist) {
                                slot = i;
                                best = dist;
                            }
                        }
                    }
                } else if (press & PDD_DGT_KL) {
                    do {
                        if (--slot < 3) slot = 5;
                    } while (!var_progress_8c1ba1cc.letters_0x2c[slot]);
                } else if (press & PDD_DGT_KR) {
                    do {
                        if (++slot > 5) slot = 3;
                    } while (!var_progress_8c1ba1cc.letters_0x2c[slot]);
                }
            }

            if (slot != var_menuState_8c1bc7a8.selected_0x38) {
                /* Selection moved: lerp the cursor to the new slot. */
                CHANGE_STATE(ALBUM_STATE_ANIMATING);
                var_menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.x = init_8c045170[slot].x;
                var_menuState_8c1bc7a8.pos.cursor.cursorTarget_0x28.y = init_8c045170[slot].y;
                var_menuState_8c1bc7a8.cursorVelocity_0x30.x =
                    (init_8c045170[slot].x - var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x) / 6.0;
                var_menuState_8c1bc7a8.cursorVelocity_0x30.y =
                    (init_8c045170[slot].y - var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.y) / 6.0;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
            } else if (press & PDD_DGT_TA) {
                /* Open the selected letter. */
                CHANGE_STATE(ALBUM_STATE_VIEWING);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            } else if (press & PDD_DGT_TB) {
                /* Leave the album. */
                CHANGE_STATE(ALBUM_STATE_FADE_OUT);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 1, 0);
                SndStartAdxFadeOut_8c010bae(0);
                SndStartAdxFadeOut_8c010bae(1);
                push_fadeout_8c022b60(10);
            }

            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                0xd,
                var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x,
                var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.y,
                -2.0
            );
            break;
        }

        case ALBUM_STATE_ANIMATING: {
            if (CourseMenuInterpolateCursor_8c016d2c()) {
                CHANGE_STATE(ALBUM_STATE_IDLE);
            }

            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                0xd,
                var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x,
                var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.y,
                -2.0
            );
            break;
        }

        case ALBUM_STATE_VIEWING: {
            if (press & PDD_DGT_TA) {
                CHANGE_STATE(ALBUM_STATE_IDLE);
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
            }

            TxtDrawSprite_8c014f54(
                &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
                slot + 7,
                0.0, 0.0, -3.0
            );
            break;
        }

        case ALBUM_STATE_FADE_OUT: {
            if (!var_isFading_8c226568) {
                if (init_8c03bd80) {
                    return;
                }
                LOG_DEBUG(("[ALBUM] albumMenuTask_8c01d300: fade-out complete, switching screen\n"));
                var_menuState_8c1bc7a8.field_0x3c = 1;
                var_menuState_8c1bc7a8.field_0x40 = 1;
                var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x = 0.0;
                FUN_8c016182();
                CourseMenuFUN_8c017ef2();
                return;
            }
            break;
        }
    }

    albumDrawGrid_8c01d290();
    var_menuState_8c1bc7a8.selected_0x38 = slot;
}

/* Album task entry: reset to INIT, park the cursor on the first received
 * letter, then request the album's resource group. */
void AlbumSwitchFromTask_8c01d6e2(Task *task)
{
    int i;

    LOG_DEBUG(("[ALBUM] AlbumSwitchFromTask_8c01d6e2: install album task\n"));

    TaskSetAction_8c014b3e(task, albumMenuTask_8c01d300);
    CHANGE_STATE(ALBUM_STATE_INIT);

    for (i = 0; i < 6; i++) {
        if (var_progress_8c1ba1cc.letters_0x2c[i]) {
            var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.x = init_8c045170[i].x;
            var_menuState_8c1bc7a8.pos.cursor.cursor_0x20.y = init_8c045170[i].y;
            var_menuState_8c1bc7a8.selected_0x38 = i;
            break;
        }
    }

    /* field_0x08 tells FADE_IN whether a letter is available (-> IDLE vs DIALOG). */
    task->field_0x08 = i < 6;

    CourseMenuFreeResourceGroup_8c0185c4(&var_menuState_8c1bc7a8.resourceGroupA_0x00);
    njGarbageTexture(var_tex_8c157af8, 0xc00);
    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();
    CourseMenuRequestSysResgrp_8c018568(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        &init_albumResourceGroup_8c045160
    );
    RouteLoadSetPvmReady_8c014330();
    AsqProcessQueues_8c011fe0(&AsqNop_8c011120, 0, 0, 0, &RouteLoadResetPvmReady_8c014322);
}
