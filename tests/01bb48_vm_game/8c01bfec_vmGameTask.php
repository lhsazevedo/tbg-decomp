<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

// PDD_DGT_* button bits (var_peripherals_8c1ba35c[0].press).
if (!defined('PDD_KU')) {
    define('PDD_KU', 0x10);
    define('PDD_KD', 0x20);
    define('PDD_KL', 0x40);
    define('PDD_KR', 0x80);
    define('PDD_TA', 0x04);
    define('PDD_TB', 0x02);
}

// VM-GAME task state machine (menuState.state_0x18).
return new class extends TestCase {
    // var_midiHandles_8c0fcd28[0]'s value doesn't matter to any assertion here
    // -- sdMidiPlay just gets it verbatim -- so seed it to a fixed constant in
    // setup() rather than leaving it to the harness's per-test random fill.
    const MIDI_HANDLE_0 = 0x11223344;

    private int $ms;
    private int $task;

    private function setup(int $state): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_var_vmuStatus_8c226048', 0x24);
        $this->setSize('_var_peripherals_8c1ba35c', 0x34);
        $this->setSize('_var_resourceGroup_8c2263a8', 0x0c);

        // External functions whose addresses the prologue loads unconditionally
        // (plus every callee used by any state) must be resolvable.
        foreach ([
            '_VmSelectUpdateAllStatus_8c019550', '_RouteGetLatch_8c01432a',
            '_AsqFreeQueues_8c011f7e', '_RenderStartFadeIn_8c022a9c', '_RenderStartFadeOut_8c022b60',
            '_PromptHandleMultiple_8c016c58', '_PromptHandleBinary_8c016caa',
            '_CourseMenuInterpolateCursor_8c016d2c', '_CourseMenuFreeResourceGroup_8c0185c4',
            '_MainMenuEnter_8c01a09a', '_VmSelectUnmountAll_8c0194de',
            '_MessageBoxSetText_8c02aefc', '_MessageBoxDrawText_8c02af1c',
            '_sdMidiPlay', '_SpriteDraw_8c014f54', '_syFree',
            '__quick_evn_mvn',
        ] as $fn) {
            $this->setSize($fn, 4);
        }

        // External data whose addresses the prologue also front-loads.
        $this->setSize('_var_isFading_8c226568', 4);
        $this->setSize('_var_vmMountBusy_8c22606c', 4);
        $this->setSize('_var_vmBusy_8c157a7c', 4);
        $this->setSize('_var_vmGameBuf_8c1bc454', 4);
        $this->setSize('_var_midiHandles_8c0fcd28', 0x20);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), self::MIDI_HANDLE_0);
        $this->setSize('_var_texbuf_8c277ca0', 0x1000);
        $this->setSize('_var_progress_8c1ba1cc', 0xe8);

        $this->ms = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->task = $this->alloc(0x20);
        $this->initUint32($this->task + 8, 0); // task->field_0x08 (download sub-phase)
        $this->initUint32($this->ms + 0x18, $state);
        $this->initUint32($this->ms + 0x38, 0); // selected_0x38
    }

    protected function isAsmObject(): bool
    {
        return str_contains($this->objectFile, '/asm/');
    }

    // vmGameTask_8c01bfec's own `slot` stack local (seeded from
    // m->selected_0x38 before the switch). This is the only frame on the
    // simulator's stack (test entry is a single call()), so its address is
    // the deterministic offset from the simulator's fixed initial SP
    // (0xFFFFFC), verified against each object's own prologue -- and it
    // differs per object because the C compiler's register-save set/frame
    // size differs from the archived asm's.
    private function slotAddr(): int
    {
        return $this->isAsmObject() ? 0xFFFFB8 : 0xFFFFEC;
    }

    // Preamble runs on every entry; call it right after ->call().
    private function expectPreamble(): void
    {
        $this->shouldCall('_VmSelectUpdateAllStatus_8c019550')
            ->with($this->addressOf('_init_vmuProbeNames_8c044e48'), 0x2d)
            ->andReturn(0);
    }

    public function test_init_pvm_busy_returns()
    {
        $this->setup(0);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(1);
    }

    public function test_init_pvm_ready_advances()
    {
        $this->setup(0);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(0);
        $this->shouldWriteLong($this->ms + 0x18, 1); // state -> MENU_FADE_IN
        $this->shouldCall('_AsqFreeQueues_8c011f7e');
        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    // Common tail: cases that `break` run drawSelectScreen_8c01be90 then the epilogue draw.
    private function expectEpilogue(int $slot = 0): void
    {
        $this->shouldCall('_drawSelectScreen_8c01be90');
        $this->shouldCall('_MessageBoxDrawText_8c02af1c')->with(0xff);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($this->ms, 1, 0.0, 0.0, -4.0);
        $this->shouldWriteLong($this->ms + 0x38, $slot); // selected_0x38 = slot
    }

    public function test_select_cursor_move_interpolating()
    {
        $this->setup(6);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_CourseMenuInterpolateCursor_8c016d2c')->andReturn(0);
        $this->expectEpilogue(0);
    }

    public function test_select_cursor_move_done()
    {
        $this->setup(6);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_CourseMenuInterpolateCursor_8c016d2c')->andReturn(1);
        $this->shouldWriteLong($this->ms + 0x18, 5); // state -> SELECT
        $this->expectEpilogue(0);
    }

    // Cases 1/2/3 skip drawSelectScreen_8c01be90 (goto), landing straight on the epilogue draw.
    private function expectDrawEpilogue(int $slot = 0): void
    {
        $this->shouldCall('_MessageBoxDrawText_8c02af1c')->with(0xff);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($this->ms, 1, 0.0, 0.0, -4.0);
        $this->shouldWriteLong($this->ms + 0x38, $slot);
    }

    public function test_menu_fade_in_still_fading()
    {
        $this->setup(1);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1); // still fading

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $rgB = $this->ms + 0x0c;
        $this->shouldCall('_SpriteDraw_8c014f54')->with($rgB, 0x6c, 0.0, 0.0, -5.0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($this->ms, 0, 0.0, 0.0, -7.0);
        $this->expectDrawEpilogue(0);
    }

    public function test_menu_fade_in_done()
    {
        $this->setup(1);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0); // fade complete

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldWriteLong($this->ms + 0x18, 2); // state -> MENU
        $rgB = $this->ms + 0x0c;
        $this->shouldCall('_SpriteDraw_8c014f54')->with($rgB, 0x6c, 0.0, 0.0, -5.0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($this->ms, 0, 0.0, 0.0, -7.0);
        $this->expectDrawEpilogue(0);
    }

    private function press(int $bits): void
    {
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, $bits);
    }

    private function seedSlots(array $statuses): void
    {
        $base = $this->addressOf('_var_vmuStatus_8c226048');
        for ($i = 0; $i < 8; $i++) {
            $this->initUint32($base + $i * 4, $statuses[$i] ?? 0);
        }
    }

    private function midi(int $data): void
    {
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI_HANDLE_0, 1, $data, 0);
    }

    private function expectMenuDraw(int $index): void
    {
        $this->shouldCall('_SpriteDraw_8c014f54')->with($this->ms + 0x0c, $index, 0.0, 0.0, -5.0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($this->ms, 0, 0.0, 0.0, -7.0);
        $this->expectDrawEpilogue($index - 0x6d);
    }

    // ---- state 2: MENU ----

    public function test_menu_confirm_download()
    {
        $this->setup(2);
        $this->initUint32($this->ms + 0x38, 0); // option 0
        $this->press(PDD_TA);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_PromptHandleMultiple_8c016c58')->with($this->slotAddr(), 3)->andReturn(0);
        $this->shouldWriteLong($this->ms + 0x18, 3);   // MENU_FADE_OUT
        $this->shouldWriteLong($this->ms + 0x1c, 0);   // mode = download
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
        $this->midi(0);
        $this->expectMenuDraw(0x6d);
    }

    public function test_menu_confirm_exp_load()
    {
        $this->setup(2);
        $this->initUint32($this->ms + 0x38, 1); // option 1
        $this->press(PDD_TA);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_PromptHandleMultiple_8c016c58')->with($this->slotAddr(), 3)->andReturn(0);
        $this->shouldWriteLong($this->ms + 0x18, 3);
        $this->shouldWriteLong($this->ms + 0x1c, 1);   // mode = exp load
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
        $this->midi(0);
        $this->expectMenuDraw(0x6e);
    }

    public function test_menu_confirm_return()
    {
        $this->setup(2);
        $this->initUint32($this->ms + 0x38, 2); // option 2
        $this->press(PDD_TA);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_PromptHandleMultiple_8c016c58')->with($this->slotAddr(), 3)->andReturn(0);
        $this->shouldWriteLong($this->ms + 0x18, 0xd);  // EXIT
        $this->shouldCall('_VmSelectUnmountAll_8c0194de');
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
        $this->midi(0);
        $this->expectMenuDraw(0x6f);
    }

    public function test_menu_cancel()
    {
        $this->setup(2);
        $this->initUint32($this->ms + 0x38, 0);
        $this->press(PDD_TB);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_PromptHandleMultiple_8c016c58')->with($this->slotAddr(), 3)->andReturn(0);
        $this->shouldWriteLong($this->ms + 0x18, 0xd);  // EXIT
        $this->midi(1);
        $this->shouldCall('_VmSelectUnmountAll_8c0194de');
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
        $this->expectMenuDraw(0x6d);
    }

    // ---- state 3: MENU_FADE_OUT ----

    public function test_menu_fade_out_done()
    {
        $this->setup(3);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldWriteLong($this->ms + 0x18, 4);   // SELECT_ENTER
        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    public function test_menu_fade_out_still_fading()
    {
        $this->setup(3);
        $this->initUint32($this->ms + 0x38, 1);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->expectMenuDraw(0x6e);
    }

    // ---- state 4: SELECT_ENTER ----

    public function test_select_enter_incompatible()
    {
        $this->setup(4);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->seedSlots([0, 3]); // slot 1 mounted but incompatible

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldWriteLong($this->ms + 0x18, 9);   // SELECT_INCOMPATIBLE
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("ＶＭをセットして下さい");
        $this->expectEpilogue(1);
    }

    public function test_select_enter_ok()
    {
        $this->setup(4);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->seedSlots([0, 0, 1]); // slot 2 mounted, ok

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_selectSlot_8c01bf2a')->with(2);
        $this->shouldCall('__quick_evn_mvn');
        $this->shouldWriteLong($this->ms + 0x18, 5);   // SELECT
        $this->expectEpilogue(2);
    }

    // ---- state 9: SELECT_INCOMPATIBLE ----

    public function test_select_incompatible_confirm()
    {
        $this->setup(9);
        $this->seedSlots([0, 3]);
        $this->press(PDD_TA);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldWriteLong($this->ms + 0x18, 0xc);  // RETURN_FADE_OUT
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("");
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
        $this->expectEpilogue(1);
    }

    public function test_select_incompatible_recovered()
    {
        $this->setup(9);
        $this->seedSlots([0, 1]); // now compatible
        $this->press(0);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_selectSlot_8c01bf2a')->with(1);
        $this->shouldCall('__quick_evn_mvn');
        $this->shouldWriteLong($this->ms + 0x18, 5);   // SELECT
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("");
        $this->expectEpilogue(1);
    }

    // ---- state 0xb: OP_COMPLETE ----

    public function test_op_complete_confirm()
    {
        $this->setup(0xb);
        $this->press(PDD_TA);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldWriteLong($this->ms + 0x18, 0xc);  // RETURN_FADE_OUT
        $this->midi(0);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->expectEpilogue(0);
    }

    public function test_op_complete_idle()
    {
        $this->setup(0xb);
        $this->press(0);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->expectEpilogue(0);
    }

    // ---- state 0xc: RETURN_FADE_OUT ----

    public function test_return_fade_out_done()
    {
        $this->setup(0xc);
        $this->initUint32($this->ms + 0x1c, 1); // mode
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldWriteLong($this->ms + 0x18, 1);   // MENU_FADE_IN
        $this->shouldWriteLong($this->ms + 0x38, 1);   // selected = subState_0x1c
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("");
        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    public function test_return_fade_out_still_fading()
    {
        $this->setup(0xc);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->expectDrawEpilogue(0);
    }

    // ---- state 0xd: EXIT ----

    public function test_exit_done()
    {
        $this->setup(0xd);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_var_vmMountBusy_8c22606c'), 0);
        $this->initUint32($this->addressOf('_var_vmGameBuf_8c1bc454'), 0x8c400000);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_CourseMenuFreeResourceGroup_8c0185c4')->with($this->addressOf('_var_resourceGroup_8c2263a8'));
        $this->shouldCall('_syFree')->with(0x8c400000);
        $this->shouldWriteLong($this->addressOf('_var_vmGameBuf_8c1bc454'), 0xffffffff);
        $this->shouldCall('_MainMenuEnter_8c01a09a')->with($this->task);
    }

    public function test_exit_waiting()
    {
        $this->setup(0xd);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_var_vmMountBusy_8c22606c'), 1); // still busy -> return

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
    }

    public function test_exit_fading()
    {
        $this->setup(0xd);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->expectDrawEpilogue(0);
    }

    // ---- state 7: DOWNLOAD ----

    public function test_download_prompt_idle()
    {
        $this->setup(7);
        $this->initUint32($this->task + 8, 0);   // sub-phase confirm
        $this->initUint32($this->ms + 0x3c, 0);  // prompt cursor

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($this->ms + 0x3c)->andReturn(0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($this->ms, 2, 228.0, 300.0, -5.0);
        $this->expectEpilogue(0);
    }

    private function expectDownloadDraw(): void
    {
        $this->shouldCall('_SpriteDraw_8c014f54')->with($this->ms, 2, 228.0, 300.0, -5.0);
    }

    public function test_download_confirm_save_ok()
    {
        $this->setup(7);
        $this->initUint32($this->task + 8, 0);
        $this->initUint32($this->ms + 0x3c, 0);
        $this->initUint32($this->ms + 0x6c, 5); // selected drive
        $this->initUint32($this->addressOf('_var_vmGameBuf_8c1bc454'), 0x8c500000);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($this->ms + 0x3c)->andReturn(1);
        $this->shouldCall('_saveExecFile_8c01bd30')->with(0x8c500000, "TOKYOBUS._VM", 0x2d, 5)->andReturn(0);
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(1);
        $this->shouldWriteLong($this->task + 8, 1);   // sub-phase -> save-in-progress
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("ダウンロード中です<E>ＶＭを絶対に抜かないで下さい");
        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 1);
        $this->expectDownloadDraw();
        $this->expectEpilogue(0);
    }

    public function test_download_prompt_cancel()
    {
        $this->setup(7);
        $this->initUint32($this->task + 8, 0);
        $this->initUint32($this->ms + 0x3c, 0);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($this->ms + 0x3c)->andReturn(2);
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("");
        $this->shouldWriteLong($this->ms + 0x18, 5);   // back to SELECT
        $this->expectDownloadDraw();
        $this->expectEpilogue(0);
    }

    public function test_download_confirm_needs_defrag()
    {
        $this->setup(7);
        $this->initUint32($this->task + 8, 0);
        $this->initUint32($this->ms + 0x3c, 0);
        $this->initUint32($this->ms + 0x6c, 5);
        $this->initUint32($this->addressOf('_var_vmGameBuf_8c1bc454'), 0x8c500000);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($this->ms + 0x3c)->andReturn(1);
        $this->shouldCall('_saveExecFile_8c01bd30')->with(0x8c500000, "TOKYOBUS._VM", 0x2d, 5)->andReturn(0x10);
        $this->shouldCall('_defragDisk_8c01bde4')->with(5)->andReturn(0);
        $this->shouldWriteLong($this->task + 8, 2);   // sub-phase -> defrag-in-progress
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("ダウンロード中です<E>ＶＭを絶対に抜かないで下さい");
        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 1);
        $this->expectDownloadDraw();
        $this->expectEpilogue(0);
    }

    public function test_download_confirm_save_error()
    {
        $this->setup(7);
        $this->initUint32($this->task + 8, 0);
        $this->initUint32($this->ms + 0x3c, 0);
        $this->initUint32($this->ms + 0x6c, 5);
        $this->initUint32($this->addressOf('_var_vmGameBuf_8c1bc454'), 0x8c500000);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($this->ms + 0x3c)->andReturn(1);
        $this->shouldCall('_saveExecFile_8c01bd30')->with(0x8c500000, "TOKYOBUS._VM", 0x2d, 5)->andReturn(0xc);
        $this->shouldWriteLong($this->ms + 0x18, 5);   // back to SELECT
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("ダウンロードに失敗しました");
        $this->midi(2);
        $this->expectDownloadDraw();
        $this->expectEpilogue(0);
    }

    public function test_download_confirm_needs_defrag_defrag_fails()
    {
        $this->setup(7);
        $this->initUint32($this->task + 8, 0);
        $this->initUint32($this->ms + 0x3c, 0);
        $this->initUint32($this->ms + 0x6c, 5);
        $this->initUint32($this->addressOf('_var_vmGameBuf_8c1bc454'), 0x8c500000);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($this->ms + 0x3c)->andReturn(1);
        $this->shouldCall('_saveExecFile_8c01bd30')->with(0x8c500000, "TOKYOBUS._VM", 0x2d, 5)->andReturn(0x10);
        $this->shouldCall('_defragDisk_8c01bde4')->with(5)->andReturn(1); // defrag failed
        $this->shouldWriteLong($this->ms + 0x18, 5);   // back to SELECT
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("ダウンロードに失敗しました");
        $this->midi(2);
        $this->expectDownloadDraw();
        $this->expectEpilogue(0);
    }

    public function test_download_confirm_save_full()
    {
        $this->setup(7);
        $this->initUint32($this->task + 8, 0);
        $this->initUint32($this->ms + 0x3c, 0);
        $this->initUint32($this->ms + 0x6c, 5);
        $this->initUint32($this->addressOf('_var_vmGameBuf_8c1bc454'), 0x8c500000);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($this->ms + 0x3c)->andReturn(1);
        $this->shouldCall('_saveExecFile_8c01bd30')->with(0x8c500000, "TOKYOBUS._VM", 0x2d, 5)->andReturn(0x11); // SAVE_ERR_FULL
        $this->shouldWriteLong($this->ms + 0x18, 5);   // back to SELECT
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("空き容量が足りません<E>ダウンロードには４５ブロック必要です");
        $this->midi(2);
        $this->expectDownloadDraw();
        $this->expectEpilogue(0);
    }

    public function test_download_confirm_save_exists()
    {
        $this->setup(7);
        $this->initUint32($this->task + 8, 0);
        $this->initUint32($this->ms + 0x3c, 0);
        $this->initUint32($this->ms + 0x6c, 5);
        $this->initUint32($this->addressOf('_var_vmGameBuf_8c1bc454'), 0x8c500000);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($this->ms + 0x3c)->andReturn(1);
        $this->shouldCall('_saveExecFile_8c01bd30')->with(0x8c500000, "TOKYOBUS._VM", 0x2d, 5)->andReturn(0xb); // SAVE_ERR_EXISTS
        $this->shouldWriteLong($this->ms + 0x18, 5);   // back to SELECT
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("既に実行ファイルが存在します");
        $this->midi(2);
        $this->expectDownloadDraw();
        $this->expectEpilogue(0);
    }

    public function test_download_poll_complete()
    {
        $this->setup(7);
        $this->initUint32($this->task + 8, 1); // save-in-progress
        $this->initUint32($this->ms + 0x6c, 5);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_pollBupOp_8c01bc44')->with($this->addressOf('_var_lcdAnimDanger_8c2260b8'), 5)->andReturn(0x13);
        $this->shouldWriteLong($this->ms + 0x18, 0xb);  // OP_COMPLETE
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("ダウンロードが完了しました");
        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 0);
        $this->expectEpilogue(0);
    }

    public function test_download_poll_error()
    {
        $this->setup(7);
        $this->initUint32($this->task + 8, 1); // save-in-progress
        $this->initUint32($this->ms + 0x6c, 5);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_pollBupOp_8c01bc44')->with($this->addressOf('_var_lcdAnimDanger_8c2260b8'), 5)->andReturn(0xffffffff);
        $this->shouldWriteLong($this->ms + 0x18, 5);   // back to SELECT
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("ダウンロードに失敗しました");
        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 0);
        $this->expectEpilogue(0);
    }

    public function test_download_defrag_poll_error()
    {
        $this->setup(7);
        $this->initUint32($this->task + 8, 2); // defrag-in-progress
        $this->initUint32($this->ms + 0x6c, 5);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_pollBupOp_8c01bc44')->with($this->addressOf('_var_lcdAnimDanger_8c2260b8'), 5)->andReturn(0xffffffff);
        $this->shouldWriteLong($this->ms + 0x18, 5);   // back to SELECT
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("ダウンロードに失敗しました");
        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 0);
        $this->expectEpilogue(0);
    }

    public function test_download_defrag_poll_reissues_save()
    {
        $this->setup(7);
        $this->initUint32($this->task + 8, 2); // defrag-in-progress
        $this->initUint32($this->ms + 0x6c, 5);
        $this->initUint32($this->addressOf('_var_vmGameBuf_8c1bc454'), 0x8c500000);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_pollBupOp_8c01bc44')->with($this->addressOf('_var_lcdAnimDanger_8c2260b8'), 5)->andReturn(0);
        $this->shouldCall('_saveExecFile_8c01bd30')->with(0x8c500000, "TOKYOBUS._VM", 0x2d, 5)->andReturn(0);
        $this->shouldWriteLong($this->task + 8, 1);   // back to save-in-progress
        $this->expectEpilogue(0);
    }

    // ---- state 8: EXP_LOAD ----

    public function test_exp_load_confirm_load_ok()
    {
        $this->setup(8);
        $this->initUint32($this->task + 8, 0);
        $this->initUint32($this->ms + 0x3c, 0);
        $this->initUint32($this->ms + 0x6c, 5);
        $tex = $this->addressOf('_var_texbuf_8c277ca0');

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($this->ms + 0x3c)->andReturn(1);
        $this->shouldCall('_loadFileEx_8c01be30')->with(5, "TOKYOBUS._VM", $tex, 0, 6)->andReturn(0);
        $this->shouldWriteLong($this->task + 8, 1);
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("ＶＭを絶対に抜かないで下さい");
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(1);
        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 1);
        $this->expectDownloadDraw();
        $this->expectEpilogue(0);
    }

    public function test_exp_load_confirm_load_error()
    {
        $this->setup(8);
        $this->initUint32($this->task + 8, 0);
        $this->initUint32($this->ms + 0x3c, 0);
        $this->initUint32($this->ms + 0x6c, 5);
        $tex = $this->addressOf('_var_texbuf_8c277ca0');

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($this->ms + 0x3c)->andReturn(1);
        $this->shouldCall('_loadFileEx_8c01be30')->with(5, "TOKYOBUS._VM", $tex, 0, 6)->andReturn(0xffffffff);
        $this->shouldWriteLong($this->ms + 0x18, 5);   // back to SELECT
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("ダウンロードに失敗しました");
        $this->expectDownloadDraw();
        $this->expectEpilogue(0);
    }

    public function test_exp_load_prompt_cancel()
    {
        $this->setup(8);
        $this->initUint32($this->task + 8, 0);
        $this->initUint32($this->ms + 0x3c, 0);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($this->ms + 0x3c)->andReturn(2);
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("");
        $this->shouldWriteLong($this->ms + 0x18, 5);   // back to SELECT
        $this->expectDownloadDraw();
        $this->expectEpilogue(0);
    }

    public function test_exp_load_poll_error()
    {
        $this->setup(8);
        $this->initUint32($this->task + 8, 1); // load-in-progress
        $this->initUint32($this->ms + 0x6c, 5);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_pollBupOp_8c01bc44')->with($this->addressOf('_var_lcdAnimDanger_8c2260b8'), 5)->andReturn(0xffffffff);
        $this->shouldWriteLong($this->ms + 0x18, 5);   // back to SELECT
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("データの読み込みに失敗しました");
        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 0);
        $this->expectEpilogue(0);
    }

    public function test_exp_load_poll_writes_exp_and_rewrites()
    {
        $this->setup(8);
        $this->initUint32($this->task + 8, 1); // load-in-progress
        $this->initUint32($this->ms + 0x6c, 5);
        $tex = $this->addressOf('_var_texbuf_8c277ca0');
        $exp = $this->addressOf('_var_progress_8c1ba1cc') + 0x90; // exp_0x90
        $this->initUint32($exp, 1000);
        $this->initUint8($tex + 0x900, 1);    // points high
        $this->initUint8($tex + 0x901, 20);   // points low
        $this->initUint8($tex + 0x902, 0x0a); // checksum inputs
        $this->initUint8($tex + 0x903, 0x0c);
        $this->initUint8($tex + 0x904, 0x03);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_pollBupOp_8c01bc44')->with($this->addressOf('_var_lcdAnimDanger_8c2260b8'), 5)->andReturn(0x17);
        $this->shouldWriteLong($exp, 1120); // 1000 + 1*100 + 20
        $this->shouldWriteByte($tex + 0x900, 0);
        $this->shouldWriteByte($tex + 0x901, 0);
        $this->shouldWriteByte($tex + 0x905, 0x05); // 0x0a ^ 0x0c ^ 0x03
        $this->shouldCall('_rewriteExecFile_8c01be60')->with(5, "TOKYOBUS._VM", $tex, 0, 6)->andReturn(0);
        $this->shouldWriteLong($this->task + 8, 2); // rewrite-in-progress
        $this->expectEpilogue(0);
    }

    public function test_exp_load_poll_writes_exp_clamped()
    {
        $this->setup(8);
        $this->initUint32($this->task + 8, 1); // load-in-progress
        $this->initUint32($this->ms + 0x6c, 5);
        $tex = $this->addressOf('_var_texbuf_8c277ca0');
        $exp = $this->addressOf('_var_progress_8c1ba1cc') + 0x90; // exp_0x90
        $this->initUint32($exp, 99990);
        $this->initUint8($tex + 0x900, 1);    // +100
        $this->initUint8($tex + 0x901, 50);   // +50 -> 100140, clamps to 99999
        $this->initUint8($tex + 0x902, 0x0a);
        $this->initUint8($tex + 0x903, 0x0c);
        $this->initUint8($tex + 0x904, 0x03);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_pollBupOp_8c01bc44')->with($this->addressOf('_var_lcdAnimDanger_8c2260b8'), 5)->andReturn(0);
        $this->shouldWriteLong($exp, 100140); // raw sum, written before the clamp check
        $this->shouldWriteLong($exp, 99999);  // then clamped
        $this->shouldWriteByte($tex + 0x900, 0);
        $this->shouldWriteByte($tex + 0x901, 0);
        $this->shouldWriteByte($tex + 0x905, 0x05);
        $this->shouldCall('_rewriteExecFile_8c01be60')->with(5, "TOKYOBUS._VM", $tex, 0, 6)->andReturn(0);
        $this->shouldWriteLong($this->task + 8, 2); // rewrite-in-progress
        $this->expectEpilogue(0);
    }

    public function test_exp_load_poll_rewrite_error()
    {
        $this->setup(8);
        $this->initUint32($this->task + 8, 1); // load-in-progress
        $this->initUint32($this->ms + 0x6c, 5);
        $tex = $this->addressOf('_var_texbuf_8c277ca0');
        $exp = $this->addressOf('_var_progress_8c1ba1cc') + 0x90; // exp_0x90
        $this->initUint32($exp, 1000);
        $this->initUint8($tex + 0x900, 1);
        $this->initUint8($tex + 0x901, 20);
        $this->initUint8($tex + 0x902, 0x0a);
        $this->initUint8($tex + 0x903, 0x0c);
        $this->initUint8($tex + 0x904, 0x03);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_pollBupOp_8c01bc44')->with($this->addressOf('_var_lcdAnimDanger_8c2260b8'), 5)->andReturn(0x17);
        $this->shouldWriteLong($exp, 1120);
        $this->shouldWriteByte($tex + 0x900, 0);
        $this->shouldWriteByte($tex + 0x901, 0);
        $this->shouldWriteByte($tex + 0x905, 0x05);
        $this->shouldCall('_rewriteExecFile_8c01be60')->with(5, "TOKYOBUS._VM", $tex, 0, 6)->andReturn(0xffffffff);
        $this->shouldWriteLong($this->ms + 0x18, 5);   // back to SELECT
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("データの書き込みに失敗しました");
        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 0);
        $this->expectEpilogue(0);
    }

    public function test_exp_load_rewrite_poll_complete()
    {
        $this->setup(8);
        $this->initUint32($this->task + 8, 2); // rewrite-in-progress
        $this->initUint32($this->ms + 0x6c, 5);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_pollBupOp_8c01bc44')->with($this->addressOf('_var_lcdAnimDanger_8c2260b8'), 5)->andReturn(0x13);
        $this->shouldWriteLong($this->ms + 0x18, 0xb);  // OP_COMPLETE
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("ポイントの加算が完了しました");
        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 0);
        $this->expectEpilogue(0);
    }

    public function test_exp_load_rewrite_poll_error()
    {
        $this->setup(8);
        $this->initUint32($this->task + 8, 2); // rewrite-in-progress
        $this->initUint32($this->ms + 0x6c, 5);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_pollBupOp_8c01bc44')->with($this->addressOf('_var_lcdAnimDanger_8c2260b8'), 5)->andReturn(0xffffffff);
        $this->shouldWriteLong($this->ms + 0x18, 5);   // back to SELECT
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("データの書き込みに失敗しました");
        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 0);
        $this->expectEpilogue(0);
    }

    // ---- state 5: SELECT (grid navigation) ----

    public function test_select_incompatible()
    {
        $this->setup(5);
        $this->seedSlots([3]); // first mounted is incompatible

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->shouldWriteLong($this->ms + 0x18, 9);   // SELECT_INCOMPATIBLE
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("ＶＭをセットして下さい");
        $this->expectEpilogue(0);
    }

    public function test_select_reselects_when_current_slot_disconnects()
    {
        $this->setup(5);
        $this->initUint32($this->ms + 0x38, 3); // was on slot 3, now unplugged
        $this->seedSlots([0, 1]);               // slot 0 also unplugged; rescan lands on slot 1
        $this->press(0);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->shouldCall('_selectSlot_8c01bf2a')->with(1);
        $this->shouldWriteLong($this->ms + 0x18, 6);   // SELECT_CURSOR_MOVE
        $this->midi(3);
        $this->expectEpilogue(1);
    }

    public function test_select_move_right()
    {
        $this->setup(5);
        $this->initUint32($this->ms + 0x38, 0); // selected slot 0
        $this->seedSlots([1, 1]);               // slots 0 and 1 mounted
        $this->press(PDD_KR);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->shouldCall('_selectSlot_8c01bf2a')->with(1);
        $this->shouldWriteLong($this->ms + 0x18, 6);   // SELECT_CURSOR_MOVE
        $this->midi(3);
        $this->expectEpilogue(1);
    }

    public function test_select_confirm_download()
    {
        $this->setup(5);
        $this->initUint32($this->ms + 0x38, 0);
        $this->initUint32($this->ms + 0x1c, 0);  // mode = download
        $this->seedSlots([1]);
        $this->press(PDD_TA);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->shouldWriteLong($this->ms + 0x18, 7);   // DOWNLOAD
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("交通標識クイズをダウンロードします<E>よろしいですか？");
        $this->midi(0);
        $this->shouldWriteLong($this->ms + 0x6c, 0);   // selectedVmuSlot = selected
        $this->shouldWriteLong($this->ms + 0x3c, 0);   // prompt cursor reset
        $this->shouldWriteLong($this->task + 8, 0);    // sub-phase reset
        $this->expectEpilogue(0);
    }

    public function test_select_confirm_exp_load()
    {
        $this->setup(5);
        $this->initUint32($this->ms + 0x38, 0);
        $this->initUint32($this->ms + 0x1c, 1);  // mode = exp load
        $this->seedSlots([5]);                   // status 5 = has save
        $this->press(PDD_TA);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->shouldWriteLong($this->ms + 0x18, 8);   // EXP_LOAD
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("ポイントを加算します<E>よろしいですか？");
        $this->midi(0);
        $this->shouldWriteLong($this->ms + 0x6c, 0);
        $this->shouldWriteLong($this->ms + 0x3c, 0);
        $this->shouldWriteLong($this->task + 8, 0);
        $this->expectEpilogue(0);
    }

    public function test_select_confirm_exp_load_wrong_status()
    {
        $this->setup(5);
        $this->initUint32($this->ms + 0x38, 0);
        $this->initUint32($this->ms + 0x1c, 1);  // mode = exp load
        $this->seedSlots([1]);                   // status 1 = no save -> reject sfx
        $this->press(PDD_TA);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->midi(2);                          // reject sound, no state change
        $this->shouldWriteLong($this->ms + 0x6c, 0);
        $this->shouldWriteLong($this->ms + 0x3c, 0);
        $this->shouldWriteLong($this->task + 8, 0);
        $this->expectEpilogue(0);
    }

    public function test_select_move_left_no_wrap()
    {
        $this->setup(5);
        $this->initUint32($this->ms + 0x38, 2); // selected slot 2
        $this->seedSlots([1, 1, 1, 1]);         // whole top row mounted
        $this->press(PDD_KL);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->shouldCall('_selectSlot_8c01bf2a')->with(1);
        $this->shouldWriteLong($this->ms + 0x18, 6);   // SELECT_CURSOR_MOVE
        $this->midi(3);
        $this->expectEpilogue(1);
    }

    public function test_select_move_left_wrap_top_row()
    {
        $this->setup(5);
        $this->initUint32($this->ms + 0x38, 0); // selected slot 0
        $this->seedSlots([1, 0, 0, 1]);         // slots 0 and 3 mounted
        $this->press(PDD_KL);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->shouldCall('_selectSlot_8c01bf2a')->with(3);
        $this->shouldWriteLong($this->ms + 0x18, 6);
        $this->midi(3);
        $this->expectEpilogue(3);
    }

    public function test_select_move_right_wrap_top_row()
    {
        $this->setup(5);
        $this->initUint32($this->ms + 0x38, 3); // selected slot 3
        $this->seedSlots([1, 0, 0, 1]);         // slots 0 and 3 mounted
        $this->press(PDD_KR);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->shouldCall('_selectSlot_8c01bf2a')->with(0);
        $this->shouldWriteLong($this->ms + 0x18, 6);
        $this->midi(3);
        $this->expectEpilogue(0);
    }

    public function test_select_move_left_wrap_bottom_row()
    {
        $this->setup(5);
        $this->initUint32($this->ms + 0x38, 4); // selected slot 4
        $this->seedSlots([0, 0, 0, 0, 1, 0, 0, 1]); // slots 4 and 7 mounted
        $this->press(PDD_KL);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->shouldCall('_selectSlot_8c01bf2a')->with(7);
        $this->shouldWriteLong($this->ms + 0x18, 6);
        $this->midi(3);
        $this->expectEpilogue(7);
    }

    public function test_select_move_right_wrap_bottom_row()
    {
        $this->setup(5);
        $this->initUint32($this->ms + 0x38, 7); // selected slot 7
        $this->seedSlots([0, 0, 0, 0, 1, 0, 0, 1]); // slots 4 and 7 mounted
        $this->press(PDD_KR);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->shouldCall('_selectSlot_8c01bf2a')->with(4);
        $this->shouldWriteLong($this->ms + 0x18, 6);
        $this->midi(3);
        $this->expectEpilogue(4);
    }

    public function test_select_move_down_direct()
    {
        $this->setup(5);
        $this->initUint32($this->ms + 0x38, 0);     // selected slot 0 (top)
        $this->seedSlots([1, 0, 0, 0, 1]);           // slot 0 and slot 4 mounted
        $this->press(PDD_KD);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->shouldCall('_selectSlot_8c01bf2a')->with(4);
        $this->shouldWriteLong($this->ms + 0x18, 6);
        $this->midi(3);
        $this->expectEpilogue(4);
    }

    public function test_select_move_down_scans_row()
    {
        $this->setup(5);
        $this->initUint32($this->ms + 0x38, 1);      // selected slot 1 (top)
        $this->seedSlots([1, 1, 0, 0, 0, 0, 0, 1]);  // bottom row: only slot 7 mounted
        $this->press(PDD_KD);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->shouldCall('_selectSlot_8c01bf2a')->with(7);
        $this->shouldWriteLong($this->ms + 0x18, 6);
        $this->midi(3);
        $this->expectEpilogue(7);
    }

    public function test_select_move_down_no_connected_slot_below()
    {
        $this->setup(5);
        $this->initUint32($this->ms + 0x38, 0); // selected slot 0 (top)
        $this->seedSlots([1]);                  // bottom row fully unmounted
        $this->press(PDD_KD);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        // slot == selected (unchanged) -> no press-TA/TB -> just redraws.
        $this->expectEpilogue(0);
    }

    public function test_select_move_up_direct()
    {
        $this->setup(5);
        $this->initUint32($this->ms + 0x38, 5);      // selected slot 5 (bottom)
        $this->seedSlots([0, 1, 0, 0, 0, 1]);        // slot 1 and slot 5 mounted
        $this->press(PDD_KU);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->shouldCall('_selectSlot_8c01bf2a')->with(1);
        $this->shouldWriteLong($this->ms + 0x18, 6);
        $this->midi(3);
        $this->expectEpilogue(1);
    }

    public function test_select_move_up_scans_row()
    {
        $this->setup(5);
        $this->initUint32($this->ms + 0x38, 6);      // selected slot 6 (bottom)
        $this->seedSlots([1, 0, 0, 0, 0, 0, 1]);     // top row: only slot 0 mounted
        $this->press(PDD_KU);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->shouldCall('_selectSlot_8c01bf2a')->with(0);
        $this->shouldWriteLong($this->ms + 0x18, 6);
        $this->midi(3);
        $this->expectEpilogue(0);
    }

    public function test_select_move_up_no_connected_slot_above()
    {
        $this->setup(5);
        $this->initUint32($this->ms + 0x38, 4); // selected slot 4 (bottom)
        $this->seedSlots([0, 0, 0, 0, 1]);      // top row fully unmounted
        $this->press(PDD_KU);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->expectEpilogue(4);
    }

    public function test_select_cancel()
    {
        $this->setup(5);
        $this->initUint32($this->ms + 0x38, 0);
        $this->seedSlots([1]);
        $this->press(PDD_TB);

        $this->call('_vmGameTask_8c01bfec')->with($this->task);
        $this->expectPreamble();
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->shouldWriteLong($this->ms + 0x18, 0xc);  // RETURN_FADE_OUT
        $this->midi(1);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
        $this->expectEpilogue(0);
    }
};
