<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_happyPath()
    {
        $sizeOfQueuedDat = 0x10;
        $queueItems = 16;

        $this->shouldCall('_syMalloc')
            ->with($queueItems * $sizeOfQueuedDat)
            ->andReturn(0xbebacafe);

        $this->shouldWriteTo('_var_datQueue_8c157a8c', 0xbebacafe);
        $this->shouldWriteTo('_var_datQueueTail_8c157a94', 0xbebacafe + $queueItems * $sizeOfQueuedDat);

        $this->singleCall('_initDatQueue_8c011124')
            ->with($queueItems)
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_returns0OnAllocError()
    {
        $sizeOfQueuedDat = 0x10;
        $queueItems = 16;

        $this->shouldCall('_syMalloc')
            ->with($queueItems * $sizeOfQueuedDat)
            ->andReturn(0);

        $this->shouldWriteTo('_var_datQueue_8c157a8c', 0);

        $this->singleCall('_initDatQueue_8c011124')
            ->with($queueItems)
            ->singleShouldReturn(0)
            ->run();
    }

    public function test_clearQueueWhenNIs0()
    {
        $this->shouldWriteTo('_var_datQueueTail_8c157a94', -1);
        $this->shouldWriteTo('_var_datQueue_8c157a8c', -1);

        $this->singleCall('_initDatQueue_8c011124')
            ->with(0)
            ->singleShouldReturn(1)
            ->run();
    }
};
