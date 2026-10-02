/* @unit Grade */

#include <shinobi.h>
#include "includes.h" /* STATIC */
#include "serial_debug.h"
#include "02b464_grading.h"
#include "0100bc_sound.h"
#include "014f54_sprite.h"
#include "015034_text.h"
#include "1ba1c8_globals.h"
#include "01fa78_hud.h" /* var_hudState_8c22643c */
#include "020214_drive_cue.h" /* var_driveCueState_8c2264b8 */
#include "010e90_vibration.h"
#include "02e400_collision.h"
#include "02e2dc_bus_collision.h"
#include "023938_bus_drive.h"
#include "028258_signal.h"
#include "0289ac_objects.h"
#include "02a9fc_message_box.h"
#include "02c884_stop.h"
#include "014a9c_tasks.h"
#include "022464_render.h"
#include "01614c_replay_menu.h"
#include "01e27c_practice_menu.h"
#include "013ae8_route.h"
#include "015ab8_title.h"
#include "024b4c_bus_camera.h"
#include "02b2f0_drive_msg.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

/* Level-crossing zones in a road attribute word. Only the Ome route's
 * attribute map carries them, at its one railway crossing. */
#define ROAD_CROSSING_MASK     0x30000000
#define ROAD_CROSSING_APPROACH 0x10000000 /* full stop required */
#define ROAD_CROSSING_TRACKS   0x20000000 /* no stopping */

/* RunState.crossingState_0x64 */
#define CROSSING_IDLE    1
#define CROSSING_ARMED   2 /* in the approach, not yet stopped */
#define CROSSING_STOPPED 5

/* ====================
 * Initialized Globals
 * ====================
 */

STATIC int init_8c04bef0[] = {
    6, 32, 33, 34, 35, 36, 37,
};

STATIC int init_8c04bf0c[] = {
    9, 32, 33, 34, 35, 36, 37, 26,
    19, 16,
};

STATIC int init_8c04bf34[] = {
    9, 32, 33, 34, 35, 36, 37, 26,
    22, 16,
};

STATIC int init_8c04bf5c[] = {
    7, 34, 35, 36, 37, 26, 19, 16,
};

STATIC int init_8c04bf7c[] = {
    7, 34, 35, 36, 37, 26, 22, 16,
};

STATIC int init_8c04bf9c[] = {
    4, 34, 35, 36, 37,
};

STATIC int init_8c04bfb0[] = {
    8, 43, 50, 48, 52, 47, 26, 17,
    21,
};

STATIC int init_8c04bfd4[] = {
    8, 43, 50, 48, 52, 47, 26, 20,
    16,
};

STATIC int init_8c04bff8[] = {
    5, 43, 50, 48, 52, 47,
};

STATIC int init_8c04c010[] = {
    9, 48, 49, 50, 51, 52, 53, 26,
    17, 16,
};

STATIC int init_8c04c038[] = {
    9, 48, 49, 50, 51, 52, 53, 26,
    19, 16,
};

STATIC int init_8c04c060[] = {
    9, 44, 45, 32, 57, 58, 59, 26,
    17, 16,
};

STATIC int init_8c04c088[] = {
    9, 44, 45, 32, 57, 58, 59, 26,
    21, 16,
};

STATIC int init_8c04c0b0[] = {
    9, 58, 59, 32, 57, 52, 53, 26,
    17, 16,
};

STATIC int init_8c04c0d8[] = {
    9, 58, 59, 32, 57, 52, 53, 26,
    18, 16,
};

STATIC int init_8c04c100[] = {
    7, 54, 55, 56, 46, 39, 26, 24,
};

STATIC int init_8c04c120[] = {
    7, 60, 61, 62, 63, 26, 23, 16,
};

STATIC int init_8c04c140[] = {
    8, 64, 65, 66, 67, 52, 53, 26,
    21,
};

STATIC int init_8c04c164[] = {
    9, 32, 57, 68, 69, 52, 53, 26,
    17, 16,
};

STATIC int init_8c04c18c[] = {
    7, 70, 71, 41, 42, 26, 24, 16,
};

STATIC int init_8c04c1ac[] = {
    9, 72, 73, 74, 75, 64, 65, 26,
    19, 16,
};

STATIC int init_8c04c1d4[] = {
    8, 72, 73, 76, 64, 32, 26, 21,
    16,
};

STATIC int init_8c04c1f8[] = {
    5, 77, 58, 26, 21, 16,
};

STATIC int init_8c04c210[] = {
    5, 78, 79, 80, 26, 21,
};

STATIC int init_8c04c228[] = {
    7, 78, 81, 82, 50, 83, 26, 21,
};

STATIC int init_8c04c248[] = {
    7, 78, 84, 85, 51, 86, 26, 21,
};

STATIC int init_8c04c268[] = {
    8, 87, 48, 64, 71, 88, 26, 18,
    16,
};

STATIC int init_8c04c28c[] = {
    6, 89, 90, 91, 48, 26, 19,
};

STATIC int init_8c04c2a8[] = {
    7, 89, 90, 91, 48, 26, 17, 16,
};

STATIC int init_8c04c2c8[] = {
    4, 92, 93, 94, 95,
};

STATIC int init_8c04c2dc[] = {
    4, 10, 11, 12, 13,
};

STATIC int init_8c04c2f0[] = {
    5, 38, 39, 40, 41, 42,
};

STATIC int init_8c04c308[] = {
    9, 28, 29, 30, 85, 48, 91, 48,
    26, 21,
};

STATIC int init_8c04c330[] = {
    10, 51, 28, 14, 31, 15, 105, 106,
    26, 17, 21,
};

/* Indexed by msgSet (0-0x21): {count, glyph id...} banner rows. Glyph ids
 * are not INSTR_* ids; init_penaltyMsgSetInstr_8c045208 maps a msgSet to
 * its INSTR_* line. */
STATIC int *init_penaltyMsgGlyphs_8c04c35c[] = {
    /* 0x00 */ init_8c04bef0, init_8c04bf0c, init_8c04bf34, init_8c04bf5c,
    /* 0x04 */ init_8c04bf7c, init_8c04bf9c, init_8c04bfb0, init_8c04bfd4,
    /* 0x08 */ init_8c04bff8, init_8c04c010, init_8c04c038, init_8c04c060,
    /* 0x0c */ init_8c04c088, init_8c04c0b0, init_8c04c0d8, init_8c04c100,
    /* 0x10 */ init_8c04c120, init_8c04c140, init_8c04c164, init_8c04c18c,
    /* 0x14 */ init_8c04c1ac, init_8c04c1d4, init_8c04c1f8, init_8c04c210,
    /* 0x18 */ init_8c04c228, init_8c04c248, init_8c04c268, init_8c04c28c,
    /* 0x1c */ init_8c04c2a8, init_8c04c2c8, init_8c04c2dc, init_8c04c2f0,
    /* 0x20 */ init_8c04c308, init_8c04c330,
};

