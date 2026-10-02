<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): int
    {
        $base = $this->addressOf('_var_runState_8c2285c4');

        $this->setSize('_var_busState_8c1bb9d0', 0x400);
        $busState = $this->addressOf('_var_busState_8c1bb9d0');
        $this->setSize('_var_driveMode_8c1bb8c8', 4);
        $this->setSize('_var_playerBus_8c1bbd9c', 4); // BusState*, allocated via alloc()
        $this->setSize('_var_peripherals_8c1ba35c', 0x1c);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_practiceRules_8c226410', 4);
        $this->setSize('_var_driveCueState_8c2264b8', 0x1c);
        $this->setSize('_var_hudState_8c22643c', 0x3c);
        $this->setSize('_StopUpdateArrival_8c02ce48', 4);
        $this->setSize('_VibStart_8c010f7a', 4);

        return $base;
    }

    private function initFloat(int $address, float $value): void
    {
        $raw = unpack('L', pack('f', $value))[1];
        $this->initUint32($address, $raw);
    }

    // "Everything idle/off" baseline so only the behavior under test fires.
    private function baseline(int $base): int
    {
        $busState = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($busState + 0x25c, 0);
        $this->initUint32($busState + 0x268, 0);
        $this->initUint32($busState + 0x34c, 0);
        $this->initUint32($busState + 0x384, 0);
        $this->initUint32($busState + 0x2b4, 0); // bus_state, not "driving" (!=1)
        $this->initFloat($busState + 0x27c, 0.0); // speed
        $this->initUint32($busState + 0x258, 0); // ang
        $this->initUint32($busState + 0x3c0, 0); // bus_substate
        $this->initUint32($busState + 0x3c4, 0);

        $this->initUint32($base + 0x58, 0); // var_runState_8c2285c4.signalHeldFrames_0x58
        $this->initUint32($base + 0x5c, 1); // var_runState_8c2285c4.hasMoved_0x5c armed -- skip idle timer branch
        $this->initUint32($base + 0x60, 0); // var_runState_8c2285c4.startIdleFrames_0x60
        $this->initUint32($base + 0x64, 0); // var_runState_8c2285c4.crossingState_0x64
        $this->initUint32($base + 0x68, 0); // var_runState_8c2285c4.crossingStopTimer_0x68
        $this->initUint32($base + 0x6c, 0); // var_runState_8c2285c4.lane_0x6c
        $this->initUint32($this->addressOf('_var_prevLane_8c228684'), 0);

        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 0.0);
        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 1); // skip speed*ang check

        $busPtr = $this->alloc(0x2b8);
        $this->initUint32($busPtr + 0x25c, 0);
        $this->initUint32($busPtr + 0x268, 0);
        $this->initUint32($this->addressOf('_var_playerBus_8c1bbd9c'), $busPtr);

        $this->initUint32($base + 0x84, 0); // var_runState_8c2285c4.fullThrottleLatch_0x84
        $this->initUint32($base + 0x88, 0); // var_runState_8c2285c4.firstUpshift_0x88, the "armed" flag
        $this->initUint32($base + 0x8c, 0); // var_runState_8c2285c4.fullThrottleFrames_0x8c
        $this->initUint32($base + 0x94, 0); // var_runState_8c2285c4.hardBrakeCooldown_0x94
        $this->initFloat($base + 0x90, 0.0); // var_runState_8c2285c4.brakeAverage_0x90
        $this->initUint32($base + 0x98, 0); // var_runState_8c2285c4.swerveCountdown_0x98
        $this->initUint32($base + 0x7c, 0); // var_runState_8c2285c4.instructionBonusPending_0x7c

        $this->initUint16($this->addressOf('_var_peripherals_8c1ba35c') + 0x18, 0);

        $this->initUint32($base + 0x54, 0); // var_runState_8c2285c4.field_0x38[7]
        $this->initUint32($base + 0x4c, 0); // var_runState_8c2285c4.field_0x38[5]

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 0);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x20, 1);
        $this->initUint32(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 0);

        $this->initUint32($base + 0x18, 0); // var_runState_8c2285c4.runClock_0x18
        $this->initUint32($base + 0x14, 0); // var_runState_8c2285c4.scheduleTime_0x14
        $this->initUint32($this->addressOf('_var_hudState_8c22643c') + 0x14, -1);

        return $busPtr;
    }

    public function test_lane_signal_reset_when_lane_delta_negative_and_left_signal_on(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x6c, 4); // var_runState_8c2285c4.lane_0x6c
        $this->initUint32($this->addressOf('_var_prevLane_8c228684'), 10); // laneDelta = 4-10 < 0
        $this->initUint32($busPtr + 0x25c, 1); // left signal on

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($busPtr + 0x25c, 0);
        $this->shouldWriteLong($busPtr + 0x268, 0);
        $this->shouldWriteLong($base + 0x68, 0); // var_runState_8c2285c4.crossingStopTimer_0x68, reset
        $this->shouldWriteLong($base + 0x58, 1); // var_runState_8c2285c4.signalHeldFrames_0x58, signalSide_0x25c-unchanged counter

        $this->shouldCall('_StopUpdateArrival_8c02ce48');

        $this->shouldCall('_adjustPoints_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1); // var_runState_8c2285c4.runClock_0x18 incremented
    }

    public function test_lane_signal_untouched_when_delta_negative_and_right_signal_on(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x6c, 4);
        $this->initUint32($this->addressOf('_var_prevLane_8c228684'), 10); // laneDelta < 0
        $this->initUint32($busPtr + 0x25c, 2); // right signal on -- mismatched side, no reset

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0); // var_runState_8c2285c4.crossingStopTimer_0x68, reset
        $this->shouldWriteLong($base + 0x58, 1); // var_runState_8c2285c4.signalHeldFrames_0x58, signalSide_0x25c-unchanged counter

        $this->shouldCall('_StopUpdateArrival_8c02ce48');

        $this->shouldCall('_adjustPoints_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1); // var_runState_8c2285c4.runClock_0x18 incremented
    }

    // Crossing idle -> armed on entering the approach -> stopped, same frame
    public function test_crossing_stop_in_approach(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x64, 1); // var_runState_8c2285c4.crossingState_0x64 = 1
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x34c, 0x10000000); // crossing approach

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x64, 2); // armed
        $this->shouldWriteLong($base + 0x64, 5); // stopped
        $this->shouldWriteLong($base + 0x68, 0); // var_runState_8c2285c4.crossingStopTimer_0x68 reset

        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_StopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Armed, then left the approach without stopping: -30, back to idle.
    public function test_crossing_left_approach_without_stopping(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x64, 2); // var_runState_8c2285c4.crossingState_0x64 = 2

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldCall('_adjustPoints_8c02b464')->with(0x14, 0xffffffe2); // -30
        $this->shouldWriteLong($base + 0x64, 1); // idle
        $this->shouldWriteLong($base + 0x68, 0); // var_runState_8c2285c4.crossingStopTimer_0x68 reset

        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_StopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Armed and still rolling through the approach: no state change, stop
    // timer only reset.
    public function test_crossing_armed_while_moving(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x64, 2); // var_runState_8c2285c4.crossingState_0x64 = 2
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x34c, 0x10000000); // crossing approach
        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 1.0); // moving

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0); // var_runState_8c2285c4.crossingStopTimer_0x68 reset (skipHoldTimer path)

        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_StopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Stopped, then out of the approach: back to idle.
    public function test_crossing_stopped_reverts_to_idle(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x64, 5); // var_runState_8c2285c4.crossingState_0x64 = 5

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x64, 1); // idle
        $this->shouldWriteLong($base + 0x68, 0); // var_runState_8c2285c4.crossingStopTimer_0x68 reset

        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_StopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Stationary on the tracks: crossingStopTimer_0x68 underflows, -50, and
    // the timer reloads to 0x78.
    public function test_crossing_stopped_on_tracks(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x34c, 0x20000000); // crossing tracks
        $this->initUint32($base + 0x68, 0); // var_runState_8c2285c4.crossingStopTimer_0x68 decrements to -1

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, -1);
        $this->shouldWriteLong($base + 0x68, 0x78);
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x15, 0xffffffce); // -50

        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_StopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Right-trigger (accelerator) armed and held past the threshold: penalty
    // and the latch is disarmed.
    public function test_right_trigger_held_too_long_applies_penalty(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x84, 1); // var_runState_8c2285c4.fullThrottleLatch_0x84 already armed
        $this->initUint32($base + 0x8c, 0xe); // var_runState_8c2285c4.fullThrottleFrames_0x8c, one away from tripping
        $this->initUint16($this->addressOf('_var_peripherals_8c1ba35c') + 0x18, 0xff); // held at max

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);

        $this->shouldWriteLong($base + 0x8c, 0xf);
        $this->shouldWriteLong($base + 0x84, 0);
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x17, 0xfffffffb); // -5

        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_StopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Right-trigger newly armed (var_runState_8c2285c4.firstUpshift_0x88 set elsewhere) and already
    // held past the threshold: arms the latch immediately.
    public function test_right_trigger_newly_armed_and_already_held(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x88, 1); // var_runState_8c2285c4.firstUpshift_0x88, armed
        $this->initUint16($this->addressOf('_var_peripherals_8c1ba35c') + 0x18, 0xff); // held at max

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);

        $this->shouldWriteLong($base + 0x84, 1); // var_runState_8c2285c4.fullThrottleLatch_0x84 armed
        $this->shouldWriteLong($base + 0x8c, 0); // var_runState_8c2285c4.fullThrottleFrames_0x8c reset
        $this->shouldWriteLong($base + 0x88, 0); // var_runState_8c2285c4.firstUpshift_0x88 cleared

        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_StopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Brake average past threshold and its cooldown expired: penalty, average
    // reset, and vibration.
    public function test_float_timer_expiry_applies_penalty_and_vibrates(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initFloat($base + 0x90, 0.5); // var_runState_8c2285c4.brakeAverage_0x90 > 0.01
        $this->initUint32($base + 0x94, 0); // var_runState_8c2285c4.hardBrakeCooldown_0x94 already expired

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);

        $this->shouldCall('_adjustPoints_8c02b464')->with(0x18, 0xfffffffb); // -5
        $this->shouldWriteFloat($base + 0x90, 0.0);
        $this->shouldWriteLong($base + 0x94, 0x3c);
        $this->shouldCall('_VibStart_8c010f7a')->with(3);

        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_StopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Sharp/violent turn (speed * steering angle out of range) while driving:
    // penalty once the counter expires.
    public function test_sharp_turn_expiry_applies_penalty(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 0);
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x2b4, 1); // driving
        $this->initFloat($this->addressOf('_var_busState_8c1bb9d0') + 0x27c, 100.0); // speed
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x258, 100); // ang
        $this->initUint32($base + 0x98, 0); // var_runState_8c2285c4.swerveCountdown_0x98 decrements to -1

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);

        $this->shouldWriteLong($base + 0x98, -1);
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x19, 0xfffffffb); // -5
        $this->shouldWriteLong($base + 0x98, 0x3c);

        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_StopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Idle timeout: bus stopped (speed 0.0) for too many frames in a row.
    public function test_idle_timeout_applies_penalty(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x5c, 0); // var_runState_8c2285c4.hasMoved_0x5c not yet armed
        $this->initUint32($base + 0x60, 0x709); // var_runState_8c2285c4.startIdleFrames_0x60 one away from tripping

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);
        $this->shouldWriteLong($base + 0x58, 1);

        $this->shouldWriteLong($base + 0x60, 0x70a); // var_runState_8c2285c4.startIdleFrames_0x60 incremented past threshold
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x13, 0xffffffb0); // -80
        $this->shouldWriteLong($base + 0x60, 0);

        $this->shouldCall('_StopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Idle timeout armed once the bus first moves (var_runState_8c2285c4.hasMoved_0x5c latches).
    public function test_idle_timeout_arms_on_first_movement(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x5c, 0); // var_runState_8c2285c4.hasMoved_0x5c not yet armed
        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 1.0); // moving

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);
        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldWriteLong($base + 0x5c, 1);

        $this->shouldCall('_StopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);
        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Message-box just closed (var_runState_8c2285c4.instructionBonusPending_0x7c is set): a small positive award.
    public function test_messagebox_closed_awards_points(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x7c, 1); // var_runState_8c2285c4.instructionBonusPending_0x7c
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x20, 0); // route into the if-branch

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);
        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_StopUpdateArrival_8c02ce48');

        $this->shouldCall('_adjustPoints_8c02b464')->with(0x1e, 0x14); // +20
        $this->shouldWriteLong($base + 0x7c, 0);

        $this->shouldWriteLong($base + 0x18, 1);
    }

    // Bus stopped in the wrong substate while still moving: flat penalty.
    public function test_wrong_substate_while_moving_applies_penalty(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x20, 0); // route messagebox branch away
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x3c0, 2); // bus_substate
        $this->initFloat($this->addressOf('_var_busState_8c1bb9d0') + 0x27c, 1.0); // moving

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);
        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_StopUpdateArrival_8c02ce48');

        $this->shouldCall('_adjustPoints_8c02b464')->with(0x21, 0xfffffff1); // -15
        $this->shouldWriteLong($this->addressOf('_var_busState_8c1bb9d0') + 0x3c4, 1);

        $this->shouldWriteLong($base + 0x18, 1);
    }

    // playMode 1 (a course run) with the "ready" bit clear: the run-progress
    // countdown decrements and, once exhausted, applies a flat penalty and
    // returns without reaching the increment path.
    public function test_run_progress_countdown_expires_applies_penalty(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 0); // bit 0 clear -> messagebox branch too
        $this->initUint32($base + 0x18, 0); // var_runState_8c2285c4.runClock_0x18 decrements to -1

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);
        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_StopUpdateArrival_8c02ce48');

        $this->shouldWriteLong($base + 0x18, -1);
        $this->shouldWriteLong($base + 0x18, 0);
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x1d, 0xffffff38); // -200
    }

    // playMode 1 with the "ready" bit set: the countdown is skipped (falls
    // to the increment path) but bit 0 also routes around the messagebox
    // award/penalty branch.
    public function test_run_progress_countdown_skipped_when_ready(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 3); // bits 0 and 1 both set

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);
        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_StopUpdateArrival_8c02ce48');

        $this->shouldCall('_adjustPoints_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);

        $this->shouldWriteLong($base + 0x18, 1); // var_runState_8c2285c4.runClock_0x18 incremented (bit 1 set -> else branch)
    }

    // Not a course run (playMode != 1): var_runState_8c2285c4.runClock_0x18 just increments, and
    // once past var_runState_8c2285c4.scheduleTime_0x14 and a multiple of some divisor, a silent
    // (msgSet -1) adjustment fires via __modls.
    public function test_run_progress_probe_modls_divisor(): void
    {
        $base = $this->resolveSymbols();
        $busPtr = $this->baseline($base);

        $this->initUint32($base + 0x18, 29); // var_runState_8c2285c4.runClock_0x18 -> increments to 30
        $this->initUint32($base + 0x14, 0); // var_runState_8c2285c4.scheduleTime_0x14 (0 < 30)
        $this->initUint32($this->addressOf('_var_hudState_8c22643c') + 0x14, 1); // != -1

        $this->call('_gradeFrame_8c02bcd8');

        $this->shouldWriteLong($base + 0x68, 0);
        $this->shouldWriteLong($base + 0x58, 1);
        $this->shouldCall('_StopUpdateArrival_8c02ce48');
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x20, 0xfffffffb); // -5
        $this->shouldWriteLong(($this->addressOf('_var_driveCueState_8c2264b8') + 0x0c), 1);

        $this->shouldWriteLong($base + 0x18, 30);

        $this->shouldCall('__modls')
            ->with(30, 30)
            ->using(new \Lhsazevedo\Sh4ObjTest\Simulator\CallingConventions\RiroCallingConvention())
            ->andReturn(0);

        $this->shouldCall('_adjustPoints_8c02b464')->with(-1, -1);
    }
};
