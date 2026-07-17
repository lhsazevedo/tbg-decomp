<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_case0_readsFirstItem()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirAStrAddress = $this->allocString('\\DIR_A');
        $dirBStrAddress = $this->allocString('\\DIR_B');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        // Nj queue has 3 items
        $currentQueuedNj = $njQueue;
        $nj1Dest = $this->alloc(4);
        $nj1Dest2 = $this->alloc(4);
        $this->initQueuedNj(
            address:  $currentQueuedNj,
            basedir:  $dirAStrAddress,
            filename: 0xcafe0002,
            dest:     $nj1Dest,
            dest2:    $nj1Dest2,
            flag:     0,
        );

        $currentQueuedNj += $sizeOfQueuedNj;
        $this->initQueuedNj(
            address:  $currentQueuedNj,
            basedir:  $dirBStrAddress,
            filename: 0xcafe1002,
            dest:     0xcafe1003,
            dest2:    0xcafe1004,
            flag:     0,
        );

        $currentQueuedNj += $sizeOfQueuedNj;
        $this->initQueuedNj(
            address:  $currentQueuedNj,
            basedir:  0xcafe2001,
            filename: 0xcafe2002,
            dest:     0xcafe2003,
            dest2:    0xcafe2004,
            flag:     0,
        );

        $this->initUint32(
            $this->addressOf('_var_njQueueRear_8c157aa0'),
            $njQueue + 3 * $sizeOfQueuedNj
        );

        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedNj_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        /// First iteration

        // TODO: Implement blind shouldRead

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dataEmptyStrAddress, $dirAStrAddress)
            ->andReturn(strcmp('\\DIR_A', 'DATA EMPTY'));

        $this->shouldWriteTo('_var_queueBaseDir_8c157a80', $dirAStrAddress);
        $this->shouldCall('_gdFsChangeDir', $dirAStrAddress);

        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe0002, 0)
            ->andReturn(0xf5f50000);

        // task->gdfs_0x0c = gdFsOpen(...)
        $this->shouldWrite($taskPtr + 0x0c, 0xf5f50000);

        $this->shouldCall('_gdFsGetFileSctSize')
            ->with(0xf5f50000, $sizeLocal)
            ->do(function ($params) {
                $this->memory->writeUInt32($params[1], U32::of(0x100));
            })
            ->andReturn(1);

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xbebacafe);
        $this->shouldWriteTo('_var_queueBuffer_8c157a84', $this->addressOf('_var_texbuf_8c277ca0'));

        $this->shouldCall('_gdFsRead')
            ->with(0xf5f50000, 0x100, $this->addressOf('_var_texbuf_8c277ca0'))
            ->andReturn(0); // GDD_ERR_OK

        $this->shouldCall('_gdFsClose')->with(0xf5f50000);
        $this->shouldWrite($njQueue + 0x10, 1);

        $this->shouldCall('_njReadBinary')
            ->with(
                $this->addressOf('_var_texbuf_8c277ca0'),
                $fposLocal,
                $rtypeLocal,
            )
            ->andReturn(0xb1b1b1b1);
        $this->shouldWrite($nj1Dest, 0xb1b1b1b1);

        $this->shouldCall('_njReadBinary')
            ->with(
                $this->addressOf('_var_texbuf_8c277ca0'),
                $fposLocal,
                $rtypeLocal,
            )
            ->andReturn(0xb2b2b2b2);
        $this->shouldWrite($nj1Dest2, 0xb2b2b2b2);

        $this->shouldWrite($taskPtr + 0x18, $njQueue + $sizeOfQueuedNj);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case0_readsFirstItemWithAlloc()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirAStrAddress = $this->allocString('\\DIR_A');
        $dirBStrAddress = $this->allocString('\\DIR_B');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        // Nj queue has 3 items
        $currentQueuedNj = $njQueue;
        $nj1Dest = $this->alloc(4);
        $nj1Dest2 = $this->alloc(4);
        $this->initQueuedNj(
            address:  $currentQueuedNj,
            basedir:  $dirAStrAddress,
            filename: 0xcafe0002,
            dest:     $nj1Dest,
            dest2:    $nj1Dest2,
            flag:     0,
        );

        $currentQueuedNj += $sizeOfQueuedNj;
        $this->initQueuedNj(
            address:  $currentQueuedNj,
            basedir:  $dirBStrAddress,
            filename: 0xcafe1002,
            dest:     0xcafe1003,
            dest2:    0xcafe1004,
            flag:     0,
        );

        $currentQueuedNj += $sizeOfQueuedNj;
        $this->initQueuedNj(
            address:  $currentQueuedNj,
            basedir:  0xcafe2001,
            filename: 0xcafe2002,
            dest:     0xcafe2003,
            dest2:    0xcafe2004,
            flag:     0,
        );

        $this->initUint32(
            $this->addressOf('_var_njQueueRear_8c157aa0'),
            $njQueue + 3 * $sizeOfQueuedNj
        );

        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedNj_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $njQueue);
        
        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xdeadbeef);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        /// First iteration

        // TODO: Implement blind shouldRead

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dataEmptyStrAddress, $dirAStrAddress)
            ->andReturn(strcmp('\\DIR_A', 'DATA EMPTY'));

        $this->shouldWriteTo('_var_queueBaseDir_8c157a80', $dirAStrAddress);
        $this->shouldCall('_gdFsChangeDir', $dirAStrAddress);

        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe0002, 0)
            ->andReturn(0xf5f50000);

        // task->gdfs_0x0c = gdFsOpen(...)
        $this->shouldWrite($taskPtr + 0x0c, 0xf5f50000);

        $this->shouldCall('_gdFsGetFileSctSize')
            ->with(0xf5f50000, $sizeLocal)
            ->do(function ($params) {
                $this->memory->writeUInt32($params[1], U32::of(0x101));
            })
            ->andReturn(1);

        $this->shouldCall('_syMalloc')->with(0x101 * 2048)->andReturn(0xbebacafe);
        $this->shouldWriteTo('_var_queueBuffer_8c157a84', 0xbebacafe);

        $this->shouldCall('_gdFsRead')
            ->with(0xf5f50000, 0x101, 0xbebacafe)
            ->andReturn(0); // GDD_ERR_OK

        $this->shouldCall('_gdFsClose')->with(0xf5f50000);
        $this->shouldWrite($njQueue + 0x10, 1);

        $this->shouldCall('_njReadBinary')
            ->with(
                0xbebacafe,
                $fposLocal,
                $rtypeLocal,
            )
            ->andReturn(0xb1b1b1b1);
        $this->shouldWrite($nj1Dest, 0xb1b1b1b1);

        $this->shouldCall('_njReadBinary')
            ->with(
                0xbebacafe,
                $fposLocal,
                $rtypeLocal,
            )
            ->andReturn(0xb2b2b2b2);
        $this->shouldWrite($nj1Dest2, 0xb2b2b2b2);

        $this->shouldCall('_syFree')->with(0xbebacafe);

        $this->shouldWrite($taskPtr + 0x18, $njQueue + $sizeOfQueuedNj);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case0_breaksOnQueueRear()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $this->initUint32(
            $this->addressOf('_var_njQueue_8c157a9c'),
            $njQueue
        );

        // Nj queue has 3 items
        $currentQueuedNj = $njQueue;
        $this->initQueuedNj(
            address:  $currentQueuedNj,
            basedir:  0xcafe0001,
            filename: 0xcafe0002,
            dest:     0xcafe0003,
            dest2:    0xcafe0004,
            flag:     1,
        );

        $currentQueuedNj += $sizeOfQueuedNj;
        $this->initQueuedNj(
            address:  $currentQueuedNj,
            basedir:  0xcafe1001,
            filename: 0xcafe1002,
            dest:     0xcafe1003,
            dest2:    0xcafe1004,
            flag:     1,
        );

        $currentQueuedNj += $sizeOfQueuedNj;
        $this->initQueuedNj(
            address:  $currentQueuedNj,
            basedir:  0xcafe2001,
            filename: 0xcafe2002,
            dest:     0xcafe2003,
            dest2:    0xcafe2004,
            flag:     1,
        );

        $this->initUint32(
            $this->addressOf('_var_njQueueRear_8c157aa0'),
            $njQueue + 3 * $sizeOfQueuedNj
        );

        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xbebacafe);
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $this->addressOf('_var_texbuf_8c277ca0'));

        $this->initUint32($this->addressOf('_var_8c157a88'), 0);
        
        $this->shouldWriteLongTo('_var_njQueueIsIdle_8c157aa8', 1);
        $this->shouldCall('_TaskFree_8c014b66')->with($taskPtr);
        
        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case0_breaksOnQueueRearWithError()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $this->initUint32(
            $this->addressOf('_var_njQueue_8c157a9c'),
            $njQueue
        );

        // Nj queue has 3 items
        $currentQueuedNj = $njQueue;
        $this->initQueuedNj(
            address:  $currentQueuedNj,
            basedir:  0xcafe0001,
            filename: 0xcafe0002,
            dest:     0xcafe0003,
            dest2:    0xcafe0004,
            flag:     1,
        );

        $currentQueuedNj += $sizeOfQueuedNj;
        $this->initQueuedNj(
            address:  $currentQueuedNj,
            basedir:  0xcafe1001,
            filename: 0xcafe1002,
            dest:     0xcafe1003,
            dest2:    0xcafe1004,
            flag:     1,
        );

        $currentQueuedNj += $sizeOfQueuedNj;
        $this->initQueuedNj(
            address:  $currentQueuedNj,
            basedir:  0xcafe2001,
            filename: 0xcafe2002,
            dest:     0xcafe2003,
            dest2:    0xcafe2004,
            flag:     1,
        );

        $this->initUint32(
            $this->addressOf('_var_njQueueRear_8c157aa0'),
            $njQueue + 3 * $sizeOfQueuedNj
        );

        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xbebacafe);
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $this->addressOf('_var_texbuf_8c277ca0'));

        $this->initUint32($this->addressOf('_var_8c157a88'), 1);

        $this->shouldWrite($taskPtr + 0x18, $njQueue);
        $this->shouldwriteTo('_var_8c157a88', 0);
        $this->shouldWriteStringTo('_var_queueBaseDir_8c157a80', 'DATA EMPTY');

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case0_skipsNjReadBinary()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirAStrAddress = $this->allocString('\\DIR_A');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        $currentQueuedNj = $njQueue;
        $nj1Dest = $this->alloc(4);
        $nj1Dest2 = $this->alloc(4);
        $this->initQueuedNj(
            address:  $currentQueuedNj,
            basedir:  $dirAStrAddress,
            filename: 0xcafe0002,
            dest:     0,
            dest2:    0,
            flag:     0,
        );

        $this->initUint32(
            $this->addressOf('_var_njQueueRear_8c157aa0'),
            $njQueue + 1 * $sizeOfQueuedNj
        );

        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        /// First iteration

        // TODO: Implement blind shouldRead

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dataEmptyStrAddress, $dirAStrAddress)
            ->andReturn(strcmp('\\DIR_A', 'DATA EMPTY'));

        $this->shouldWriteTo('_var_queueBaseDir_8c157a80', $dirAStrAddress);
        $this->shouldCall('_gdFsChangeDir', $dirAStrAddress);

        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe0002, 0)
            ->andReturn(0xf5f50000);

        // task->gdfs_0x0c = gdFsOpen(...)
        $this->shouldWrite($taskPtr + 0x0c, 0xf5f50000);

        $this->shouldCall('_gdFsGetFileSctSize')
            ->with(0xf5f50000, $sizeLocal)
            ->do(function ($params) {
                $this->memory->writeUInt32($params[1], U32::of(0x100));
            })
            ->andReturn(1);

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xbebacafe);
        $this->shouldWriteTo('_var_queueBuffer_8c157a84', $this->addressOf('_var_texbuf_8c277ca0'));

        $this->shouldCall('_gdFsRead')
            ->with(0xf5f50000, 0x100, $this->addressOf('_var_texbuf_8c277ca0'))
            ->andReturn(0); // GDD_ERR_OK

        $this->shouldCall('_gdFsClose')->with(0xf5f50000);
        $this->shouldWrite($njQueue + 0x10, 1);

        $this->shouldWrite($taskPtr + 0x18, $njQueue + $sizeOfQueuedNj);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case0_skipsFirstNjReadBinary()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirAStrAddress = $this->allocString('\\DIR_A');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        $currentQueuedNj = $njQueue;
        $nj1Dest = $this->alloc(4);
        $nj1Dest2 = $this->alloc(4);
        $this->initQueuedNj(
            address:  $currentQueuedNj,
            basedir:  $dirAStrAddress,
            filename: 0xcafe0002,
            dest:     0,
            dest2:    $nj1Dest2,
            flag:     0,
        );

        $this->initUint32(
            $this->addressOf('_var_njQueueRear_8c157aa0'),
            $njQueue + 1 * $sizeOfQueuedNj
        );

        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        // TODO: Implement blind shouldRead

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dataEmptyStrAddress, $dirAStrAddress)
            ->andReturn(strcmp('\\DIR_A', 'DATA EMPTY'));

        $this->shouldWriteTo('_var_queueBaseDir_8c157a80', $dirAStrAddress);
        $this->shouldCall('_gdFsChangeDir', $dirAStrAddress);

        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe0002, 0)
            ->andReturn(0xf5f50000);

        // task->gdfs_0x0c = gdFsOpen(...)
        $this->shouldWrite($taskPtr + 0x0c, 0xf5f50000);

        $this->shouldCall('_gdFsGetFileSctSize')
            ->with(0xf5f50000, $sizeLocal)
            ->do(function ($params) {
                $this->memory->writeUInt32($params[1], U32::of(0x100));
            })
            ->andReturn(1);

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xbebacafe);
        $this->shouldWriteTo('_var_queueBuffer_8c157a84', $this->addressOf('_var_texbuf_8c277ca0'));

        $this->shouldCall('_gdFsRead')
            ->with(0xf5f50000, 0x100, $this->addressOf('_var_texbuf_8c277ca0'))
            ->andReturn(0); // GDD_ERR_OK

        $this->shouldCall('_gdFsClose')->with(0xf5f50000);
        $this->shouldWrite($njQueue + 0x10, 1);

        $this->shouldCall('_njReadBinary')
            ->with(
                $this->addressOf('_var_texbuf_8c277ca0'),
                $fposLocal,
                $rtypeLocal,
            )
            ->andReturn(0xb2b2b2b2);
        $this->shouldWrite($nj1Dest2, 0xb2b2b2b2);

        $this->shouldWrite($taskPtr + 0x18, $njQueue + $sizeOfQueuedNj);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case0_skipsSecondNjReadBinary()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirAStrAddress = $this->allocString('\\DIR_A');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        $currentQueuedNj = $njQueue;
        $nj1Dest = $this->alloc(4);
        $nj1Dest2 = $this->alloc(4);
        $this->initQueuedNj(
            address:  $currentQueuedNj,
            basedir:  $dirAStrAddress,
            filename: 0xcafe0002,
            dest:     $nj1Dest,
            dest2:    0,
            flag:     0,
        );

        $this->initUint32(
            $this->addressOf('_var_njQueueRear_8c157aa0'),
            $njQueue + 1 * $sizeOfQueuedNj
        );

        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        // TODO: Implement blind shouldRead

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dataEmptyStrAddress, $dirAStrAddress)
            ->andReturn(strcmp('\\DIR_A', 'DATA EMPTY'));

        $this->shouldWriteTo('_var_queueBaseDir_8c157a80', $dirAStrAddress);
        $this->shouldCall('_gdFsChangeDir', $dirAStrAddress);

        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe0002, 0)
            ->andReturn(0xf5f50000);

        // task->gdfs_0x0c = gdFsOpen(...)
        $this->shouldWrite($taskPtr + 0x0c, 0xf5f50000);

        $this->shouldCall('_gdFsGetFileSctSize')
            ->with(0xf5f50000, $sizeLocal)
            ->do(function ($params) {
                $this->memory->writeUInt32($params[1], U32::of(0x100));
            })
            ->andReturn(1);

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xbebacafe);
        $this->shouldWriteTo('_var_queueBuffer_8c157a84', $this->addressOf('_var_texbuf_8c277ca0'));

        $this->shouldCall('_gdFsRead')
            ->with(0xf5f50000, 0x100, $this->addressOf('_var_texbuf_8c277ca0'))
            ->andReturn(0); // GDD_ERR_OK

        $this->shouldCall('_gdFsClose')->with(0xf5f50000);
        $this->shouldWrite($njQueue + 0x10, 1);

        $this->shouldCall('_njReadBinary')
            ->with(
                $this->addressOf('_var_texbuf_8c277ca0'),
                $fposLocal,
                $rtypeLocal,
            )
            ->andReturn(0xb1b1b1b1);
        $this->shouldWrite($nj1Dest, 0xb1b1b1b1);

        $this->shouldWrite($taskPtr + 0x18, $njQueue + $sizeOfQueuedNj);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case0_skipItemsWith0x10FlagSet()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirAStrAddress = $this->allocString('\\DIR_A');
        $dirBStrAddress = $this->allocString('\\DIR_B');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        /* Nj queue has 3 items */

        $currentQueuedNj = $njQueue;
        $nj1Dest = $this->alloc(4);
        $nj1Dest2 = $this->alloc(4);
        $this->initQueuedNj(
            address:  $currentQueuedNj,
            basedir:  $dirAStrAddress,
            filename: 0xcafe1002,
            dest:     $nj1Dest,
            dest2:    $nj1Dest2,
            flag:     1,
        );

        $currentQueuedNj += $sizeOfQueuedNj;
        $nj2Dest = $this->alloc(4);
        $nj2Dest2 = $this->alloc(4);
        $this->initQueuedNj(
            address:  $currentQueuedNj,
            basedir:  $dirBStrAddress,
            filename: 0xcafe2002,
            dest:     $nj2Dest,
            dest2:    $nj2Dest2,
            flag:     0,
        );

        $currentQueuedNj += $sizeOfQueuedNj;
        $this->initQueuedNj(
            address:  $currentQueuedNj,
            basedir:  0xcafe3001,
            filename: 0xcafe3002,
            dest:     0xcafe3003,
            dest2:    0xcafe3004,
            flag:     0,
        );

        $this->initUint32(
            $this->addressOf('_var_njQueueRear_8c157aa0'),
            $njQueue + 3 * $sizeOfQueuedNj
        );

        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        // TODO: Implement blind shouldRead

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dataEmptyStrAddress, $dirBStrAddress)
            ->andReturn(1);

        $this->shouldWriteTo('_var_queueBaseDir_8c157a80', $dirBStrAddress);
        $this->shouldCall('_gdFsChangeDir', $dirBStrAddress);

        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe2002, 0)
            ->andReturn(0xf5f50000);

        // task->gdfs_0x0c = gdFsOpen(...)
        $this->shouldWrite($taskPtr + 0x0c, 0xf5f50000);

        $this->shouldCall('_gdFsGetFileSctSize')
            ->with(0xf5f50000, $sizeLocal)
            ->do(function ($params) {
                $this->memory->writeUInt32($params[1], U32::of(0x100));
            })
            ->andReturn(1);

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xbebacafe);
        $this->shouldWriteTo('_var_queueBuffer_8c157a84', $this->addressOf('_var_texbuf_8c277ca0'));

        $this->shouldCall('_gdFsRead')
            ->with(0xf5f50000, 0x100, $this->addressOf('_var_texbuf_8c277ca0'))
            ->andReturn(0); // GDD_ERR_OK

        $this->shouldCall('_gdFsClose')->with(0xf5f50000);
        $this->shouldWrite($njQueue + 1 * $sizeOfQueuedNj + 0x10, 1);

        $this->shouldCall('_njReadBinary')
            ->with(
                $this->addressOf('_var_texbuf_8c277ca0'),
                $fposLocal,
                $rtypeLocal,
            )
            ->andReturn(0xb1b1b1b1);
        $this->shouldWrite($nj2Dest, 0xb1b1b1b1);

        $this->shouldCall('_njReadBinary')
            ->with(
                $this->addressOf('_var_texbuf_8c277ca0'),
                $fposLocal,
                $rtypeLocal,
            )
            ->andReturn(0xb2b2b2b2);
        $this->shouldWrite($nj2Dest2, 0xb2b2b2b2);

        $this->shouldWrite($taskPtr + 0x18, $njQueue + 2 * $sizeOfQueuedNj);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case0_gdFsOpenFailure()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirAStrAddress = $this->allocString('\\DIR_A');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        $this->initQueuedNj(
            address:  $njQueue,
            basedir:  $dirAStrAddress,
            filename: 0xcafe1002,
            dest:     0xcafe1003,
            dest2:    0xcafe1004,
            flag:     0,
        );

        $this->initUint32(
            $this->addressOf('_var_njQueueRear_8c157aa0'),
            $njQueue + 1 * $sizeOfQueuedNj
        );
        
        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $njQueue);
        
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $this->addressOf('_var_texbuf_8c277ca0'));

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        // TODO: Implement blind shouldRead

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dataEmptyStrAddress, $dirAStrAddress)
            ->andReturn(1);

        $this->shouldWriteTo('_var_queueBaseDir_8c157a80', $dirAStrAddress);
        $this->shouldCall('_gdFsChangeDir', $dirAStrAddress);

        // task->gdfs_0x0c = gdFsOpen(...)
        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe1002, 0)
            ->andReturn(0);
        $this->shouldWrite($taskPtr + 0x0c, 0);

        $this->shouldWriteTo('_var_8c157a88', 1);
        $this->shouldWrite($taskPtr + 0x18, $njQueue + 1 * $sizeOfQueuedNj);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case0_gdFsOpenFailureWithFree()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirAStrAddress = $this->allocString('\\DIR_A');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        $this->initQueuedNj(
            address:  $njQueue,
            basedir:  $dirAStrAddress,
            filename: 0xcafe1002,
            dest:     0xcafe1003,
            dest2:    0xcafe1004,
            flag:     0,
        );

        $this->initUint32(
            $this->addressOf('_var_njQueueRear_8c157aa0'),
            $njQueue + 1 * $sizeOfQueuedNj
        );
        
        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $readTarget = $this->alloc(4);

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xbebacafe);
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $readTarget);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        // TODO: Implement blind shouldRead

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dataEmptyStrAddress, $dirAStrAddress)
            ->andReturn(1);

        $this->shouldWriteTo('_var_queueBaseDir_8c157a80', $dirAStrAddress);
        $this->shouldCall('_gdFsChangeDir', $dirAStrAddress);

        // task->gdfs_0x0c = gdFsOpen(...)
        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe1002, 0)
            ->andReturn(0);
        $this->shouldWrite($taskPtr + 0x0c, 0);

        $this->shouldCall('_syFree')->with($readTarget);

        $this->shouldWriteTo('_var_8c157a88', 1);
        $this->shouldWrite($taskPtr + 0x18, $njQueue + 1 * $sizeOfQueuedNj);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case0_gdFsGetFileSctSizeFailure()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirAStrAddress = $this->allocString('\\DIR_A');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        $this->initQueuedNj(
            address:  $njQueue,
            basedir:  $dirAStrAddress,
            filename: 0xcafe1002,
            dest:     0xcafe1003,
            dest2:    0xcafe1004,
            flag:     0,
        );

        $this->initUint32(
            $this->addressOf('_var_njQueueRear_8c157aa0'),
            $njQueue + 1 * $sizeOfQueuedNj
        );
        
        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $njQueue);
        
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $this->addressOf('_var_texbuf_8c277ca0'));

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        // TODO: Implement blind shouldRead

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dataEmptyStrAddress, $dirAStrAddress)
            ->andReturn(1);

        $this->shouldWriteTo('_var_queueBaseDir_8c157a80', $dirAStrAddress);
        $this->shouldCall('_gdFsChangeDir', $dirAStrAddress);

        // task->gdfs_0x0c = gdFsOpen(...)
        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe1002, 0)
            ->andReturn(0xf5f50000);
        $this->shouldWrite($taskPtr + 0x0c, 0xf5f50000);

        $this->shouldCall('_gdFsGetFileSctSize')
            ->with(0xf5f50000, $sizeLocal)
            ->do(function ($params) {
                $this->memory->writeUInt32($params[1], U32::of(0x100));
            })
            ->andReturn(0);

        $this->shouldWriteTo('_var_8c157a88', 1);
        $this->shouldWrite($taskPtr + 0x18, $njQueue + 1 * $sizeOfQueuedNj);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case0_gdFsGetFileSctSizeFailureWithFree()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirAStrAddress = $this->allocString('\\DIR_A');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        $this->initQueuedNj(
            address:  $njQueue,
            basedir:  $dirAStrAddress,
            filename: 0xcafe1002,
            dest:     0xcafe1003,
            dest2:    0xcafe1004,
            flag:     0,
        );

        $this->initUint32(
            $this->addressOf('_var_njQueueRear_8c157aa0'),
            $njQueue + 1 * $sizeOfQueuedNj
        );
        
        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $readTarget = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $readTarget);
        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xbebacafe);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        // TODO: Implement blind shouldRead

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dataEmptyStrAddress, $dirAStrAddress)
            ->andReturn(1);

        $this->shouldWriteTo('_var_queueBaseDir_8c157a80', $dirAStrAddress);
        $this->shouldCall('_gdFsChangeDir', $dirAStrAddress);

        // task->gdfs_0x0c = gdFsOpen(...)
        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe1002, 0)
            ->andReturn(0xf5f50000);
        $this->shouldWrite($taskPtr + 0x0c, 0xf5f50000);

        $this->shouldCall('_gdFsGetFileSctSize')
            ->with(0xf5f50000, $sizeLocal)
            ->do(function ($params) {
                $this->memory->writeUInt32($params[1], U32::of(0x100));
            })
            ->andReturn(0);

        $this->shouldCall('_syFree')->with($readTarget);

        $this->shouldWriteTo('_var_8c157a88', 1);
        $this->shouldWrite($taskPtr + 0x18, $njQueue + 1 * $sizeOfQueuedNj);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case0_gdFsReadFailure()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirAStrAddress = $this->allocString('\\DIR_A');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        $this->initQueuedNj(
            address:  $njQueue,
            basedir:  $dirAStrAddress,
            filename: 0xcafe1002,
            dest:     0xcafe1003,
            dest2:    0xcafe1004,
            flag:     0,
        );

        $this->initUint32(
            $this->addressOf('_var_njQueueRear_8c157aa0'),
            $njQueue + 1 * $sizeOfQueuedNj
        );
        
        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $njQueue);
        
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $this->addressOf('_var_texbuf_8c277ca0'));

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        // TODO: Implement blind shouldRead

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dataEmptyStrAddress, $dirAStrAddress)
            ->andReturn(1);

        $this->shouldWriteTo('_var_queueBaseDir_8c157a80', $dirAStrAddress);
        $this->shouldCall('_gdFsChangeDir', $dirAStrAddress);

        // task->gdfs_0x0c = gdFsOpen(...)
        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe1002, 0)
            ->andReturn(0xf5f50000);
        $this->shouldWrite($taskPtr + 0x0c, 0xf5f50000);

        $this->shouldCall('_gdFsGetFileSctSize')
            ->with(0xf5f50000, $sizeLocal)
            ->do(function ($params) {
                $this->memory->writeUInt32($params[1], U32::of(0x100));
            })
            ->andReturn(1);

        $this->shouldWriteTo('_var_queueBuffer_8c157a84', $this->addressOf('_var_texbuf_8c277ca0'));

        $this->shouldCall('_gdFsRead')
            ->with(0xf5f50000, 0x100, $this->addressOf('_var_texbuf_8c277ca0'))
            ->andReturn(-13); // GDD_ERR_BUSY

        $this->shouldWriteTo('_var_8c157a88', 1);
        $this->shouldWrite($taskPtr + 0x18, $njQueue + 1 * $sizeOfQueuedNj);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case0_gdFsReadFailureWithFree()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirAStrAddress = $this->allocString('\\DIR_A');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        $this->initQueuedNj(
            address:  $njQueue,
            basedir:  $dirAStrAddress,
            filename: 0xcafe1002,
            dest:     0xcafe1003,
            dest2:    0xcafe1004,
            flag:     0,
        );

        $this->initUint32(
            $this->addressOf('_var_njQueueRear_8c157aa0'),
            $njQueue + 1 * $sizeOfQueuedNj
        );
        
        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedDat_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xbebacafe);

        // TODO: Implement blind shouldRead

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dataEmptyStrAddress, $dirAStrAddress)
            ->andReturn(1);

        $this->shouldWriteTo('_var_queueBaseDir_8c157a80', $dirAStrAddress);
        $this->shouldCall('_gdFsChangeDir', $dirAStrAddress);

        // task->gdfs_0x0c = gdFsOpen(...)
        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe1002, 0)
            ->andReturn(0xf5f50000);
        $this->shouldWrite($taskPtr + 0x0c, 0xf5f50000);

        $this->shouldCall('_gdFsGetFileSctSize')
            ->with(0xf5f50000, $sizeLocal)
            ->do(function ($params) {
                $this->memory->writeUInt32($params[1], U32::of(0x101));
            })
            ->andReturn(1);

        $texBuf = $this->alloc(0x4);
        $this->shouldCall('_syMalloc')
            ->with(0x101 * 2048)
            ->andReturn($texBuf);
        $this->shouldWriteTo('_var_queueBuffer_8c157a84', $texBuf);

        $this->shouldCall('_gdFsRead')
            ->with(0xf5f50000, 0x101, $texBuf)
            ->andReturn(-13); // GDD_ERR_BUSY

        $this->shouldCall('_syFree')->with($texBuf);

        $this->shouldWriteTo('_var_8c157a88', 1);
        $this->shouldWrite($taskPtr + 0x18, $njQueue + 1 * $sizeOfQueuedNj);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case1_gdFsStatComplete()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $nj1Dest = $this->alloc(4);
        $nj1Dest2 = $this->alloc(4);
        $this->initQueuedNj(
            address:  $njQueue,
            basedir:  0xcafe1001,
            filename: 0xcafe1002,
            dest:     $nj1Dest,
            dest2:    $nj1Dest2,
            flag:     0,
        );

        $taskPtr = $this->alloc(0x20);
        $this->initUint32($taskPtr + 0x08, 1);
        $this->initUint32($taskPtr + 0x0c, 0xf5f50000);
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xbebacafe);
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $this->addressOf('_var_texbuf_8c277ca0'));

        $this->shouldCall('_gdFsGetStat')
            ->with(0xf5f50000)
            ->andReturn(1); // GDD_STAT_COMPLETE

        $this->shouldCall('_gdFsClose')
            ->with(0xf5f50000);

        $this->shouldWrite($njQueue + 0x10, 1);

        $this->shouldCall('_njReadBinary')
            ->with(
                $this->addressOf('_var_texbuf_8c277ca0'),
                $fposLocal,
                $rtypeLocal,
            )
            ->andReturn(0xb1b1b1b1);
        $this->shouldWrite($nj1Dest, 0xb1b1b1b1);

        $this->shouldCall('_njReadBinary')
            ->with(
                $this->addressOf('_var_texbuf_8c277ca0'),
                $fposLocal,
                $rtypeLocal,
            )
            ->andReturn(0xb2b2b2b2);
        $this->shouldWrite($nj1Dest2, 0xb2b2b2b2);

        $this->shouldWrite($taskPtr + 0x18, $njQueue + $sizeOfQueuedNj);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case1_gddStatCompleteSkipsReadBinary()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $nj1Dest = $this->alloc(4);
        $nj1Dest2 = $this->alloc(4);
        $this->initQueuedNj(
            address:  $njQueue,
            basedir:  0xcafe1001,
            filename: 0xcafe1002,
            dest:     0,
            dest2:    0,
            flag:     0,
        );

        $taskPtr = $this->alloc(0x20);
        $this->initUint32($taskPtr + 0x08, 1);
        $this->initUint32($taskPtr + 0x0c, 0xf5f50000);
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xbebacafe);
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $this->addressOf('_var_texbuf_8c277ca0'));

        $this->shouldCall('_gdFsGetStat')
            ->with(0xf5f50000)
            ->andReturn(1); // GDD_STAT_COMPLETE

        $this->shouldCall('_gdFsClose')
            ->with(0xf5f50000);

        $this->shouldWrite($njQueue + 0x10, 1);

        $this->shouldWrite($taskPtr + 0x18, $njQueue + $sizeOfQueuedNj);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case1_gddStatCompleteSkipsFirstReadBinary()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $nj1Dest = $this->alloc(4);
        $nj1Dest2 = $this->alloc(4);
        $this->initQueuedNj(
            address:  $njQueue,
            basedir:  0xcafe1001,
            filename: 0xcafe1002,
            dest:     0,
            dest2:    $nj1Dest2,
            flag:     0,
        );

        $taskPtr = $this->alloc(0x20);
        $this->initUint32($taskPtr + 0x08, 1);
        $this->initUint32($taskPtr + 0x0c, 0xf5f50000);
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xbebacafe);
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $this->addressOf('_var_texbuf_8c277ca0'));

        $this->shouldCall('_gdFsGetStat')
            ->with(0xf5f50000)
            ->andReturn(1); // GDD_STAT_COMPLETE

        $this->shouldCall('_gdFsClose')
            ->with(0xf5f50000);

        $this->shouldWrite($njQueue + 0x10, 1);

        $this->shouldCall('_njReadBinary')
            ->with(
                $this->addressOf('_var_texbuf_8c277ca0'),
                $fposLocal,
                $rtypeLocal,
            )
            ->andReturn(0xb2b2b2b2);
        $this->shouldWrite($nj1Dest2, 0xb2b2b2b2);

        $this->shouldWrite($taskPtr + 0x18, $njQueue + $sizeOfQueuedNj);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case1_gddStatCompleteSkipsSecondReadBinary()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $nj1Dest = $this->alloc(4);
        $nj1Dest2 = $this->alloc(4);
        $this->initQueuedNj(
            address:  $njQueue,
            basedir:  0xcafe1001,
            filename: 0xcafe1002,
            dest:     $nj1Dest,
            dest2:    0,
            flag:     0,
        );

        $taskPtr = $this->alloc(0x20);
        $this->initUint32($taskPtr + 0x08, 1);
        $this->initUint32($taskPtr + 0x0c, 0xf5f50000);
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xbebacafe);
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $this->addressOf('_var_texbuf_8c277ca0'));

        $this->shouldCall('_gdFsGetStat')
            ->with(0xf5f50000)
            ->andReturn(1); // GDD_STAT_COMPLETE

        $this->shouldCall('_gdFsClose')
            ->with(0xf5f50000);

        $this->shouldWrite($njQueue + 0x10, 1);

        $this->shouldCall('_njReadBinary')
            ->with(
                $this->addressOf('_var_texbuf_8c277ca0'),
                $fposLocal,
                $rtypeLocal,
            )
            ->andReturn(0xb1b1b1b1);
        $this->shouldWrite($nj1Dest, 0xb1b1b1b1);

        $this->shouldWrite($taskPtr + 0x18, $njQueue + $sizeOfQueuedNj);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case1_gddStatCompleteWithFree()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $nj1Dest = $this->alloc(4);
        $nj1Dest2 = $this->alloc(4);
        $this->initQueuedNj(
            address:  $njQueue,
            basedir:  0xcafe1001,
            filename: 0xcafe1002,
            dest:     0,
            dest2:    0,
            flag:     0,
        );

        $taskPtr = $this->alloc(0x20);
        $this->initUint32($taskPtr + 0x08, 1);
        $this->initUint32($taskPtr + 0x0c, 0xf5f50000);
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xbebacafe);
        $readTarget = $this->alloc(0x4);
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $readTarget);

        $this->shouldCall('_gdFsGetStat')
            ->with(0xf5f50000)
            ->andReturn(1); // GDD_STAT_COMPLETE

        $this->shouldCall('_gdFsClose')
            ->with(0xf5f50000);

        $this->shouldWrite($njQueue + 0x10, 1);

        $this->shouldCall('_syFree')->with($readTarget);

        $this->shouldWrite($taskPtr + 0x18, $njQueue + $sizeOfQueuedNj);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case1_gddStatRead()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $nj1Dest = $this->alloc(4);
        $nj1Dest2 = $this->alloc(4);
        $this->initQueuedNj(
            address:  $njQueue,
            basedir:  0xcafe1001,
            filename: 0xcafe1002,
            dest:     $nj1Dest,
            dest2:    $nj1Dest2,
            flag:     0,
        );

        $taskPtr = $this->alloc(0x20);
        $this->initUint32($taskPtr + 0x08, 1);
        $this->initUint32($taskPtr + 0x0c, 0xf5f50000);
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xbebacafe);
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $this->addressOf('_var_texbuf_8c277ca0'));

        $this->shouldCall('_gdFsGetStat')
            ->with(0xf5f50000)
            ->andReturn(2); // GDD_STAT_READ

        $this->shouldCall('_gdFsGetTransStat')
            ->with(0xf5f50000)
            ->andReturn(0); // GDD_FS_TRANS_READY

        $this->shouldCall('_gdFsTrans32')
            ->with(0xf5f50000, 2048, $this->addressOf('_var_texbuf_8c277ca0'));

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case1_gddStatRead_NotReady()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $nj1Dest = $this->alloc(4);
        $nj1Dest2 = $this->alloc(4);
        $this->initQueuedNj(
            address:  $njQueue,
            basedir:  0xcafe1001,
            filename: 0xcafe1002,
            dest:     $nj1Dest,
            dest2:    $nj1Dest2,
            flag:     0,
        );

        $taskPtr = $this->alloc(0x20);
        $this->initUint32($taskPtr + 0x08, 1);
        $this->initUint32($taskPtr + 0x0c, 0xf5f50000);
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xbebacafe);
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $this->addressOf('_var_texbuf_8c277ca0'));

        $this->shouldCall('_gdFsGetStat')
            ->with(0xf5f50000)
            ->andReturn(2); // GDD_STAT_READ

        $this->shouldCall('_gdFsGetTransStat')
            ->with(0xf5f50000)
            ->andReturn(1); // GDD_FS_TRANS_BUSY

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case1_gddStatReadWithFree()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $nj1Dest = $this->alloc(4);
        $nj1Dest2 = $this->alloc(4);
        $this->initQueuedNj(
            address:  $njQueue,
            basedir:  0xcafe1001,
            filename: 0xcafe1002,
            dest:     $nj1Dest,
            dest2:    $nj1Dest2,
            flag:     0,
        );

        $taskPtr = $this->alloc(0x20);
        $this->initUint32($taskPtr + 0x08, 1);
        $this->initUint32($taskPtr + 0x0c, 0xf5f50000);
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xbebacafe);
        $readTarget = $this->alloc(0x4);
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $readTarget);

        $this->shouldCall('_gdFsGetStat')
            ->with(0xf5f50000)
            ->andReturn(2); // GDD_STAT_READ

        $this->shouldCall('_gdFsGetTransStat')
            ->with(0xf5f50000)
            ->andReturn(0); // GDD_FS_TRANS_READY

        $this->shouldCall('_gdFsTrans32')
            ->with(0xf5f50000, 2048, $readTarget);

        // $this->shouldCall('_syFree')->with($readTarget);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case1_gddStatOther()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $nj1Dest = $this->alloc(4);
        $nj1Dest2 = $this->alloc(4);
        $this->initQueuedNj(
            address:  $njQueue,
            basedir:  0xcafe1001,
            filename: 0xcafe1002,
            dest:     $nj1Dest,
            dest2:    $nj1Dest2,
            flag:     0,
        );

        $taskPtr = $this->alloc(0x20);
        $this->initUint32($taskPtr + 0x08, 1);
        $this->initUint32($taskPtr + 0x0c, 0xf5f50000);
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xbebacafe);
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $this->addressOf('_var_texbuf_8c277ca0'));

        $this->shouldCall('_gdFsGetStat')
            ->with(0xf5f50000)
            ->andReturn(3); // GDD_STAT_SEEK

        $this->shouldCall('_gdFsClose')
            ->with(0xf5f50000);

        $this->shouldwriteTo('_var_8c157a88', 1);
        $this->shouldWrite($taskPtr + 0x18, $njQueue + 1 * $sizeOfQueuedNj);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case1_gddStatOtherWithFree()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $nj1Dest = $this->alloc(4);
        $nj1Dest2 = $this->alloc(4);
        $this->initQueuedNj(
            address:  $njQueue,
            basedir:  0xcafe1001,
            filename: 0xcafe1002,
            dest:     $nj1Dest,
            dest2:    $nj1Dest2,
            flag:     0,
        );

        $taskPtr = $this->alloc(0x20);
        $this->initUint32($taskPtr + 0x08, 1);
        $this->initUint32($taskPtr + 0x0c, 0xf5f50000);
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xdeadbeef);
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), 0xbebacafe);

        $this->shouldCall('_gdFsGetStat')
            ->with(0xf5f50000)
            ->andReturn(3); // GDD_STAT_SEEK

        $this->shouldCall('_gdFsClose')
            ->with(0xf5f50000);

        $this->shouldCall('_syFree')->with(0xbebacafe);
        $this->shouldCall('_syFree')->with(0xbebacafe);
        
        $this->shouldwriteTo('_var_8c157a88', 1);
        $this->shouldWrite($taskPtr + 0x18, $njQueue + 1 * $sizeOfQueuedNj);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case2()
    {
        $this->resolveImports();

        $sizeOfQueuedNj = 0x14;
        $queueSize = 16;
        $njQueue = $this->alloc($queueSize * $sizeOfQueuedNj);

        $nj1Dest = $this->alloc(4);
        $nj1Dest2 = $this->alloc(4);
        $this->initQueuedNj(
            address:  $njQueue,
            basedir:  0xcafe1001,
            filename: 0xcafe1002,
            dest:     $nj1Dest,
            dest2:    $nj1Dest2,
            flag:     0,
        );

        $taskPtr = $this->alloc(0x20);
        $this->initUint32($taskPtr + 0x08, 2);
        $this->initUint32($taskPtr + 0x0c, 0xf5f50000);
        $this->initUint32($taskPtr + 0x18, $njQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd0 : 0xffffcc;
        $fposLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd0;
        $rtypeLocal = $this->isAsmObject() ? 0xffffd8 : 0xffffd4;

        $this->initUint32($this->addressOf('_var_texbuf_8c277ca0'), 0xdeadbeef);
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), 0xbebacafe);

        $this->singleCall('_taskLoadQueuedNjs_8c0114cc')
            ->with($taskPtr, 0)
            ->run();
    }

    private function resolveImports(): void
    {
        // Functions
        $this->setSize('_syFree', 4);
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
