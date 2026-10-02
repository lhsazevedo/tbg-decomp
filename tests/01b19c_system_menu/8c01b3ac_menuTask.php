<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /* The asm loads these external function pointers into callee-saved regs at
     * function entry, so every path reads the relocations even when it never
     * calls them. Resolve any not otherwise exercised by the test. */
    private function resolveEagerPointers(bool $txtDraw = true, bool $swapBox = true, bool $midi = true): void
    {
        if ($txtDraw) {
            $this->setSize('_SpriteDraw_8c014f54', 4);
        }
        if ($swapBox) {
            $this->setSize('_MessageBoxSwapFor_8c02aefc', 4);
        }
        if ($midi) {
            $this->setSize('_sdMidiPlay', 4);
        }
        /* C build hoists &var_midiHandles at entry on every path. */
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0);
    }

    /* Reserve the full MenuState (0x7c) so no adjacent global overlaps its
     * fields; addressOf alone allocates minimally and lets +0x38/+0x3c collide. */
    private function menuStateBase(): int
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        return $this->addressOf('_var_menuState_8c1bc7a8');
    }

    /* Draw the current save/load screen (common tail, states 1-8). */
    private function expectDrawTail(int $menuState, int $selected, int $textboxResult): void
    {
        $rgA = $menuState;              // resourceGroupA_0x00
        $rgB = $menuState + 0xc;        // resourceGroupB_0x0c

        $this->shouldCall('_SpriteDraw_8c014f54')->with($rgB, $selected + 5, 0.0, 0.0, -4.0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($rgB, 4, 0.0, 0.0, -5.0);
        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0xff)->andReturn($textboxResult);
        if ($textboxResult) {
            $this->shouldCall('_SpriteDraw_8c014f54')->with($rgA, 1, 0.0, 0.0, -5.0);
        }
        $this->shouldCall('_SpriteDraw_8c014f54')->with($rgA, 0, 0.0, 0.0, -7.0);
    }

    /* Top-of-function VMU status refresh (states < 7, VMU selected). Seeds a
     * valid slot and returns the resulting vmStatus into vmuStatus[0]. */
    private function expectVmStatusTop(int $vmStatus): void
    {
        $this->setSize('_init_saveNames_8c044d50', 0x2c);
        $this->initUint32($this->addressOf('_init_saveNames_8c044d50'), 0x8c440000);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_vmuStatus_8c226048'), $vmStatus);
        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(0, 0x8c440000, 3);
    }

    /* State 2 confirm handler: PromptHandleMultiple + optional cursor sfx. */
    private function state2Enter(int $menuState, int $selected, int $press): void
    {
        $this->initUint32($menuState + 0x18, 2);
        $this->initUint32($menuState + 0x38, $selected);
        /* Reserve one PDS_PERIPHERAL so .press (+0x10) can't overlap a neighbor. */
        $this->setSize('_var_peripherals_8c1ba35c', 0x34);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, $press);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0);
    }

    /* Draw tail for the confirm states (3/5/6): the highlight sprite over the
     * active yes/no option, then the common screen draw. */
    private function expectPromptTail(int $menuState, int $field3c, int $selected, int $textboxResult): void
    {
        $this->shouldCall('_SpriteDraw_8c014f54')->with($menuState, $field3c + 2, 228.0, 266.0, -4.0);
        $this->expectDrawTail($menuState, $selected, $textboxResult);
    }

    /* Enter a confirm state (3/5/6): state, sub-state subState_0x1c, prompt cursor
     * cursorCol_0x3c, selected, and the entry-hoisted globals. */
    private function stateEnter(int $menuState, int $state, int $field1c, int $field3c, int $selected): void
    {
        $this->initUint32($menuState + 0x18, $state);
        $this->initUint32($menuState + 0x1c, $field1c);
        $this->initUint32($menuState + 0x38, $selected);
        $this->initUint32($menuState + 0x3c, $field3c);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0);
    }

    /* Enter an exit state (7/8): no top VMU refresh runs (state >= 7), but the
     * entry still hoists &selectedVm; seed the fade/teardown gates. */
    private function exitStateEnter(int $menuState, int $state, int $selected, int $isFading, int $gate): void
    {
        $this->initUint32($menuState + 0x18, $state);
        $this->initUint32($menuState + 0x38, $selected);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), $isFading);
        $this->initUint32($this->addressOf('_var_vmMountBusy_8c22606c'), $gate);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0);
    }

    public function test_state0_waits_for_pvm(): void
    {
        $this->resolveEagerPointers();
        $menuState = $this->menuStateBase();
        $this->initUint32($menuState + 0x18, 0);            // state
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), -1);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(1);
    }

    public function test_state0_advances_when_ready(): void
    {
        $this->resolveEagerPointers();
        $menuState = $this->menuStateBase();
        $this->initUint32($menuState + 0x18, 0);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), -1);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(0);
        $this->shouldCall('_AsqFreeQueues_8c011f7e');
        $this->shouldWriteLong($menuState + 0x18, 1);
    }

    public function test_state1_advances_after_fade_and_draws(): void
    {
        $this->resolveEagerPointers(txtDraw: false);
        $menuState = $this->menuStateBase();
        $this->initUint32($menuState + 0x18, 1);
        $this->initUint32($menuState + 0x38, 2);           // selected
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), -1);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->shouldWriteLong($menuState + 0x18, 2);
        $this->expectDrawTail($menuState, 2, 0);
    }

    public function test_state1_stays_while_fading(): void
    {
        $this->resolveEagerPointers(txtDraw: false);
        $menuState = $this->menuStateBase();
        $this->initUint32($menuState + 0x18, 1);
        $this->initUint32($menuState + 0x38, 0);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), -1);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectDrawTail($menuState, 0, 1);
    }

    public function test_state2_confirm_load_vm_not_connected(): void
    {
        $menuState = $this->menuStateBase();
        $this->state2Enter($menuState, 0, 0x4);        // selected=load, A pressed

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_PromptHandleMultiple_8c016c58')->with($menuState + 0x38, 4);
        $this->shouldWriteLong($menuState + 0x3c, 0);
        $this->shouldWriteLong($menuState + 0x1c, 0);
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("設定されたポートにＶＭが<E>接続されていません");
        $this->shouldCall('_sdMidiPlay')->with(0, 1, 2, 0);
        $this->expectDrawTail($menuState, 0, 0);
    }

    public function test_state2_confirm_load_ready(): void
    {
        $menuState = $this->menuStateBase();
        $this->state2Enter($menuState, 0, 0x4);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(5);
        $this->shouldCall('_PromptHandleMultiple_8c016c58')->with($menuState + 0x38, 4);
        $this->shouldWriteLong($menuState + 0x3c, 0);
        $this->shouldWriteLong($menuState + 0x1c, 0);
        $this->shouldWriteLong($menuState + 0x18, 3);
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("ファイルをロードします。<E>よろしいですか？");
        $this->shouldCall('_sdMidiPlay')->with(0, 1, 0, 0);
        $this->expectDrawTail($menuState, 0, 0);
    }

    public function test_state2_confirm_save_create(): void
    {
        $menuState = $this->menuStateBase();
        $this->state2Enter($menuState, 1, 0x4);        // selected=save

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(4);
        $this->shouldCall('_PromptHandleMultiple_8c016c58')->with($menuState + 0x38, 4);
        $this->shouldWriteLong($menuState + 0x3c, 0);
        $this->shouldWriteLong($menuState + 0x1c, 0);
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("ファイルを作成します。<E>よろしいですか？");
        $this->shouldWriteLong($menuState + 0x18, 5);
        $this->shouldCall('_sdMidiPlay')->with(0, 1, 0, 0);
        $this->expectDrawTail($menuState, 1, 0);
    }

    public function test_state2_confirm_save_vm_not_connected(): void
    {
        $menuState = $this->menuStateBase();
        $this->state2Enter($menuState, 1, 0x4);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_PromptHandleMultiple_8c016c58')->with($menuState + 0x38, 4);
        $this->shouldWriteLong($menuState + 0x3c, 0);
        $this->shouldWriteLong($menuState + 0x1c, 0);
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("ＶＭが接続されていません<E>セーブには３ブロック必要です");
        $this->shouldCall('_sdMidiPlay')->with(0, 1, 2, 0);
        $this->expectDrawTail($menuState, 1, 0);
    }

    public function test_state2_back_to_course_menu(): void
    {
        $this->resolveEagerPointers(txtDraw: false, swapBox: true, midi: false);
        $menuState = $this->menuStateBase();
        $this->state2Enter($menuState, 2, 0x4);        // selected=back

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_PromptHandleMultiple_8c016c58')->with($menuState + 0x38, 4);
        $this->shouldWriteLong($menuState + 0x3c, 0);
        $this->shouldWriteLong($menuState + 0x1c, 0);
        $this->shouldWriteLong($menuState + 0x18, 7);
        $this->shouldCall('_sdMidiPlay')->with(0, 1, 0, 0);
        $this->shouldCall('_VmSelectUnmountAll_8c0194de');
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
        $this->expectDrawTail($menuState, 2, 0);
    }

    public function test_state2_quit_confirm_story_mode(): void
    {
        $menuState = $this->menuStateBase();
        $this->state2Enter($menuState, 3, 0x4);        // selected=quit
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_PromptHandleMultiple_8c016c58')->with($menuState + 0x38, 4);
        $this->shouldWriteLong($menuState + 0x3c, 0);
        $this->shouldWriteLong($menuState + 0x1c, 0);
        $this->shouldWriteLong($menuState + 0x18, 6);
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("ストーリーモードを終了します。<E>よろしいですか？");
        $this->shouldCall('_sdMidiPlay')->with(0, 1, 0, 0);
        $this->expectDrawTail($menuState, 3, 0);
    }

    public function test_state2_quit_confirm_free_mode(): void
    {
        $menuState = $this->menuStateBase();
        $this->state2Enter($menuState, 3, 0x4);
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 1);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_PromptHandleMultiple_8c016c58')->with($menuState + 0x38, 4);
        $this->shouldWriteLong($menuState + 0x3c, 0);
        $this->shouldWriteLong($menuState + 0x1c, 0);
        $this->shouldWriteLong($menuState + 0x18, 6);
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("フリーランモードを終了します。<E>よろしいですか？");
        $this->shouldCall('_sdMidiPlay')->with(0, 1, 0, 0);
        $this->expectDrawTail($menuState, 3, 0);
    }

    public function test_state2_cancel(): void
    {
        $this->resolveEagerPointers(txtDraw: false, swapBox: true, midi: false);
        $menuState = $this->menuStateBase();
        $this->state2Enter($menuState, 0, 0x2);        // B pressed

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_PromptHandleMultiple_8c016c58')->with($menuState + 0x38, 4);
        $this->shouldWriteLong($menuState + 0x18, 7);
        $this->shouldCall('_sdMidiPlay')->with(0, 1, 1, 0);
        $this->shouldCall('_VmSelectUnmountAll_8c0194de');
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
        $this->expectDrawTail($menuState, 0, 0);
    }

    public function test_state2_cursor_move_plays_no_confirm(): void
    {
        $this->resolveEagerPointers(txtDraw: false, swapBox: false, midi: true);
        $menuState = $this->menuStateBase();
        $this->state2Enter($menuState, 0, 0x40);       // KL only: cursor moved

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_PromptHandleMultiple_8c016c58')->with($menuState + 0x38, 4);
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("");
        $this->expectDrawTail($menuState, 0, 0);
    }

    public function test_state2_idle(): void
    {
        $this->resolveEagerPointers(txtDraw: false);
        $menuState = $this->menuStateBase();
        $this->state2Enter($menuState, 0, 0);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_PromptHandleMultiple_8c016c58')->with($menuState + 0x38, 4);
        $this->expectDrawTail($menuState, 0, 0);
    }

    /* ---- State 6: quit-to-title confirm ---- */

    public function test_state6_confirm_quit(): void
    {
        $this->resolveEagerPointers(txtDraw: false, swapBox: true, midi: true);
        $menuState = $this->menuStateBase();
        $this->stateEnter($menuState, 6, 0, 0, 3);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($menuState + 0x3c)->andReturn(1);
        // state=8 is written in the JSR delay slot, before VmSelectUnmountVms runs.
        $this->shouldWriteLong($menuState + 0x18, 8);
        $this->shouldCall('_VmSelectUnmountAll_8c0194de');
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
        $this->expectPromptTail($menuState, 0, 3, 0);
    }

    public function test_state6_cancel(): void
    {
        $this->resolveEagerPointers(txtDraw: false, swapBox: false, midi: true);
        $menuState = $this->menuStateBase();
        $this->stateEnter($menuState, 6, 0, 1, 3);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($menuState + 0x3c)->andReturn(2);
        // state=2 is written in the JSR delay slot, before swapMessageBoxFor runs.
        $this->shouldWriteLong($menuState + 0x18, 2);
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("");
        $this->expectPromptTail($menuState, 1, 3, 0);
    }

    public function test_state6_idle(): void
    {
        $this->resolveEagerPointers(txtDraw: false, swapBox: true, midi: true);
        $menuState = $this->menuStateBase();
        $this->stateEnter($menuState, 6, 0, 2, 3);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($menuState + 0x3c)->andReturn(0);
        $this->expectPromptTail($menuState, 2, 3, 0);
    }

    /* ---- State 7: drive-exit, fade then hand to course menu ---- */

    public function test_state7_waits_for_fade(): void
    {
        $this->resolveEagerPointers(txtDraw: false, swapBox: true, midi: true);
        $menuState = $this->menuStateBase();
        $this->exitStateEnter($menuState, 7, 1, 1, 0);   // isFading=1

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectDrawTail($menuState, 1, 0);
    }

    public function test_state7_waits_for_gate(): void
    {
        $this->resolveEagerPointers(txtDraw: false, swapBox: true, midi: true);
        $menuState = $this->menuStateBase();
        $this->exitStateEnter($menuState, 7, 1, 0, 1);   // gate var_vmMountBusy_8c22606c=1

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectDrawTail($menuState, 1, 0);
    }

    public function test_state7_complete(): void
    {
        $this->resolveEagerPointers(txtDraw: true, swapBox: true, midi: true);
        $menuState = $this->menuStateBase();
        $this->exitStateEnter($menuState, 7, 1, 0, 0);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->shouldWriteLong($menuState + 0x3c, 1);
        $this->shouldWriteLong($menuState + 0x40, 0);
        $this->shouldCall('_FileSelectFreeBuffers_8c0187d0');
        $this->shouldCall('_CourseMenuEnter_8c017e18')->with(0x8ce00000);
    }

    /* ---- State 8: title-exit, fade then hand to title ---- */

    public function test_state8_waits_for_fade(): void
    {
        $this->resolveEagerPointers(txtDraw: false, swapBox: true, midi: true);
        $menuState = $this->menuStateBase();
        $this->exitStateEnter($menuState, 8, 1, 1, 0);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectDrawTail($menuState, 1, 0);
    }

    public function test_state8_complete(): void
    {
        $this->resolveEagerPointers(txtDraw: true, swapBox: true, midi: true);
        $menuState = $this->menuStateBase();
        $this->exitStateEnter($menuState, 8, 1, 0, 0);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->shouldCall('_ReplayMenuFreeSessionAssets_8c016182');
        $this->shouldCall('_TitleStart_8c015fd6')->with(1);
    }

    /* ---- State 3: LOAD confirm (subState_0x1c == 0) ---- */

    public function test_state3_confirm_load(): void
    {
        $this->resolveEagerPointers(txtDraw: false, swapBox: false, midi: true);
        $menuState = $this->menuStateBase();
        $this->stateEnter($menuState, 3, 0, 0, 0);
        $this->initUint32($this->addressOf('_var_saveBuf_8c1ba2e0'), 0x8c500000);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($menuState + 0x3c)->andReturn(1);
        $this->shouldCall('_BupLoad_8c014bc6')->with(0, 0x8c440000, 0x8c500000);
        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 1);
        $this->shouldWriteLong($menuState + 0x1c, 1);
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc');
        $this->shouldCall('_VmGameSetLcdSlot_8c01c8fc')->with(1);
        $this->expectPromptTail($menuState, 0, 0, 0);
    }

    public function test_state3_cancel(): void
    {
        $this->resolveEagerPointers(txtDraw: false, swapBox: false, midi: true);
        $menuState = $this->menuStateBase();
        $this->stateEnter($menuState, 3, 0, 1, 0);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($menuState + 0x3c)->andReturn(2);
        // state=2 is written in the JSR delay slot, before swapMessageBoxFor runs.
        $this->shouldWriteLong($menuState + 0x18, 2);
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("");
        $this->expectPromptTail($menuState, 1, 0, 0);
    }

    public function test_state3_save_no_longer_loadable(): void
    {
        // PromptHandleBinary returns 0, vmStatus not 5/6 -> error message
        $this->resolveEagerPointers(txtDraw: false, swapBox: false, midi: true);
        $menuState = $this->menuStateBase();
        $this->stateEnter($menuState, 3, 0, 2, 0);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(3);   // PROCEED_WITHOUT_SAVING: nothing to read
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($menuState + 0x3c)->andReturn(0);
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc');
        $this->shouldWriteLong($menuState + 0x18, 2);
        $this->expectPromptTail($menuState, 2, 0, 0);
    }

    public function test_state3_idle_ready(): void
    {
        // PromptHandleBinary returns 0, vmStatus == 5 -> nothing, just redraw
        $this->resolveEagerPointers(txtDraw: false, swapBox: true, midi: true);
        $menuState = $this->menuStateBase();
        $this->stateEnter($menuState, 3, 0, 2, 0);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(5);
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($menuState + 0x3c)->andReturn(0);
        $this->expectPromptTail($menuState, 2, 0, 0);
    }

    /* ---- State 3: LOAD in progress (subState_0x1c == 1) ---- */

    public function test_state3_load_busy(): void
    {
        $this->resolveEagerPointers(txtDraw: false, swapBox: true, midi: true);
        $menuState = $this->menuStateBase();
        $this->stateEnter($menuState, 3, 1, 0, 0);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_buStat')->with(0)->andReturn(1);   // still busy
        $this->expectPromptTail($menuState, 0, 0, 0);
    }

    public function test_state3_load_read_error(): void
    {
        $this->resolveEagerPointers(txtDraw: false, swapBox: false, midi: true);
        $menuState = $this->menuStateBase();
        $this->stateEnter($menuState, 3, 1, 0, 0);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_buStat')->with(0)->andReturn(0);
        $this->shouldCall('_buGetLastError')->with(0)->andReturn(1);   // error
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc');
        $this->shouldWriteLong($menuState + 0x18, 2);
        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 0);
        $this->shouldCall('_VmGameSetLcdSlot_8c01c8fc')->with(0);
        $this->expectPromptTail($menuState, 0, 0, 0);
    }

    public function test_state3_load_valid(): void
    {
        $this->resolveEagerPointers(txtDraw: false, swapBox: false, midi: true);
        $menuState = $this->menuStateBase();
        $this->stateEnter($menuState, 3, 1, 0, 0);
        $this->setSize('_var_backupFileHeader_8c1ba2e4', 0x60);
        $this->setSize('_var_progress_8c1ba1cc', 0xe8);
        $this->initUint32($this->addressOf('_var_saveBuf_8c1ba2e0'), 0x8c500000);
        $this->initUint32($this->addressOf('_var_backupFileHeader_8c1ba2e4') + 0x58, 0x8c510000);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_buStat')->with(0)->andReturn(0);
        $this->shouldCall('_buGetLastError')->with(0)->andReturn(0);
        $this->shouldCall('_buAnalyzeBackupFileImage');
        $this->shouldCall('_njMemCopy');
        $this->shouldCall('_FileSelectIsSaveValid_8c018804')->andReturn(1);
        $this->shouldCall('_SystemMenuApplyLoadedProgress_8c01b19c');
        $this->shouldCall('_FileSelectApplySoundSettings_8c0189fc');
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc');
        $this->shouldWriteLong($menuState + 0x18, 2);
        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 0);
        $this->shouldCall('_VmGameSetLcdSlot_8c01c8fc')->with(0);
        $this->expectPromptTail($menuState, 0, 0, 0);
    }

    public function test_state3_load_invalid(): void
    {
        $this->resolveEagerPointers(txtDraw: false, swapBox: false, midi: false);
        $menuState = $this->menuStateBase();
        $this->stateEnter($menuState, 3, 1, 0, 0);
        $this->setSize('_var_backupFileHeader_8c1ba2e4', 0x60);
        $this->setSize('_var_progress_8c1ba1cc', 0xe8);
        $this->initUint32($this->addressOf('_var_saveBuf_8c1ba2e0'), 0x8c500000);
        $this->initUint32($this->addressOf('_var_backupFileHeader_8c1ba2e4') + 0x58, 0x8c510000);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_buStat')->with(0)->andReturn(0);
        $this->shouldCall('_buGetLastError')->with(0)->andReturn(0);
        $this->shouldCall('_buAnalyzeBackupFileImage');
        $this->shouldCall('_njMemCopy');
        $this->shouldCall('_FileSelectIsSaveValid_8c018804')->andReturn(0);   // corrupt
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc');
        $this->shouldWriteLong($menuState + 0x18, 4);
        $this->shouldCall('_sdMidiPlay')->with(0, 1, 2, 0);
        $this->expectPromptTail($menuState, 0, 0, 0);
    }

    /* ---- State 5: SAVE confirm (subState_0x1c == 0), non-write branches ---- */

    public function test_state5_cancel(): void
    {
        $this->resolveEagerPointers(txtDraw: false, swapBox: false, midi: true);
        $menuState = $this->menuStateBase();
        $this->stateEnter($menuState, 5, 0, 1, 1);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($menuState + 0x3c)->andReturn(2);
        // state=2 is written in the JSR delay slot, before swapMessageBoxFor runs.
        $this->shouldWriteLong($menuState + 0x18, 2);
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("");
        // LAB_8c01b88c draws the highlight, then LAB_8c01b92c draws it again.
        $this->shouldCall('_SpriteDraw_8c014f54')->with($menuState, 3, 228.0, 266.0, -4.0);
        $this->expectPromptTail($menuState, 1, 1, 0);
    }

    public function test_state5_can_no_longer_save(): void
    {
        // returns 0, vmStatus not 4/5/6 -> error message, no second draw
        $this->resolveEagerPointers(txtDraw: false, swapBox: false, midi: true);
        $menuState = $this->menuStateBase();
        $this->stateEnter($menuState, 5, 0, 2, 1);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(3);
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($menuState + 0x3c)->andReturn(0);
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc');
        $this->shouldWriteLong($menuState + 0x18, 2);
        $this->expectPromptTail($menuState, 2, 1, 0);
    }

    public function test_state5_idle_ready(): void
    {
        // returns 0, vmStatus == 4 -> nothing, redraw
        $this->resolveEagerPointers(txtDraw: false, swapBox: true, midi: true);
        $menuState = $this->menuStateBase();
        $this->stateEnter($menuState, 5, 0, 2, 1);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(4);
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($menuState + 0x3c)->andReturn(0);
        $this->expectPromptTail($menuState, 2, 1, 0);
    }

    public function test_state5_confirm_save(): void
    {
        // SystemMenuWriteToVmu_8c01b26c is a same-unit sibling; mock it.
        $this->resolveEagerPointers(txtDraw: false, swapBox: false, midi: true);
        $menuState = $this->menuStateBase();
        $this->stateEnter($menuState, 5, 0, 0, 1);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($menuState + 0x3c)->andReturn(1);
        $this->shouldCall('_SystemMenuWriteToVmu_8c01b26c');
        // subState_0x1c=1 is written in the swap JSR delay slot, before swap runs.
        $this->shouldWriteLong($menuState + 0x1c, 1);
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc');
        $this->shouldCall('_VmGameSetLcdSlot_8c01c8fc')->with(1);
        // LAB_8c01b88c draws the highlight, then LAB_8c01b92c draws it again.
        $this->shouldCall('_SpriteDraw_8c014f54')->with($menuState, 2, 228.0, 266.0, -4.0);
        $this->expectPromptTail($menuState, 0, 1, 0);
    }

    /* ---- State 5: SAVE in progress (subState_0x1c == 1) ---- */

    public function test_state5_save_busy(): void
    {
        $this->resolveEagerPointers(txtDraw: false, swapBox: true, midi: true);
        $menuState = $this->menuStateBase();
        $this->stateEnter($menuState, 5, 1, 0, 1);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(0);
        $this->shouldCall('_buStat')->with(0)->andReturn(1);   // still busy
        $this->expectPromptTail($menuState, 0, 1, 0);
    }

    public function test_state5_save_done(): void
    {
        $this->resolveEagerPointers(txtDraw: false, swapBox: false, midi: true);
        $menuState = $this->menuStateBase();
        $this->stateEnter($menuState, 5, 1, 0, 1);
        $this->initUint32($this->addressOf('_var_backupFileImageBuf_8c1ba348'), 0x8c520000);

        $this->call('_menuTask_8c01b3ac')->with(0x8ce00000, 0);
        $this->expectVmStatusTop(0);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(0);
        $this->shouldCall('_buStat')->with(0)->andReturn(0);
        $this->shouldCall('_buGetLastError')->with(0)->andReturn(0);
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc');
        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 0);
        $this->shouldCall('_syFree')->with(0x8c520000);
        $this->shouldWriteLong($this->addressOf('_var_backupFileImageBuf_8c1ba348'), -1);
        $this->shouldWriteLong($menuState + 0x18, 2);
        $this->shouldCall('_VmGameSetLcdSlot_8c01c8fc')->with(0);
        $this->expectPromptTail($menuState, 0, 1, 0);
    }
};
