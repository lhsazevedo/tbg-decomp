<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_offense_code_0x20000_applies_penalty(): void
    {
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 0x20000);
        $this->initUint32($this->addressOf('_var_headingVsRoad_8c22868c'), 0);

        $this->call('_gradeOffCourse_8c02b886');

        $this->shouldCall('_adjustPoints_8c02b464')->with(7, 0xffffffd8); // -40
        $this->shouldCall('_armCooldowns_8c02b578')->with(2);
    }

    public function test_other_nonzero_offense_code_applies_lighter_penalty(): void
    {
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 5);
        $this->initUint32($this->addressOf('_var_headingVsRoad_8c22868c'), 0);

        $this->call('_gradeOffCourse_8c02b886');

        $this->shouldCall('_adjustPoints_8c02b464')->with(6, 0xfffffff1); // -15
        $this->shouldCall('_armCooldowns_8c02b578')->with(2);
    }

    public function test_zero_offense_code_is_a_no_op(): void
    {
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 0);
        $this->initUint32($this->addressOf('_var_headingVsRoad_8c22868c'), 0);

        $this->call('_gradeOffCourse_8c02b886');
    }

    public function test_state_2_is_a_no_op(): void
    {
        $this->initUint32($this->addressOf('_var_offCourseBits_8c228680'), 0x20000);
        $this->initUint32($this->addressOf('_var_headingVsRoad_8c22868c'), 2);

        $this->call('_gradeOffCourse_8c02b886');
    }
};