/* =======================
 * Non-initialized Globals
 * =======================
 */

RunState var_runState_8c2285c4;

int var_wallHitBits_8c228660;

/* Written and never read. */
STATIC BusState *var_hitCandidate_8c228664;
STATIC BusState *var_hitVehicle_8c228668;

/* Snapshotted before the graders run, so they all see one value. */
STATIC float var_frameSpeed_8c22866c;

/* The bumped vehicle's speed before the bump, given to the player as
 * rebound speed. */
STATIC float var_bumpSpeed_8c228670;

/* One frame's driving state, snapshotted before the graders run.
 * laneA/B/C are the three road probes' lanes. offCourseBits is the larger
 * of probes A and B's 0x30000 bits, 0x30000 being severe. headingVsRoad:
 * 0 = no road data, 1 = along the road, 2 = against it. */
STATIC int var_laneA_8c228674;
STATIC int var_laneB_8c228678;
STATIC int var_laneC_8c22867c;
STATIC int var_offCourseBits_8c228680;
STATIC int var_prevLane_8c228684;
STATIC int var_prevLaneFlags_8c228688;
STATIC int var_headingVsRoad_8c22868c;

/* One per offense category, in frames; negative = expired. */
STATIC int var_cooldownCollision_8c228690;
STATIC int var_cooldownOffCourse_8c228694;
STATIC int var_cooldownSignal_8c228698;
STATIC int var_cooldownLane_8c22869c;
STATIC int var_cooldownIntersection_8c2286a0;


/* ====================
 * Functions
 * ====================
 */

STATIC void adjustPoints_8c02b464(int msgSet, int delta) {
    if (var_playMode_8c1bb8d0 == PLAY_MODE_DEMO) {
        return;
    }

    if (delta < 0) {
        var_penaltyCount_8c1bb8f4 = var_penaltyCount_8c1bb8f4 + 1;
    } else if (var_progress_8c1ba1cc.difficulty_0xc4 == 2 && var_playMode_8c1bb8d0 != PLAY_MODE_PRACTICE) {
        return;
    }

    var_runState_8c2285c4.driverPoints_0x0c = var_runState_8c2285c4.driverPoints_0x0c + delta;

    if (delta < var_worstPenaltyDelta_8c1bb8f0) {
        var_worstPenaltyDelta_8c1bb8f0 = delta;
        var_worstPenaltyMsgSet_8c1bb8ec = msgSet;
    }

    if (var_runState_8c2285c4.driverPoints_0x0c < 0) {
        var_runState_8c2285c4.driverPoints_0x0c = 0;
    } else if (var_runState_8c2285c4.driverPointsMax_0x10 < var_runState_8c2285c4.driverPoints_0x0c) {
        var_runState_8c2285c4.driverPoints_0x0c = var_runState_8c2285c4.driverPointsMax_0x10;
    }

    if (msgSet != -1) {
        int soundId = (delta < 1) ? 5 : 4;
        sdMidiPlay(var_midiHandles_8c0fcd28[5], 1, soundId, 0);

        var_driveMsgQueue_8c228564[3] = var_driveMsgQueue_8c228564[2];
        var_driveMsgQueue_8c228564[2] = var_driveMsgQueue_8c228564[1];
        var_driveMsgQueue_8c228564[1] = var_driveMsgQueue_8c228564[0];

        {
            int count = init_penaltyMsgGlyphs_8c04c35c[msgSet][0];
            int t = count * -0x20 + 0x280;

            var_driveMsgQueue_8c228564[0].count = count;
            var_driveMsgQueue_8c228564[0].ids = &init_penaltyMsgGlyphs_8c04c35c[msgSet][1];
            var_driveMsgQueue_8c228564[0].x = (float)((t + (t < 0)) >> 1);
            var_driveMsgQueue_8c228564[0].revealed = 0;
            var_driveMsgQueue_8c228564[0].revealCounter = 0;
            var_driveMsgQueue_8c228564[0].holdFrames = 0x3c;
        }
    }
}

STATIC void armCooldowns_8c02b578(int type) {
    var_cooldownCollision_8c228690 = 1;
    var_cooldownOffCourse_8c228694 = 1;
    var_cooldownSignal_8c228698 = 1;
    var_cooldownLane_8c22869c = 1;
    var_cooldownIntersection_8c2286a0 = 1;

    switch (type) {
    case 1:
        var_cooldownCollision_8c228690 = 0x96;
        break;
    case 2:
        var_cooldownOffCourse_8c228694 = 0xd2;
        break;
    case 3:
        var_cooldownSignal_8c228698 = 0x96;
        break;
    case 4:
        var_cooldownLane_8c22869c = 0xd2;
        return;
    case 5:
        var_cooldownIntersection_8c2286a0 = 0xd2;
        return;
    default:
        return;
    }

    var_runState_8c2285c4.field_0x38[0] = 0;
}

