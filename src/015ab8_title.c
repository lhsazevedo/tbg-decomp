/* @unit Title */

#include <shinobi.h>
#include <sg_sd.h>
#include "012324_input.h"
#include "0129cc_game.h"
#include "013ae8_route.h"
#include "014a9c_tasks.h"
#include "014f54_sprite.h"
#include "015034_text.h"
#include "015ab8_title.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "0100bc_sound.h"
#include "0193c8_vm_select.h"
#include "011120_asset_queues.h"
#include "016d2c_course_menu.h"
#include "01614c_replay_menu.h"
#include "022464_render.h"
#include "02a9fc_message_box.h"
#include "1ba1c8_globals.h"

/* ====================
 * Non-initialized Globals
 * ====================
 */

MenuState var_menuState_8c1bc7a8;

/* ====================
 * Initialized Globals
 * ====================
 */

/* Unreferenced: CourseMenuRequestCommonResources_8c01852c inlines these
 * three names instead. */
ResourceGroupInfo init_commonResourceGroup_8c044244 = {
    "common_parts.dat",
    "common.dat",
    "common.pvm",
    1
};
ResourceGroupInfo init_titleResourceGroup_8c044254 = {
    "title_parts.dat",
    "title.dat",
    "title.pvm",
    1
};
ResourceGroupInfo init_mainMenuResourceGroup_8c044264 = {
    "menu_parts.dat",
    "menu.dat",
    "menu.pvm",
    2
};
ResourceGroupInfo init_practice01ResourceGroup_8c044274 = {
    "practice01_parts.dat",
    "practice01.dat",
    "practice01.pvm",
    3
};
ResourceGroupInfo init_practice02ResourceGroup_8c044284 = {
    "practice02_parts.dat",
    "practice02.dat",
    "practice02.pvm",
    5
};


/* ====================
 * Functions
 * ====================
 */

