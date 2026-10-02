<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _StopSetup_8c02caba(void): per-run bus-stop setup, called right after
 * the course loads. Clears var_segmentHasStop_8c2286a4 (active-stop flags), flags one
 * segment per candidate story event, then forces a stop at every type-2
 * course segment and randomly flags additional segments (never type-3)
 * until the course's randomized total stop count
 * (courseConfig->[randomStopCountMin_0x14, randomStopCountMax_0x18)) is
 * reached. Finishes by priming var_nextStopSegment_8c228710 /
 * var_prevStopSegment_8c22870c from var_startStopIndex_8c228704, resetting the upcoming
 * stop's state/passengers, and setting a couple of run-scoped
 * timers/thresholds (var_runState_8c2285c4.driverPoints_0x0c/var_runState_8c2285c4.driverPointsMax_0x10,
 * var_runState_8c2285c4.scheduleTime_0x14/var_runState_8c2285c4.runClock_0x18) from progress and play mode.
 */
return new class extends TestCase {
    const PLAY_MODE_NORMAL = 0;
    const PLAY_MODE_PRACTICE = 1;
    const PLAY_MODE_DEMO = 2;

    private function resolveSymbols(): void
    {

        $this->setSize('_var_eventCandidates_8c228520', 0x40);
        $this->setSize('_var_eventCandidateCount_8c228560', 4);
        $this->setSize('_var_routeEvents_8c22851c', 4);

        $this->setSize('_var_currentCourseConfig_8c18ad18', 4);

        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_activeTrafficPreset_8c227e14', 4);
        $this->setSize('_var_activePedPreset_8c22822c', 4);

        $this->setSize('_var_progress_8c1ba1cc', 0xd2);
        $this->setSize('_var_practiceRules_8c226410', 4);
        $this->setSize('_var_runState_8c2285c4', 0x9c);

        $this->setSize('_EventScanCandidates_8c02b03c', 4);
        $this->setSize('_AsqGetRandomInRangeA_8c012178', 4);
        $this->setSize('_ReplayMenuResetDemoCursor_8c016770', 4);
        // _resetStopState_8c02c884, _pickWaitingPassengers_8c02c8ae,
        // _StopAdvance_8c02ccc6, _advanceStopSegment_8c02ccae are same-object --
        // mock with shouldCall() directly, no setSize().
    }

    // CourseSegment is 0x2c bytes; only type_0x00 (offset 0) is read here.
    private function allocSegments(array $types): int
    {
        $base = $this->alloc(0x2c * count($types));
        foreach ($types as $i => $type) {
            $this->initUint16($base + $i * 0x2c, $type);
        }
        return $base;
    }

    public function test_forced_stop_meets_target_no_extra_pick_no_events(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_eventCandidateCount_8c228560'), 0);

        // segments: [0]=forced (type 2), then terminator -- totalSegments=1
        $segments = $this->allocSegments([2, 0]);
        $config = $this->alloc(0x1c);
        $this->initUint32($config + 8, $segments);
        $this->initUint32($config + 0x14, 1); // randomStopCountMin
        $this->initUint32($config + 0x18, 2); // randomStopCountMax
        $this->initUint32($this->addressOf('_var_currentCourseConfig_8c18ad18'), $config);

        $this->initUint32($this->addressOf('_var_startStopIndex_8c228704'), 7);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), self::PLAY_MODE_NORMAL);

        $this->initUint8($this->addressOf('_var_progress_8c1ba1cc') + 0xc4, 0); // < 1
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 0);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x14, 500);

        $this->call('_StopSetup_8c02caba')->with();

        $flags = $this->addressOf('_var_segmentHasStop_8c2286a4');
        for ($i = 0; $i < 96; $i += 4) {
            $this->shouldWriteLong($flags + $i, 0);
        }

        $this->shouldCall('_EventScanCandidates_8c02b03c');

        // Segment scan: seg[0] type==2, not yet active -> force it active.
        $this->shouldWriteLong($flags + 0, 1);

        // range(2-1=1) -> target = 0 + 1 = 1; extraStopCount = 1 - 1 = 0.
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(1)->andReturn(0);

        $this->shouldCall('_ReplayMenuResetDemoCursor_8c016770');

        $this->shouldWriteLongTo('_var_activeTrafficPreset_8c227e14', 0);
        $this->shouldWriteLongTo('_var_activePedPreset_8c22822c', 0);

        $this->shouldWriteLongTo('_var_nextStopSegment_8c228710', 7);
        $this->shouldWriteLongTo('_var_prevStopSegment_8c22870c', 6);

        $this->shouldCall('_resetStopState_8c02c884');
        $this->shouldCall('_pickWaitingPassengers_8c02c8ae');
        $this->shouldCall('_StopAdvance_8c02ccc6');
        $this->shouldCall('_advanceStopSegment_8c02ccae');

        // difficulty_0xc4 < 1 && playMode != DEMO -> 200
        $this->shouldWriteLong($this->addressOf('_var_runState_8c2285c4') + 0x10, 200);
        $this->shouldWriteLong($this->addressOf('_var_runState_8c2285c4') + 0x0c, 200);

        // playMode != PRACTICE -> dc = d8 - 0x1c2
        $this->shouldWriteLong($this->addressOf('_var_runState_8c2285c4') + 0x18, 500 - 0x1c2);
    }

    public function test_practice_mode_single_extra_pick(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_eventCandidateCount_8c228560'), 0);

        // segments: [0]=normal(1), [1]=excluded(3), then terminator -- totalSegments=2.
        // The reroll loop's type check indexes off the *scan's own walking
        // pointer* (left sitting at the terminator, index 2), not the table
        // start -- so a pick of P there actually reads index (2+P). A 4th
        // "ghost" record beyond the terminator makes pick=1 land on a
        // controlled, deterministic value (see StopSetup_8c02caba).
        $segments = $this->allocSegments([1, 3, 0, 3]);
        $config = $this->alloc(0x1c);
        $this->initUint32($config + 8, $segments);
        $this->initUint32($config + 0x14, 1); // randomStopCountMin
        $this->initUint32($config + 0x18, 2); // randomStopCountMax
        $this->initUint32($this->addressOf('_var_currentCourseConfig_8c18ad18'), $config);

        $this->initUint32($this->addressOf('_var_startStopIndex_8c228704'), 3);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), self::PLAY_MODE_PRACTICE);

        $this->initUint8($this->addressOf('_var_progress_8c1ba1cc') + 0xc4, 0);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 0); // bit 2 clear
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x14, 42);

        $this->call('_StopSetup_8c02caba')->with();

        $flags = $this->addressOf('_var_segmentHasStop_8c2286a4');
        for ($i = 0; $i < 96; $i += 4) {
            $this->shouldWriteLong($flags + $i, 0);
        }

        $this->shouldCall('_EventScanCandidates_8c02b03c');

        // No stop forced: activeStopCount = 0, totalSegments = 2.
        // range(2-1=1) -> target = 0 + 1 = 1; extraStopCount = 1 - 0 = 1.
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(1)->andReturn(0);

        // First roll (pick=1) reads the ghost record at index 2+1=3
        // (type 3, excluded) -> rerolls. Second roll (pick=0) reads index
        // 2+0=2, the terminator itself (type 0, not excluded) -> flags
        // var_segmentHasStop_8c2286a4[0] (the *actual* pick value, not the ghost index).
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(2)->andReturn(1);
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(2)->andReturn(0);
        $this->shouldWriteLong($flags + 0, 1);

        $this->shouldCall('_ReplayMenuResetDemoCursor_8c016770');

        $this->shouldWriteLongTo('_var_activeTrafficPreset_8c227e14', 0);
        $this->shouldWriteLongTo('_var_activePedPreset_8c22822c', 0);

        $this->shouldWriteLongTo('_var_nextStopSegment_8c228710', 3);
        $this->shouldWriteLongTo('_var_prevStopSegment_8c22870c', 2);

        $this->shouldCall('_resetStopState_8c02c884');
        $this->shouldCall('_pickWaitingPassengers_8c02c8ae');
        $this->shouldCall('_StopAdvance_8c02ccc6');
        $this->shouldCall('_advanceStopSegment_8c02ccae');

        // difficulty_0xc4 < 1, but playMode == PRACTICE -> 100
        $this->shouldWriteLong($this->addressOf('_var_runState_8c2285c4') + 0x10, 100);
        $this->shouldWriteLong($this->addressOf('_var_runState_8c2285c4') + 0x0c, 100);

        // playMode == PRACTICE && bit 2 clear -> dc = d8 (42), d8 = 0
        $this->shouldWriteLong($this->addressOf('_var_runState_8c2285c4') + 0x18, 42);
        $this->shouldWriteLong($this->addressOf('_var_runState_8c2285c4') + 0x14, 0);
    }

    public function test_demo_mode_skips_demo_entry_clear_and_event_flags_a_segment(): void
    {
        $this->resolveSymbols();

        // One candidate event, pointing at segment 1.
        $this->initUint32($this->addressOf('_var_eventCandidateCount_8c228560'), 1);
        $this->initUint32($this->addressOf('_var_eventCandidates_8c228520'), 4);

        $events = $this->alloc(0x10 * 5);
        $this->initUint16($events + 4 * 0x10 + 2, 1); // event[4].segmentId_0x02 = 1
        $this->initUint32($this->addressOf('_var_routeEvents_8c22851c'), $events);

        // segments: [0]=normal(1), [1]=normal(1), then terminator -- totalSegments=2
        // segment 1 is pre-flagged active by the event scan.
        $segments = $this->allocSegments([1, 1, 0]);
        $config = $this->alloc(0x1c);
        $this->initUint32($config + 8, $segments);
        $this->initUint32($config + 0x14, 1); // randomStopCountMin
        $this->initUint32($config + 0x18, 2); // randomStopCountMax
        $this->initUint32($this->addressOf('_var_currentCourseConfig_8c18ad18'), $config);

        $this->initUint32($this->addressOf('_var_startStopIndex_8c228704'), 0);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), self::PLAY_MODE_DEMO);

        $this->initUint8($this->addressOf('_var_progress_8c1ba1cc') + 0xc4, 0);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 0);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x14, 10);

        $this->call('_StopSetup_8c02caba')->with();

        $flags = $this->addressOf('_var_segmentHasStop_8c2286a4');
        for ($i = 0; $i < 96; $i += 4) {
            $this->shouldWriteLong($flags + $i, 0);
        }

        $this->shouldCall('_EventScanCandidates_8c02b03c');
        $this->shouldWriteLong($flags + 4, 1); // event flags segment 1 active (int-scaled: index 1 * 4)

        // Scan: seg[0] not active, type 1 (not forced) -> no change.
        //       seg[1] already active (from event) -> counted, activeStopCount=1.
        // totalSegments = 2. range(2-1=1) -> target = 0 + 1 = 1; extraStopCount = 1 - 1 = 0.
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(1)->andReturn(0);

        $this->shouldCall('_ReplayMenuResetDemoCursor_8c016770');

        // playMode == DEMO -> demo entry values are NOT cleared.

        $this->shouldWriteLongTo('_var_nextStopSegment_8c228710', 0);
        $this->shouldWriteLongTo('_var_prevStopSegment_8c22870c', -1);

        $this->shouldCall('_resetStopState_8c02c884');
        $this->shouldCall('_pickWaitingPassengers_8c02c8ae');
        $this->shouldCall('_StopAdvance_8c02ccc6');
        $this->shouldCall('_advanceStopSegment_8c02ccae');

        // difficulty_0xc4 < 1 && playMode != PRACTICE (DEMO qualifies) -> 200
        $this->shouldWriteLong($this->addressOf('_var_runState_8c2285c4') + 0x10, 200);
        $this->shouldWriteLong($this->addressOf('_var_runState_8c2285c4') + 0x0c, 200);

        // playMode != PRACTICE -> dc = d8 - 0x1c2
        $this->shouldWriteLong($this->addressOf('_var_runState_8c2285c4') + 0x18, 10 - 0x1c2);
    }

    public function test_practice_mode_with_bit_set_still_takes_the_else_gate_branch(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_eventCandidateCount_8c228560'), 0);

        // segments: [0]=forced (type 2), then terminator -- totalSegments=1
        $segments = $this->allocSegments([2, 0]);
        $config = $this->alloc(0x1c);
        $this->initUint32($config + 8, $segments);
        $this->initUint32($config + 0x14, 1); // randomStopCountMin
        $this->initUint32($config + 0x18, 2); // randomStopCountMax
        $this->initUint32($this->addressOf('_var_currentCourseConfig_8c18ad18'), $config);

        $this->initUint32($this->addressOf('_var_startStopIndex_8c228704'), 5);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), self::PLAY_MODE_PRACTICE);

        $this->initUint8($this->addressOf('_var_progress_8c1ba1cc') + 0xc4, 0);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 2); // bit 2 set
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x14, 900);

        $this->call('_StopSetup_8c02caba')->with();

        $flags = $this->addressOf('_var_segmentHasStop_8c2286a4');
        for ($i = 0; $i < 96; $i += 4) {
            $this->shouldWriteLong($flags + $i, 0);
        }

        $this->shouldCall('_EventScanCandidates_8c02b03c');
        $this->shouldWriteLong($flags + 0, 1);

        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(1)->andReturn(0);

        $this->shouldCall('_ReplayMenuResetDemoCursor_8c016770');

        $this->shouldWriteLongTo('_var_activeTrafficPreset_8c227e14', 0);
        $this->shouldWriteLongTo('_var_activePedPreset_8c22822c', 0);

        $this->shouldWriteLongTo('_var_nextStopSegment_8c228710', 5);
        $this->shouldWriteLongTo('_var_prevStopSegment_8c22870c', 4);

        $this->shouldCall('_resetStopState_8c02c884');
        $this->shouldCall('_pickWaitingPassengers_8c02c8ae');
        $this->shouldCall('_StopAdvance_8c02ccc6');
        $this->shouldCall('_advanceStopSegment_8c02ccae');

        // playMode == PRACTICE -> 100
        $this->shouldWriteLong($this->addressOf('_var_runState_8c2285c4') + 0x10, 100);
        $this->shouldWriteLong($this->addressOf('_var_runState_8c2285c4') + 0x0c, 100);

        // playMode == PRACTICE but bit 2 SET -> still dc = d8 - 0x1c2
        $this->shouldWriteLong($this->addressOf('_var_runState_8c2285c4') + 0x18, 900 - 0x1c2);
    }
};