STATIC void handleBump_8c02b6d4(void) {
    TrafficEntry *other;
    float dx, dz, dist;

    if (CollisionQueueTest_8c02e4ac() != NULL) {
        adjustPoints_8c02b464(0x1f, -200); /* -> INSTR_NEAR_MISS_PEDESTRIAN */
        var_cooldownCollision_8c228690 = 0x7fff;
        return;
    }

    if (var_playerBus_8c1bbd9c->driveState_0x2b4 != 1) {
        return;
    }

    other = BusCollisionFindHit_8c02e2dc();
    var_hitCandidate_8c228664 = (BusState *)other;
    if (other == NULL) {
        return;
    }

    sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0x13, 0);

    var_hitVehicle_8c228668 = (BusState *)other;
    var_bumpSpeed_8c228670 = other->speed_0x27c;
    dx = other->posX_0xf4 - var_playerBus_8c1bbd9c->posX_0x0f4;
    dz = other->posZ_0xfc - var_playerBus_8c1bbd9c->posZ_0x0fc;
    dist = njSqrt(dx * dx + dz * dz);

    if (other->isParked_0x2e4 == 0) {
        other->driveState_0x2b4 = 1;
        other->speed_0x27c = var_frameSpeed_8c22866c + 0.3f;
        other->dirX_0x29c = dx / dist;
        other->dirZ_0x2a0 = dz / dist;
        other->groundProbe_0x190[0].count_0x0c = 0;
        other->groundProbe_0x190[1].count_0x0c = 0;
        other->groundProbe_0x190[2].count_0x0c = 0;
    } else {
        other->driveState_0x2b4 = 1;
        other->speed_0x27c = var_frameSpeed_8c22866c + 0.3f;
        other->dirX_0x29c = dx / dist;
        other->dirZ_0x2a0 = dz / dist;
    }

    BusDriveEnterKnockback_8c023bce();

    var_playerBus_8c1bbd9c->speed_0x27c = var_bumpSpeed_8c228670 + 0.3f;
    var_playerBus_8c1bbd9c->dir_x_0x29c = -other->dirX_0x29c;
    var_playerBus_8c1bbd9c->dir_z_0x2a0 = -other->dirZ_0x2a0;
    var_playerBus_8c1bbd9c->dir_x2_0x2ac = -other->dirX_0x29c;
    var_playerBus_8c1bbd9c->dir_z2_0x2b0 = -other->dirZ_0x2a0;

    if (var_frameSpeed_8c22866c < 0.1388889f) {
        VibStart_8c010f7a(4);
    } else if (var_frameSpeed_8c22866c < 0.2777778f) {
        VibStart_8c010f7a(5);
    } else {
        VibStart_8c010f7a(6);
    }

    if (var_cooldownCollision_8c228690 >= 0) {
        return;
    }

    if (var_frameSpeed_8c22866c < 0.1388889f) {
        adjustPoints_8c02b464(1, -30); /* -> INSTR_COLLISION_CAR_MEDIUM */
    } else if (var_frameSpeed_8c22866c < 0.2777778f) {
        adjustPoints_8c02b464(2, -60); /* -> INSTR_COLLISION_CAR_FATAL */
    } else {
        adjustPoints_8c02b464(0, -200); /* -> INSTR_COLLISION_CAR_MINOR */
    }

    armCooldowns_8c02b578(1);
}

STATIC void gradeWallHit_8c02b7ea(void) {
    unsigned int flags = var_wallHitBits_8c228660 & 6;

    if (flags == 0) {
        return;
    }

    if (flags == 2) {
        adjustPoints_8c02b464(3, -30); /* -> INSTR_COLLISION_WALL_MINOR */
    } else if (flags == 4) {
        adjustPoints_8c02b464(4, -60); /* -> INSTR_COLLISION_WALL_MEDIUM */
    } else if (flags == 6) {
        adjustPoints_8c02b464(5, -200); /* -> INSTR_COLLISION_WALL_SEVERE */
    }

    armCooldowns_8c02b578(1);
}

STATIC void gradeOffCourseSevere_8c02b864(void) {
    if (var_offCourseBits_8c228680 == 0x30000) {
        adjustPoints_8c02b464(8, -200); /* -> INSTR_OFF_COURSE_MEDIUM */
        armCooldowns_8c02b578(2);
        var_cooldownCollision_8c228690 = 0x7fff;
    }
}

STATIC void gradeOffCourse_8c02b886(void) {
    if (var_offCourseBits_8c228680 != 0 && var_headingVsRoad_8c22868c != 2) {
        if (var_offCourseBits_8c228680 == 0x20000) {
            adjustPoints_8c02b464(7, -40); /* -> INSTR_OFF_COURSE_MEDIUM */
        } else {
            adjustPoints_8c02b464(6, -15); /* -> INSTR_OFF_COURSE_MINOR */
        }
        armCooldowns_8c02b578(2);
    }
}

/* Wrong way, running a red, and stopping past the line. The signal id is
 * junctionARoadFlags_0x34c's low 12 bits. */
STATIC void gradeSignals_8c02b8b8(void) {
    int signalId;

    if (var_driveMode_8c1bb8c8 == 0 && var_offCourseBits_8c228680 == 0 && var_headingVsRoad_8c22868c == 2) {
        adjustPoints_8c02b464(0x16, -50); /* -> INSTR_WRONG_WAY */
        armCooldowns_8c02b578(3);
    }

    if ((var_busState_8c1bb9d0.junctionARoadFlags_0x34c & 0xfff) == 0) {
        if (var_runState_8c2285c4.field_0x38[0] != 0 && (var_busState_8c1bb9d0.junctionARoadFlags2_0x358 & 0xf000000) == 0) {
            if (SignalGetFrame_8c028900(var_runState_8c2285c4.field_0x38[0] & 0xffff) == 0) {
                adjustPoints_8c02b464(0x10, -70); /* -> INSTR_SIGNAL_VIOLATION */
            }
            armCooldowns_8c02b578(3);
        }
        var_runState_8c2285c4.stopLineGraded_0x34 = 0;
        var_runState_8c2285c4.field_0x38[0] = 0;
    } else {
        signalId = var_busState_8c1bb9d0.junctionARoadFlags_0x34c & 0xfff;
        var_runState_8c2285c4.field_0x38[0] = signalId;
        var_runState_8c2285c4.field_0x38[1] = signalId;
        if (var_runState_8c2285c4.stopLineGraded_0x34 == 0 && var_frameSpeed_8c22866c == 0.0f
            && SignalGetFrame_8c028900(var_runState_8c2285c4.field_0x38[0]) == 0) {
            adjustPoints_8c02b464(0x11, -5); /* -> INSTR_BAD_STOP_LINE */
            var_runState_8c2285c4.stopLineGraded_0x34 = 1;
        }
    }
}

/* Wrong lane, an illegal lane change, and straddling lanes. 0x40000 in the
 * road flags marks a lane change as illegal; the straddle threshold depends
 * on the road's turn-direction bits. */
