<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_sortQueuedNjs()
    {
        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $dirStrAddress = $this->allocString('\\DIR');
        $fileAStrAddr = $this->allocString('FILEA.BIN');
        $fileBStrAddr = $this->allocString('FILEB.BIN');
        $fileCStrAddr = $this->allocString('FILEC.BIN');

        // Nj queue has 3 items
        $currentQueuedNj = $njQueue;
        $queuedNjA = $currentQueuedNj;

        $this->initQueuedNj($currentQueuedNj, $dirStrAddress, $fileAStrAddr, 0xcafe0003, 0xcafe0004, 0);
        $queuedNjB = $currentQueuedNj += $sizeOfQueuedNj;
        $this->initQueuedNj($currentQueuedNj, $dirStrAddress, $fileCStrAddr, 0xcafe1003, 0xcafe1004, 0);
        $queuedNjC = $currentQueuedNj += $sizeOfQueuedNj;
        $this->initQueuedNj($currentQueuedNj, $dirStrAddress, $fileBStrAddr, 0xcafe2003, 0xcafe2004, 0);


        $this->initUint32($this->addressOf('_var_njQueue_8c157a9c'), $njQueue);
        $this->initUint32(
            $this->addressOf('_var_njQueueRear_8c157aa0'),
            $njQueue + 3 * $sizeOfQueuedNj
        );

        //

        $this->shouldWriteTo('_var_njQueueIsIdle_8c157aa8', 0);

        $tempQueuedNj = $this->alloc(3 * $sizeOfQueuedNj);
        $this->shouldCall('_syMalloc')
            ->with(3 * $sizeOfQueuedNj)
            ->andReturn($tempQueuedNj);

        // 1st iteration
        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $strCmpFn = function ($params) {
            $this->registers[0] = U32::of(strcmp(
                $this->memory->readString($params[0]),
                $this->memory->readString($params[1])
            ) & 0xffffffff);
        };

        $this->shouldCall($strCmp)
            ->with($fileAStrAddr, $fileCStrAddr)
            ->do($strCmpFn);

        $this->shouldCall($strCmp)
            ->with($fileCStrAddr, $fileBStrAddr)
            ->do($strCmpFn);


        // TODO: Move implementation to Simulator
        $oddMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8($dst->value + $i, $this->readUInt8($src->value + $i));
            }
        };

        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);
        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);
        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);

        $this->shouldCall($strCmp)
            ->with($fileAStrAddr, $fileBStrAddr)
            ->do($strCmpFn);

        $this->shouldCall($strCmp)
            ->with($fileBStrAddr, $fileCStrAddr)
            ->do($strCmpFn);

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

        $this->shouldWrite($createdTask + 0x18, $njQueue);
        $this->shouldWrite($createdTask + 0x08, 0);
        $this->shouldWriteTo('_var_8c157a88', 0);
        $this->shouldWriteStringTo('_var_queueBaseDir_8c157a80', 'DATA EMPTY');

        $this->singleCall('_sortAndLoadNjQueue_8c0116b6')
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_returnsZeroOnEmptyQueue()
    {
        $sizeOfQueuedNj = 0x10;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $this->initUint32($this->addressOf('_var_njQueue_8c157a9c'), $njQueue);
        $this->initUint32(
            $this->addressOf('_var_njQueueRear_8c157aa0'),
            $njQueue,
        );

        $this->singleCall('_sortAndLoadNjQueue_8c0116b6')
            ->singleShouldReturn(0)
            ->run();
    }

    public function test_returnsZeroOnPushFailure()
    {
        $sizeOfQueuedNj = 0x10;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $dirStrAddress = $this->allocString('\\DIR');
        $fileAStrAddr = $this->allocString('FILEA.BIN');

        $currentQueuedNj = $njQueue;
        $queuedNjA = $currentQueuedNj;

        $this->initUint32($this->addressOf('_var_njQueue_8c157a9c'), $njQueue);
        $this->initUint32(
            $this->addressOf('_var_njQueueRear_8c157aa0'),
            $njQueue + 1 * $sizeOfQueuedNj
        );

        //

        $this->shouldWriteTo('_var_njQueueIsIdle_8c157aa8', 0);

        $tempQueuedNj = $this->alloc(4);
        $this->shouldCall('_syMalloc')
            ->with(1 * $sizeOfQueuedNj)
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

        $this->singleCall('_sortAndLoadNjQueue_8c0116b6')
            ->singleShouldReturn(0)
            ->run();
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }

    protected function initQueuedNj(int $address, int $basedir, int $filename, int $dest, int $dest2, int $flag): void
    {
        $this->initUint32($address + 0x00, $basedir); // char* basedir;
        $this->initUint32($address + 0x04, $filename); // char* filename;
        $this->initUint32($address + 0x08, $dest); // void* dest;
        $this->initUint32($address + 0x0c, $dest2); // void* dest;
        $this->initUint32($address + 0x10, $flag); // int field_0x10;
    }
};
