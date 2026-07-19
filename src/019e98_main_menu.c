/* @unit MainMenu */
#include <shinobi.h>
// #include <njdef.h>
#include <sg_sd.h>
#include "019e98_main_menu.h"
#include "013ae8_route_load.h"
#include "014a9c_tasks.h"
#include "014f54_text.h"
#include "015ab8_title.h"
#include "0100bc_sound.h"
#include "011120_asset_queues.h"
#include "016d2c_course_menu.h"
#include "022464.h"
#include "014f54_text_pre_data.h"
#include "sectionB.h"
#include "serial_debug.h"

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
    var_menuState_8c1bc7a8.state_0x18 = x;                                         \
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


STATIC void mainMenuTask_8c019e98(Task *task) {
    switch (var_menuState_8c1bc7a8.state_0x18)
    {
        case MAIN_MENU_STATE_INIT: {
            if (RouteLoadIsPvmReady_8c01432a()) {
                return;
            }

            AsqFreeQueues_8c011f7e();
            CHANGE_STATE(MAIN_MENU_STATE_FADE_IN);
            push_fadein_8c022a9c(10);
            return;
        }

        case MAIN_MENU_STATE_FADE_IN: {
            if (var_isFading_8c226568) {
                break;
            }

            CHANGE_STATE(MAIN_MENU_STATE_IDLE);
            break;
        }

        case MAIN_MENU_STATE_IDLE: {
            if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KL) {
                if (var_menuState_8c1bc7a8.selected_0x38 != 0) {
                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                    var_menuState_8c1bc7a8.selected_0x38--;
                    CHANGE_STATE(MAIN_MENU_STATE_ANIMATING_LEFT);
                    var_menuState_8c1bc7a8.startTimer_0x64 = 0;
                    var_menuState_8c1bc7a8.logo_timer_0x68 = 0;
                }
            }
            else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_KR) {
                if (var_menuState_8c1bc7a8.selected_0x38 < 3) {
                    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 3, 0);
                    var_menuState_8c1bc7a8.selected_0x38++;
                    CHANGE_STATE(MAIN_MENU_STATE_ANIMATING_RIGHT);
                    var_menuState_8c1bc7a8.startTimer_0x64 = 0;
                    var_menuState_8c1bc7a8.logo_timer_0x68 = 0;
                }
            } else if (var_peripherals_8c1ba35c[0].press & PDD_DGT_TA) {
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                CHANGE_STATE(MAIN_MENU_STATE_SELECTED);
                push_fadeout_8c022b60(10);
            }
            break;
        }

        case MAIN_MENU_STATE_ANIMATING_RIGHT: {
            if (var_menuState_8c1bc7a8.logo_timer_0x68 % 4 == 0) {
                var_menuState_8c1bc7a8.field_0x5c++;
                var_menuState_8c1bc7a8.startTimer_0x64++;
            }
            var_menuState_8c1bc7a8.logo_timer_0x68++;
            if (var_menuState_8c1bc7a8.startTimer_0x64 >= 2) {
                CHANGE_STATE(MAIN_MENU_STATE_IDLE);
            }
            break;
        }

        case MAIN_MENU_STATE_ANIMATING_LEFT: {
            if (var_menuState_8c1bc7a8.logo_timer_0x68 % 4 == 0) {
                var_menuState_8c1bc7a8.field_0x5c--;
                var_menuState_8c1bc7a8.startTimer_0x64++;
            }
            var_menuState_8c1bc7a8.logo_timer_0x68++;
            if (var_menuState_8c1bc7a8.startTimer_0x64 >= 2) {
                CHANGE_STATE(MAIN_MENU_STATE_IDLE);
            }
            break;
        }

        case MAIN_MENU_STATE_SELECTED: {
            if (var_isFading_8c226568) {
                break;
            }

            switch (var_menuState_8c1bc7a8.selected_0x38)
            {
                // Story / Free Run
                case 0:
                case 1: {
                    int result;
                    var_menuState_8c1bc7a8.field_0x3c = 2;
                    var_menuState_8c1bc7a8.field_0x40 = 0;
                    var_gameMode_8c1bb8fc = var_menuState_8c1bc7a8.selected_0x38;
                    var_shouldShowFreeRunIntro_8c1bb8c0 = 1;
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

    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        0x65 + var_menuState_8c1bc7a8.field_0x5c,
        0,
        0,
        -4.0
    );

    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        0x64,
        0,
        0,
        -5.0
    );

    TxtDrawSprite_8c014f54(
        &var_menuState_8c1bc7a8.resourceGroupA_0x00,
        0x2d,
        0,
        0,
        -7.0
    );
}

void MainMenuSwitchFromTask_8c01a09a(Task* task) {
    TaskSetAction_8c014b3e(task, mainMenuTask_8c019e98);
    CHANGE_STATE(MAIN_MENU_STATE_INIT);
    var_menuState_8c1bc7a8.selected_0x38 = 0;
    var_menuState_8c1bc7a8.field_0x5c = 0;
    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();
    CourseMenuRequestSysResgrp_8c018568(
        &var_menuState_8c1bc7a8.resourceGroupB_0x0c,
        &init_mainMenuResourceGroup_8c044264
    );
    RouteLoadSetPvmReady_8c014330();
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteLoadResetPvmReady_8c014322);
}
