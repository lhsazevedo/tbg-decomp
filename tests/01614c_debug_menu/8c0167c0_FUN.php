<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_resets_play_mode_and_calls_FUN_8c01328c()
    {
        $this->setSize('_var_playMode_8c1bb8d0', 4);

        $this->call('_FUN_8c0167c0');

        $this->shouldWriteLongTo('_var_playMode_8c1bb8d0', 0); // PLAY_MODE_NORMAL
        $this->shouldCall('_FUN_8c01328c');
    }
};
