<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): int
    {
        $base = $this->addressOf('_var_runState_8c2285c4');

        $this->setSize('_SignalGetFrame_8c028900', 4);
        $this->setSize('_var_driveMode_8c1bb8c8', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x400);

        return $base;
    }

    public function test_offense_code_penalty_when_signal_idle(): void
    {
        $base = $this->resolveSymbols();
        $signal0 = $base + 0x38;
        $graded = $base + 0x34;

        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 0);
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 0);
        $this->initUint32($this->addressOf('_var_headingVsRoad_8c22868c'), 2);
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x34c, 0); // no signal id
        $this->initUint32($signal0, 0); // [0] == 0
        $this->initUint32($signal0 + 4, 0); // [1]
        $this->initUint32($graded, 0);

        $this->call('_gradeSignals_8c02b8b8');

        $this->shouldCall('_adjustPoints_8c02b464')->with(0x16, 0xffffffce); // -50
        $this->shouldCall('_armCooldowns_8c02b578')->with(3);
        $this->shouldWriteLong($graded, 0);
        $this->shouldWriteLong($signal0, 0);
    }

    public function test_no_penalty_when_other_conditions_not_met(): void
    {
        $base = $this->resolveSymbols();
        $signal0 = $base + 0x38;
        $graded = $base + 0x34;

        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 1); // blocks the offense check
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 0);
        $this->initUint32($this->addressOf('_var_headingVsRoad_8c22868c'), 2);
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x34c, 0);
        $this->initUint32($signal0, 0);
        $this->initUint32($signal0 + 4, 0);
        $this->initUint32($graded, 0);

        $this->call('_gradeSignals_8c02b8b8');

        $this->shouldWriteLong($graded, 0);
        $this->shouldWriteLong($signal0, 0);
    }

    public function test_signal_gone_with_pending_id_and_frame_zero_applies_penalty(): void
    {
        $base = $this->resolveSymbols();
        $signal0 = $base + 0x38;
        $graded = $base + 0x34;

        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 1);
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 0);
        $this->initUint32($this->addressOf('_var_headingVsRoad_8c22868c'), 0);
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x34c, 0); // signal gone
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x358, 0); // not driving through
        $this->initUint32($signal0, 5); // [0] pending id
        $this->initUint32($signal0 + 4, 0);
        $this->initUint32($graded, 0);

        $this->call('_gradeSignals_8c02b8b8');

        $this->shouldCall('_SignalGetFrame_8c028900')->with(5)->andReturn(0);
        $this->shouldCall('_adjustPoints_8c02b464')->with(0x10, 0xffffffba); // -70
        $this->shouldCall('_armCooldowns_8c02b578')->with(3);
        $this->shouldWriteLong($graded, 0);
        $this->shouldWriteLong($signal0, 0);
    }

    public function test_signal_gone_with_pending_id_and_nonzero_frame_only_rearms(): void
    {
        $base = $this->resolveSymbols();
        $signal0 = $base + 0x38;
        $graded = $base + 0x34;

        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 1);
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 0);
        $this->initUint32($this->addressOf('_var_headingVsRoad_8c22868c'), 0);
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x34c, 0);
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x358, 0);
        $this->initUint32($signal0, 5);
        $this->initUint32($signal0 + 4, 0);
        $this->initUint32($graded, 0);

        $this->call('_gradeSignals_8c02b8b8');

        $this->shouldCall('_SignalGetFrame_8c028900')->with(5)->andReturn(1);
        $this->shouldCall('_armCooldowns_8c02b578')->with(3);
        $this->shouldWriteLong($graded, 0);
        $this->shouldWriteLong($signal0, 0);
    }

    public function test_signal_gone_but_bus_still_driving_through_skips_grading(): void
    {
        $base = $this->resolveSymbols();
        $signal0 = $base + 0x38;
        $graded = $base + 0x34;

        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 1);
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 0);
        $this->initUint32($this->addressOf('_var_headingVsRoad_8c22868c'), 0);
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x34c, 0);
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x358, 0xf000000); // still driving through
        $this->initUint32($signal0, 5);
        $this->initUint32($signal0 + 4, 0);
        $this->initUint32($graded, 0);

        $this->call('_gradeSignals_8c02b8b8');

        $this->shouldWriteLong($graded, 0);
        $this->shouldWriteLong($signal0, 0);
    }

    // NOTE: the "signal present" branch (junctionARoadFlags_0x34c & 0xfff != 0, which
    // assigns signalId via what compiles to EXTS.W) can't be exercised
    // here: sh4objtest doesn't implement EXTS.W yet, and that's true of the
    // unmodified .src object too, not just the C translation (see
    // 8c0293f6_pedestriansTask.php for the same limitation). The C mirrors
    // Ghidra's output for that branch but it is untested against src.obj.

    private function initFloat(int $address, float $value): void
    {
        $raw = unpack('L', pack('f', $value))[1];
        $this->initUint32($address, $raw);
    }
};
