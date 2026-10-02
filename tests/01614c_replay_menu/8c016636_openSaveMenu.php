<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_installs_task_and_inits_state()
    {
        $this->setSize('_var_tasks_8c1ba3c8', 4);

        $installedTask = $this->alloc(0x14);
        $installedState = $this->alloc(0x14);

        $this->call('_openSaveMenu_8c016636');

        $this->shouldCall('_njSetBackColor')->with(0, 0, 0xc060);
        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_saveMenuTask_8c01628c'),
                0xfffff4,
                0xfffff0,
                0x14,
            )
            ->do(function ($params) use ($installedTask, $installedState) {
                $this->memory->writeUInt32($params[2], U32::of($installedTask));
                $this->memory->writeUInt32($params[3], U32::of($installedState));
            });
        $this->shouldWriteLong($installedState + 0x00, 0);
        $this->shouldWriteLong($installedState + 0x0c, 0);
    }
};
