/* Driver points: the running score a drive is graded against, and what
 * happens when it runs out or the route finishes. */
#ifndef _02B464_GRADING_H
#define _02B464_GRADING_H

/* Per-run drive state. */
typedef struct {
    /* The run phase, stepped by runPhaseTask_8c02c072 (02b464): 0 = not
     * driving, 2 = driving and being graded, 3 = at the stop, 4 = wrapping up,
     * 5 = done. Several units gate their per-frame work on it. */
    int runPhase_0x00;

    /* Set when a drive ends with points left and every owed stop served -- by
     * StopUpdateArrival_8c02ce48 (02c884) on a finished stop, and by
     * runPhaseTask_8c02c072 (02b464) on the phase-3 wrap-up. Cleared for the
     * next drive by GradeSpawnTask_8c02c46a. DriveMsgDraw_8c02b388 (02b2f0)
     * is its only reader. */
    int runPassed_0x04;

    /* Frames left in run phase 4, the hold between the last grade and the
     * fade-in: loaded with 0x1e (1s) by whichever of
     * StopUpdateArrival_8c02ce48 (02c884) or runPhaseTask_8c02c072 (02b464)
     * steps the phase to 4, counted down by runPhaseTask_8c02c072, which moves
     * to phase 5 once it goes negative and the music has finished fading. */
    int driveEndHold_0x08;

    /* Driver points left when the run ended, out of driverPointsMax_0x10.
     * ResultsShowPassedRun_8c01e0b4 cuts the award tier at 70/80/90 and scores
     * them at 10 a point. */
    int driverPoints_0x0c;

    /* The run's starting driver points (100, or 200 on the easiest difficulty
     * outside practice), set with driverPoints_0x0c by StopSetup_8c02caba.
     * Full-scale value of the HUD points meter. */
    int driverPointsMax_0x10;

    /* The timetable slot for the current segment, reloaded from the course's
     * per-segment table by advanceStopSegment_8c02ccae, and the run clock,
     * which StopSetup_8c02caba starts 450 frames (15s) before the first slot
     * and gradeFrame_8c02bcd8 advances every frame. Both are 30fps frame
     * counts, both drawn as HH:MM:SS by the HUD. Running past the slot costs
     * points once a second and closes the story-event window
     * (EventPickForSegment_8c02b170). In a practice drill without rule bit 1
     * the pair is inverted: the slot goes to 0 and the clock counts down to a
     * hard TIME_MANAGEMENT failure. */
    int scheduleTime_0x14;
    int runClock_0x18;

    /* Frames added to runClock_0x18 each frame of the bus-stop scene, winding
     * it up to scheduleTime_0x14 so the HUD clock reaches the departure time as
     * the scene ends: a fifth of what is left while that is at least 50, then a
     * flat 10 (setCountUpStep_8c02d5d8, 02d19c). */
    int clockCatchUpStep_0x1c;

    /* Bus-stop arrival state machine driven by StopUpdateArrival_8c02ce48
     * (02c884): 0 = cruising, 1 = departed-previous-stop wait, 2 = approaching
     * (mirror-view draw enabled -- gates pedestriansTask_8c0293f6's
     * StopDrawWaitingPassengers_8c02d06c registration), 3 = stopped/waiting,
     * 4 = finishing. */
    int stopPhase_0x20;

    /* How the approach ended, written by StopUpdateArrival_8c02ce48 (02c884)
     * and graded once by runPhaseTask_8c02c072's phase 3 (02b464): 0 = pulled up
     * at the marker (graded on heading and turn signal), 2 = drove past the stop
     * segment (INSTR_MISSED_STOP, -20). Phase 3 also handles a 1
     * (INSTR_BAD_STOP_POSITION_1, -10) that nothing in the image ever writes. */
    int stopArrivalGrade_0x24;

    /* Running minimum distance-to-stop while approaching (stopPhase_0x20 == 2),
     * reset to 9999.0 on arming. */
    float stopMinDistance_0x28;

    /* Three per-offense counters of gradeSignals_8c02b8b8 / gradeLaneUse_8c02b986 /
     * gradeIntersection_8c02bb1c (02b464), all cleared by GradeSpawnTask_8c02c46a.
     * wrongLaneCount counts frames off-course, and escalates INSTR_WRONG_LANE from
     * -10 to -50 after the first. speedingCountdown is reloaded to 120 on each
     * speeding dock and zeroed the moment the bus is back under the limit, so a
     * sustained overspeed costs points every 4s. stopLineGraded is the one-shot
     * "this signal already cost a stop-line penalty" latch, cleared when the
     * signal goes out of range. */
    int wrongLaneCount_0x2c;
    int speedingCountdown_0x30;
    int stopLineGraded_0x34;

    /* [0]/[1] both take the current traffic-signal id from gradeSignals_8c02b8b8
     * (02b464); [0] is cleared when the signal goes out of range, [1] keeps the
     * last one and is what gradeIntersection_8c02bb1c reads. [3] counts moving
     * frames since the bus's lane probes last agreed and [4] how many lane-straddle
     * penalties have landed (both raise the threshold gradeLaneUse_8c02b986 grades
     * against). [6] latches that the bus is inside a junction; when it leaves,
     * gradeIntersection_8c02bb1c grades the turn signal against the turn taken.
     * Other slots unclear. */
    int field_0x38[8];

    int signalHeldFrames_0x58;
    int hasMoved_0x5c;
    int startIdleFrames_0x60;

    /* Level-crossing state machine and its stationary-on-the-tracks timer --
     * see gradeFrame_8c02bcd8 (02b464). */
    int crossingState_0x64;
    int crossingStopTimer_0x68;

    /* The bus's lane, junctionARoadFlags2_0x358 masked with 0xf0000001,
     * refreshed by runPhaseTask_8c02c072 (02b464) only on frames where the A and
     * B road probes agree. var_prevLane_8c228684 is the previous frame's copy,
     * and the difference is what the lane-change graders read. */
    int lane_0x6c;

    /* [0] is junctionARoadFlags2_0x358 masked with 0xf000000, refreshed by
     * runPhaseTask_8c02c072 (02b464) in step with lane_0x6c;
     * var_prevLaneFlags_8c228688 is the previous frame's copy. [1]/[2] are the raw
     * junctionARoadFlags_0x34c / junctionBRoadFlags_0x368, stored after the graders
     * have run, so gradeLaneUse_8c02b986 sees last frame's 0x40000 turn-signal bit
     * in them and this frame's in busState. */
    int field_0x70[3];

    /* Latched when the bus leaves a stop with a drive-mark instruction on screen
     * (var_hudState_8c22643c.driveMarkIcon_0x14 != -1); gradeFrame_8c02bcd8
     * (02b464) spends it on a +20 msgSet-0x1e award. */
    int instructionBonusPending_0x7c;

    /* Counts frames the bus has been moving, reset at a standstill by
     * BusInputUpdate_8c0246b2 (024280) and read nowhere in the image -- a dead
     * store. Its neighbours are all gradeFrame_8c02bcd8 scoring inputs, so it
     * looks like a dropped scoring rule. */
    int field_0x80;

    /* gradeFrame_8c02bcd8's rapid-acceleration penalty: the latch is armed by an
     * upshift taken with the .r trigger past 0xfe (firstUpshift_0x88) and
     * disarmed as soon as the trigger eases off; the frame count docks
     * INSTR_RAPID_ACCEL once the trigger stays there past 14 frames. */
    int fullThrottleLatch_0x84;

    /* Set to 1 on the upshift out of gear 0 by applyThrottle_8c024320 (024280),
     * arming gradeFrame_8c02bcd8's rapid-acceleration penalty (02b464), which
     * clears it. */
    int firstUpshift_0x88;
    int fullThrottleFrames_0x8c;

    /* Running average of applyBraking_8c024530's per-frame brake amount
     * (avg = (avg + amount) / 2, reset to 0 on any non-braking frame).
     * gradeFrame_8c02bcd8 docks INSTR_HARD_BRAKE once it passes 0.01, then sets the
     * cooldown to 60 frames so one long stab is only docked once. */
    float brakeAverage_0x90;
    int hardBrakeCooldown_0x94;

    /* Frames left before gradeFrame_8c02bcd8 docks INSTR_SWERVING again; reloaded
     * to 60 on each dock and zeroed whenever speed * steering angle comes back
     * inside +/-2000. */
    int swerveCountdown_0x98;
} RunState;
extern RunState var_runState_8c2285c4;

/* Bitflags set by hitWall_8c023bea (023938_bus_drive); bits
 * 0x2/0x4 are read by gradeWallHit_8c02b7ea (02b464) to grade a
 * driver-points penalty -- both set is worse than either alone. Other bits
 * unclear. */
extern int var_wallHitBits_8c228660;

/* Starts a drive: installs the master per-frame drive task
 * (runPhaseTask_8c02c072) and resets this unit's whole scratch scoring/
 * state region for a fresh run. */
void GradeSpawnTask_8c02c46a(void);

/* Reports whether the run has reached the last stop it owes: a required
 * var_nextStopSegment_8c228710, picked by practice-lesson id when
 * practicing and by route otherwise. */
int GradeRunComplete_8c02c586(void);

/* var_fadeCompleteCallback_8c22656c for a finished drive: retries the route
 * segment if the run still owes a stop, otherwise heads for the results or
 * lesson-retry screen. Installed by StopUpdateArrival_8c02ce48 (02c884)
 * and by this unit's own drive-end paths. */
void GradeOnFadePracticeEnd_8c02c784(void);

#endif // _02B464_GRADING_H
