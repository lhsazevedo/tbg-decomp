<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_resetsLcdIconCounters()
    {
        $this->call('_VmGameResetLcdAnims_8c01c8dc');

        $this->shouldWriteLong($this->addressOf('_var_lcdAnimBus_8c2260ac') + 4, 10);
        $this->shouldWriteLong($this->addressOf('_var_lcdAnimBus_8c2260ac') + 8, 0);
        $this->shouldWriteLong($this->addressOf('_var_lcdAnimDanger_8c2260b8') + 4, 10);
        $this->shouldWriteLong($this->addressOf('_var_lcdAnimDanger_8c2260b8') + 8, 0);
        $this->shouldWriteLong($this->addressOf('_var_lcdAnimLoading_8c2260c4') + 4, 10);
        $this->shouldWriteLong($this->addressOf('_var_lcdAnimLoading_8c2260c4') + 8, 0);
        $this->shouldWriteLong($this->addressOf('_var_lcdAnimIdx_8c2263a0'), 0);
        $this->shouldWriteLong($this->addressOf('_var_lcdAnimActive_8c2260a8'), 0);
    }
};
