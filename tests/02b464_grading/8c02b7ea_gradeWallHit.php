<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_no_flags_set_is_a_no_op(): void
    {
        $this->initUint32($this->addressOf('_var_wallHitBits_8c228660'), 0xfffffff9); // ~6, i.e. bits 0x2/0x4 clear

        $this->call('_gradeWallHit_8c02b7ea');
    }

    public function test_bit_0x2_only_applies_mild_penalty(): void
    {
        $this->initUint32($this->addressOf('_var_wallHitBits_8c228660'), 2);

        $this->call('_gradeWallHit_8c02b7ea');

        $this->shouldCall('_adjustPoints_8c02b464')->with(3, 0xffffffe2); // -30
        $this->shouldCall('_armCooldowns_8c02b578')->with(1);
    }

    public function test_bit_0x4_only_applies_medium_penalty(): void
    {
        $this->initUint32($this->addressOf('_var_wallHitBits_8c228660'), 4);

        $this->call('_gradeWallHit_8c02b7ea');

        $this->shouldCall('_adjustPoints_8c02b464')->with(4, 0xffffffc4); // -60
        $this->shouldCall('_armCooldowns_8c02b578')->with(1);
    }

    public function test_both_bits_apply_worst_penalty(): void
    {
        $this->initUint32($this->addressOf('_var_wallHitBits_8c228660'), 6);

        $this->call('_gradeWallHit_8c02b7ea');

        $this->shouldCall('_adjustPoints_8c02b464')->with(5, 0xffffff38); // -200
        $this->shouldCall('_armCooldowns_8c02b578')->with(1);
    }

    public function test_ignores_other_bits(): void
    {
        $this->initUint32($this->addressOf('_var_wallHitBits_8c228660'), 0xfffffff8); // only unrelated bits set

        $this->call('_gradeWallHit_8c02b7ea');
    }
};