STATIC void gradeLaneUse_8c02b986(void) {
    int cmpDir;
    int threshold;

    if (var_headingVsRoad_8c22868c == 0 || var_offCourseBits_8c228680 == 0) {
        var_runState_8c2285c4.wrongLaneCount_0x2c = 0;
    } else {
        if (var_runState_8c2285c4.wrongLaneCount_0x2c == 0) {
            adjustPoints_8c02b464(0xb, -10); /* -> INSTR_WRONG_LANE */
        } else {
            adjustPoints_8c02b464(0xc, -50); /* -> INSTR_WRONG_LANE */
        }
        var_runState_8c2285c4.wrongLaneCount_0x2c = var_runState_8c2285c4.wrongLaneCount_0x2c + 1;
        armCooldowns_8c02b578(4);
    }

    if (var_runState_8c2285c4.lane_0x6c == var_prevLane_8c228684 || (var_busState_8c1bb9d0.junctionARoadFlags_0x34c & 0x40000) == 0
        || (var_busState_8c1bb9d0.junctionBRoadFlags_0x368 & 0x40000) == 0 || (var_runState_8c2285c4.field_0x70[1] & 0x40000) == 0
        || (var_runState_8c2285c4.field_0x70[2] & 0x40000) == 0) {
        if (var_driveMode_8c1bb8c8 == 0) {
            if (var_laneA_8c228674 != var_laneC_8c22867c) {
                cmpDir = (var_laneA_8c228674 >= var_laneC_8c22867c) ? 1 : 2;
            } else if (var_laneB_8c228678 != var_laneC_8c22867c) {
                cmpDir = (var_laneB_8c228678 >= var_laneC_8c22867c) ? 1 : 2;
            } else {
                cmpDir = 0;
            }

            if (cmpDir == 0) {
                var_runState_8c2285c4.field_0x38[3] = 0;
                var_runState_8c2285c4.field_0x38[4] = 0;
            } else {
                if (var_runState_8c2285c4.field_0x38[4] == 0) {
                    if ((cmpDir == 2 && (var_busState_8c1bb9d0.junctionARoadFlags_0x34c & 0xc0000000) == 0x40000000)
                        || (cmpDir == 1 && (var_busState_8c1bb9d0.junctionARoadFlags_0x34c & 0xc0000000) == 0x80000000)) {
                        threshold = 0xd2;
                    } else {
                        threshold = 0x3c;
                    }
                } else {
                    threshold = 0x78;
                }
                if (threshold < var_runState_8c2285c4.field_0x38[3]) {
                    var_runState_8c2285c4.field_0x38[3] = 0;
                    if (var_runState_8c2285c4.field_0x38[4] == 0) {
                        adjustPoints_8c02b464(0xd, -10); /* -> INSTR_LANE_STRADDLE */
                    } else {
                        adjustPoints_8c02b464(0xe, -20); /* -> INSTR_LANE_STRADDLE */
                    }
                    var_runState_8c2285c4.field_0x38[4] = var_runState_8c2285c4.field_0x38[4] + 1;
                }
            }
        }
    } else {
        adjustPoints_8c02b464(0x12, -10); /* -> INSTR_ILLEGAL_LANE_CHANGE */
        armCooldowns_8c02b578(4);
    }

    if (var_frameSpeed_8c22866c != 0.0f) {
        var_runState_8c2285c4.field_0x38[3] = var_runState_8c2285c4.field_0x38[3] + 1;
    }
}

/* Blocking an intersection, speeding, and changing lanes or turning at a
 * junction without the matching signal. The speed limit is
 * junctionARoadFlags_0x34c's 0xf00000 nibble, in tens of km/h. */
STATIC void gradeIntersection_8c02bb1c(void) {
    int laneDelta;
    float speedLimit;

    if ((var_busState_8c1bb9d0.junctionARoadFlags2_0x358 & 0xf000000) == 0
        && (var_busState_8c1bb9d0.markCueByte_0x3b4 & 0xff000000) != 0
        && SignalGetFrame_8c028900(var_runState_8c2285c4.field_0x38[1] & 0xffff) == 0
        && var_frameSpeed_8c22866c == 0.0f) {
        adjustPoints_8c02b464(0x13, -80); /* -> INSTR_BLOCK_INTERSECTION */
        armCooldowns_8c02b578(5);
    }

    speedLimit = (float)(((var_busState_8c1bb9d0.junctionARoadFlags_0x34c & 0xf00000) >> 0x14) * 10) * 1000.0f
                 / 108000.0f;
    if (var_frameSpeed_8c22866c <= speedLimit + 0.055555556f) {
        var_runState_8c2285c4.speedingCountdown_0x30 = 0;
    } else {
        var_runState_8c2285c4.speedingCountdown_0x30 = var_runState_8c2285c4.speedingCountdown_0x30 - 1;
        if (var_runState_8c2285c4.speedingCountdown_0x30 < 0) {
            var_runState_8c2285c4.speedingCountdown_0x30 = 0x78;
            if (speedLimit + 0.18518518f <= var_frameSpeed_8c22866c) {
                adjustPoints_8c02b464(10, -30); /* -> INSTR_SPEEDING_MAJOR */
            } else {
                adjustPoints_8c02b464(9, -10); /* -> INSTR_SPEEDING_MINOR */
            }
        }
    }

    if (var_driveMode_8c1bb8c8 == 0) {
        laneDelta = (var_runState_8c2285c4.lane_0x6c & ~1) - (var_prevLane_8c228684 & ~1);
        if ((laneDelta < 0 && var_playerBus_8c1bbd9c->signalSide_0x25c != 1)
            || (laneDelta >= 1 && var_playerBus_8c1bbd9c->signalSide_0x25c != 2)) {
            if ((var_prevLaneFlags_8c228688 & 0xf000000) != 0 && (var_runState_8c2285c4.field_0x70[0] & 0xf000000) != 0) {
                adjustPoints_8c02b464(0xf, -8); /* -> INSTR_NO_SIGNAL */
            }
        }
    }

    if (var_runState_8c2285c4.field_0x38[6] == 0) {
        if ((var_busState_8c1bb9d0.junctionARoadFlags2_0x358 & 0xf000000) != 0) {
            var_runState_8c2285c4.field_0x38[6] = 1;
        }
        return;
    }
    if (var_runState_8c2285c4.field_0x38[6] != 1) {
        return;
    }
    if ((var_busState_8c1bb9d0.junctionARoadFlags2_0x358 & 0xf000000) != 0) {
        return;
    }

    if ((var_busState_8c1bb9d0.junctionARoadFlags_0x34c & 0xc0000000) == 0x40000000) {
        if (var_busState_8c1bb9d0.signalSide_0x25c == 1) {
            var_runState_8c2285c4.field_0x38[6] = 0;
            return;
        }
    } else {
        if ((var_busState_8c1bb9d0.junctionARoadFlags_0x34c & 0xc0000000) != 0x80000000) {
            var_runState_8c2285c4.field_0x38[6] = 0;
            return;
        }
        if (var_busState_8c1bb9d0.signalSide_0x25c == 2) {
            var_runState_8c2285c4.field_0x38[6] = 0;
            return;
        }
    }

    adjustPoints_8c02b464(0xf, -8); /* -> INSTR_NO_SIGNAL */
    var_runState_8c2285c4.field_0x38[6] = 0;
}

/* The checks outside the cooldown chain. Also cancels the turn signal once
 * a lane change or turn lane is done, and advances the run clock. */
