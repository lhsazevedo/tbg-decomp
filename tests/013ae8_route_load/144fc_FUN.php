<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    /** Install task_load and prime the asset queues for the route-load screen. */
    public function test_installs_task_and_inits_queues(): void
    {
        $this->setSize('_var_tasks_8c1ba3c8', 4);
        $this->setSize('_var_loadScreenActive_8c157a6c', 4);
        $this->setSize('_var_tex_8c157af8', 4);
        $this->setSize('_njSetBackColor', 4);
        $this->setSize('_njGarbageTexture', 4);

        $createdTask = $this->alloc(0x20);
        $createdState = $this->alloc(0x1c);

        $this->call('_pushRouteLoadTask_8c0144fc');

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
        $this->shouldWriteLong($this->addressOf('_var_loadScreenActive_8c157a6c'), 1);
        $this->shouldCall('_pushTask_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_routeLoadTask_8c014338'),
                0xffffec,
                0xfffff0,
                0,
            )
            ->do(function ($params) use ($createdTask, $createdState) {
                $this->memory->writeUInt32($params[2], U32::of($createdTask));
                $this->memory->writeUInt32($params[3], U32::of($createdState));
            });
        $this->shouldWriteLong($createdTask + 0x08, 0);
        $this->shouldWriteLong($createdTask + 0x0c, 0);
        $this->shouldCall('_njGarbageTexture')->with($this->addressOf('_var_tex_8c157af8'), 0xc00);
        $this->shouldCall('_AsqInitQueues_8c011f36')->with(0x20, 0x800, 0x800, 0x40);
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
