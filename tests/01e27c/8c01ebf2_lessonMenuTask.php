<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_state_0_waits_for_pvm_ready(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 0);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(1);
    }

    public function test_state_0_advances_when_pvm_ready(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 0);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(0);
        $this->shouldCall('_AsqFreeQueues_8c011f7e');

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x18, 1);

        $this->shouldCall('_SndStopBgm_8c010d8a');
        $this->shouldCall('_SndPlayAdx_8c010cd6')->with(0, 0xd);
        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    public function test_state_1_idle_while_fading(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 1);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $this->expectDrawingTail(1);
    }

    public function test_state_1_advances_when_fade_complete(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 1);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_var_lessonDialogQueue_8c226414'), 0x16);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $this->shouldCall('_CourseMenuInstallDialog_8c0170c6')->with(0x16, 0);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x18, 2);

        $this->expectDrawingTail(2);
    }

    public function test_state_2_idle_while_dialog_active(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 2);
        $this->initUint32($this->addressOf('_var_instructorDialogActive_8c225fb4'), 1);

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 3);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $this->expectDrawingTail(2);
    }

    public function test_state_2_advances_to_next_dialog(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 2);
        $this->initUint32($this->addressOf('_var_instructorDialogActive_8c225fb4'), 0);
        $this->initUint32($this->addressOf('_var_lessonDialogQueue_8c226414') + 1 * 4, 0x17);

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 0);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $this->shouldWriteLong($task + 0x08, 1);
        $this->shouldCall('_CourseMenuInstallDialog_8c0170c6')->with(0x17, 0);

        $this->expectDrawingTail(2);
    }

    public function test_state_2_queue_exhausted_advances_to_state_3(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 2);
        $this->initUint32($this->addressOf('_var_instructorDialogActive_8c225fb4'), 0);
        $this->initUint32($this->addressOf('_var_lessonDialogQueue_8c226414') + 5 * 4, 0xffffffff); // -1 terminator

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 4);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $this->shouldWriteLong($task + 0x08, 5);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x18, 3);
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("");

        $this->expectDrawingTail(3);
    }

    public function test_state_3_idle_no_input(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 3);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $this->shouldCall('_scrollTowardSelection_8c01ebc8');

        $this->expectDrawingTail(3);
    }

    public function test_state_3_cancel_starts_fadeout(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 3);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 4);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x18, 6);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0, 0);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);

        $this->shouldCall('_scrollTowardSelection_8c01ebc8');

        $this->expectDrawingTail(6);
    }

    public function test_state_3_confirm_shows_quit_prompt(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 3);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 2);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x1c, 3);
        $this->shouldWriteLong($menuStateBase + 0x18, 5);
        $this->shouldWriteLong($menuStateBase + 0x3c, 0);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 1, 0);
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("\x97\xfb\x8f\x4b\x83\x82\x81\x5b\x83\x68\x82\xf0\x8f\x49\x97\xb9\x82\xb5\x82\xdc\x82\xb7\x3c\x45\x3e\x82\xe6\x82\xeb\x82\xb5\x82\xa2\x82\xc5\x82\xb7\x82\xa9\x81\x48")->andReturn(0x2a);
        $this->shouldWriteLong($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 0x2a);

        $this->shouldCall('_scrollTowardSelection_8c01ebc8');

        $this->expectDrawingTail(5, 0, 0, 0x2a);
    }

    public function test_state_3_scroll_up_decrements_selection(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail(1, 0);

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 3);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x10);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x38, 0);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("");

        $this->shouldCall('_scrollTowardSelection_8c01ebc8');

        $this->expectDrawingTail(3, 0, 0);
    }

    public function test_state_3_scroll_up_at_zero_is_noop(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail(0, 0);

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 3);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x10);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $this->shouldCall('_scrollTowardSelection_8c01ebc8');

        $this->expectDrawingTail(3);
    }

    public function test_state_3_scroll_down_increments_selection(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail(5, 0);

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 3);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x20);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x38, 6);
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("");
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);

        $this->shouldCall('_scrollTowardSelection_8c01ebc8');

        $this->expectDrawingTail(3, 6, 0);
    }

    public function test_state_3_scroll_down_overflow_advances_state(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail(10, 0);

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 3);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x20);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x38, 11);
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("");
        $this->shouldWriteLong($menuStateBase + 0x18, 4);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);

        $this->shouldCall('_scrollTowardSelection_8c01ebc8');

        // scrollTowardSelection_8c01ebc8 is mocked above (same-object call), so its
        // real body never runs -- scrollTopRow_0x44 stays at the value setUpDrawingTail set.
        $this->expectDrawingTail(4, 11, 0);
    }

    public function test_state_4_idle_no_input(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail(11, 0);

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 4);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $this->expectDrawingTail(4, 11, 0);
    }

    public function test_state_4_confirm_shows_quit_prompt(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail(11, 0);

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 4);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 4);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x1c, 4);
        $this->shouldWriteLong($menuStateBase + 0x18, 5);
        $this->shouldWriteLong($menuStateBase + 0x3c, 0);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0, 0);
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("\x97\xfb\x8f\x4b\x83\x82\x81\x5b\x83\x68\x82\xf0\x8f\x49\x97\xb9\x82\xb5\x82\xdc\x82\xb7\x3c\x45\x3e\x82\xe6\x82\xeb\x82\xb5\x82\xa2\x82\xc5\x82\xb7\x82\xa9\x81\x48")->andReturn(0x2a);
        $this->shouldWriteLong($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 0x2a);

        // Unlike state 5, state 4's own branch has no shared prompt-indicator
        // draw -- it falls straight through to the drawing tail.
        $this->expectDrawingTail(5, 11, 0, 0x2a);
    }

    public function test_state_4_scroll_up_returns_to_state_3(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail(11, 0);

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 4);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x10);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x38, 10);
        $this->shouldWriteLong($menuStateBase + 0x18, 3);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);

        $this->expectDrawingTail(3, 10, 0);
    }

    public function test_state_5_confirm_starts_loading(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 5);
        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x3c, 0);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($menuStateBase + 0x3c)->andReturn(1);
        $this->shouldWriteLong($menuStateBase + 0x18, 7);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0, 0);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(0);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(1);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);

        $this->shouldCall('_SpriteDraw_8c014f54')->with($menuStateBase, 2, 228.0, 320.0, -5.0);

        $this->expectDrawingTail(7);
    }

    public function test_state_5_cancel_returns_to_saved_state(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 5);
        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x1c, 3);
        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x3c, 0);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($menuStateBase + 0x3c)->andReturn(2);
        $this->shouldWriteLong($menuStateBase + 0x18, 3);
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("");
        $this->shouldWriteLong($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 0);

        $this->shouldCall('_SpriteDraw_8c014f54')->with($menuStateBase, 2, 228.0, 320.0, -5.0);

        $this->expectDrawingTail(3, 0, 0, 0);
    }

    public function test_state_5_idle_no_result(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 5);
        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x3c, 0);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldCall('_PromptHandleBinary_8c016caa')->with($menuStateBase + 0x3c)->andReturn(0);

        $this->shouldCall('_SpriteDraw_8c014f54')->with($menuStateBase, 2, 228.0, 320.0, -5.0);

        $this->expectDrawingTail(5);
    }

    public function test_drawing_tail_scrolled_practice_mode_with_textbox(): void
    {
        // Covers the shared tail's remaining branches: scroll indicator
        // (scrollTopRow_0x44 > 0), the textbox-has-text draw, and the practice-mode
        // (gameMode != 0) icon -- exercised together via a passthrough state.
        $this->resolveSymbols();
        $this->setUpDrawingTail(selected: 4, scroll: 2, gameMode: 1);

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 1);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $this->expectDrawingTail(1, selected: 4, scroll: 2, gameMode: 1, textboxHasText: true);
    }

    public function test_state_6_idle_while_fading(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 6);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $this->expectDrawingTail(6);
    }

    public function test_state_6_returns_to_course_review(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 6);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x38, 7); // selected_0x38

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $this->shouldWriteLong($this->addressOf('_var_practiceLesson_8c22640c'), 7);
        $this->shouldWriteLong($this->addressOf('_var_runWasPractice_8c1bb8bc'), 0);
        $this->shouldCall('_enterGuide_8c01e920')->with($task);
    }

    public function test_state_7_idle_while_fading(): void
    {
        $this->resolveSymbols();
        $this->setUpDrawingTail();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 7);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);
        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x3c, 0);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldCall('_SpriteDraw_8c014f54')->with($menuStateBase, 2, 228.0, 320.0, -5.0);

        $this->expectDrawingTail(7);
    }

    public function test_state_7_loading_still_pending(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 7);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_adxPlaying_8c03bd80'), 1);

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);
    }

    public function test_state_7_first_pass_returns_to_course_menu(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 7);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_adxPlaying_8c03bd80'), 0);
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);
        $this->initUint32($this->addressOf('_var_lessonAttempts_8c22642c'), 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 10); // days_0x00

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $this->shouldWriteLong($this->addressOf('_var_runReportPending_8c1bb8b8'), 0);
        $this->shouldWriteLong($this->addressOf('_var_runWasPractice_8c1bb8bc'), 1);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x3c, 0);
        $this->shouldWriteLong($menuStateBase + 0x40, 0);
        $this->shouldWriteLong($menuStateBase + 0x24, 0); // pos.title.flagY_0x24
        $this->shouldCall('_ReplayMenuFreeSessionAssets_8c016182');
        $this->shouldCall('_CourseMenuReturn_8c017ef2');
    }

    public function test_state_7_repeat_award_bumps_days_and_exp(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 7);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_adxPlaying_8c03bd80'), 0);
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);
        $this->initUint32($this->addressOf('_var_lessonAttempts_8c22642c'), 1);

        $progressBase = $this->addressOf('_var_progress_8c1ba1cc');
        $this->initUint32($progressBase, 10); // days_0x00
        $this->initUint32($progressBase + 0x90, 99980); // exp_0x90

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $this->shouldWriteLong($progressBase, 11); // days_0x00 + 1
        // exp_0x90 += 50 (99980 -> 100030) then clamped down to 99999: two
        // separate stores, both consumed off the same expectation queue.
        $this->shouldWriteLong($progressBase + 0x90, 100030);
        $this->shouldWriteLong($progressBase + 0x90, 99999);
        $this->shouldWriteLong($this->addressOf('_var_runReportPending_8c1bb8b8'), 1);
        $this->shouldWriteLong($this->addressOf('_var_runWasPractice_8c1bb8bc'), 1);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x3c, 0);
        $this->shouldWriteLong($menuStateBase + 0x40, 0);
        $this->shouldWriteLong($menuStateBase + 0x24, 0);
        $this->shouldCall('_ReplayMenuFreeSessionAssets_8c016182');
        $this->shouldCall('_CourseMenuReturn_8c017ef2');
    }

    public function test_state_7_days_over_30_unlocks_courses(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 7);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_adxPlaying_8c03bd80'), 0);
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);
        $this->initUint32($this->addressOf('_var_lessonAttempts_8c22642c'), 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 31); // days_0x00

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $this->shouldWriteLong($this->addressOf('_var_runReportPending_8c1bb8b8'), 0);
        $this->shouldWriteLong($this->addressOf('_var_runWasPractice_8c1bb8bc'), 1);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x3c, 0);
        $this->shouldWriteLong($menuStateBase + 0x40, 0);
        $this->shouldWriteLong($menuStateBase + 0x24, 0);
        $this->shouldCall('_ReplayMenuFreeSessionAssets_8c016182');
        $this->shouldCall('_CourseMenuBuildCourseUnlockList_8c0172dc');
        $this->shouldCall('_CourseMenuApplyUnlocks_8c0173e6');
        $this->shouldCall('_EndingStart_8c01f954');
    }

    public function test_state_7_nonzero_gamemode_skips_progress_update(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 7);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_adxPlaying_8c03bd80'), 0);
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 1);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 31); // days_0x00, > 30 but gameMode == 1

        $task = $this->alloc(0x20);
        $this->call('_lessonMenuTask_8c01ebf2')->with($task, 0);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x3c, 0);
        $this->shouldWriteLong($menuStateBase + 0x40, 0);
        $this->shouldWriteLong($menuStateBase + 0x24, 0);
        $this->shouldCall('_ReplayMenuFreeSessionAssets_8c016182');
        $this->shouldCall('_CourseMenuReturn_8c017ef2');
    }

    /**
     * Neutral state for the shared drawing tail: no scroll, first row
     * selected, exp/story mode, single-digit scores so drawDigits_8c01ead8
     * (a real call, not mocked) makes exactly one sprite draw per row.
     * $selected/$scroll let callers override selected_0x38/scrollTopRow_0x44 for
     * state-3 input tests.
     */
    private function setUpDrawingTail(int $selected = 0, int $scroll = 0, int $gameMode = 0): void
    {
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), $gameMode);
        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x44, $scroll); // scrollTopRow_0x44 (scroll)
        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x38, $selected); // selected_0x38
        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x60, 5); // instructorSprite_0x60
        $this->initUint32($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 0xff);

        $progressBase = $this->addressOf('_var_progress_8c1ba1cc');
        for ($i = 0; $i < 5; $i++) {
            $row = $scroll + $i;
            $this->initUint32($progressBase + 0x98 + $row * 4, $row); // practiceLessonBestScores_0x98[row], single digit
        }
    }

    private function expectDrawingTail(
        int $state,
        int $selected = 0,
        int $scroll = 0,
        int $charLimit = 0xff,
        int $gameMode = 0,
        bool $textboxHasText = false
    ): void {
        $groupB = $this->addressOf('_var_menuState_8c1bc7a8') + 0x0c;
        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');

        if ($gameMode == 0) {
            $this->shouldCall('_CourseMenuDrawDateAndExp_8c016ee6');
        }

        $y = 108.0;
        for ($i = 0; $i < 5; $i++) {
            $row = $scroll + $i;
            $spriteId = ($row === $selected && $state !== 4) ? $row + 0x34 : $row + 1;
            $this->shouldCall('_SpriteDraw_8c014f54')->with($groupB, $spriteId, 48.0, $y, -4.5);
            $this->shouldCall('_drawDigits_8c01ead8')->with($row, $y);
            $y += 33.0;
        }

        if ($selected > 10) {
            $this->shouldCall('_SpriteDraw_8c014f54')->with($groupB, 0x1b, 0.0, 0.0, -2.0);
        }
        if ($scroll > 0) {
            $this->shouldCall('_SpriteDraw_8c014f54')->with($groupB, 0x1a, 0.0, 0.0, -2.0);
        }
        if ($scroll < 6) {
            $this->shouldCall('_SpriteDraw_8c014f54')->with($groupB, 0x19, 0.0, 0.0, -2.0);
        }

        $this->shouldCall('_MessageBoxDrawText_8c02af1c')->with($charLimit)->andReturn($textboxHasText ? 1 : 0);
        if ($textboxHasText) {
            $this->shouldCall('_SpriteDraw_8c014f54')->with($menuStateBase, 1, 0.0, 0.0, -5.0);
        }

        $this->shouldCall('_SpriteDraw_8c014f54')->with($groupB, 5 + 0x32, 0.0, 0.0, -6.0);
        $modeIcon = $gameMode == 0 ? 0 : 0x40;
        $this->shouldCall('_SpriteDraw_8c014f54')->with($groupB, $modeIcon, 0.0, 0.0, -3.0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($menuStateBase, 0, 0.0, 0.0, -8.0);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x84);
        $this->setSize('_MessageBoxSetText_8c02aefc', 4);
        $this->setSize('_var_isFading_8c226568', 4);
        $this->setSize('_CourseMenuInstallDialog_8c0170c6', 4);
        $this->setSize('_var_instructorDialogActive_8c225fb4', 4);
        $this->setSize('_var_peripherals_8c1ba35c', 0x68); // 2 x PDS_PERIPHERAL
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_var_midiHandles_8c0fcd28', 0x20); // 8 x SDMIDI
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x1234); // sentinel handle
        $this->setSize('_var_menuTextboxCharLimit_8c225fb8', 4);
        $this->setSize('_RenderStartFadeOut_8c022b60', 4);
        $this->setSize('_PromptHandleBinary_8c016caa', 4);
        $this->setSize('_SndStartAdxFadeOut_8c010bae', 4);
        $this->setSize('_var_runWasPractice_8c1bb8bc', 4);
        $this->setSize('_init_adxPlaying_8c03bd80', 4);
        $this->setSize('_var_gameMode_8c1bb8fc', 4);
        $this->setSize('_var_runReportPending_8c1bb8b8', 4);
        $this->setSize('_var_progress_8c1ba1cc', 0xe8);
        $this->setSize('_ReplayMenuFreeSessionAssets_8c016182', 4);
        $this->setSize('_CourseMenuBuildCourseUnlockList_8c0172dc', 4);
        $this->setSize('_CourseMenuApplyUnlocks_8c0173e6', 4);
        $this->setSize('_EndingStart_8c01f954', 4);
        $this->setSize('_CourseMenuReturn_8c017ef2', 4);
        $this->setSize('_CourseMenuDrawDateAndExp_8c016ee6', 4);
        $this->setSize('_MessageBoxDrawText_8c02af1c', 4);
        $this->setSize('_SpriteDraw_8c014f54', 4);
    }
};
