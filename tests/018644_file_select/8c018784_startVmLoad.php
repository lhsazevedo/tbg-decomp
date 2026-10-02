<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    /* Queue loadFileTask over the full save-file list and allocate the staging buffer. */
    public function test_pushes_task_and_allocates_buffer(): void
    {
        $this->setSize('_var_tasks_8c1ba3c8', 4);
        $this->setSize('_var_vmBusy_8c157a7c', 4);
        $this->setSize('_init_saveNames_8c044d50', 0x2c); // char*[11]
        $this->setSize('_var_saveBuf_8c1ba2e0', 4);
        $this->setSize('_syMalloc', 4);

        $installedTask = $this->alloc(0x20);
        $installedState = $this->alloc(0x1c);
        $buf = $this->alloc(0x3c00);

        $this->call('_startVmLoad_8c018784');

        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_loadFileTask_8c018644'),
                0xffffec, // &installedTask on the stack
                0xfffff0, // &installedState on the stack
                0,
            )
            ->do(function ($params) use ($installedTask, $installedState) {
                $this->memory->writeUInt32($params[2], U32::of($installedTask));
                $this->memory->writeUInt32($params[3], U32::of($installedState));
            });
        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 1);
        $this->shouldWriteLong($installedTask + 0x08, 0);
        $this->shouldWriteLong($installedTask + 0x0c, 0);
        $this->shouldWriteLong($installedTask + 0x18, $this->addressOf('_init_saveNames_8c044d50'));
        $this->shouldCall('_syMalloc')->with(0x3c00)->andReturn($buf);
        $this->shouldWriteLong($this->addressOf('_var_saveBuf_8c1ba2e0'), $buf);
        $this->shouldWriteLong($this->addressOf('_var_saveBufCursor_8c225fe0'), $buf);
        $this->shouldWriteLong($this->addressOf('_var_saveLoadResult_8c226010'), 0);
    }
};
