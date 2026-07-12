/* 8c015ab8 */
// April 24, 2022
// For now we don't need to keep reversing this one...

#include <shinobi.h>
#include <sg_sd.h>
#include "012504_input.h"
#include "012f44.h"
#include "013ae8_route_load.h"
#include "014a9c_tasks.h"
#include "014f54_text.h"
#include "015ab8_title.h"
#include "serial_debug.h"
#include "0100bc_sound.h"
#include "0193c8_vm_menu.h"
#include "011120_asset_queues.h"
#include "016d2c_course_menu.h"
#include "01614c.h"
#include "022464.h"
#include "028258.h"
#include "014f54_text_pre_data.h"
#include "0fcd20_sectionB.h"


/* ===================
 * Initialized Globals
 * ===================
 */

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
    2
};
ResourceGroupInfo init_mainMenuResourceGroup_8c044264 = {
    "menu_parts.dat",
    "menu.dat",
    "menu.pvm",
    3
};
ResourceGroupInfo init_8c044274 = {
    "practice01_parts.dat",
    "practice01.dat",
    "practice01.pvm",
    4
};
ResourceGroupInfo init_8c044284 = {
    "practice02_parts.dat",
    "practice02.dat",
    "practice02.pvm",
    5
};


/* ====================
 * Forward Declarations
 * ====================
 */


/* =========
 * Functions
 * =========
 */

