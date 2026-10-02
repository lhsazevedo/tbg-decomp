<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_tasks_8c1ba5e8', 4);
    }

    public function test_skipped_in_demo_playback(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 2); // PLAY_MODE_DEMO

        $this->call('_DriveCueInit_8c020528');
    }

    public function test_pushes_task_and_seeds_state(): void
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1);
        $state = $this->addressOf('_var_driveCueState_8c2264b8');

        $this->call('_DriveCueInit_8c020528');

        $this->shouldCall('_TaskInstall_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba5e8'),
            $this->addressOf('_driveCueTask_8c020214'),
            0xffffe8,
            0xffffec,
            0,
        );
        $this->shouldWriteLong($state + 0x00, 0);
        $this->shouldCall('_AsqGetRandomInRangeB_8c0121be')->with(300)->andReturn(7);
        $this->shouldWriteLong($state + 0x04, 157);
        $this->shouldWriteLong($state + 0x08, 3);
        $this->shouldWriteLong($state + 0x0c, 1);
        $this->shouldWriteLong($state + 0x14, 0);
        $this->shouldWriteLong($state + 0x18, 0);
    }
};
