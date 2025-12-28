<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase
{
    public function test_happyPath(): void
    {
        $this->initUint32($this->addressOf('_var_texlistQueue_8c157aac'), 42);
        $this->initUint32($this->addressOf('_var_texlistQueueRear_8c157ab0'), 46);

        $this->shouldWriteTo('_var_texlistQueueIsIdle_8c157ab8', 0);

        $createdTask = $this->alloc(0x1c);
        $createdTaskLocal = 0x00fffff0;
        $createdState = 0x00fffff4;

        $this->shouldCall('_pushTask_8c014ae8')
            ->with($this->addressOf('_var_tasks_8c1ba3c8'), $this->addressOf('_task_loadQueuedTexlists_8c01183e'), $createdTaskLocal, $createdState, 0)
            ->do(function ($params) use ($createdTask) {
                $this->writeUInt32($params[2], 0, U32::of($createdTask));
            })
            ->andReturn(1);

        $this->shouldWrite($createdTask + 0x18, 42);

        $this->singleCall('_loadTexlistQueue_8c0119f8')
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_handlesEmptyQueue(): void
    {
        $this->initUint32($this->addressOf('_var_texlistQueue_8c157aac'), 42);
        $this->initUint32($this->addressOf('_var_texlistQueueRear_8c157ab0'), 42);

        $this->singleCall('_loadTexlistQueue_8c0119f8')
            ->singleShouldReturn(0)
            ->run();
    }

    public function test_handlesPushFailure(): void
    {
        $this->initUint32($this->addressOf('_var_texlistQueue_8c157aac'), 42);
        $this->initUint32($this->addressOf('_var_texlistQueueRear_8c157ab0'), 46);

        $this->shouldWriteTo('_var_texlistQueueIsIdle_8c157ab8', 0);

        $createdTask = $this->alloc(0x1c);
        $createdTaskLocal = 0x00fffff0;
        $createdState = 0x00fffff4;

        $this->shouldCall('_pushTask_8c014ae8')
            ->with($this->addressOf('_var_tasks_8c1ba3c8'), $this->addressOf('_task_loadQueuedTexlists_8c01183e'), $createdTaskLocal, $createdState, 0)
            ->andReturn(0);

        $this->singleCall('_loadTexlistQueue_8c0119f8')
            ->singleShouldReturn(0)
            ->run();
    }
};