STATIC void gradeFrame_8c02bcd8(void) {
    int laneDelta;
    unsigned int crossingZone;
    int crossingState;
    int skipHoldTimer = 0;

    laneDelta = (var_runState_8c2285c4.lane_0x6c & ~1) - (var_prevLane_8c228684 & ~1);
    if ((laneDelta < 0 && var_playerBus_8c1bbd9c->signalSide_0x25c == 1)
        || (laneDelta > 0 && var_playerBus_8c1bbd9c->signalSide_0x25c == 2)) {
        var_playerBus_8c1bbd9c->signalSide_0x25c = 0;
        var_playerBus_8c1bbd9c->mirror_0x268 = 0;
    }

    if (var_runState_8c2285c4.field_0x38[5] == 0) {
        if ((var_busState_8c1bb9d0.junctionARoadFlags_0x34c & 0xc0000000) != 0) {
            var_runState_8c2285c4.field_0x38[5] = 1;
        }
    } else if ((var_busState_8c1bb9d0.junctionARoadFlags_0x34c & 0xc0000000) == 0) {
        var_busState_8c1bb9d0.signalSide_0x25c = 0;
        var_busState_8c1bb9d0.mirror_0x268 = 0;
        var_runState_8c2285c4.field_0x38[5] = 0;
    }

    /* junctionA is sampled beside the bus's position (its front), junctionC
     * 4.9m behind it. */
    crossingZone = var_busState_8c1bb9d0.junctionARoadFlags_0x34c & ROAD_CROSSING_MASK;
    crossingState = var_runState_8c2285c4.crossingState_0x64;
    if (crossingState == CROSSING_IDLE && crossingZone == ROAD_CROSSING_APPROACH) {
        var_runState_8c2285c4.crossingState_0x64 = CROSSING_ARMED;
        crossingState = CROSSING_ARMED;
    }

    if (crossingState == CROSSING_ARMED) {
        if (crossingZone != ROAD_CROSSING_APPROACH) {
            adjustPoints_8c02b464(0x14, -30); /* -> INSTR_BLANK; banner: no stop before crossing */
            var_runState_8c2285c4.crossingState_0x64 = CROSSING_IDLE;
        } else if (var_frameSpeed_8c22866c == 0.0f) {
            var_runState_8c2285c4.crossingState_0x64 = CROSSING_STOPPED;
        } else {
            var_runState_8c2285c4.crossingStopTimer_0x68 = 0;
            skipHoldTimer = 1;
        }
    } else if (crossingState == CROSSING_STOPPED && crossingZone != ROAD_CROSSING_APPROACH) {
        var_runState_8c2285c4.crossingState_0x64 = CROSSING_IDLE;
    }

    /* Starts at 0, so the first stationary frame on the tracks is already
     * penalised; 0x78 frames between repeats. */
    if (!skipHoldTimer) {
        if (var_frameSpeed_8c22866c == 0.0f
            && ((var_busState_8c1bb9d0.junctionARoadFlags_0x34c & ROAD_CROSSING_MASK) == ROAD_CROSSING_TRACKS
                || (var_busState_8c1bb9d0.junctionCRoadFlags_0x384 & ROAD_CROSSING_MASK) == ROAD_CROSSING_TRACKS)) {
            var_runState_8c2285c4.crossingStopTimer_0x68 = var_runState_8c2285c4.crossingStopTimer_0x68 - 1;
            if (var_runState_8c2285c4.crossingStopTimer_0x68 < 0) {
                var_runState_8c2285c4.crossingStopTimer_0x68 = 0x78;
                adjustPoints_8c02b464(0x15, -50); /* -> INSTR_BLANK; banner: stopped in crossing */
            }
        } else {
            var_runState_8c2285c4.crossingStopTimer_0x68 = 0;
        }
    }

    if (var_runState_8c2285c4.fullThrottleLatch_0x84 == 0) {
        if (var_runState_8c2285c4.firstUpshift_0x88 != 0) {
            if (var_peripherals_8c1ba35c[0].r > 0xfe) {
                var_runState_8c2285c4.fullThrottleLatch_0x84 = 1;
                var_runState_8c2285c4.fullThrottleFrames_0x8c = 0;
            }
            var_runState_8c2285c4.firstUpshift_0x88 = 0;
        }
    } else if (var_runState_8c2285c4.fullThrottleLatch_0x84 == 1) {
        if (var_peripherals_8c1ba35c[0].r < 0xff) {
            var_runState_8c2285c4.fullThrottleLatch_0x84 = 0;
        } else {
            var_runState_8c2285c4.fullThrottleFrames_0x8c = var_runState_8c2285c4.fullThrottleFrames_0x8c + 1;
            if (var_runState_8c2285c4.fullThrottleFrames_0x8c > 0xe) {
                var_runState_8c2285c4.fullThrottleLatch_0x84 = 0;
                adjustPoints_8c02b464(0x17, -5); /* -> INSTR_RAPID_ACCEL */
            }
        }
    }

    if (var_runState_8c2285c4.hardBrakeCooldown_0x94 != 0) {
        var_runState_8c2285c4.hardBrakeCooldown_0x94 = var_runState_8c2285c4.hardBrakeCooldown_0x94 - 1;
    }
    if (var_runState_8c2285c4.brakeAverage_0x90 > 0.01f && var_runState_8c2285c4.hardBrakeCooldown_0x94 == 0) {
        adjustPoints_8c02b464(0x18, -5); /* -> INSTR_HARD_BRAKE */
        var_runState_8c2285c4.brakeAverage_0x90 = 0.0f;
        var_runState_8c2285c4.hardBrakeCooldown_0x94 = 0x3c;
        VibStart_8c010f7a(3);
    }

    if (var_driveMode_8c1bb8c8 == 0 && var_busState_8c1bb9d0.driveState_0x2b4 == 1) {
        float t = var_busState_8c1bb9d0.speed_0x27c * (float)var_busState_8c1bb9d0.ang_0x258;
        if (t < -2000.0f || t > 2000.0f) {
            var_runState_8c2285c4.swerveCountdown_0x98 = var_runState_8c2285c4.swerveCountdown_0x98 - 1;
            if (var_runState_8c2285c4.swerveCountdown_0x98 < 0) {
                adjustPoints_8c02b464(0x19, -5); /* -> INSTR_SWERVING */
                var_runState_8c2285c4.swerveCountdown_0x98 = 0x3c;
            }
        } else {
            var_runState_8c2285c4.swerveCountdown_0x98 = 0;
        }
    }

    if (var_runState_8c2285c4.field_0x38[7] == var_busState_8c1bb9d0.signalSide_0x25c) {
        var_runState_8c2285c4.signalHeldFrames_0x58 = var_runState_8c2285c4.signalHeldFrames_0x58 + 1;
    } else {
        var_runState_8c2285c4.field_0x38[7] = var_busState_8c1bb9d0.signalSide_0x25c;
        var_runState_8c2285c4.signalHeldFrames_0x58 = 0;
    }

    if (var_runState_8c2285c4.hasMoved_0x5c == 0) {
        if (var_frameSpeed_8c22866c == 0.0f) {
            var_runState_8c2285c4.startIdleFrames_0x60 = var_runState_8c2285c4.startIdleFrames_0x60 + 1;
            if (var_runState_8c2285c4.startIdleFrames_0x60 > 0x708) {
                adjustPoints_8c02b464(0x13, -80); /* -> INSTR_BLOCK_INTERSECTION */
                var_runState_8c2285c4.startIdleFrames_0x60 = 0;
            }
        } else {
            var_runState_8c2285c4.hasMoved_0x5c = 1;
        }
    }

    StopUpdateArrival_8c02ce48();

    if ((var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE && (var_practiceRules_8c226410 & 1) != 1)
        || var_runState_8c2285c4.stopPhase_0x20 == 0
        || var_driveCueState_8c2264b8.announceRequested_0x0c != 0) {
        if (var_runState_8c2285c4.instructionBonusPending_0x7c != 0) {
            adjustPoints_8c02b464(0x1e, 20); /* -> INSTR_BLANK */
            var_runState_8c2285c4.instructionBonusPending_0x7c = 0;
        }
    } else {
        adjustPoints_8c02b464(0x20, -5); /* -> INSTR_ANNOUNCEMENT */
        var_driveCueState_8c2264b8.announceRequested_0x0c = 1;
    }

    if (var_busState_8c1bb9d0.doorState_0x3c0 == 2 && var_busState_8c1bb9d0.speed_0x27c != 0.0f) {
        adjustPoints_8c02b464(0x21, -15); /* -> INSTR_DOOR_OPERATION */
        var_busState_8c1bb9d0.doorRequest_0x3c4 = 1;
    }

    if (var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE && (var_practiceRules_8c226410 & 2) != 2) {
        var_runState_8c2285c4.runClock_0x18--;
        if (var_runState_8c2285c4.runClock_0x18 < 0) {
            var_runState_8c2285c4.runClock_0x18 = 0;
            adjustPoints_8c02b464(0x1d, -200); /* -> INSTR_TIME_MANAGEMENT */
            return;
        }
    } else {
        var_runState_8c2285c4.runClock_0x18 = var_runState_8c2285c4.runClock_0x18 + 1;
        if (var_hudState_8c22643c.driveMarkIcon_0x14 != -1
            && var_runState_8c2285c4.scheduleTime_0x14 < var_runState_8c2285c4.runClock_0x18
            && var_runState_8c2285c4.runClock_0x18 % 30 == 0) {
            adjustPoints_8c02b464(-1, -1);
            return;
        }
    }
}

