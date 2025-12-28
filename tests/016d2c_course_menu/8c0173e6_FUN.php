<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_basic_test()
    {
        $this->setSize('_var_coursesToUnlock_8c225fd4', 4 * 9);
        $this->setSize('_var_progress_8c1ba1cc', 0x94);

        $this->initUint8($this->addressOf('_var_coursesToUnlock_8c225fd4') + 0, 6);
        $this->initUint8($this->addressOf('_var_coursesToUnlock_8c225fd4') + 1, 4);
        $this->initUint8($this->addressOf('_var_coursesToUnlock_8c225fd4') + 2, 5);
        $this->initUint8($this->addressOf('_var_coursesToUnlock_8c225fd4') + 3, 2);
        $this->initUint8($this->addressOf('_var_coursesToUnlock_8c225fd4') + 4, 3);
        $this->initUint8($this->addressOf('_var_coursesToUnlock_8c225fd4') + 5, 1);
        $this->initUint8($this->addressOf('_var_coursesToUnlock_8c225fd4') + 6, -1);

        $this->call('_CourseMenuApplyUnlocks_8c0173e6');

        $shouldWriteBoth = function (int $index, int $value) {
            $this->shouldWriteByte(
                $this->addressOf('_var_progress_8c1ba1cc') + 0x44 + $index * 8 + 0, $value
            );
            $this->shouldWriteByte(
                $this->addressOf('_var_progress_8c1ba1cc') + 0x44 + $index * 8 + 1, $value
            );
        };

        $shouldWriteBoth(index: 6, value: 1);
        $shouldWriteBoth(index: 4, value: 1);
        $shouldWriteBoth(index: 5, value: 1);
        $shouldWriteBoth(index: 2, value: 1);
        $shouldWriteBoth(index: 3, value: 1);
        $shouldWriteBoth(index: 1, value: 1);
    }
};
