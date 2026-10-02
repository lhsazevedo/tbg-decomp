/* @unit Passenger */

#include <shinobi.h>
#include <sg_sd.h>
#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "011120_asset_queues.h"
#include "1ba1c8_globals.h"
#include "014a9c_tasks.h"
#include "013ae8_route.h"
#include "015034_text.h"
#include "022464_render.h"
#include "0100bc_sound.h"
#include "024b4c_bus_camera.h"
#include "02a9fc_message_box.h"
#include "02c884_stop.h"
#include "02d06c_stop_draw.h"
#include "02d19c_passenger.h"
#include "02b464_grading.h"

/* ====================
 * Forward Declarations
 * ====================
 */

STATIC void drawPassengerSprite_8c02d19c(int arg0);
STATIC void drawInterior_8c02d1f4(int arg0);
STATIC void setCountUpStep_8c02d5d8(void);

/* ====================
 * Non-initialized Globals
 * ====================
 */

NJS_POINT3 var_boardSpot2_8c228910;
NJS_POINT3 var_boardSpot3_8c22891c;
NJS_POINT3 var_boardSpot1_8c228928;
NJS_POINT3 var_exitSpot1_8c228934;
NJS_POINT3 var_exitSpot2_8c228940;
NJS_POINT3 var_exitSpot3_8c22894c;

int var_passengersFadedOut_8c22895c;

float var_passengerFadeColor_8c228960[5];

/* Set by any passenger task that advanced this frame; cleared by the
 * scene task before it pumps the group, then read back to decide whether
 * the stop is still in progress. Int, not float, despite sitting in the
 * middle of the anchor-point float run. */
STATIC int var_passengerActed_8c228958;

/* ====================
 * Initialized Globals
 * ====================
 */

SeatPos init_seatPositions_8c04c3e4[31] = {
    { 0.8399999737739563f, -0.33000001311302185f },
    { 0.8600000143051147f, 3.950000047683716f },
    { 0.8399999737739563f, 4.78000020980835f },
    { 0.8600000143051147f, 5.599999904632568f },
    { 0.5f, 6.400000095367432f },
    { 0.8399999737739563f, 6.400000095367432f },
    { 0.8600000143051147f, 7.25f },
    { 0.5400000214576721f, 7.25f },
    { -0.8399999737739563f, -0.33000001311302185f },
    { -0.8600000143051147f, 0.550000011920929f },
    { -0.8399999737739563f, 1.5f },
    { -0.8600000143051147f, 2.380000114440918f },
    { -0.8399999737739563f, 3.200000047683716f },
    { -0.8600000143051147f, 3.950000047683716f },
    { -0.8399999737739563f, 4.78000020980835f },
    { -0.8600000143051147f, 5.599999904632568f },
    { -0.5f, 6.400000095367432f },
    { -0.8399999737739563f, 6.400000095367432f },
    { -0.8600000143051147f, 7.25f },
    { -0.5400000214576721f, 7.25f },
    { 0.3499999940395355f, 0.14499999582767487f },
    { 0.20999999344348907f, 0.6399999856948853f },
    { 0.23999999463558197f, 1.600000023841858f },
    { 0.2199999988079071f, 3.4000000953674316f },
    { 0.2800000011920929f, 4.25f },
    { -0.019999999552965164f, 4.599999904632568f },
    { -0.20000000298023224f, 0.27000001072883606f },
    { -0.25f, 0.800000011920929f },
    { -0.28999999165534973f, 1.899999976158142f },
    { -0.20000000298023224f, 2.8399999141693115f },
    { -0.25600001215934753f, 4.0f },
};

/* ====================
 * Functions
 * ====================
 */

/* Installed as a DrawFn (via literal-pool pointer, both by
 * PassengerSeatedTask_8c02d5ca and by the per-passenger task actions below);
 * draws one passenger's sprite, waiting or scripted. arg0 is a
 * StopScheduleState* threaded through the int parameter (the same idiom as
 * StopDrawLightBegin_8c02d0fc/d146 in 02d06c). A stop index (the byte at *entry_0x00) out of
 * range, or whose asset slot has no texlist loaded yet, is skipped. */
