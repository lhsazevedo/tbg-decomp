<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_state_0_waits_for_pvm_ready(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 0);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(1);
    }

    public function test_state_0_advances(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 0);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 3);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(0);
        $this->shouldCall('_AsqFreeQueues_8c011f7e');

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x18, 1);
        // Course 3's page range: init_lessonPageStarts_8c0451b4[3..4] = {4, 6}
        $this->shouldWriteLong($menuStateBase + 0x54, 4);
        $this->shouldWriteLong($menuStateBase + 0x5c, 4);
        $this->shouldWriteLong($menuStateBase + 0x58, 5);

        $this->shouldCall('_SndPlayAdx_8c010cd6')->with(0, 0xd);
        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    public function test_state_1_waits_for_fade(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 1);
        $this->initMenuStateUint32(0x5c, 4);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase + 0x0c,
            4,
            0.0, 0.0, -4.5
        );
    }

    public function test_state_1_advances_when_fade_complete(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 1);
        $this->initMenuStateUint32(0x5c, 4);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x18, 2);
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase + 0x0c,
            4,
            0.0, 0.0, -4.5
        );
    }

    public function test_state_2_idle_when_no_press(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 2);
        $this->initMenuStateUint32(0x5c, 4);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase + 0x0c,
            4,
            0.0, 0.0, -4.5
        );
    }

    public function test_state_2_press_advances_page(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 2);
        $this->initMenuStateUint32(0x5c, 4);
        $this->initMenuStateUint32(0x58, 5);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x4);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x5c, 5);
        $this->shouldCall('_sdMidiPlay')->with(
            0x1234,
            1, 0, 0
        );
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase + 0x0c,
            5,
            0.0, 0.0, -4.5
        );
    }

    public function test_state_2_press_on_last_page_shows_prompt(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 2);
        $this->initMenuStateUint32(0x5c, 5);
        $this->initMenuStateUint32(0x58, 5);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x4);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x18, 3);
        $this->shouldWriteLong($menuStateBase + 0x38, 0);
        // "—ûK‚ð‚Í‚¶‚ß‚Ü‚·‚©H" (Shift-JIS bytes, matches const_confirmStartMsg_8c03896c)
        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("\x97\xfb\x8f\x4b\x82\xf0\x82\xcd\x82\xb6\x82\xdf\x82\xdc\x82\xb7\x82\xa9\x81\x48");
        $this->shouldCall('_sdMidiPlay')->with(
            0x1234,
            1, 0, 0
        );
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase + 0x0c,
            5,
            0.0, 0.0, -4.5
        );
    }

    public function test_state_3_idle(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 3);
        $this->initMenuStateUint32(0x38, 0);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $this->shouldCall('_PromptHandleBinary_8c016caa')->andReturn(0);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase,
            2,
            224.0, 300.0, -4.0
        );
        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0xff)->andReturn(0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase,
            0,
            0.0, 0.0, -8.0
        );
    }

    public function test_state_3_confirm_starts_fadeout(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 3);
        $this->initMenuStateUint32(0x38, 0);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $this->shouldCall('_PromptHandleBinary_8c016caa')->andReturn(1);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x18, 4);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(0);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(1);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);

        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase,
            2,
            224.0, 300.0, -4.0
        );
        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0xff)->andReturn(0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase,
            0,
            0.0, 0.0, -8.0
        );
    }

    public function test_state_3_cancel_starts_fadeout(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 3);
        $this->initMenuStateUint32(0x38, 1);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $this->shouldCall('_PromptHandleBinary_8c016caa')->andReturn(2);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x18, 8);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(0);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(1);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);

        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase,
            3,
            224.0, 300.0, -4.0
        );
        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0xff)->andReturn(0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase,
            0,
            0.0, 0.0, -8.0
        );
    }

    public function test_state_3_text_reveal_draws_extra_sprite(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 3);
        $this->initMenuStateUint32(0x38, 0);
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $this->shouldCall('_PromptHandleBinary_8c016caa')->andReturn(0);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase,
            2,
            224.0, 300.0, -4.0
        );
        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0xff)->andReturn(1);
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase,
            1,
            0.0, 0.0, -5.0
        );
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase,
            0,
            0.0, 0.0, -8.0
        );
    }

    public function test_state_4_advances_when_fade_complete(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 4);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x18, 5);
        $this->shouldWriteLong($menuStateBase + 0x68, 0);
        $this->shouldCall('_njSetBackColor')->with(0, 0, 0);
        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(0x14);
    }

    public function test_state_4_draws_while_fading(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 4);
        $this->initMenuStateUint32(0x38, 0);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase,
            2,
            224.0, 300.0, -4.0
        );
        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0xff)->andReturn(0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase,
            0,
            0.0, 0.0, -8.0
        );
    }

    public function test_state_5_waits_for_fade(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 5);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 3);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase + 0x0c,
            3 + 0x19,
            0.0, 0.0, -4.5
        );
    }

    public function test_state_5_advances_when_fade_complete(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 5);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 3);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x18, 6);
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase + 0x0c,
            3 + 0x19,
            0.0, 0.0, -4.5
        );
    }

    public function test_state_6_counts_up(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 6);
        $this->initMenuStateUint32(0x68, 3);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 3);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x68, 4);
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase + 0x0c,
            3 + 0x19,
            0.0, 0.0, -4.5
        );
    }

    public function test_state_6_advances_after_timeout(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 6);
        $this->initMenuStateUint32(0x68, 10);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 3);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x68, 11);
        $this->shouldWriteLong($menuStateBase + 0x18, 7);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(0x14);
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase + 0x0c,
            3 + 0x19,
            0.0, 0.0, -4.5
        );
    }

    public function test_state_7_draws_while_fading(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 7);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 3);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase + 0x0c,
            3 + 0x19,
            0.0, 0.0, -4.5
        );
    }

    public function test_state_7_waits_for_asset_load(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 7);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_adxPlaying_8c03bd80'), 1);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);
    }

    public function test_state_7_starts_loading_task(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 7);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_adxPlaying_8c03bd80'), 0);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 7);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $this->shouldCall('_ReplayMenuFreeSessionAssets_8c016182');
        $this->shouldWriteLong($this->addressOf('_var_worstPenaltyDelta_8c1bb8f0'), 0);
        $this->shouldWriteLong($this->addressOf('_var_worstPenaltyMsgSet_8c1bb8ec'), 0x1d);
        $this->shouldWriteLong($this->addressOf('_var_penaltyCount_8c1bb8f4'), 0);
        $this->shouldWriteLong($this->addressOf('_var_practiceRules_8c226410'), 6);
        $this->shouldCall('_GameStartCourse_8c013310')->with(7 + 0x1b);
    }

    public function test_state_8_waits_for_asset_load(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 8);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_adxPlaying_8c03bd80'), 1);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);
    }

    public function test_state_8_installs_next_task(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 8);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_adxPlaying_8c03bd80'), 0);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $this->shouldCall('_practiceCancelReturn_8c01e920')->with($task);
    }

    public function test_state_8_draws_while_fading(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 8);
        $this->initMenuStateUint32(0x38, 0);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase,
            2,
            224.0, 300.0, -4.0
        );
        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0xff)->andReturn(0);
    }

    public function test_state_8_text_reveal_draws_extra_sprite(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 8);
        $this->initMenuStateUint32(0x38, 0);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $task = $this->alloc(0x20);
        $this->call('_lessonDescriptionTask_8c01e27c')->with($task);

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase,
            2,
            224.0, 300.0, -4.0
        );
        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(0xff)->andReturn(1);
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $menuStateBase,
            1,
            0.0, 0.0, -4.5
        );
    }

    private function initMenuStateUint32($offset, $value): void
    {
        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + $offset, $value);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x84);
        $this->setSize('_var_isFading_8c226568', 4);
        $this->setSize('_SpriteDraw_8c014f54', 4);
        $this->setSize('_init_adxPlaying_8c03bd80', 4);
        $this->setSize('_MessageBoxMenuTextboxText_8c02af1c', 4);
        $this->setSize('_var_peripherals_8c1ba35c', 0x68); // 2 x PDS_PERIPHERAL
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_var_midiHandles_8c0fcd28', 0x20); // 8 x SDMIDI
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x1234); // sentinel handle
        $this->setSize('_MessageBoxSwapFor_8c02aefc', 4);
        $this->setSize('_PromptHandleBinary_8c016caa', 4);
        $this->setSize('_SndStartAdxFadeOut_8c010bae', 4);
        $this->setSize('_RenderStartFadeOut_8c022b60', 4);
        $this->setSize('_njSetBackColor', 4);
        $this->setSize('_ReplayMenuFreeSessionAssets_8c016182', 4);
        $this->setSize('_GameStartCourse_8c013310', 4);
        $this->setSize('_var_worstPenaltyDelta_8c1bb8f0', 4);
        $this->setSize('_var_worstPenaltyMsgSet_8c1bb8ec', 4);
        $this->setSize('_var_penaltyCount_8c1bb8f4', 4);
    }
};
