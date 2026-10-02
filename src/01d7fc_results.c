/* @unit Results */
#include <shinobi.h>

#include "01d7fc_results.h"
#include "0100bc_sound.h"
#include "011120_asset_queues.h"
#include "012324_input.h"
#include "0129cc_game.h"
#include "013ae8_route.h"
#include "014b8c_backup.h"
#include "014f54_sprite.h"
#include "01614c_replay_menu.h"
#include "015ab8_title.h"
#include "016c58_prompt.h"
#include "016d2c_course_menu.h"
#include "018644_file_select.h"
#include "0193c8_vm_select.h"
#include "01b19c_system_menu.h"
#include "01bb48_vm_game.h"
#include "01f3c0_ending.h"
#include "02a9fc_message_box.h"
#include "02b464_grading.h"
#include "022464_render.h"
#include "014a9c_tasks.h"
#include "1ba1c8_globals.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "strings.h"

/* ====================
 * Non-initialized Globals
 * ====================
 */

STATIC int var_scoreCourseClearBonus_8c2263ec;
STATIC int var_scoreFirstClearBonus_8c2263f0;
STATIC int var_scoreDriverPointsBonus_8c2263f4;
STATIC int var_scoreBadgeBonus_8c2263f8;
STATIC int var_scorePassengerBonus_8c2263fc;
STATIC int var_scoreEventBonus_8c226400;
STATIC int var_scoreTotal_8c226404;
STATIC int var_runFailed_8c226408;

/* ====================
 * Initialized Globals
 * ====================
 */

STATIC Uint16 init_courseClearScoreTable_8c0451a0[10] = {
    240, 320, 340, 200, 280, 300, 200, 220, 260, 0,
};

/* ====================
 * Functions
 * ====================
 */

STATIC void drawScoreDigits_8c01d7fc(int value, float y)
{
    float x = 464.0f;
    int digit;

    do {
        digit = value % 10;
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8, digit + 0x1f, x, y, -3.0f);
        value = value / 10;
        x -= 18.0f;
    } while (value != 0);
}

STATIC void drawPrompt_8c01d864(void)
{
    SpriteDraw_8c014f54(&var_menuState_8c1bc7a8, var_menuState_8c1bc7a8.selected_0x38 + 2, 224.0f, 300.0f, -5.0f);

    if (MessageBoxDrawText_8c02af1c(0xff) == 0) {
        return;
    }

    SpriteDraw_8c014f54(&var_menuState_8c1bc7a8, 1, 0.0f, 0.0f, -4.3f);
}

