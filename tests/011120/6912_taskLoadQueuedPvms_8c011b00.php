<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_case0_readsFirstItem()
    {
        $this->resolveImports();

        $sizeOfQueuedPvm = 0x18;
        $queueSize = 16;
        $pvmQueue = $this->alloc($queueSize * $sizeOfQueuedPvm);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirStrAddress = $this->allocString('\\DIR');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        // Pvm queue has 3 items
        $currentQueuedPvm = $pvmQueue;
        $pvm1Dest = $this->alloc(4);
        $this->initUint32($currentQueuedPvm + 0x00, $dirStrAddress); // char* basedir;
        $this->initUint32($currentQueuedPvm + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedPvm + 0x08, $pvm1Dest); // void** field_0x08;
        $this->initUint32($currentQueuedPvm + 0x0c, 2); // int count_0x0c;
        $this->initUint32($currentQueuedPvm + 0x10, 0xcafe0005); // int attr_0x10;
        $this->initUint32($currentQueuedPvm + 0x14, 0); // int field_0x14;

        $currentQueuedPvm += $sizeOfQueuedPvm;
        $this->initUint32($currentQueuedPvm + 0x00, 0xcafe1001); // char* basedir;
        $this->initUint32($currentQueuedPvm + 0x04, 0xcafe1002); // char* filename;
        $this->initUint32($currentQueuedPvm + 0x08, 0xcafe1003); // void** field_0x08;
        $this->initUint32($currentQueuedPvm + 0x0c, 0xcafe1004); // int count_0x0c;
        $this->initUint32($currentQueuedPvm + 0x10, 0xcafe1005); // int attr_0x10;
        $this->initUint32($currentQueuedPvm + 0x14, 0); // int field_0x14;

        $currentQueuedPvm += $sizeOfQueuedPvm;
        $this->initUint32($currentQueuedPvm + 0x00, 0xcafe2001); // char* basedir;
        $this->initUint32($currentQueuedPvm + 0x04, 0xcafe2002); // char* filename;
        $this->initUint32($currentQueuedPvm + 0x08, 0xcafe2003); // void** field_0x08;
        $this->initUint32($currentQueuedPvm + 0x0c, 0xcafe2004); // int count_0x0c;
        $this->initUint32($currentQueuedPvm + 0x10, 0xcafe2005); // int attr_0x10;
        $this->initUint32($currentQueuedPvm + 0x14, 0); // int field_0x14;

        $this->initUint32(
            $this->addressOf('_var_pvmQueueRear_8c157ac0'),
            $pvmQueue + 3 * $sizeOfQueuedPvm
        );

        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedPvm_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $pvmQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd4;

        /// First iteration

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dataEmptyStrAddress, $dirStrAddress)
            ->andReturn(1);

        $this->shouldWriteTo('_var_queueBaseDir_8c157a80', $dirStrAddress);
        $this->shouldCall('_gdFsChangeDir', $dirStrAddress);

        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe0002, 0)
            ->andReturn(0xf5f50000);

        // task->gdfs_0x0c = gdFsOpen(...)
        $this->shouldWrite($taskPtr + 0x0c, 0xf5f50000);

        $this->shouldCall('_gdFsGetFileSctSize')
            ->with(0xf5f50000, $sizeLocal)
            ->do(function (...$params) use ($sizeLocal) {
                $this->writeUInt32($sizeLocal, 0, U32::of(0x100));
            })
            ->andReturn(1);

        // TODO: Test malloc path
        // $this->shouldCall('_syMalloc')
        //     ->with(0x100 * 2048)
        //     ->andReturn(0xbebacafe);
        // $this->shouldWrite($pvm1Dest, 0xbebacafe);

        $this->shouldWriteTo('_var_queueBuffer_8c157a84', $this->addressOf('_var_texbuf_8c277ca0'));

        $this->shouldCall('_gdFsRead')
            ->with(0xf5f50000, 0x100, $this->addressOf('_var_texbuf_8c277ca0'))
            ->andReturn(0); // GDD_ERR_OK

        $this->shouldCall('_gdFsClose')
            ->with(0xf5f50000);

        $this->shouldWrite($pvmQueue + 0 * $sizeOfQueuedPvm + 0x14, 1);

        $this->shouldCall('_syMalloc')->with(8)->andReturn(0xa110c001);
        $this->shouldWrite($pvm1Dest, 0xa110c001);

        $texname = $this->alloc(2 * 0x0c);
        $this->shouldCall('_syMalloc')->with(2 * 0x0c)->andReturn($texname);

        $this->shouldWrite($texname + 0x00 + 0x04, 0xcafe0005);
        $this->shouldWrite($texname + 0x0c + 0x04, 0xcafe0005);

        $this->shouldCall('_syMalloc')->with(2 * 0x1c)->andReturn(0xa110c002);

        $this->shouldCall('_njSetPvmTextureList')
            ->with(0xa110c001, $texname, 0xa110c002, 2);

        $this->shouldCall('_njLoadTexturePvmMemory')
            ->with($this->addressOf('_var_texbuf_8c277ca0'), 0xa110c001);   

        $this->shouldWrite($taskPtr + 0x18, $pvmQueue + $sizeOfQueuedPvm);
        $this->shouldWrite($taskPtr + 0x08, 0);
        // $this->shouldWrite($pvmQueue + 0x0c, 1);

        $this->singleCall('_taskLoadQueuedPvms_8c011b00')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case0_skipsItemsWith0x14FlagSet()
    {
        $this->resolveImports();

        $sizeOfQueuedPvm = 0x18;
        $queueSize = 16;
        $pvmQueue = $this->alloc($queueSize * $sizeOfQueuedPvm);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirStrAddress = $this->allocString('\\DIR');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        // Pvm queue has 3 items
        $currentQueuedPvm = $pvmQueue;
        $this->initUint32($currentQueuedPvm + 0x00, 0xcafe0001); // char* basedir;
        $this->initUint32($currentQueuedPvm + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedPvm + 0x08, 0xcafe0003); // void** field_0x08;
        $this->initUint32($currentQueuedPvm + 0x0c, 0xcafe0004); // int count_0x0c;
        $this->initUint32($currentQueuedPvm + 0x10, 0xcafe0005); // int attr_0x10;
        $this->initUint32($currentQueuedPvm + 0x14, 1); // int field_0x14;
        
        $pvm1Dest = $this->alloc(4);
        $currentQueuedPvm += $sizeOfQueuedPvm;
        $this->initUint32($currentQueuedPvm + 0x00, $dirStrAddress); // char* basedir;
        $this->initUint32($currentQueuedPvm + 0x04, 0xcafe1002); // char* filename;
        $this->initUint32($currentQueuedPvm + 0x08, $pvm1Dest); // void** field_0x08;
        $this->initUint32($currentQueuedPvm + 0x0c, 2); // int count_0x0c;
        $this->initUint32($currentQueuedPvm + 0x10, 0xcafe1005); // int attr_0x10;
        $this->initUint32($currentQueuedPvm + 0x14, 0); // int field_0x14;

        $currentQueuedPvm += $sizeOfQueuedPvm;
        $this->initUint32($currentQueuedPvm + 0x00, 0xcafe2001); // char* basedir;
        $this->initUint32($currentQueuedPvm + 0x04, 0xcafe2002); // char* filename;
        $this->initUint32($currentQueuedPvm + 0x08, 0xcafe2003); // void** field_0x08;
        $this->initUint32($currentQueuedPvm + 0x0c, 0xcafe2004); // int count_0x0c;
        $this->initUint32($currentQueuedPvm + 0x10, 0xcafe2005); // int attr_0x10;
        $this->initUint32($currentQueuedPvm + 0x14, 0); // int field_0x14;

        $this->initUint32(
            $this->addressOf('_var_pvmQueueRear_8c157ac0'),
            $pvmQueue + 3 * $sizeOfQueuedPvm
        );

        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedPvm_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $pvmQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd4;

        /// First iteration skipped

        // Second iteration

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';
        $this->shouldCall($strCmp)
            ->with($dataEmptyStrAddress, $dirStrAddress)
            ->andReturn(1);

        $this->shouldWriteTo('_var_queueBaseDir_8c157a80', $dirStrAddress);
        $this->shouldCall('_gdFsChangeDir', $dirStrAddress);

        $this->shouldCall('_gdFsOpen')
            ->with(0xcafe1002, 0)
            ->andReturn(0xf5f50000);

        // task->gdfs_0x0c = gdFsOpen(...)
        $this->shouldWrite($taskPtr + 0x0c, 0xf5f50000);

        $this->shouldCall('_gdFsGetFileSctSize')
            ->with(0xf5f50000, $sizeLocal)
            ->do(function (...$params) use ($sizeLocal) {
                $this->writeUInt32($sizeLocal, 0, U32::of(0x100));
            })
            ->andReturn(1);

        $this->shouldWriteTo('_var_queueBuffer_8c157a84', $this->addressOf('_var_texbuf_8c277ca0'));

        $this->shouldCall('_gdFsRead')
            ->with(0xf5f50000, 0x100, $this->addressOf('_var_texbuf_8c277ca0'))
            ->andReturn(0); // GDD_ERR_OK

        $this->shouldCall('_gdFsClose')
            ->with(0xf5f50000);

        $this->shouldWrite($pvmQueue + 1 * $sizeOfQueuedPvm + 0x14, 1);

        $this->shouldCall('_syMalloc')->with(8)->andReturn(0xa110c001);
        $this->shouldWrite($pvm1Dest, 0xa110c001);

        $texname = $this->alloc(2 * 0x0c);
        $this->shouldCall('_syMalloc')->with(2 * 0x0c)->andReturn($texname);

        $this->shouldWrite($texname + 0x00 + 0x04, 0xcafe1005);
        $this->shouldWrite($texname + 0x0c + 0x04, 0xcafe1005);

        $this->shouldCall('_syMalloc')->with(2 * 0x1c)->andReturn(0xa110c002);

        $this->shouldCall('_njSetPvmTextureList')
            ->with(0xa110c001, $texname, 0xa110c002, 2);

        $this->shouldCall('_njLoadTexturePvmMemory')
            ->with($this->addressOf('_var_texbuf_8c277ca0'), 0xa110c001);   

        $this->shouldWrite($taskPtr + 0x18, $pvmQueue + 2 * $sizeOfQueuedPvm);
        $this->shouldWrite($taskPtr + 0x08, 0);
        // $this->shouldWrite($pvmQueue + 0x0c, 1);

        $this->singleCall('_taskLoadQueuedPvms_8c011b00')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case0_breaksOnQueueRear()
    {
        $this->resolveImports();

        $sizeOfQueuedPvm = 0x18;
        $queueSize = 16;
        $pvmQueue = $this->alloc($queueSize * $sizeOfQueuedPvm);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirStrAddress = $this->allocString('\\DIR');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        // Pvm queue has 3 items
        $currentQueuedPvm = $pvmQueue;
        $this->initUint32($currentQueuedPvm + 0x00, 0xcafe0001); // char* basedir;
        $this->initUint32($currentQueuedPvm + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedPvm + 0x08, 0xcafe0003); // void** field_0x08;
        $this->initUint32($currentQueuedPvm + 0x0c, 0xcafe0004); // int count_0x0c;
        $this->initUint32($currentQueuedPvm + 0x10, 0xcafe0005); // int attr_0x10;
        $this->initUint32($currentQueuedPvm + 0x14, 1); // int field_0x14;
        
        $pvm1Dest = $this->alloc(4);
        $currentQueuedPvm += $sizeOfQueuedPvm;
        $this->initUint32($currentQueuedPvm + 0x00, 0xcafe1001); // char* basedir;
        $this->initUint32($currentQueuedPvm + 0x04, 0xcafe1002); // char* filename;
        $this->initUint32($currentQueuedPvm + 0x08, 0xcafe1003); // void** field_0x08;
        $this->initUint32($currentQueuedPvm + 0x0c, 0xcafe1004); // int count_0x0c;
        $this->initUint32($currentQueuedPvm + 0x10, 0xcafe1005); // int attr_0x10;
        $this->initUint32($currentQueuedPvm + 0x14, 1); // int field_0x14;

        $currentQueuedPvm += $sizeOfQueuedPvm;
        $this->initUint32($currentQueuedPvm + 0x00, 0xcafe2001); // char* basedir;
        $this->initUint32($currentQueuedPvm + 0x04, 0xcafe2002); // char* filename;
        $this->initUint32($currentQueuedPvm + 0x08, 0xcafe2003); // void** field_0x08;
        $this->initUint32($currentQueuedPvm + 0x0c, 0xcafe2004); // int count_0x0c;
        $this->initUint32($currentQueuedPvm + 0x10, 0xcafe2005); // int attr_0x10;
        $this->initUint32($currentQueuedPvm + 0x14, 1); // int field_0x14;

        $this->initUint32(
            $this->addressOf('_var_pvmQueueRear_8c157ac0'),
            $pvmQueue + 3 * $sizeOfQueuedPvm
        );

        $this->initUint32($this->addressOf('_var_pvmQueue_8c157abc'), 0xbaadf00d);

        $this->initUint32($this->addressOf('_var_8c157a88'), 1);

        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedPvm_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $pvmQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd4;

        $this->shouldWrite($taskPtr + 0x18, 0xbaadf00d);
        $this->shouldWriteTo('_var_8c157a88', 0);
        $this->shouldWriteStringTo('_var_queueBaseDir_8c157a80', 'DATA EMPTY');

        $this->singleCall('_taskLoadQueuedPvms_8c011b00')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case0_breaksOnQueueRearAndFreeTask()
    {
        $this->resolveImports();

        $sizeOfQueuedPvm = 0x18;
        $queueSize = 16;
        $pvmQueue = $this->alloc($queueSize * $sizeOfQueuedPvm);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirStrAddress = $this->allocString('\\DIR');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        // Pvm queue has 3 items
        $currentQueuedPvm = $pvmQueue;
        $this->initUint32($currentQueuedPvm + 0x00, 0xcafe0001); // char* basedir;
        $this->initUint32($currentQueuedPvm + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedPvm + 0x08, 0xcafe0003); // void** field_0x08;
        $this->initUint32($currentQueuedPvm + 0x0c, 0xcafe0004); // int count_0x0c;
        $this->initUint32($currentQueuedPvm + 0x10, 0xcafe0005); // int attr_0x10;
        $this->initUint32($currentQueuedPvm + 0x14, 1); // int field_0x14;
        
        $pvm1Dest = $this->alloc(4);
        $currentQueuedPvm += $sizeOfQueuedPvm;
        $this->initUint32($currentQueuedPvm + 0x00, 0xcafe1001); // char* basedir;
        $this->initUint32($currentQueuedPvm + 0x04, 0xcafe1002); // char* filename;
        $this->initUint32($currentQueuedPvm + 0x08, 0xcafe1003); // void** field_0x08;
        $this->initUint32($currentQueuedPvm + 0x0c, 0xcafe1004); // int count_0x0c;
        $this->initUint32($currentQueuedPvm + 0x10, 0xcafe1005); // int attr_0x10;
        $this->initUint32($currentQueuedPvm + 0x14, 1); // int field_0x14;

        $currentQueuedPvm += $sizeOfQueuedPvm;
        $this->initUint32($currentQueuedPvm + 0x00, 0xcafe2001); // char* basedir;
        $this->initUint32($currentQueuedPvm + 0x04, 0xcafe2002); // char* filename;
        $this->initUint32($currentQueuedPvm + 0x08, 0xcafe2003); // void** field_0x08;
        $this->initUint32($currentQueuedPvm + 0x0c, 0xcafe2004); // int count_0x0c;
        $this->initUint32($currentQueuedPvm + 0x10, 0xcafe2005); // int attr_0x10;
        $this->initUint32($currentQueuedPvm + 0x14, 1); // int field_0x14;

        $this->initUint32(
            $this->addressOf('_var_pvmQueueRear_8c157ac0'),
            $pvmQueue + 3 * $sizeOfQueuedPvm
        );

        $this->initUint32($this->addressOf('_var_pvmQueue_8c157abc'), 0xbaadf00d);

        $this->initUint32($this->addressOf('_var_8c157a88'), 0);

        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 0);
        // task->queuedPvm_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $pvmQueue);

        $sizeLocal = $this->isAsmObject() ? 0xffffd4 : 0xffffd4;

        $this->shouldWriteTo('_var_pvmQueueIsIdle_8c157ac8', 1);
        $this->shouldCall('_TaskFree_8c014b66')->with($taskPtr);

        $this->singleCall('_taskLoadQueuedPvms_8c011b00')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case1_gdfsStatComplete()
    {
        $this->resolveImports();

        $sizeOfQueuedPvm = 0x18;
        $queueSize = 16;
        $pvmQueue = $this->alloc($queueSize * $sizeOfQueuedPvm);
        $this->initUint32($this->addressOf('_var_pvmQueue_8c157a8c'), $pvmQueue);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirStrAddress = $this->allocString('\\DIR');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $this->addressOf('_var_texbuf_8c277ca0'));

        // Pvm queue has 1 item
        $currentQueuedPvm = $pvmQueue;
        $this->initUint32($currentQueuedPvm + 0x00, 0xcafe0001); // char* basedir;
        $this->initUint32($currentQueuedPvm + 0x04, 0xcafe0002); // char* filename;
        $texlistPtr = $this->alloc(4);
        $this->initUint32($currentQueuedPvm + 0x08, $texlistPtr); // void** field_0x08;
        $this->initUint32($currentQueuedPvm + 0x0c, 3); // int count_0x0c;
        $this->initUint32($currentQueuedPvm + 0x10, 0xcafe0005); // int attr_0x10;
        $this->initUint32($currentQueuedPvm + 0x14, 1); // int field_0x14;

        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 1);
        // task->gdfs_0x0c
        $this->initUint32($taskPtr + 0x0c, 0xbebacafe);
        // task->queuedPvm_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $pvmQueue);

        $this->shouldCall('_gdFsGetStat')
            ->with(0xbebacafe)
            ->andReturn(1); // GDD_STAT_COMPLETE
        $this->shouldCall('_gdFsClose')->with(0xbebacafe);
        $this->shouldWrite($pvmQueue + 0x14, 1);

        $this->shouldCall('_syMalloc')->with(8)->andReturn(0xa110c001);
        $this->shouldWrite($texlistPtr, 0xa110c001);

        $texname = $this->alloc(3 * 0x0c);
        $this->shouldCall('_syMalloc')->with(3 * 0x0c)->andReturn($texname);
        $this->shouldWrite($texname + 0 * 0x0c + 4, 0xcafe0005);
        $this->shouldWrite($texname + 1 * 0x0c + 4, 0xcafe0005);
        $this->shouldWrite($texname + 2 * 0x0c + 4, 0xcafe0005);

        $this->shouldCall('_syMalloc')->with(3 * 0x1c)->andReturn(0xa110c002);

        $this->shouldCall('_njSetPvmTextureList')
            ->with(0xa110c001, $texname, 0xa110c002, 3);

        $this->shouldCall('_njLoadTexturePvmMemory')
            ->with($this->addressOf('_var_texbuf_8c277ca0'), 0xa110c001);

        $this->shouldWrite($taskPtr + 0x18, $pvmQueue + 1 * $sizeOfQueuedPvm);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $texname = $this->alloc(2 * 0x0c);

        //$this->shouldWrite($taskPtr + 0x18, $pvmQueue + $sizeOfQueuedPvm);
        //$this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedPvms_8c011b00')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case1_gdfsStatRead_gdfsFsTransReady()
    {
        $this->resolveImports();

        $sizeOfQueuedPvm = 0x18;
        $queueSize = 16;
        $pvmQueue = $this->alloc($queueSize * $sizeOfQueuedPvm);
        $this->initUint32($this->addressOf('_var_pvmQueue_8c157a8c'), $pvmQueue);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirStrAddress = $this->allocString('\\DIR');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $this->addressOf('_var_texbuf_8c277ca0'));

        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 1);
        // task->gdfs_0x0c
        $this->initUint32($taskPtr + 0x0c, 0xbebacafe);
        // task->queuedPvm_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $pvmQueue);

        $this->shouldCall('_gdFsGetStat')
            ->with(0xbebacafe)
            ->andReturn(2); // GDD_STAT_READ
        $this->shouldCall('_gdFsGetTransStat')
            ->with(0xbebacafe)
            ->andReturn(0); // GDD_FS_TRANS_READY
        $this->shouldCall('_gdFsTrans32')
            ->with(0xbebacafe, 2048, $this->addressOf('_var_texbuf_8c277ca0'));

        $this->singleCall('_taskLoadQueuedPvms_8c011b00')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case1_gdfsStatRead_gdfsFsTransBusy()
    {
        $this->resolveImports();

        $sizeOfQueuedPvm = 0x18;
        $queueSize = 16;
        $pvmQueue = $this->alloc($queueSize * $sizeOfQueuedPvm);
        $this->initUint32($this->addressOf('_var_pvmQueue_8c157a8c'), $pvmQueue);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirStrAddress = $this->allocString('\\DIR');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $this->addressOf('_var_texbuf_8c277ca0'));

        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 1);
        // task->gdfs_0x0c
        $this->initUint32($taskPtr + 0x0c, 0xbebacafe);
        // task->queuedPvm_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $pvmQueue);

        $this->shouldCall('_gdFsGetStat')
            ->with(0xbebacafe)
            ->andReturn(2); // GDD_STAT_READ
        $this->shouldCall('_gdFsGetTransStat')
            ->with(0xbebacafe)
            ->andReturn(1); // GDD_FS_TRANS_BUSY

        $this->singleCall('_taskLoadQueuedPvms_8c011b00')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_case1_gdfsStatBusy()
    {
        $this->resolveImports();

        $sizeOfQueuedPvm = 0x18;
        $queueSize = 16;
        $pvmQueue = $this->alloc($queueSize * $sizeOfQueuedPvm);
        $this->initUint32($this->addressOf('_var_pvmQueue_8c157a8c'), $pvmQueue);

        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $dirStrAddress = $this->allocString('\\DIR');

        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);
        $this->initUint32($this->addressOf('_var_queueBuffer_8c157a84'), $this->addressOf('_var_texbuf_8c277ca0'));

        $currentQueuedPvm = $pvmQueue;
        $pvm1Dest = $this->alloc(4);
        $this->initUint32($pvm1Dest, 0xcafe1000);
        $this->initUint32($currentQueuedPvm + 0x00, $dirStrAddress); // char* basedir;
        $this->initUint32($currentQueuedPvm + 0x04, 0xcafe0002); // char* filename;
        $this->initUint32($currentQueuedPvm + 0x08, $pvm1Dest); // void** dest;
        $this->initUint32($currentQueuedPvm + 0x0c, 0); // int count_0x0c;

        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 1);
        // task->gdfs_0x0c
        $this->initUint32($taskPtr + 0x0c, 0xbebacafe);
        // task->queuedPvm_0x18 points to the first item in the queue
        $this->initUint32($taskPtr + 0x18, $pvmQueue);

        $this->shouldCall('_gdFsGetStat')
            ->with(0xbebacafe)
            ->andReturn(4); // GDD_STAT_BUSY
        $this->shouldCall('_gdFsClose')
            ->with(0xbebacafe);
    
        $this->shouldWriteTo('_var_8c157a88', 1);
        $this->shouldWrite($taskPtr + 0x18, $pvmQueue + $sizeOfQueuedPvm);
        $this->shouldWrite($taskPtr + 0x08, 0);

        $this->singleCall('_taskLoadQueuedPvms_8c011b00')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_nocase()
    {
        $this->resolveImports();

        $sizeOfQueuedPvm = 0x18;
        $queueSize = 16;
        $pvmQueue = $this->alloc($queueSize * $sizeOfQueuedPvm);
        $this->initUint32($this->addressOf('_var_pvmQueue_8c157a8c'), $pvmQueue);

        $taskPtr = $this->alloc(0x20);
        // task->field_0x08
        $this->initUint32($taskPtr + 0x08, 2);

        $this->singleCall('_taskLoadQueuedPvms_8c011b00')
            ->with($taskPtr, 0)
            ->run();
    }

    private function resolveImports()
    {
        $this->setSize('_var_texbuf_8c277ca0', 0x80800);

        // Functions
        $this->setSize('_gdFsClose', 4);
        $this->setSize('_TaskFree_8c014b66', 4);
        $this->setSize('_syMalloc', 4);
        $this->setSize('_gsFsGetFileSctSize', 4);
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
