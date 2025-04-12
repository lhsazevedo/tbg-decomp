<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_happyPath()
    {
        $sizeOfQueuedPvm = 0x18;
        $queueItems = 16;

        $this->shouldCall('_syMalloc')
            ->with($queueItems * $sizeOfQueuedPvm)
            ->andReturn(0xbebacafe);

        $this->shouldWriteTo('_var_pvmQueue_8c157abc', 0xbebacafe);
        $this->shouldWriteTo('_var_pvmQueueTail_8c157ac4', 0xbebacafe + $queueItems * $sizeOfQueuedPvm);

        $this->singleCall('_initPvmQueue_8c011a5c')
            ->with($queueItems)
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_returnsZeroOnAllocError()
    {
        $sizeOfQueuedPvm = 0x18;
        $queueItems = 16;

        $this->shouldCall('_syMalloc')
            ->with($queueItems * $sizeOfQueuedPvm)
            ->andReturn(0);

        $this->shouldWriteTo('_var_pvmQueue_8c157abc', 0);

        $this->singleCall('_initPvmQueue_8c011a5c')
            ->with($queueItems)
            ->singleShouldReturn(0)
            ->run();
    }

    public function test_clearQueueWhenNIs0()
    {
        $this->shouldWriteTo('_var_pvmQueueTail_8c157ac4', -1);
        $this->shouldWriteTo('_var_pvmQueue_8c157abc', -1);

        $this->singleCall('_initPvmQueue_8c011a5c')
            ->with(0)
            ->singleShouldReturn(1)
            ->run();
    }
};
