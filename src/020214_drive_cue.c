/* @unit DriveCue */
#include <shinobi.h>

#include "includes.h" /* STATIC */
#include "024b4c_bus_camera.h"
#include "015034_text.h"
#include "1ba1c8_globals.h"
#include "01e27c_practice_menu.h" /* var_practiceLesson_8c22640c, var_practiceRules_8c226410 */
#include "014a9c_tasks.h"
#include "013ae8_route.h" /* enum ROUTE */
#include "011120_asset_queues.h" /* AsqGetRandomB_8c0121a8, AsqGetRandomInRangeB_8c0121be */
#include "010e90_vibration.h" /* VibStart_8c010f7a, VibUpdate_8c010fae */
#include "0100bc_sound.h" /* SndPlayAdx_8c010cd6, var_midiHandles_8c0fcd28 */
#include "020214_drive_cue.h"
#include "02c884_stop.h"
#include "02b464_grading.h"

/* ====================
 * Non-initialized Globals
 * ====================
 */

DriveCueState var_driveCueState_8c2264b8;

/* ====================
 * Functions
 * ====================
 */

/* Three independent cues: an ambient chime plus rumble while moving, the
 * stop announcement (chime, then the spoken stop name), and a chime at fixed
 * route segments. Also drives rumble playback. */
