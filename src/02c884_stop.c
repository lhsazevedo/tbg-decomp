/* @unit Stop */

#include <shinobi.h>
#include <stdlib.h> /* rand */
#include "includes.h" /* STATIC */

#include "026710_traffic.h"
#include "015034_text.h"
#include "1ba1c8_globals.h"
#include "01fa78_hud.h" /* var_hudState_8c22643c */
#include "020214_drive_cue.h" /* var_driveCueState_8c2264b8 */
#include "01e27c_practice_menu.h" /* var_practiceRules_8c226410 */
#include "0289ac_objects.h"
#include "013ae8_route.h" /* enum ROUTE */
#include "011120_asset_queues.h" /* AsqGetRandomInRangeA_8c012178 */
#include "014a9c_tasks.h" /* TaskKillGroup_8c014ab4 */
#include "020914_ground_query.h" /* GroundQueryFindPolygon_8c020914, GroundQueryResult */
#include "020b6c_ground_probe.h"
#include "02af78_event.h" /* EventScanCandidates_8c02b03c */
#include "01614c_replay_menu.h" /* ReplayMenuResetDemoCursor_8c016770 */
#include "02c884_stop.h" /* StopGetSegment_8c02cd6a */
#include "022464_render.h" /* RenderQueueDraw_8c0223ea */
#include "0100bc_sound.h" /* SndStartAdxFadeOut_8c010bae */
#include "02b464_grading.h" /* GradeRunComplete_8c02c586, GradeOnFadeDriveEnd_8c02c784 */

/* ====================
 * Compiler Definitions
 * ====================
 */

#define MAX_WAITING ((int)(sizeof(var_waitingPassengers_8c228798) / \
                           sizeof(var_waitingPassengers_8c228798[0])))

/* ====================
 * Non-initialized Globals
 * ====================
 */

/* per-segment "has an active stop" flag, one word each, indexed by a segment
 * record's candidate-list entry byte (see StopGetSegment_8c02cd6a, 02c884) */
STATIC int var_segmentHasStop_8c2286a4[24];

int var_startStopIndex_8c228704;

int var_currentSegment_8c228708;

int var_prevStopSegment_8c22870c; // segment index of the previous stop
int var_nextStopSegment_8c228710; // segment index of the upcoming stop

int var_nextStopHeading_8c228714;

int var_stopSchedule_8c228718[31];

int var_waitingPassengerCount_8c228794;

WaitingPassengerSlot var_waitingPassengers_8c228798[16];

NJS_SPRITE var_passengerSprite_8c2288d8;

void* var_stopTaskGroup_8c2288f8;

/* Same computation as var_nextStopHeading_8c228714, but masked to unsigned 16
 * bits instead of sign-extended. */
STATIC int var_currentStopHeading_8c2288fc;

/* scratch ground-query point for the upcoming stop, snapped to ground by
 * pickWaitingPassengers_8c02c8ae; x/z (only) also set by
 * StopUpdateHeadings_8c02ccc6 ahead of that ground snap */
STATIC NJS_POINT3 var_nextStopPoint_8c228900;

/* Spawn-area record for the upcoming stop's segment (the course's lineHum_0x2c
 * table, selected by its segment record's ukn_0x06). */
STATIC StopAreaRecord *var_nextStopArea_8c22890c;
/* ====================
 * Functions
 * ====================
 */

/* Empties the scripted schedule (-1 = unused slot) and resets the shared
 * passenger sprite ahead of a new stop. */
STATIC void resetStopState_8c02c884(void)
{
    int i;

    for (i = 0; i < (int)(sizeof(var_stopSchedule_8c228718) /
                          sizeof(var_stopSchedule_8c228718[0])); i++) {
        var_stopSchedule_8c228718[i] = -1;
    }

    var_passengerSprite_8c2288d8.sx = 0.014f;
    var_passengerSprite_8c2288d8.sy = 0.014f;
    var_passengerSprite_8c2288d8.ang = 0;
    var_passengerSprite_8c2288d8.tanim = init_pedestrianTexAnims_8c04623c;
}

CourseSegment *StopGetSegment_8c02cd6a(int segmentIndex)
{
    return &var_currentCourseConfig_8c18ad18->segments_0x08[segmentIndex];
}

/* Advances prevStopSegment to the just-armed stop and reloads the
 * event-gate threshold (var_runState_8c2285c4.scheduleTime_0x14) from the course's per-segment gate
 * table (courseConfig's ukn_0x0c). */
