<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // Rounds to float32 precision, so expected sums/quotients can be computed
    // the same way the SH4 FPU does (round each operand to float32 first,
    // then round the float32 result) rather than PHP's double precision.
    private function f32(float $value): float
    {
        return unpack('f', pack('f', $value))[1];
    }

    private function initFloat(int $address, float $value): void
    {
        $raw = unpack('L', pack('f', $value))[1];
        $this->initUint32($address, $raw);
    }

    public function test_low_speed_plays_lowest_cue_and_resets_to_0_3(): void
    {
        $busState = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initFloat($busState + 0x27c, 0.1); // < 0.1388889
        $this->initUint32($busState + 0x2e4, 1);
        $this->initUint32($busState + 0x2f4, 3);
        $this->initUint32($busState + 0x2b4, 0);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x1234);
        $this->initUint32($this->addressOf('_var_wallHitBits_8c228660'), 8); // pre-existing bits

        $this->call('_hitWall_8c023bea');

        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0x13, 0);
        $this->shouldWriteFloat($busState + 0x27c, 0.3);
        $this->shouldWriteLongTo('_var_wallHitBits_8c228660', 8 | 2);
        $this->shouldCall('_BusDriveEnterKnockback_8c023bce');
    }

    public function test_mid_speed_plays_mid_cue_and_halves_toward_0_3(): void
    {
        $busState = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initFloat($busState + 0x27c, 0.2); // 0.1388889 <= speed < 0.2777778
        $this->initUint32($busState + 0x2e4, 1);
        $this->initUint32($busState + 0x2f4, 5); // reverse, left alone
        $this->initUint32($busState + 0x2b4, 0);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x1234);
        $this->initUint32($this->addressOf('_var_wallHitBits_8c228660'), 0);

        $this->call('_hitWall_8c023bea');

        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0x14, 0);
        $this->shouldWriteFloat($busState + 0x27c, $this->f32($this->f32(0.2) / 2.0 + 0.3));
        $this->shouldWriteLongTo('_var_wallHitBits_8c228660', 4);
        $this->shouldCall('_BusDriveEnterKnockback_8c023bce');
    }

    public function test_high_speed_plays_highest_cue_and_halves_toward_0_3(): void
    {
        $busState = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initFloat($busState + 0x27c, 0.5); // >= 0.2777778
        $this->initUint32($busState + 0x2e4, 1);
        $this->initUint32($busState + 0x2f4, 3);
        $this->initUint32($busState + 0x2b4, 0);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x1234);
        $this->initUint32($this->addressOf('_var_wallHitBits_8c228660'), 0);

        $this->call('_hitWall_8c023bea');

        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0x15, 0);
        $this->shouldWriteFloat($busState + 0x27c, $this->f32($this->f32(0.5) / 2.0 + 0.3));
        $this->shouldWriteLongTo('_var_wallHitBits_8c228660', 6);
        $this->shouldCall('_BusDriveEnterKnockback_8c023bce');
    }
};