STATIC void resultsTask_8c01d8e0(void)
{
    Bool pressedA = var_peripherals_8c1ba35c[0].press & PDD_DGT_TA;

    switch (var_menuState_8c1bc7a8.state_0x18) {
    case 0:
        if (var_selectedVm_8c1ba34c != -1) {
            VmSelectUpdateStatus_8c01967c(var_selectedVm_8c1ba34c, init_saveNames_8c044d50[var_saveSlot_8c1ba350], 3);
        }
        if (RouteGetLatch_8c01432a() != 0) {
            return;
        }
        AsqFreeQueues_8c011f7e();
        if (var_runFailed_8c226408 == 0) {
            var_menuState_8c1bc7a8.state_0x18 = 1;
            SndMidiResetFxAndPlay_8c010846(0, var_runSucceeded_8c1bb8dc == 0 ? 6 : 5);
            RenderStartFadeIn_8c022a9c(10);
            return;
        }
        var_isFading_8c226568 = 0;
        var_menuState_8c1bc7a8.selected_0x38 = 0;
        /* A failed run has no score: straight into the save flow. */
        /* fallthrough */
    case 4: {
        int vmuStatus;
        int soundId;

        VmSelectUpdateStatus_8c01967c(var_selectedVm_8c1ba34c, init_saveNames_8c044d50[var_saveSlot_8c1ba350], 3);
        vmuStatus = var_vmuStatus_8c226048[var_selectedVm_8c1ba34c];
        if (var_isFading_8c226568 != 0) {
            goto tail;
        }
        /* Cached status; state 5 falls back here when it changes. */
        var_menuState_8c1bc7a8.selectedVmuSlot_0x6c = vmuStatus;
        if (vmuStatus == VMU_STATUS_SAVING_POSSIBLE || vmuStatus == VMU_STATUS_SAVE_EXISTS_NO_SPACE ||
            vmuStatus == VMU_STATUS_SAVE_EXISTS) {
            MessageBoxSetText_8c02aefc(
                vmuStatus == VMU_STATUS_SAVING_POSSIBLE ? MSG_CONFIRM_CREATE_FILE : MSG_CONFIRM_OVERWRITE);
            var_menuState_8c1bc7a8.state_0x18 = 8;
            soundId = 0;
        } else {
            if (vmuStatus == VMU_STATUS_NOT_ENOUGH_SPACE) {
                MessageBoxSetText_8c02aefc(MSG_SAVE_NEED_3_BLOCKS);
            } else if (vmuStatus == VMU_STATUS_NOT_AVAILABLE) {
                MessageBoxSetText_8c02aefc(MSG_VM_CANT_SAVE);
            } else if (vmuStatus != VMU_STATUS_NOT_CONNECTED) {
                RenderStartFadeIn_8c022a9c(10);
                return;
            } else {
                MessageBoxSetText_8c02aefc(MSG_VM_NOT_CONN_SAVE);
            }
            soundId = 2;
            var_menuState_8c1bc7a8.state_0x18 = 5;
        }
        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, soundId, 0);
        RenderStartFadeIn_8c022a9c(10);
        return;
    }

    case 1:
        if (!var_isFading_8c226568) {
            var_menuState_8c1bc7a8.state_0x18 = 2;
            var_menuState_8c1bc7a8.counter_0x64 = 0;
            var_menuState_8c1bc7a8.timer_0x68 = 0;
        }
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8, 0x1b, 0.0f, 0.0f, -4.0f);
        goto tail;

    case 2:
        var_menuState_8c1bc7a8.timer_0x68++;
        if (var_menuState_8c1bc7a8.timer_0x68 > 10) {
            var_menuState_8c1bc7a8.counter_0x64++;
            var_menuState_8c1bc7a8.timer_0x68 = 0;
            sdMidiPlay(var_midiHandles_8c0fcd28[4], 1, 6, 0);
        }
        if (var_menuState_8c1bc7a8.counter_0x64 == 7) {
            if (var_award_8c1bb8f8 == AWARD_TIER_GOLD) {
                SndMidiResetFxAndPlay_8c010846(5, 2);
            } else if (var_award_8c1bb8f8 == AWARD_TIER_SILVER) {
                SndMidiResetFxAndPlay_8c010846(5, 3);
            } else if (var_award_8c1bb8f8 == AWARD_TIER_BRONZE) {
                SndMidiResetFxAndPlay_8c010846(5, 4);
            }
            var_menuState_8c1bc7a8.state_0x18 = 3;
        }
        switch (var_menuState_8c1bc7a8.counter_0x64) {
        case 7:
            drawScoreDigits_8c01d7fc(var_scoreTotal_8c226404, 312.0f);
            /* fallthrough */
        case 6:
            drawScoreDigits_8c01d7fc(var_scoreEventBonus_8c226400, 266.0f);
            /* fallthrough */
        case 5:
            drawScoreDigits_8c01d7fc(var_scorePassengerBonus_8c2263fc, 232.0f);
            /* fallthrough */
        case 4:
            drawScoreDigits_8c01d7fc(var_scoreBadgeBonus_8c2263f8, 198.0f);
            /* fallthrough */
        case 3:
            drawScoreDigits_8c01d7fc(var_scoreDriverPointsBonus_8c2263f4, 164.0f);
            /* fallthrough */
        case 2:
            drawScoreDigits_8c01d7fc(var_scoreFirstClearBonus_8c2263f0, 130.0f);
            /* fallthrough */
        case 1:
            drawScoreDigits_8c01d7fc(var_scoreCourseClearBonus_8c2263ec, 96.0f);
            break;
        }
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8, 0x1b, 0.0f, 0.0f, -4.0f);
        goto tail;

    case 3:
        if (pressedA) {
            /* Nothing to save: no VM picked, free run, or the last day done. */
            if (var_selectedVm_8c1ba34c == -1 || var_gameMode_8c1bb8fc != 0 ||
                var_progress_8c1ba1cc.days_0x00 > 0x1e) {
                var_menuState_8c1bc7a8.state_0x18 = 0xd;
            } else {
                var_menuState_8c1bc7a8.state_0x18 = 4;
            }
            RenderStartFadeOut_8c022b60(10);
        }
        drawScoreDigits_8c01d7fc(var_scoreTotal_8c226404, 312.0f);
        drawScoreDigits_8c01d7fc(var_scoreEventBonus_8c226400, 266.0f);
        drawScoreDigits_8c01d7fc(var_scorePassengerBonus_8c2263fc, 232.0f);
        drawScoreDigits_8c01d7fc(var_scoreBadgeBonus_8c2263f8, 198.0f);
        drawScoreDigits_8c01d7fc(var_scoreDriverPointsBonus_8c2263f4, 164.0f);
        drawScoreDigits_8c01d7fc(var_scoreFirstClearBonus_8c2263f0, 130.0f);
        drawScoreDigits_8c01d7fc(var_scoreCourseClearBonus_8c2263ec, 96.0f);
        if (var_award_8c1bb8f8 != 0) {
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8, 0x1f - var_award_8c1bb8f8, 0.0f, 0.0f, -4.0f);
        }
        SpriteDraw_8c014f54(&var_menuState_8c1bc7a8, 0x1b, 0.0f, 0.0f, -4.0f);
        goto tail;

    case 5: {
        int textboxActive;

        VmSelectUpdateStatus_8c01967c(var_selectedVm_8c1ba34c, init_saveNames_8c044d50[var_saveSlot_8c1ba350], 3);
        if (!var_isFading_8c226568) {
            if (var_vmuStatus_8c226048[var_selectedVm_8c1ba34c] == var_menuState_8c1bc7a8.selectedVmuSlot_0x6c) {
                if (pressedA) {
                    MessageBoxSetText_8c02aefc(MSG_RESULT_CANCEL_SAVE);
                    var_menuState_8c1bc7a8.state_0x18 = 6;
                    var_menuState_8c1bc7a8.selected_0x38 = 1;
                }
            } else {
                var_menuState_8c1bc7a8.state_0x18 = 4;
            }
        }
        textboxActive = MessageBoxDrawText_8c02af1c(0xff);
        if (textboxActive != 0) {
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8, 1, 0.0f, 0.0f, -4.3f);
        }
        goto tail;
    }
    case 6: {
        int promptResult;

        VmSelectUpdateStatus_8c01967c(var_selectedVm_8c1ba34c, init_saveNames_8c044d50[var_saveSlot_8c1ba350], 3);
        promptResult = PromptHandleBinary_8c016caa(&var_menuState_8c1bc7a8.selected_0x38);
        if (promptResult == 1) {
            var_menuState_8c1bc7a8.state_0x18 = 0xd;
            RenderStartFadeOut_8c022b60(10);
        } else if (promptResult == 2) {
            var_menuState_8c1bc7a8.state_0x18 = 7;
        }
        break;
    }
    case 7:
        if (!var_isFading_8c226568) {
            var_menuState_8c1bc7a8.state_0x18 = 4;
            RenderStartFadeIn_8c022a9c(10);
            return;
        }
        break;
    case 8:
        if (!var_isFading_8c226568) {
            var_menuState_8c1bc7a8.state_0x18 = 9;
            var_menuState_8c1bc7a8.subState_0x1c = 0;
            var_menuState_8c1bc7a8.selected_0x38 = 0;
        }
        break;
    case 9: {
        int promptResult;
        int vmuStatus;

        if (var_menuState_8c1bc7a8.subState_0x1c == 0) {
            VmSelectUpdateStatus_8c01967c(var_selectedVm_8c1ba34c, init_saveNames_8c044d50[var_saveSlot_8c1ba350], 3);
            vmuStatus = var_vmuStatus_8c226048[var_selectedVm_8c1ba34c];
            promptResult = PromptHandleBinary_8c016caa(&var_menuState_8c1bc7a8.selected_0x38);
            if (promptResult == 1) {
                if (var_runFailed_8c226408 == 1) {
                    FileSelectResetProgress_8c01890a();
                }
                SystemMenuWriteToVmu_8c01b26c();
                var_menuState_8c1bc7a8.subState_0x1c = 1;
                MessageBoxSetText_8c02aefc(MSG_SAVING_NO_POWER_OFF);
                VmGameSetLcdAnim_8c01c8fc(1);
            } else if (promptResult == 2) {
                var_menuState_8c1bc7a8.state_0x18 = 0xd;
                RenderStartFadeOut_8c022b60(10);
            } else if (vmuStatus != VMU_STATUS_SAVING_POSSIBLE && vmuStatus != VMU_STATUS_SAVE_EXISTS_NO_SPACE &&
                       vmuStatus != VMU_STATUS_SAVE_EXISTS) {
                var_menuState_8c1bc7a8.state_0x18 = 4;
                var_isFading_8c226568 = 0;
                sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 2, 0);
            }
        } else if (var_menuState_8c1bc7a8.subState_0x1c == 1) {
            BupGetInfo_8c014bba(var_selectedVm_8c1ba34c);
            if (buStat(var_selectedVm_8c1ba34c) == 0) {
                if (buGetLastError(var_selectedVm_8c1ba34c) == 0) {
                    MessageBoxSetText_8c02aefc(MSG_SAVE_DONE);
                    var_menuState_8c1bc7a8.state_0x18 = 0xd;
                } else {
                    MessageBoxSetText_8c02aefc(MSG_SAVE_FAILED);
                    var_menuState_8c1bc7a8.state_0x18 = 10;
                }
                var_vmBusy_8c157a7c = 0;
                syFree(var_backupFileImageBuf_8c1ba348);
                var_backupFileImageBuf_8c1ba348 = (void *) -1;
                VmGameSetLcdAnim_8c01c8fc(0);
            }
        }
        break;
    }
    case 10:
        if (pressedA) {
            MessageBoxSetText_8c02aefc(MSG_RESULT_CANCEL_SAVE);
            var_menuState_8c1bc7a8.state_0x18 = 6;
            var_menuState_8c1bc7a8.selected_0x38 = 1;
        }
        break;
    case 0xb:
        /* No path here sets 0xb, so neither it nor 0xc ever runs. */
        if (!var_isFading_8c226568) {
            var_menuState_8c1bc7a8.state_0x18 = 0xc;
        }
        break;
    case 0xc:
        if (pressedA) {
            var_menuState_8c1bc7a8.state_0x18 = 0xd;
            RenderStartFadeOut_8c022b60(10);
        }
        break;
    case 0xd: {
        int textboxActive;

        if (!var_isFading_8c226568) {
            ReplayMenuFreeSessionAssets_8c016182();
            if (var_runFailed_8c226408 != 0) {
                TitleStart_8c015fd6(0);
                return;
            }
            if (var_progress_8c1ba1cc.days_0x00 > 0x1e) {
                CourseMenuBuildCourseUnlockList_8c0172dc();
                CourseMenuApplyUnlocks_8c0173e6();
                EndingStart_8c01f954();
                return;
            }
            CourseMenuReturn_8c017ef2();
            return;
        }
        textboxActive = MessageBoxDrawText_8c02af1c(0xff);
        if (textboxActive != 0) {
            SpriteDraw_8c014f54(&var_menuState_8c1bc7a8, 1, 0.0f, 0.0f, -4.3f);
        }
        goto tail;
    }
    default:
        goto tail;
    }

    drawPrompt_8c01d864();