STATIC void advanceStopSegment_8c02ccae(void)
{
    var_prevStopSegment_8c22870c++;
    var_runState_8c2285c4.scheduleTime_0x14 = ((int *)var_currentCourseConfig_8c18ad18->ukn_0x0c)[var_prevStopSegment_8c22870c];
}

StopAreaRecord *StopGetArea_8c02cd7a(int segmentIndex)
{
    CourseSegment *seg = StopGetSegment_8c02cd6a(segmentIndex);
    return *(StopAreaRecord **)((char *)var_currentCourse_8c1bb868.lineBus_0x08 + seg->stopAreaId_0x02 * 8);
}

/* The same njArcTan2 angle is stored twice in different forms: var_currentStopHeading_8c2288fc
 * masked to unsigned 16 bits for the stop being left, var_nextStopHeading_8c228714
 * sign-extended to a full Angle for the one coming up. */
void StopUpdateHeadings_8c02ccc6(void)
{
    StopAreaRecord *rec;
    int seg;
    int angle;

    var_currentSegment_8c228708 = var_nextStopSegment_8c228710;
    rec = StopGetArea_8c02cd7a(var_nextStopSegment_8c228710);
    var_currentStopHeading_8c2288fc = (int)(njArcTan2(rec->dx_0x0c, rec->dz_0x10) + 0x8000) & 0xffff;

    seg = var_currentSegment_8c228708;
    do {
        seg++;
        var_nextStopSegment_8c228710 = seg;
    } while (var_segmentHasStop_8c2286a4[seg] == 0);

    rec = StopGetArea_8c02cd7a(var_nextStopSegment_8c228710);
    var_nextStopPoint_8c228900.x = rec->x_0x04;
    var_nextStopPoint_8c228900.z = rec->z_0x08;
    angle = njArcTan2(rec->dx_0x0c, rec->dz_0x10) + 0x8000;
    var_nextStopHeading_8c228714 = angle;
    if ((angle & 0x8000) != 0) {
        var_nextStopHeading_8c228714 = angle | 0xffff0000;
    }
}

void StopKillTaskGroup_8c02ca96(void)
{
    if (var_stopTaskGroup_8c2288f8 != (void *)-1) {
        TaskKillGroup_8c014ab4((Task *)var_stopTaskGroup_8c2288f8);
        syFree(var_stopTaskGroup_8c2288f8);
        var_stopTaskGroup_8c2288f8 = (void *)-1;
    }
}

/* Picks the waiting passengers for the upcoming stop (var_nextStopSegment_8c228710):
 * snaps its ground position, collects the segment's candidate stop spots that
 * have an active-stop flag (var_segmentHasStop_8c2286a4) into a scratch list, then randomly
 * picks 1-16 of them without replacement into var_waitingPassengers_8c228798, positioned along
 * the stop's spawn-area strip (the course's lineHum_0x2c table) with per-passenger jitter --
 * except on ROUTE_OME, which skips the jitter. */