STATIC void driveCueTask_8c020214(Task *task, void *state)
{
    int atCueSegment;

    (void)state;

    if (var_runState_8c2285c4.runPhase_0x00 >= 3) {
        TaskUninstall_8c014b66(task);
        return;
    }

    switch (var_driveCueState_8c2264b8.ambientChimeState_0x00) {
    case 0:
        /* Cleared below every frame. */
        if (var_driveCueState_8c2264b8.trafficChimeArmed_0x18 != 0) {
            sdMidiPlay(var_midiHandles_8c0fcd28[3], 1, AsqGetRandomInRangeB_8c0121be(6) + 9, 0);
            var_driveCueState_8c2264b8.ambientChimeTimer_0x04 = (AsqGetRandomInRangeB_8c0121be(3) + 2) * 30;
            var_driveCueState_8c2264b8.ambientChimeState_0x00 = 1;
        } else if (var_busState_8c1bb9d0.speed_0x27c > 0.09259258955717087f) {
            if (--var_driveCueState_8c2264b8.ambientChimeTimer_0x04 < 0) {
                int r = AsqGetRandomB_8c0121a8();
                if ((r & 1) == 0) {
                    sdMidiPlay(var_midiHandles_8c0fcd28[3], 1, 0x3c, 0);
                    VibStart_8c010f7a(2);
                } else {
                    sdMidiPlay(var_midiHandles_8c0fcd28[3], 1, 0x3b, 0);
                    VibStart_8c010f7a(1);
                }
                var_driveCueState_8c2264b8.ambientChimeTimer_0x04 = 60;
                var_driveCueState_8c2264b8.ambientChimeState_0x00 = 1;
            }
        }
        break;

    case 1:
        if (--var_driveCueState_8c2264b8.ambientChimeTimer_0x04 < 0) {
            var_driveCueState_8c2264b8.segmentChimeLatch_0x14 = 0;
            var_driveCueState_8c2264b8.ambientChimeTimer_0x04 = AsqGetRandomInRangeB_8c0121be(300) + 150;
            var_driveCueState_8c2264b8.ambientChimeState_0x00 = 0;
        }
        break;

    default:
        break;
    }

    var_driveCueState_8c2264b8.trafficChimeArmed_0x18 = 0;

    switch (var_driveCueState_8c2264b8.stopAnnounceState_0x08) {
    case 0:
        if (var_driveCueState_8c2264b8.announceRequested_0x0c == 0) {
            break;
        }

        if (var_playMode_8c1bb8d0 != PLAY_MODE_PRACTICE || (var_practiceRules_8c226410 & 1) != 0) {
            int soundId;

            switch (var_route_8c18ad1c) {
            case ROUTE_SHINJUKU:
                soundId = 0x22;
                break;
            case ROUTE_WANGAN:
            case ROUTE_OME:
            default:
                soundId = 0x23;
                break;
            }

            sdMidiPlay(var_midiHandles_8c0fcd28[2], 1, soundId, 0);
            var_driveCueState_8c2264b8.stopAnnounceState_0x08 = 1;
            var_driveCueState_8c2264b8.stopAnnounceTimer_0x10 = 0;
        }
        break;

    case 1: {
        int local0, local1;

        var_driveCueState_8c2264b8.stopAnnounceTimer_0x10++;
        if (var_driveCueState_8c2264b8.stopAnnounceTimer_0x10 <= 60) {
            break;
        }

        local0 = 0;
        if (var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE) {
            if (var_practiceLesson_8c22640c == 8) {
                local0 = 6;
            } else if (var_practiceLesson_8c22640c == 9) {
                local0 = 10;
            }
        }

        switch (var_route_8c18ad1c) {
        case ROUTE_WANGAN:
            local1 = var_prevStopSegment_8c22870c + 0x3f;
            break;
        case ROUTE_SHINJUKU:
        default:
            local1 = var_prevStopSegment_8c22870c + 0x28;
            break;
        case ROUTE_OME:
            local1 = var_prevStopSegment_8c22870c + 0x11;
            break;
        }

        SndPlayAdx_8c010cd6(1, local0 + local1);

        if (var_prevStopSegment_8c22870c == var_nextStopSegment_8c228710) {
            var_driveCueState_8c2264b8.stopAnnounceState_0x08 = 2;
            if (var_runState_8c2285c4.stopPhase_0x20 != 0) {
                var_driveCueState_8c2264b8.stopAnnounceTimer_0x10 = AsqGetRandomInRangeB_8c0121be(60) + 120;
            } else {
                var_driveCueState_8c2264b8.stopAnnounceTimer_0x10 = 30;
            }
        } else {
            var_driveCueState_8c2264b8.stopAnnounceState_0x08 = 3;
        }
        break;
    }

    case 2:
        if (--var_driveCueState_8c2264b8.stopAnnounceTimer_0x10 >= 0) {
            break;
        }

        sdMidiPlay(var_midiHandles_8c0fcd28[4], 1, 0x1a, 0);
        var_driveCueState_8c2264b8.stopAnnounceState_0x08 = 3;
        break;

    case 3:
        /* Re-arms once the next stop is targeted. */
        if (var_driveCueState_8c2264b8.announceRequested_0x0c == 0) {
            var_driveCueState_8c2264b8.stopAnnounceState_0x08 = 0;
        }
        break;

    default:
        break;
    }

    /* Hand-authored segments, unrelated to the run's active stops.
     * Third-person camera only. */
    atCueSegment = 0;
    if (var_cameraMode_8c227d9c >= 2) {
        int prevSeg = var_prevStopSegment_8c22870c;

        switch (var_route_8c18ad1c) {
        case ROUTE_SHINJUKU:
            if (prevSeg == 4 || prevSeg == 5 || prevSeg == 10 ||
                prevSeg == 21 || prevSeg == 22 || prevSeg == 23) {
                atCueSegment = 1;
            }
            break;
        case ROUTE_WANGAN:
            if (prevSeg == 10 || prevSeg == 11 || prevSeg == 16) {
                atCueSegment = 1;
            }
            break;
        case ROUTE_OME:
        default:
            break;
        }
    }

    if (atCueSegment != 0) {
        if (var_driveCueState_8c2264b8.segmentChimeLatch_0x14 == 0) {
            int r = AsqGetRandomB_8c0121a8();
            int soundId = (r & 1) == 0 ? 0x12 : 0x11;

            sdMidiPlay(var_midiHandles_8c0fcd28[3], 1, soundId, 0);
            var_driveCueState_8c2264b8.segmentChimeLatch_0x14 = 1;
        }
    } else {
        var_driveCueState_8c2264b8.segmentChimeLatch_0x14 = 0;
    }

    if (var_vibport_8c1ba354 != (Uint32)-1 && var_progress_8c1ba1cc.vibration_0xc7 == 0) {
        VibUpdate_8c010fae(var_vibport_8c1ba354);
    }
}

void DriveCueInit_8c020528()
{
    Task* installed_task;
    void* installed_state;

    if (var_playMode_8c1bb8d0 != PLAY_MODE_DEMO) {
        TaskInstall_8c014ae8(var_tasks_8c1ba5e8, &driveCueTask_8c020214, &installed_task, &installed_state, 0);
        var_driveCueState_8c2264b8.ambientChimeState_0x00 = 0;
        var_driveCueState_8c2264b8.ambientChimeTimer_0x04 = AsqGetRandomInRangeB_8c0121be(300) + 150;
        var_driveCueState_8c2264b8.stopAnnounceState_0x08 = 3;
        var_driveCueState_8c2264b8.announceRequested_0x0c = 1;
        var_driveCueState_8c2264b8.segmentChimeLatch_0x14 = 0;
        var_driveCueState_8c2264b8.trafficChimeArmed_0x18 = 0;
    }
}
