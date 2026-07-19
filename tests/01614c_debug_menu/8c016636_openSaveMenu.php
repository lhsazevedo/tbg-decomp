<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_installs_task_and_inits_state()
    {
        $this->setSize('_var_tasks_8c1ba3c8', 4);

        $createdTask = $this->alloc(0x14);
        $createdState = $this->alloc(0x14);

        $this->call('_openSaveMenu_8c016636');

        $this->shouldCall('_njSetBackColor')->with(0, 0, 0xc060);
        $this->shouldCall('_TaskPush_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_saveMenuTask_8c01628c'),
                0xfffff4,
                0xfffff0,
                0x14,
            )
            ->do(function ($params) use ($createdTask, $createdState) {
                $this->memory->writeUInt32($params[2], U32::of($createdTask));
                $this->memory->writeUInt32($params[3], U32::of($createdState));
            });
        $this->shouldWriteLong($createdState + 0x00, 0);
        $this->shouldWriteLong($createdState + 0x0c, 0);
    }
};
