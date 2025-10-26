<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    public function testFUN_8c01306e_DemoIsNot2()
    {
        $var_8c18ad28Ptr = $this->alloc(0x14);
        $this->initUint8($var_8c18ad28Ptr + 0x08, 0x10);
        $this->initUint8($var_8c18ad28Ptr + 0x09, 0x20);
        $this->initUint8($var_8c18ad28Ptr + 0x0a, 0x30);
        $this->initUint8($var_8c18ad28Ptr + 0x0b, 0x40);
        $this->initUint32($var_8c18ad28Ptr + 0x0c, fdec(42.0));
        $this->initUint32($var_8c18ad28Ptr + 0x10, fdec(43.0));
        $this->initUint32($this->addressOf('_var_8c18ad28'), $var_8c18ad28Ptr);

        $this->initUint32($this->addressOf('_var_seed_8c157a64'), 0xcafe0001);
        $this->initUint32($this->addressOf('_var_demo_8c1bb8d0'), 1);

        $createdTask = $this->alloc(0x0c);
        $this->initUint32(0xffffe4, $createdTask);

        $this->shouldCall('_njInitMatrix')->with($this->addressOf('_var_matrix_8c2f8ca0'), 16, 0);
        $this->shouldCall('_njSetBackColor')->with(0, 0, 0);
        $this->shouldCall('_njSetFogColor')->with(0x40302010);
        $this->shouldCall('_njGenerateFogTable3')->with($this->addressOf('_var_fogTable_8c18aaf8'), 42.0, 43.0);
        $this->shouldCall('_njFogEnable');
        $this->shouldCall('_kmSetCheapShadowMode')->with(0x80);
        $this->shouldCall('_kmSetFogTable')->with($this->addressOf('_var_fogTable_8c18aaf8'));

        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1ba5e8'), 0x10);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1ba808'), 0x20);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1bac28'), 0x40);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1bb448'), 0x20);

        // njRandomSeed
        $this->shouldCall('_srand')->with(0xcafe0001);
        $this->shouldCall('_AsqSetSeedA_12160')->with(0xcafe0001);
        $this->shouldCall('_AsqSetSeedB_121a2')->with(0xcafe0001);
        
        $this->shouldCall('_FUN_8c0128cc')->with(1);

        $this->shouldCall('_pushTask_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_task_8c012cbc'),
                0xffffe4, 0xFFFFE8, 0
            );
        $this->shouldCall('_pushTask_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba5e8'),
                $this->addressOf('_task_8c01677e'),
                0xffffe4, 0xFFFFE8, 0
            );

        $this->shouldWriteTo('_var_8c1bb8cc', 0);
        $this->shouldWriteTo('_var_8c22847c', 0);

        $this->shouldCall('_FUN_8c023610');
        $this->shouldCall('_FUN_8c02845a');
        
        $this->shouldCall('_FUN_8c029920');
        
        $this->shouldCall('_FUN_8c0296d6');
        $this->shouldCall('_FUN_8c02769e');
        $this->shouldCall('_FUN_8c0222dc');
        $this->shouldCall('_FUN_8c02a6ac');
        $this->shouldCall('_FUN_8c02c46a');
        $this->shouldCall('_FUN_8c02018c');
        $this->shouldCall('_FUN_8c02d968');
        $this->shouldCall('_FUN_8c020528');

        $this->shouldCall('_pushTask_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba5e8'),
            new WildcardArgument,
            0xffffe4,
            0xFFFFE8,
            0
        );

        $this->shouldRead(0xffffe4, $createdTask);
        $this->shouldWrite($createdTask + 0x08, 0);

        $this->shouldCall('_FUN_8c0228a2');

        $this->singleCall('_FUN_8c01306e')->run();
    }

    public function testFUN_8c01306e_DemoIs2_8c1bb8d4Is0()
    {
        $var_8c18ad28Ptr = $this->alloc(0x14);
        $this->initUint8($var_8c18ad28Ptr + 0x08, 0x10);
        $this->initUint8($var_8c18ad28Ptr + 0x09, 0x20);
        $this->initUint8($var_8c18ad28Ptr + 0x0a, 0x30);
        $this->initUint8($var_8c18ad28Ptr + 0x0b, 0x40);
        $this->initUint32($var_8c18ad28Ptr + 0x0c, fdec(42.0));
        $this->initUint32($var_8c18ad28Ptr + 0x10, fdec(43.0));
        $this->initUint32($this->addressOf('_var_8c18ad28'), $var_8c18ad28Ptr);

        $this->initUint32($this->addressOf('_var_seed_8c157a64'), 0xcafe0001);
        $this->initUint32($this->addressOf('_var_demo_8c1bb8d0'), 2);

        $var_8c1bb8d4Ptr = $this->allocRellocate('_var_8c1bb8d4', 4);
        $this->initUint32($var_8c1bb8d4Ptr, 0);

        $createdTask = $this->alloc(0x0c);
        $this->initUint32(0xffffe4, $createdTask);

        $this->shouldCall('_njInitMatrix')->with($this->addressOf('_var_matrix_8c2f8ca0'), 16, 0);
        $this->shouldCall('_njSetBackColor')->with(0, 0, 0);
        $this->shouldCall('_njSetFogColor')->with(0x40302010);
        $this->shouldCall('_njGenerateFogTable3')->with($this->addressOf('_var_fogTable_8c18aaf8'), 42.0, 43.0);
        $this->shouldCall('_njFogEnable');
        $this->shouldCall('_kmSetCheapShadowMode')->with(0x80);
        $this->shouldCall('_kmSetFogTable')->with($this->addressOf('_var_fogTable_8c18aaf8'));

        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1ba5e8'), 0x10);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1ba808'), 0x20);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1bac28'), 0x40);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1bb448'), 0x20);

        // njRandomSeed
        $this->shouldCall('_srand')->with(0xcafe0001);
        $this->shouldCall('_AsqSetSeedA_12160')->with(0xcafe0001);
        $this->shouldCall('_AsqSetSeedB_121a2')->with(0xcafe0001);
        
        $this->shouldCall('_FUN_8c0128cc')->with(1);

        $this->shouldCall('_pushTask_8c014ae8')->with($this->addressOf('_var_tasks_8c1ba3c8'), $this->addressOf('_task_8c012d06'), 0xffffe4, 0xFFFFE8, 0);
        $this->shouldCall('_pushTask_8c014ae8')->with($this->addressOf('_var_tasks_8c1ba5e8'), $this->addressOf('_task_8c016bf4'), 0xffffe4, 0xFFFFE8, 0);
        $this->shouldCall('_FUN_8c025af4');

        $this->shouldWriteTo('_var_8c1bb8cc', 0);
        $this->shouldWriteTo('_var_8c22847c', 0);

        $this->shouldCall('_FUN_8c023610');
        $this->shouldCall('_FUN_8c02845a');
        
        $this->shouldCall('_FUN_8c0296d6');
        $this->shouldCall('_FUN_8c02769e');
        $this->shouldCall('_FUN_8c0222dc');
        $this->shouldCall('_FUN_8c02a6ac');
        $this->shouldCall('_FUN_8c02c46a');
        $this->shouldCall('_FUN_8c02018c');
        $this->shouldCall('_FUN_8c02d968');
        $this->shouldCall('_FUN_8c020528');

        $this->shouldCall('_pushTask_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba5e8'),
            new WildcardArgument,
            0xffffe4,
            0xFFFFE8,
            0
        );
        $this->shouldRead(0xffffe4, $createdTask);
        $this->shouldWrite($createdTask + 0x08, 0);

        $this->shouldCall('_FUN_8c0228a2');

        $this->singleCall('_FUN_8c01306e')->run();
    }

    public function testFUN_8c01306e_DemoIs2_8c1bb8d4Is1()
    {
        $var_8c18ad28Ptr = $this->alloc(0x14);
        $this->initUint8($var_8c18ad28Ptr + 0x08, 0x10);
        $this->initUint8($var_8c18ad28Ptr + 0x09, 0x20);
        $this->initUint8($var_8c18ad28Ptr + 0x0a, 0x30);
        $this->initUint8($var_8c18ad28Ptr + 0x0b, 0x40);
        $this->initUint32($var_8c18ad28Ptr + 0x0c, fdec(42.0));
        $this->initUint32($var_8c18ad28Ptr + 0x10, fdec(43.0));
        $this->initUint32($this->addressOf('_var_8c18ad28'), $var_8c18ad28Ptr);

        $this->initUint32($this->addressOf('_var_seed_8c157a64'), 0xcafe0001);
        $this->initUint32($this->addressOf('_var_demo_8c1bb8d0'), 2);
        $this->initUint32($this->addressOf('_var_8c1bb8d4'), 1);

        $createdTask = $this->alloc(0x0c);
        $createdTask2 = $this->alloc(0x0c);

        $this->shouldCall('_njInitMatrix')->with($this->addressOf('_var_matrix_8c2f8ca0'), 16, 0);
        $this->shouldCall('_njSetBackColor')->with(0, 0, 0);
        $this->shouldCall('_njSetFogColor')->with(0x40302010);
        $this->shouldCall('_njGenerateFogTable3')->with($this->addressOf('_var_fogTable_8c18aaf8'), 42.0, 43.0);
        $this->shouldCall('_njFogEnable');
        $this->shouldCall('_kmSetCheapShadowMode')->with(0x80);
        $this->shouldCall('_kmSetFogTable')->with($this->addressOf('_var_fogTable_8c18aaf8'));

        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1ba5e8'), 0x10);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1ba808'), 0x20);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1bac28'), 0x40);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1bb448'), 0x20);

        // njRandomSeed
        $this->shouldCall('_srand')->with(0xcafe0001);
        $this->shouldCall('_AsqSetSeedA_12160')->with(0xcafe0001);
        $this->shouldCall('_AsqSetSeedB_121a2')->with(0xcafe0001);
        
        $this->shouldCall('_FUN_8c0128cc')->with(1);

        $this->shouldCall('_pushTask_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_task_8c012d5a'),
                0xffffe4,
                0xFFFFE8,
                0
            )
            ->do(function ($params) use ($createdTask) {
                $this->memory->writeUInt32($params[2], U32::of($createdTask));
            });

        $this->shouldRead(0xffffe4, $createdTask);
        $this->shouldWrite($createdTask + 0x08, 0);
        $this->shouldWrite($createdTask + 0x0c, 0);

        $this->shouldCall('_pushTask_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba5e8'),
                $this->addressOf('_task_8c016bf4'),
                0xffffe4,
                0xFFFFE8,
                0
            );

        $this->shouldCall('_FUN_8c025af4');

        $this->shouldWriteTo('_var_8c1bb8cc', 0);
        $this->shouldWriteTo('_var_8c22847c', 0);

        $this->shouldCall('_FUN_8c023610');
        $this->shouldCall('_FUN_8c02845a');
        
        $this->shouldCall('_FUN_8c0296d6');
        $this->shouldCall('_FUN_8c02769e');
        $this->shouldCall('_FUN_8c0222dc');
        $this->shouldCall('_FUN_8c02a6ac');
        $this->shouldCall('_FUN_8c02c46a');
        $this->shouldCall('_FUN_8c02018c');
        $this->shouldCall('_FUN_8c02d968');
        $this->shouldCall('_FUN_8c020528');

        $this->shouldCall('_pushTask_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba5e8'),
                new WildcardArgument,
                0xffffe4,
                0xFFFFE8,
                0
            )
            ->do(function ($params) use ($createdTask2) {
                $this->memory->writeUInt32($params[2], U32::of($createdTask2));
            });

        $this->shouldRead(0xffffe4, $createdTask2);
        $this->shouldWrite($createdTask2 + 0x08, 0);

        $this->shouldCall('_FUN_8c0228a2');

        $this->singleCall('_FUN_8c01306e')->run();
    }

    ////// task_8c013388 //////

    public function test_task_8c013388_field0x08Is0_PvmBoolIs0()
    {
        // FIXME
        $this->doNotRandomizeMemory();

        $taskPtr = $this->alloc(0xc);

        $this->shouldCall('_getUknPvmBool_8c01432a')
            ->andReturn(0);

        $this->singleCall('_task_8c013388')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_task_8c013388_field0x08Is0_PvmBoolIs1()
    {
        // FIXME
        $this->doNotRandomizeMemory();

        $taskPtr = $this->alloc(0xc);

        $var_loadedFooNjm_8c1bc448Ptr = $this->alloc(0x08);
        $this->initUint32($this->addressOf('_var_loadedFooNjm_8c1bc448'), $var_loadedFooNjm_8c1bc448Ptr);
        $this->initUint32($var_loadedFooNjm_8c1bc448Ptr + 4, 42);

        $this->shouldCall('_getUknPvmBool_8c01432a')->andReturn(1);

        $this->shouldWrite($taskPtr + 8, 1);
        $this->shouldWriteTo('_var_8c1bc450', fdec(41));

        $this->shouldCall('_AsqResetQueues_11f6c');
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SOUND", "manatee.drv", $this->addressOf('_var_memblkSource_8c0fcd48'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SOUND", "bus.mlt", $this->addressOf('_var_memblkSource_8c0fcd4c'));
        $this->shouldCall('_resetUknPvmBool_8c014322');
        $this->shouldCall('_AsqProcessQueues_11fe0')->with($this->addressOf('_AsqNop_11120'), 0, 0, 0, $this->addressOf('_setUknPvmBool_8c014330'));

        $this->singleCall('_task_8c013388')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_task_8c013388_field0x08Is1_PvmBoolIs0()
    {
        // FIXME
        $this->doNotRandomizeMemory();

        $taskPtr = $this->alloc(0xc);
        $this->initUint32($taskPtr + 8, 1);

        $this->shouldCall('_getUknPvmBool_8c01432a')->andReturn(0);

        $this->singleCall('_task_8c013388')
            ->with($taskPtr, 0)
            ->run();
    }

    public function test_task_8c013388_field0x08Is1_PvmBoolIs1()
    {
        // FIXME
        $this->doNotRandomizeMemory();

        $taskPtr = $this->alloc(0xc);
        $this->initUint32($taskPtr + 8, 1);
        //$var_8c2260a8Ptr = $this->allocRellocate('_var_8c2260a8', 4);

        $this->shouldCall('_getUknPvmBool_8c01432a')->andReturn(1);

        $this->shouldCall('_AsqFreeQueues_11f7e');
        $this->shouldCall('_freeTask_8c014b66')->with($taskPtr);
        $this->shouldCall('_initSoundMidiAdx_8c010e18');
        $this->shouldWriteTo('_var_8c2260a8', 1);
        $this->shouldCall('_pushTitle_8c015fd6');

        $this->singleCall('_task_8c013388')
            ->with($taskPtr, 0)
            ->run();
    }

    ////// njUserInit_8c0134ec //////

    public function test_njUserInit_8c0134ec_Vga_getSoundMode_8c010924Returns1()
    {
        // Resolutions/Bindings
        $this->setSize('_menuState_8c1bc7a8', 0x6c);

        /* Stack locals */
        $infoLocal = 0xffffbc;
        $createdTaskLocal = 0xffffb4;
        $createdStateLocal = 0xffffb8;

        /* Expectations */
        $this->shouldCall('_njSetTextureMemorySize')->with(0x100000);

        // SYE_CBL_CABLE_VGA
        $this->shouldCall('_syCblCheckCable')->andReturn(0);

        $this->shouldCall('_sbInitSystem_8c0149b0')
            ->with(
                0x31, // NJD_RESOLUTION_VGA
                0x00, // NJD_FRAMEBUFFER_MODE_RGB565
                2
            );

        $this->shouldCall('_njInitMatrix')->with($this->addressOf('_var_matrix_8c2f8ca0'), 16, 0);
        $this->shouldCall('_njInit3D')->with($this->addressOf('_var_vbuf_8c255ca0'), 2048);
        $this->shouldCall('_njInitVertexBuffer')->with(800000, 320000, 320000, 320000, 20000);
        $this->shouldCall('_njInitTextureBuffer')->with($this->addressOf('_var_texbuf_8c277ca0'), 0x80800);
        $this->shouldCall('_njInitTexture')->with($this->addressOf('_var_tex_8c157af8'), 3072);
        $this->shouldCall('_njInitCacheTextureBuffer')->with($this->addressOf('_var_cachebuf_8c235ca0'), 0x20000);
        $this->shouldCall('_njInitShape')->with($this->addressOf('_var_shapebuf_8c2f84a0'));
        $this->shouldCall('_syRtcInit');

        $this->shouldCall('_getSoundMode_8c010924')->andReturn(1);
        $this->shouldWriteByteTo('_var_soundMode_8c226070', 1);
        $this->shouldCall('_setSoundMode_8c0108c0')->with(1);

        $this->shouldCall('_vibClear_8c010fbe');
        $this->shouldCall('_bupInit_8c014b8c');

        $this->shouldCall('_njSetTextureInfo')
            ->with(
                $infoLocal,
                $this->addressOf('_var_texbuf_8c277ca0'),
                0xb01, // NJD_TEXFMT_STRIDE | NJD_TEXFMT_RGB_565
                256, // RENDER_X
                512, // RENDER_Y
            );

        $this->shouldCall('_njSetTextureName')->with(
            $this->addressOf('_var_texname_8c18acf8'),
            $infoLocal,
            999,
            0x40800000, // NJD_TEXATTR_TYPE_MEMORY|NJD_TEXATTR_GLOBALINDEX
        );

        $this->shouldCall('_njSetRenderWidth')->with(256);
        $this->shouldCall('_njLoadTexture')->with($this->addressOf('_init_texlist_8c03bf44'));

        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1ba3c8'), 0x10);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1ba5e8'), 0x10);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1ba808'), 0x20);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1bac28'), 0x40);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1bb448'), 0x20);

        $this->shouldWriteTo('_var_8c1bb86c', -1);

        $this->shouldCall('_clearUnknowArray_8c013bbc')->with($this->addressOf('_var_8c1bbddc'), 0x20);
        $this->shouldCall('_clearUnknowArray_8c013bbc')->with($this->addressOf('_var_8c1bbfdc'), 0x41);

        $this->shouldWriteTo('_var_8c1bc3ec', -1);
        $this->shouldWriteTo('_var_8c1bc3f0', -1);
        $this->shouldWriteTo('_var_8c1bc3f4', -1);

        $this->shouldCall('_clearUnknownVar_8c02171c');
        $this->shouldCall('_clearUnknownVar_8c029acc');
        $this->shouldCall('_clearUnknownVars_8c02aa28');

        $this->shouldWriteTo('_var_8c1bc404', -1);
        $this->shouldWriteTo('_var_8c226434', -1);
        $this->shouldWriteTo('_var_8c226438', -1);
        $this->shouldWriteTo('_var_8c228234', -1);
        $this->shouldWriteTo('_var_8c227e20', -1);
        $this->shouldWriteTo('_var_8c227e24', -1);
        $this->shouldWriteTo('_var_8c2288f8', -1);
        $this->shouldWriteTo('_var_8c1bc438', -1);
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x0 + 0, -1);
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0xc + 0, -1);
        $this->shouldWriteTo('_var_resourceGroup_8c2263a8', -1);
        $this->shouldWriteTo('_var_8c1ba2e0', -1);
        $this->shouldWriteTo('_var_8c1ba348', -1);
        $this->shouldWriteTo('_var_8c1ba344', -1);
        $this->shouldWriteTo('_var_8c225fb0', -1);
        $this->shouldWriteTo('_var_demoBuf_8c1ba3c4', -1);
        $this->shouldWriteTo('_var_8c1bc454', -1);
        $this->shouldWriteTo('_var_selectedVm_8c1ba34c', -1);


        $this->shouldWriteTo('_var_8c1bb8c4', 0);
        $this->shouldWriteTo('_var_demoIndex_8c1bb8d8', 100);
        $this->shouldWriteTo('_var_8c157a6c', 0);

        $this->shouldCall('_FUN_8c01c8dc');
        $this->shouldCall('_FUN_8c0189d2');
        $this->shouldCall('_njSetBorderColor')->with(0);
        $this->shouldCall('_vmsLcd_8c01c8fc')->with(3);
        $this->shouldCall('_vmsLcd_8c01c910');

        // FIXME: Confusing var names
        $createdTaskPtr = $this->alloc(0x0c);
        $this->initUint32($createdTaskLocal, $createdTaskPtr);

        $this->shouldCall('_pushTask_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            new WildcardArgument, // TODO: Export for testing
            $createdTaskLocal,
            $createdStateLocal,
            0
        );
        $this->shouldWrite($createdTaskPtr + 0x08, 0);

        $this->shouldCall('_AsqInitQueues_11f36')->with(0x10, 8, 0, 8);
        $this->shouldCall('_AsqResetQueues_11f6c');

        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "mark_parts.dat", $this->addressOf('_var_mark_parts_dat_8c1bc41c'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "mark.dat", $this->addressOf('_var_mark_dat_8c1bc420'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "busstop_parts.dat", $this->addressOf('_var_busstop_parts_dat_8c1bc428'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "busstop.dat", $this->addressOf('_var_busstop_dat_8c1bc42c'));

        $this->shouldCall('_AsqRequestPvm_11ac0')->with("\\SYSTEM", "loading.pvm", $this->addressOf('_var_8c1bc3f8'), 1, 0x80000000);
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "load_parts.dat", $this->addressOf('_var_8c1bc3f8') + 4);
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "loading.dat", $this->addressOf('_var_8c1bc3f8') + 8);

        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "bus_font.fff", $this->addressOf('_var_busFont_8c1ba1c8'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "vm_bus.lcd", $this->addressOf('_var_8c2260ac'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "vm_danger.lcd", $this->addressOf('_var_8c2260b8'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "now_loading.lcd", $this->addressOf('_var_8c2260c4'));

        $this->shouldCall('_AsqRequestPvm_11ac0')->with("\\SYSTEM", "fuu.pvm", $this->addressOf('_var_8c1bc440'), 1, 0);
        $this->shouldCall('_AsqRequestNj_11492')->with("\\SYSTEM", "fuu.njd", $this->addressOf('_var_8c1bc444'), 0);
        $this->shouldCall('_AsqRequestNj_11492')->with("\\SYSTEM", "fuu.njm", $this->addressOf('_var_loadedFooNjm_8c1bc448'), 0);
        $this->shouldCall('_AsqRequestNj_11492')->with("\\SD_COMMON", "3s_bus_m2.njm", $this->addressOf('_var_8c1bc410'), 0);
        $this->shouldCall('_AsqRequestNj_11492')->with("\\SD_COMMON", "3s_bus_m2.njs", $this->addressOf('_var_8c1bc414'), 0);

        $this->shouldCall('_resetUknPvmBool_8c014322');
        $this->shouldCall('_AsqProcessQueues_11fe0')->with($this->addressOf('_AsqNop_11120'), 0, 0, 0, $this->addressOf('_setUknPvmBool_8c014330'));;

        $this->shouldWriteTo('_var_gdErr_8c18ad14', 0);

        $this->shouldCall('_gdFsEntryErrFuncAll')->with(new WildcardArgument, 0);

        $this->singleCall('_njUserInit_8c0134ec')->run();
    }

    public function test_njUserInit_8c0134ec_Vga_getSoundMode_8c010924ReturnsNegative()
    {
        // Resolutions/Bindings
        $this->setSize('_menuState_8c1bc7a8', 0x6c);

        /* Stack locals */
        $infoLocal = 0xffffbc;
        $createdTaskLocal = 0xffffb4;
        $createdStateLocal = 0xffffb8;

        /* Expectations */
        $this->shouldCall('_njSetTextureMemorySize')->with(0x100000);

        // SYE_CBL_CABLE_VGA
        $this->shouldCall('_syCblCheckCable')->andReturn(0);

        $this->shouldCall('_sbInitSystem_8c0149b0')
            ->with(
                0x31, // NJD_RESOLUTION_VGA
                0x00, // NJD_FRAMEBUFFER_MODE_RGB565
                2
            );

        $this->shouldCall('_njInitMatrix')->with($this->addressOf('_var_matrix_8c2f8ca0'), 16, 0);
        $this->shouldCall('_njInit3D')->with($this->addressOf('_var_vbuf_8c255ca0'), 2048);
        $this->shouldCall('_njInitVertexBuffer')->with(800000, 320000, 320000, 320000, 20000);
        $this->shouldCall('_njInitTextureBuffer')->with($this->addressOf('_var_texbuf_8c277ca0'), 0x80800);
        $this->shouldCall('_njInitTexture')->with($this->addressOf('_var_tex_8c157af8'), 3072);
        $this->shouldCall('_njInitCacheTextureBuffer')->with($this->addressOf('_var_cachebuf_8c235ca0'), 0x20000);
        $this->shouldCall('_njInitShape')->with($this->addressOf('_var_shapebuf_8c2f84a0'));
        $this->shouldCall('_syRtcInit');

        // FIXME: Handle SInt8 -1, using & 0xff for now... 
        $this->shouldCall('_getSoundMode_8c010924')->andReturn(-1 & 0xff);
        $this->shouldWriteByteTo('_var_soundMode_8c226070', -1 & 0xff);
        $this->shouldCall('_setSoundMode_8c0108c0')->with(0);

        $this->shouldCall('_vibClear_8c010fbe');
        $this->shouldCall('_bupInit_8c014b8c');

        $this->shouldCall('_njSetTextureInfo')
            ->with(
                $infoLocal,
                $this->addressOf('_var_texbuf_8c277ca0'),
                0xb01, // NJD_TEXFMT_STRIDE | NJD_TEXFMT_RGB_565
                256, // RENDER_X
                512, // RENDER_Y
            );

        $this->shouldCall('_njSetTextureName')->with(
            $this->addressOf('_var_texname_8c18acf8'),
            $infoLocal,
            999,
            0x40800000, // NJD_TEXATTR_TYPE_MEMORY|NJD_TEXATTR_GLOBALINDEX
        );

        $this->shouldCall('_njSetRenderWidth')->with(256);
        $this->shouldCall('_njLoadTexture')->with($this->addressOf('_init_texlist_8c03bf44'));

        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1ba3c8'), 0x10);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1ba5e8'), 0x10);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1ba808'), 0x20);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1bac28'), 0x40);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1bb448'), 0x20);

        $this->shouldWriteTo('_var_8c1bb86c', -1);

        $this->shouldCall('_clearUnknowArray_8c013bbc')->with($this->addressOf('_var_8c1bbddc'), 0x20);
        $this->shouldCall('_clearUnknowArray_8c013bbc')->with($this->addressOf('_var_8c1bbfdc'), 0x41);

        $this->shouldWriteTo('_var_8c1bc3ec', -1);
        $this->shouldWriteTo('_var_8c1bc3f0', -1);
        $this->shouldWriteTo('_var_8c1bc3f4', -1);

        $this->shouldCall('_clearUnknownVar_8c02171c');
        $this->shouldCall('_clearUnknownVar_8c029acc');
        $this->shouldCall('_clearUnknownVars_8c02aa28');

        $this->shouldWriteTo('_var_8c1bc404', -1);
        $this->shouldWriteTo('_var_8c226434', -1);
        $this->shouldWriteTo('_var_8c226438', -1);
        $this->shouldWriteTo('_var_8c228234', -1);
        $this->shouldWriteTo('_var_8c227e20', -1);
        $this->shouldWriteTo('_var_8c227e24', -1);
        $this->shouldWriteTo('_var_8c2288f8', -1);
        $this->shouldWriteTo('_var_8c1bc438', -1);
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x0 + 0, -1);
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0xc + 0, -1);
        $this->shouldWriteTo('_var_resourceGroup_8c2263a8', -1);
        $this->shouldWriteTo('_var_8c1ba2e0', -1);
        $this->shouldWriteTo('_var_8c1ba348', -1);
        $this->shouldWriteTo('_var_8c1ba344', -1);
        $this->shouldWriteTo('_var_8c225fb0', -1);
        $this->shouldWriteTo('_var_demoBuf_8c1ba3c4', -1);
        $this->shouldWriteTo('_var_8c1bc454', -1);
        $this->shouldWriteTo('_var_selectedVm_8c1ba34c', -1);


        $this->shouldWriteTo('_var_8c1bb8c4', 0);
        $this->shouldWriteTo('_var_demoIndex_8c1bb8d8', 100);
        $this->shouldWriteTo('_var_8c157a6c', 0);

        $this->shouldCall('_FUN_8c01c8dc');
        $this->shouldCall('_FUN_8c0189d2');
        $this->shouldCall('_njSetBorderColor')->with(0);
        $this->shouldCall('_vmsLcd_8c01c8fc')->with(3);
        $this->shouldCall('_vmsLcd_8c01c910');

        // FIXME: Confusing var names
        $createdTaskPtr = $this->alloc(0x0c);
        $this->initUint32($createdTaskLocal, $createdTaskPtr);

        $this->shouldCall('_pushTask_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            new WildcardArgument, // TODO: Export for testing
            $createdTaskLocal,
            $createdStateLocal,
            0
        );
        $this->shouldWrite($createdTaskPtr + 0x08, 0);

        $this->shouldCall('_AsqInitQueues_11f36')->with(0x10, 8, 0, 8);
        $this->shouldCall('_AsqResetQueues_11f6c');

        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "mark_parts.dat", $this->addressOf('_var_mark_parts_dat_8c1bc41c'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "mark.dat", $this->addressOf('_var_mark_dat_8c1bc420'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "busstop_parts.dat", $this->addressOf('_var_busstop_parts_dat_8c1bc428'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "busstop.dat", $this->addressOf('_var_busstop_dat_8c1bc42c'));

        $this->shouldCall('_AsqRequestPvm_11ac0')->with("\\SYSTEM", "loading.pvm", $this->addressOf('_var_8c1bc3f8'), 1, 0x80000000);
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "load_parts.dat", $this->addressOf('_var_8c1bc3f8') + 4);
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "loading.dat", $this->addressOf('_var_8c1bc3f8') + 8);

        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "bus_font.fff", $this->addressOf('_var_busFont_8c1ba1c8'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "vm_bus.lcd", $this->addressOf('_var_8c2260ac'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "vm_danger.lcd", $this->addressOf('_var_8c2260b8'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "now_loading.lcd", $this->addressOf('_var_8c2260c4'));

        $this->shouldCall('_AsqRequestPvm_11ac0')->with("\\SYSTEM", "fuu.pvm", $this->addressOf('_var_8c1bc440'), 1, 0);
        $this->shouldCall('_AsqRequestNj_11492')->with("\\SYSTEM", "fuu.njd", $this->addressOf('_var_8c1bc444'), 0);
        $this->shouldCall('_AsqRequestNj_11492')->with("\\SYSTEM", "fuu.njm", $this->addressOf('_var_loadedFooNjm_8c1bc448'), 0);
        $this->shouldCall('_AsqRequestNj_11492')->with("\\SD_COMMON", "3s_bus_m2.njm", $this->addressOf('_var_8c1bc410'), 0);
        $this->shouldCall('_AsqRequestNj_11492')->with("\\SD_COMMON", "3s_bus_m2.njs", $this->addressOf('_var_8c1bc414'), 0);

        $this->shouldCall('_resetUknPvmBool_8c014322');
        $this->shouldCall('_AsqProcessQueues_11fe0')->with($this->addressOf('_AsqNop_11120'), 0, 0, 0, $this->addressOf('_setUknPvmBool_8c014330'));;

        $this->shouldWriteTo('_var_gdErr_8c18ad14', 0);

        $this->shouldCall('_gdFsEntryErrFuncAll')->with(new WildcardArgument, 0);

        $this->singleCall('_njUserInit_8c0134ec')->run();
    }

    public function test_njUserInit_8c0134ec_Ntsci_getSoundMode_8c010924Returns1()
    {
        // Resolutions/Bindings
        $this->setSize('_menuState_8c1bc7a8', 0x6c);

        /* Stack locals */
        $infoLocal = 0xffffbc;
        $createdTaskLocal = 0xffffb4;
        $createdStateLocal = 0xffffb8;

        /* Expectations */
        $this->shouldCall('_njSetTextureMemorySize')->with(0x100000);

        // SYE_CBL_CABLE_NTSC
        $this->shouldCall('_syCblCheckCable')->andReturn(3);
        
        $this->shouldCall('_sbInitSystem_8c0149b0')
        ->with(
            0x38, // NJD_RESOLUTION_640x480_NTSCNI
            0x00, // NJD_FRAMEBUFFER_MODE_RGB565
            2
        );
        $this->shouldCall('_njSetAspect')->with(1.0, 0.91);

        $this->shouldCall('_njInitMatrix')->with($this->addressOf('_var_matrix_8c2f8ca0'), 16, 0);
        $this->shouldCall('_njInit3D')->with($this->addressOf('_var_vbuf_8c255ca0'), 2048);
        $this->shouldCall('_njInitVertexBuffer')->with(800000, 320000, 320000, 320000, 20000);
        $this->shouldCall('_njInitTextureBuffer')->with($this->addressOf('_var_texbuf_8c277ca0'), 0x80800);
        $this->shouldCall('_njInitTexture')->with($this->addressOf('_var_tex_8c157af8'), 3072);
        $this->shouldCall('_njInitCacheTextureBuffer')->with($this->addressOf('_var_cachebuf_8c235ca0'), 0x20000);
        $this->shouldCall('_njInitShape')->with($this->addressOf('_var_shapebuf_8c2f84a0'));
        $this->shouldCall('_syRtcInit');

        $this->shouldCall('_getSoundMode_8c010924')->andReturn(1);
        $this->shouldWriteByteTo('_var_soundMode_8c226070', 1);
        $this->shouldCall('_setSoundMode_8c0108c0')->with(1);

        $this->shouldCall('_vibClear_8c010fbe');
        $this->shouldCall('_bupInit_8c014b8c');

        $this->shouldCall('_njSetTextureInfo')
            ->with(
                $infoLocal,
                $this->addressOf('_var_texbuf_8c277ca0'),
                0xb01, // NJD_TEXFMT_STRIDE | NJD_TEXFMT_RGB_565
                256, // RENDER_X
                512, // RENDER_Y
            );

        $this->shouldCall('_njSetTextureName')->with(
            $this->addressOf('_var_texname_8c18acf8'),
            $infoLocal,
            999,
            0x40800000, // NJD_TEXATTR_TYPE_MEMORY|NJD_TEXATTR_GLOBALINDEX
        );

        $this->shouldCall('_njSetRenderWidth')->with(256);
        $this->shouldCall('_njLoadTexture')->with($this->addressOf('_init_texlist_8c03bf44'));

        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1ba3c8'), 0x10);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1ba5e8'), 0x10);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1ba808'), 0x20);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1bac28'), 0x40);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1bb448'), 0x20);

        $this->shouldWriteTo('_var_8c1bb86c', -1);

        $this->shouldCall('_clearUnknowArray_8c013bbc')->with($this->addressOf('_var_8c1bbddc'), 0x20);
        $this->shouldCall('_clearUnknowArray_8c013bbc')->with($this->addressOf('_var_8c1bbfdc'), 0x41);

        $this->shouldWriteTo('_var_8c1bc3ec', -1);
        $this->shouldWriteTo('_var_8c1bc3f0', -1);
        $this->shouldWriteTo('_var_8c1bc3f4', -1);

        $this->shouldCall('_clearUnknownVar_8c02171c');
        $this->shouldCall('_clearUnknownVar_8c029acc');
        $this->shouldCall('_clearUnknownVars_8c02aa28');

        $this->shouldWriteTo('_var_8c1bc404', -1);
        $this->shouldWriteTo('_var_8c226434', -1);
        $this->shouldWriteTo('_var_8c226438', -1);
        $this->shouldWriteTo('_var_8c228234', -1);
        $this->shouldWriteTo('_var_8c227e20', -1);
        $this->shouldWriteTo('_var_8c227e24', -1);
        $this->shouldWriteTo('_var_8c2288f8', -1);
        $this->shouldWriteTo('_var_8c1bc438', -1);
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x0 + 0, -1);
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0xc + 0, -1);
        $this->shouldWriteTo('_var_resourceGroup_8c2263a8', -1);
        $this->shouldWriteTo('_var_8c1ba2e0', -1);
        $this->shouldWriteTo('_var_8c1ba348', -1);
        $this->shouldWriteTo('_var_8c1ba344', -1);
        $this->shouldWriteTo('_var_8c225fb0', -1);
        $this->shouldWriteTo('_var_demoBuf_8c1ba3c4', -1);
        $this->shouldWriteTo('_var_8c1bc454', -1);
        $this->shouldWriteTo('_var_selectedVm_8c1ba34c', -1);


        $this->shouldWriteTo('_var_8c1bb8c4', 0);
        $this->shouldWriteTo('_var_demoIndex_8c1bb8d8', 100);
        $this->shouldWriteTo('_var_8c157a6c', 0);

        $this->shouldCall('_FUN_8c01c8dc');
        $this->shouldCall('_FUN_8c0189d2');
        $this->shouldCall('_njSetBorderColor')->with(0);
        $this->shouldCall('_vmsLcd_8c01c8fc')->with(3);
        $this->shouldCall('_vmsLcd_8c01c910');

        // FIXME: Confusing var names
        $createdTaskPtr = $this->alloc(0x0c);
        $this->initUint32($createdTaskLocal, $createdTaskPtr);

        $this->shouldCall('_pushTask_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            new WildcardArgument, // TODO: Export for testing
            $createdTaskLocal,
            $createdStateLocal,
            0
        );
        $this->shouldWrite($createdTaskPtr + 0x08, 0);

        $this->shouldCall('_AsqInitQueues_11f36')->with(0x10, 8, 0, 8);
        $this->shouldCall('_AsqResetQueues_11f6c');

        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "mark_parts.dat", $this->addressOf('_var_mark_parts_dat_8c1bc41c'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "mark.dat", $this->addressOf('_var_mark_dat_8c1bc420'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "busstop_parts.dat", $this->addressOf('_var_busstop_parts_dat_8c1bc428'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "busstop.dat", $this->addressOf('_var_busstop_dat_8c1bc42c'));

        $this->shouldCall('_AsqRequestPvm_11ac0')->with("\\SYSTEM", "loading.pvm", $this->addressOf('_var_8c1bc3f8'), 1, 0x80000000);
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "load_parts.dat", $this->addressOf('_var_8c1bc3f8') + 4);
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "loading.dat", $this->addressOf('_var_8c1bc3f8') + 8);

        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "bus_font.fff", $this->addressOf('_var_busFont_8c1ba1c8'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "vm_bus.lcd", $this->addressOf('_var_8c2260ac'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "vm_danger.lcd", $this->addressOf('_var_8c2260b8'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "now_loading.lcd", $this->addressOf('_var_8c2260c4'));

        $this->shouldCall('_AsqRequestPvm_11ac0')->with("\\SYSTEM", "fuu.pvm", $this->addressOf('_var_8c1bc440'), 1, 0);
        $this->shouldCall('_AsqRequestNj_11492')->with("\\SYSTEM", "fuu.njd", $this->addressOf('_var_8c1bc444'), 0);
        $this->shouldCall('_AsqRequestNj_11492')->with("\\SYSTEM", "fuu.njm", $this->addressOf('_var_loadedFooNjm_8c1bc448'), 0);
        $this->shouldCall('_AsqRequestNj_11492')->with("\\SD_COMMON", "3s_bus_m2.njm", $this->addressOf('_var_8c1bc410'), 0);
        $this->shouldCall('_AsqRequestNj_11492')->with("\\SD_COMMON", "3s_bus_m2.njs", $this->addressOf('_var_8c1bc414'), 0);

        $this->shouldCall('_resetUknPvmBool_8c014322');
        $this->shouldCall('_AsqProcessQueues_11fe0')->with($this->addressOf('_AsqNop_11120'), 0, 0, 0, $this->addressOf('_setUknPvmBool_8c014330'));;

        $this->shouldWriteTo('_var_gdErr_8c18ad14', 0);

        $this->shouldCall('_gdFsEntryErrFuncAll')->with(new WildcardArgument, 0);

        $this->singleCall('_njUserInit_8c0134ec')->run();
    }

    public function test_njUserInit_8c0134ec_Ntsci_getSoundMode_8c010924ReturnsNegative()
    {
        // Resolutions/Bindings
        $this->setSize('_menuState_8c1bc7a8', 0x6c);

        /* Stack locals */
        $infoLocal = 0xffffbc;
        $createdTaskLocal = 0xffffb4;
        $createdStateLocal = 0xffffb8;

        /* Expectations */
        $this->shouldCall('_njSetTextureMemorySize')->with(0x100000);

        // SYE_CBL_CABLE_NTSC
        $this->shouldCall('_syCblCheckCable')->andReturn(3);
        
        $this->shouldCall('_sbInitSystem_8c0149b0')
        ->with(
            0x38, // NJD_RESOLUTION_640x480_NTSCNI
            0x00, // NJD_FRAMEBUFFER_MODE_RGB565
            2
        );
        $this->shouldCall('_njSetAspect')->with(1.0, 0.91);

        $this->shouldCall('_njInitMatrix')->with($this->addressOf('_var_matrix_8c2f8ca0'), 16, 0);
        $this->shouldCall('_njInit3D')->with($this->addressOf('_var_vbuf_8c255ca0'), 2048);
        $this->shouldCall('_njInitVertexBuffer')->with(800000, 320000, 320000, 320000, 20000);
        $this->shouldCall('_njInitTextureBuffer')->with($this->addressOf('_var_texbuf_8c277ca0'), 0x80800);
        $this->shouldCall('_njInitTexture')->with($this->addressOf('_var_tex_8c157af8'), 3072);
        $this->shouldCall('_njInitCacheTextureBuffer')->with($this->addressOf('_var_cachebuf_8c235ca0'), 0x20000);
        $this->shouldCall('_njInitShape')->with($this->addressOf('_var_shapebuf_8c2f84a0'));
        $this->shouldCall('_syRtcInit');

        // FIXME andReturn(-1) or andReturn(new SInt8(-1))?
        $this->shouldCall('_getSoundMode_8c010924')->andReturn(-1 & 0xff);
        $this->shouldWriteByteTo('_var_soundMode_8c226070', -1 & 0xff);
        $this->shouldCall('_setSoundMode_8c0108c0')->with(0);

        $this->shouldCall('_vibClear_8c010fbe');
        $this->shouldCall('_bupInit_8c014b8c');

        $this->shouldCall('_njSetTextureInfo')
            ->with(
                $infoLocal,
                $this->addressOf('_var_texbuf_8c277ca0'),
                0xb01, // NJD_TEXFMT_STRIDE | NJD_TEXFMT_RGB_565
                256, // RENDER_X
                512, // RENDER_Y
            );

        $this->shouldCall('_njSetTextureName')->with(
            $this->addressOf('_var_texname_8c18acf8'),
            $infoLocal,
            999,
            0x40800000, // NJD_TEXATTR_TYPE_MEMORY|NJD_TEXATTR_GLOBALINDEX
        );

        $this->shouldCall('_njSetRenderWidth')->with(256);
        $this->shouldCall('_njLoadTexture')->with($this->addressOf('_init_texlist_8c03bf44'));

        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1ba3c8'), 0x10);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1ba5e8'), 0x10);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1ba808'), 0x20);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1bac28'), 0x40);
        $this->shouldCall('_clearTasks_8c014a9c')->with($this->addressOf('_var_tasks_8c1bb448'), 0x20);

        $this->shouldWriteTo('_var_8c1bb86c', -1);

        $this->shouldCall('_clearUnknowArray_8c013bbc')->with($this->addressOf('_var_8c1bbddc'), 0x20);
        $this->shouldCall('_clearUnknowArray_8c013bbc')->with($this->addressOf('_var_8c1bbfdc'), 0x41);

        $this->shouldWriteTo('_var_8c1bc3ec', -1);
        $this->shouldWriteTo('_var_8c1bc3f0', -1);
        $this->shouldWriteTo('_var_8c1bc3f4', -1);

        $this->shouldCall('_clearUnknownVar_8c02171c');
        $this->shouldCall('_clearUnknownVar_8c029acc');
        $this->shouldCall('_clearUnknownVars_8c02aa28');

        $this->shouldWriteTo('_var_8c1bc404', -1);
        $this->shouldWriteTo('_var_8c226434', -1);
        $this->shouldWriteTo('_var_8c226438', -1);
        $this->shouldWriteTo('_var_8c228234', -1);
        $this->shouldWriteTo('_var_8c227e20', -1);
        $this->shouldWriteTo('_var_8c227e24', -1);
        $this->shouldWriteTo('_var_8c2288f8', -1);
        $this->shouldWriteTo('_var_8c1bc438', -1);
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x0 + 0, -1);
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0xc + 0, -1);
        $this->shouldWriteTo('_var_resourceGroup_8c2263a8', -1);
        $this->shouldWriteTo('_var_8c1ba2e0', -1);
        $this->shouldWriteTo('_var_8c1ba348', -1);
        $this->shouldWriteTo('_var_8c1ba344', -1);
        $this->shouldWriteTo('_var_8c225fb0', -1);
        $this->shouldWriteTo('_var_demoBuf_8c1ba3c4', -1);
        $this->shouldWriteTo('_var_8c1bc454', -1);
        $this->shouldWriteTo('_var_selectedVm_8c1ba34c', -1);

        $this->shouldWriteTo('_var_8c1bb8c4', 0);
        $this->shouldWriteTo('_var_demoIndex_8c1bb8d8', 100);
        $this->shouldWriteTo('_var_8c157a6c', 0);

        $this->shouldCall('_FUN_8c01c8dc');
        $this->shouldCall('_FUN_8c0189d2');
        $this->shouldCall('_njSetBorderColor')->with(0);
        $this->shouldCall('_vmsLcd_8c01c8fc')->with(3);
        $this->shouldCall('_vmsLcd_8c01c910');

        // FIXME: Confusing var names
        $createdTaskPtr = $this->alloc(0x0c);
        $this->initUint32($createdTaskLocal, $createdTaskPtr);

        $this->shouldCall('_pushTask_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            new WildcardArgument, // TODO: Export for testing
            $createdTaskLocal,
            $createdStateLocal,
            0
        );
        $this->shouldWrite($createdTaskPtr + 0x08, 0);

        $this->shouldCall('_AsqInitQueues_11f36')->with(0x10, 8, 0, 8);
        $this->shouldCall('_AsqResetQueues_11f6c');

        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "mark_parts.dat", $this->addressOf('_var_mark_parts_dat_8c1bc41c'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "mark.dat", $this->addressOf('_var_mark_dat_8c1bc420'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "busstop_parts.dat", $this->addressOf('_var_busstop_parts_dat_8c1bc428'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "busstop.dat", $this->addressOf('_var_busstop_dat_8c1bc42c'));

        $this->shouldCall('_AsqRequestPvm_11ac0')->with("\\SYSTEM", "loading.pvm", $this->addressOf('_var_8c1bc3f8'), 1, 0x80000000);
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "load_parts.dat", $this->addressOf('_var_8c1bc3f8') + 4);
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "loading.dat", $this->addressOf('_var_8c1bc3f8') + 8);

        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "bus_font.fff", $this->addressOf('_var_busFont_8c1ba1c8'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "vm_bus.lcd", $this->addressOf('_var_8c2260ac'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "vm_danger.lcd", $this->addressOf('_var_8c2260b8'));
        $this->shouldCall('_AsqRequestDat_11182')->with("\\SYSTEM", "now_loading.lcd", $this->addressOf('_var_8c2260c4'));

        $this->shouldCall('_AsqRequestPvm_11ac0')->with("\\SYSTEM", "fuu.pvm", $this->addressOf('_var_8c1bc440'), 1, 0);
        $this->shouldCall('_AsqRequestNj_11492')->with("\\SYSTEM", "fuu.njd", $this->addressOf('_var_8c1bc444'), 0);
        $this->shouldCall('_AsqRequestNj_11492')->with("\\SYSTEM", "fuu.njm", $this->addressOf('_var_loadedFooNjm_8c1bc448'), 0);
        $this->shouldCall('_AsqRequestNj_11492')->with("\\SD_COMMON", "3s_bus_m2.njm", $this->addressOf('_var_8c1bc410'), 0);
        $this->shouldCall('_AsqRequestNj_11492')->with("\\SD_COMMON", "3s_bus_m2.njs", $this->addressOf('_var_8c1bc414'), 0);

        $this->shouldCall('_resetUknPvmBool_8c014322');
        $this->shouldCall('_AsqProcessQueues_11fe0')->with($this->addressOf('_AsqNop_11120'), 0, 0, 0, $this->addressOf('_setUknPvmBool_8c014330'));;

        $this->shouldWriteTo('_var_gdErr_8c18ad14', 0);

        $this->shouldCall('_gdFsEntryErrFuncAll')->with(new WildcardArgument, 0);

        $this->singleCall('_njUserInit_8c0134ec')->run();
    }

    ////// njUserMain_8c01392e //////

    public function test_njUserMain_8c01392e_happyPath()
    {
        $this->resolveNjUserMain();

        $this->initUint32($this->addressOf('_var_vibport_8c1ba354'), 0xbebacafe);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0);
        $this->initUint32($this->addressOf('_var_queuesAreInitialized_8c157a60'), 0);
        $this->initUint32($this->addressOf('_init_8c03bfa8'), 1);
        $this->initUint32($this->addressOf('_var_gdErr_8c18ad14'), 0);

        $this->shouldCall('_gdFsGetSysHn')->andReturn(0xbeba0001);
        // GDD_STAT_IDLE
        $this->shouldCall('_gdFsGetStat')->with(0xbeba0001)->andReturn(0);
        $this->shouldWriteTo('_init_8c03bfa8', 0);

        // GDD_DRVSTAT_BUSY
        $this->shouldCall('_gdFsGetDrvStat')->andReturn(0x00);

        // GDD_DRVSTAT_BUSY
        $this->shouldCall('_gdFsGetDrvStat')->andReturn(0x00);

        $this->shouldCall('_gdFsReqDrvStat');
        
        $this->shouldReadFrom('_var_gdErr_8c18ad14', 0);
        
        $this->shouldCall('_execTasks_8c014b42')->with($this->addressOf('_var_tasks_8c1ba3c8'));

        $this->singleCall('_njUserMain_8c01392e')->singleShouldReturn(0)->run();
    }

    public function test_njUserMain_8c01392e_block1_ok()
    {
        $this->resolveNjUserMain();

        $this->initUint32($this->addressOf('_init_8c03bd80'), 1);
        $this->initUint32($this->addressOf('_init_8c03bd84'), 1);

        $this->shouldCall('_execTasks_8c014b42')->with($this->addressOf('_var_tasks_8c1ba3c8'));

        $this->singleCall('_njUserMain_8c01392e')->singleShouldReturn(0)->run();
    }

    public function test_njUserMain_8c01392e_block1_fail_noVib()
    {
        $this->resolveNjUserMain();

        $this->initUint32($this->addressOf('_init_8c03bd80'), 1);
        $this->initUint32($this->addressOf('_init_8c03bd84'), 0);
        // TODO: -1
        $this->initUint32($this->addressOf('_var_vibport_8c1ba354'), -1 & 0xffffffff);

        // FIXME: -1 & 0xffffffff
        $this->singleCall('_njUserMain_8c01392e')->singleShouldReturn(-1 & 0xffffffff)->run();
    }

    public function test_njUserMain_8c01392e_block1_fail_vib()
    {
        $this->resolveNjUserMain();

        $this->initUint32($this->addressOf('_var_vibport_8c1ba354'), 0xbebacafe);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 1);
        $this->initUint32($this->addressOf('_init_8c03bd84'), 0);

        $this->shouldReadFrom('_init_8c03bd80', 1);
        $this->shouldReadFrom('_init_8c03bd84', 0);
        $this->shouldReadFrom('_var_vibport_8c1ba354', 0xbebacafe);

        $this->shouldCall('_pdVibMxStop')->with(0xbebacafe);

        // FIXME: -1 & 0xffffffff
        $this->singleCall('_njUserMain_8c01392e')->singleShouldReturn(-1 & 0xffffffff)->run();
    }

    // public function test_njUserMain_8c01392e_0_0_0_idle_open_vib()
    // {
    //     $init_8c03bd80Ptr = $this->allocRellocate('_init_8c03bd80', 4);
    //     $var_queuesAreInitialized_8c157a60Ptr = $this->allocRellocate('_var_queuesAreInitialized_8c157a60', 4);
    //     $init_8c03bfa8Ptr = $this->allocRellocate('_init_8c03bfa8', 4);
    //     $var_vibport_8c1ba354Ptr = $this->allocRellocate('_var_vibport_8c1ba354', 4);
    //     $this->initUint32($var_vibport_8c1ba354Ptr, 0xbebacafe);

    //     $var_tasks_8c1ba3c8Ptr = $this->allocRellocate('_var_tasks_8c1ba3c8', 4);

    //     $this->shouldRead($init_8c03bd80Ptr, 0);
    //     $this->shouldRead($var_queuesAreInitialized_8c157a60Ptr, 0);
    //     $this->shouldRead($init_8c03bfa8Ptr, 0);

    //     $this->shouldCall('_gdFsReqDrvStat')->andReturn(0);
    //     $this->shouldWrite($init_8c03bfa8Ptr, 1);

    //     // GDD_DRVSTAT_OPEN
    //     $this->shouldCall('_gdFsGetDrvStat')->andReturn(0x06);

    //     $this->shouldRead($var_vibport_8c1ba354Ptr, 0xbebacafe);
    //     $this->shouldCall('_pdVibMxStop')->with(0xbebacafe);

    //     // FIXME: -1 & 0xffffffff
    //     $this->singleCall('_njUserMain_8c01392e')->singleShouldReturn(-1 & 0xffffffff)->run();
    // }

    // public function test_njUserMain_8c01392e_0_0_0_idle_open()
    // {
    //     $init_8c03bd80Ptr = $this->allocRellocate('_init_8c03bd80', 4);
    //     $var_queuesAreInitialized_8c157a60Ptr = $this->allocRellocate('_var_queuesAreInitialized_8c157a60', 4);
    //     $init_8c03bfa8Ptr = $this->allocRellocate('_init_8c03bfa8', 4);
    //     $var_vibport_8c1ba354Ptr = $this->allocRellocate('_var_vibport_8c1ba354', 4);
    //     $this->initUint32($var_vibport_8c1ba354Ptr, 0xbebacafe);

    //     $var_tasks_8c1ba3c8Ptr = $this->allocRellocate('_var_tasks_8c1ba3c8', 4);

    //     $this->shouldRead($init_8c03bd80Ptr, 0);
    //     $this->shouldRead($var_queuesAreInitialized_8c157a60Ptr, 0);
    //     $this->shouldRead($init_8c03bfa8Ptr, 0);
        
    //     $this->shouldCall('_gdFsReqDrvStat')->andReturn(0);
    //     $this->shouldWrite($init_8c03bfa8Ptr, 1);

    //     // GDD_DRVSTAT_OPEN
    //     $this->shouldCall('_gdFsGetDrvStat')->andReturn(0x06);

    //     $this->shouldRead($var_vibport_8c1ba354Ptr, -1);

    //     // FIXME: -1 & 0xffffffff
    //     $this->singleCall('_njUserMain_8c01392e')->singleShouldReturn(-1 & 0xffffffff)->run();
    // }

    private function resolveNjUserMain()
    {
        $this->setSize('_var_vibport_8c1ba354', 4);
        $this->setSize('_init_8c03bd80', 4);
        $this->setSize('_var_queuesAreInitialized_8c157a60', 4);
        $this->setSize('_var_gdErr_8c18ad14', 4);
        $this->setSize('_var_tasks_8c1ba3c8', 4);

        // Functions
        $this->setSize('_gdFsGetSysHn', 4);
        $this->setSize('_gdFsGetStat', 4);
        $this->setSize('_gdFsGetDrvStat', 4);
        $this->setSize('_gdFsReqDrvStat', 4);
        $this->setSize('_execTasks_8c014b42', 4);
        $this->setSize('_pdVibMxStop', 4);
    }

    private function allocRellocate($name, $size)
    {
        $ptr = $this->alloc($size);
        $this->rellocate($name, $ptr);
        return $ptr;
    }
};
