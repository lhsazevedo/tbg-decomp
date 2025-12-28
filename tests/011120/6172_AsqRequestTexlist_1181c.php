<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_1()
    {
        $sizeOfStruct = 8;
        $queueSize = 8;
        $queue = $this->alloc($queueSize * $sizeOfStruct);
        $queueTail = $queue + $queueSize * $sizeOfStruct;

        $this->initUint32($this->addressOf('_var_texlistQueueRear_8c157ab0'), $queue);
        $this->initUint32($this->addressOf('_var_texlistQueueTail_8c157ab4'), $queueTail);

        $dirname = $this->allocString('\\MY_DIR');
        $unknownStruct = $this->alloc(4);

        $this->shouldWriteString($queue + 0, '\\MY_DIR');
        $this->shouldWrite($queue + 4, $unknownStruct);
        $this->shouldWriteTo('_var_texlistQueueRear_8c157ab0', $queue + $sizeOfStruct);

        $this->singleCall('_AsqRequestTexlist_1181c')
            ->with($dirname, $unknownStruct)
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_2()
    {
        $sizeOfStruct = 8;
        $queueSize = 8;
        $queue = $this->alloc($queueSize * $sizeOfStruct);
        $queueTail = $queue + $queueSize * $sizeOfStruct;

        $this->initUint32($this->addressOf('_var_texlistQueueRear_8c157ab0'), $queueTail);
        $this->initUint32($this->addressOf('_var_texlistQueueTail_8c157ab4'), $queueTail);

        $dirname = $this->allocString('\\MY_DIR');
        $unknownStruct = $this->alloc(4);


        $this->singleCall('_AsqRequestTexlist_1181c')
            ->with($dirname, $unknownStruct)
            ->singleShouldReturn(0)
            ->run();
    }
};
