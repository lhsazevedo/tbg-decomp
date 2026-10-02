<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _driveCueTask_8c020214(Task*, void*): ambient driving-cue task, see
 * 020214_drive_cue.h.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_TaskKill_8c014b66', 4);
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_AsqGetRandomInRangeB_8c0121be', 4);
        $this->setSize('_AsqGetRandomB_8c0121a8', 4);
        $this->setSize('_VibStart_8c010f7a', 4);
        $this->setSize('_VibUpdate_8c010fae', 4);
        $this->setSize('_SndPlayAdx_8c010cd6', 4);
        $this->setSize('_var_runState_8c2285c4', 0x9c);
        $this->setSize('_var_midiHandles_8c0fcd28', 0x20);
        $this->setSize('_var_vibport_8c1ba354', 4);
        $this->setSize('_var_progress_8c1ba1cc', 0xe8);
        $this->setSize('_var_cameraMode_8c227d9c', 4);
        $this->setSize('_var_prevStopSegment_8c22870c', 4);
        $this->setSize('_var_nextStopSegment_8c228710', 4);
        $this->setSize('_var_route_8c18ad1c', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_practiceRules_8c226410', 4);
        $this->setSize('_var_practiceLesson_8c22640c', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
    }

    private function struct(): int
    {
        return $this->addressOf('_var_driveCueState_8c2264b8');
    }

    /** Sets up the struct's fields (ambientChimeState_0x00 and stopAnnounceState_0x08's dispatch
     * values default to something inert so a test can focus on one switch). */
    private function initStruct(int $f00, int $f04, int $f08, int $f0c, int $f10, int $f14, int $f18): void
    {
        $s = $this->struct();
        $this->initUint32($s + 0x00, $f00);
        $this->initUint32($s + 0x04, $f04);
        $this->initUint32($s + 0x08, $f08);
        $this->initUint32($s + 0x0c, $f0c);
        $this->initUint32($s + 0x10, $f10);
        $this->initUint32($s + 0x14, $f14);
        $this->initUint32($s + 0x18, $f18);
    }

    private function initInactiveWorld(): void
    {
        // run phase < 3 (task stays alive)
        $this->initUint32($this->addressOf('_var_runState_8c2285c4'), 0);
        // No vibration cue at the end unless a test wants one.
        $this->initUint32($this->addressOf('_var_vibport_8c1ba354'), 0xffffffff);
        $this->initUint8($this->addressOf('_var_progress_8c1ba1cc') + 0xc7, 0);
        // Mirror-view marker chime off by default.
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 0);
        $this->initUint32($this->addressOf('_var_prevStopSegment_8c22870c'), 0);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 0); // ROUTE_SHINJUKU
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0); // PLAY_MODE_NORMAL
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 0);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 0);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x20, 0);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x27c), 0);

        $midi = $this->addressOf('_var_midiHandles_8c0fcd28');
        for ($i = 0; $i < 8; $i++) {
            $this->initUint32($midi + $i * 4, 0);
        }
    }

    // ------------------------------------------------------------------
    // Early exit: drive has reached its ending phase.
    // ------------------------------------------------------------------

    public function test_frees_task_when_drive_ending(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_runState_8c2285c4'), 3);

        $task = $this->alloc(4);

        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldCall('_TaskKill_8c014b66')->with($task);
    }

    // ------------------------------------------------------------------
    // ambientChimeState_0x00 == 0: idle/first-chime state.
    // ------------------------------------------------------------------

    /**
     * trafficChimeArmed_0x18 != 0 -- armed by TrafficDriveVehicleTask_8c025b98 when
     * a CPU vehicle sits stopped at a junction, and cleared again below
     * every call.
     */
    public function test_state0_plays_first_chime_when_trafficChimeArmed_0x18_set(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initStruct(0, 0, 4 /* dead switch2 case */, 0, 0, 0, 1 /* trafficChimeArmed_0x18 != 0 */);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldCall('_AsqGetRandomInRangeB_8c0121be')->with(6)->andReturn(2);
        $this->shouldCall('_sdMidiPlay')->with(0, 1, 11, 0); // midiHandles[3] == 0 (zeroed alloc)
        $this->shouldCall('_AsqGetRandomInRangeB_8c0121be')->with(3)->andReturn(4);
        $this->shouldWriteLong($this->struct() + 0x04, (4 + 2) * 30);
        $this->shouldWriteLong($this->struct() + 0x00, 1);

        $this->shouldWriteLong($this->struct() + 0x18, 0);

        // var_cameraMode_8c227d9c stays 0 (< 2): segmentChimeLatch_0x14 gets reset to 0.
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    public function test_state0_idles_below_speed_threshold(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initStruct(0, 0, 4, 1 /* latch set */, 0, 0, 0);
        // var_8c1bbc4c already 0.0, below the ~0.0926 threshold.

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    public function test_state0_idles_while_countdown_still_positive(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x27c), $this->f32(1.0)); // above threshold
        $this->initStruct(0, 0, 4, 1, 0, 0, 0);
        $this->initUint32($this->struct() + 0x04, 5); // ambientChimeTimer_0x04: still > 0 after decrement

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x04, 4); // ambientChimeTimer_0x04 decrement, 5 -> 4

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    public function test_state0_countdown_expires_even_random(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x27c), $this->f32(1.0));
        $this->initStruct(0, 0, 4, 1, 0, 0, 0);
        $this->initUint32($this->struct() + 0x04, 0); // decrements to -1, expires

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x04, -1); // ambientChimeTimer_0x04 decrement, 0 -> -1

        $this->shouldCall('_AsqGetRandomB_8c0121a8')->andReturn(4); // even
        $this->shouldCall('_sdMidiPlay')->with(0, 1, 0x3c, 0);
        $this->shouldCall('_VibStart_8c010f7a')->with(2);
        $this->shouldWriteLong($this->struct() + 0x04, 60);
        $this->shouldWriteLong($this->struct() + 0x00, 1);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    public function test_state0_countdown_expires_odd_random(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x27c), $this->f32(1.0));
        $this->initStruct(0, 0, 4, 1, 0, 0, 0);
        $this->initUint32($this->struct() + 0x04, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x04, -1); // ambientChimeTimer_0x04 decrement, 0 -> -1

        $this->shouldCall('_AsqGetRandomB_8c0121a8')->andReturn(5); // odd
        $this->shouldCall('_sdMidiPlay')->with(0, 1, 0x3b, 0);
        $this->shouldCall('_VibStart_8c010f7a')->with(1);
        $this->shouldWriteLong($this->struct() + 0x04, 60);
        $this->shouldWriteLong($this->struct() + 0x00, 1);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    // ------------------------------------------------------------------
    // ambientChimeState_0x00 == 1: periodic-chime cooldown.
    // ------------------------------------------------------------------

    public function test_state1_counts_down(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initStruct(1, 5, 4, 0, 0, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x04, 4);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    public function test_state1_expires_and_resets(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initStruct(1, 0, 4, 0, 0, 1, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x04, -1);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
        $this->shouldCall('_AsqGetRandomInRangeB_8c0121be')->with(300)->andReturn(7);
        $this->shouldWriteLong($this->struct() + 0x04, 7 + 150);
        $this->shouldWriteLong($this->struct() + 0x00, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        // segmentChimeLatch_0x14 was just reset to 0 above by this very case, and stays
        // 0 (var_cameraMode_8c227d9c still 0 by default).
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    // ------------------------------------------------------------------
    // stopAnnounceState_0x08 == 0: stop-approach jingle, first chime.
    // ------------------------------------------------------------------

    public function test_switch2_state0_skips_when_latch_unset(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initStruct(4 /* dead switch1 case */, 0, 0, 0, 0, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        // nearFlag took on announceRequested_0x0c's value (0) here, and var_cameraMode_8c227d9c<2
        // resets it to 0 too either way.
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    public function test_switch2_state0_plays_shinjuku_jingle(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initStruct(4, 0, 0, 1, 0, 0, 0);
        // ROUTE_SHINJUKU (0) already set by initInactiveWorld.

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldCall('_sdMidiPlay')->with(0, 1, 0x22, 0);
        $this->shouldWriteLong($this->struct() + 0x08, 1);
        $this->shouldWriteLong($this->struct() + 0x10, 0);

        // nearFlag = midiHandles[2] (0, the zeroed alloc) -- var_cameraMode_8c227d9c<2
        // resets it to 0 either way, so this doesn't distinguish the two,
        // but confirms the segmentChimeLatch_0x14 write still happens.
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    public function test_switch2_state0_plays_wangan_jingle(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 1); // ROUTE_WANGAN
        $this->initStruct(4, 0, 0, 1, 0, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldCall('_sdMidiPlay')->with(0, 1, 0x23, 0);
        $this->shouldWriteLong($this->struct() + 0x08, 1);
        $this->shouldWriteLong($this->struct() + 0x10, 0);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    public function test_switch2_state0_skips_inner_in_practice_mode(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1); // PLAY_MODE_PRACTICE
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 0); // bit0 unset
        $this->initStruct(4, 0, 0, 1, 0, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        // No sdMidiPlay/stopAnnounceState_0x08 write -- inner block skipped. nearFlag
        // stays announceRequested_0x0c's value (1), so var_cameraMode_8c227d9c<2 still forces the
        // final flag to 0 for this test.
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    // ------------------------------------------------------------------
    // stopAnnounceState_0x08 == 1: waiting for the stop-crossing SndProc cue.
    // ------------------------------------------------------------------

    public function test_switch2_state1_practice_mode_ordinary_course(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1); // PLAY_MODE_PRACTICE
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 3); // neither 8 nor 9 -> local0 = 0
        $this->initUint32($this->addressOf('_var_prevStopSegment_8c22870c'), 5);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 9);
        $this->initStruct(4, 0, 1, 0, 61, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldWriteLong($this->struct() + 0x10, 62);
        $this->shouldCall('_SndPlayAdx_8c010cd6')->with(1, 5 + 0x28); // ROUTE_SHINJUKU
        $this->shouldWriteLong($this->struct() + 0x08, 3);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    public function test_switch2_state1_waits_for_timer(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initStruct(4, 0, 1, 0, 10, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldWriteLong($this->struct() + 0x10, 11);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    public function test_switch2_state1_fires_wangan_and_reaches_stop(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 1); // ROUTE_WANGAN
        $this->initUint32($this->addressOf('_var_prevStopSegment_8c22870c'), 5);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 5); // same: reached
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x20, 0); // -> stopAnnounceTimer_0x10=30
        $this->initStruct(4, 0, 1, 0, 61, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldWriteLong($this->struct() + 0x10, 62);
        $this->shouldCall('_SndPlayAdx_8c010cd6')->with(1, 5 + 0x3f);
        $this->shouldWriteLong($this->struct() + 0x08, 2);
        $this->shouldWriteLong($this->struct() + 0x10, 30);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    public function test_switch2_state1_reaches_stop_with_mirror_active(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 1); // ROUTE_WANGAN
        $this->initUint32($this->addressOf('_var_prevStopSegment_8c22870c'), 5);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 5); // reached
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x20, 1); // -> random hold
        $this->initStruct(4, 0, 1, 0, 61, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldWriteLong($this->struct() + 0x10, 62);
        $this->shouldCall('_SndPlayAdx_8c010cd6')->with(1, 5 + 0x3f);
        $this->shouldWriteLong($this->struct() + 0x08, 2);
        $this->shouldCall('_AsqGetRandomInRangeB_8c0121be')->with(60)->andReturn(9);
        $this->shouldWriteLong($this->struct() + 0x10, 9 + 120);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    public function test_switch2_state1_fires_shinjuku_and_continues(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        // ROUTE_SHINJUKU (0) already set.
        $this->initUint32($this->addressOf('_var_prevStopSegment_8c22870c'), 5);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 9); // different: not yet reached
        $this->initStruct(4, 0, 1, 0, 61, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldWriteLong($this->struct() + 0x10, 62);
        $this->shouldCall('_SndPlayAdx_8c010cd6')->with(1, 5 + 0x28);
        $this->shouldWriteLong($this->struct() + 0x08, 3);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    public function test_switch2_state1_ome_practice_special_course_9(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 2); // ROUTE_OME
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1); // PLAY_MODE_PRACTICE
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 9); // -> local0 = 10
        $this->initUint32($this->addressOf('_var_prevStopSegment_8c22870c'), 5);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 9);
        $this->initStruct(4, 0, 1, 0, 61, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldWriteLong($this->struct() + 0x10, 62);
        $this->shouldCall('_SndPlayAdx_8c010cd6')->with(1, 10 + 5 + 0x11);
        $this->shouldWriteLong($this->struct() + 0x08, 3);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    public function test_switch2_state1_ome_practice_special_course_8(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 2); // ROUTE_OME
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1); // PLAY_MODE_PRACTICE
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 8); // -> local0 = 6
        $this->initUint32($this->addressOf('_var_prevStopSegment_8c22870c'), 5);
        $this->initUint32($this->addressOf('_var_nextStopSegment_8c228710'), 9);
        $this->initStruct(4, 0, 1, 0, 61, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldWriteLong($this->struct() + 0x10, 62);
        $this->shouldCall('_SndPlayAdx_8c010cd6')->with(1, 6 + 5 + 0x11);
        $this->shouldWriteLong($this->struct() + 0x08, 3);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    // ------------------------------------------------------------------
    // stopAnnounceState_0x08 == 2: waiting to advance to state 3.
    // ------------------------------------------------------------------

    public function test_switch2_state2_waits_for_timer(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initStruct(4, 0, 2, 0, 5, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldWriteLong($this->struct() + 0x10, 4);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    public function test_switch2_state2_expires_and_advances(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initStruct(4, 0, 2, 0, 0, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldWriteLong($this->struct() + 0x10, -1);
        $this->shouldCall('_sdMidiPlay')->with(0, 1, 0x1a, 0);
        $this->shouldWriteLong($this->struct() + 0x08, 3);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    // ------------------------------------------------------------------
    // stopAnnounceState_0x08 == 3: no-op while the A-press latch (announceRequested_0x0c) is still
    // set; restarts the jingle sequence once it's been reset elsewhere.
    // ------------------------------------------------------------------

    public function test_switch2_state3_is_a_noop_while_latched(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initStruct(4, 0, 3, 1 /* announceRequested_0x0c set */, 0, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        // stopAnnounceState_0x08 stays 3 (no write). Marker chime gate forced to 0
        // (var_cameraMode_8c227d9c < 2).
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    public function test_switch2_state3_restarts_when_unlatched(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initStruct(4, 0, 3, 0 /* announceRequested_0x0c unset */, 0, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldWriteLong($this->struct() + 0x08, 0);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    // ------------------------------------------------------------------
    // Final block: near-stop-marker chime, gated by var_cameraMode_8c227d9c.
    // ------------------------------------------------------------------

    public function test_marker_chime_fires_when_mirror_active_and_unlatched(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 2);
        // ROUTE_SHINJUKU (default) + a matching prevStopSegment.
        $this->initUint32($this->addressOf('_var_prevStopSegment_8c22870c'), 5);
        $this->initStruct(4, 0, 4, 1, 0, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldCall('_AsqGetRandomB_8c0121a8')->andReturn(4); // even
        $this->shouldCall('_sdMidiPlay')->with(0, 1, 0x12, 0);
        $this->shouldWriteLong($this->struct() + 0x14, 1);
    }

    public function test_marker_chime_stays_latched(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 2);
        $this->initUint32($this->addressOf('_var_prevStopSegment_8c22870c'), 5); // matches
        $this->initStruct(4, 0, 4, 1, 0, 1 /* already latched */, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        // No sdMidiPlay -- segmentChimeLatch_0x14 already 1 and nearFlag matched, so
        // it's left alone (no re-write either way).
    }

    public function test_marker_chime_fires_on_wangan_match(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 2);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 1); // ROUTE_WANGAN
        $this->initUint32($this->addressOf('_var_prevStopSegment_8c22870c'), 11); // matches
        $this->initStruct(4, 0, 4, 1, 0, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldCall('_AsqGetRandomB_8c0121a8')->andReturn(5); // odd
        $this->shouldCall('_sdMidiPlay')->with(0, 1, 0x11, 0);
        $this->shouldWriteLong($this->struct() + 0x14, 1);
    }

    public function test_marker_chime_resets_latch_when_not_near(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 2);
        // ROUTE_SHINJUKU, prevStopSegment not in the matching list.
        $this->initStruct(4, 0, 4, 1, 0, 1 /* previously latched */, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        // Not near any listed segment: latch resets to 0, no chime.
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    // ------------------------------------------------------------------
    // VibStop gating.
    // ------------------------------------------------------------------

    public function test_vibupdate_ticked_when_vibration_on(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initUint32($this->addressOf('_var_vibport_8c1ba354'), 7);
        $this->initUint8($this->addressOf('_var_progress_8c1ba1cc') + 0xc7, 0);
        $this->initStruct(4, 0, 4, 0, 0, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
        $this->shouldCall('_VibUpdate_8c010fae')->with(7);
    }

    public function test_vibupdate_skipped_when_port_unset(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        // vibport already -1 from initInactiveWorld.
        $this->initStruct(4, 0, 4, 0, 0, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    public function test_vibupdate_skipped_when_vibration_off(): void
    {
        $this->resolveSymbols();
        $this->initInactiveWorld();
        $this->initUint32($this->addressOf('_var_vibport_8c1ba354'), 7);
        $this->initUint8($this->addressOf('_var_progress_8c1ba1cc') + 0xc7, 1);
        $this->initStruct(4, 0, 4, 0, 0, 0, 0);

        $task = $this->alloc(4);
        $this->call('_driveCueTask_8c020214')->with($task, 0);

        $this->shouldWriteLong($this->struct() + 0x18, 0);
        $this->shouldWriteLong($this->struct() + 0x14, 0);
    }

    /** PHP double -> SH4 float32 round-trip. */
    private function f32(float $v): int
    {
        return unpack('L', pack('f', $v))[1];
    }
};