STATIC void drawPassengerSprite_8c02d19c(int arg0)
{
    StopScheduleState *state = (StopScheduleState *)arg0;
    Sint8 stopIndex = *(Sint8 *)state->entry_0x00;

    if (stopIndex >= 0x41) {
        return;
    }
    if (var_pedestrianAssets_8c1bbfdc[stopIndex].texlist_0x08 == (NJS_TEXLIST *)-1) {
        return;
    }

    var_passengerSprite_8c2288d8.tlist = var_pedestrianAssets_8c1bbfdc[stopIndex].texlist_0x08;
    var_passengerSprite_8c2288d8.p = state->pos_0x08;
    njDrawSprite3D(&var_passengerSprite_8c2288d8, state->spriteNo_0x14, state->isSeated_0x28 == 0 ? 0x32 : 0x30);
}

/* Installed as a DrawFn (via literal-pool pointer in PassengerStopSceneTask_8c02d644);
 * ignores its arg. Draws the bus interior model under the layer-0 (bus)
 * simple light, sibling of StopDrawLightBegin_8c02d0fc (02d06c) which does the same for
 * the bus-stop anchor points. */
STATIC void drawInterior_8c02d1f4(int arg0)
{
    njCnkSetSimpleLight(var_busSimpleLightDir_8c227db8[0], var_busSimpleLightDir_8c227db8[1], var_busSimpleLightDir_8c227db8[2]);
    njSetTexture(var_interiorTexlist_8c1bc438);
    njCnkSimpleDrawObject(var_interiorNj_8c1bc43c);
}

/* Task action for a scripted-stop slot whose segment differs from the bus's
 * current one -- spawned immediately by StopSpawnInit_8c02d968 (no shuffle, no
 * countdown). Registers the draw callback every frame; ignores task. */
void PassengerSeatedTask_8c02d5ca(Task *task, void *state)
{
    RenderQueueDraw_8c0223ea(RENDER_LAYER_CABIN, drawPassengerSprite_8c02d19c, (int)state);
}

/* Sizes the step PassengerStopSceneTask_8c02d644's case 5 adds to the run
 * clock each frame: a fifth of what is left to the schedule time while that is
 * at least 50 frames, then a flat 10 to close the last stretch. */
STATIC void setCountUpStep_8c02d5d8(void)
{
    int remaining = var_runState_8c2285c4.scheduleTime_0x14 - var_runState_8c2285c4.runClock_0x18;

    if (remaining >= 0x32) {
        var_runState_8c2285c4.clockCatchUpStep_0x1c = remaining / 5;
    } else {
        var_runState_8c2285c4.clockCatchUpStep_0x1c = 0xa;
    }
}

/* Task action for an already-picked waiting passenger (state->state_0x04 ==
 * 1 always, from 02d968); walks it through the boarding animation via the
 * three waypoints var_boardSpot1_8c228928, var_boardSpot2_8c228910 and
 * var_boardSpot3_8c22891c in turn, claiming an empty scripted-stop slot once
 * done so it continues as an ordinary scripted passenger, then re-registers the draw
 * callback in its terminal state (0). Each phase only advances on a frame
 * where var_passengersFadedOut_8c22895c is set, so the passenger never visibly
 * jumps between waypoints. */
