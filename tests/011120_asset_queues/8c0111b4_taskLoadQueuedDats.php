<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_case0_readsFirstItem()
    {
        $this->resolveImports();

        $sizeOfQueuedDat = 0x10;
        $queueSize = 16;
        $datQueue = $this->alloc($queueSize * $sizeOfQueuedDat);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirStrAddress = $this->allocString('\\DIR');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        // Dat queue has 3 items
        $currentQueuedDat = $datQueue;
        $dat1Dest = $this->alloc(4);
        $this->initUint32($currentQueuedDat + 0x00, $dirStrAddress); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, $dat1Dest); // void* dest;
        $this->initUint32($currentQueuedDat + 0x0c, 0); // int loaded_0x0c;

        $currentQueuedDat += $sizeOfQueuedDat;
        $this->initUint32($currentQueuedDat + 0x00, 0xcafe0001); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, 0xcafe0003); // void* dest;
        $this->initUint32($currentQueuedDat + 0x0c, 0); // int loaded_0x0c;

        $currentQueuedDat += $sizeOfQueuedDat;
        $this->initUint32($currentQueuedDat + 0x00, 0xcafe0001); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, 0xcafe0003); // void* dest;
        $this->initUint32($currentQueuedDat + 0x0c, 0); // int loaded_0x0c;

        $this->initUint32(
            $this->addressOf('_var_datQueueRear_8c157a90'),
            $datQueue + 3 * $sizeOfQueuedDat
        );

        $taskPtr = $this->alloc(0x20);
        // task->phase_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $datQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffdc : 0xffffd8;

        /// First iteration


        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dataEmptyStrAddress, $dirStrAddress)
            ->andReturn(1);

        $this->shouldWriteTo('_var_queueBaseDir_8c157a80', $dirStrAddress);
        $this->shouldCall('_gdFsChangeDir')->with($dirStrAddress);

        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe0002, 0)
            ->andReturn(0xf5f50000);

        // task->gdfs_0x0c = gdFsOpen(...)
        $this->shouldWrite($taskPtr + 0x0c, 0xf5f50000);

        $this->shouldCall('_gdFsGetFileSctSize')
            ->with(0xf5f50000, $sizeLocal)
            ->do(function (...$params) use ($sizeLocal) {
                $this->writeUInt32($sizeLocal, 0, U32::of(5));
            })
            ->andReturn(1);

        $this->shouldCall('_syMalloc')
            ->with(5 * 2048)
            ->andReturn(0xbebacafe);
        $this->shouldWrite($dat1Dest, 0xbebacafe);

        $this->shouldCall('_gdFsRead')
            ->with(0xf5f50000, 5, 0xbebacafe)
            ->andReturn(0); // GDD_ERR_OK

        $this->shouldCall('_gdFsClose')
            ->with(0xf5f50000);

        $this->shouldWrite($datQueue + 0x0c, 1);
        $this->shouldWrite($taskPtr + 0x18, $datQueue + $sizeOfQueuedDat);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedDats_8c0111b4')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case0_skipsItemsWith0x0cFlagSet()
    {
        $this->resolveImports();

        $sizeOfQueuedDat = 0x10;
        $queueSize = 16;
        $datQueue = $this->alloc($queueSize * $sizeOfQueuedDat);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirStrAddress = $this->allocString('\\DIR');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        // Dat queue has 3 items
        $currentQueuedDat = $datQueue;
        $dat1Dest = $this->alloc(4);

        // This will be skipped
        $this->initUint32($currentQueuedDat + 0x00, 0xcafe0001); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, 0xcafe0003); // void* dest;
        $this->initUint32($currentQueuedDat + 0x0c, 1); // int loaded_0x0c;

        // This will be read
        $currentQueuedDat += $sizeOfQueuedDat;
        $this->initUint32($currentQueuedDat + 0x00, $dirStrAddress); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, $dat1Dest); // void* dest;
        $this->initUint32($currentQueuedDat + 0x0c, 0); // int loaded_0x0c;

        $currentQueuedDat += $sizeOfQueuedDat;
        $this->initUint32($currentQueuedDat + 0x00, 0xcafe0001); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, 0xcafe0003); // void* dest;
        $this->initUint32($currentQueuedDat + 0x0c, 0); // int loaded_0x0c;

        $this->initUint32(
            $this->addressOf('_var_datQueueRear_8c157a90'),
            $datQueue + 3 * $sizeOfQueuedDat
        );

        $testQueuedDat = $datQueue + $sizeOfQueuedDat;

        $taskPtr = $this->alloc(0x20);
        // task->phase_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $datQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffdc : 0xffffd8;

        /// First iteration


        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dataEmptyStrAddress, $dirStrAddress)
            ->andReturn(1);

        $this->shouldWriteTo('_var_queueBaseDir_8c157a80', $dirStrAddress);
        $this->shouldCall('_gdFsChangeDir')->with($dirStrAddress);

        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe0002, 0)
            ->andReturn(0xf5f50000);

        // task->gdfs_0x0c = gdFsOpen(...)
        $this->shouldWrite($taskPtr + 0x0c, 0xf5f50000);

        $this->shouldCall('_gdFsGetFileSctSize')
            ->with(0xf5f50000, $sizeLocal)
            ->do(function (...$params) use ($sizeLocal) {
                $this->writeUInt32($sizeLocal, 0, U32::of(5));
            })
            ->andReturn(1);

        $this->shouldCall('_syMalloc')
            ->with(5 * 2048)
            ->andReturn(0xbebacafe);
        $this->shouldWrite($dat1Dest, 0xbebacafe);

        $this->shouldCall('_gdFsRead')
            ->with(0xf5f50000, 5, 0xbebacafe)
            ->andReturn(0); // GDD_ERR_OK

        $this->shouldCall('_gdFsClose')
            ->with(0xf5f50000);

        $this->shouldWrite($testQueuedDat + 0x0c, 1);
        $this->shouldWrite($taskPtr + 0x18, $testQueuedDat + $sizeOfQueuedDat);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedDats_8c0111b4')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case0_breaksOnQueueRear()
    {
        $this->resolveImports();

        $sizeOfQueuedDat = 0x10;
        $queueSize = 16;
        $datQueue = $this->alloc($queueSize * $sizeOfQueuedDat);
        $this->initUint32($this->addressOf('_var_datQueue_8c157a8c'), $datQueue);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirStrAddress = $this->allocString('\\DIR');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        // Dat queue has 3 items
        $currentQueuedDat = $datQueue;
        $dat1Dest = $this->alloc(4);

        $this->initUint32($currentQueuedDat + 0x00, 0xcafe0001); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, 0xcafe0003); // void* dest;
        $this->initUint32($currentQueuedDat + 0x0c, 1); // int loaded_0x0c;

        $currentQueuedDat += $sizeOfQueuedDat;
        $this->initUint32($currentQueuedDat + 0x00, 0xcafe0001); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, 0xcafe0003); // void* dest;
        $this->initUint32($currentQueuedDat + 0x0c, 1); // int loaded_0x0c;

        $currentQueuedDat += $sizeOfQueuedDat;
        $this->initUint32($currentQueuedDat + 0x00, $dirStrAddress); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, $dat1Dest); // void* dest;
        $this->initUint32($currentQueuedDat + 0x0c, 1); // int loaded_0x0c;

        $this->initUint32($this->addressOf('_var_loadRetryNeeded_8c157a88'), 1);

        $this->initUint32(
            $this->addressOf('_var_datQueueRear_8c157a90'),
            $datQueue + 3 * $sizeOfQueuedDat
        );

        $taskPtr = $this->alloc(0x20);
        // task->phase_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $datQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffdc : 0xffffd8;

        $this->shouldReadFrom('_var_loadRetryNeeded_8c157a88', 1);
        $this->shouldWrite($taskPtr + 0x18, $datQueue);
        $this->shouldWriteTo('_var_loadRetryNeeded_8c157a88', 0);
        $this->shouldWriteStringTo('_var_queueBaseDir_8c157a80', 'DATA EMPTY');

        $this->singleCall('_taskLoadQueuedDats_8c0111b4')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case0_breaksOnQueueCursorAndFreeTask()
    {
        $this->doNotRandomizeMemory();

        $this->resolveImports();

        $sizeOfQueuedDat = 0x10;
        $queueSize = 16;
        $datQueue = $this->alloc($queueSize * $sizeOfQueuedDat);
        $this->initUint32($this->addressOf('_var_datQueue_8c157a8c'), $datQueue);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirStrAddress = $this->allocString('\\DIR');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        // Dat queue has 3 items
        $currentQueuedDat = $datQueue;
        $dat1Dest = $this->alloc(4);

        $this->initUint32($currentQueuedDat + 0x00, 0xcafe0001); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, 0xcafe0003); // void* dest;
        $this->initUint32($currentQueuedDat + 0x0c, 1); // int loaded_0x0c;

        $currentQueuedDat += $sizeOfQueuedDat;
        $this->initUint32($currentQueuedDat + 0x00, 0xcafe0001); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, 0xcafe0003); // void* dest;
        $this->initUint32($currentQueuedDat + 0x0c, 1); // int loaded_0x0c;

        $currentQueuedDat += $sizeOfQueuedDat;
        $this->initUint32($currentQueuedDat + 0x00, $dirStrAddress); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, $dat1Dest); // void* dest;
        $this->initUint32($currentQueuedDat + 0x0c, 1); // int loaded_0x0c;

        $this->initUint32(
            $this->addressOf('_var_datQueueRear_8c157a90'),
            $datQueue + 3 * $sizeOfQueuedDat
        );

        $taskPtr = $this->alloc(0x20);
        // task->phase_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $datQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffdc : 0xffffd8;

        $this->shouldReadFrom('_var_loadRetryNeeded_8c157a88', 0);
        $this->shouldWriteTo('_var_datQueueIsIdle_8c157a98', 1);
        $this->shouldCall('_TaskUninstall_8c014b66')
            ->with($taskPtr);

        $this->singleCall('_taskLoadQueuedDats_8c0111b4')
            ->with($taskPtr, 0)
            ->run();
    }

    /*
     * The three case-0 failure paths below share one epilogue (L17/L18 in the
     * archived asm): set the retry flag, step the cursor past the item that
     * failed, go back to phase 0.
     */

    public function test_case0_openFailureMarksRetryAndAdvances()
    {
        $this->resolveImports();

        $sizeOfQueuedDat = 0x10;
        $datQueue = $this->alloc(16 * $sizeOfQueuedDat);

        $dirStrAddress = $this->allocString('\\DIR');
        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dirStrAddress);

        $this->initQueuedDat($datQueue, $dirStrAddress, 0xcafe0002, 0xcafe0003, 0);
        $this->initUint32(
            $this->addressOf('_var_datQueueRear_8c157a90'),
            $datQueue + 1 * $sizeOfQueuedDat
        );

        $taskPtr = $this->alloc(0x20);
        $this->initUint32($taskPtr + 0x08, 0);
        $this->initUint32($taskPtr + 0x18, $datQueue);

        $this->call('_taskLoadQueuedDats_8c0111b4')->with($taskPtr, 0);

        // Already in the item's basedir, so no gdFsChangeDir.
        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dirStrAddress, $dirStrAddress)
            ->andReturn(0);

        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe0002, 0)
            ->andReturn(0);
        $this->shouldWrite($taskPtr + 0x0c, 0);

        $this->shouldWriteTo('_var_loadRetryNeeded_8c157a88', 1);
        $this->shouldWrite($taskPtr + 0x18, $datQueue + 1 * $sizeOfQueuedDat);
        $this->shouldWrite($taskPtr + 0x08, 0);
    }

    public function test_case0_sizeQueryFailureMarksRetryAndAdvances()
    {
        $this->resolveImports();

        $sizeOfQueuedDat = 0x10;
        $datQueue = $this->alloc(16 * $sizeOfQueuedDat);

        $dirStrAddress = $this->allocString('\\DIR');
        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dirStrAddress);

        $this->initQueuedDat($datQueue, $dirStrAddress, 0xcafe0002, 0xcafe0003, 0);
        $this->initUint32(
            $this->addressOf('_var_datQueueRear_8c157a90'),
            $datQueue + 1 * $sizeOfQueuedDat
        );

        $taskPtr = $this->alloc(0x20);
        $this->initUint32($taskPtr + 0x08, 0);
        $this->initUint32($taskPtr + 0x18, $datQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffdc : 0xffffd8;

        $this->call('_taskLoadQueuedDats_8c0111b4')->with($taskPtr, 0);

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dirStrAddress, $dirStrAddress)
            ->andReturn(0);

        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe0002, 0)
            ->andReturn(0xf5f50000);
        $this->shouldWrite($taskPtr + 0x0c, 0xf5f50000);

        $this->shouldCall('_gdFsGetFileSctSize')
            ->with(0xf5f50000, $sizeLocal)
            ->andReturn(0);

        // No syMalloc, no gdFsRead: the file is abandoned unopened-for-read.
        $this->shouldWriteTo('_var_loadRetryNeeded_8c157a88', 1);
        $this->shouldWrite($taskPtr + 0x18, $datQueue + 1 * $sizeOfQueuedDat);
        $this->shouldWrite($taskPtr + 0x08, 0);
    }

    public function test_case0_readFailureMarksRetryAndLeavesTheBufferAllocated()
    {
        $this->resolveImports();

        $sizeOfQueuedDat = 0x10;
        $datQueue = $this->alloc(16 * $sizeOfQueuedDat);

        $dirStrAddress = $this->allocString('\\DIR');
        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dirStrAddress);

        $dat1Dest = $this->alloc(4);
        $this->initQueuedDat($datQueue, $dirStrAddress, 0xcafe0002, $dat1Dest, 0);
        $this->initUint32(
            $this->addressOf('_var_datQueueRear_8c157a90'),
            $datQueue + 1 * $sizeOfQueuedDat
        );

        $taskPtr = $this->alloc(0x20);
        $this->initUint32($taskPtr + 0x08, 0);
        $this->initUint32($taskPtr + 0x18, $datQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffdc : 0xffffd8;

        $this->call('_taskLoadQueuedDats_8c0111b4')->with($taskPtr, 0);

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dirStrAddress, $dirStrAddress)
            ->andReturn(0);

        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe0002, 0)
            ->andReturn(0xf5f50000);
        $this->shouldWrite($taskPtr + 0x0c, 0xf5f50000);

        $this->shouldCall('_gdFsGetFileSctSize')
            ->with(0xf5f50000, $sizeLocal)
            ->do(function (...$params) use ($sizeLocal) {
                $this->writeUInt32($sizeLocal, 0, U32::of(5));
            })
            ->andReturn(1);

        $this->shouldCall('_syMalloc')
            ->with(5 * 2048)
            ->andReturn(0xbebacafe);
        $this->shouldWrite($dat1Dest, 0xbebacafe);

        $this->shouldCall('_gdFsRead')
            ->with(0xf5f50000, 5, 0xbebacafe)
            ->andReturn(1); // not GDD_ERR_OK

        // The destination buffer is neither freed nor cleared; the retry pass
        // mallocs another one over it.
        $this->shouldWriteTo('_var_loadRetryNeeded_8c157a88', 1);
        $this->shouldWrite($taskPtr + 0x18, $datQueue + 1 * $sizeOfQueuedDat);
        $this->shouldWrite($taskPtr + 0x08, 0);
    }

    public function test_case0_failureAdvancesPastTheFailedItemNotTheTaskCursor()
    {
        $this->resolveImports();

        $sizeOfQueuedDat = 0x10;
        $datQueue = $this->alloc(16 * $sizeOfQueuedDat);

        $dirStrAddress = $this->allocString('\\DIR');
        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dirStrAddress);

        // Item 0 is already loaded, so the scan skips it and fails on item 1.
        $this->initQueuedDat($datQueue, 0xcafe0001, 0xcafe0002, 0xcafe0003, 1);
        $this->initQueuedDat($datQueue + 1 * $sizeOfQueuedDat, $dirStrAddress, 0xcafe1002, 0xcafe1003, 0);
        $this->initUint32(
            $this->addressOf('_var_datQueueRear_8c157a90'),
            $datQueue + 2 * $sizeOfQueuedDat
        );

        $taskPtr = $this->alloc(0x20);
        $this->initUint32($taskPtr + 0x08, 0);
        $this->initUint32($taskPtr + 0x18, $datQueue);

        $this->call('_taskLoadQueuedDats_8c0111b4')->with($taskPtr, 0);

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dirStrAddress, $dirStrAddress)
            ->andReturn(0);

        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe1002, 0)
            ->andReturn(0);
        $this->shouldWrite($taskPtr + 0x0c, 0);

        $this->shouldWriteTo('_var_loadRetryNeeded_8c157a88', 1);
        $this->shouldWrite($taskPtr + 0x18, $datQueue + 2 * $sizeOfQueuedDat);
        $this->shouldWrite($taskPtr + 0x08, 0);
    }

    public function test_case1_gdfsStatComplete()
    {
        $this->resolveImports();

        $sizeOfQueuedDat = 0x10;
        $queueSize = 16;
        $datQueue = $this->alloc($queueSize * $sizeOfQueuedDat);
        $this->initUint32($this->addressOf('_var_datQueue_8c157a8c'), $datQueue);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirStrAddress = $this->allocString('\\DIR');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        $taskPtr = $this->alloc(0x20);
        // task->phase_0x08
        $this->initUint32($taskPtr + 0x08, 1);
        // task->gdfs_0x0c
        $this->initUint32($taskPtr + 0x0c, 0xbebacafe);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $datQueue);

        $this->shouldCall('_gdFsGetStat')
            ->with(0xbebacafe)
            ->andReturn(1); // GDD_STAT_COMPLETE
        $this->shouldCall('_gdFsClose')
            ->with(0xbebacafe);
        $this->shouldWrite($datQueue + 0x0c, 1);
        $this->shouldWrite($taskPtr + 0x18, $datQueue + $sizeOfQueuedDat);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedDats_8c0111b4')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case1_gdfsStatRead()
    {
        $this->resolveImports();

        $sizeOfQueuedDat = 0x10;
        $queueSize = 16;
        $datQueue = $this->alloc($queueSize * $sizeOfQueuedDat);
        $this->initUint32($this->addressOf('_var_datQueue_8c157a8c'), $datQueue);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirStrAddress = $this->allocString('\\DIR');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        $currentQueuedDat = $datQueue;
        $dat1Dest = $this->alloc(4);
        $this->initUint32($dat1Dest, 0xcafe1000);
        $this->initUint32($currentQueuedDat + 0x00, $dirStrAddress); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, $dat1Dest); // void** dest;
        $this->initUint32($currentQueuedDat + 0x0c, 0); // int loaded_0x0c;

        $taskPtr = $this->alloc(0x20);
        // task->phase_0x08
        $this->initUint32($taskPtr + 0x08, 1);
        // task->gdfs_0x0c
        $this->initUint32($taskPtr + 0x0c, 0xbebacafe);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $datQueue);

        $this->shouldCall('_gdFsGetStat')
            ->with(0xbebacafe)
            ->andReturn(2); // GDD_STAT_READ
        $this->shouldCall('_gdFsGetTransStat')
            ->with(0xbebacafe)
            ->andReturn(0); // GDD_FS_TRANS_READY
        $this->shouldCall('_gdFsTrans32')
            ->with(0xbebacafe, 2048, 0xcafe1000);

        $this->singleCall('_taskLoadQueuedDats_8c0111b4')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case1_gdfsStatRead_gdfsFsTransReady()
    {
        $this->resolveImports();

        $sizeOfQueuedDat = 0x10;
        $queueSize = 16;
        $datQueue = $this->alloc($queueSize * $sizeOfQueuedDat);
        $this->initUint32($this->addressOf('_var_datQueue_8c157a8c'), $datQueue);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirStrAddress = $this->allocString('\\DIR');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        $currentQueuedDat = $datQueue;
        $dat1Dest = $this->alloc(4);
        $this->initUint32($dat1Dest, 0xcafe1000);
        $this->initUint32($currentQueuedDat + 0x00, $dirStrAddress); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, $dat1Dest); // void** dest;
        $this->initUint32($currentQueuedDat + 0x0c, 0); // int loaded_0x0c;

        $taskPtr = $this->alloc(0x20);
        // task->phase_0x08
        $this->initUint32($taskPtr + 0x08, 1);
        // task->gdfs_0x0c
        $this->initUint32($taskPtr + 0x0c, 0xbebacafe);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $datQueue);

        $this->shouldCall('_gdFsGetStat')
            ->with(0xbebacafe)
            ->andReturn(2); // GDD_STAT_READ
        $this->shouldCall('_gdFsGetTransStat')
            ->with(0xbebacafe)
            ->andReturn(0); // GDD_FS_TRANS_READY
        $this->shouldCall('_gdFsTrans32')
            ->with(0xbebacafe, 2048, 0xcafe1000);

        $this->singleCall('_taskLoadQueuedDats_8c0111b4')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case1_gdfsStatRead_gdfsFsTransBusy()
    {
        $this->resolveImports();

        $sizeOfQueuedDat = 0x10;
        $queueSize = 16;
        $datQueue = $this->alloc($queueSize * $sizeOfQueuedDat);
        $this->initUint32($this->addressOf('_var_datQueue_8c157a8c'), $datQueue);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirStrAddress = $this->allocString('\\DIR');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        $currentQueuedDat = $datQueue;
        $dat1Dest = $this->alloc(4);
        $this->initUint32($dat1Dest, 0xcafe1000);
        $this->initUint32($currentQueuedDat + 0x00, $dirStrAddress); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, $dat1Dest); // void** dest;
        $this->initUint32($currentQueuedDat + 0x0c, 0); // int loaded_0x0c;

        $taskPtr = $this->alloc(0x20);
        // task->phase_0x08
        $this->initUint32($taskPtr + 0x08, 1);
        // task->gdfs_0x0c
        $this->initUint32($taskPtr + 0x0c, 0xbebacafe);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $datQueue);

        $this->shouldCall('_gdFsGetStat')
            ->with(0xbebacafe)
            ->andReturn(2); // GDD_STAT_READ
        $this->shouldCall('_gdFsGetTransStat')
            ->with(0xbebacafe)
            ->andReturn(1); // GDD_FS_TRANS_BUSY

        $this->singleCall('_taskLoadQueuedDats_8c0111b4')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case1_gdfsStatBusy()
    {
        $this->resolveImports();

        $sizeOfQueuedDat = 0x10;
        $queueSize = 16;
        $datQueue = $this->alloc($queueSize * $sizeOfQueuedDat);
        $this->initUint32($this->addressOf('_var_datQueue_8c157a8c'), $datQueue);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirStrAddress = $this->allocString('\\DIR');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        $currentQueuedDat = $datQueue;
        $dat1Dest = $this->alloc(4);
        $this->initUint32($dat1Dest, 0xcafe1000);
        $this->initUint32($currentQueuedDat + 0x00, $dirStrAddress); // char* basedir;
        $this->initUint32($currentQueuedDat + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedDat + 0x08, $dat1Dest); // void** dest;
        $this->initUint32($currentQueuedDat + 0x0c, 0); // int loaded_0x0c;

        $taskPtr = $this->alloc(0x20);
        // task->phase_0x08
        $this->initUint32($taskPtr + 0x08, 1);
        // task->gdfs_0x0c
        $this->initUint32($taskPtr + 0x0c, 0xbebacafe);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $datQueue);

        $this->shouldCall('_gdFsGetStat')
            ->with(0xbebacafe)
            ->andReturn(4); // GDD_STAT_BUSY
        $this->shouldCall('_gdFsClose')
            ->with(0xbebacafe);
        $this->shouldCall('_syFree')->with(0xcafe1000);

        $this->shouldWriteTo('_var_loadRetryNeeded_8c157a88', 1);
        $this->shouldWrite($taskPtr + 0x18, $datQueue + $sizeOfQueuedDat);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedDats_8c0111b4')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_nocase()
    {
        $this->resolveImports();

        $sizeOfQueuedDat = 0x10;
        $queueSize = 16;
        $datQueue = $this->alloc($queueSize * $sizeOfQueuedDat);
        $this->initUint32($this->addressOf('_var_datQueue_8c157a8c'), $datQueue);

        $taskPtr = $this->alloc(0x20);
        // task->phase_0x08
        $this->initUint32($taskPtr + 0x08, 2);

        $this->singleCall('_taskLoadQueuedDats_8c0111b4')
            ->with($taskPtr, 0)
            ->run();
    }

    protected function initQueuedDat(int $address, int $basedir, int $filename, int $dest, int $loaded): void
    {
        $this->initUint32($address + 0x00, $basedir);
        $this->initUint32($address + 0x04, $filename);
        $this->initUint32($address + 0x08, $dest);
        $this->initUint32($address + 0x0c, $loaded);
    }

    private function resolveImports()
    {
        // Functions
        $this->setSize('_gdFsClose', 4);
        $this->setSize('_TaskUninstall_8c014b66', 4);
    }

    protected function isAsmObject(): bool
    {
        return str_contains($this->objectFile, '/asm/');
    }
};
