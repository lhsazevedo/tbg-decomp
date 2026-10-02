<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_init_waits_for_pvm_ready()
    {
        $this->resolveSymbols();
        $menuState = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($menuState + 0x18, 0); // state_0x18 = GUIDE_STATE_INIT

        $task = $this->alloc(0x20);
        $this->call('_lessonGuideTask_8c01e63c')->with($task);

        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(1);
    }

    public function test_init_frees_queues_and_fades_in_when_pvm_ready()
    {
        $this->resolveSymbols();
        $menuState = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($menuState + 0x18, 0); // state_0x18 = GUIDE_STATE_INIT

        $task = $this->alloc(0x20);
        $this->call('_lessonGuideTask_8c01e63c')->with($task);

        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(0);
        $this->shouldCall('_AsqFreeQueues_8c011f7e');
        $this->shouldWriteLong($menuState + 0x18, 1);
        $this->shouldCall('_SndPlayAdx_8c010cd6')->with(0, 0xd);
        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    public function test_fade_in_advances_to_view_and_draws()
    {
        $this->resolveSymbols();
        $menuState = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($menuState + 0x18, 1); // state_0x18 = GUIDE_STATE_FADE_IN
        $this->initUint32($menuState + 0x40, 2); // cursorRow_0x40
        $this->initUint32($menuState + 0x0c, 0x11111111); // resourceGroupB_0x0c
        $this->initUint32($menuState + 0x00, 0x22222222); // resourceGroupA_0x00
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 5);
        $this->initUint8($this->addressOf('_init_lessonGuideRows_8c0451ec') + 5, 10);
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);

        $task = $this->alloc(0x20);
        $this->call('_lessonGuideTask_8c01e63c')->with($task);

        $this->shouldWriteLong($menuState + 0x18, 2);

        $this->shouldCall('_CourseMenuDrawDateAndExp_8c016ee6');

        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuState + 0x0c, 5 + 0x1c, 0.0, 0.0, -4.0
        );

        $this->shouldCall('_njUserClipping')->with(2, $this->addressOf('_init_guideClipRect_8c0451f8'));
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuState + 0x0c, 5 + 0x27, 65.0, 195.0 - 2.0 * 24.0, -4.0
        );
        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_guideClipRect_8c0451f8'));

        // cursorRow_0x40 (2) > 0 -> up arrow
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuState + 0x0c, 0x18, 0.0, 0.0, -4.5
        );

        // cursorRow_0x40 + 3 (5) != init_lessonGuideRows_8c0451ec[5] (10) -> down arrow
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuState + 0x0c, 0x19, 0.0, 0.0, -4.5
        );

        // not GUIDE_STATE_VIEW_END -> no 0x1b sprite

        $this->shouldCall('_MessageBoxDrawText_8c02af1c')->with(0xff)->andReturn(0);
        // menuTextboxText returned 0 -> no id=1 sprite

        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuState + 0x0c, 0x32, 0.0, 0.0, -6.0
        );

        // gameMode == 0 -> id 0x17
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuState + 0x0c, 0x17, 0.0, 0.0, -7.0
        );

        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuState + 0x00, 0, 0.0, 0.0, -8.0
        );
    }

    public function test_view_confirm_fades_out_to_back()
    {
        $this->resolveSymbols();
        $menuState = $this->setupViewDraw(state: 2, field: 0, press: 0x4);

        $task = $this->alloc(0x20);
        $this->call('_lessonGuideTask_8c01e63c')->with($task);

        $this->shouldWriteLong($menuState + 0x18, 4); // GUIDE_STATE_FADE_OUT_TO_DESCRIPTION
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(0);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(1);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0, 0);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);

        $this->expectDrawTail($menuState, field: 0, course: 5, pageCount: 10, gameMode: 0);
    }

    public function test_view_cancel_fades_out_to_start()
    {
        $this->resolveSymbols();
        $menuState = $this->setupViewDraw(state: 2, field: 0, press: 0x2);

        $task = $this->alloc(0x20);
        $this->call('_lessonGuideTask_8c01e63c')->with($task);

        $this->shouldWriteLong($menuState + 0x18, 5); // GUIDE_STATE_FADE_OUT_TO_MENU
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 1, 0);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);

        $this->expectDrawTail($menuState, field: 0, course: 5, pageCount: 10, gameMode: 0);
    }

    public function test_view_up_scrolls_back_when_above_top()
    {
        $this->resolveSymbols();
        $menuState = $this->setupViewDraw(state: 2, field: 2, press: 0x10);

        $task = $this->alloc(0x20);
        $this->call('_lessonGuideTask_8c01e63c')->with($task);

        $this->shouldWriteLong($menuState + 0x40, 1);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);

        $this->expectDrawTail($menuState, field: 1, course: 5, pageCount: 10, gameMode: 0);
    }

    public function test_view_up_does_nothing_at_top()
    {
        $this->resolveSymbols();
        $menuState = $this->setupViewDraw(state: 2, field: 0, press: 0x10);

        $task = $this->alloc(0x20);
        $this->call('_lessonGuideTask_8c01e63c')->with($task);

        $this->expectDrawTail($menuState, field: 0, course: 5, pageCount: 10, gameMode: 0);
    }

    public function test_view_down_scrolls_forward_before_end()
    {
        $this->resolveSymbols();
        $menuState = $this->setupViewDraw(state: 2, field: 0, press: 0x20);

        $task = $this->alloc(0x20);
        $this->call('_lessonGuideTask_8c01e63c')->with($task);

        $this->shouldWriteLong($menuState + 0x40, 1);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);

        $this->expectDrawTail($menuState, field: 1, course: 5, pageCount: 10, gameMode: 0);
    }

    public function test_view_down_scrolls_forward_just_before_end()
    {
        $this->resolveSymbols();
        // cursorRow_0x40 + 3 (9) < pageCount(10) -> still scrolls
        $menuState = $this->setupViewDraw(state: 2, field: 6, press: 0x20);

        $task = $this->alloc(0x20);
        $this->call('_lessonGuideTask_8c01e63c')->with($task);

        $this->shouldWriteLong($menuState + 0x40, 7);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);

        $this->expectDrawTail($menuState, field: 7, course: 5, pageCount: 10, gameMode: 0);
    }

    public function test_view_down_reaches_end()
    {
        $this->resolveSymbols();
        // cursorRow_0x40 + 3 (10) >= pageCount(10) -> reaches the end
        $menuState = $this->setupViewDraw(state: 2, field: 7, press: 0x20);

        $task = $this->alloc(0x20);
        $this->call('_lessonGuideTask_8c01e63c')->with($task);

        $this->shouldWriteLong($menuState + 0x18, 3); // GUIDE_STATE_VIEW_END
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);

        $this->expectDrawTail($menuState, field: 7, course: 5, pageCount: 10, gameMode: 0, viewEnd: true);
    }

    public function test_view_end_confirm_fades_out_to_start()
    {
        $this->resolveSymbols();
        $menuState = $this->setupViewDraw(state: 3, field: 7, press: 0x4);

        $task = $this->alloc(0x20);
        $this->call('_lessonGuideTask_8c01e63c')->with($task);

        $this->shouldWriteLong($menuState + 0x18, 5); // GUIDE_STATE_FADE_OUT_TO_MENU
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 1, 0);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);

        // state is now FADE_OUT_START (5), not VIEW_END, so no 0x1b sprite
        $this->expectDrawTail($menuState, field: 7, course: 5, pageCount: 10, gameMode: 0, viewEnd: false);
    }

    public function test_view_end_up_returns_to_view()
    {
        $this->resolveSymbols();
        $menuState = $this->setupViewDraw(state: 3, field: 7, press: 0x10);

        $task = $this->alloc(0x20);
        $this->call('_lessonGuideTask_8c01e63c')->with($task);

        $this->shouldWriteLong($menuState + 0x18, 2); // GUIDE_STATE_VIEW
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);

        // state is now VIEW (2), not VIEW_END, so the draw tail no longer shows the 0x1b sprite
        $this->expectDrawTail($menuState, field: 7, course: 5, pageCount: 10, gameMode: 0, viewEnd: false);
    }

    public function test_fade_out_back_still_fading_only_draws()
    {
        $this->resolveSymbols();
        $menuState = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($menuState + 0x18, 4); // GUIDE_STATE_FADE_OUT_TO_DESCRIPTION
        $this->initUint32($menuState + 0x40, 0);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 5);
        $this->initUint8($this->addressOf('_init_lessonGuideRows_8c0451ec') + 5, 10);
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);
        $this->initUint32($menuState + 0x0c, 0x11111111);
        $this->initUint32($menuState + 0x00, 0x22222222);

        $task = $this->alloc(0x20);
        $this->call('_lessonGuideTask_8c01e63c')->with($task);

        $this->expectDrawTail($menuState, field: 0, course: 5, pageCount: 10, gameMode: 0);
    }

    public function test_fade_out_back_done_returns_to_description_reveal()
    {
        $this->resolveSymbols();
        $menuState = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($menuState + 0x18, 4); // GUIDE_STATE_FADE_OUT_TO_DESCRIPTION
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_adxPlaying_8c03bd80'), 0);

        $task = $this->alloc(0x20);
        $this->call('_lessonGuideTask_8c01e63c')->with($task);

        $this->shouldCall('_enterDescription_8c01e576')->with($task);
    }

    public function test_fade_out_back_waits_while_still_loading()
    {
        $this->resolveSymbols();
        $menuState = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($menuState + 0x18, 4); // GUIDE_STATE_FADE_OUT_TO_DESCRIPTION
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_adxPlaying_8c03bd80'), 1);

        $task = $this->alloc(0x20);
        $this->call('_lessonGuideTask_8c01e63c')->with($task);
    }

    public function test_fade_out_start_done_starts_lesson()
    {
        $this->resolveSymbols();
        $menuState = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($menuState + 0x18, 5); // GUIDE_STATE_FADE_OUT_TO_MENU
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 5);

        $task = $this->alloc(0x20);
        $this->call('_lessonGuideTask_8c01e63c')->with($task);

        $this->shouldWriteLong($menuState + 0x38, 5); // selected_0x38 = var_practiceLesson_8c22640c
        $this->shouldCall('_PracticeMenuEnter_8c01f114')->with($task);
    }

    public function test_fade_out_start_still_fading_only_draws()
    {
        $this->resolveSymbols();
        $menuState = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($menuState + 0x18, 5); // GUIDE_STATE_FADE_OUT_TO_MENU
        $this->initUint32($menuState + 0x40, 0);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 5);
        $this->initUint8($this->addressOf('_init_lessonGuideRows_8c0451ec') + 5, 10);
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);
        $this->initUint32($menuState + 0x0c, 0x11111111);
        $this->initUint32($menuState + 0x00, 0x22222222);

        $task = $this->alloc(0x20);
        $this->call('_lessonGuideTask_8c01e63c')->with($task);

        $this->expectDrawTail($menuState, field: 0, course: 5, pageCount: 10, gameMode: 0);
    }

    public function test_view_draws_textbox_when_present()
    {
        $this->resolveSymbols();
        $menuState = $this->setupViewDraw(state: 2, field: 0, press: 0x10);

        $task = $this->alloc(0x20);
        $this->call('_lessonGuideTask_8c01e63c')->with($task);

        $this->expectDrawTail($menuState, field: 0, course: 5, pageCount: 10, gameMode: 0, textboxHasText: true);
    }

    private function setupViewDraw(int $state, int $field, int $press): int
    {
        $menuState = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($menuState + 0x18, $state);
        $this->initUint32($menuState + 0x40, $field);
        $this->initUint32($menuState + 0x0c, 0x11111111);
        $this->initUint32($menuState + 0x00, 0x22222222);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, $press);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 5);
        $this->initUint8($this->addressOf('_init_lessonGuideRows_8c0451ec') + 5, 10);
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);

        return $menuState;
    }

    private function expectDrawTail(
        int $menuState, int $field, int $course, int $pageCount, int $gameMode,
        bool $viewEnd = false, bool $textboxHasText = false
    ): void {
        $this->shouldCall('_CourseMenuDrawDateAndExp_8c016ee6');

        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuState + 0x0c, $course + 0x1c, 0.0, 0.0, -4.0
        );

        $this->shouldCall('_njUserClipping')->with(2, $this->addressOf('_init_guideClipRect_8c0451f8'));
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuState + 0x0c, $course + 0x27, 65.0, 195.0 - (float)$field * 24.0, -4.0
        );
        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_guideClipRect_8c0451f8'));

        if ($field > 0) {
            $this->shouldCall('_SpriteDraw_8c014f54')->with(
                $menuState + 0x0c, 0x18, 0.0, 0.0, -4.5
            );
        }

        if ($field + 3 != $pageCount) {
            $this->shouldCall('_SpriteDraw_8c014f54')->with(
                $menuState + 0x0c, 0x19, 0.0, 0.0, -4.5
            );
        }

        if ($viewEnd) {
            $this->shouldCall('_SpriteDraw_8c014f54')->with(
                $menuState + 0x0c, 0x1b, 0.0, 0.0, -4.5
            );
        }

        $this->shouldCall('_MessageBoxDrawText_8c02af1c')->with(0xff)->andReturn($textboxHasText ? 1 : 0);

        if ($textboxHasText) {
            $this->shouldCall('_SpriteDraw_8c014f54')->with(
                $menuState + 0x00, 1, 0.0, 0.0, -5.0
            );
        }

        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuState + 0x0c, 0x32, 0.0, 0.0, -6.0
        );

        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuState + 0x0c, $gameMode == 0 ? 0x17 : 0x41, 0.0, 0.0, -7.0
        );

        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuState + 0x00, 0, 0.0, 0.0, -8.0
        );
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x84);
        $this->setSize('_RouteGetLatch_8c01432a', 4);
        $this->setSize('_AsqFreeQueues_8c011f7e', 4);
        $this->setSize('_SndPlayAdx_8c010cd6', 4);
        $this->setSize('_RenderStartFadeIn_8c022a9c', 4);
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_var_midiHandles_8c0fcd28', 4);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x1234); // sentinel handle
        $this->setSize('_var_peripherals_8c1ba35c', 0x34);
        $this->setSize('_var_isFading_8c226568', 4);
        $this->setSize('_init_adxPlaying_8c03bd80', 4);
        $this->setSize('_SndStartAdxFadeOut_8c010bae', 4);
        $this->setSize('_RenderStartFadeOut_8c022b60', 4);
        $this->setSize('_njUserClipping', 4);
        $this->setSize('_SpriteDraw_8c014f54', 4);
        $this->setSize('_var_gameMode_8c1bb8fc', 4);
        $this->setSize('_CourseMenuDrawDateAndExp_8c016ee6', 4);
        $this->setSize('_MessageBoxDrawText_8c02af1c', 4);
    }
};