void PassengerBoardTask_8c02d21c(Task *task, StopScheduleState *state)
{
    int mode = state->state_0x04;
    int layer = 0;

    switch (mode) {
    case 1:
        if (var_passengersFadedOut_8c22895c != 0) {
            Sint32 n = state->delay_0x20 - 1;
            state->delay_0x20 = n;
            if (n < 0) {
                state->pos_0x08.x = var_boardSpot1_8c228928.x + state->jitterX_0x18;
                state->pos_0x08.y = var_boardSpot1_8c228928.y;
                state->pos_0x08.z = var_boardSpot1_8c228928.z + state->jitterZ_0x1c;
                state->spriteNo_0x14 = 0x10;
                state->state_0x04 = 2;
                if (var_route_8c18ad1c != ROUTE_OME) {
                    sdMidiPlay(var_midiHandles_8c0fcd28[state->voice_0x2c], 1, state->soundIdA_0x30, 0);
                }
            }
        }
        break;

    case 2:
        if (var_passengersFadedOut_8c22895c != 0) {
            state->pos_0x08 = var_boardSpot2_8c228910;
            state->spriteNo_0x14 = (var_route_8c18ad1c == ROUTE_OME) ? 0x18 : 8;
            state->state_0x04 = 3;
            if (var_route_8c18ad1c != ROUTE_OME) {
                sdMidiPlay(var_midiHandles_8c0fcd28[5], 1, AsqGetRandomInRangeA_8c012178(3) + 0x1f, 0);
            }
        }
        layer = 1;
        break;

    case 3:
        if (var_passengersFadedOut_8c22895c != 0) {
            state->pos_0x08.x = var_boardSpot3_8c22891c.x + state->jitterX_0x18;
            state->pos_0x08.y = var_boardSpot3_8c22891c.y;
            state->pos_0x08.z = var_boardSpot3_8c22891c.z + state->jitterZ_0x1c;
            state->state_0x04 = 4;
            if (var_route_8c18ad1c != ROUTE_OME) {
                sdMidiPlay(var_midiHandles_8c0fcd28[state->voice_0x2c], 1, state->soundIdB_0x34, 0);
            }
        }
        layer = 2;
        break;

    case 4:
        if (var_passengersFadedOut_8c22895c != 0) {
            int slot = AsqGetRandomInRangeA_8c012178(31);

            while (var_stopSchedule_8c228718[slot] != -1) {
                slot++;
                if (slot >= 31) {
                    slot = 0;
                }
            }
            var_stopSchedule_8c228718[slot] = (int)state->entry_0x00;
            state->pos_0x08.x = init_seatPositions_8c04c3e4[slot].x;
            state->pos_0x08.z = init_seatPositions_8c04c3e4[slot].z;
            if (slot < 0x14) {
                state->spriteNo_0x14 = 0x20;
                state->pos_0x08.y -= 0.18000000715255737f;
            } else if (slot < 0x1a) {
                state->spriteNo_0x14 = 0x22;
            } else {
                state->spriteNo_0x14 = 0x23;
            }
            state->state_0x04 = 5;
        }
        layer = 2;
        break;

    case 5:
        if (!(1.0f > var_passengerFadeColor_8c228960[0])) {
            state->state_0x04 = 0;
            state->isSeated_0x28 = 1;
        }
        layer = 2;
        break;

    case 0:
        RenderQueueDraw_8c0223ea(RENDER_LAYER_CABIN, drawPassengerSprite_8c02d19c, (int)state);
        return;

    default:
        break;
    }

    if (layer != 0) {
        RenderQueueDraw_8c0223ea(layer, drawPassengerSprite_8c02d19c, (int)state);
    }
    var_passengerActed_8c228958 = 1;
}

/* Task action for a scripted-stop slot that matched the bus's current
 * segment (spawned after the Fisher-Yates shuffle by StopSpawnInit_8c02d968).
 * Sibling of PassengerBoardTask_8c02d21c: same shape (countdown, then walk three door
 * anchor points), but the anchor points, sound-gating route and terminal
 * behavior all differ -- this one frees itself once var_passengersFadedOut_8c22895c fires in
 * its terminal state, instead of looping forever. */
void PassengerExitTask_8c02d46c(Task *task, StopScheduleState *state)
{
    int mode = state->state_0x04;
    int layer = 0;

    switch (mode) {
    case 0:
        if (var_passengersFadedOut_8c22895c != 0) {
            Sint32 n = state->delay_0x20 - 1;
            state->delay_0x20 = n;
            if (n < 0) {
                state->state_0x04 = 6;
            }
        }
        layer = 2;
        break;

    case 6:
        if (!(1.0f > var_passengerFadeColor_8c228960[0])) {
            state->isSeated_0x28 = 0;
        }
        if (var_passengersFadedOut_8c22895c != 0) {
            var_stopSchedule_8c228718[state->slotIndex_0x24] = -1;
            state->pos_0x08.x = var_exitSpot1_8c228934.x + state->jitterX_0x18;
            state->pos_0x08.z = var_exitSpot1_8c228934.z + state->jitterZ_0x1c;
            state->spriteNo_0x14 = (var_route_8c18ad1c == ROUTE_OME) ? 0x20 : 0x10;
            state->state_0x04 = 7;
            if (var_route_8c18ad1c == ROUTE_OME) {
                sdMidiPlay(var_midiHandles_8c0fcd28[state->voice_0x2c], 1, state->soundIdA_0x30, 0);
            }
        }
        layer = 2;
        break;

    case 7:
        if (var_passengersFadedOut_8c22895c != 0) {
            state->pos_0x08 = var_exitSpot2_8c228940;
            state->state_0x04 = 8;
            if (var_route_8c18ad1c == ROUTE_OME) {
                sdMidiPlay(var_midiHandles_8c0fcd28[5], 1, AsqGetRandomInRangeB_8c0121be(3) + 0x1f, 0);
            }
        }
        layer = 2;
        break;

    case 8:
        if (var_passengersFadedOut_8c22895c != 0) {
            state->pos_0x08.x = var_exitSpot3_8c22894c.x + state->jitterX_0x18;
            state->pos_0x08.y = var_exitSpot3_8c22894c.y;
            state->pos_0x08.z = var_exitSpot3_8c22894c.z + state->jitterZ_0x1c;
            state->spriteNo_0x14 = 0x18;
            state->state_0x04 = 9;
            if (var_route_8c18ad1c == ROUTE_OME) {
                sdMidiPlay(var_midiHandles_8c0fcd28[state->voice_0x2c], 1, state->soundIdB_0x34, 0);
            }
        }
        layer = 2;
        break;

    case 9:
        if (var_passengersFadedOut_8c22895c != 0) {
            TaskKill_8c014b66(task);
            return;
        }
        layer = 1;
        break;

    default:
        break;
    }

    if (layer != 0) {
        RenderQueueDraw_8c0223ea(layer, drawPassengerSprite_8c02d19c, (int)state);
    }
    var_passengerActed_8c228958 = 1;
}

