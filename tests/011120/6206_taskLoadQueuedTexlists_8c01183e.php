<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_loadSingleItem()
    {
        $itemSize = 8;
        $queueLength = 8;
        $queue = $this->alloc($queueLength * $itemSize);
        $queueTail = $queue + $queueLength * $itemSize;

        $this->initUint32($this->addressOf('_var_texlistQueueCount_8c157a68'), 0);
        $this->initUint32($this->addressOf('_var_texlistQueue_8c157aac'), $queue);
        $this->initUint32($this->addressOf('_var_texlistQueueRear_8c157ab0'), $queue + 1 * $itemSize);
        $baseDirStrAddress = $this->allocString('\\DIR');
        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $baseDirStrAddress);

        $task = $this->alloc(0x1c);
        $this->initUint32($task + 0x18, $queue);

        // ------------
        // NJS_TEXNAMES
        // ------------
        $texnameSize = 0xc;
        $textures = $this->alloc($texnameSize * 3);
        $textures;

        $texnameA = $textures;
        $texAFile = $this->allocString('TEX_A');
        $this->initTexname($texnameA, $texAFile, 0, 0);
        
        $texnameB = $texnameA + $texnameSize;
        $texBFile = $this->allocString('TEX_B');
        $this->initTexname($texnameB, $texBFile, 0, 0);

        $texnameC = $texnameB + $texnameSize;
        $texCFile = $this->allocString('TEX_C');
        $this->initTexname($texnameC, $texCFile, 0, 0);

        // -----------
        // NJS_TEXLIST
        // -----------
        $texlist = $this->alloc(0x8);
        $this->initUint32($texlist + 0x0, $textures);
        $this->initUint32($texlist + 0x4, 3);  // nbTexture

        // -------------
        // QueuedTexlist
        // -------------
        $dirStrAddress = $this->allocString('\\DIR');
        $this->initUint32($queue + 0 * $itemSize + 0, $dirStrAddress);
        $this->initUint32($queue + 0 * $itemSize + 4, $texlist);

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';

        $this->shouldCall($strCmp)
            ->with($baseDirStrAddress, $dirStrAddress)
            ->andReturn(strcmp('\\DIR', '\\DIR'));
        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njLoadTextureNum')->with(0);
        $this->shouldCall('_njLoadTextureNum')->with(1);
        $this->shouldCall('_njLoadTextureNum')->with(2);

        $this->shouldWriteTo('_var_texlistQueueCount_8c157a68', 1);
        $this->shouldWriteTo('_var_texlistQueueIsIdle_8c157ab8', 1);

        $this->shouldCall('_TaskFree_8c014b66')
            ->with($task);

        $this->singleCall('_taskLoadQueuedTexlists_8c01183e')
            ->with($task, 0)
            ->run();
    }

    public function test_loadSingleItemAndAdvancesToNext()
    {
        $itemSize = 8;
        $queueLength = 8;
        $queue = $this->alloc($queueLength * $itemSize);
        $queueTail = $queue + $queueLength * $itemSize;

        $this->initUint32($this->addressOf('_var_texlistQueueCount_8c157a68'), 0);
        $this->initUint32($this->addressOf('_var_texlistQueue_8c157aac'), $queue);
        $this->initUint32($this->addressOf('_var_texlistQueueRear_8c157ab0'), $queue + 2 * $itemSize);
        $baseDirStrAddress = $this->allocString('\\DIR');
        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $baseDirStrAddress);

        $task = $this->alloc(0x1c);
        $this->initUint32($task + 0x18, $queue);

        // ------------
        // NJS_TEXNAMES
        // ------------
        $texnameSize = 0xc;
        $textures = $this->alloc($texnameSize * 3);
        $textures;

        $texnameA = $textures;
        $texAFile = $this->allocString('TEX_A');
        $this->initTexname($texnameA, $texAFile, 0, 0);
        
        $texnameB = $texnameA + $texnameSize;
        $texBFile = $this->allocString('TEX_B');
        $this->initTexname($texnameB, $texBFile, 0, 0);

        $texnameC = $texnameB + $texnameSize;
        $texCFile = $this->allocString('TEX_C');
        $this->initTexname($texnameC, $texCFile, 0, 0);

        // -----------
        // NJS_TEXLIST
        // -----------
        $texlist = $this->alloc(0x8);
        $this->initUint32($texlist + 0x0, $textures);
        $this->initUint32($texlist + 0x4, 3);  // nbTexture

        // -------------
        // QueuedTexlist
        // -------------
        $dirStrAddress = $this->allocString('\\DIR');
        $this->initUint32($queue + 0 * $itemSize + 0, $dirStrAddress);
        $this->initUint32($queue + 0 * $itemSize + 4, $texlist);

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';

        $this->shouldCall($strCmp)
            ->with($baseDirStrAddress, $dirStrAddress)
            ->andReturn(strcmp('\\DIR', '\\DIR'));
        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njLoadTextureNum')->with(0);
        $this->shouldCall('_njLoadTextureNum')->with(1);
        $this->shouldCall('_njLoadTextureNum')->with(2);

        $this->shouldWriteTo('_var_texlistQueueCount_8c157a68', 1);

        $this->shouldWrite($task + 0x18, $queue + 1 * $itemSize);

        // $this->shouldWriteTo('_var_texlistQueueIsIdle_8c157ab8', 1);

        // $this->shouldCall('_TaskFree_8c014b66')
        //     ->with($task);

        $this->singleCall('_taskLoadQueuedTexlists_8c01183e')
            ->with($task, 0)
            ->run();
    }

    public function test_changesBaseDir()
    {
        $itemSize = 8;
        $queueLength = 8;
        $queue = $this->alloc($queueLength * $itemSize);
        $queueTail = $queue + $queueLength * $itemSize;

        $this->initUint32($this->addressOf('_var_texlistQueueCount_8c157a68'), 0);
        $this->initUint32($this->addressOf('_var_texlistQueue_8c157aac'), $queue);
        $this->initUint32($this->addressOf('_var_texlistQueueRear_8c157ab0'), $queue + 1 * $itemSize);
        $dataEmptyStrAddress = $this->allocString('DATA EMPTY');
        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dataEmptyStrAddress);

        $task = $this->alloc(0x1c);
        $this->initUint32($task + 0x18, $queue);

        // ------------
        // NJS_TEXNAMES
        // ------------
        $texnameSize = 0xc;
        $textures = $this->alloc($texnameSize * 3);
        $textures;

        $texnameA = $textures;
        $texAFile = $this->allocString('TEX_A');
        $this->initTexname($texnameA, $texAFile, 0, 0);
        
        $texnameB = $texnameA + $texnameSize;
        $texBFile = $this->allocString('TEX_B');
        $this->initTexname($texnameB, $texBFile, 0, 0);

        $texnameC = $texnameB + $texnameSize;
        $texCFile = $this->allocString('TEX_C');
        $this->initTexname($texnameC, $texCFile, 0, 0);

        // -----------
        // NJS_TEXLIST
        // -----------
        $texlist = $this->alloc(0x8);
        $this->initUint32($texlist + 0x0, $textures);
        $this->initUint32($texlist + 0x4, 3);  // nbTexture

        // -------------
        // QueuedTexlist
        // -------------
        $dirStrAddress = $this->allocString('\\DIR');
        $this->initUint32($queue + 0 * $itemSize + 0, $dirStrAddress);
        $this->initUint32($queue + 0 * $itemSize + 4, $texlist);

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';

        // Not found
        $this->shouldCall($strCmp)
            ->with($dataEmptyStrAddress, $dirStrAddress)
            ->andReturn(strcmp('\\DIR', '\\DATA EMPTY'));
        $this->shouldWriteTo('_var_queueBaseDir_8c157a80', $dirStrAddress);
        $this->shouldCall('_gdFsChangeDir')->with($dirStrAddress);
        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njLoadTextureNum')->with(0);
        $this->shouldCall('_njLoadTextureNum')->with(1);
        $this->shouldCall('_njLoadTextureNum')->with(2);

        $this->shouldWriteTo('_var_texlistQueueCount_8c157a68', 1);
        $this->shouldWriteTo('_var_texlistQueueIsIdle_8c157ab8', 1);

        $this->shouldCall('_TaskFree_8c014b66')
            ->with($task);

        $this->singleCall('_taskLoadQueuedTexlists_8c01183e')
            ->with($task, 0)
            ->run();
    }

    public function test_ignoresAlreadyLoadedTexture()
    {
        $itemSize = 8;
        $queueLength = 8;
        $queue = $this->alloc($queueLength * $itemSize);
        $queueTail = $queue + $queueLength * $itemSize;

        $dirStrAddress = $this->allocString('\\DIR');
        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dirStrAddress);
        $this->initUint32($this->addressOf('_var_texlistQueueCount_8c157a68'), 1);
        $this->initUint32($this->addressOf('_var_texlistQueue_8c157aac'), $queue);
        $this->initUint32($this->addressOf('_var_texlistQueueRear_8c157ab0'), $queue + 2 * $itemSize);

        $task = $this->alloc(0x1c);
        $this->initUint32($task + 0x18, $queue + 1 * $itemSize);

        // ----------------------------
        // QueuedTexlist A NJS_TEXNAMES
        // ----------------------------
        $texnameSize = 0xc;
        $texturesA = $this->alloc($texnameSize * 3);
        $texturesA;

        $texnameAA = $texturesA;
        $texAAFile = $this->allocString('TEX_A_A');
        $this->initTexname($texnameAA, $texAAFile, 0, 0xcafe0002);

        $texnameAB = $texnameAA + $texnameSize;
        $texABFile = $this->allocString('TEX_A_B');
        $this->initTexname($texnameAB, $texABFile, 0, 0xcafe1002);

        $texnameAC = $texnameAB + $texnameSize;
        $texACFile = $this->allocString('TEX_A_C');
        $this->initTexname($texnameAC, $texACFile, 0, 0xcafe2002);

        // ---------------------------
        // QueuedTexlist A NJS_TEXLIST
        // ---------------------------
        $texlistA = $this->alloc(0x8);
        $this->initUint32($texlistA + 0x0, $texturesA);
        $this->initUint32($texlistA + 0x4, 3);  // nbTexture

        // ---------------
        // QueuedTexlist A
        // ---------------
        $this->initUint32($queue + 0 * $itemSize + 0, $dirStrAddress);
        $this->initUint32($queue + 0 * $itemSize + 4, $texlistA);

        // ----------------------------
        // QueuedTexlist B NJS_TEXNAMES
        // ----------------------------
        $texnameSize = 0xc;
        $texturesB = $this->alloc($texnameSize * 3);
        $texturesB;

        $texnameBA = $texturesB;
        $texBAFile = $this->allocString('TEX_B_A');
        $this->initTexname($texnameBA, $texBAFile, 0, 0);

        $texnameBB = $texnameBA + $texnameSize;
        $texBBFile = $this->allocString('TEX_A_B');
        $this->initTexname($texnameBB, $texBBFile, 0, 0);

        $texnameBC = $texnameBB + $texnameSize;
        $texBCFile = $this->allocString('TEX_B_C');
        $this->initTexname($texnameBC, $texBCFile, 0, 0);

        // ---------------------------
        // QueuedTexlist B NJS_TEXLIST
        // ---------------------------
        $texlistB = $this->alloc(0x8);
        $this->initUint32($texlistB + 0x0, $texturesB);
        $this->initUint32($texlistB + 0x4, 3);  // nbTexture

        // ---------------
        // QueuedTexlist B
        // ---------------
        $this->initUint32($queue + 1 * $itemSize + 0, $dirStrAddress);
        $this->initUint32($queue + 1 * $itemSize + 4, $texlistB);

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';

        // 1st iteration
        $this->shouldCall($strCmp)
            ->with($texBAFile, $texAAFile)
            ->andReturn(strcmp('TEX_B_A', 'TEX_A_A'));

        // 2nd iteration
        $this->shouldCall($strCmp)
            ->with($texBAFile, $texABFile)
            ->andReturn(strcmp('TEX_B_A', 'TEX_A_B'));

        // 3rd iteration
        $this->shouldCall($strCmp)
            ->with($texBAFile, $texACFile)
            ->andReturn(strcmp('TEX_B_A', 'TEX_A_C'));

        // Next texture
        // 1st iteration
        $this->shouldCall($strCmp)
            ->with($texBBFile, $texAAFile)
            ->andReturn(strcmp('TEX_A_B', 'TEX_A_A'));

        // 2nd iteration
        $this->shouldCall($strCmp)
            ->with($texBBFile, $texABFile)
            ->andReturn(strcmp('TEX_A_B', 'TEX_A_B'));

        // Break
        $this->shouldWrite($texnameBB + 0x08, 0xcafe1002);

        // Next texture
        // 1st iteration
        $this->shouldCall($strCmp)
            ->with($texBCFile, $texAAFile)
            ->andReturn(strcmp('TEX_B_C', 'TEX_A_A'));

        // 2nd iteration
        $this->shouldCall($strCmp)
            ->with($texBCFile, $texABFile)
            ->andReturn(strcmp('TEX_B_C', 'TEX_A_B'));

        // 3rd iteration
        $this->shouldCall($strCmp)
            ->with($texBCFile, $texACFile)
            ->andReturn(strcmp('TEX_B_C', 'TEX_A_C'));

        $this->shouldCall($strCmp)
            ->with($dirStrAddress, $dirStrAddress)
            ->andReturn(strcmp('\\DIR', '\\DIR'));
        $this->shouldCall('_njSetTexture')->with($texlistB);
        $this->shouldCall('_njLoadTextureNum')->with(0);
        // Skip texture #1, as it is loaded already
        $this->shouldCall('_njLoadTextureNum')->with(2);

        // Increment texlist count (why?)
        $this->shouldWriteTo('_var_texlistQueueCount_8c157a68', 2);

        $this->shouldWriteTo('_var_texlistQueueIsIdle_8c157ab8', 1);

        $this->shouldCall('_TaskFree_8c014b66')
            ->with($task);

        $this->singleCall('_taskLoadQueuedTexlists_8c01183e')
            ->with($task, 0)
            ->run();
    }

    public function test_ignoresAlreadyLoadedTexlist()
    {
        $itemSize = 8;
        $queueLength = 8;
        $queue = $this->alloc($queueLength * $itemSize);
        $queueTail = $queue + $queueLength * $itemSize;

        $dirStrAddress = $this->allocString('\\DIR');
        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dirStrAddress);
        $this->initUint32($this->addressOf('_var_texlistQueueCount_8c157a68'), 1);
        $this->initUint32($this->addressOf('_var_texlistQueue_8c157aac'), $queue);
        $this->initUint32($this->addressOf('_var_texlistQueueRear_8c157ab0'), $queue + 2 * $itemSize);

        $task = $this->alloc(0x1c);
        $this->initUint32($task + 0x18, $queue + 1 * $itemSize);

        // ----------------------------
        // QueuedTexlist A NJS_TEXNAMES
        // ----------------------------
        $texnameSize = 0xc;
        $texturesA = $this->alloc($texnameSize * 3);
        $texturesA;

        $texnameAA = $texturesA;
        $texAAFile = $this->allocString('TEX_A_A');
        $this->initTexname($texnameAA, $texAAFile, 0, 0xcafe0001);

        $texnameAB = $texnameAA + $texnameSize;
        $texABFile = $this->allocString('TEX_A_B');
        $this->initTexname($texnameAB, $texABFile, 0, 0xcafe0002);

        $texnameAC = $texnameAB + $texnameSize;
        $texACFile = $this->allocString('TEX_A_C');
        $this->initTexname($texnameAC, $texACFile, 0, 0xcafe0003);

        // ---------------------------
        // QueuedTexlist A NJS_TEXLIST
        // ---------------------------
        $texlistA = $this->alloc(0x8);
        $this->initUint32($texlistA + 0x0, $texturesA);
        $this->initUint32($texlistA + 0x4, 3);  // nbTexture

        // ---------------
        // QueuedTexlist A
        // ---------------
        $this->initUint32($queue + 0 * $itemSize + 0, $dirStrAddress);
        $this->initUint32($queue + 0 * $itemSize + 4, $texlistA);

        // ----------------------------
        // QueuedTexlist B NJS_TEXNAMES
        // ----------------------------
        $texnameSize = 0xc;
        $texturesB = $this->alloc($texnameSize * 3);
        $texturesB;

        $texnameBA = $texturesB;
        $texBAFile = $this->allocString('TEX_A_A');
        $this->initTexname($texnameBA, $texBAFile, 0, 0);

        $texnameBB = $texnameBA + $texnameSize;
        $texBBFile = $this->allocString('TEX_A_B');
        $this->initTexname($texnameBB, $texBBFile, 0, 0);

        $texnameBC = $texnameBB + $texnameSize;
        $texBCFile = $this->allocString('TEX_A_C');
        $this->initTexname($texnameBC, $texBCFile, 0, 0);

        // ---------------------------
        // QueuedTexlist B NJS_TEXLIST
        // ---------------------------
        $texlistB = $this->alloc(0x8);
        $this->initUint32($texlistB + 0x0, $texturesB);
        $this->initUint32($texlistB + 0x4, 3);  // nbTexture

        // ---------------
        // QueuedTexlist B
        // ---------------
        $this->initUint32($queue + 1 * $itemSize + 0, $dirStrAddress);
        $this->initUint32($queue + 1 * $itemSize + 4, $texlistB);

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';

        // 1st iteration
        $this->shouldCall($strCmp)
            ->with($texBAFile, $texAAFile)
            ->andReturn(strcmp('TEX_A_A', 'TEX_A_A'));

        $this->shouldWrite($texnameBA + 0x08, 0xcafe0001);

        // Next texture
        // 1st iteration
        $this->shouldCall($strCmp)
            ->with($texBBFile, $texAAFile)
            ->andReturn(strcmp('TEX_A_B', 'TEX_A_A'));

        // 2nd iteration
        $this->shouldCall($strCmp)
            ->with($texBBFile, $texABFile)
            ->andReturn(strcmp('TEX_A_B', 'TEX_A_B'));

        // Break
        $this->shouldWrite($texnameBB + 0x08, 0xcafe0002);

        // Next texture
        // 1st iteration
        $this->shouldCall($strCmp)
            ->with($texBCFile, $texAAFile)
            ->andReturn(strcmp('TEX_A_C', 'TEX_A_A'));

        // 2nd iteration
        $this->shouldCall($strCmp)
            ->with($texBCFile, $texABFile)
            ->andReturn(strcmp('TEX_A_C', 'TEX_A_B'));

        // 3rd iteration
        $this->shouldCall($strCmp)
            ->with($texBCFile, $texACFile)
            ->andReturn(strcmp('TEX_A_C', 'TEX_A_C'));

        // Break
        $this->shouldWrite($texnameBC + 0x08, 0xcafe0003);

        $this->shouldWrite($queue + 1 * $itemSize + 4, 0);

        $this->shouldWriteTo('_var_texlistQueueCount_8c157a68', 2);
        $this->shouldWriteTo('_var_texlistQueueIsIdle_8c157ab8', 1);
        $this->shouldCall('_TaskFree_8c014b66')
            ->with($task);

        $this->singleCall('_taskLoadQueuedTexlists_8c01183e')
            ->with($task, 0)
            ->run();
    }

    public function test_ignoresAlreadyLoadedTexlistAndAdvancesToNext()
    {
        $itemSize = 8;
        $queueLength = 8;
        $queue = $this->alloc($queueLength * $itemSize);
        $queueTail = $queue + $queueLength * $itemSize;

        $dirStrAddress = $this->allocString('\\DIR');
        $this->initUint32($this->addressOf('_var_queueBaseDir_8c157a80'), $dirStrAddress);
        $this->initUint32($this->addressOf('_var_texlistQueueCount_8c157a68'), 1);
        $this->initUint32($this->addressOf('_var_texlistQueue_8c157aac'), $queue);
        $this->initUint32($this->addressOf('_var_texlistQueueRear_8c157ab0'), $queue + 3 * $itemSize);

        $task = $this->alloc(0x1c);
        $this->initUint32($task + 0x18, $queue + 1 * $itemSize);

        // ----------------------------
        // QueuedTexlist A NJS_TEXNAMES
        // ----------------------------
        $texnameSize = 0xc;
        $texturesA = $this->alloc($texnameSize * 3);
        $texturesA;

        $texnameAA = $texturesA;
        $texAAFile = $this->allocString('TEX_A_A');
        $this->initTexname($texnameAA, $texAAFile, 0, 0xcafe0001);

        $texnameAB = $texnameAA + $texnameSize;
        $texABFile = $this->allocString('TEX_A_B');
        $this->initTexname($texnameAB, $texABFile, 0, 0xcafe0002);

        $texnameAC = $texnameAB + $texnameSize;
        $texACFile = $this->allocString('TEX_A_C');
        $this->initTexname($texnameAC, $texACFile, 0, 0xcafe0003);

        // ---------------------------
        // QueuedTexlist A NJS_TEXLIST
        // ---------------------------
        $texlistA = $this->alloc(0x8);
        $this->initUint32($texlistA + 0x0, $texturesA);
        $this->initUint32($texlistA + 0x4, 3);  // nbTexture

        // ---------------
        // QueuedTexlist A
        // ---------------
        $this->initUint32($queue + 0 * $itemSize + 0, $dirStrAddress);
        $this->initUint32($queue + 0 * $itemSize + 4, $texlistA);

        // ----------------------------
        // QueuedTexlist B NJS_TEXNAMES
        // ----------------------------
        $texnameSize = 0xc;
        $texturesB = $this->alloc($texnameSize * 3);
        $texturesB;

        $texnameBA = $texturesB;
        $texBAFile = $this->allocString('TEX_A_A');
        $this->initTexname($texnameBA, $texBAFile, 0, 0);

        $texnameBB = $texnameBA + $texnameSize;
        $texBBFile = $this->allocString('TEX_A_B');
        $this->initTexname($texnameBB, $texBBFile, 0, 0);

        $texnameBC = $texnameBB + $texnameSize;
        $texBCFile = $this->allocString('TEX_A_C');
        $this->initTexname($texnameBC, $texBCFile, 0, 0);

        // ---------------------------
        // QueuedTexlist B NJS_TEXLIST
        // ---------------------------
        $texlistC = $this->alloc(0x8);
        $this->initUint32($texlistC + 0x0, $texturesB);
        $this->initUint32($texlistC + 0x4, 3);  // nbTexture

        // ---------------
        // QueuedTexlist B
        // ---------------
        $this->initUint32($queue + 1 * $itemSize + 0, $dirStrAddress);
        $this->initUint32($queue + 1 * $itemSize + 4, $texlistC);

        // ----------------------------
        // QueuedTexlist C NJS_TEXNAMES
        // ----------------------------
        $texnameSize = 0xc;
        $texturesC = $this->alloc($texnameSize * 3);
        $texturesC;

        $texnameCA = $texturesC;
        $texCAFile = $this->allocString('TEX_C_A');
        $this->initTexname($texnameCA, $texCAFile, 0, 0);

        $texnameCB = $texnameCA + $texnameSize;
        $texCBFile = $this->allocString('TEX_C_B');
        $this->initTexname($texnameCB, $texCBFile, 0, 0);

        $texnameCC = $texnameCB + $texnameSize;
        $texCCFile = $this->allocString('TEX_C_C');
        $this->initTexname($texnameCC, $texCCFile, 0, 0);

        // ---------------------------
        // QueuedTexlist C NJS_TEXLIST
        // ---------------------------
        $texlistC = $this->alloc(0x8);
        $this->initUint32($texlistC + 0x0, $texturesC);
        $this->initUint32($texlistC + 0x4, 3);  // nbTexture

        // ---------------
        // QueuedTexlist C
        // ---------------
        $this->initUint32($queue + 2 * $itemSize + 0, $dirStrAddress);
        $this->initUint32($queue + 2 * $itemSize + 4, $texlistC);

        $strCmp = $this->isAsmObject() ? '_strcmp' : '__slow_strcmp1';

        // 1st iteration
        $this->shouldCall($strCmp)
            ->with($texBAFile, $texAAFile)
            ->andReturn(strcmp('TEX_A_A', 'TEX_A_A'));

        $this->shouldWrite($texnameBA + 0x08, 0xcafe0001);

        // Next texture
        // 1st iteration
        $this->shouldCall($strCmp)
            ->with($texBBFile, $texAAFile)
            ->andReturn(strcmp('TEX_A_B', 'TEX_A_A'));

        // 2nd iteration
        $this->shouldCall($strCmp)
            ->with($texBBFile, $texABFile)
            ->andReturn(strcmp('TEX_A_B', 'TEX_A_B'));

        // Break
        $this->shouldWrite($texnameBB + 0x08, 0xcafe0002);

        // Next texture
        // 1st iteration
        $this->shouldCall($strCmp)
            ->with($texBCFile, $texAAFile)
            ->andReturn(strcmp('TEX_A_C', 'TEX_A_A'));

        // 2nd iteration
        $this->shouldCall($strCmp)
            ->with($texBCFile, $texABFile)
            ->andReturn(strcmp('TEX_A_C', 'TEX_A_B'));

        // 3rd iteration
        $this->shouldCall($strCmp)
            ->with($texBCFile, $texACFile)
            ->andReturn(strcmp('TEX_A_C', 'TEX_A_C'));

        // Break
        $this->shouldWrite($texnameBC + 0x08, 0xcafe0003);

        $this->shouldWrite($queue + 1 * $itemSize + 4, 0);

        $this->shouldWriteTo('_var_texlistQueueCount_8c157a68', 2);


        // Advancing to next item

        // 1st iteration
        $this->shouldCall($strCmp)
            ->with($texCAFile, $texAAFile)
            ->andReturn(strcmp('TEX_C_A', 'TEX_A_A'));

        // 2nd iteration
        $this->shouldCall($strCmp)
            ->with($texCAFile, $texABFile)
            ->andReturn(strcmp('TEX_C_A', 'TEX_A_B'));

        // 3rd iteration
        $this->shouldCall($strCmp)
            ->with($texCAFile, $texACFile)
            ->andReturn(strcmp('TEX_C_A', 'TEX_A_C'));

        // Next texture
        // 1st iteration
        $this->shouldCall($strCmp)
            ->with($texCBFile, $texAAFile)
            ->andReturn(strcmp('TEX_C_B', 'TEX_A_A'));

        // 2nd iteration
        $this->shouldCall($strCmp)
            ->with($texCBFile, $texABFile)
            ->andReturn(strcmp('TEX_C_B', 'TEX_A_B'));

        // 3rd iteration
        $this->shouldCall($strCmp)
            ->with($texCBFile, $texACFile)
            ->andReturn(strcmp('TEX_C_B', 'TEX_A_C'));

        // Next texture
        // 1st iteration
        $this->shouldCall($strCmp)
            ->with($texCCFile, $texAAFile)
            ->andReturn(strcmp('TEX_C_C', 'TEX_A_A'));

        // 2nd iteration
        $this->shouldCall($strCmp)
            ->with($texCCFile, $texABFile)
            ->andReturn(strcmp('TEX_C_C', 'TEX_A_B'));

        // 3rd iteration
        $this->shouldCall($strCmp)
            ->with($texCCFile, $texACFile)
            ->andReturn(strcmp('TEX_C_C', 'TEX_A_C'));


        $this->shouldCall($strCmp)
            ->with($dirStrAddress, $dirStrAddress)
            ->andReturn(strcmp('\\DIR', '\\DIR'));

        $this->shouldCall('_njSetTexture')->with($texlistC);
        $this->shouldCall('_njLoadTextureNum')->with(0);
        $this->shouldCall('_njLoadTextureNum')->with(1);
        $this->shouldCall('_njLoadTextureNum')->with(2);

        $this->shouldWriteTo('_var_texlistQueueCount_8c157a68', 3);
        $this->shouldWriteTo('_var_texlistQueueIsIdle_8c157ab8', 1);

        $this->shouldCall('_TaskFree_8c014b66')
            ->with($task);

        $this->singleCall('_taskLoadQueuedTexlists_8c01183e')
            ->with($task, 0)
            ->run();
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }

    public function initTexname($address, int $name, int $attr, int $addr)
    {
        $this->initUint32($address, $name);
        $this->initUint32($address + 4, $attr);
        $this->initUint32($address + 8, $addr);
    }
};
