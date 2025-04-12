#include <shinobi.h>
// #include <njdef.h>
#include <sg_sd.h>
#include "014a9c_tasks.h"
#include "015ab8_title.h"
#include "011120_asset_queues.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#define TEX_NUM         3072

#ifdef SERIAL_DEBUG
char *DEBUG_mainMenuStateNames[] = {
    "INIT",
    "FADE_IN",
    "IDLE",
    "ANIMATING",
    "FADE_OUT",
};
#endif

#define CHANGE_STATE(x)                                                        \
    menuState_8c1bc7a8.state_0x18 = x;                                         \
    LOG_DEBUG(("[MAIN_MENU] State changed: %s\n", DEBUG_mainMenuStateNames[x]))


/* =================
 * Type Declarations
 * =================
 */

enum MAIN_MENU_STATE {
    MAIN_MENU_STATE_INIT            = 0,
    MAIN_MENU_STATE_FADE_IN         = 1,
    MAIN_MENU_STATE_IDLE            = 2,
    MAIN_MENU_STATE_ANIMATING_RIGHT = 3,
    MAIN_MENU_STATE_ANIMATING_LEFT  = 4,
    MAIN_MENU_STATE_SELECTED        = 5,
};

/* =====================
 * External Declarations
 * =====================
 */

extern void resetUknPvmBool_8c014322();
extern void drawSprite_8c014f54(ResourceGroup *r4, int r5, float fr4, float fr5, float fr6);
extern void StoryMenuTask_8c017718(Task* task, void* state);
extern void FreeRunMenuTask_8c017ada(Task* task, void* state);
extern void FUN_8c01bfec(Task* task, void* state);
extern void push_fadein_8c022a9c();
extern void push_fadeout_8c022b60();
extern ResourceGroupInfo init_mainMenuResourceGroup_8c044264;
extern ResourceGroupInfo init_8c044e90;
extern SDMIDI var_midiHandles_8c0fcd28[7];
extern NJS_TEXMEMLIST var_tex_8c157af8[TEX_NUM];
extern PDS_PERIPHERAL var_peripheral_8c1ba35c[2];
extern int var_8c1bb8c0;
extern int var_demo_8c1bb8d0;
extern int var_game_mode_8c1bb8fc;
extern void* var_8c1bc454;
extern void* var_resourceGroup_8c2263a8;
extern Bool isFading_8c226568;

/* =======================
 * Non-initialized Globals
 * =======================
 */

/* ===================
 * Initialized Globals
 * ===================
 */

/* =========
 * Functions
 * =========
 */


void MainMenuTask_8c019e98(Task *task) {
    switch (menuState_8c1bc7a8.state_0x18)
    {
        case MAIN_MENU_STATE_INIT: {
            if (getUknPvmBool_8c01432a()) {
                return;
            }

            AsqFreeQueues_11f7e();
            CHANGE_STATE(MAIN_MENU_STATE_FADE_IN);
            push_fadein_8c022a9c(10);
            return;
        }

        case MAIN_MENU_STATE_FADE_IN: {
            if (isFading_8c226568) {
                break;
            }

            CHANGE_STATE(MAIN_MENU_STATE_IDLE);
            break;
        }

        case MAIN_MENU_STATE_IDLE: {
            if (var_peripheral_8c1ba35c[0].press & PDD_DGT_KL) {
                if (menuState_8c1bc7a8.selected_0x38 != 0) {
                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                    menuState_8c1bc7a8.selected_0x38--;
                    CHANGE_STATE(MAIN_MENU_STATE_ANIMATING_LEFT);
                    menuState_8c1bc7a8.startTimer_0x64 = 0;
                    menuState_8c1bc7a8.logo_timer_0x68 = 0;
                }
            }
            else if (var_peripheral_8c1ba35c[0].press & PDD_DGT_KR) {
                if (menuState_8c1bc7a8.selected_0x38 < 3) {
                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                    menuState_8c1bc7a8.selected_0x38++;
                    CHANGE_STATE(MAIN_MENU_STATE_ANIMATING_RIGHT);
                    menuState_8c1bc7a8.startTimer_0x64 = 0;
                    menuState_8c1bc7a8.logo_timer_0x68 = 0;
                }
            } else if (var_peripheral_8c1ba35c[0].press & PDD_DGT_TA) {
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                CHANGE_STATE(MAIN_MENU_STATE_SELECTED);
                push_fadeout_8c022b60(10);
            }
            break;
        }

        case MAIN_MENU_STATE_ANIMATING_RIGHT: {
            if (menuState_8c1bc7a8.logo_timer_0x68 % 4 == 0) {
                menuState_8c1bc7a8.field_0x5c++;
                menuState_8c1bc7a8.startTimer_0x64++;
            }
            menuState_8c1bc7a8.logo_timer_0x68++;
            if (menuState_8c1bc7a8.startTimer_0x64 >= 2) {
                CHANGE_STATE(MAIN_MENU_STATE_IDLE);
            }
            break;
        }

        case MAIN_MENU_STATE_ANIMATING_LEFT: {
            if (menuState_8c1bc7a8.logo_timer_0x68 % 4 == 0) {
                menuState_8c1bc7a8.field_0x5c--;
                menuState_8c1bc7a8.startTimer_0x64++;
            }
            menuState_8c1bc7a8.logo_timer_0x68++;
            if (menuState_8c1bc7a8.startTimer_0x64 >= 2) {
                CHANGE_STATE(MAIN_MENU_STATE_IDLE);
            }
            break;
        }

        case MAIN_MENU_STATE_SELECTED: {
            if (isFading_8c226568) {
                break;
            }

            switch (menuState_8c1bc7a8.selected_0x38)
            {
                // Story / Free Run
                case 0:
                case 1: {
                    int result;
                    menuState_8c1bc7a8.field_0x3c = 2;
                    menuState_8c1bc7a8.field_0x40 = 0;
                    var_game_mode_8c1bb8fc = menuState_8c1bc7a8.selected_0x38;
                    var_8c1bb8c0 = 1;
                    CourseMenuSwitchFromTask_8c017e18(task);
                    break;
                }

                // Option
                case 2: {
                    FUN_8c01b122();
                    break;
                }

                // VM Game
                case 3: {
                    FUN_8c01c880();
                    break;
                }
            }

            return;
        }
    }

    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupB_0x0c,
        0x65 + menuState_8c1bc7a8.field_0x5c,
        0,
        0,
        -4.0
    );

    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupB_0x0c,
        0x64,
        0,
        0,
        -5.0
    );

    drawSprite_8c014f54(
        &menuState_8c1bc7a8.resourceGroupA_0x00,
        0x2d,
        0,
        0,
        -7.0
    );
}

void MainMenuSwitchFromTask_8c01a09a(Task* task) {
    setTaskAction_8c014b3e(task, MainMenuTask_8c019e98);
    CHANGE_STATE(MAIN_MENU_STATE_INIT);
    menuState_8c1bc7a8.selected_0x38 = 0;
    menuState_8c1bc7a8.field_0x5c = 0;
    AsqInitQueues_11f36(8, 0, 0, 8);
    AsqResetQueues_11f6c();
    requestSysResgrp_8c018568(
        &menuState_8c1bc7a8.resourceGroupB_0x0c,
        &init_mainMenuResourceGroup_8c044264
    );
    setUknPvmBool_8c014330();
    AsqProcessQueues_11fe0(AsqNop_11120, 0, 0, 0, resetUknPvmBool_8c014322);
}