STATIC void pickWaitingPassengers_8c02c8ae(void)
{
    GroundQueryResult ground;
    void **candidates;
    CourseSegment *seg;
    StopAreaRecord *area;
    char *list;
    void **slot;
    int candidateCount;
    int pick;
    int i;
    float x0, z0, dx, dz;
    float counter;

    var_waitingPassengerCount_8c228794 = 0;
    var_activeGroundGrid_8c2264d4 = var_currentCourse_8c1bb868.atariBus_0x04;

    GroundQueryFindPolygon_8c020914(var_nextStopPoint_8c228900.x, var_nextStopPoint_8c228900.y, var_nextStopPoint_8c228900.z, &ground);
    GroundProbeInterpolateHeight_8c020f7e(&ground, (float *)&var_nextStopPoint_8c228900);

    seg = StopGetSegment_8c02cd6a(var_nextStopSegment_8c228710);
    /* Sized for 16 candidates, but the scan below is bounded only by the
     * segment's list -- a segment offering more than 16 active spots would
     * run past this. */
    candidates = (void **)syMalloc(16 * sizeof(void *));

    list = (char *)seg->stopCandidates_0x08;
    candidateCount = 0;
    if (list != 0) {
        for (; list[1] != 0; list += 2) {
            if (var_segmentHasStop_8c2286a4[(int)list[1]] != 0) {
                candidates[candidateCount++] = list;
            }
        }

        if (candidateCount != 0) {
            var_nextStopArea_8c22890c = *(StopAreaRecord **)((char *)var_currentCourse_8c1bb868.lineHum_0x2c + seg->ukn_0x06 * 0xc);

            var_waitingPassengerCount_8c228794 = AsqGetRandomInRangeA_8c012178(candidateCount) + 1;
            if (var_waitingPassengerCount_8c228794 > MAX_WAITING) {
                var_waitingPassengerCount_8c228794 = MAX_WAITING;
            }

            area = var_nextStopArea_8c22890c;
            x0 = area->x_0x04;
            z0 = area->z_0x08;
            dx = area->dx_0x0c;
            dz = area->dz_0x10;

            counter = 0.0f;
            for (i = 0; i < var_waitingPassengerCount_8c228794; i++) {
                do {
                    pick = AsqGetRandomInRangeA_8c012178(candidateCount);
                    slot = &candidates[pick];
                } while (*slot == (void *)-1);

                var_waitingPassengers_8c228798[i].spot_0x00 = *slot;

                if (var_route_8c18ad1c == ROUTE_OME) {
                    var_waitingPassengers_8c228798[i].pos_0x04.x = counter * dx + x0;
                    var_waitingPassengers_8c228798[i].pos_0x04.z = counter * dz + z0;
                } else {
                    var_waitingPassengers_8c228798[i].pos_0x04.x =
                        counter * dx + x0 + (float)rand() / 32768.0f - 0.5f;
                    var_waitingPassengers_8c228798[i].pos_0x04.z =
                        counter * dz + z0 + (float)rand() / 32768.0f - 0.5f;
                }

                GroundQueryFindPolygon_8c020914(var_waitingPassengers_8c228798[i].pos_0x04.x, 0.0f,
                                                var_waitingPassengers_8c228798[i].pos_0x04.z, &ground);
                GroundProbeInterpolateHeight_8c020f7e(
                    &ground, (float *)&var_waitingPassengers_8c228798[i].pos_0x04);

                var_waitingPassengers_8c228798[i].index_0x10 = counter;
                counter += 1.0f;

                *slot = (void *)-1;
            }
        }
    }

    syFree(candidates);
}

/* Per-run bus-stop setup, called right after the course loads. Clears the
 * active-stop flags, flags one segment per candidate story event, then
 * forces a stop at every type-2 segment and randomly flags additional
 * segments (never type-3) until the course's randomized total stop count
 * (courseConfig's [randomStopCountMin_0x14, randomStopCountMax_0x18) range)
 * is reached. Finishes by priming the next/prev stop segment and the
 * upcoming stop's passengers, and resetting a couple of run-scoped
 * timers/thresholds (var_runState_8c2285c4.driverPoints_0x0c/var_runState_8c2285c4.driverPointsMax_0x10,
 * var_runState_8c2285c4.scheduleTime_0x14/var_runState_8c2285c4.runClock_0x18) based on the difficulty setting
 * and play mode. */
