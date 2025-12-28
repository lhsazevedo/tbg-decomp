<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;

return new class extends TestCase {
    public function test_simpleTest()
    {
        $sizeOfQueuedDat = 0x10;
        $queueSize = 16;
        $datQueue = $this->alloc($queueSize * $sizeOfQueuedDat);

        $dirStrAddress = $this->allocString('\\DIR');
        $fileAStrAddr = $this->allocString('FILEA.BIN');
        $fileBStrAddr = $this->allocString('FILEB.BIN');
        $fileCStrAddr = $this->allocString('FILEC.BIN');

        // Dat queue has 3 items
        $currentQueuedDat = $datQueue;
        $queuedDatA = $currentQueuedDat;
        $dat1Dest = $this->alloc(4);
        $this->initUint32($currentQueuedDat + 0x00, $dirStrAddress); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, $fileAStrAddr); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, $dat1Dest); // void* dest;
        $this->initUint32($currentQueuedDat + 0x0c, 0); // int field_0x0c;

        $currentQueuedDat += $sizeOfQueuedDat;
        $queuedDatB = $currentQueuedDat;
        $this->initUint32($currentQueuedDat + 0x00, $dirStrAddress); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, $fileCStrAddr); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, 0xcafe0003); // void* dest;
        $this->initUint32($currentQueuedDat + 0x0c, 0); // int field_0x0c;

        $currentQueuedDat += $sizeOfQueuedDat;
        $queuedDatC = $currentQueuedDat;
        $this->initUint32($currentQueuedDat + 0x00, $dirStrAddress); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, $fileBStrAddr); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, 0xcafe0003); // void* dest;
        $this->initUint32($currentQueuedDat + 0x0c, 0); // int field_0x0c;

        $this->initUint32($this->addressOf('_var_datQueue_8c157a8c'), $datQueue);
        $this->initUint32(
            $this->addressOf('_var_datQueueRear_8c157a90'),
            $datQueue + 3 * $sizeOfQueuedDat
        );

        //

        $this->shouldWriteTo('_var_datQueueIsIdle_8c157a98', 0);

        $tempQueuedDat = $this->alloc(4);
        $this->shouldCall('_syMalloc')
            ->with(3 * $sizeOfQueuedDat)
            ->andReturn($tempQueuedDat);

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
            ->with($tempQueuedDat);

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

        $this->shouldWrite($createdTask + 0x18, $datQueue);
        $this->shouldWrite($createdTask + 0x08, 0);
        $this->shouldWriteTo('_var_8c157a88', 0);
        $this->shouldWriteStringTo('_var_queueBaseDir_8c157a80', 'DATA EMPTY');

        $this->singleCall('_sortAndLoadDatQueue_8c011310')
            ->singleShouldReturn(1)
            ->run();
    }

    public function test_returnsZeroOnEmptyQueue()
    {
        $sizeOfQueuedDat = 0x10;
        $queueSize = 16;
        $datQueue = $this->alloc($queueSize * $sizeOfQueuedDat);

        $this->initUint32($this->addressOf('_var_datQueue_8c157a8c'), $datQueue);
        $this->initUint32(
            $this->addressOf('_var_datQueueRear_8c157a90'),
            $datQueue,
        );

        $this->singleCall('_sortAndLoadDatQueue_8c011310')
            ->singleShouldReturn(0)
            ->run();
    }

    public function test_returnsZeroOnPushFailure()
    {
        $sizeOfQueuedDat = 0x10;
        $queueSize = 16;
        $datQueue = $this->alloc($queueSize * $sizeOfQueuedDat);

        $dirStrAddress = $this->allocString('\\DIR');
        $fileAStrAddr = $this->allocString('FILEA.BIN');

        $currentQueuedDat = $datQueue;
        $queuedDatA = $currentQueuedDat;
        $dat1Dest = $this->alloc(4);
        $this->initUint32($currentQueuedDat + 0x00, $dirStrAddress); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, $fileAStrAddr); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, $dat1Dest); // void* dest;
        $this->initUint32($currentQueuedDat + 0x0c, 0); // int field_0x0c;

        $this->initUint32($this->addressOf('_var_datQueue_8c157a8c'), $datQueue);
        $this->initUint32(
            $this->addressOf('_var_datQueueRear_8c157a90'),
            $datQueue + 1 * $sizeOfQueuedDat
        );

        //

        $this->shouldWriteTo('_var_datQueueIsIdle_8c157a98', 0);

        $tempQueuedDat = $this->alloc(4);
        $this->shouldCall('_syMalloc')
            ->with(1 * $sizeOfQueuedDat)
            ->andReturn($tempQueuedDat);

        $this->shouldCall('_syFree')
            ->with($tempQueuedDat);

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

        $this->singleCall('_sortAndLoadDatQueue_8c011310')
            ->singleShouldReturn(0)
            ->run();
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
