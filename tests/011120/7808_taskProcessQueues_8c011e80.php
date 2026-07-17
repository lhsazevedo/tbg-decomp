<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;

return new class extends TestCase {
    public function test_datQueue_itWaitsForIdleBeforeAdvancing()
    {
        $task = $this->alloc(4);
        $state = $this->alloc(0x18);

        $this->initUint32($task, 0);
        $this->initUint32($state, 0);
        $this->initUint32($state + 0x04, 0xcafe0001);

        $this->shouldCall('_datQueueIsIdle_8c0113d2')->andReturn(0);

        $this->shouldCall(0xcafe0001);

        $this->singleCall('_taskProcessQueues_8c011e80')
            ->with($task, $state)
            ->run();
    }

    public function test_datQueue_itAdvancesWhenDone()
    {
        // FIXME
        $this->doNotRandomizeMemory();

        $task = $this->alloc(4);
        $state = $this->alloc(0x18);

        $this->initUint32($task, 0);
        $this->initUint32($state, 0);
        $this->initUint32($state + 0x04, 0xcafe0001);

        $this->shouldCall('_datQueueIsIdle_8c0113d2')->andReturn(1);

        $this->shouldWrite($state, 1);
        $this->shouldCall('_sortAndLoadNjQueue_8c0116b6');

        $this->shouldCall(0xcafe0001);

        $this->singleCall('_taskProcessQueues_8c011e80')
            ->with($task, $state)
            ->run();
    }

    public function test_datQueue_itCallsCallbackWhenDone()
    {
        $task = $this->alloc(4);
        $state = $this->alloc(0x18);

        $this->initUint32($task, 0);
        $this->initUint32($state, 0);
        $this->initUint32($state + 0x04, 0xcafe0001);
        $this->initUint32($state + 0x08, 0xcafe0002);

        $this->shouldCall('_datQueueIsIdle_8c0113d2')->andReturn(1);

        $this->shouldCall(0xcafe0002);

        $this->shouldWrite($state, 1);
        $this->shouldCall('_sortAndLoadNjQueue_8c0116b6');

        $this->shouldCall(0xcafe0001);

        $this->singleCall('_taskProcessQueues_8c011e80')
            ->with($task, $state)
            ->run();
    }

    public function test_njQueue_itWaitsForIdleBeforeAdvancing()
    {
        $task = $this->alloc(4);
        $state = $this->alloc(0x18);

        $this->initUint32($task, 0);
        $this->initUint32($state, 1);
        $this->initUint32($state + 0x04, 0xcafe0001);

        $this->shouldCall('_njQueueIsIdle_8c01179e')->andReturn(0);

        $this->shouldCall(0xcafe0001);

        $this->singleCall('_taskProcessQueues_8c011e80')
            ->with($task, $state)
            ->run();
    }

    public function test_njQueue_itAdvancesWhenDone()
    {
        // FIXME
        $this->doNotRandomizeMemory();

        $task = $this->alloc(4);
        $state = $this->alloc(0x18);

        $this->initUint32($task, 0);
        $this->initUint32($state, 1);
        $this->initUint32($state + 0x04, 0xcafe0001);

        $this->shouldCall('_njQueueIsIdle_8c01179e')->andReturn(1);

        $this->shouldWrite($state, 2);
        $this->shouldCall('_sortAndLoadPvmQueue_8c011d24');

        $this->shouldCall(0xcafe0001);

        $this->singleCall('_taskProcessQueues_8c011e80')
            ->with($task, $state)
            ->run();
    }

    public function test_njQueue_itCallsCallbackWhenDone()
    {
        $task = $this->alloc(4);
        $state = $this->alloc(0x18);

        $this->initUint32($task, 0);
        $this->initUint32($state, 1);
        $this->initUint32($state + 0x04, 0xcafe0001);
        $this->initUint32($state + 0x0c, 0xcafe0002);

        $this->shouldCall('_njQueueIsIdle_8c01179e')->andReturn(1);

        $this->shouldCall(0xcafe0002);

        $this->shouldWrite($state, 2);
        $this->shouldCall('_sortAndLoadPvmQueue_8c011d24');

        $this->shouldCall(0xcafe0001);

        $this->singleCall('_taskProcessQueues_8c011e80')
            ->with($task, $state)
            ->run();
    }

    public function test_pvmQueue_itWaitsForIdleBeforeAdvancing()
    {
        $task = $this->alloc(4);
        $state = $this->alloc(0x18);

        $this->initUint32($task, 0);
        $this->initUint32($state, 2);
        $this->initUint32($state + 0x04, 0xcafe0001);

        $this->shouldCall('_pvmQueueIsIdle_8c011e22')->andReturn(0);

        $this->shouldCall(0xcafe0001);

        $this->singleCall('_taskProcessQueues_8c011e80')
            ->with($task, $state)
            ->run();
    }

    public function test_pvmQueue_itAdvancesWhenDone()
    {
        // FIXME
        $this->doNotRandomizeMemory();

        $task = $this->alloc(4);
        $state = $this->alloc(0x18);

        $this->initUint32($task, 0);
        $this->initUint32($state, 2);
        $this->initUint32($state + 0x04, 0xcafe0001);

        $this->shouldCall('_pvmQueueIsIdle_8c011e22')->andReturn(1);

        $this->shouldWrite($state, 3);
        $this->shouldCall('_loadTexlistQueue_8c0119f8');

        $this->shouldCall(0xcafe0001);

        $this->singleCall('_taskProcessQueues_8c011e80')
            ->with($task, $state)
            ->run();
    }

    public function test_pvmQueue_itCallsCallbackWhenDone()
    {
        $task = $this->alloc(4);
        $state = $this->alloc(0x18);

        $this->initUint32($task, 0);
        $this->initUint32($state, 2);
        $this->initUint32($state + 0x04, 0xcafe0001);
        $this->initUint32($state + 0x10, 0xcafe0002);

        $this->shouldCall('_pvmQueueIsIdle_8c011e22')->andReturn(1);

        $this->shouldCall(0xcafe0002);

        $this->shouldWrite($state, 3);
        $this->shouldCall('_loadTexlistQueue_8c0119f8');

        $this->shouldCall(0xcafe0001);

        $this->singleCall('_taskProcessQueues_8c011e80')
            ->with($task, $state)
            ->run();
    }

    public function test_texlist_itWaitsForIdleBeforeAdvancing()
    {
        $task = $this->alloc(4);
        $state = $this->alloc(0x18);

        $this->initUint32($task, 0);
        $this->initUint32($state, 3);
        $this->initUint32($state + 0x04, 0xcafe0001);

        $this->shouldCall('_texlistQueueIsIdle_8c011a42')->andReturn(0);

        $this->shouldCall(0xcafe0001);

        $this->singleCall('_taskProcessQueues_8c011e80')
            ->with($task, $state)
            ->run();
    }

    public function test_texlist_itAdvancesWhenDone()
    {
        // FIXME
        $this->doNotRandomizeMemory();

        $task = $this->alloc(4);
        $state = $this->alloc(0x18);

        $this->initUint32($task, 0);
        $this->initUint32($state, 3);
        $this->initUint32($state + 0x04, 0xcafe0001);

        $this->shouldCall('_texlistQueueIsIdle_8c011a42')->andReturn(1);

        $this->shouldCall('_TaskFree_8c014b66')->with($task);

        $this->singleCall('_taskProcessQueues_8c011e80')
            ->with($task, $state)
            ->run();
    }

    public function test_texlist_itCallsCallbackWhenDone()
    {
        $task = $this->alloc(4);
        $state = $this->alloc(0x18);

        $this->initUint32($task, 0);
        $this->initUint32($state, 3);
        $this->initUint32($state + 0x04, 0xcafe0001);
        $this->initUint32($state + 0x14, 0xcafe0002);

        $this->shouldCall('_texlistQueueIsIdle_8c011a42')->andReturn(1);

        $this->shouldCall('_TaskFree_8c014b66')->with($task);
        $this->shouldCall(0xcafe0002);

        $this->singleCall('_taskProcessQueues_8c011e80')
            ->with($task, $state)
            ->run();
    }
};