STATIC void TitleTask_8c015ab8(Task* task, void *state) {

    if (var_menuState_8c1bc7a8.state_0x18 >= TITLE_STATE_0X0B_BUS_SLIDE /* 8c015aec */
        && var_menuState_8c1bc7a8.state_0x18 <= TITLE_STATE_0X0C_FLAG_REVEAL) { /* 8c015af6 */
            if (var_peripherals_8c1ba35c[0].press & PDD_DGT_ST) { /* 8c015afa */
                /* 8c015b00 */
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);

                var_peripherals_8c1ba35c[0].press = 0;
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X0E_PRESS_START;
                LOG_DEBUG(("[TITLE] State changed: 0X0E_PRESS_START\n"));
                var_isFading_8c226568 = FALSE;
            }
    }

    switch (var_menuState_8c1bc7a8.state_0x18) {
        /* 0x8c015b88 (0x8c015b32 + 4 + 0x052) */
        case TITLE_STATE_0X00_INIT: {
            if (isPvmReady_8c01432a() == FALSE) {
                /* 8c015b96 */
                AsqFreeQueues_8c011f7e();
                VmMenuMountVms_8c01940e();

                if (task->field_0x08 == FALSE) {
                    /* 8c015bd8 */
                    var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X01_FORTYFIVE_FADE_IN;
                    LOG_DEBUG(("[TITLE] State changed: 0X01_FORTYFIVE_FADE_IN\n"));

                    push_fadein_8c022a9c(20);

                    /* 8c015c84 */
                    njSetBackColor(0xff000000, 0xff000000, 0xff000000);
                } else {
                    /* 8c015baa */
                    var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X0D_TITLE_FADE_IN_DIRECT;
                    LOG_DEBUG(("[TITLE] State changed: 0X0D_TITLE_FADE_IN_DIRECT\n"));

                    push_fadein_8c022a9c(10);

                    njSetBackColor(0xffffffff, 0xffffffff, 0xffffffff);
                }
            }

            break;
        }

        /* 0x8c015be4 (0x8c015b32 + 4 + 0x0AE) */
        case TITLE_STATE_0X01_FORTYFIVE_FADE_IN: {
            if (var_isFading_8c226568 == FALSE) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X02_FORTYFIVE;
                LOG_DEBUG(("[TITLE] State changed: 0X02_FORTYFIVE\n"));
                var_menuState_8c1bc7a8.logo_timer_0x68 = 0;
            }

            /* 0x8c015c1a (shared) */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0, 0.0, 0.0, -5.0);

            break;
        }

        /* 0x8c015bf4 (0x8c015b32 + 4 + 0x0BE) */
        case TITLE_STATE_0X02_FORTYFIVE: {
            /* var_menuState_8c1bc7a8.logo_timer_0x68++; */
            if (++var_menuState_8c1bc7a8.logo_timer_0x68 > 30) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X03_FORTYFIVE_FADE_OUT;
                LOG_DEBUG(("[TITLE] State changed: 0X03_FORTYFIVE_FADE_OUT\n"));
                push_fadeout_8c022b60(20);
            }

            /* 0x8c015c1a (shared) */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0, 0.0, 0.0, -5.0);

            break;
        }

        /* 0x8c015c0c (0x8c015b32 + 4 + 0x0D6) */
        case TITLE_STATE_0X03_FORTYFIVE_FADE_OUT: {
            if (var_isFading_8c226568 == FALSE) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X04_ADX_FADE_IN;
                LOG_DEBUG(("[TITLE] State changed: 0X04_ADX_FADE_IN\n"));
                push_fadein_8c022a9c(20);
                return;
            }

            /* 0x8c015c1a (shared) */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 0, 0.0, 0.0, -5.0);

            break;
        }

        /* 0x8c015c1e (0x8c015b32 + 4 + 0x0E8) */
        case TITLE_STATE_0X04_ADX_FADE_IN: {
            if (var_isFading_8c226568 == FALSE) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X05_ADX;
                LOG_DEBUG(("[TITLE] State changed: 0X05_ADX\n"));
                var_menuState_8c1bc7a8.logo_timer_0x68 = 0;
            }

            /* 8c015c68 (shared) */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 3, 0.0, 0.0, -5.0);

            break;
        }

        /* 0x8c015c2e (0x8c015b32 + 4 + 0x0F8) */
        case TITLE_STATE_0X05_ADX: {
            /* var_menuState_8c1bc7a8.logo_timer_0x68++; */
            if (++var_menuState_8c1bc7a8.logo_timer_0x68 > 30) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X06_ADX_FADE_OUT;
                LOG_DEBUG(("[TITLE] State changed: 0X06_ADX_FADE_OUT\n"));
                push_fadeout_8c022b60(20);
            }

            /* 8c015c68 (shared) */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 3, 0.0, 0.0, -5.0);

            break;
        }

        /* 0x8c015c46 (0x8c015b32 + 4 + 0x110) */
        case TITLE_STATE_0X06_ADX_FADE_OUT: {
            if (var_isFading_8c226568 == FALSE) {
                // VMU Check?
                if (setName_8c012984() != FALSE && VmMenuUpdateVmusStatus_8c019550(init_saveNames_8c044d50, 3) == FALSE) {
                    /* 8c015c62 */
                    var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X07_VMU_WARNING_FADE_IN;
                    LOG_DEBUG(("[TITLE] State changed: 0X07_VMU_WARNING_FADE_IN\n"));
                    push_fadein_8c022a9c(10);
                    return;
                }

                /* 8c015cda */
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X0A_TITLE_FADE_IN;
                LOG_DEBUG(("[TITLE] State changed: 0X0A_TITLE_FADE_IN\n"));

                /* 8c015cde */
                push_fadein_8c022a9c(10);
                return;
            } 

            /* 8c015c68 (shared) */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 3, 0.0, 0.0, -5.0);

            break;
        }

        /* 0x8c015c6c (0x8c015b32 + 4 + 0x136) */
        case TITLE_STATE_0X07_VMU_WARNING_FADE_IN: {
            if (var_isFading_8c226568 == FALSE) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X08_VMU_WARNING;
                LOG_DEBUG(("[TITLE] State changed: 0X08_VMU_WARNING\n"));
            }

            /* 8c015c78 */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 17, 0.0, 0.0, -5.0);

            /* 8c015c84 (shared) */
            njSetBackColor(0xffffffff, 0xffffffff, 0xffffffff);
            break;
        }

        /* 0x8c015ca8 (0x8c015b32 + 4 + 0x172) */
        case TITLE_STATE_0X08_VMU_WARNING: {
            if (
                var_peripherals_8c1ba35c[0].press & (PDD_DGT_TA | PDD_DGT_ST)
                || VmMenuUpdateVmusStatus_8c019550(init_saveNames_8c044d50, 3)
            ) {
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X09_VMU_WARNING_FADE_OUT;
                LOG_DEBUG(("[TITLE] State changed: 0X09_VMU_WARNING_FADE_OUT\n"));
                push_fadeout_8c022b60(10);
            }

            /* 8c015ce8 (shared) */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 17, 0.0, 0.0, -5.0);

            break;
        }

        /* 0x8c015cd4 (0x8c015b32 + 4 + 0x19E) */
        case TITLE_STATE_0X09_VMU_WARNING_FADE_OUT: {
            if (var_isFading_8c226568 == FALSE) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X0A_TITLE_FADE_IN;
                LOG_DEBUG(("[TITLE] State changed: 0X0A_TITLE_FADE_IN\n"));
                push_fadein_8c022a9c(10);
                return;
            }

            /* 8c015ce8 (shared) */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 17, 0.0, 0.0, -5.0);
            break;
        }

        /* 0x8c015cf2 (0x8c015b32 + 4 + 0x1BC) */
        case TITLE_STATE_0X0A_TITLE_FADE_IN: {
            if (var_isFading_8c226568 == FALSE) {
                /* 8c015cf2 */
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X0B_BUS_SLIDE;
                LOG_DEBUG(("[TITLE] State changed: 0X0B_BUS_SLIDE\n"));
                var_menuState_8c1bc7a8.pos.title.busX_0x20 = 640;

                /* Related to music */
                snd_8c010cd6(0, 0);
            }

            /* 8c015d7c (shared) - Draw title */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 2, 0.0, 0.0, -5.0);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 46, 0.0, 0.0, -7.0);

            break;
        }

        /* 0x8c015d10 (0x8c015b32 + 4 + 0x1DA) */
        case TITLE_STATE_0X0B_BUS_SLIDE: {
            var_menuState_8c1bc7a8.pos.title.busX_0x20 -= 5.111111; /* ~ 46/9 */

            if (var_menuState_8c1bc7a8.pos.title.busX_0x20 <= 180) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X0C_FLAG_REVEAL;
                LOG_DEBUG(("[TITLE] State changed: 0X0C_FLAG_REVEAL\n"));
                var_menuState_8c1bc7a8.pos.title.flagY_0x24 = 167.0;

                /* Missing break! */
                goto gambi;
            }

            /* 8c015d38 - Draw bus */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 1, var_menuState_8c1bc7a8.pos.title.busX_0x20, 0.0, -4.0);

            /* 8c015d7c (shared) - Draw title */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 2, 0.0, 0.0, -5.0);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 46, 0.0, 0.0, -7.0);

            break;
        }

        /* 0x8c015d4a (0x8c015b32 + 4 + 0x214) */
        case TITLE_STATE_0X0C_FLAG_REVEAL: {
            gambi:
            var_menuState_8c1bc7a8.pos.title.flagY_0x24 -= 2.3333333; /* ~ 7/3 */

            if (var_menuState_8c1bc7a8.pos.title.flagY_0x24 <= 97) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X0E_PRESS_START;
                LOG_DEBUG(("[TITLE] State changed: 0X0E_PRESS_START\n"));
            }

            /* 8c015d6a - Draw flag */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 4, 302, var_menuState_8c1bc7a8.pos.title.flagY_0x24, -4.5);

            /* 8c015da4 - Draw bus */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 1, 180, 0.0, -4.0);

            /* 8c015db4 - Draw title */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 2, 0.0, 0.0, -5.0);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 46, 0.0, 0.0, -7.0);

            break;
        }

        /* 8c015d94 */
        case TITLE_STATE_0X0D_TITLE_FADE_IN_DIRECT: {
            if (var_isFading_8c226568 == FALSE) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X0E_PRESS_START;
                LOG_DEBUG(("[TITLE] State changed: 0X0E_PRESS_START\n"));
            }

            /* 8c015f60 (shared) */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 5, 0, 0, -4.0);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 6, 0, 0, -4.5);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 4, 302, 97, -4.5);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 1, 180, 0, -4.0);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 2, 0, 0, -5.0);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 46, 0, 0, -7.0);

            break;
        }

        /* 0x8c015e18 (0x8c015b32 + 4 + 0x2E2) */
        case TITLE_STATE_0X0E_PRESS_START: {
            if (var_peripherals_8c1ba35c[0].press & PDD_DGT_ST) {
                /* 8c015e20 */
                startAdxFadeOut_8c010bae(0);
                startAdxFadeOut_8c010bae(1);

                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0, 0);

                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X0F_START_PRESSED;
                LOG_DEBUG(("[TITLE] State changed: 0X0F_START_PRESSED\n"));
                var_menuState_8c1bc7a8.logo_timer_0x68 = 0;
            } else {
                /* 8c015e42 */
                if (++var_menuState_8c1bc7a8.startTimer_0x64 > 1050) {
                    /* 8c015e54 */
                    var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X11_TIME_OUT;
                    LOG_DEBUG(("[TITLE] State changed: 0X11_TIME_OUT\n"));
                    startAdxFadeOut_8c010bae(0);
                    startAdxFadeOut_8c010bae(1);

                    push_fadeout_8c022b60(60);
                }
            }

            /* 8c015f60 (shared) */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 5, 0, 0, -4.0);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 6, 0, 0, -4.5);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 4, 302, 97, -4.5);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 1, 180, 0, -4.0);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 2, 0, 0, -5.0);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 46, 0, 0, -7.0);

            break;
        }

        /* 0x8c015e68 (0x8c015b32 + 4 + 0x332) */
        case TITLE_STATE_0X0F_START_PRESSED: {
            if (++var_menuState_8c1bc7a8.logo_timer_0x68 > 10) {
                var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X10_START_PRESSED_FADE_OUT;
                LOG_DEBUG(("[TITLE] State changed: 0X10_START_PRESSED_FADE_OUT\n"));
                push_fadeout_8c022b60(10);
            }

            /* 8c015e7e */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 5, 0, 0, -4.0);
            if ((var_menuState_8c1bc7a8.logo_timer_0x68 & 1) != 0) {
                drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 6, 0, 0, -4.5);
            }
            /* 8c015f7c */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 4, 302, 97, -4.5);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 1, 180, 0, -4.0);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 2, 0, 0, -5.0);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 46, 0, 0, -7.0);
            break;
        }

        /* 0x8c015e98 (0x8c015b32 + 4 + 0x362) */
        case TITLE_STATE_0X10_START_PRESSED_FADE_OUT: {
            VmMenuUpdateVmusStatus_8c019550(init_saveNames_8c044d50, 3);

            if (var_isFading_8c226568 == FALSE) {
                if (!init_8c03bd80) {
                    /* 8c015eb2 */
                    var_8c1bb8c4 = FALSE;

                    /* Push menu task */
                    VmMenuSwitchFromTask_8c019e44(task);
                }

                return;
            }
            
            /* 8c015ed6 */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 5, 0, 0, -4.0);

            if ((++var_menuState_8c1bc7a8.logo_timer_0x68 & 1) != 0) {
                drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 6, 0, 0, -4.5);
            }
            /* 8c015f7c */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 4, 302, 97, -4.5);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 1, 180, 0, -4.0);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 2, 0, 0, -5.0);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 46, 0, 0, -7.0);

            break;
        }

        /* 0x8c015f04 (0x8c015b32 + 4 + 0x3CE) */
        case TITLE_STATE_0X11_TIME_OUT: {
            if (var_isFading_8c226568 == FALSE) {
                if (init_8c03bd80 == FALSE) {
                    FUN_8c016182();
                    FUN_8c0159ac();
                }

                return;
            }

            /* 8c015f60 (shared) */
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 5, 0, 0, -4.0);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 6, 0, 0, -4.5);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 4, 302, 97, -4.5);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 1, 180, 0, -4.0);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, 2, 0, 0, -5.0);
            drawSprite_8c014f54(&var_menuState_8c1bc7a8.resourceGroupA_0x00, 46, 0, 0, -7.0);
            break;
        }

    }
}

