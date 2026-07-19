<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_resets_backcolor_pushes_input_and_installs_task()
    {
        $this->setSize('_var_tasks_8c1ba3c8', 4);

        $createdTask = $this->alloc(0x10);

        $this->call('_DebugMenuOpen_8c01673a');

        $this->shouldCall('_njSetBackColor')->with(0, 0, 0);
        $this->shouldCall('_InputPushTask_8c0128cc')->with(0);
        $this->shouldCall('_TaskPush_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_listMenuTask_8c01666a'),
                0xfffff0,
                0xfffff4,
                0,
            )
            ->do(function ($params) use ($createdTask) {
                $this->memory->writeUInt32($params[2], U32::of($createdTask));
            });
        $this->shouldWriteLong($createdTask + 0x08, 0);
    }
};
