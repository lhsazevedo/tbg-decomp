<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// RESULTS task state machine (menuState.state_0x18).
return new class extends TestCase {
    private int $ms;

    private function setup(int $state): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_var_peripherals_8c1ba35c', 0x68);
        $this->setSize('_var_vmuStatus_8c226048', 0x24);
        $this->setSize('_var_progress_8c1ba1cc', 0xe8);
        $this->setSize('_var_midiHandles_8c0fcd28', 0x20);
        $this->setSize('_init_saveNames_8c044d50', 4 * 11);
        $this->setSize('_init_titleResourceGroup_8c044254', 0x10);

        foreach ([
            '_SpriteDraw_8c014f54', '_MessageBoxMenuTextboxText_8c02af1c',
            '_MessageBoxSwapFor_8c02aefc', '_VmSelectUpdateStatus_8c01967c',
            '_RouteGetLatch_8c01432a', '_AsqFreeQueues_8c011f7e',
            '_SndMidiResetFxAndPlay_8c010846', '_sdMidiPlay',
            '_RenderStartFadeOut_8c022b60', '_PromptHandleBinary_8c016caa',
            '_RenderStartFadeIn_8c022a9c', '_FileSelectResetProgress_8c01890a',
            '_SystemMenuWriteToVmu_8c01b26c', '_VmGameSetLcdAnim_8c01c8fc',
            '_BupGetInfo_8c014bba', '_buStat', '_buGetLastError', '_syFree',
            '_ReplayMenuFreeSessionAssets_8c016182', '_CourseMenuReturn_8c017ef2',
            '_CourseMenuBuildCourseUnlockList_8c0172dc',
            '_CourseMenuApplyUnlocks_8c0173e6', '_TitleStart_8c015fd6',
            '_njSetBackColor', '_InputSpawnTask_8c0128cc', '_GameSoftResetTask_8c012f44',
            '_TaskSpawn_8c014ae8', '_njGarbageTexture', '_MessageBoxOpenTextbox_8c02ae3e',
            '_AsqInitQueues_8c011f36', '_AsqResetQueues_8c011f6c',
            '_CourseMenuRequestSysResgrp_8c018568',
            '_CourseMenuRequestCommonResources_8c01852c', '_AsqRequestDat_8c011182',
            '_RouteSetLatch_8c014330', '_RouteClearLatch_8c014322',
            '_AsqNop_8c011120', '_AsqProcessQueues_8c011fe0', '_EndingStart_8c01f954',
        ] as $fn) {
            $this->setSize($fn, 4);
        }

        foreach ([
            '_var_selectedVm_8c1ba34c', '_var_isFading_8c226568', '_var_saveSlot_8c1ba350',
            '_var_runSucceeded_8c1bb8dc', '_var_award_8c1bb8f8',
            '_var_gameMode_8c1bb8fc', '_var_vmBusy_8c157a7c', '_var_backupFileImageBuf_8c1ba348',
            '_var_tasks_8c1ba3c8', '_var_tex_8c157af8', '_var_vmuIconFileBuf_8c1ba344',
            '_var_timeOfDay_8c18ad20', '_var_route_8c18ad1c', '_var_firstClearOfCourse_8c1bb8e0',
            '_var_passengerCount_8c1bb8e4', '_var_eventCount_8c1bb8e8',
        ] as $sym) {
            $this->setSize($sym, 4);
            $this->initUint32($this->addressOf($sym), 0);
        }

        foreach ([
            '_var_scoreTotal_8c226404', '_var_scoreEventBonus_8c226400', '_var_scorePassengerBonus_8c2263fc', '_var_scoreBadgeBonus_8c2263f8',
            '_var_scoreDriverPointsBonus_8c2263f4', '_var_scoreFirstClearBonus_8c2263f0', '_var_scoreCourseClearBonus_8c2263ec',
        ] as $sym) {
            $this->initUint32($this->addressOf($sym), 0);
        }

        $this->setSize('_var_runState_8c2285c4', 0x9c);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x0c, 0); // driverPoints_0x0c

        $this->ms = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($this->ms + 0x18, $state); // state_0x18
        $this->initUint32($this->ms + 0x38, 0);       // selected_0x38
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0); // press
        $this->initUint32($this->addressOf('_var_runFailed_8c226408'), 0); // same-object global, no setSize
    }

    public function test_state_0xb_advances_to_0xc_once_fade_clears(): void
    {
        $this->setup(0xb);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldWriteLong($this->ms + 0x18, 0xc); // state_0x18 = 0xc

        $this->shouldCall('_drawTextboxSprite_8c01d864');

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_0xc_advances_to_0xd_and_fades_out_on_press(): void
    {
        $this->setup(0xc);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 4); // press & PDD_TA

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldWriteLong($this->ms + 0x18, 0xd); // state_0x18 = 0xd

        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);

        $this->shouldCall('_drawTextboxSprite_8c01d864');

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_0xc_stays_put_without_press(): void
    {
        $this->setup(0xc);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_drawTextboxSprite_8c01d864');

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_7_advances_to_4_and_fades_in_once_fade_clears(): void
    {
        $this->setup(7);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldWriteLong($this->ms + 0x18, 4); // state_0x18 = 4

        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    public function test_state_7_waits_while_fading(): void
    {
        $this->setup(7);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_drawTextboxSprite_8c01d864');

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_8_advances_to_9_once_fade_clears(): void
    {
        $this->setup(8);
        $this->initUint32($this->ms + 0x1c, 5); // subState_0x1c: garbage, must be reset
        $this->initUint32($this->ms + 0x38, 5); // selected_0x38: garbage, must be reset

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldWriteLong($this->ms + 0x18, 9);   // state_0x18 = 9
        $this->shouldWriteLong($this->ms + 0x1c, 0);   // subState_0x1c = 0
        $this->shouldWriteLong($this->ms + 0x38, 0);   // selected_0x38 = 0

        $this->shouldCall('_drawTextboxSprite_8c01d864');

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_8_waits_while_fading(): void
    {
        $this->setup(8);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_drawTextboxSprite_8c01d864');

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_10_switches_to_confirm_prompt_on_press(): void
    {
        $this->setup(10);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 4); // press & PDD_DGT_TA

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("セーブを中止しますか？");

        $this->shouldWriteLong($this->ms + 0x18, 6);  // state_0x18 = 6
        $this->shouldWriteLong($this->ms + 0x38, 1);  // selected_0x38 = 1

        $this->shouldCall('_drawTextboxSprite_8c01d864');

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_10_stays_put_without_press(): void
    {
        $this->setup(10);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_drawTextboxSprite_8c01d864');

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_9_field0_confirm_resets_progress_and_writes_vmu(): void
    {
        $this->setup(9);
        $this->initUint32($this->ms + 0x1c, 0); // subState_0x1c
        $this->initUint32($this->addressOf('_var_runFailed_8c226408'), 1); // failed run
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0x11223344);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')
            ->with(0, 0x11223344, 3);

        $this->shouldCall('_PromptHandleBinary_8c016caa')
            ->with($this->ms + 0x38)
            ->andReturn(1);

        $this->shouldCall('_FileSelectResetProgress_8c01890a');

        $this->shouldCall('_SystemMenuWriteToVmu_8c01b26c');

        $this->shouldWriteLong($this->ms + 0x1c, 1); // subState_0x1c = 1

        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("セーブ実行中です<E>電源を切らないで下さい");

        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(1);

        $this->shouldCall('_drawTextboxSprite_8c01d864');

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_9_field0_cancel_fades_out(): void
    {
        $this->setup(9);
        $this->initUint32($this->ms + 0x1c, 0); // subState_0x1c
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldCall('_PromptHandleBinary_8c016caa')
            ->with($this->ms + 0x38)
            ->andReturn(2);

        $this->shouldWriteLong($this->ms + 0x18, 0xd); // state_0x18 = 0xd

        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);

        $this->shouldCall('_drawTextboxSprite_8c01d864');

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_9_field0_no_prompt_answer_and_vm_not_connected_returns_to_state_4(): void
    {
        $this->setup(9);
        $this->initUint32($this->ms + 0x1c, 0); // subState_0x1c
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0);
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048'), 0); // VMU_STATUS_NOT_CONNECTED
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1); // must be reset to 0
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x11223344); // handle 0

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldCall('_PromptHandleBinary_8c016caa')
            ->with($this->ms + 0x38)
            ->andReturn(0);

        $this->shouldWriteLong($this->ms + 0x18, 4); // state_0x18 = 4

        $this->shouldWriteLong($this->addressOf('_var_isFading_8c226568'), 0);

        $this->shouldCall('_sdMidiPlay')->with(0x11223344, 1, 2, 0);

        $this->shouldCall('_drawTextboxSprite_8c01d864');

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_9_field0_no_prompt_answer_and_save_still_possible_stays_put(): void
    {
        $this->setup(9);
        $this->initUint32($this->ms + 0x1c, 0); // subState_0x1c
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0);
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048'), 4); // VMU_STATUS_SAVING_POSSIBLE

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldCall('_PromptHandleBinary_8c016caa')
            ->with($this->ms + 0x38)
            ->andReturn(0);

        $this->shouldCall('_drawTextboxSprite_8c01d864');

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_9_field1_write_ok_shows_done_message(): void
    {
        $this->setup(9);
        $this->initUint32($this->ms + 0x1c, 1); // subState_0x1c
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_vmBusy_8c157a7c'), 1); // must be reset to 0
        $this->initUint32($this->addressOf('_var_backupFileImageBuf_8c1ba348'), 0x11223344); // must be freed

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0);
        $this->shouldCall('_buStat')->with(0)->andReturn(0);
        $this->shouldCall('_buGetLastError')->with(0)->andReturn(0);

        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("セーブ終了");

        $this->shouldWriteLong($this->ms + 0x18, 0xd); // state_0x18 = 0xd

        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 0);

        $this->shouldCall('_syFree')->with(0x11223344);

        $this->shouldWriteLong($this->addressOf('_var_backupFileImageBuf_8c1ba348'), 0xffffffff);

        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);

        $this->shouldCall('_drawTextboxSprite_8c01d864');

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_9_field1_write_failed_shows_error_and_confirms_retry(): void
    {
        $this->setup(9);
        $this->initUint32($this->ms + 0x1c, 1); // subState_0x1c
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_vmBusy_8c157a7c'), 1);
        $this->initUint32($this->addressOf('_var_backupFileImageBuf_8c1ba348'), 0x11223344);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0);
        $this->shouldCall('_buStat')->with(0)->andReturn(0);
        $this->shouldCall('_buGetLastError')->with(0)->andReturn(1);

        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("セーブが失敗しました");

        $this->shouldWriteLong($this->ms + 0x18, 10); // state_0x18 = 10

        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 0);

        $this->shouldCall('_syFree')->with(0x11223344);

        $this->shouldWriteLong($this->addressOf('_var_backupFileImageBuf_8c1ba348'), 0xffffffff);

        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);

        $this->shouldCall('_drawTextboxSprite_8c01d864');

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_9_field1_still_writing_stays_put(): void
    {
        $this->setup(9);
        $this->initUint32($this->ms + 0x1c, 1); // subState_0x1c
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0);
        $this->shouldCall('_buStat')->with(0)->andReturn(1);

        $this->shouldCall('_drawTextboxSprite_8c01d864');

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_0_no_vm_selected_waits_for_pvm(): void
    {
        $this->setup(0);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0xffffffff); // -1: no VM selected

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(1);
    }

    public function test_state_0_vm_selected_updates_vmu_status_then_waits_for_pvm(): void
    {
        $this->setup(0);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0x11223344);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0x11223344, 3);

        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(1);
    }

    public function test_state_0_fanfare_uses_fx6_when_run_not_succeeded(): void
    {
        $this->setup(0);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0xffffffff); // -1: no VM selected
        $this->initUint32($this->addressOf('_var_runFailed_8c226408'), 0); // passed run
        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 0);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(0);

        $this->shouldCall('_AsqFreeQueues_8c011f7e');

        $this->shouldWriteLong($this->ms + 0x18, 1); // state_0x18 = 1

        $this->shouldCall('_SndMidiResetFxAndPlay_8c010846')->with(0, 6);

        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    public function test_state_0_fanfare_uses_fx5_when_run_succeeded(): void
    {
        $this->setup(0);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0xffffffff); // -1: no VM selected
        $this->initUint32($this->addressOf('_var_runFailed_8c226408'), 0); // passed run
        $this->initUint32($this->addressOf('_var_runSucceeded_8c1bb8dc'), 1);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(0);

        $this->shouldCall('_AsqFreeQueues_8c011f7e');

        $this->shouldWriteLong($this->ms + 0x18, 1); // state_0x18 = 1

        $this->shouldCall('_SndMidiResetFxAndPlay_8c010846')->with(0, 5);

        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    public function test_state_0_failed_run_falls_into_state_4_vm_not_connected(): void
    {
        $this->setup(0);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0x11223344);
        $this->initUint32($this->addressOf('_var_runFailed_8c226408'), 1); // failed run
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1); // must be reset to 0
        $this->initUint32($this->ms + 0x38, 5); // selected_0x38: garbage, must be reset
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048'), 0); // VMU_STATUS_NOT_CONNECTED
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x55667788);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0x11223344, 3);

        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(0);

        $this->shouldCall('_AsqFreeQueues_8c011f7e');

        $this->shouldWriteLong($this->addressOf('_var_isFading_8c226568'), 0);
        $this->shouldWriteLong($this->ms + 0x38, 0); // selected_0x38 = 0

        // case 4, entered by fallthrough:
        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0x11223344, 3);

        $this->shouldWriteLong($this->ms + 0x6c, 0); // selectedVmuSlot_0x6c caches the status

        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("ＶＭが接続されていません<E>セーブには３ブロック必要です");

        $this->shouldWriteLong($this->ms + 0x18, 5); // state_0x18 = 5

        $this->shouldCall('_sdMidiPlay')->with(0x55667788, 1, 2, 0);

        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    public function test_state_0_failed_run_falls_into_state_4_vm_not_available(): void
    {
        $this->setup(0);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0);
        $this->initUint32($this->addressOf('_var_runFailed_8c226408'), 1); // failed run
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048'), 1); // VMU_STATUS_NOT_AVAILABLE
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x55667788);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(0);

        $this->shouldCall('_AsqFreeQueues_8c011f7e');

        $this->shouldWriteLong($this->addressOf('_var_isFading_8c226568'), 0);
        $this->shouldWriteLong($this->ms + 0x38, 0); // selected_0x38 = 0

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldWriteLong($this->ms + 0x6c, 1); // selectedVmuSlot_0x6c caches the status

        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("このＶＭはセーブできない状態です");

        $this->shouldWriteLong($this->ms + 0x18, 5); // state_0x18 = 5

        $this->shouldCall('_sdMidiPlay')->with(0x55667788, 1, 2, 0);

        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    public function test_state_0_failed_run_falls_into_state_4_not_enough_space(): void
    {
        $this->setup(0);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0);
        $this->initUint32($this->addressOf('_var_runFailed_8c226408'), 1); // failed run
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048'), 2); // VMU_STATUS_NOT_ENOUGH_SPACE
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x55667788);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(0);

        $this->shouldCall('_AsqFreeQueues_8c011f7e');

        $this->shouldWriteLong($this->addressOf('_var_isFading_8c226568'), 0);
        $this->shouldWriteLong($this->ms + 0x38, 0); // selected_0x38 = 0

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldWriteLong($this->ms + 0x6c, 2); // selectedVmuSlot_0x6c caches the status

        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("空きブロックが不足しています<E>セーブには３ブロック必要です");

        $this->shouldWriteLong($this->ms + 0x18, 5); // state_0x18 = 5

        $this->shouldCall('_sdMidiPlay')->with(0x55667788, 1, 2, 0);

        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    public function test_state_0_failed_run_falls_into_state_4_saving_possible(): void
    {
        $this->setup(0);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0);
        $this->initUint32($this->addressOf('_var_runFailed_8c226408'), 1); // failed run
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048'), 4); // VMU_STATUS_SAVING_POSSIBLE
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x55667788);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(0);

        $this->shouldCall('_AsqFreeQueues_8c011f7e');

        $this->shouldWriteLong($this->addressOf('_var_isFading_8c226568'), 0);
        $this->shouldWriteLong($this->ms + 0x38, 0); // selected_0x38 = 0

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldWriteLong($this->ms + 0x6c, 4); // selectedVmuSlot_0x6c caches the status

        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("ファイルを作成します。<E>よろしいですか？");

        $this->shouldWriteLong($this->ms + 0x18, 8); // state_0x18 = 8

        $this->shouldCall('_sdMidiPlay')->with(0x55667788, 1, 0, 0);

        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    public function test_state_0_failed_run_falls_into_state_4_save_exists(): void
    {
        $this->setup(0);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0);
        $this->initUint32($this->addressOf('_var_runFailed_8c226408'), 1); // failed run
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048'), 5); // VMU_STATUS_SAVE_EXISTS_NO_SPACE
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x55667788);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(0);

        $this->shouldCall('_AsqFreeQueues_8c011f7e');

        $this->shouldWriteLong($this->addressOf('_var_isFading_8c226568'), 0);
        $this->shouldWriteLong($this->ms + 0x38, 0); // selected_0x38 = 0

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldWriteLong($this->ms + 0x6c, 5); // selectedVmuSlot_0x6c caches the status

        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("ファイルが上書きされます。<E>よろしいですか？");

        $this->shouldWriteLong($this->ms + 0x18, 8); // state_0x18 = 8

        $this->shouldCall('_sdMidiPlay')->with(0x55667788, 1, 0, 0);

        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    public function test_state_0_failed_run_falls_into_state_4_save_exists_no_space(): void
    {
        $this->setup(0);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0);
        $this->initUint32($this->addressOf('_var_runFailed_8c226408'), 1); // failed run
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048'), 6); // VMU_STATUS_SAVE_EXISTS
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x55667788);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(0);

        $this->shouldCall('_AsqFreeQueues_8c011f7e');

        $this->shouldWriteLong($this->addressOf('_var_isFading_8c226568'), 0);
        $this->shouldWriteLong($this->ms + 0x38, 0); // selected_0x38 = 0

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldWriteLong($this->ms + 0x6c, 6); // selectedVmuSlot_0x6c caches the status

        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("ファイルが上書きされます。<E>よろしいですか？");

        $this->shouldWriteLong($this->ms + 0x18, 8); // state_0x18 = 8

        $this->shouldCall('_sdMidiPlay')->with(0x55667788, 1, 0, 0);

        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    public function test_state_0_failed_run_falls_into_state_4_proceed_without_saving(): void
    {
        $this->setup(0);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0);
        $this->initUint32($this->addressOf('_var_runFailed_8c226408'), 1); // failed run
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048'), 3); // VMU_STATUS_PROCEED_WITHOUT_SAVING

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(0);

        $this->shouldCall('_AsqFreeQueues_8c011f7e');

        $this->shouldWriteLong($this->addressOf('_var_isFading_8c226568'), 0);
        $this->shouldWriteLong($this->ms + 0x38, 0); // selected_0x38 = 0

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldWriteLong($this->ms + 0x6c, 3); // selectedVmuSlot_0x6c caches the status

        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    public function test_state_1_advances_to_2_and_draws_indicator_once_fade_clears(): void
    {
        $this->setup(1);
        $this->initUint32($this->ms + 0x64, 5); // counter_0x64: garbage, must be reset
        $this->initUint32($this->ms + 0x68, 5); // timer_0x68: garbage, must be reset

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldWriteLong($this->ms + 0x18, 2); // state_0x18 = 2
        $this->shouldWriteLong($this->ms + 0x64, 0); // counter_0x64 = 0
        $this->shouldWriteLong($this->ms + 0x68, 0); // timer_0x68 = 0

        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($this->ms, 0x1b, 0.0, 0.0, -4.0);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_1_waits_while_fading(): void
    {
        $this->setup(1);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($this->ms, 0x1b, 0.0, 0.0, -4.0);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_2_no_digits_yet_before_timer_threshold(): void
    {
        $this->setup(2);
        $this->initUint32($this->ms + 0x64, 0); // counter_0x64
        $this->initUint32($this->ms + 0x68, 0); // timer_0x68: below threshold after increment

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldWriteLong($this->ms + 0x68, 1); // timer_0x68 = 1

        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($this->ms, 0x1b, 0.0, 0.0, -4.0);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_2_first_digit_revealed_when_timer_ticks_over(): void
    {
        $this->setup(2);
        $this->initUint32($this->ms + 0x64, 0);  // counter_0x64
        $this->initUint32($this->ms + 0x68, 10); // timer_0x68: crosses threshold on increment
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28') + 0x10, 0x55667788); // index 4

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldWriteLong($this->ms + 0x68, 11); // timer_0x68 = 11 (pre-reset increment)
        $this->shouldWriteLong($this->ms + 0x64, 1);  // counter_0x64 = 1
        $this->shouldWriteLong($this->ms + 0x68, 0);  // timer_0x68 = 0

        $this->shouldCall('_sdMidiPlay')->with(0x55667788, 1, 6, 0);

        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 96.0);

        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($this->ms, 0x1b, 0.0, 0.0, -4.0);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_2_all_digits_revealed_and_advances_to_3_with_no_award(): void
    {
        $this->setup(2);
        $this->initUint32($this->ms + 0x64, 6);  // counter_0x64: about to reach 7
        $this->initUint32($this->ms + 0x68, 10); // timer_0x68: crosses threshold on increment
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0); // no award
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28') + 0x10, 0x55667788); // index 4

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldWriteLong($this->ms + 0x68, 11); // timer_0x68 = 11 (pre-reset increment)
        $this->shouldWriteLong($this->ms + 0x64, 7);  // counter_0x64 = 7
        $this->shouldWriteLong($this->ms + 0x68, 0);  // timer_0x68 = 0

        $this->shouldCall('_sdMidiPlay')->with(0x55667788, 1, 6, 0);

        $this->shouldWriteLong($this->ms + 0x18, 3); // state_0x18 = 3

        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 312.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 266.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 232.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 198.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 164.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 130.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 96.0);

        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($this->ms, 0x1b, 0.0, 0.0, -4.0);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_2_reaching_7_plays_award_fx_gold(): void
    {
        $this->setup(2);
        $this->initUint32($this->ms + 0x64, 6);
        $this->initUint32($this->ms + 0x68, 10);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 3); // gold
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28') + 0x10, 0x55667788); // index 4

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldWriteLong($this->ms + 0x68, 11); // timer_0x68 = 11 (pre-reset increment)
        $this->shouldWriteLong($this->ms + 0x64, 7);  // counter_0x64 = 7
        $this->shouldWriteLong($this->ms + 0x68, 0);  // timer_0x68 = 0

        $this->shouldCall('_sdMidiPlay')->with(0x55667788, 1, 6, 0);

        $this->shouldCall('_SndMidiResetFxAndPlay_8c010846')->with(5, 2);

        $this->shouldWriteLong($this->ms + 0x18, 3); // state_0x18 = 3

        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 312.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 266.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 232.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 198.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 164.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 130.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 96.0);

        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($this->ms, 0x1b, 0.0, 0.0, -4.0);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_2_reaching_7_plays_award_fx_silver(): void
    {
        $this->setup(2);
        $this->initUint32($this->ms + 0x64, 6);
        $this->initUint32($this->ms + 0x68, 10);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 2); // silver
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28') + 0x10, 0x55667788); // index 4

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldWriteLong($this->ms + 0x68, 11); // timer_0x68 = 11 (pre-reset increment)
        $this->shouldWriteLong($this->ms + 0x64, 7);  // counter_0x64 = 7
        $this->shouldWriteLong($this->ms + 0x68, 0);  // timer_0x68 = 0

        $this->shouldCall('_sdMidiPlay')->with(0x55667788, 1, 6, 0);

        $this->shouldCall('_SndMidiResetFxAndPlay_8c010846')->with(5, 3);

        $this->shouldWriteLong($this->ms + 0x18, 3); // state_0x18 = 3

        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 312.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 266.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 232.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 198.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 164.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 130.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 96.0);

        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($this->ms, 0x1b, 0.0, 0.0, -4.0);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_2_reaching_7_plays_award_fx_bronze(): void
    {
        $this->setup(2);
        $this->initUint32($this->ms + 0x64, 6);
        $this->initUint32($this->ms + 0x68, 10);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 1); // bronze
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28') + 0x10, 0x55667788); // index 4

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldWriteLong($this->ms + 0x68, 11); // timer_0x68 = 11 (pre-reset increment)
        $this->shouldWriteLong($this->ms + 0x64, 7);  // counter_0x64 = 7
        $this->shouldWriteLong($this->ms + 0x68, 0);  // timer_0x68 = 0

        $this->shouldCall('_sdMidiPlay')->with(0x55667788, 1, 6, 0);

        $this->shouldCall('_SndMidiResetFxAndPlay_8c010846')->with(5, 4);

        $this->shouldWriteLong($this->ms + 0x18, 3); // state_0x18 = 3

        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 312.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 266.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 232.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 198.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 164.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 130.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 96.0);

        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($this->ms, 0x1b, 0.0, 0.0, -4.0);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_3_draws_digits_without_award_when_no_press(): void
    {
        $this->setup(3);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0); // no award

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 312.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 266.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 232.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 198.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 164.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 130.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 96.0);

        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($this->ms, 0x1b, 0.0, 0.0, -4.0);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_3_draws_award_sprite_when_award_nonzero(): void
    {
        $this->setup(3);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 2); // silver

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 312.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 266.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 232.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 198.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 164.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 130.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 96.0);

        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($this->ms, 0x1f - 2, 0.0, 0.0, -4.0);

        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($this->ms, 0x1b, 0.0, 0.0, -4.0);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_3_press_with_no_vm_selected_ends_results(): void
    {
        $this->setup(3);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 4); // press & PDD_DGT_TA
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0xffffffff); // -1: no VM selected
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldWriteLong($this->ms + 0x18, 0xd); // state_0x18 = 0xd

        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);

        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 312.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 266.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 232.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 198.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 164.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 130.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 96.0);

        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($this->ms, 0x1b, 0.0, 0.0, -4.0);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_3_press_with_game_mode_nonzero_ends_results(): void
    {
        $this->setup(3);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 4); // press & PDD_DGT_TA
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 1); // free-run mode
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 0); // days_0x00

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldWriteLong($this->ms + 0x18, 0xd); // state_0x18 = 0xd

        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);

        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 312.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 266.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 232.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 198.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 164.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 130.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 96.0);

        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($this->ms, 0x1b, 0.0, 0.0, -4.0);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_3_press_with_days_exceeded_ends_results(): void
    {
        $this->setup(3);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 4); // press & PDD_DGT_TA
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 0x1f); // days_0x00 > 0x1e

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldWriteLong($this->ms + 0x18, 0xd); // state_0x18 = 0xd

        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);

        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 312.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 266.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 232.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 198.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 164.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 130.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 96.0);

        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($this->ms, 0x1b, 0.0, 0.0, -4.0);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_3_press_with_vm_and_days_within_limit_advances_to_4(): void
    {
        $this->setup(3);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 4); // press & PDD_DGT_TA
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);
        $this->initUint32($this->addressOf('_var_award_8c1bb8f8'), 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 0x1e); // days_0x00 == 0x1e (not exceeded)

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldWriteLong($this->ms + 0x18, 4); // state_0x18 = 4

        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);

        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 312.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 266.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 232.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 198.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 164.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 130.0);
        $this->shouldCall('_drawScoreDigits_8c01d7fc')->with(0, 96.0);

        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($this->ms, 0x1b, 0.0, 0.0, -4.0);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_5_fading_skips_state_update(): void
    {
        $this->setup(5);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0x11223344);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0x11223344, 3);

        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0xff)->andReturn(0);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_5_slot_matches_no_press_stays_put(): void
    {
        $this->setup(5);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0);
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048'), 2);
        $this->initUint32($this->ms + 0x6c, 2); // cached status still matches

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0xff)->andReturn(0);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_5_slot_matches_press_confirms_cancel(): void
    {
        $this->setup(5);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 4); // press & PDD_DGT_TA
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0);
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048'), 2);
        $this->initUint32($this->ms + 0x6c, 2); // cached status still matches

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("セーブを中止しますか？");

        $this->shouldWriteLong($this->ms + 0x18, 6); // state_0x18 = 6
        $this->shouldWriteLong($this->ms + 0x38, 1); // selected_0x38 = 1

        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0xff)->andReturn(0);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_5_slot_mismatch_returns_to_state_4(): void
    {
        $this->setup(5);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0);
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048'), 1);
        $this->initUint32($this->ms + 0x6c, 2); // cached status: mismatched

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldWriteLong($this->ms + 0x18, 4); // state_0x18 = 4

        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0xff)->andReturn(0);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_5_draws_arrow_when_textbox_active(): void
    {
        $this->setup(5);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0xff)->andReturn(1);

        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($this->ms, 1, 0.0, 0.0, -4.3);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_6_confirm_cancels_results(): void
    {
        $this->setup(6);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0x11223344);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0x11223344, 3);

        $this->shouldCall('_PromptHandleBinary_8c016caa')
            ->with($this->ms + 0x38)
            ->andReturn(1);

        $this->shouldWriteLong($this->ms + 0x18, 0xd); // state_0x18 = 0xd

        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);

        $this->shouldCall('_drawTextboxSprite_8c01d864');

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_6_deny_returns_to_state_7(): void
    {
        $this->setup(6);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldCall('_PromptHandleBinary_8c016caa')
            ->with($this->ms + 0x38)
            ->andReturn(2);

        $this->shouldWriteLong($this->ms + 0x18, 7); // state_0x18 = 7

        $this->shouldCall('_drawTextboxSprite_8c01d864');

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_6_no_answer_stays_put(): void
    {
        $this->setup(6);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0, 3);

        $this->shouldCall('_PromptHandleBinary_8c016caa')
            ->with($this->ms + 0x38)
            ->andReturn(0);

        $this->shouldCall('_drawTextboxSprite_8c01d864');

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_0xd_fading_draws_arrow_when_textbox_active(): void
    {
        $this->setup(0xd);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0xff)->andReturn(1);

        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($this->ms, 1, 0.0, 0.0, -4.3);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_0xd_fading_no_textbox(): void
    {
        $this->setup(0xd);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0xff)->andReturn(0);

        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    public function test_state_0xd_failed_run_pushes_title(): void
    {
        $this->setup(0xd);
        $this->initUint32($this->addressOf('_var_runFailed_8c226408'), 1); // failed run

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_ReplayMenuFreeSessionAssets_8c016182');

        $this->shouldCall('_TitleStart_8c015fd6')->with(0);
    }

    public function test_state_0xd_days_exceeded_builds_unlocks(): void
    {
        $this->setup(0xd);
        $this->initUint32($this->addressOf('_var_runFailed_8c226408'), 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 0x1f); // days_0x00 > 0x1e

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_ReplayMenuFreeSessionAssets_8c016182');

        $this->shouldCall('_CourseMenuBuildCourseUnlockList_8c0172dc');
        $this->shouldCall('_CourseMenuApplyUnlocks_8c0173e6');
        $this->shouldCall('_EndingStart_8c01f954');
    }

    public function test_state_0xd_normal_return_to_course_menu(): void
    {
        $this->setup(0xd);
        $this->initUint32($this->addressOf('_var_runFailed_8c226408'), 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 0x1e); // days_0x00 == 0x1e (not exceeded)

        $this->call('_resultsTask_8c01d8e0');

        $this->shouldCall('_ReplayMenuFreeSessionAssets_8c016182');

        $this->shouldCall('_CourseMenuReturn_8c017ef2');
    }
};