tail:
    njSetBackColor(0xff418dff, 0xff418dff, 0xff418dff);
}

STATIC void spawnResultsTask_8c01df8e(void)
{
    Task *created_task;
    void *created_state;

    InputSpawnTask_8c0128cc(0);
    TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, GameSoftResetTask_8c012f44, &created_task, &created_state, 0);
    TaskSpawn_8c014ae8(var_tasks_8c1ba3c8, resultsTask_8c01d8e0, &created_task, &created_state, 0);
    var_menuState_8c1bc7a8.state_0x18 = 0;
    njGarbageTexture(var_tex_8c157af8, 0xc00);
    MessageBoxOpenTextbox_8c02ae3e(0x20, 0x180, -2.0f, 0x240, 0x40, 0, 0, -1);
    AsqInitQueues_8c011f36(8, 0, 0, 8);
    AsqResetQueues_8c011f6c();
    CourseMenuRequestSysResgrp_8c018568(&var_menuState_8c1bc7a8.resourceGroupB_0x0c, &init_titleResourceGroup_8c044254);
    CourseMenuRequestCommonResources_8c01852c();
    AsqRequestDat_8c011182("\\SYSTEM", "bus_mem.VMI", &var_vmuIconFileBuf_8c1ba344);
    RouteSetLatch_8c014330();
    AsqProcessQueues_8c011fe0(AsqNop_8c011120, 0, 0, 0, RouteClearLatch_8c014322);
}