int GradeRunComplete_8c02c586(void) {
    int threshold = 0;

    if (var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE) {
        if ((var_practiceRules_8c226410 & 4) != 4) {
            return 1;
        }
        if (var_practiceLesson_8c22640c == 7) {
            threshold = 2;
        } else if (var_practiceLesson_8c22640c == 8) {
            threshold = 1;
        } else if (var_practiceLesson_8c22640c == 9 || var_practiceLesson_8c22640c == 10) {
            threshold = 4;
        }
    } else if (var_route_8c18ad1c == 1) {
        threshold = 0xf;
    } else if (var_route_8c18ad1c == 0 || var_route_8c18ad1c == 2) {
        threshold = 0x16;
    }

    return threshold <= var_nextStopSegment_8c228710;
}

STATIC void driveEndFadeTask_8c02c69a(Task *task, void *state) {
    int *phase = (int *)&task->field_0x08;
    int *counter = (int *)&task->field_0x0c;
    float priority = -3.0f;

    if (*phase == 0) {
        if (var_isFading_8c226568 == 0) {
            *counter = 0;
            *phase = 1;
        }
    } else if (*phase == 1) {
        *counter = *counter + 1;
        if (*counter > 0x1e) {
            *phase = 2;
            RenderStartFadeOut_8c022b60(10);
        }
    } else if (*phase == 2 && var_isFading_8c226568 == 0) {
        ReplayMenuFreeSessionAssets_8c016182();
        if (var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE) {
            PracticeMenuReturn_8c01f21c();
            return;
        }
        if (var_gameMode_8c1bb8fc == 0) {
            ResultsShowPassedRun_8c01e0b4();
            return;
        }
        var_shouldShowFreeRunIntro_8c1bb8c0 = 0;
        var_runWasPractice_8c1bb8bc = 0;
        CourseMenuReturn_8c017ef2();
        return;
    }

    SpriteDraw_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418, 0x79, 0.0f, 0.0f, priority);
}

STATIC void enterDriveEnd_8c02c738(void) {
    Task *installed_task;
    void *installed_state;

    ReplayMenuUninstallDrive_8c01614c();
    TaskInstall_8c014ae8(var_tasks_8c1ba3c8, (void *)driveEndFadeTask_8c02c69a, &installed_task, &installed_state, 0);
    installed_task->field_0x08 = 0;
    RenderStartFadeIn_8c022a9c(10);
}

/* Fade-complete callback for a non-practice run that ran out of points. */
STATIC void onFadeRunFailed_8c02c76a(void) {
    var_progress_8c1ba1cc.days_0x00 = var_progress_8c1ba1cc.days_0x00 + 1;
    var_runSucceeded_8c1bb8dc = 0;
    var_runReportPending_8c1bb8b8 = 1;
    var_runWasPractice_8c1bb8bc = 0;
    enterDriveEnd_8c02c738();
}

/* Fade-complete callback for a non-practice stop reached with points left:
 * loads the next segment while a stop is owed, otherwise the run passed. */
STATIC void onFadeStopEnded_8c02c624(void) {
    if (GradeRunComplete_8c02c586() == 0) {
        ObjectsUninstallPedestrianGroups_8c0297da();
        SignalFree_8c0288be();
        TaskUninstallGroup_8c014ab4(var_tasks_8c1bb448);
        TaskUninstallGroup_8c014ab4(var_tasks_8c1bac28);
        TaskUninstallGroup_8c014ab4(var_tasks_8c1ba5e8);
        TaskUninstallGroup_8c014ab4(var_tasks_8c1ba3c8);
        StopAdvance_8c02ccc6();
        RouteInstallSegmentReload_8c01468e();
        return;
    }

    var_runReportPending_8c1bb8b8 = 1;
    var_runWasPractice_8c1bb8bc = 0;
    ReplayMenuFreeSessionAssets_8c016182();

    if (var_gameMode_8c1bb8fc == 0) {
        var_progress_8c1ba1cc.days_0x00 = var_progress_8c1ba1cc.days_0x00 + 1;
        var_runSucceeded_8c1bb8dc = 1;
        ResultsShowPassedRun_8c01e0b4();
        return;
    }

    CourseMenuReturn_8c017ef2();
}

