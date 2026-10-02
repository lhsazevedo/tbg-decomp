/* Driver points: the running score a drive is graded against, and what
 * happens when it runs out or the route finishes. */
#ifndef _02B464_GRADING_H
#define _02B464_GRADING_H

/* Per-run drive state. */
typedef struct {
    /* 0 = not driving, 2 = driving and being graded, 3 = at the stop,
     * 4 = wrapping up, 5 = done. */
    int runPhase_0x00;

    /* Set when a drive ends with points left and every owed stop served. */
    int runPassed_0x04;

    /* Run phase 4's hold, in frames, from 0x1e. Phase 5 follows once it goes
     * negative and the music has finished fading. */
    int driveEndHold_0x08;

    /* The results screen cuts the award tier at 70/80/90 and scores 10 a
     * point. */
    int driverPoints_0x0c;

    /* 100, or 200 on the easiest difficulty outside practice. */
    int driverPointsMax_0x10;

    /* The current segment's timetable slot and the run clock, both frames,
     * drawn as HH:MM:SS. The clock starts 450 frames (15s) before the first
     * slot. Running past the slot costs a point a second and closes the
     * story-event window. In a practice drill without rule bit 1 the pair is
     * inverted: the slot goes to 0 and the clock counts down to a hard
     * TIME_MANAGEMENT failure. */
    int scheduleTime_0x14;
    int runClock_0x18;

    /* Added to runClock_0x18 each frame of the bus-stop scene, so it reaches
     * scheduleTime_0x14 as the scene ends: a fifth of what is left while that
     * is at least 50, then a flat 10. */
    int clockCatchUpStep_0x1c;

    /* 0 = cruising, 1 = departed-previous-stop wait, 2 = approaching
     * (waiting passengers drawn), 3 = stopped/waiting, 4 = finishing. */
    int stopPhase_0x20;

    /* How the approach ended, graded in run phase 3: 0 = pulled up at the
     * marker (graded on heading and turn signal), 2 = drove past the stop
     * segment (INSTR_MISSED_STOP, -20). Phase 3 also handles a 1
     * (INSTR_BAD_STOP_POSITION_1, -10) that nothing in the image writes. */
    int stopArrivalGrade_0x24;

    /* Running minimum distance-to-stop while approaching (stopPhase_0x20 == 2),
     * reset to 9999.0 on arming. */
    float stopMinDistance_0x28;

    /* wrongLaneCount escalates INSTR_WRONG_LANE from -10 to -50 after the
     * first. speedingCountdown makes a sustained overspeed cost points every
     * 120 frames (4s). stopLineGraded allows one stop-line penalty per
     * signal. */
    int wrongLaneCount_0x2c;
    int speedingCountdown_0x30;
    int stopLineGraded_0x34;

    /* [0]/[1] take the current traffic-signal id; [0] is cleared when the
     * signal goes out of range, [1] keeps the last one. [3] counts moving
     * frames since the lane probes last agreed, [4] the lane-straddle
     * penalties so far. [5] latches a turn lane (leaving it cancels the turn
     * signal), [6] a junction (leaving it grades the turn signal). [7] is the
     * signalSide_0x25c that signalHeldFrames_0x58 counts. [2] unclear. */
    int field_0x38[8];

    int signalHeldFrames_0x58;
    int hasMoved_0x5c;
    int startIdleFrames_0x60;

    /* CROSSING_* */
    int crossingState_0x64;
    int crossingStopTimer_0x68;

    /* junctionARoadFlags2_0x358 & 0xf0000001, refreshed only on frames where
     * the A and B road probes agree. */
    int lane_0x6c;

    /* [0] is junctionARoadFlags2_0x358 & 0xf000000, refreshed with lane_0x6c.
     * [1]/[2] are last frame's junctionARoadFlags_0x34c /
     * junctionBRoadFlags_0x368. */
    int field_0x70[3];

    /* Latched when the bus leaves a stop with a drive-mark instruction on
     * screen; spent on a +20 award. */
    int instructionBonusPending_0x7c;

    /* Frames the bus has been moving, reset at a standstill and read nowhere.
     * Its neighbours are all scoring inputs, so likely a dropped rule. */
    int field_0x80;

    /* Rapid acceleration: armed by the first upshift taken with the .r
     * trigger past 0xfe, disarmed when it eases off; holding it past 14 frames
     * docks INSTR_RAPID_ACCEL. */
    int fullThrottleLatch_0x84;

    /* Set on the upshift out of gear 0. */
    int firstUpshift_0x88;
    int fullThrottleFrames_0x8c;

    /* avg = (avg + amount) / 2 over the per-frame brake amount, 0 on any
     * non-braking frame. Past 0.01 docks INSTR_HARD_BRAKE, at most once per
     * 60 frames. */
    float brakeAverage_0x90;
    int hardBrakeCooldown_0x94;

    int swerveCountdown_0x98;
} RunState;
extern RunState var_runState_8c2285c4;

/* Bits 0x2/0x4 grade a wall hit, both set worst. Other bits unclear. */
extern int var_wallHitBits_8c228660;

/* Also resets the run's grading state. The task idles in run phase 0 until
 * another unit sets phase 2. */
void GradeInstall_8c02c46a(void);

/* Whether var_nextStopSegment_8c228710 has reached the last stop the
 * route, or the practice lesson, requires. */
int GradeRunComplete_8c02c586(void);

/* Reloads the segment while a stop is owed and points remain, otherwise
 * returns to the lesson list. */
void GradeOnFadePracticeEnd_8c02c784(void);

#endif // _02B464_GRADING_H
