<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private const BASE = 0x100000;

    private int $ms;

    /* ---- state 0: waiting on the background loader (var_saveLoadResult_8c226010) ---- */

    /* Load still running (0): just refresh the "please wait" textbox + frame. */
    public function test_state0_loading(): void
    {
        $this->setup(0, 0);

        $this->call('_fileSelectTask_8c018e7e');

        $this->shouldCall('_MessageBoxDrawText_8c02af1c')->with(0xff);
        $this->expectFrame();
    }

    /* Load done (1), every image valid: build the card list and advance to READY (3). */
    public function test_state0_all_valid(): void
    {
        $this->setup(0, 1);
        $this->initUint32($this->addressOf('_var_loadedSaveCount_8c22600c'), 2);

        $this->call('_fileSelectTask_8c018e7e');

        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 0);
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->shouldWriteLong($this->addressOf('_var_saveBufCursor_8c225fe0'), self::BASE);
        $this->shouldCall('_FileSelectIsSaveValid_8c018804')->with(self::BASE)->andReturn(1);
        $this->shouldWriteLong($this->addressOf('_var_saveBufCursor_8c225fe0'), self::BASE + 0x600);
        $this->shouldCall('_FileSelectIsSaveValid_8c018804')->with(self::BASE + 0x600)->andReturn(1);
        $this->shouldWriteLong($this->addressOf('_var_saveBufCursor_8c225fe0'), self::BASE + 0xc00);
        $this->shouldCall('_buildFileList_8c018a22');
        $this->shouldWriteLong($this->ms + 0x18, 3);
        $this->expectFrame();
    }

    /* Load done (1) but an image fails validation: free, error box, LOAD_ERROR (1). */
    public function test_state0_invalid_save(): void
    {
        $this->setup(0, 1);
        $this->initUint32($this->addressOf('_var_loadedSaveCount_8c22600c'), 2);

        $this->call('_fileSelectTask_8c018e7e');

        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 0);
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->shouldWriteLong($this->addressOf('_var_saveBufCursor_8c225fe0'), self::BASE);
        $this->shouldCall('_FileSelectIsSaveValid_8c018804')->with(self::BASE)->andReturn(0);
        $this->shouldCall('_FileSelectFreeBuffers_8c0187d0');
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with('ロードに失敗しました');
        $this->shouldWriteLong($this->ms + 0x18, 1);
        $this->expectFrame();
    }

    /* Loader reported an error (2): free, error box, LOAD_ERROR (1), reset the VMU LCD. */
    public function test_state0_error(): void
    {
        $this->setup(0, 2);

        $this->call('_fileSelectTask_8c018e7e');

        $this->shouldWriteLong($this->addressOf('_var_vmBusy_8c157a7c'), 0);
        $this->shouldCall('_FileSelectFreeBuffers_8c0187d0');
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with('ロードに失敗しました');
        $this->shouldWriteLong($this->ms + 0x18, 1);
        $this->shouldCall('_VmGameSetLcdAnim_8c01c8fc')->with(0);
        $this->expectFrame();
    }

    /* ---- state 1: waiting for the player to acknowledge an error box ---- */

    /* A pressed: confirm blip, start the fade, advance to fading-out (2). */
    public function test_state1_confirm(): void
    {
        $this->setup(1, 0);
        $this->setPress(0x4);   // PDD_DGT_TA
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x777);

        $this->call('_fileSelectTask_8c018e7e');

        $this->shouldWriteLong($this->ms + 0x18, 2);
        $this->shouldCall('_sdMidiPlay')->with(0x777, 1, 0, 0);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
    }

    /* No input: keep the error box and frame on screen. */
    public function test_state1_idle(): void
    {
        $this->setup(1, 0);
        $this->setPress(0);

        $this->call('_fileSelectTask_8c018e7e');

        $this->shouldCall('_MessageBoxDrawText_8c02af1c')->with(0xff);
        $this->expectFrame();
    }

    /* ---- state 2: fading out toward VM SELECT ---- */

    /* Still fading: keep the box/frame up. */
    public function test_state2_fading(): void
    {
        $this->setup(2, 0);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_fileSelectTask_8c018e7e');

        $this->shouldCall('_MessageBoxDrawText_8c02af1c')->with(0xff);
        $this->expectFrame();
    }

    /* Fade complete: hand control back to the VM SELECT task. */
    public function test_state2_done(): void
    {
        $task = $this->alloc(0x20);
        $this->setup(2, 0);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);

        $this->call('_fileSelectTask_8c018e7e')->with($task);

        $this->shouldCall('_VmSelectEnter_8c019e44')->with($task);
    }

    private function setPress(int $bits): void
    {
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, $bits);
    }

    /* ---- state 3: card navigation / selection ---- */

    /* Left with the cursor off column 0: play the move blip and step it left. */
    public function test_state3_left_column(): void
    {
        $this->setup(3, 0);
        $this->cursor(2, 1);          // leftmost card 2, cursor column 1
        $this->setPress(0x40);        // PDD_DGT_KL
        $this->midi(0x777);

        $this->call('_fileSelectTask_8c018e7e');

        $this->shouldCall('_sdMidiPlay')->with(0x777, 1, 3, 0);
        $this->shouldWriteLong($this->ms + 0x38, 0);
        $this->expectDraw();
    }

    /* Left at column 0 with cards to the left: scroll one card back. */
    public function test_state3_left_scroll(): void
    {
        $this->setup(3, 0);
        $this->cursor(2, 0);
        $this->setPress(0x40);
        $this->midi(0x777);

        $this->call('_fileSelectTask_8c018e7e');

        $this->shouldCall('_sdMidiPlay')->with(0x777, 1, 3, 0);
        $this->shouldWriteLong($this->ms + 0x3c, 1);
        $this->expectDraw();
    }

    /* Left at the very first card: nothing to move, redraw only. */
    public function test_state3_left_edge(): void
    {
        $this->setup(3, 0);
        $this->cursor(0, 0);
        $this->setPress(0x40);

        $this->call('_fileSelectTask_8c018e7e');

        $this->expectDraw();
    }

    /* Right from the last column with a card at offset+3: scroll one card forward. */
    public function test_state3_right_scroll(): void
    {
        $this->setup(3, 0);
        $this->cursor(1, 2);
        $this->card(1 + 3, 5);        // var_fileCards_8c226018[offset+3] is a real save
        $this->setPress(0x80);        // PDD_DGT_KR
        $this->midi(0x777);

        $this->call('_fileSelectTask_8c018e7e');

        $this->shouldCall('_sdMidiPlay')->with(0x777, 1, 3, 0);
        $this->shouldWriteLong($this->ms + 0x3c, 2);
        $this->expectDraw();
    }

    /* Right onto an existing card within the three visible columns: step the cursor right. */
    public function test_state3_right_column(): void
    {
        $this->setup(3, 0);
        $this->cursor(0, 0);
        $this->card(0 + 0 + 1, 5);
        $this->setPress(0x80);
        $this->midi(0x777);

        $this->call('_fileSelectTask_8c018e7e');

        $this->shouldCall('_sdMidiPlay')->with(0x777, 1, 3, 0);
        $this->shouldWriteLong($this->ms + 0x38, 1);
        $this->expectDraw();
    }

    /* Right onto an empty slot (0xb): nothing to move, redraw only. */
    public function test_state3_right_edge(): void
    {
        $this->setup(3, 0);
        $this->cursor(0, 0);
        $this->card(1, 0xb);
        $this->setPress(0x80);

        $this->call('_fileSelectTask_8c018e7e');

        $this->expectDraw();
    }

    /* B: cancel back to VM SELECT -- blip, state 7, start fade, redraw. */
    public function test_state3_back(): void
    {
        $this->setup(3, 0);
        $this->cursor(0, 0);
        $this->setPress(0x2);         // PDD_DGT_TB
        $this->midi(0x777);

        $this->call('_fileSelectTask_8c018e7e');

        $this->shouldCall('_sdMidiPlay')->with(0x777, 1, 1, 0);
        $this->shouldWriteLong($this->ms + 0x18, 7);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
        $this->expectDraw();
    }

    /* A on the NEW FILE card: confirm with the "create a new file?" prompt. */
    public function test_state3_confirm_new(): void
    {
        $this->setup(3, 0);
        $this->cursor(0, 0);
        $this->card(0, 0xa);          // var_fileCards_8c226018[offset+selected] == NEW FILE
        $this->setPress(0x4);         // PDD_DGT_TA
        $this->midi(0x777);

        $this->call('_fileSelectTask_8c018e7e');

        $this->shouldCall('_sdMidiPlay')->with(0x777, 1, 0, 0);
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with('新たにファイルを作成します<E>よろしいですか？');
        $this->shouldWriteLong($this->ms + 0x40, 0);
        $this->shouldWriteLong($this->ms + 0x18, 4);
        $this->expectDraw();
    }

    /* A on a save card: confirm with the "this file?" prompt. */
    public function test_state3_confirm_file(): void
    {
        $this->setup(3, 0);
        $this->cursor(0, 0);
        $this->card(0, 5);            // a save, not NEW FILE
        $this->setPress(0x4);
        $this->midi(0x777);

        $this->call('_fileSelectTask_8c018e7e');

        $this->shouldCall('_sdMidiPlay')->with(0x777, 1, 0, 0);
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with('このファイルでよろしいですか？');
        $this->shouldWriteLong($this->ms + 0x40, 0);
        $this->shouldWriteLong($this->ms + 0x18, 4);
        $this->expectDraw();
    }

    /* No input: just redraw the screen. */
    public function test_state3_idle(): void
    {
        $this->setup(3, 0);
        $this->cursor(0, 0);
        $this->setPress(0);

        $this->call('_fileSelectTask_8c018e7e');

        $this->expectDraw();
    }

    /* ---- state 4: confirmation prompt ("new file?" / "this file?") ---- */

    /* A on the NEW FILE prompt: reset to a new game, pick the lowest free VMU file index,
     * fade out (5). */
    public function test_state4_confirm_new(): void
    {
        $this->setup(4, 0);
        $this->cursor(0, 0);
        $this->card(0, 0xa);
        $this->initUint32($this->addressOf('_var_fileCardCount_8c226014'), 1);   // only the NEW FILE card
        $this->setPress(0x4);   // PDD_DGT_TA
        $this->midi(0x777);

        $this->call('_fileSelectTask_8c018e7e');

        $this->shouldWriteLong($this->ms + 0x18, 5);
        $this->shouldCall('_FileSelectResetNewGame_8c01895e');
        $this->shouldWriteLong($this->addressOf('_var_saveSlot_8c1ba350'), 0);
        $this->expectConfirmTail();
    }

    /* A on a save prompt: locate its image, copy it into PlayerProgress, fade out (5). */
    public function test_state4_confirm_file(): void
    {
        $this->setup(4, 0);
        $this->cursor(0, 0);
        $this->card(0, 5);
        $this->initUint32($this->addressOf('_var_loadedSaveSlots_8c225fe4'), 5);   // VMU file 5 was loaded first
        $this->setPress(0x4);   // PDD_DGT_TA
        $this->midi(0x777);

        $this->call('_fileSelectTask_8c018e7e');

        $this->shouldWriteLong($this->ms + 0x18, 5);
        $this->shouldCall('_njMemCopy')
            ->with($this->addressOf('_var_progress_8c1ba1cc'), self::BASE, 0xe8);
        $this->shouldCall('_SystemMenuApplyLoadedProgress_8c01b19c');
        $this->shouldWriteLong($this->addressOf('_var_saveSlot_8c1ba350'), 5);
        $this->shouldCall('_FileSelectApplySoundSettings_8c0189fc');
        $this->expectConfirmTail();
    }

    /* B: cancel the prompt -- clear the box and return to navigation (3). */
    public function test_state4_cancel(): void
    {
        $this->setup(4, 0);
        $this->cursor(0, 0);
        $this->setPress(0x2);   // PDD_DGT_TB
        $this->midi(0x777);

        $this->call('_fileSelectTask_8c018e7e');

        $this->shouldWriteLong($this->ms + 0x18, 3);
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with('');
        $this->shouldCall('_sdMidiPlay')->with(0x777, 1, 1, 0);
        $this->expectDraw();
        $this->shouldCall('_MessageBoxDrawText_8c02af1c')->with(0xff);
    }

    /* No input: keep the prompt on screen. */
    public function test_state4_idle(): void
    {
        $this->setup(4, 0);
        $this->cursor(0, 0);
        $this->setPress(0);

        $this->call('_fileSelectTask_8c018e7e');

        $this->expectDraw();
        $this->shouldCall('_MessageBoxDrawText_8c02af1c')->with(0xff);
    }

    /* ---- state 5: waiting for the confirm fade, then unmounting ---- */

    /* Still fading: just keep redrawing. */
    public function test_state5_fading(): void
    {
        $this->setup(5, 0);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_fileSelectTask_8c018e7e');

        $this->expectDraw();
    }

    /* Fade done: free buffers, unmount the VMUs, advance to 6. */
    public function test_state5_unmount(): void
    {
        $this->setup(5, 0);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);

        $this->call('_fileSelectTask_8c018e7e');

        $this->shouldCall('_FileSelectFreeBuffers_8c0187d0');
        $this->shouldCall('_VmSelectUnmountAll_8c0194de');
        $this->shouldWriteLong($this->ms + 0x18, 6);
    }

    /* ---- state 6: waiting on the unmount before switching to the main menu ---- */

    /* Unmount pending (var_vmMountBusy_8c22606c set): do nothing this frame. */
    public function test_state6_pending(): void
    {
        $this->setup(6, 0);
        $this->initUint32($this->addressOf('_var_vmMountBusy_8c22606c'), 1);

        $this->call('_fileSelectTask_8c018e7e');
    }

    /* Unmounted, but an ADX stream is still playing out (init_adxPlaying_8c03bd80): keep waiting. */
    public function test_state6_adx_busy(): void
    {
        $this->setup(6, 0);
        $this->initUint32($this->addressOf('_var_vmMountBusy_8c22606c'), 0);
        $this->initUint32($this->addressOf('_init_adxPlaying_8c03bd80'), 1);

        $this->call('_fileSelectTask_8c018e7e');
    }

    /* Ready: invalidate the system resource group and hand off to the main menu. */
    public function test_state6_go(): void
    {
        $task = $this->alloc(0x20);
        $this->setup(6, 0);
        $this->initUint32($this->addressOf('_var_vmMountBusy_8c22606c'), 0);
        $this->initUint32($this->addressOf('_init_adxPlaying_8c03bd80'), 0);

        $this->call('_fileSelectTask_8c018e7e')->with($task);

        $this->shouldWriteLong($this->addressOf('_var_currentSysResGroupInfo_8c225fb0'), -1);
        $this->shouldCall('_MainMenuEnter_8c01a09a')->with($task);
    }

    /* ---- state 7: cancel fade-out, then back to VM SELECT ---- */

    /* Still fading: redraw. */
    public function test_state7_fading(): void
    {
        $this->setup(7, 0);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_fileSelectTask_8c018e7e');

        $this->expectDraw();
    }

    /* Fade done: free buffers and hand back to the VM SELECT task. */
    public function test_state7_done(): void
    {
        $task = $this->alloc(0x20);
        $this->setup(7, 0);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);

        $this->call('_fileSelectTask_8c018e7e')->with($task);

        $this->shouldCall('_FileSelectFreeBuffers_8c0187d0');
        $this->shouldCall('_VmSelectEnter_8c019e44')->with($task);
    }

    /* ---- state 8: waiting on unmount before returning to VM SELECT ---- */

    /* Unmount pending: do nothing. */
    public function test_state8_pending(): void
    {
        $this->setup(8, 0);
        $this->initUint32($this->addressOf('_var_vmMountBusy_8c22606c'), 1);

        $this->call('_fileSelectTask_8c018e7e');
    }

    /* Unmounted: switch back to the VM SELECT task. */
    public function test_state8_done(): void
    {
        $task = $this->alloc(0x20);
        $this->setup(8, 0);
        $this->initUint32($this->addressOf('_var_vmMountBusy_8c22606c'), 0);

        $this->call('_fileSelectTask_8c018e7e')->with($task);

        $this->shouldCall('_VmSelectEnter_8c019e44')->with($task);
    }

    /* Shared tail of both A-confirm paths: fade out audio+screen, then redraw. */
    private function expectConfirmTail(): void
    {
        $this->shouldCall('_sdMidiPlay')->with(0x777, 1, 0, 0);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(0);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(1);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
        $this->expectDraw();
        $this->shouldCall('_MessageBoxDrawText_8c02af1c')->with(0xff);
    }

    private function cursor(int $scroll, int $selected): void
    {
        $this->initUint32($this->ms + 0x3c, $scroll);
        $this->initUint32($this->ms + 0x38, $selected);
    }

    private function card(int $index, int $value): void
    {
        $this->initUint32($this->addressOf('_var_fileCards_8c226018') + $index * 4, $value);
    }

    private function midi(int $handle): void
    {
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), $handle);
    }

    private function expectDraw(): void
    {
        $this->shouldCall('_drawFileSelect_8c018d46');
    }

    private function setup(int $state, int $loadResult): void
    {
        /* external functions -- resolved unconditionally (the C literal pool may
         * load an address on a path that doesn't call it) */
        foreach ([
            '_SpriteDraw_8c014f54', '_MessageBoxDrawText_8c02af1c', '_VmGameSetLcdAnim_8c01c8fc',
            '_MessageBoxSetText_8c02aefc', '_sdMidiPlay', '_RenderStartFadeOut_8c022b60',
            '_njMemCopy', '_SndStartAdxFadeOut_8c010bae', '_SystemMenuApplyLoadedProgress_8c01b19c',
            '_VmSelectUnmountAll_8c0194de', '_VmSelectEnter_8c019e44',
            '_MainMenuEnter_8c01a09a',
        ] as $fn) {
            $this->setSize($fn, 4);
        }

        /* external data */
        $this->setSize('_var_peripherals_8c1ba35c', 0x68);
        $this->setSize('_var_menuState_8c1bc7a8', 0x6c);
        $this->setSize('_var_isFading_8c226568', 4);
        $this->setSize('_var_midiHandles_8c0fcd28', 0x20);
        $this->setSize('_var_vmBusy_8c157a7c', 4);
        $this->setSize('_var_saveBuf_8c1ba2e0', 4);
        $this->setSize('_var_saveSlot_8c1ba350', 4);
        $this->setSize('_var_vmMountBusy_8c22606c', 4);
        $this->setSize('_var_progress_8c1ba1cc', 0xe8);
        $this->setSize('_init_adxPlaying_8c03bd80', 4);
        $this->setSize('_var_currentSysResGroupInfo_8c225fb0', 4);

        $this->ms = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($this->ms + 0x18, $state);
        $this->initUint32($this->addressOf('_var_saveLoadResult_8c226010'), $loadResult);
        $this->initUint32($this->addressOf('_var_saveBuf_8c1ba2e0'), self::BASE);
    }

    /* The two frame sprites drawn on the way out of states 0/1/2. */
    private function expectFrame(): void
    {
        $this->shouldCall('_SpriteDraw_8c014f54')->with($this->ms, 1, 0.0, 0.0, -4.3);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($this->ms, 0, 0.0, 0.0, -5.0);
    }
};
