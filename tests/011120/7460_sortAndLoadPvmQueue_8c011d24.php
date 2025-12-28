<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;

return new class extends TestCase {
    public function test_sortQueuedPvms()
    {
        $sizeOfQueuedPvm = 0x18;
        $queueSize = 16;
        $pvmQueue = $this->alloc($queueSize * $sizeOfQueuedPvm);

        $dirStrAddress = $this->allocString('\\DIR');
        $fileAStrAddr = $this->allocString('FILEA.BIN');
        $fileBStrAddr = $this->allocString('FILEB.BIN');
        $fileCStrAddr = $this->allocString('FILEC.BIN');

        // Pvm queue has 3 items. We init filename only.
        $currentQueuedPvm = $pvmQueue;
        $this->initUint32($currentQueuedPvm + 0x04, $fileAStrAddr);
        $currentQueuedPvm += $sizeOfQueuedPvm;
        $this->initUint32($currentQueuedPvm + 0x04, $fileCStrAddr);
        $currentQueuedPvm += $sizeOfQueuedPvm;
        $this->initUint32($currentQueuedPvm + 0x04, $fileBStrAddr);

        $this->initUint32($this->addressOf('_var_pvmQueue_8c157abc'), $pvmQueue);
        $this->initUint32(
            $this->addressOf('_var_pvmQueueRear_8c157ac0'),
            $pvmQueue + 3 * $sizeOfQueuedPvm
        );

        $this->initUint32($this->addressOf('_var_8c157a6c'), 1);

        $this->shouldWriteTo('_var_pvmQueueIsIdle_8c157ac8', 0);

        $tempQueuedNj = $this->alloc(3 * $sizeOfQueuedPvm);
        $this->shouldCall('_syMalloc')
            ->with(3 * $sizeOfQueuedPvm)
            ->andReturn($tempQueuedNj);

        // 1st iteration
        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($fileAStrAddr, $fileCStrAddr)
            ->andReturn(strcmp('FILEA.BIN', 'FILEC.BIN'));

        $this->shouldCall($strCmp)
            ->with($fileCStrAddr, $fileBStrAddr)
            ->andReturn(strcmp('FILEC.BIN', 'FILEB.BIN'));

        // TODO: Move implementation to Simulator
        $evnMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8($dst->value + $i, $this->readUInt8($src->value + $i));
            }
        };

        $this->shouldCall('__quick_evn_mvn')->do($evnMvn);
        $this->shouldCall('__quick_evn_mvn')->do($evnMvn);
        $this->shouldCall('__quick_evn_mvn')->do($evnMvn);

        $this->shouldCall($strCmp)
            ->with($fileAStrAddr, $fileBStrAddr)
            ->andReturn(strcmp('FILEA.BIN', 'FILEB.BIN'));

        $this->shouldCall($strCmp)
            ->with($fileBStrAddr, $fileCStrAddr)
            ->andReturn(strcmp('FILEB.BIN', 'FILEC.BIN'));

        $this->shouldCall('_syFree')
            ->with($tempQueuedNj);

        $createdTask = $this->alloc(0x1c);
        $this->initUint32(0xffffd4, $createdTask);
        $this->shouldCall('_pushTask_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                new WildcardArgument(), // TODO: Make addressOf handle exports
                0xffffd4,
                0xffffd8,
                0
            )
            ->andReturn(1);

        $this->shouldWrite($createdTask + 0x18, $pvmQueue);
        $this->shouldWrite($createdTask + 0x08, 0);
        $this->shouldWriteTo('_var_8c157a88', 0);
        $this->shouldWriteStringTo('_var_queueBaseDir_8c157a80', 'DATA EMPTY');

        $this->singleCall('_sortAndLoadPvmQueue_8c011d24')
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_doNotSortWhen8c157a6cIsZero()
    {
        $sizeOfQueuedPvm = 0x18;
        $queueSize = 16;
        $pvmQueue = $this->alloc($queueSize * $sizeOfQueuedPvm);

        $this->initUint32($this->addressOf('_var_pvmQueue_8c157abc'), $pvmQueue);
        $this->initUint32(
            $this->addressOf('_var_pvmQueueRear_8c157ac0'),
            $pvmQueue + 3 * $sizeOfQueuedPvm
        );

        $this->initUint32($this->addressOf('_var_8c157a6c'), 0);

        $this->shouldWriteTo('_var_pvmQueueIsIdle_8c157ac8', 0);

        $tempQueuedNj = $this->alloc(3 * $sizeOfQueuedPvm);
        $this->shouldCall('_syMalloc')
            ->with(3 * $sizeOfQueuedPvm)
            ->andReturn($tempQueuedNj);

        $this->shouldCall('_syFree')
            ->with($tempQueuedNj);

        $createdTask = $this->alloc(0x1c);
        $this->initUint32(0xffffd4, $createdTask);
        $this->shouldCall('_pushTask_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                new WildcardArgument(), // TODO: Make addressOf handle exports
                0xffffd4,
                0xffffd8,
                0
            )
            ->andReturn(1);

        $this->shouldWrite($createdTask + 0x18, $pvmQueue);
        $this->shouldWrite($createdTask + 0x08, 0);
        $this->shouldWriteTo('_var_8c157a88', 0);
        $this->shouldWriteStringTo('_var_queueBaseDir_8c157a80', 'DATA EMPTY');

        $this->singleCall('_sortAndLoadPvmQueue_8c011d24')
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_returnsZeroOnEmptyQueue()
    {
        $sizeOfQueuedPvm = 0x18;
        $queueSize = 16;
        $pvmQueue = $this->alloc($queueSize * $sizeOfQueuedPvm);

        $this->initUint32($this->addressOf('_var_pvmQueue_8c157abc'), $pvmQueue);
        $this->initUint32(
            $this->addressOf('_var_pvmQueueRear_8c157ac0'),
            $pvmQueue,
        );

        $this->singleCall('_sortAndLoadPvmQueue_8c011d24')
            ->singleShouldReturn(0)
            ->run();
    }

    public function test_returnsZeroOnPushFailure()
    {
        $sizeOfQueuedPvm = 0x18;
        $queueSize = 16;
        $pvmQueue = $this->alloc($queueSize * $sizeOfQueuedPvm);

        $dirStrAddress = $this->allocString('\\DIR');
        $fileAStrAddr = $this->allocString('FILEA.BIN');

        $currentQueuedPvm = $pvmQueue;
        $queuedPvmA = $currentQueuedPvm;

        $this->initUint32($this->addressOf('_var_pvmQueue_8c157abc'), $pvmQueue);
        $this->initUint32(
            $this->addressOf('_var_pvmQueueRear_8c157ac0'),
            $pvmQueue + 1 * $sizeOfQueuedPvm
        );

        $this->initUint32($this->addressOf('_var_8c157a6c'), 1);

        $this->shouldWriteTo('_var_pvmQueueIsIdle_8c157ac8', 0);

        $tempQueuedNj = $this->alloc(4);
        $this->shouldCall('_syMalloc')
            ->with(1 * $sizeOfQueuedPvm)
            ->andReturn($tempQueuedNj);

        $this->shouldCall('_syFree')
            ->with($tempQueuedNj);

        $createdTask = $this->alloc(0x1c);
        $this->initUint32(0xffffd4, $createdTask);
        $this->shouldCall('_pushTask_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                new WildcardArgument(), // TODO: Make addressOf handle exports
                0xffffd4,
                0xffffd8,
                0
            )
            ->andReturn(0);

        $this->singleCall('_sortAndLoadPvmQueue_8c011d24')
            ->singleShouldReturn(0)
            ->run();
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
