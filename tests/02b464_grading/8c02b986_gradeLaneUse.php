<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): int
    {
        $base = $this->addressOf('_var_runState_8c2285c4');

        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_var_driveMode_8c1bb8c8', 4);

        return $base;
    }

    // Makes var_runState_8c2285c4.lane_0x6c (base+0x6c) equal var_prevLane_8c228684, which alone
    // makes the outer OR true (short-circuiting the rest) and sends the
    // block into the var_driveMode_8c1bb8c8-gated timing logic instead
    // of the flat penalty at the top of that block.
    private function matchSignalState(int $base): void
    {
        $this->initUint32($base + 0x6c, 5);
        $this->initUint32($this->addressOf('_var_prevLane_8c228684'), 5);
    }

    private function initFloat(int $address, float $value): void
    {
        $raw = unpack('L', pack('f', $value))[1];
        $this->initUint32($address, $raw);
    }

    public function test_offense_code_zero_resets_repeat_count(): void
    {
        $base = $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_headingVsRoad_8c22868c'), 0);
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 5);
        $this->initUint32($base + 0x2c, 3); // repeatCount, nonzero
        $this->matchSignalState($base);
        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 1); // skip timing block
        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 0.0);

        $this->call('_gradeLaneUse_8c02b986');

        $this->shouldWriteLong($base + 0x2c, 0);
    }

    public function test_first_repeat_applies_light_penalty(): void
    {
        $base = $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_headingVsRoad_8c22868c'), 1);
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 5);
        $this->initUint32($base + 0x2c, 0); // repeatCount
        $this->matchSignalState($base);
        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 1);
        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 0.0);

        $this->call('_gradeLaneUse_8c02b986');

        $this->shouldCall('_adjustPoints_8c02b464')->with(0xb, 0xfffffff6); // -10
        $this->shouldWriteLong($base + 0x2c, 1);
        $this->shouldCall('_armCooldowns_8c02b578')->with(4);
    }

    public function test_repeated_offense_applies_heavier_penalty(): void
    {
        $base = $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_headingVsRoad_8c22868c'), 1);
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 5);
        $this->initUint32($base + 0x2c, 2); // repeatCount, already nonzero
        $this->matchSignalState($base);
        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 1);
        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 0.0);

        $this->call('_gradeLaneUse_8c02b986');

        $this->shouldCall('_adjustPoints_8c02b464')->with(0xc, 0xffffffce); // -50
        $this->shouldWriteLong($base + 0x2c, 3);
        $this->shouldCall('_armCooldowns_8c02b578')->with(4);
    }

    public function test_signal_state_disagreement_applies_penalty(): void
    {
        $base = $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_headingVsRoad_8c22868c'), 0);
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 0);
        $this->initUint32($base + 0x2c, 0);

        // All five OR terms false: var_runState_8c2285c4.lane_0x6c != var_prevLane_8c228684, and
        // each 0x40000 bit present.
        $this->initUint32($base + 0x6c, 5); // var_runState_8c2285c4.lane_0x6c
        $this->initUint32($this->addressOf('_var_prevLane_8c228684'), 6);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x34c), 0x40000);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x368), 0x40000);
        $this->initUint32($base + 0x74, 0x40000); // var_runState_8c2285c4.field_0x70[1]
        $this->initUint32($base + 0x78, 0x40000); // var_runState_8c2285c4.field_0x70[2]
        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 0.0);

        $this->call('_gradeLaneUse_8c02b986');

        $this->shouldWriteLong($base + 0x2c, 0);
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x12, 0xfffffff6); // -10
        $this->shouldCall('_armCooldowns_8c02b578')->with(4);
    }

    public function test_signals_match_and_bus_stopped_clears_counters(): void
    {
        $base = $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_headingVsRoad_8c22868c'), 0);
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 0);
        $this->initUint32($base + 0x2c, 0);
        $this->matchSignalState($base);

        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 0);
        $this->initUint32($this->addressOf('_var_laneA_8c228674'), 7);
        $this->initUint32($this->addressOf('_var_laneC_8c22867c'), 7); // == -> matches
        $this->initUint32($this->addressOf('_var_laneB_8c228678'), 7); // == -> matches
        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 0.0);

        $this->call('_gradeLaneUse_8c02b986');

        $this->shouldWriteLong($base + 0x2c, 0);

        $this->shouldWriteLong($base + 0x44, 0); // var_runState_8c2285c4.field_0x38[3]
        $this->shouldWriteLong($base + 0x48, 0); // var_runState_8c2285c4.field_0x38[4]
    }

    public function test_signal_duration_over_threshold_applies_penalty(): void
    {
        $base = $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_headingVsRoad_8c22868c'), 0);
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 0);
        $this->initUint32($base + 0x2c, 0);
        $this->matchSignalState($base);

        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 0);
        $this->initUint32($this->addressOf('_var_laneA_8c228674'), 3);
        $this->initUint32($this->addressOf('_var_laneC_8c22867c'), 7); // 674 < 67c -> cmpDir=2
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x34c), 0x40000000); // matches cmpDir==2 case
        $this->initUint32($base + 0x48, 0); // var_runState_8c2285c4.field_0x38[4] == 0 -> threshold 0xd2
        $this->initUint32($base + 0x44, 0xd3); // var_runState_8c2285c4.field_0x38[3] > threshold
        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 0.0);

        $this->call('_gradeLaneUse_8c02b986');

        $this->shouldWriteLong($base + 0x2c, 0);

        $this->shouldWriteLong($base + 0x44, 0);
        $this->shouldCall('_adjustPoints_8c02b464')->with(0xd, 0xfffffff6); // -10
        $this->shouldWriteLong($base + 0x48, 1);
    }

    public function test_signal_duration_under_threshold_is_a_no_op(): void
    {
        $base = $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_headingVsRoad_8c22868c'), 0);
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 0);
        $this->initUint32($base + 0x2c, 0);
        $this->matchSignalState($base);

        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 0);
        $this->initUint32($this->addressOf('_var_laneA_8c228674'), 3);
        $this->initUint32($this->addressOf('_var_laneC_8c22867c'), 7); // 674 < 67c -> cmpDir=2
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x34c), 0); // doesn't match -> threshold 0x3c
        $this->initUint32($base + 0x48, 0);
        $this->initUint32($base + 0x44, 0x10); // below threshold 0x3c
        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 0.0);

        $this->call('_gradeLaneUse_8c02b986');

        $this->shouldWriteLong($base + 0x2c, 0);
    }

    public function test_moving_increments_signal_duration_counter(): void
    {
        $base = $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_headingVsRoad_8c22868c'), 0);
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 0);
        $this->initUint32($base + 0x2c, 0);
        $this->matchSignalState($base);
        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 1); // skip timing block
        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 1.0); // moving
        $this->initUint32($base + 0x44, 5); // var_runState_8c2285c4.field_0x38[3]

        $this->call('_gradeLaneUse_8c02b986');

        $this->shouldWriteLong($base + 0x2c, 0);

        $this->shouldWriteLong($base + 0x44, 6);
    }
};
