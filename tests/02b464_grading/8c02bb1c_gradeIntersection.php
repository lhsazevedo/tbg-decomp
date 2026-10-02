<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): int
    {
        $base = $this->addressOf('_var_runState_8c2285c4');

        $this->setSize('_var_busState_8c1bb9d0', 0x400);
        $this->setSize('_SignalGetFrame_8c028900', 4);
        $this->setSize('_var_driveMode_8c1bb8c8', 4);
        $this->setSize('_var_playerBus_8c1bbd9c', 4); // BusState*, allocated via alloc()

        return $base;
    }

    private function initFloat(int $address, float $value): void
    {
        $raw = unpack('L', pack('f', $value))[1];
        $this->initUint32($address, $raw);
    }

    // Sets up a "stopped, no active signal, off-schedule fields cleared"
    // baseline so only the behavior under test fires.
    private function baseline(int $base): void
    {
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x358, 0); // not driving through a signal -> tail latch is a no-op
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x3b4, 0);
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x34c, 0); // speed limit code 0 -> limit 0
        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 0.0);
        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 1); // skip lane check
        $this->initUint32($base + 0x50, 0); // var_runState_8c2285c4.field_0x38[6], stopped-at-signal latch
    }

    public function test_stale_signal_applies_penalty(): void
    {
        $base = $this->resolveSymbols();
        $this->baseline($base);

        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x358, 0); // not driving through
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x3b4, 0xff000000);
        $this->initUint32($base + 0x3c, 7); // var_runState_8c2285c4.field_0x38[1], signal id

        $this->call('_gradeIntersection_8c02bb1c');

        $this->shouldCall('_SignalGetFrame_8c028900')->with(7)->andReturn(0);
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x13, 0xffffffb0); // -80
        $this->shouldCall('_armCooldowns_8c02b578')->with(5);

        $this->shouldWriteLong($base + 0x30, 0); // var_runState_8c2285c4.speedingCountdown_0x30 reset
    }

    public function test_moving_skips_stale_signal_penalty(): void
    {
        $base = $this->resolveSymbols();
        $this->baseline($base);

        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x358, 0);
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x3b4, 0xff000000);
        $this->initUint32($base + 0x3c, 7);
        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 1.0); // moving, so also over the (zero) speed limit
        $this->initUint32($base + 0x30, 5); // var_runState_8c2285c4.speedingCountdown_0x30, stays positive after decrement

        $this->call('_gradeIntersection_8c02bb1c');

        $this->shouldCall('_SignalGetFrame_8c028900')->with(7)->andReturn(0);

        $this->shouldWriteLong($base + 0x30, 4);
    }

    public function test_speed_within_limit_resets_counter(): void
    {
        $base = $this->resolveSymbols();
        $this->baseline($base);
        $this->initUint32($base + 0x30, 5); // var_runState_8c2285c4.speedingCountdown_0x30, nonzero

        $this->call('_gradeIntersection_8c02bb1c');

        $this->shouldWriteLong($base + 0x30, 0);
    }

    public function test_speeding_decrements_counter_without_penalty(): void
    {
        $base = $this->resolveSymbols();
        $this->baseline($base);
        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 1.0); // over the (zero) limit
        $this->initUint32($base + 0x30, 5); // still positive after decrement

        $this->call('_gradeIntersection_8c02bb1c');

        $this->shouldWriteLong($base + 0x30, 4);
    }

    public function test_speeding_counter_expires_at_high_speed(): void
    {
        $base = $this->resolveSymbols();
        $this->baseline($base);
        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 1.0); // > limit + 0.185...
        $this->initUint32($base + 0x30, 0); // decrements to -1

        $this->call('_gradeIntersection_8c02bb1c');

        $this->shouldWriteLong($base + 0x30, -1);
        $this->shouldWriteLong($base + 0x30, 0x78);
        $this->shouldCall('_adjustPoints_8c02b464')->with(10, 0xffffffe2); // -30
    }

    public function test_speeding_counter_expires_at_low_speed(): void
    {
        $base = $this->resolveSymbols();
        $this->baseline($base);
        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 0.1); // over limit (0) but under +0.185...
        $this->initUint32($base + 0x30, 0);

        $this->call('_gradeIntersection_8c02bb1c');

        $this->shouldWriteLong($base + 0x30, -1);
        $this->shouldWriteLong($base + 0x30, 0x78);
        $this->shouldCall('_adjustPoints_8c02b464')->with(9, 0xfffffff6); // -10
    }

    public function test_lane_change_without_signal_applies_penalty(): void
    {
        $base = $this->resolveSymbols();
        $this->baseline($base);
        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 0); // enable lane check

        $busState = $this->alloc(0x2b8);
        $this->initUint32($busState + 0x25c, 0); // turn signal not matching left (1)
        $this->initUint32($this->addressOf('_var_playerBus_8c1bbd9c'), $busState);

        $this->initUint32($base + 0x6c, 4); // var_runState_8c2285c4.lane_0x6c
        $this->initUint32($this->addressOf('_var_prevLane_8c228684'), 10); // laneDelta = 4-10 < 0
        $this->initUint32($this->addressOf('_var_prevLaneFlags_8c228688'), 0xf000000);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x70, 0xf000000);

        $this->call('_gradeIntersection_8c02bb1c');

        $this->shouldWriteLong($base + 0x30, 0); // var_runState_8c2285c4.speedingCountdown_0x30 reset

        $this->shouldCall('_adjustPoints_8c02b464')->with(0xf, 0xfffffff8); // -8
    }

    public function test_lane_change_with_correct_signal_is_a_no_op(): void
    {
        $base = $this->resolveSymbols();
        $this->baseline($base);
        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 0);

        $busState = $this->alloc(0x2b8);
        $this->initUint32($busState + 0x25c, 1); // left signal matches laneDelta < 0
        $this->initUint32($this->addressOf('_var_playerBus_8c1bbd9c'), $busState);

        $this->initUint32($base + 0x6c, 4);
        $this->initUint32($this->addressOf('_var_prevLane_8c228684'), 10); // laneDelta < 0

        $this->call('_gradeIntersection_8c02bb1c');

        $this->shouldWriteLong($base + 0x30, 0); // var_runState_8c2285c4.speedingCountdown_0x30 reset
    }

    public function test_input_map_sel_skips_lane_check(): void
    {
        $base = $this->resolveSymbols();
        $this->baseline($base); // inputMapSel already 1

        $busState = $this->alloc(0x2b8);
        $this->initUint32($busState + 0x25c, 0);
        $this->initUint32($this->addressOf('_var_playerBus_8c1bbd9c'), $busState);

        $this->initUint32($base + 0x6c, 4);
        $this->initUint32($this->addressOf('_var_prevLane_8c228684'), 10);
        $this->initUint32($this->addressOf('_var_prevLaneFlags_8c228688'), 0xf000000);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x70, 0xf000000);

        $this->call('_gradeIntersection_8c02bb1c');

        $this->shouldWriteLong($base + 0x30, 0); // var_runState_8c2285c4.speedingCountdown_0x30 reset
    }

    public function test_stopping_at_signal_arms_latch(): void
    {
        $base = $this->resolveSymbols();
        $this->baseline($base);
        $this->initUint32($base + 0x50, 0); // var_runState_8c2285c4.field_0x38[6] not armed
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x358, 0xf000000); // driving through signal

        $this->call('_gradeIntersection_8c02bb1c');

        $this->shouldWriteLong($base + 0x30, 0); // var_runState_8c2285c4.speedingCountdown_0x30 reset

        $this->shouldWriteLong($base + 0x50, 1);
    }

    public function test_armed_latch_with_matching_turn_signal_clears(): void
    {
        $base = $this->resolveSymbols();
        $this->baseline($base);
        $this->initUint32($base + 0x50, 1); // armed
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x358, 0); // no longer driving through
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x34c, 0x40000000); // turning left
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x25c, 1); // left signal on

        $this->call('_gradeIntersection_8c02bb1c');

        $this->shouldWriteLong($base + 0x30, 0); // var_runState_8c2285c4.speedingCountdown_0x30 reset

        $this->shouldWriteLong($base + 0x50, 0);
    }

    public function test_armed_latch_with_mismatched_turn_signal_applies_penalty(): void
    {
        $base = $this->resolveSymbols();
        $this->baseline($base);
        $this->initUint32($base + 0x50, 1); // armed
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x358, 0);
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x34c, 0x40000000); // turning left
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x25c, 2); // wrong signal

        $this->call('_gradeIntersection_8c02bb1c');

        $this->shouldWriteLong($base + 0x30, 0); // var_runState_8c2285c4.speedingCountdown_0x30 reset

        $this->shouldCall('_adjustPoints_8c02b464')->with(0xf, 0xfffffff8); // -8
        $this->shouldWriteLong($base + 0x50, 0);
    }
};