STATIC void titleTask_8c015ab8(Task* task, void *state) {

    if (var_menuState_8c1bc7a8.state_0x18 >= TITLE_STATE_0X0B_BUS_SLIDE
        && var_menuState_8c1bc7a8.state_0x18 <= TITLE_STATE_0X0C_FLAG_REVEAL) {
            if (var_peripherals_8c1ba35c[0].press & PDD_DGT_ST) {
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);

                var_peripherals_8c1ba35c[0].press = 0;
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X0E_PRESS_START;
                LOG_DEBUG(("[TITLE] State changed: 0X0E_PRESS_START\n"));
                var_isFading_8c226568 = FALSE;
            }
    }

    switch (var_menuState_8c1bc7a8.state_0x18) {
        case TITLE_STATE_0X00_INIT: {
            if (RouteGetLatch_8c01432a() == FALSE) {
                AsqFreeQueues_8c011f7e();
                VmSelectMountAll_8c01940e();

                if (task->field_0x08 == FALSE) {
                    var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X01_FORTYFIVE_FADE_IN;
                    LOG_DEBUG(("[TITLE] State changed: 0X01_FORTYFIVE_FADE_IN\n"));

                    RenderStartFadeIn_8c022a9c(20);

                    njSetBackColor(0xff000000, 0xff000000, 0xff000000);
                } else {
                    var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X0D_TITLE_FADE_IN_DIRECT;
                    LOG_DEBUG(("[TITLE] State changed: 0X0D_TITLE_FADE_IN_DIRECT\n"));

                    RenderStartFadeIn_8c022a9c(10);

                    njSetBackColor(0xffffffff, 0xffffffff, 0xffffffff);
                }
            }

            break;
        }

        case TITLE_STATE_0X01_FORTYFIVE_FADE_IN: {
            if (var_isFading_8c226568 == FALSE) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X02_FORTYFIVE;
                LOG_DEBUG(("[TITLE] State changed: 0X02_FORTYFIVE\n"));
                var_menuState_8c1bc7a8.timer_0x68 = 0;
            }

            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0, 0.0, 0.0, -5.0);

            break;
        }

        case TITLE_STATE_0X02_FORTYFIVE: {
            if (++var_menuState_8c1bc7a8.timer_0x68 > 30) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X03_FORTYFIVE_FADE_OUT;
                LOG_DEBUG(("[TITLE] State changed: 0X03_FORTYFIVE_FADE_OUT\n"));
                RenderStartFadeOut_8c022b60(20);
            }

            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0, 0.0, 0.0, -5.0);

            break;
        }

        case TITLE_STATE_0X03_FORTYFIVE_FADE_OUT: {
            if (var_isFading_8c226568 == FALSE) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X04_ADX_FADE_IN;
                LOG_DEBUG(("[TITLE] State changed: 0X04_ADX_FADE_IN\n"));
                RenderStartFadeIn_8c022a9c(20);
                return;
            }

            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0, 0.0, 0.0, -5.0);

            break;
        }

        case TITLE_STATE_0X04_ADX_FADE_IN: {
            if (var_isFading_8c226568 == FALSE) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X05_ADX;
                LOG_DEBUG(("[TITLE] State changed: 0X05_ADX\n"));
                var_menuState_8c1bc7a8.timer_0x68 = 0;
            }

            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 3, 0.0, 0.0, -5.0);

            break;
        }

        case TITLE_STATE_0X05_ADX: {
            if (++var_menuState_8c1bc7a8.timer_0x68 > 30) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X06_ADX_FADE_OUT;
                LOG_DEBUG(("[TITLE] State changed: 0X06_ADX_FADE_OUT\n"));
                RenderStartFadeOut_8c022b60(20);
            }

            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 3, 0.0, 0.0, -5.0);

            break;
        }

        case TITLE_STATE_0X06_ADX_FADE_OUT: {
            if (var_isFading_8c226568 == FALSE) {
                /* The no-save warning shows once per power-on. */
                if (InputCheckColdBoot_8c012984() != FALSE && VmSelectUpdateAllStatus_8c019550(init_saveNames_8c044d50, 3) == FALSE) {
                    var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X07_VMU_WARNING_FADE_IN;
                    LOG_DEBUG(("[TITLE] State changed: 0X07_VMU_WARNING_FADE_IN\n"));
                    RenderStartFadeIn_8c022a9c(10);
                    return;
                }

                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X0A_TITLE_FADE_IN;
                LOG_DEBUG(("[TITLE] State changed: 0X0A_TITLE_FADE_IN\n"));

                RenderStartFadeIn_8c022a9c(10);
                return;
            } 

            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 3, 0.0, 0.0, -5.0);

            break;
        }

        case TITLE_STATE_0X07_VMU_WARNING_FADE_IN: {
            if (var_isFading_8c226568 == FALSE) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X08_VMU_WARNING;
                LOG_DEBUG(("[TITLE] State changed: 0X08_VMU_WARNING\n"));
            }

            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 17, 0.0, 0.0, -5.0);

            njSetBackColor(0xffffffff, 0xffffffff, 0xffffffff);
            break;
        }

        case TITLE_STATE_0X08_VMU_WARNING: {
            if (
                var_peripherals_8c1ba35c[0].press & (PDD_DGT_TA | PDD_DGT_ST)
                || VmSelectUpdateAllStatus_8c019550(init_saveNames_8c044d50, 3)
            ) {
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X09_VMU_WARNING_FADE_OUT;
                LOG_DEBUG(("[TITLE] State changed: 0X09_VMU_WARNING_FADE_OUT\n"));
                RenderStartFadeOut_8c022b60(10);
            }

            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 17, 0.0, 0.0, -5.0);

            break;
        }

        case TITLE_STATE_0X09_VMU_WARNING_FADE_OUT: {
            if (var_isFading_8c226568 == FALSE) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X0A_TITLE_FADE_IN;
                LOG_DEBUG(("[TITLE] State changed: 0X0A_TITLE_FADE_IN\n"));
                RenderStartFadeIn_8c022a9c(10);
                return;
            }

            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 17, 0.0, 0.0, -5.0);
            break;
        }

        case TITLE_STATE_0X0A_TITLE_FADE_IN: {
            if (var_isFading_8c226568 == FALSE) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X0B_BUS_SLIDE;
                LOG_DEBUG(("[TITLE] State changed: 0X0B_BUS_SLIDE\n"));
                var_menuState_8c1bc7a8.pos.title.busX_0x20 = 640;

                SndPlayAdx_8c010cd6(0, 0);
            }

            /* Draw title */
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 2, 0.0, 0.0, -5.0);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 46, 0.0, 0.0, -7.0);

            break;
        }

        case TITLE_STATE_0X0B_BUS_SLIDE: {
            var_menuState_8c1bc7a8.pos.title.busX_0x20 -= 5.111111; /* ~ 46/9 */

            if (var_menuState_8c1bc7a8.pos.title.busX_0x20 <= 180) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X0C_FLAG_REVEAL;
                LOG_DEBUG(("[TITLE] State changed: 0X0C_FLAG_REVEAL\n"));
                var_menuState_8c1bc7a8.pos.title.flagY_0x24 = 167.0;

                /* No break in the original: the flag already steps on the
                 * frame the bus lands. */
                goto flagReveal;
            }

            /* Draw bus */
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 1, var_menuState_8c1bc7a8.pos.title.busX_0x20, 0.0, -4.0);

            /* Draw title */
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 2, 0.0, 0.0, -5.0);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 46, 0.0, 0.0, -7.0);

            break;
        }

        case TITLE_STATE_0X0C_FLAG_REVEAL: {
            flagReveal:
            var_menuState_8c1bc7a8.pos.title.flagY_0x24 -= 2.3333333; /* ~ 7/3 */

            if (var_menuState_8c1bc7a8.pos.title.flagY_0x24 <= 97) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X0E_PRESS_START;
                LOG_DEBUG(("[TITLE] State changed: 0X0E_PRESS_START\n"));
            }

            /* Draw flag */
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 4, 302, var_menuState_8c1bc7a8.pos.title.flagY_0x24, -4.5);

            /* Draw bus */
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 1, 180, 0.0, -4.0);

            /* Draw title */
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 2, 0.0, 0.0, -5.0);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 46, 0.0, 0.0, -7.0);

            break;
        }

        case TITLE_STATE_0X0D_TITLE_FADE_IN_DIRECT: {
            if (var_isFading_8c226568 == FALSE) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X0E_PRESS_START;
                LOG_DEBUG(("[TITLE] State changed: 0X0E_PRESS_START\n"));
            }

            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 5, 0, 0, -4.0);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 6, 0, 0, -4.5);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 4, 302, 97, -4.5);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 1, 180, 0, -4.0);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 2, 0, 0, -5.0);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 46, 0, 0, -7.0);

            break;
        }

        case TITLE_STATE_0X0E_PRESS_START: {
            if (var_peripherals_8c1ba35c[0].press & PDD_DGT_ST) {
                SndStartAdxFadeOut_8c010bae(0);
                SndStartAdxFadeOut_8c010bae(1);

                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);

                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X0F_START_PRESSED;
                LOG_DEBUG(("[TITLE] State changed: 0X0F_START_PRESSED\n"));
                var_menuState_8c1bc7a8.timer_0x68 = 0;
            } else {
                /* 1050 idle frames (~17s) and the attract demo takes over. */
                if (++var_menuState_8c1bc7a8.counter_0x64 > 1050) {
                    var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X11_TIME_OUT;
                    LOG_DEBUG(("[TITLE] State changed: 0X11_TIME_OUT\n"));
                    SndStartAdxFadeOut_8c010bae(0);
                    SndStartAdxFadeOut_8c010bae(1);

                    RenderStartFadeOut_8c022b60(60);
                }
            }

            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 5, 0, 0, -4.0);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 6, 0, 0, -4.5);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 4, 302, 97, -4.5);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 1, 180, 0, -4.0);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 2, 0, 0, -5.0);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 46, 0, 0, -7.0);

            break;
        }

        case TITLE_STATE_0X0F_START_PRESSED: {
            if (++var_menuState_8c1bc7a8.timer_0x68 > 10) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X10_START_PRESSED_FADE_OUT;
                LOG_DEBUG(("[TITLE] State changed: 0X10_START_PRESSED_FADE_OUT\n"));
                RenderStartFadeOut_8c022b60(10);
            }

            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 5, 0, 0, -4.0);
            if ((var_menuState_8c1bc7a8.timer_0x68 & 1) != 0) {
                SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 6, 0, 0, -4.5);
            }
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 4, 302, 97, -4.5);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 1, 180, 0, -4.0);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 2, 0, 0, -5.0);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 46, 0, 0, -7.0);
            break;
        }

        case TITLE_STATE_0X10_START_PRESSED_FADE_OUT: {
            VmSelectUpdateAllStatus_8c019550(init_saveNames_8c044d50, 3);

            if (var_isFading_8c226568 == FALSE) {
                if (!init_adxPlaying_8c03bd80) {
                    var_titleActive_8c1bb8c4 = FALSE;

                    VmSelectEnter_8c019e44(task);
                }

                return;
            }
            
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 5, 0, 0, -4.0);

            if ((++var_menuState_8c1bc7a8.timer_0x68 & 1) != 0) {
                SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 6, 0, 0, -4.5);
            }
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 4, 302, 97, -4.5);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 1, 180, 0, -4.0);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 2, 0, 0, -5.0);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 46, 0, 0, -7.0);

            break;
        }

        case TITLE_STATE_0X11_TIME_OUT: {
            if (var_isFading_8c226568 == FALSE) {
                if (init_adxPlaying_8c03bd80 == FALSE) {
                    ReplayMenuFreeSessionAssets_8c016182();
                    TxtStartAttractDemo_8c0159ac();
                }

                return;
            }

            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 5, 0, 0, -4.0);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 6, 0, 0, -4.5);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 4, 302, 97, -4.5);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 1, 180, 0, -4.0);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 2, 0, 0, -5.0);
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 46, 0, 0, -7.0);
            break;
        }

    }
}

