<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_offense_code_0x30000_applies_penalty(): void
    {
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 0x30000);

        $this->call('_gradeOffCourseSevere_8c02b864');

        $this->shouldCall('_adjustPoints_8c02b464')->with(8, 0xffffff38); // -200
        $this->shouldCall('_armCooldowns_8c02b578')->with(2);
        $this->shouldWriteLongTo('_var_cooldownCollision_8c228690', 0x7fff);
    }

    public function test_other_offense_code_is_a_no_op(): void
    {
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 0x20000);

        $this->call('_gradeOffCourseSevere_8c02b864');
    }

    public function test_zero_is_a_no_op(): void
    {
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 0);

        $this->call('_gradeOffCourseSevere_8c02b864');
    }
};