/* Per-frame driver for the bus-stop passenger subsystem: pumps
 * var_stopTaskGroup_8c2288f8's tasks, fades var_passengerFadeColor_8c228960[0] (the bus
 * interior's own light level, separate from the anchor-point one in
 * 02d06c) in and out around the fade-arrival overlay, and tears the
 * subsystem down and frees itself when done. Spawned once by StopSpawnInit_8c02d968
 * with a private 2-int state (field_0x00 the phase, field_0x04 a sub-phase
 * used only by phase 1); NOT the 0x38-byte StopScheduleState the other
 * task actions in this unit use. Every call re-registers this frame's
 * interior-draw (drawInterior_8c02d1f4) and light (StopDrawLightBegin_8c02d0fc) DrawFns
 * regardless of phase. */
void PassengerStopSceneTask_8c02d644(Task *task, PassengerStopSceneState *state)
{
    int phase = state->phase_0x00;
    Bool execGroup = FALSE;

    RenderQueueDraw_8c0223ea(RENDER_LAYER_CABIN, drawInterior_8c02d1f4, 0);
    RenderQueueDraw_8c0223ea(RENDER_LAYER_MIRROR, StopDrawLightBegin_8c02d0fc, 0);
    RenderQueueDraw_8c0223ea(RENDER_LAYER_CABIN, StopDrawLightBegin_8c02d0fc, 0);

    switch (phase) {
    case 0:
        if (var_isFading_8c226568 == 0) {
            state->phase_0x00 = 1;
        }
        var_runState_8c2285c4.runClock_0x18++;
        execGroup = TRUE;
        break;

    case 1: {
        /* The original re-enters this sub-state-machine in place (a plain
         * branch back, not a fresh call) when var_playMode_8c1bb8d0 ==
         * PLAY_MODE_DEMO and the task group did something this frame --
         * it does not re-run the three RenderQueueDraw registrations
         * above or re-read phase. */
        for (;;) {
            switch (state->subPhase_0x04) {
            case 0: {
                float v = var_passengerFadeColor_8c228960[0] - 0.06666667014360428f;
                var_passengerFadeColor_8c228960[0] = v;
                if (!(v > 0.0f)) {
                    var_passengerFadeColor_8c228960[0] = 0.0f;
                    var_passengersFadedOut_8c22895c = 1;
                    state->subPhase_0x04 = 1;
                } else {
                    var_passengersFadedOut_8c22895c = 0;
                }
                break;
            }
            case 1: {
                float v = var_passengerFadeColor_8c228960[0] + 0.06666667014360428f;
                var_passengerFadeColor_8c228960[0] = v;
                if (!(1.0f > v)) {
                    var_passengerFadeColor_8c228960[0] = 1.0f;
                    state->subPhase_0x04 = 0;
                }
                var_passengersFadedOut_8c22895c = 0;
                break;
            }
            default:
                break;
            }

            var_passengerActed_8c228958 = 0;
            TaskRunGroup_8c014b42((Task *)var_stopTaskGroup_8c2288f8);
            if (var_passengerActed_8c228958 != 0) {
                var_runState_8c2285c4.runClock_0x18++;
                if (var_playMode_8c1bb8d0 == PLAY_MODE_DEMO) {
                    continue;
                }
                break;
            }

            if (var_cutsceneActive_8c1bb900 == 0 || var_playMode_8c1bb8d0 != PLAY_MODE_NORMAL) {
                var_runState_8c2285c4.runPhase_0x00 = 2;
                state->phase_0x00 = 5;
                setCountUpStep_8c02d5d8();
            } else {
                state->phase_0x00 = 2;
                var_fadeRequest_8c226564 = FADE_REQUEST_OUT;
            }
            var_runState_8c2285c4.runClock_0x18++;
            break;
        }
        break;
    }

    case 2:
        if (var_isFading_8c226568 == 0) {
            state->phase_0x00 = 3;
            var_fadeRequest_8c226564 = FADE_REQUEST_IN;
            var_sceneVisible_8c226560 = 0;
            MessageBoxStart_8c02ad8c();
        }
        execGroup = TRUE;
        break;

    case 3:
        if (var_messageBoxActive_8c22847c == 0) {
            state->phase_0x00 = 4;
        }
        break;

    case 4:
        if (RouteGetLatch_8c01432a() != 0 && var_isFading_8c226568 == 0) {
            var_runState_8c2285c4.runPhase_0x00 = 2;
            state->phase_0x00 = 5;
            setCountUpStep_8c02d5d8();
        }
        execGroup = TRUE;
        break;

    case 5:
        if (var_runState_8c2285c4.runClock_0x18 >= var_runState_8c2285c4.scheduleTime_0x14) {
            StopKillTaskGroup_8c02ca96();
            njReleaseTexture(var_interiorTexlist_8c1bc438);
            if (var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE) {
                var_cameraMode_8c227d9c = 2;
                var_cameraCueState_8c227da4 = 0;
                BusCameraApplyMode_8c024f32();
            } else {
                if (var_playMode_8c1bb8d0 != PLAY_MODE_DEMO) {
                    BusCameraRestoreState_8c024b86();
                }
            }

            var_busState_8c1bb9d0.driveState_0x2b4 = 1;
            var_busState_8c1bb9d0.signalSide_0x25c =
                (var_playMode_8c1bb8d0 != PLAY_MODE_PRACTICE && var_route_8c18ad1c == ROUTE_OME &&
                 var_currentSegment_8c228708 == 0) ? 1 : 2;

            var_viewLayout_8c22655c = 2;

            if (var_playMode_8c1bb8d0 != PLAY_MODE_PRACTICE) {
                int soundOffset;

                switch (var_route_8c18ad1c) {
                case ROUTE_SHINJUKU:
                    soundOffset = 2;
                    break;
                case ROUTE_WANGAN:
                    soundOffset = 8;
                    break;
                case ROUTE_OME:
                    soundOffset = 5;
                    break;
                default:
                    /* unreachable: var_route_8c18ad1c only ever holds the three
                     * ROUTE_* values above -- the original leaves the register
                     * that becomes this arg uninitialized in this case. */
                    soundOffset = 0;
                    break;
                }
                SndPlayAdx_8c010cd6(0, var_timeOfDay_8c18ad20 + soundOffset);
            }

            TaskKill_8c014b66(task);
            return;
        }

        var_runState_8c2285c4.runClock_0x18 += var_runState_8c2285c4.clockCatchUpStep_0x1c;
        if (var_runState_8c2285c4.runClock_0x18 >= var_runState_8c2285c4.scheduleTime_0x14) {
            var_runState_8c2285c4.runClock_0x18 = var_runState_8c2285c4.scheduleTime_0x14;
        }
        if (var_playMode_8c1bb8d0 == PLAY_MODE_NORMAL) {
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 6, 0);
        }
        execGroup = TRUE;
        break;

    default:
        break;
    }

    if (execGroup) {
        TaskRunGroup_8c014b42((Task *)var_stopTaskGroup_8c2288f8);
    }
    RenderQueueDraw_8c0223ea(RENDER_LAYER_MIRROR, StopDrawLightEnd_8c02d146, 0);
    RenderQueueDraw_8c0223ea(RENDER_LAYER_CABIN, StopDrawLightEnd_8c02d146, 0);
}

/* Task action spawned instead of the normal per-passenger tasks when
 * var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE and the course-restart flag
 * var_practiceRules_8c226410 bit 3 is clear -- resets the fade-arrival bookkeeping once
 * (guarded by var_isFading_8c226568) and frees itself. */
void PassengerSkipStopTask_8c02d8f0(Task *task, void *state)
{
    if (var_isFading_8c226568 == 0) {
        var_busState_8c1bb9d0.driveState_0x2b4 = 1; /* BusState.driveState_0x2b4: back to driving */
        var_viewLayout_8c22655c = 0;
        var_cameraMode_8c227d9c = 2;
        var_cameraCueState_8c227da4 = 0;
        BusCameraApplyMode_8c024f32();
        TaskKill_8c014b66(task);
    }
}
