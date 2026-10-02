<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Selects the animated VMU LCD slot; -1 disables and clears it.
return new class extends TestCase {
    public function test_select_slot()
    {
        $this->call('_VmGameSetLcdAnim_8c01c8fc')->with(1);
        $this->shouldWriteLong($this->addressOf('_var_lcdAnimIdx_8c2263a0'), 1);
    }

    public function test_disable_clears_lcd()
    {
        $this->call('_VmGameSetLcdAnim_8c01c8fc')->with(0xffffffff); // -1
        $this->shouldWriteLong($this->addressOf('_var_lcdAnimIdx_8c2263a0'), 0xffffffff);
        $this->shouldCall('_advanceLcdAnim_8c01bb48')->with(0, 0);
    }
};