void GradeOnFadePracticeEnd_8c02c784(void) {
    if (GradeRunComplete_8c02c586() == 0 && var_runState_8c2285c4.driverPoints_0x0c > 0) {
        ObjectsUninstallPedestrianGroups_8c0297da();
        SignalFree_8c0288be();
        TaskUninstallGroup_8c014ab4(var_tasks_8c1bb448);
        TaskUninstallGroup_8c014ab4(var_tasks_8c1bac28);
        TaskUninstallGroup_8c014ab4(var_tasks_8c1ba5e8);
        TaskUninstallGroup_8c014ab4(var_tasks_8c1ba3c8);
        StopAdvance_8c02ccc6();
        RouteInstallSegmentReload_8c01468e();
        return;
    }

    var_menuState_8c1bc7a8.selected_0x38 = var_practiceLesson_8c22640c;
    var_runReportPending_8c1bb8b8 = 1;
    var_runWasPractice_8c1bb8bc = 1;

    if (var_runState_8c2285c4.driverPoints_0x0c < 1) {
        var_runSucceeded_8c1bb8dc = 0;
        enterDriveEnd_8c02c738();
        return;
    }

    var_runSucceeded_8c1bb8dc = 1;
    ReplayMenuFreeSessionAssets_8c016182();
    PracticeMenuReturn_8c01f21c();
}

/* Grades each frame of run phase 2 and the stop arrival in phase 3; both
 * end in phase 4 with a fade-complete callback picked. Ticks the banners in
 * every phase. */
STATIC void runPhaseTask_8c02c072() {
    int i;

    if (var_runState_8c2285c4.runPhase_0x00 != 0) {
        if (var_runState_8c2285c4.runPhase_0x00 == 2) {
            unsigned int flags;
            int vib;
            int laneAlias;

            var_frameSpeed_8c22866c = var_busState_8c1bb9d0.speed_0x27c;
            var_offCourseBits_8c228680 = var_busState_8c1bb9d0.junctionBRoadFlags_0x368 & 0x30000;
            if (var_offCourseBits_8c228680 < (var_busState_8c1bb9d0.junctionARoadFlags_0x34c & 0x30000)) {
                var_offCourseBits_8c228680 = var_busState_8c1bb9d0.junctionARoadFlags_0x34c & 0x30000;
            }

            if ((var_busState_8c1bb9d0.junctionBAttr1_0x36c & 0x80) == 0) {
                int ang = var_busState_8c1bb9d0.ang_0x250
                          + (var_busState_8c1bb9d0.junctionBAttr1_0x36c & 0xf) * -0x1000;
                if (var_busState_8c1bb9d0.ang_0x250 < 0) {
                    ang += 0x10000;
                }
                if ((ang < 0x4001 || ang > 0xbfff) && (ang > -0x4001 || ang < -0xbfff)) {
                    var_headingVsRoad_8c22868c = 1;
                } else {
                    var_headingVsRoad_8c22868c = 2;
                }
            } else {
                var_headingVsRoad_8c22868c = 0;
            }

            var_prevLane_8c228684 = var_runState_8c2285c4.lane_0x6c;
            var_prevLaneFlags_8c228688 = var_runState_8c2285c4.field_0x70[0];
            laneAlias = var_busState_8c1bb9d0.junctionARoadFlags2_0x358 & 0xf0000001;
            var_laneA_8c228674 = laneAlias;
            var_laneB_8c228678 = var_busState_8c1bb9d0.junctionBRoadFlags2_0x374 & 0xf0000001;
            var_laneC_8c22867c = var_busState_8c1bb9d0.junctionCRoadFlags2_0x390 & 0xf0000001;
            if (laneAlias == var_laneB_8c228678) {
                var_runState_8c2285c4.field_0x70[0] = var_busState_8c1bb9d0.junctionARoadFlags2_0x358 & 0xf000000;
                var_runState_8c2285c4.lane_0x6c = laneAlias;
            }

            handleBump_8c02b6d4();

            flags = var_wallHitBits_8c228660 & 6;
            if (flags != 0) {
                vib = 0;
                if (flags == 2) {
                    vib = 4;
                } else if (flags == 4) {
                    vib = 5;
                } else if (flags == 6) {
                    vib = 6;
                }
                if (vib != 0) {
                    VibStart_8c010f7a(vib);
                }
            }

            /* A priority chain: an offense's cooldown silences every lesser
             * category too. */
            gradeOffCourseSevere_8c02b864();
            if (--var_cooldownCollision_8c228690 < 0) {
                gradeWallHit_8c02b7ea();
                if (--var_cooldownOffCourse_8c228694 < 0) {
                    gradeOffCourse_8c02b886();
                    if (--var_cooldownSignal_8c228698 < 0) {
                        gradeSignals_8c02b8b8();
                        if (--var_cooldownLane_8c22869c < 0) {
                            gradeLaneUse_8c02b986();
                            if (--var_cooldownIntersection_8c2286a0 < 0) {
                                gradeIntersection_8c02bb1c();
                            }
                        }
                    }
                }
            }

            gradeFrame_8c02bcd8();
            var_runState_8c2285c4.field_0x70[1] = var_busState_8c1bb9d0.junctionARoadFlags_0x34c;
            var_runState_8c2285c4.field_0x70[2] = var_busState_8c1bb9d0.junctionBRoadFlags_0x368;

            if (var_runState_8c2285c4.driverPoints_0x0c == 0) {
                var_busState_8c1bb9d0.driveState_0x2b4 = 4;
                var_runState_8c2285c4.runPhase_0x00 = 4;
                var_runState_8c2285c4.driveEndHold_0x08 = 0x1e;
                var_messageBoxActive_8c22847c = 1;
                if (var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE) {
                    var_fadeCompleteCallback_8c22656c = GradeOnFadePracticeEnd_8c02c784;
                } else {
                    var_fadeCompleteCallback_8c22656c = onFadeRunFailed_8c02c76a;
                }

                SndStartAdxFadeOut_8c010bae(0);
                SndStartAdxFadeOut_8c010bae(1);
            }
        } else if (var_runState_8c2285c4.runPhase_0x00 == 3) {
            if (var_runState_8c2285c4.stopArrivalGrade_0x24 == 0) {
                if (var_driveMode_8c1bb8c8 == 0) {
                    int diff = var_nextStopHeading_8c228714 - var_busState_8c1bb9d0.ang_0x250;
                    if ((diff < -0x71c && diff > -0xf8e3) || (diff > 0x71c && diff < 0xf8e3)) {
                        adjustPoints_8c02b464(0x1b, -3); /* -> INSTR_BAD_STOP_POSITION_1 */
                    }
                }
                if (var_busState_8c1bb9d0.signalSide_0x25c != 1 || var_runState_8c2285c4.signalHeldFrames_0x58 < 0x3c) {
                    adjustPoints_8c02b464(0xf, -8); /* -> INSTR_NO_SIGNAL */
                }
            } else if (var_runState_8c2285c4.stopArrivalGrade_0x24 == 1) {
                adjustPoints_8c02b464(0x1c, -10); /* -> INSTR_BAD_STOP_POSITION_1 */
            } else if (var_runState_8c2285c4.stopArrivalGrade_0x24 == 2) {
                adjustPoints_8c02b464(0x1a, -20); /* -> INSTR_MISSED_STOP */
            }

            var_runState_8c2285c4.runPhase_0x00 = 4;
            var_runState_8c2285c4.driveEndHold_0x08 = 0x1e;
            var_messageBoxActive_8c22847c = 1;
            if (var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE) {
                var_fadeCompleteCallback_8c22656c = GradeOnFadePracticeEnd_8c02c784;
            } else if (var_runState_8c2285c4.driverPoints_0x0c == 0) {
                var_fadeCompleteCallback_8c22656c = onFadeRunFailed_8c02c76a;
            } else {
                var_fadeCompleteCallback_8c22656c = onFadeStopEnded_8c02c624;
            }

            if (var_runState_8c2285c4.driverPoints_0x0c > 0 && GradeRunComplete_8c02c586() != 0) {
                var_runState_8c2285c4.runPassed_0x04 = 1;
            }

            SndStartAdxFadeOut_8c010bae(0);
            SndStartAdxFadeOut_8c010bae(1);
        } else {
            if (var_runState_8c2285c4.runPhase_0x00 == 4) {
                var_runState_8c2285c4.driveEndHold_0x08 = var_runState_8c2285c4.driveEndHold_0x08 - 1;
                if (var_runState_8c2285c4.driveEndHold_0x08 < 0 && init_adxPlaying_8c03bd80 == 0) {
                    sdMidiStopAll();
                    if (var_vibport_8c1ba354 != -1) {
                        pdVibMxStop(var_vibport_8c1ba354);
                    }
                    var_runState_8c2285c4.runPhase_0x00 = 5;
                    var_fadeRequest_8c226564 = FADE_REQUEST_OUT;
                    BusCameraSaveState_8c024b4c();
                } else {
                    SndUpdateAdxVolFade_8c010a40();
                }
            }
        }
    }

    for (i = 0; i < 4; i++) {
        DriveMsgSlot *slot = &var_driveMsgQueue_8c228564[i];
        if (slot->holdFrames != 0) {
            if (slot->revealed < slot->count) {
                slot->revealCounter = slot->revealCounter + 1;
                slot->revealed = slot->revealCounter >> 1;
            } else {
                slot->holdFrames = slot->holdFrames - 1;
            }
        }
    }

    RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, DriveMsgDraw_8c02b388, 0);
}

