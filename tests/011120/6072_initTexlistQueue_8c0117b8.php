<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;

return new class extends TestCase {
    public function test_paramIs1()
    {
        $this->shouldCall('_syMalloc')
            ->with(8)
            ->andReturn(0xbebacafe);
        
        $this->shouldWriteTo('_var_texlistQueue_8c157aac', 0xbebacafe);
        $this->shouldWriteTo('_var_texlistQueueTail_8c157ab4', 0xbebacafe + 8);

        $this->singleCall('_initTexlistQueue_8c0117b8')
            ->with(1)
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_paramIs2()
    {
        $this->shouldCall('_syMalloc')
            ->with(16)
            ->andReturn(0xbebacafe);

        $this->shouldWriteTo('_var_texlistQueue_8c157aac', 0xbebacafe);
        $this->shouldWriteTo('_var_texlistQueueTail_8c157ab4', 0xbebacafe + 16);

        $this->singleCall('_initTexlistQueue_8c0117b8')
            ->with(2)
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_returnsZeroOnFailedAllocation()
    {
        $this->shouldCall('_syMalloc')
            ->with(8)
            ->andReturn(0);

        $this->shouldWriteTo('_var_texlistQueue_8c157aac', 0);

        $this->singleCall('_initTexlistQueue_8c0117b8')
            ->with(1)
            ->singleShouldReturn(0)
            ->run();
    }

    public function test_resetsPointersWhenParamIsZero()
    {
        $this->shouldWriteTo('_var_texlistQueueTail_8c157ab4', -1);
        $this->shouldWriteTo('_var_texlistQueue_8c157aac', -1);

        $this->singleCall('_initTexlistQueue_8c0117b8')
            ->with(0)
            ->singleShouldReturn(1)
            ->run();
    }

    // public function test_returnsZeroOnEmptyQueue()
    // {
    //     $sizeOfQueuedNj = 0x10;
    //     $queueSize = 16;
    //     $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

    //     $this->initUint32($this->addressOf('_var_njQueue_8c157a9c'), $njQueue);
    //     $this->initUint32(
    //         $this->addressOf('_var_njQueueRear_8c157aa0'),
    //         $njQueue,
    //     );

    //     $this->singleCall('_sortAndLoadNjQueue_8c0116b6')
    //         ->singleShouldReturn(0)
    //         ->run();
    // }

    // public function test_returnsZeroOnPushFailure()
    // {
    //     $sizeOfQueuedNj = 0x10;
    //     $queueSize = 16;
    //     $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

    //     $dirStrAddress = $this->allocString('\\DIR');
    //     $fileAStrAddr = $this->allocString('FILEA.BIN');

    //     $currentQueuedNj = $njQueue;
    //     $queuedNjA = $currentQueuedNj;

    //     $this->initUint32($this->addressOf('_var_njQueue_8c157a9c'), $njQueue);
    //     $this->initUint32(
    //         $this->addressOf('_var_njQueueRear_8c157aa0'),
    //         $njQueue + 1 * $sizeOfQueuedNj
    //     );

    //     //

    //     $this->shouldWriteTo('_var_njQueueIsIdle_8c157aa8', 0);

    //     $tempQueuedNj = $this->alloc(4);
    //     $this->shouldCall('_syMalloc')
    //         ->with(1 * $sizeOfQueuedNj)
    //         ->andReturn($tempQueuedNj);

    //     $this->shouldCall('_syFree')
    //         ->with($tempQueuedNj);

    //     $createdTask = $this->alloc(0x1c);
    //     $this->initUint32(0xffffd4, $createdTask);
    //     $this->shouldCall('_pushTask_8c014ae8')
    //         ->with(
    //             $this->addressOf('_var_tasks_8c1ba3c8'),
    //             new WildcardArgument(), // TODO: Make addressOf handle exports
    //             0xffffd4,
    //             0xffffd8,
    //             0
    //         )
    //         ->andReturn(0);

    //     $this->singleCall('_sortAndLoadNjQueue_8c0116b6')
    //         ->singleShouldReturn(0)
    //         ->run();
    // }
};