void StopSetup_8c02caba(void)
{
    int i;
    int candidateId;
    int activeStopCount;
    int extraStopCount;
    int totalSegments;
    int pick;
    CourseSegment *segments;

    for (i = 0; i < (int)(sizeof(var_segmentHasStop_8c2286a4) / sizeof(var_segmentHasStop_8c2286a4[0])); i++) {
        var_segmentHasStop_8c2286a4[i] = 0;
    }

    EventScanCandidates_8c02b03c();
    for (i = 0; i < var_eventCandidateCount_8c228560; i++) {
        candidateId = var_eventCandidates_8c228520[i];
        var_segmentHasStop_8c2286a4[var_routeEvents_8c22851c[candidateId].segmentId_0x02] = 1;
    }

    segments = var_currentCourseConfig_8c18ad18->segments_0x08;
    activeStopCount = 0;
    totalSegments = 0;
    while (segments->type_0x00 != 0) {
        if (var_segmentHasStop_8c2286a4[totalSegments] != 0) {
            activeStopCount++;
        } else if (segments->type_0x00 == 2) {
            var_segmentHasStop_8c2286a4[totalSegments] = 1;
            activeStopCount++;
        }
        totalSegments++;
        segments++;
    }

    extraStopCount = AsqGetRandomInRangeA_8c012178(
                         var_currentCourseConfig_8c18ad18->randomStopCountMax_0x18 -
                         var_currentCourseConfig_8c18ad18->randomStopCountMin_0x14) +
                     var_currentCourseConfig_8c18ad18->randomStopCountMin_0x14 -
                     activeStopCount;

    /* segments now points at the scan's terminator record (courseConfig's
     * segments array + totalSegments), not its start -- and the reroll loop
     * below indexes segments[pick] off that same (unreset) pointer, so its
     * type_0x00 check actually lands totalSegments+pick records past the
     * course's segment table start. Preserved as-is: real asm behavior. */
    while (extraStopCount > 0) {
        pick = AsqGetRandomInRangeA_8c012178(totalSegments);
        if (segments[pick].type_0x00 != 3 && var_segmentHasStop_8c2286a4[pick] == 0) {
            extraStopCount--;
            var_segmentHasStop_8c2286a4[pick] = 1;
        }
    }

    ReplayMenuResetDemoCursor_8c016770();

    if (var_playMode_8c1bb8d0 != PLAY_MODE_DEMO) {
        var_activeTrafficPreset_8c227e14 = 0;
        var_activePedPreset_8c22822c = 0;
    }

    var_nextStopSegment_8c228710 = var_startStopIndex_8c228704;
    var_prevStopSegment_8c22870c = var_startStopIndex_8c228704 - 1;

    resetStopState_8c02c884();
    pickWaitingPassengers_8c02c8ae();
    StopUpdateHeadings_8c02ccc6();
    advanceStopSegment_8c02ccae();

    if (var_progress_8c1ba1cc.difficulty_0xc4 < 1 && var_playMode_8c1bb8d0 != PLAY_MODE_PRACTICE) {
        var_runState_8c2285c4.driverPointsMax_0x10 = 200;
    } else {
        var_runState_8c2285c4.driverPointsMax_0x10 = 100;
    }
    var_runState_8c2285c4.driverPoints_0x0c = var_runState_8c2285c4.driverPointsMax_0x10;

    if (var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE && (var_practiceRules_8c226410 & 2) != 2) {
        var_runState_8c2285c4.runClock_0x18 = var_runState_8c2285c4.scheduleTime_0x14;
        var_runState_8c2285c4.scheduleTime_0x14 = 0;
    } else {
        var_runState_8c2285c4.runClock_0x18 = var_runState_8c2285c4.scheduleTime_0x14 - 0x1c2;
    }
}

/* Draws the stop marker (the "Foo" model, var_fuuNjm_8c1bc448) at the
 * upcoming stop, facing its heading and animated by var_fuuFrame_8c1bc44c. Installed
 * as a DrawFn by StopUpdateArrival_8c02ce48 during the approach;
 * the callback arg is unused. */
STATIC void drawStopMarker_8c02cd92(int arg0)
{
    float frame;

    njUnitMatrix(&var_scratchMatrix_8c1bc46c);
    frame = var_fuuFrame_8c1bc44c;
    njTranslate(&var_scratchMatrix_8c1bc46c, var_nextStopPoint_8c228900.x, var_nextStopPoint_8c228900.y, var_nextStopPoint_8c228900.z);
    njRotateY(&var_scratchMatrix_8c1bc46c, var_nextStopHeading_8c228714);
    njMultiMatrix(0, &var_scratchMatrix_8c1bc46c);
    njSetTexture(var_fuuTexlist_8c1bc440);
    njCnkSimpleDrawMotion(var_fuuNj_8c1bc444, var_fuuNjm_8c1bc448, frame);
}

/* Per-frame bus-stop arrival state machine, called once per frame by
 * 02b464_grading. States:
 * 0 = cruising -- watches var_busState_8c1bb9d0.markCueByte_0x3b4 for a
 *     just-crossed segment (its high byte) matching the next or previous
 *     stop segment, arming state 2 (approach) or 1 (post-departure wait);
 * 1 = waits for markCueByte_0x3b4's low byte to clear, then re-arms cruising and
 *     advances to the next stop segment;
 * 2 = approach -- tracks the running minimum straight-line distance from
 *     the bus (busState's posX_0x0f4/posZ_0x0fc) to the upcoming stop
 *     (var_nextStopPoint_8c228900.x/.z) and drives the marker's animation
 *     frame/draw callback, transitioning to "stopped" (3, via bus_state 3)
 *     once close enough and halted (var_busState_8c1bb9d0.speed_0x27c == 0), or once the stop
 *     segment is reached outright (bus_state 4);
 * 3 = idle, waiting for something external to request the finish;
 * 4 = finishes the stop once markDriveFlags_0x3b0's top byte is set, installing the
 *     fade-complete callback and starting both ADX fade-outs. */