void TitleStart_8c015fd6 (Bool direct) {
    Task* created_task;
    void* created_state;
    InputSpawnTask_8c0128cc(0);
    TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, &GameSoftResetTask_8c012f44, &created_task, &created_state, 0);

    njSetBackColor(0,0,0);
    TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, &titleTask_8c015ab8, &created_task, &created_state, 0);
    var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X00_INIT;
    LOG_DEBUG(("[TITLE] State changed: 0X00_INIT\n"));
    var_menuState_8c1bc7a8.counter_0x64 = 0;
    created_task->field_0x08 = direct;
    /* GameSoftResetTask_8c012f44 re-spawns the title only while this is clear. */
    var_titleActive_8c1bb8c4 = 1;

    njGarbageTexture(var_tex_8c157af8, 3072);
    MessageBoxOpenTextbox_8c02ae3e(0x20, 0x178, -2.0, 0x240, 0x40, 0, 0, -1);
    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();
    var_currentSysResGroupInfo_8c225fb0 = (void *) -1;
    CourseMenuRequestSysResgrp_8c018568(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, &init_titleResourceGroup_8c044254);
    CourseMenuRequestCommonResources_8c01852c();
    RouteSetLatch_8c014330();
    AsqProcessQueues_8c011fe0(&AsqNop_8c011120, 0, 0, 0, &RouteClearLatch_8c014322);
}
