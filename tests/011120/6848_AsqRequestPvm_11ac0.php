<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_requestsPvm()
    {
        $sizeOfQueuedPvm = 0x18;
        $queueSize = 8;
        $queue = $this->alloc($queueSize * $sizeOfQueuedPvm);
        $queueTail = $queue + $queueSize * $sizeOfQueuedPvm;

        $this->initUint32($this->addressOf('_var_pvmQueueRear_8c157ac0'), $queue);
        $this->initUint32($this->addressOf('_var_pvmQueueTail_8c157ac4'), $queueTail);

        $dirname = $this->allocString('\\MY_DIR');
        $filename = $this->allocString('MY_FILE.PVM');

        $this->shouldWriteString($queue + 0x00, '\\MY_DIR');
        $this->shouldWriteString($queue + 0x04, 'MY_FILE.PVM');
        $this->shouldWrite($queue + 0x08, 0xcafe0001);
        $this->shouldWrite($queue + 0x0c, 0xcafe0002);
        $this->shouldWrite($queue + 0x10, 0xcafe0003);
        $this->shouldWrite($queue + 0x14, 0);
        $this->shouldWriteTo('_var_pvmQueueRear_8c157ac0', $queue + $sizeOfQueuedPvm);

        $this->singleCall('_AsqRequestPvm_11ac0')
            ->with($dirname, $filename, 0xcafe0001, 0xcafe0002, 0xcafe0003)
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_failsWhenRearIsTail()
    {
        $sizeOfQueuedPvm = 0x18;
        $queueSize = 8;
        $queue = $this->alloc($queueSize * $sizeOfQueuedPvm);
        $queueTail = $queue + $queueSize * $sizeOfQueuedPvm;

        $this->initUint32($this->addressOf('_var_pvmQueueRear_8c157ac0'), $queueTail);
        $this->initUint32($this->addressOf('_var_pvmQueueTail_8c157ac4'), $queueTail);

        $dirname = $this->allocString('\\MY_DIR');
        $filename = $this->allocString('MY_FILE.PVM');

        $this->singleCall('_AsqRequestPvm_11ac0')
            ->with($dirname, $filename)
            ->singleShouldReturn(0)
            ->run();
    }

    public function test_failsWhenFilenameIsEmpty()
    {
        $sizeOfQueuedPvm = 0x18;
        $queueSize = 8;
        $queue = $this->alloc($queueSize * $sizeOfQueuedPvm);
        $queueTail = $queue + $queueSize * $sizeOfQueuedPvm;

        $this->initUint32($this->addressOf('_var_pvmQueueRear_8c157ac0'), $queue);
        $this->initUint32($this->addressOf('_var_pvmQueueTail_8c157ac4'), $queueTail);

        $dirname = $this->allocString('\\MY_DIR');
        $filename = $this->allocString('');

        $this->singleCall('_AsqRequestPvm_11ac0')
            ->with($dirname, $filename)
            ->singleShouldReturn(0)
            ->run();
    }
};
