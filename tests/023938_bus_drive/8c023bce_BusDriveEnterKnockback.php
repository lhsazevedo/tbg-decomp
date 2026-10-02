<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_zeroes_gear_when_not_reverse(): void
    {
        $busState = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($busState + 0x2e4, 1);
        $this->initUint32($busState + 0x2f4, 3); // gear != 5
        $this->initUint32($busState + 0x2b4, 0);

        $this->call('_BusDriveEnterKnockback_8c023bce');

        $this->shouldWriteLong($busState + 0x2e4, 0);
        $this->shouldWriteLong($busState + 0x2f4, 0);
        $this->shouldWriteLong($busState + 0x2b4, 2);
    }

    public function test_leaves_reverse_gear_untouched(): void
    {
        $busState = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($busState + 0x2e4, 1);
        $this->initUint32($busState + 0x2f4, 5); // gear == 5 (reverse)
        $this->initUint32($busState + 0x2b4, 0);

        $this->call('_BusDriveEnterKnockback_8c023bce');

        $this->shouldWriteLong($busState + 0x2e4, 0);
        $this->shouldWriteLong($busState + 0x2b4, 2);
    }
};