void ResultsShowPassedRun_8c01e0b4(void)
{
    int courseGroup;
    int courseIndex;

    if (var_runSucceeded_8c1bb8dc == 0) {
        var_scoreCourseClearBonus_8c2263ec = 0;
    } else {
        var_scoreCourseClearBonus_8c2263ec =
            init_courseClearScoreTable_8c0451a0[var_route_8c18ad1c * 3 + var_timeOfDay_8c18ad20];
    }
    if (var_firstClearOfCourse_8c1bb8e0 == 0) {
        var_scoreFirstClearBonus_8c2263f0 = 0;
    } else {
        var_scoreFirstClearBonus_8c2263f0 = 100;
    }
    var_scoreDriverPointsBonus_8c2263f4 = var_runState_8c2285c4.driverPoints_0x0c * 10;
    if (var_route_8c18ad1c == ROUTE_SHINJUKU) {
        courseGroup = 1;
    } else if (var_route_8c18ad1c == ROUTE_WANGAN) {
        courseGroup = 0;
    } else {
        /* courses_0x44 orders routes Wangan, Shinjuku, Ome; ROUTE_* and the
         * score table are Shinjuku, Wangan, Ome. The original leaves the
         * register unset for any other route. */
        courseGroup = 2;
    }
    courseIndex = courseGroup * 3 + var_timeOfDay_8c18ad20;
    if (var_runState_8c2285c4.driverPoints_0x0c < 0x5a) {
        if (var_runState_8c2285c4.driverPoints_0x0c < 0x50) {
            if (var_runState_8c2285c4.driverPoints_0x0c < 0x46) {
                var_award_8c1bb8f8 = AWARD_TIER_NONE;
            } else {
                var_award_8c1bb8f8 = AWARD_TIER_BRONZE;
            }
        } else {
            var_award_8c1bb8f8 = AWARD_TIER_SILVER;
        }
    } else {
        var_award_8c1bb8f8 = AWARD_TIER_GOLD;
    }
    if (var_progress_8c1ba1cc.courses_0x44[courseIndex].storyAward_0x03 < var_award_8c1bb8f8) {
        if (var_award_8c1bb8f8 == AWARD_TIER_GOLD) {
            var_scoreBadgeBonus_8c2263f8 = 200;
        } else if (var_award_8c1bb8f8 == AWARD_TIER_SILVER) {
            var_scoreBadgeBonus_8c2263f8 = 100;
        } else if (var_award_8c1bb8f8 == AWARD_TIER_BRONZE) {
            var_scoreBadgeBonus_8c2263f8 = 50;
        }
        var_progress_8c1ba1cc.courses_0x44[courseIndex].storyAward_0x03 = var_award_8c1bb8f8;
        if (var_progress_8c1ba1cc.courses_0x44[courseIndex].freeRunAward_0x04 < var_award_8c1bb8f8) {
            var_progress_8c1ba1cc.courses_0x44[courseIndex].freeRunAward_0x04 = var_award_8c1bb8f8;
        }
    } else {
        var_award_8c1bb8f8 = AWARD_TIER_NONE;
        var_scoreBadgeBonus_8c2263f8 = 0;
    }
    var_scorePassengerBonus_8c2263fc = var_passengerCount_8c1bb8e4 * 5;
    var_scoreEventBonus_8c226400 = var_eventCount_8c1bb8e8 * 0x32;
    var_scoreTotal_8c226404 = var_scoreCourseClearBonus_8c2263ec + var_scoreFirstClearBonus_8c2263f0 + var_scoreDriverPointsBonus_8c2263f4 + var_scoreBadgeBonus_8c2263f8 + var_scorePassengerBonus_8c2263fc +
                   var_scoreEventBonus_8c226400;
    var_progress_8c1ba1cc.exp_0x90 = var_progress_8c1ba1cc.exp_0x90 + var_scoreTotal_8c226404;
    if (99999 < var_progress_8c1ba1cc.exp_0x90) {
        var_progress_8c1ba1cc.exp_0x90 = 99999;
    }
    var_runFailed_8c226408 = 0;
    spawnResultsTask_8c01df8e();
}

void ResultsShowFailedRun_8c01e24e(void)
{
    var_runFailed_8c226408 = 1;
    spawnResultsTask_8c01df8e();
}
