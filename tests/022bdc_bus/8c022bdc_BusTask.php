<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('f32')) {
    function f32(float $value): float
    {
        return unpack('f', pack('f', $value))[1];
    }
}

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

/*
 * _BusTask_8c022bdc(Task *task, void *state): the player's bus per-frame
 * dispatcher, installed as the run's task action by BusInitStart_8c023610.
 * This test covers the simplest reachable path: driveState_0x2b4==0
 * (boarding), doorState_0x3c0==0 with no doors-open trigger (no state
 * change, no sdMidiPlay), speed_0x27c==0 (skips the BusDriveSampleGround_8c023938/023cba/
 * ground-query block), var_driveMode_8c1bb8c8==0 (skips the steering-
 * smoothing block -- state 0 never reads steering itself), timeOfDay
 * neither DAY nor NIGHT (skips the blinker branch), gear!=5
 * and signalSide_0x25c==0 (sdMidiStop, not sdMidiPlay), and playMode != DEMO
 * (BusCameraUpdate_8c025078 instead of demoUpdateCamera).
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_var_currentCourse_8c1bb868', 0x24);
        $this->setSize('_var_midiHandles_8c0fcd28', 0x20);
        $this->setSize('_var_activeGroundGrid_8c2264d4', 4);
        $this->setSize('_var_wallHitBits_8c228660', 4);
        $this->setSize('_acosf', 4);
        $this->setSize('_var_busDoorFrame_8c227db0', 4);
        $this->setSize('_var_busDoorLastFrame_8c227db4', 4);
        $this->setSize('_var_peripherals_8c1ba35c', 0x28);
        $this->setSize('_BusInputUpdate_8c0246b2', 4);
        $this->setSize('_var_driveCueState_8c2264b8', 0x1c);
        $this->setSize('_var_driveMode_8c1bb8c8', 4);
        $this->setSize('_BusDriveFindLaneTarget_8c023e7e', 4);
        $this->setSize('_BusLineAdvance_8c02412c', 4);
        $this->setSize('_njSin', 4);
        $this->setSize('_njCos', 4);
        $this->setSize('_BusInputCapMirrorTraffic_8c024280', 4);
        $this->setSize('_GeomDistanceXZ_8c02081c', 4);
        $this->setSize('_SndUpdateEngine_8c010c6e', 4);
        $this->setSize('_BusDriveSampleGround_8c023938', 4);
        $this->setSize('_BusDriveApplyGround_8c023cba', 4);
        $this->setSize('_var_timeOfDay_8c18ad20', 4);
        $this->setSize('_var_playerBus_8c1bbd9c', 4);
        $this->setSize('_BusDrawFadeLights_8c028022', 4);
        $this->setSize('_sdMidiStop', 4);
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_VehicleModelPlace_8c020594', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_BusCameraUpdate_8c025078', 4);
        $this->setSize('_CinematicUpdateCamera_8c025906', 4);
        $this->setSize('_BusCameraUpdateMirror_8c025604', 4);
        $this->setSize('_var_runState_8c2285c4', 0x9c);
    }

    public function test_boarding_no_trigger(): void
    {
        $this->resolveSymbols();

        $base = $this->addressOf('_var_busState_8c1bb9d0');
        for ($off = 0; $off < 0x3cc; $off += 4) {
            $this->initUint32($base + $off, 0);
        }

        // driveState_0x2b4/doorState_0x3c0/doorRequest_0x3c4/speed_0x27c/
        // gear_0x2f4/signalSide_0x25c/ang_0x258/posX_0x0f4/posHistory_0x100[0].x
        // are all already 0 from the zero-fill above.

        $acosArg = 0.6;
        $this->initUint32($base + 0x278, fdec($acosArg));

        $course = $this->addressOf('_var_currentCourse_8c1bb868');
        $atariBus = 0xcafe0100;
        $this->initUint32($course + 0x04, $atariBus);

        $midiHandle1 = 0xcafe0900;
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28') + 4, $midiHandle1);

        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 0);
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0);
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_playerBus_8c1bbd9c'), $base);

        $this->call('_BusTask_8c022bdc')->with($this->alloc(4), $this->alloc(4));

        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', $atariBus);
        $this->shouldWriteLongTo('_var_wallHitBits_8c228660', 0);
        $this->shouldWriteLong($base + 0x080, 0);

        $acosResult = 0.8;
        $this->shouldCall('_acosf')->with(f32($acosArg))->andReturn($acosResult);
        $ang = (int)(($acosResult * 65536.0) / 6.283184);
        $this->shouldWriteLong($base + 0x250, $ang);

        // Boarding, substate 0, no doors-open trigger: nothing else in the
        // state-dispatch block.

        $this->shouldCall('_SndUpdateEngine_8c010c6e');

        // speed_0x27c == 0: the BusDriveSampleGround_8c023938/023cba + ground-query block is
        // skipped entirely.

        // var_driveMode_8c1bb8c8 == 0: the steering-smoothing block is
        // skipped entirely.

        $this->shouldWriteLong($base + 0x070, 0);
        $this->shouldWriteLong($base + 0x074, 0);

        // acc_hist_0x280 shift: all zero in, all zero out; pitchAngle_0x078 = 0.
        $this->shouldWriteFloat($base + 0x280, 0.0);
        $this->shouldWriteFloat($base + 0x284, 0.0);
        $this->shouldWriteFloat($base + 0x288, 0.0);
        $this->shouldWriteFloat($base + 0x28c, 0.0);
        $this->shouldWriteLong($base + 0x078, 0);

        $this->shouldWriteLong($base + 0x07c, 0);

        // var_timeOfDay_8c18ad20 == 0: neither the DAY blinker bit nor
        // BusDrawFadeLights_8c028022 runs.

        // gear_0x2f4 != 5, signalSide_0x25c == 0: turnSignalBlinkCounter_0x260 = 0, sdMidiStop.
        $this->shouldWriteLong($base + 0x260, 0);
        $this->shouldCall('_sdMidiStop')->with($midiHandle1);

        $this->shouldCall('_VehicleModelPlace_8c020594')->with(
            ($this->addressOf('_var_busState_8c1bb9d0') + 0x084),
            $base,
        );

        // playMode != PLAY_MODE_DEMO (2): gameplay camera update.
        $this->shouldCall('_BusCameraUpdate_8c025078');

        $this->shouldCall('_BusCameraUpdateMirror_8c025604');
    }
};
