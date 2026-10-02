<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_RenderQueueDraw_8c0223ea', 4);
        $this->setSize('_TaskRunGroup_8c014b42', 4);
        $this->setSize('_TaskUninstall_8c014b66', 4);
        $this->setSize('_MessageBoxStart_8c02ad8c', 4);
        $this->setSize('_RouteGetLatch_8c01432a', 4);
        $this->setSize('_StopUninstallGroup_8c02ca96', 4);
        $this->setSize('_njReleaseTexture', 4);
        $this->setSize('_BusCameraRestoreState_8c024b86', 4);
        $this->setSize('_BusCameraApplyMode_8c024f32', 4);
        $this->setSize('_SndPlayAdx_8c010cd6', 4);
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_var_isFading_8c226568', 4);
        $this->setSize('_var_stopTaskGroup_8c2288f8', 4);
        $this->setSize('_var_cutsceneActive_8c1bb900', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_fadeRequest_8c226564', 4);
        $this->setSize('_var_sceneVisible_8c226560', 4);
        $this->setSize('_var_messageBoxActive_8c22847c', 4);
        $this->setSize('_var_runState_8c2285c4', 0x9c);
        $this->setSize('_var_interiorTexlist_8c1bc438', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_var_route_8c18ad1c', 4);
        $this->setSize('_var_currentSegment_8c228708', 4);
        $this->setSize('_var_viewLayout_8c22655c', 4);
        $this->setSize('_var_timeOfDay_8c18ad20', 4);
        $this->setSize('_var_midiHandles_8c0fcd28', 8 * 4);
        $this->setSize('_var_cameraMode_8c227d9c', 4);
        $this->setSize('_var_cameraCueState_8c227da4', 4);
    }

    private function f(float $v): int
    {
        return unpack('L', pack('f', $v))[1];
    }

    private function makeState(int $phase, int $subPhase): int
    {
        $state = $this->alloc(8);
        $this->initUint32($state + 0, $phase);
        $this->initUint32($state + 4, $subPhase);
        return $state;
    }

    /** The three DrawFn registrations every call makes unconditionally. */
    private function expectFrameCallbacks(): void
    {
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(2, $this->addressOf('_drawInterior_8c02d1f4'), 0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(1, $this->addressOf('_StopDrawLightBegin_8c02d0fc'), 0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(2, $this->addressOf('_StopDrawLightBegin_8c02d0fc'), 0);
    }

    private function expectTaskRunGroup(int $group): void
    {
        $this->shouldCall('_TaskRunGroup_8c014b42')->with($group);
    }

    private function expectRegisterFadeOverlay(): void
    {
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(1, $this->addressOf('_StopDrawLightEnd_8c02d146'), 0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(2, $this->addressOf('_StopDrawLightEnd_8c02d146'), 0);
    }

    public function test_case0_still_fading_advances_counter_only(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);
        $group = $this->alloc(0x20);
        $this->initUint32($this->addressOf('_var_stopTaskGroup_8c2288f8'), $group);
        $base = $this->addressOf('_var_runState_8c2285c4');
        $this->initUint32($base + 6 * 4, 5);

        $state = $this->makeState(0, 0);

        $this->call('_PassengerStopSceneTask_8c02d644')->with(0, $state);

        $this->expectFrameCallbacks();
        $this->shouldWriteLong($base + 6 * 4, 6);
        $this->expectTaskRunGroup($group);
        $this->expectRegisterFadeOverlay();
    }

    public function test_case0_not_fading_advances_to_phase1(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $group = $this->alloc(0x20);
        $this->initUint32($this->addressOf('_var_stopTaskGroup_8c2288f8'), $group);
        $base = $this->addressOf('_var_runState_8c2285c4');
        $this->initUint32($base + 6 * 4, 0);

        $state = $this->makeState(0, 0);

        $this->call('_PassengerStopSceneTask_8c02d644')->with(0, $state);

        $this->expectFrameCallbacks();
        $this->shouldWriteLong($state + 0, 1);
        $this->shouldWriteLong($base + 6 * 4, 1);
        $this->expectTaskRunGroup($group);
        $this->expectRegisterFadeOverlay();
    }

    public function test_case1_fading_out_not_done_leaves_playmode_normal_to_fade_request(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0); // NORMAL
        $this->initUint32($this->addressOf('_var_cutsceneActive_8c1bb900'), 1); // cutscene active
        $mat = $this->addressOf('_var_passengerFadeColor_8c228960');
        $this->initUint32($mat, $this->f(1.0)); // fading out from 1.0

        $group = $this->alloc(0x20);
        $this->initUint32($this->addressOf('_var_stopTaskGroup_8c2288f8'), $group);
        $base = $this->addressOf('_var_runState_8c2285c4');
        $this->initUint32($base + 6 * 4, 0);

        $state = $this->makeState(1, 0);

        $this->call('_PassengerStopSceneTask_8c02d644')->with(0, $state);

        $this->expectFrameCallbacks();
        $this->shouldWriteFloat($mat, 1.0 - 0.06666667014360428);
        $this->shouldWriteLongTo('_var_passengersFadedOut_8c22895c', 0);
        $this->shouldWriteLongTo('_var_passengerActed_8c228958', 0);
        $this->expectTaskRunGroup($group);
        // var_passengerActed_8c228958 not touched by the mocked TaskRunGroup -> stays 0 -> "nothing happened" path
        $this->shouldWriteLong($state + 0, 2);
        $this->shouldWriteLongTo('_var_fadeRequest_8c226564', 2); // FADE_REQUEST_OUT
        $this->shouldWriteLong($base + 6 * 4, 1);
        $this->expectRegisterFadeOverlay();
    }

    public function test_case1_fading_out_finishes_sets_flag_and_reruns(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0); // NORMAL
        $this->initUint32($this->addressOf('_var_cutsceneActive_8c1bb900'), 0);
        $mat = $this->addressOf('_var_passengerFadeColor_8c228960');
        $this->initUint32($mat, $this->f(0.05)); // fading out, about to clamp to 0

        $group = $this->alloc(0x20);
        $this->initUint32($this->addressOf('_var_stopTaskGroup_8c2288f8'), $group);
        $base = $this->addressOf('_var_runState_8c2285c4');
        $this->initUint32($base + 6 * 4, 0);

        $state = $this->makeState(1, 0);

        $this->call('_PassengerStopSceneTask_8c02d644')->with(0, $state);

        $this->expectFrameCallbacks();
        $this->shouldWriteFloat($mat, 0.05 - 0.06666667014360428); // unconditional store, before the clamp
        $this->shouldWriteFloat($mat, 0.0);
        $this->shouldWriteLongTo('_var_passengersFadedOut_8c22895c', 1);
        $this->shouldWriteLong($state + 4, 1); // sub-phase advances
        $this->shouldWriteLongTo('_var_passengerActed_8c228958', 0);
        $this->expectTaskRunGroup($group);
        $this->shouldWriteLong($base + 0, 2);
        $this->shouldWriteLong($state + 0, 5);
        $this->shouldCall('_setCountUpStep_8c02d5d8');
        $this->shouldWriteLong($base + 6 * 4, 1);
        $this->expectRegisterFadeOverlay();
    }

    public function test_case2_not_fading_starts_message_box(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $group = $this->alloc(0x20);
        $this->initUint32($this->addressOf('_var_stopTaskGroup_8c2288f8'), $group);

        $state = $this->makeState(2, 0);

        $this->call('_PassengerStopSceneTask_8c02d644')->with(0, $state);

        $this->expectFrameCallbacks();
        $this->shouldWriteLong($state + 0, 3);
        $this->shouldWriteLongTo('_var_fadeRequest_8c226564', 1); // FADE_REQUEST_IN
        $this->shouldWriteLongTo('_var_sceneVisible_8c226560', 0);
        $this->shouldCall('_MessageBoxStart_8c02ad8c');
        $this->expectTaskRunGroup($group);
        $this->expectRegisterFadeOverlay();
    }

    public function test_case3_message_box_closed_advances_to_case4(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_messageBoxActive_8c22847c'), 0);

        $state = $this->makeState(3, 0);

        $this->call('_PassengerStopSceneTask_8c02d644')->with(0, $state);

        $this->expectFrameCallbacks();
        $this->shouldWriteLong($state + 0, 4);
        $this->expectRegisterFadeOverlay();
    }

    public function test_case4_not_ready_does_nothing(): void
    {
        $this->resolveSymbols();

        $group = $this->alloc(0x20);
        $this->initUint32($this->addressOf('_var_stopTaskGroup_8c2288f8'), $group);

        $state = $this->makeState(4, 0);

        $this->call('_PassengerStopSceneTask_8c02d644')->with(0, $state);

        $this->expectFrameCallbacks();
        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(0);
        $this->expectTaskRunGroup($group);
        $this->expectRegisterFadeOverlay();
    }

    public function test_case4_ready_advances_to_case5(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $group = $this->alloc(0x20);
        $this->initUint32($this->addressOf('_var_stopTaskGroup_8c2288f8'), $group);
        $base = $this->addressOf('_var_runState_8c2285c4');
        $this->initUint32($base + 6 * 4, 0);

        $state = $this->makeState(4, 0);

        $this->call('_PassengerStopSceneTask_8c02d644')->with(0, $state);

        $this->expectFrameCallbacks();
        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(1);
        $this->shouldWriteLong($base + 0, 2);
        $this->shouldWriteLong($state + 0, 5);
        $this->shouldCall('_setCountUpStep_8c02d5d8');
        $this->expectTaskRunGroup($group);
        $this->expectRegisterFadeOverlay();
    }

    public function test_case5_still_counting_advances_and_plays_sound(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0); // NORMAL
        $group = $this->alloc(0x20);
        $this->initUint32($this->addressOf('_var_stopTaskGroup_8c2288f8'), $group);

        $base = $this->addressOf('_var_runState_8c2285c4');
        $this->initUint32($base + 5 * 4, 100); // var_runState_8c2285c4.scheduleTime_0x14
        $this->initUint32($base + 6 * 4, 10);  // var_runState_8c2285c4.runClock_0x18
        $this->initUint32($base + 7 * 4, 5);   // var_runState_8c2285c4.clockCatchUpStep_0x1c

        $midi = $this->addressOf('_var_midiHandles_8c0fcd28');
        $this->initUint32($midi, 0x9999);

        $state = $this->makeState(5, 0);

        $this->call('_PassengerStopSceneTask_8c02d644')->with(0, $state);

        $this->expectFrameCallbacks();
        $this->shouldWriteLong($base + 6 * 4, 15);
        $this->shouldCall('_sdMidiPlay')->with(0x9999, 1, 6, 0);
        $this->expectTaskRunGroup($group);
        $this->expectRegisterFadeOverlay();
    }

    public function test_case5_done_tears_down_and_frees_self_practice_mode(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1); // PRACTICE

        $base = $this->addressOf('_var_runState_8c2285c4');
        $this->initUint32($base + 5 * 4, 10);
        $this->initUint32($base + 6 * 4, 10); // clock has reached the schedule -> teardown

        $tlist = 0xabcdef;
        $this->initUint32($this->addressOf('_var_interiorTexlist_8c1bc438'), $tlist);

        $task = $this->alloc(0x20);
        $state = $this->makeState(5, 0);

        $this->call('_PassengerStopSceneTask_8c02d644')->with($task, $state);

        $this->expectFrameCallbacks();
        $this->shouldCall('_StopUninstallGroup_8c02ca96');
        $this->shouldCall('_njReleaseTexture')->with($tlist);
        $this->shouldWriteLongTo('_var_cameraMode_8c227d9c', 2);
        $this->shouldWriteLongTo('_var_cameraCueState_8c227da4', 0);
        $this->shouldCall('_BusCameraApplyMode_8c024f32');
        $busState = $this->addressOf('_var_busState_8c1bb9d0');
        $this->shouldWriteLong($busState + 0x2b4, 1);
        $this->shouldWriteLong($busState + 0x25c, 2); // playMode==PRACTICE -> always 2
        $this->shouldWriteLongTo('_var_viewLayout_8c22655c', 2);
        $this->shouldCall('_TaskUninstall_8c014b66')->with($task);
    }

    public function test_case5_done_tears_down_normal_mode_ome_segment0(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0); // NORMAL
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 2); // ROUTE_OME
        $this->initUint32($this->addressOf('_var_currentSegment_8c228708'), 0);
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 1);

        $base = $this->addressOf('_var_runState_8c2285c4');
        $this->initUint32($base + 5 * 4, 10);
        $this->initUint32($base + 6 * 4, 10);

        $tlist = 0x111222;
        $this->initUint32($this->addressOf('_var_interiorTexlist_8c1bc438'), $tlist);

        $task = $this->alloc(0x20);
        $state = $this->makeState(5, 0);

        $this->call('_PassengerStopSceneTask_8c02d644')->with($task, $state);

        $this->expectFrameCallbacks();
        $this->shouldCall('_StopUninstallGroup_8c02ca96');
        $this->shouldCall('_njReleaseTexture')->with($tlist);
        $this->shouldCall('_BusCameraRestoreState_8c024b86'); // not DEMO, not PRACTICE
        $busState = $this->addressOf('_var_busState_8c1bb9d0');
        $this->shouldWriteLong($busState + 0x2b4, 1);
        $this->shouldWriteLong($busState + 0x25c, 1); // !PRACTICE && OME && segment==0
        $this->shouldWriteLongTo('_var_viewLayout_8c22655c', 2);
        $this->shouldCall('_SndPlayAdx_8c010cd6')->with(0, 1 + 5); // OME offset = 5
        $this->shouldCall('_TaskUninstall_8c014b66')->with($task);
    }

    public function test_default_phase_registers_overlay_only(): void
    {
        $this->resolveSymbols();

        $state = $this->makeState(42, 0);

        $this->call('_PassengerStopSceneTask_8c02d644')->with(0, $state);

        $this->expectFrameCallbacks();
        $this->expectRegisterFadeOverlay();
    }
};