void StopUpdateArrival_8c02ce48(void)
{
    int crossedSegment;
    float distance;
    float dx, dz;

    if (var_runState_8c2285c4.stopPhase_0x20 == 0) {
        if ((var_busState_8c1bb9d0.markCueByte_0x3b4 & 0xff00) != 0) {
            crossedSegment = (var_busState_8c1bb9d0.markCueByte_0x3b4 & 0xff00) >> 8;
            if (crossedSegment == var_nextStopSegment_8c228710) {
                var_runState_8c2285c4.stopPhase_0x20 = 2;
                var_runState_8c2285c4.stopMinDistance_0x28 = 9999.0f;
                var_hudState_8c22643c.blinkTimer_0x18 = 0;
                pickWaitingPassengers_8c02c8ae();
                var_fuuFrame_8c1bc44c = 0.0f;
            } else if (crossedSegment == var_prevStopSegment_8c22870c) {
                var_runState_8c2285c4.stopPhase_0x20 = 1;
                var_hudState_8c22643c.blinkTimer_0x18 = 0;
            }
        }
    } else if (var_runState_8c2285c4.stopPhase_0x20 == 1) {
        if ((var_busState_8c1bb9d0.markCueByte_0x3b4 & 0xff) != 0) {
            var_runState_8c2285c4.stopPhase_0x20 = 0;
            if (var_hudState_8c22643c.driveMarkIcon_0x14 != -1) {
                var_runState_8c2285c4.instructionBonusPending_0x7c = 1;
            }
            var_driveCueState_8c2264b8.announceRequested_0x0c = 0;
            advanceStopSegment_8c02ccae();
        }
    } else if (var_runState_8c2285c4.stopPhase_0x20 == 2) {
        dx = var_nextStopPoint_8c228900.x - var_busState_8c1bb9d0.posX_0x0f4;
        dz = var_nextStopPoint_8c228900.z - var_busState_8c1bb9d0.posZ_0x0fc;
        distance = njSqrt(dx * dx + dz * dz);
        var_fuuFrame_8c1bc44c += 1.0f;
        if (var_fuuLastFrame_8c1bc450 <= var_fuuFrame_8c1bc44c) {
            var_fuuFrame_8c1bc44c = 0.0f;
        }
        RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, drawStopMarker_8c02cd92, 0);
        if (distance < var_runState_8c2285c4.stopMinDistance_0x28) {
            var_runState_8c2285c4.stopMinDistance_0x28 = distance;
        }
        if ((distance < 3.0f && var_busState_8c1bb9d0.speed_0x27c == 0.0f) ||
            (var_runState_8c2285c4.stopMinDistance_0x28 < 3.0f && var_busState_8c1bb9d0.speed_0x27c == 0.0f)) {
            var_busState_8c1bb9d0.driveState_0x2b4 = 3;
            var_runState_8c2285c4.stopPhase_0x20 = 3;
            var_runState_8c2285c4.runPhase_0x00 = 3;
            var_runState_8c2285c4.stopArrivalGrade_0x24 = 0;
        } else if ((var_busState_8c1bb9d0.markCueByte_0x3b4 & 0xff) == var_nextStopSegment_8c228710) {
            var_busState_8c1bb9d0.driveState_0x2b4 = 4;
            var_runState_8c2285c4.stopPhase_0x20 = 3;
            var_runState_8c2285c4.runPhase_0x00 = 3;
            var_runState_8c2285c4.stopArrivalGrade_0x24 = 2;
        }
    } else if (var_runState_8c2285c4.stopPhase_0x20 == 4) {
        if ((var_busState_8c1bb9d0.markDriveFlags_0x3b0 & 0xff000000) != 0) {
            var_busState_8c1bb9d0.driveState_0x2b4 = 4;
            var_runState_8c2285c4.stopPhase_0x20 = 3;
            var_runState_8c2285c4.stopArrivalGrade_0x24 = 0;
            var_runState_8c2285c4.runPhase_0x00 = 4;
            var_runState_8c2285c4.driveEndHold_0x08 = 0x1e;
            var_fadeCompleteCallback_8c22656c = GradeOnFadeDriveEnd_8c02c784;
            if (0 < var_runState_8c2285c4.driverPoints_0x0c && GradeRunComplete_8c02c586() != 0) {
                var_runState_8c2285c4.runPassed_0x04 = 1;
            }
            SndStartAdxFadeOut_8c010bae(0);
            SndStartAdxFadeOut_8c010bae(1);
        }
    }
}
