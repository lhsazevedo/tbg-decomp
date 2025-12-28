<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    protected ?string $objectFile = __DIR__ . '/main.obj';

    public function testState0x00_Init_SkipTitleAnimationWhenStartIsPressed() {
        /* Arrange */
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18 , 0x0b);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 8);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0xbebacafe);

        /* Assert */
        $this->shouldCall('_sdMidiPlay')->with(0xbebacafe, 1, 0, 0);

        $this->shouldWrite($this->addressOf('_var_peripherals_8c1ba35c') + 16, 0);
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 0x0e);
        $this->shouldWrite($this->addressOf('_isFading_8c226568'), 0);

        $this->forceStop();

        /* Act */
        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x00_Init_AdvanceToFortyFiveFadeIn() {
        // FIXME
        $this->doNotRandomizeMemory();

        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18 , 0);

        $this->shouldCall('_getUknPvmBool_8c01432a')->andReturn(0);
        $this->shouldCall('_AsqFreeQueues_11f7e');
        $this->shouldCall('_VmMenuMountVms_1940e');

        $task = $this->alloc(0x0c);
        $this->shouldRead($task + 0x08, 0);

        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 1);

        $this->shouldCall('_push_fadein_8c022a9c')->with(20);

        $this->shouldCall('_njSetBackColor')->with(0xff000000, 0xff000000, 0xff000000);

        $this->singleCall('_task_title_8c015ab8')
            ->with($task, 0)
            ->run();
    }

    public function testState0x00_Init_NoopWhenUknPvmBoolIsTrue() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18 , 0);

        $this->shouldCall('_getUknPvmBool_8c01432a')->andReturn(1);

        // TODO: Fix Task size
        $taskPtr = $this->alloc(0x0c);

        $this->singleCall('_task_title_8c015ab8')
            ->with($taskPtr, 0)
            ->run();
    }

    public function testState0x00_Init_SkipToTitleFadeInDirectWhenTaskField0x08IsTrue() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18 , 0);

        $this->shouldCall('_getUknPvmBool_8c01432a')->andReturn(0);
        $this->shouldCall('_AsqFreeQueues_11f7e');
        $this->shouldCall('_VmMenuMountVms_1940e');

        // TODO: Fix Task size
        $taskPtr = $this->alloc(0x0c);
        $this->initUint32($taskPtr + 0x08, 1);

        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 0x0d);

        $this->shouldCall('_push_fadein_8c022a9c')->with(10);

        $this->shouldCall('_njSetBackColor')->with(0xffffffff, 0xffffffff, 0xffffffff);

        $this->singleCall('_task_title_8c015ab8')
            ->with($taskPtr, 0)
            ->run();
    }

    public function testState0x01_FortyfiveFadeIn_WaitsForFadeInBeforeAdvancing() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18 , 1);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);

        $this->shouldCall('_drawSprite_8c014f54')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0, 0.0, 0.0, -5.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x01_FortyfiveFadeIn_AdvancesWhenFadeInIsOver() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 1);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 2);
        // Init logo timer
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x68, 0);

        $this->shouldCall('_drawSprite_8c014f54')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0, 0.0, 0.0, -5.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x02_Fortyfive_WaitsForTimerBeforePushingFadeOut() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 29);

        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x68, 30);

        $this->shouldCall('_drawSprite_8c014f54')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0, 0.0, 0.0, -5.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x02_Fortyfive_PushesFadeOutAfterThirteenTicks() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 30);

        // Check timer
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x68, 31);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 3);

        $this->shouldCall('_push_fadeout_8c022b60')->with(20);

        $this->shouldCall('_drawSprite_8c014f54')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0, 0.0, 0.0, -5.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x03_FortyfiveFadeOut_WaitsForFadeOutBeforeAdvancing() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 3);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);

        $this->shouldCall('_drawSprite_8c014f54')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0, 0.0, 0.0, -5.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x03_FortyfiveFadeOut_AdvancesAndPushesFadeInAfterFadeOut() {
        $this->resolveImports();


        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 3);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 4);
        $this->shouldCall('_push_fadein_8c022a9c')->with(20);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x04_AdxFadeIn_WaitsForFadeInBeforeAdvancing() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 4);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);

        $this->shouldCall('_drawSprite_8c014f54')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 3, 0.0, 0.0, -5.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x04_AdxFadeIn_AdvancesWhenFadeInIsOver() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 4);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 5);
        // Init logo timer
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x68, 0);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 3, 0.0, 0.0, -5.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x05_Adx_WaitsForTimerBeforePushingFadeOut() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 5);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 29);

        // Check timer
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x68, 30);

        $this->shouldCall('_drawSprite_8c014f54')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 3, 0.0, 0.0, -5.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x05_Adx_PushesFadeOutAfterThirteenTicks() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 5);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 30);

        // Check timer
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x68, 31);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 6);

        $this->shouldCall('_push_fadeout_8c022b60')->with(20);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 3, 0.0, 0.0, -5.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x06_AdxFadeOut_WaitsForFadeOutBeforeAdvancing() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 6);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);

        $this->shouldCall('_drawSprite_8c014f54')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 3, 0.0, 0.0, -5.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x06_AdxFadeOut_AdvancesToTitleWhenFirstConditionFails() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 6);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        $this->shouldCall('_FUN_8c012984')->andReturn(0);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 0x0a);
        $this->shouldCall('_push_fadein_8c022a9c')->with(10);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x06_AdxFadeOut_AdvancesToTitleWhenSecondConditionFails() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 6);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        $this->shouldCall('_FUN_8c012984')->andReturn(1);

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')
            ->with($this->addressOf('_init_saveNames_8c044d50'), 3)
            ->andReturn(1);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 0x0a);
        $this->shouldCall('_push_fadein_8c022a9c')->with(10);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x06_AdxFadeOut_AdvancesToWarningWhenBothConditionsPasses() {
        $this->resolveImports();


        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 6);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        $this->shouldCall('_FUN_8c012984')->andReturn(1);

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')
            ->with($this->addressOf('_init_saveNames_8c044d50'), 3)
            ->andReturn(0);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 0x07);
        $this->shouldCall('_push_fadein_8c022a9c')->with(10);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x047_VmuWarningFadeIn_WaitsForFadeInBeforeAdvancing() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 7);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 17, 0.0, 0.0, -5.0);
        $this->shouldCall('_njSetBackColor')->with(0xffffffff, 0xffffffff, 0xffffffff);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x07_VmuWarningFadeIn_AdvancesWhenFadeInIsOver() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 7);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 8);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 17, 0.0, 0.0, -5.0);
        $this->shouldCall('_njSetBackColor')->with(0xffffffff, 0xffffffff, 0xffffffff);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x08_VmuWarning_WaitsWhenNoInputOrSaveNames() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 8);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 16, 0);

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')->with($this->addressOf('_init_saveNames_8c044d50'), 3)->andReturn(0);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 17, 0.0, 0.0, -5.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x08_VmuWarning_AdvancesWhenStartIsPressed() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 8);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 16, 1 << 3);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0xbebacafe);

        $this->shouldCall('_sdMidiPlay')->with(0xbebacafe, 1, 0, 0, 0);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 9);

        $this->shouldCall('_push_fadeout_8c022b60')->with(10);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 17, 0.0, 0.0, -5.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x08_VmuWarning_AdvancesWhenAIsPressed() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 8);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 16, 1 << 2);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0xbebacafe);

        $this->shouldCall('_sdMidiPlay')->with(0xbebacafe, 1, 0, 0, 0);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 9);

        $this->shouldCall('_push_fadeout_8c022b60')->with(10);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 17, 0.0, 0.0, -5.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x08_VmuWarning_DoesNotAdvancesWhenOtherButtonsArePressed() {
        $this->resolveImports();


        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 8);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 16, 0xFFFFFFF3);

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')->with($this->addressOf('_init_saveNames_8c044d50'), 3)->andReturn(0);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 17, 0.0, 0.0, -5.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x08_VmuWarning_AdvancesWhenSaveNamePasses() {
        $this->resolveImports();


        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 8);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 16, 0);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0xbebacafe);

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')->with($this->addressOf('_init_saveNames_8c044d50'), 3)->andReturn(1);
        $this->shouldCall('_sdMidiPlay')->with(0xbebacafe, 1, 0, 0, 0);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 9);

        $this->shouldCall('_push_fadeout_8c022b60')->with(10);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 17, 0.0, 0.0, -5.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x09_VmuWarningFadeOut_WaitsForFadeOutBeforeAdvancing() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 9);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 9);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);

        $this->shouldCall('_drawSprite_8c014f54')
            ->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 17, 0.0, 0.0, -5.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x09_VmuWarningFadeOut_AdvancesToTitleAfterFadeOut() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x09);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 0x0a);
        $this->shouldCall('_push_fadein_8c022a9c')->with(10);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x0a_TitleFadeIn_WaitsForFadeInBeforeAdvancing() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0a);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0a);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 2, 0.0, 0.0, -5.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x00, 46, 0.0, 0.0, -7.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x0a_TitleFadeIn_AdvancesAfterFadeIn() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0a);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0a);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 0x0b);
        // 640.0 is stored as 0x44200000
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x20, 0x44200000);

        $this->shouldCall('_snd_8c010cd6')->with(0,0);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 2, 0.0, 0.0, -5.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x00, 46, 0.0, 0.0, -7.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x0b_BusSlide_AnimatesBusSlide() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0b);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 16, 0);
        // 640.0 is stored as 0x44200000
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x20, 0x44200000);

        // 634.888889
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x20, 0x441eb8e4);

        // Draw bus
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 1, 634.888889, 0.0, -4.0);
        
        // Draw title
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 2, 0.0, 0.0, -5.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x00, 46, 0.0, 0.0, -7.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x0b_BusSlide_AdvancesWhenBusReachesCenterOfScreen() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0b);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 16, 0);
        // 185.0 is stored as 0x43390000
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x20, 0x43390000);

        // 185.0 - 5.111111 is stored as 0x4333e38e
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x20, 0x4333e38e);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 0x0c);
        // Init flag Y position as 167.0
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x24, 0x43270000);

        // A break statement is missing, so we continue
        // to the next switch case (Flag Reveal).

        // 164.666671753
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x24, 0x4324aaab);

        // Draw flag
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 4, 302.0, 164.666671753, -4.5);

        // Draw bus
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 1, 180.0, 0.0, -4.0);

        // Draw title
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 2, 0.0, 0.0, -5.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x00, 46, 0.0, 0.0, -7.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x0c_FlagReveal_AnimatesFlag() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0c);
        // Anim skip check
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 16, 0);
        // 167.0 is stored as 0x43270000
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x24, 0x43270000);

        // 164.666671753
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x24, 0x4324aaab);

        // Draw flag
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 4, 302.0, 164.666671753, -4.5);

        // Draw bus
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 1, 180.0, 0.0, -4.0);
        
        // Draw title
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 2, 0.0, 0.0, -5.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x00, 46, 0.0, 0.0, -7.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x0c_FlagReveal_AdvancesWhenFlagIsRevealed() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0c);
        // Anim skip check
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 16, 0);
        // 98 is stored as 0x42c40000
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x24, 0x42c40000);

        // 95.6666641235
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x24, 0x42bf5555);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 0x0e);

        // Draw flag
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 4, 302.0, 95.6666641235, -4.5);

        // Draw bus
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 1, 180.0, 0.0, -4.0);
        
        // Draw title
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 2, 0.0, 0.0, -5.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x00, 46, 0.0, 0.0, -7.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x0d_TitleFadeInDirect_WaitsForFadeIn() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0d);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);

        // TODO: assert that state is still 0x0d when drawSprite is called;

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 5, 0.0, 0.0, -4.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 6, 0.0, 0.0, -4.5);
        // Draw flag
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 4, 302.0, 97.0, -4.5);
        // Draw bus
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 1, 180.0, 0.0, -4.0);
        // Draw title
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 2, 0.0, 0.0, -5.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x00, 46, 0.0, 0.0, -7.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x0d_TitleFadeInDirect_AdvancesAfterFadeIn() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0d);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 0x0e);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 5, 0.0, 0.0, -4.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 6, 0.0, 0.0, -4.5);
        // Draw flag
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 4, 302.0, 97.0, -4.5);
        // Draw bus
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 1, 180.0, 0.0, -4.0);
        // Draw title
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 2, 0.0, 0.0, -5.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x00, 46, 0.0, 0.0, -7.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x0e_PressStart_WaitsWhenNoInputAndTimeIsNotUp() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0e);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 16, 0);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x64, 1049);

        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x64, 1050);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 5, 0.0, 0.0, -4.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 6, 0.0, 0.0, -4.5);
        // Draw flag
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 4, 302.0, 97.0, -4.5);
        // Draw bus
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 1, 180.0, 0.0, -4.0);
        // Draw title
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 2, 0.0, 0.0, -5.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x00, 46, 0.0, 0.0, -7.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x0e_PressStart_AdvancesToStartPressedWhenStartIsPressed() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0e);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 16, 1 << 3);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0xbebacafe);

        $this->shouldCall('_FUN_8c010bae')->with(0);
        $this->shouldCall('_FUN_8c010bae')->with(1);

        $this->shouldCall('_sdMidiPlay')->with(0xbebacafe, 1, 0, 0);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 0x0f);
        // Reset logo timer
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x68, 0);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 5, 0.0, 0.0, -4.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 6, 0.0, 0.0, -4.5);
        // Draw flag
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 4, 302.0, 97.0, -4.5);
        // Draw bus
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 1, 180.0, 0.0, -4.0);
        // Draw title
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 2, 0.0, 0.0, -5.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x00, 46, 0.0, 0.0, -7.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x0e_PressStart_AdvancesToTimeOutWhenTimeIsUp() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0e);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 16, 0xFFFFFFF3);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x64, 1051);

        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x64, 1052);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 0x11);

        $this->shouldCall('_FUN_8c010bae')->with(0);
        $this->shouldCall('_FUN_8c010bae')->with(1);
        $this->shouldCall('_push_fadeout_8c022b60')->with(60);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 5, 0.0, 0.0, -4.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 6, 0.0, 0.0, -4.5);
        // Draw flag
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 4, 302.0, 97.0, -4.5);
        // Draw bus
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 1, 180.0, 0.0, -4.0);
        // Draw title
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 2, 0.0, 0.0, -5.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x00, 46, 0.0, 0.0, -7.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x0f_StartPressed_WaitsForTimer() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0f);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 1);

        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x68, 2);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 5, 0.0, 0.0, -4.0);

        // Blink check
        //$this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 6, 0.0, 0.0, -4.5);
        // Draw flag
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 4, 302.0, 97.0, -4.5);
        // Draw bus
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 1, 180.0, 0.0, -4.0);
        // Draw title
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 2, 0.0, 0.0, -5.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x00, 46, 0.0, 0.0, -7.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x0f_StartPressed_BlinksSpriteOnEveryOtherTickWhileWaitingForTimer() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0f);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 0);

        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x68, 1);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 5, 0.0, 0.0, -4.0);

        // Blink check
        $this->shouldRead($this->addressOf('_menuState_8c1bc7a8') + 0x68, 1);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 6, 0.0, 0.0, -4.5);

        // Draw flag
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 4, 302.0, 97.0, -4.5);
        // Draw bus
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 1, 180.0, 0.0, -4.0);
        // Draw title
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 2, 0.0, 0.0, -5.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x00, 46, 0.0, 0.0, -7.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x0f_StartPressed_AdvancesAfterTimer() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0f);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 10);

        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x68, 11);

        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 0x10);
        $this->shouldCall('_push_fadeout_8c022b60')->with(10);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 5, 0.0, 0.0, -4.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 6, 0.0, 0.0, -4.5);

        // Draw flag
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 4, 302.0, 97.0, -4.5);
        // Draw bus
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 1, 180.0, 0.0, -4.0);
        // Draw title
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 2, 0.0, 0.0, -5.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x00, 46, 0.0, 0.0, -7.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x10_StartPressedFadeOut_WaitsForFadeOut() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x10);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 0);

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')->with($this->addressOf('_init_saveNames_8c044d50'), 3);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 5, 0.0, 0.0, -4.0);

        // Blink check
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x68, 1);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 6, 0.0, 0.0, -4.5);

        // Draw flag
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 4, 302.0, 97.0, -4.5);
        // Draw bus
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 1, 180.0, 0.0, -4.0);
        // Draw title
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 2, 0.0, 0.0, -5.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x00, 46, 0.0, 0.0, -7.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x10_StartPressedFadeOut_BlinksSpriteWhileWaitingForFadeOut() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x10);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 1);

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')->with($this->addressOf('_init_saveNames_8c044d50'), 3);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 5, 0.0, 0.0, -4.0);

        // Blink check
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x68, 2);

        // Draw flag
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 4, 302.0, 97.0, -4.5);
        // Draw bus
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 1, 180.0, 0.0, -4.0);
        // Draw title
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 2, 0.0, 0.0, -5.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x00, 46, 0.0, 0.0, -7.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x10_StartPressedFadeOut_WaitsFor8c03bd80BeforeAdvancing() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x10);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 1);

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')->with($this->addressOf('_init_saveNames_8c044d50'), 3);

        $this->shouldReadSymbolOffset('_init_8c03bd80', 0, 1);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x10_StartPressedFadeOut_AdvancesWhenTimeIsUpAnd8c03bd80IsFalse() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x10);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0);

        $this->shouldCall('_VmMenuUpdateVmusStatus_19550')->with($this->addressOf('_init_saveNames_8c044d50'), 3);

        $this->shouldReadSymbolOffset('_init_8c03bd80', 0, 0);
        $this->shouldWriteSymbolOffset('_var_8c1bb8c4', 0, 0);
        $this->shouldCall('_VmMenuSwitchFromTask_19e44')->with(0xbebacafe);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0xbebacafe, 0)
            ->run();
    }

    public function testState0x11_TimeOut_WaitsForFadeOut() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x11);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);

        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 5, 0.0, 0.0, -4.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 6, 0.0, 0.0, -4.5);
        // Draw flag
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 4, 302.0, 97.0, -4.5);
        // Draw bus
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 1, 180.0, 0.0, -4.0);
        // Draw title
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x0c, 2, 0.0, 0.0, -5.0);
        $this->shouldCall('_drawSprite_8c014f54')->with($this->addressOf('_menuState_8c1bc7a8') + 0x00, 46, 0.0, 0.0, -7.0);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x11_TimeOut_WaitsFor8c03bd80BeforeAdvancing() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x11);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 1);   

        $this->shouldReadSymbolOffset('_init_8c03bd80', 0, 1);

        $this->singleCall('_task_title_8c015ab8')
            ->with(0, 0)
            ->run();
    }

    public function testState0x11_TimeOut_AdvancesWhenFadedAnd8c03bd80IsFalse() {
        // FIXME
        $this->doNotRandomizeMemory();

        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x11);

        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);
        $this->shouldReadSymbolOffset('_init_8c03bd80', 0, 0);
        $this->shouldCall('_FUN_8c016182');
        $this->shouldCall('_FUN_demo_8c0159ac');

        $this->singleCall('_task_title_8c015ab8')
            ->with(0xbebacafe, 0)
            ->run();
    }

    public function testState0x0b_BusSlide_SkipsToPressStartWhenStartIsPressed() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0b);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 16, 1 << 3);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0xbebacafe);

        $this->shouldReadFrom('_var_midiHandles_8c0fcd28', 0xbebacafe);
        $this->shouldCall('_sdMidiPlay')->with(0xbebacafe, 1, 0, 0);

        $this->shouldWrite($this->addressOf('_var_peripherals_8c1ba35c') + 16, 0);
        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 0x0e);
        $this->shouldWriteSymbolOffset('_isFading_8c226568', 0, 0);

        // Switch case
        // $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0e);

        $this->forceStop();

        $this->singleCall('_task_title_8c015ab8')
            ->with(0xbebacafe, 0)
            ->run();
    }

    public function testState0x0c_FlagReveal_SkipsToPressStartWhenStartIsPressed() {
        $this->resolveImports();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0c);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 16, 1 << 3);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0xbebacafe);

        $this->shouldReadSymbolOffset('_var_midiHandles_8c0fcd28', 0, 0xbebacafe);
        $this->shouldCall('_sdMidiPlay')->with(0xbebacafe, 1, 0, 0);

        $this->shouldWrite($this->addressOf('_var_peripherals_8c1ba35c') + 16, 0);
        // Advance title state
        $this->shouldWriteSymbolOffset('_menuState_8c1bc7a8', 0x18, 0x0e);
        $this->shouldWriteSymbolOffset('_isFading_8c226568', 0, 0);

        // Switch case
        // $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0x0e);

        $this->forceStop();

        $this->singleCall('_task_title_8c015ab8')
            ->with(0xbebacafe, 0)
            ->run();

    }

    private function resolveImports() {
        $this->setSize('_drawSprite_8c014f54', 4);
        $this->setSize('_menuState_8c1bc7a8', 0x6c);
        // sizeof PERIPHERAL = 52
        $this->setSize('_var_peripherals_8c1ba35c', 52 * 2);
        $this->setSize('_var_midiHandles_8c0fcd28', 0x8);
        $this->setSize('_isFading_8c226568', 4);

        /* Functions */
        $this->setSize('_push_fadeout_8c022b60', 4);
        $this->setSize('_push_fadein_8c022a9c', 4);
        $this->setSize('_getUknPvmBool_8c01432a', 4);
        $this->setSize('_FUN_8c010bae', 4);
    }
};
