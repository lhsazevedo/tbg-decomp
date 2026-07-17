<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_flag_zero_writes_41_and_sentinel(): void
    {
        $flag = $this->addressOf('_var_shouldShowFreeRunIntro_8c1bb8c0');
        $queue = $this->addressOf('_var_dialogQueue_8c225fbc');

        $this->initUint32($flag, 0);

        $this->call('_buildFreeRunMenuDialogFlow_8c017a20');

        $this->shouldWriteLong($queue + 0 * 4, 0x41);
        $this->shouldWriteLong($queue + 1 * 4, -1);
        $this->shouldWriteLong($flag, 0);
    }

    public function test_flag_nonzero_writes_40_41_and_sentinel(): void
    {
        $flag = $this->addressOf('_var_shouldShowFreeRunIntro_8c1bb8c0');
        $queue = $this->addressOf('_var_dialogQueue_8c225fbc');

        $this->initUint32($flag, 1);

        $this->call('_buildFreeRunMenuDialogFlow_8c017a20');

        $this->shouldWriteLong($queue + 0 * 4, 0x40);
        $this->shouldWriteLong($queue + 1 * 4, 0x41);
        $this->shouldWriteLong($queue + 2 * 4, -1);
        $this->shouldWriteLong($flag, 0);
    }
};