/* Matched */
void pushTitle_8c015fd6 (Bool direct) {
    Task* created_task;
    void* created_state;
    pushInputTask_8c0128cc(0);
    pushTask_8c014ae8(var_tasks_8c1ba3c8, &task_8c012f44, &created_task, &created_state, 0);

    njSetBackColor(0,0,0);
    pushTask_8c014ae8(var_tasks_8c1ba3c8, &TitleTask_8c015ab8, &created_task, &created_state, 0);
    var_menuState_8c1bc7a8.state_0x18 = TITLE_STATE_0X00_INIT;
    LOG_DEBUG(("[TITLE] State changed: 0X00_INIT\n"));
    var_menuState_8c1bc7a8.startTimer_0x64 = 0;
    created_task->field_0x08 = direct;
    var_8c1bb8c4 = 1;

    njGarbageTexture(var_tex_8c157af8, 3072);
    FUN_8c02ae3e(0x20, 0x178, -2.0, 0x240, 0x40, 0, 0, -1);
    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();
    var_currentSysResGroupInfo_8c225fb0 = (void *) -1;
    CourseMenuRequestSysResgrp_8c018568(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, &init_titleResourceGroup_8c044254);
    CourseMenuRequestCommonResources_8c01852c();
    setPvmReady_8c014330();
    AsqProcessQueues_8c011fe0(&AsqNop_8c011120, 0, 0, 0, &resetPvmReady_8c014322);
}
