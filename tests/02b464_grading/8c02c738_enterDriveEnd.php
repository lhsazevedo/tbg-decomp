<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_starts_fade_out_flow(): void
    {
        $this->setSize('_var_tasks_8c1ba3c8', 4);

        $task = $this->alloc(0x1c);

        $this->call('_enterDriveEnd_8c02c738');

        $this->shouldCall('_ReplayMenuKillDriveTasks_8c01614c');
        // enterDriveEnd_8c02c738 has an empty stack frame of its own beyond the two
        // TaskSpawn out-params (created_task, created_state), pushed right
        // after the STS.L PR/ADD #-8,R15 prologue -- so their addresses are
        // the initial test stack pointer (16MiB-4, see sh4objtest's Run)
        // minus 12 and 8 respectively. Same layout in both objects.
        $sp0 = 1024 * 1024 * 16 - 4;
        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_driveEndFadeTask_8c02c69a'),
                $sp0 - 12, // &created_task
                $sp0 - 8, // &created_state
                0
            )
            ->do(function () use ($task) {
                // TaskSpawn itself allocates the real Task and writes it
                // through R6 (created_task's address, $sp0-12 above).
                $this->writeUInt32($this->getRegister(6)->value, 0, U32::of($task));
            })
            ->andReturn(1);

        $this->shouldWriteLong($task + 8, 0);
        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }
};