void GradeInstall_8c02c46a(void) {
    Task *installed_task;
    void *installed_state;
    int i;

    TaskInstall_8c014ae8(var_tasks_8c1ba5e8, (void *)runPhaseTask_8c02c072, &installed_task, &installed_state, 0);

    var_runState_8c2285c4.runPhase_0x00 = 0;
    var_runState_8c2285c4.runPassed_0x04 = 0;

    if (var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE && (var_practiceRules_8c226410 & 4) != 4) {
        var_runState_8c2285c4.stopPhase_0x20 = 4;
    } else {
        var_runState_8c2285c4.stopPhase_0x20 = 1;
    }

    var_runState_8c2285c4.wrongLaneCount_0x2c = 0;
    var_runState_8c2285c4.speedingCountdown_0x30 = 0;
    var_runState_8c2285c4.stopLineGraded_0x34 = 0;
    var_runState_8c2285c4.field_0x38[0] = 0;
    var_runState_8c2285c4.field_0x38[2] = 0;
    var_runState_8c2285c4.field_0x38[3] = 0;
    var_runState_8c2285c4.field_0x38[4] = 0;
    var_runState_8c2285c4.field_0x38[5] = 0;
    var_runState_8c2285c4.field_0x38[6] = 0;
    var_runState_8c2285c4.field_0x38[7] = -1;
    var_runState_8c2285c4.hasMoved_0x5c = 0;
    var_runState_8c2285c4.startIdleFrames_0x60 = 0;
    var_runState_8c2285c4.crossingState_0x64 = CROSSING_IDLE;
    var_runState_8c2285c4.crossingStopTimer_0x68 = 0;
    var_runState_8c2285c4.lane_0x6c = var_busState_8c1bb9d0.junctionARoadFlags2_0x358 & 0xf0000000;
    var_runState_8c2285c4.field_0x70[0] = var_busState_8c1bb9d0.junctionARoadFlags2_0x358 & 0xf000000;
    var_runState_8c2285c4.field_0x70[1] = var_busState_8c1bb9d0.junctionARoadFlags_0x34c;
    var_runState_8c2285c4.field_0x70[2] = var_busState_8c1bb9d0.junctionBRoadFlags_0x368;
    var_runState_8c2285c4.instructionBonusPending_0x7c = 0;
    var_runState_8c2285c4.firstUpshift_0x88 = 0;
    var_runState_8c2285c4.brakeAverage_0x90 = 0.0f;
    var_runState_8c2285c4.hardBrakeCooldown_0x94 = 0;
    var_runState_8c2285c4.swerveCountdown_0x98 = 0;
    var_cooldownCollision_8c228690 = 0;
    var_cooldownOffCourse_8c228694 = 0;
    var_cooldownSignal_8c228698 = 0;
    var_cooldownLane_8c22869c = 0;
    var_cooldownIntersection_8c2286a0 = 0;

    for (i = 0; i < 4; i++) {
        var_driveMsgQueue_8c228564[i].holdFrames = 0;
    }
}
