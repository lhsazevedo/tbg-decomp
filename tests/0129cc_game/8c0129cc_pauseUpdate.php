<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_closed_without_start_returns_1()
    {
        $this->setup(press: 0, ctrl: 0);

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldReturn(1);
    }

    public function test_start_opens_the_menu()
    {
        $this->setup(press: 8, ctrl: 0, vibport: -1);

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldWriteLong($this->addressOf('_var_pauseActive_8c1bb8cc'), 1);
        $this->shouldWriteLong($this->addressOf('_var_pauseSettle_8c18ad04'), 0);
        $this->shouldWriteLong($this->addressOf('_var_onRetire_8c18ad10'), 0);
        $this->shouldCall('_SndSetPaused_8c0107d2')->with(1);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0, 0);
        $this->shouldReturn(1);
    }

    public function test_opening_stops_the_vibration_motor()
    {
        $this->setup(press: 8, ctrl: 0, vibport: 3);

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldWriteLong($this->addressOf('_var_pauseActive_8c1bb8cc'), 1);
        $this->shouldWriteLong($this->addressOf('_var_pauseSettle_8c18ad04'), 0);
        $this->shouldWriteLong($this->addressOf('_var_onRetire_8c18ad10'), 0);
        $this->shouldCall('_SndSetPaused_8c0107d2')->with(1);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0, 0);
        $this->shouldCall('_pdVibMxStop')->with(3);
        $this->shouldReturn(1);
    }

    public function test_settle_gate_advances_to_2()
    {
        $this->setup(press: 0, ctrl: 0, pauseActive: 1, settle: 1);

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldWriteLong($this->addressOf('_var_pauseSettle_8c18ad04'), 2);
        $this->shouldReturn(0);
    }

    public function test_settle_gate_holds_at_2()
    {
        $this->setup(press: 0, ctrl: 0, pauseActive: 1, settle: 2);

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldReturn(0);
    }

    public function test_start_closes_the_menu()
    {
        $this->setup(press: 8, ctrl: 0, pauseActive: 1, settle: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_pauseActive_8c1bb8cc'), 0);
        $this->shouldCall('_SndSetPaused_8c0107d2')->with(0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_continue_a_resumes_the_drive()
    {
        $this->setup(press: 4, ctrl: 0, pauseActive: 1, settle: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_pauseActive_8c1bb8cc'), 0);
        $this->shouldCall('_SndSetPaused_8c0107d2')->with(0);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0, 0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x75, 0.0, 0.0, -1.09);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_continue_down_moves_to_retire()
    {
        $this->setup(press: 0x20, ctrl: 0, pauseActive: 1, settle: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_onRetire_8c18ad10'), 1);
        $this->shouldWriteLong($this->addressOf('_var_retirePhase_8c18ad08'), 0);
        $this->shouldWriteLong($this->addressOf('_var_confirmChoice_8c18ad0c'), 1);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x75, 0.0, 0.0, -1.09);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_continue_stick_down_moves_to_retire()
    {
        $this->setup(press: 0, ctrl: 0, pauseActive: 1, settle: 0, y1: 0x41);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_onRetire_8c18ad10'), 1);
        $this->shouldWriteLong($this->addressOf('_var_retirePhase_8c18ad08'), 0);
        $this->shouldWriteLong($this->addressOf('_var_confirmChoice_8c18ad0c'), 1);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x75, 0.0, 0.0, -1.09);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_continue_idle_draws_its_mark()
    {
        $this->setup(press: 0, ctrl: 0, pauseActive: 1, settle: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x75, 0.0, 0.0, -1.09);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_retire_a_opens_the_confirm()
    {
        $this->setup(press: 4, ctrl: 0, pauseActive: 1, settle: 0, onRetire: 1, retirePhase: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_retirePhase_8c18ad08'), 1);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0, 0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x7a, 0.0, 0.0, -1.09);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_retire_up_moves_back_to_continue()
    {
        $this->setup(press: 0x10, ctrl: 0, pauseActive: 1, settle: 0, onRetire: 1, retirePhase: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_onRetire_8c18ad10'), 0);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x7a, 0.0, 0.0, -1.09);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_retire_stick_up_moves_back_to_continue()
    {
        // y1 is a Sint16: 0xffbf is -0x41.
        $this->setup(press: 0, ctrl: 0, pauseActive: 1, settle: 0, onRetire: 1, retirePhase: 0, y1: 0xffbf);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_onRetire_8c18ad10'), 0);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x7a, 0.0, 0.0, -1.09);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_retire_idle_draws_its_mark()
    {
        $this->setup(press: 0, ctrl: 0, pauseActive: 1, settle: 0, onRetire: 1, retirePhase: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x7a, 0.0, 0.0, -1.09);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_confirm_b_cancels_back_to_retire()
    {
        $this->setup(press: 2, ctrl: 0, pauseActive: 1, onRetire: 1, retirePhase: 1);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_pauseUpdate_8c0129cc');

        // The RETIRE mark is drawn before the cancel sound, not after.
        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_retirePhase_8c18ad08'), 0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x7a, 0.0, 0.0, -1.09);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 1, 0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_confirm_yes_a_commits_and_fades_out()
    {
        $this->setup(press: 4, ctrl: 0, pauseActive: 1, onRetire: 1, retirePhase: 1, confirmChoice: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_pauseUpdate_8c0129cc');

        // Committing draws no arrow mark this frame.
        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_retirePhase_8c18ad08'), 2);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0, 0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_confirm_yes_right_selects_no()
    {
        $this->setup(press: 0x80, ctrl: 0, pauseActive: 1, onRetire: 1, retirePhase: 1, confirmChoice: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_confirmChoice_8c18ad0c'), 1);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x76, 0.0, 0.0, -1.09);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_confirm_yes_stick_right_selects_no()
    {
        $this->setup(press: 0, ctrl: 0, pauseActive: 1, onRetire: 1, retirePhase: 1, confirmChoice: 0, x1: 0x41);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_confirmChoice_8c18ad0c'), 1);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x76, 0.0, 0.0, -1.09);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_confirm_yes_idle_draws_its_mark()
    {
        $this->setup(press: 0, ctrl: 0, pauseActive: 1, onRetire: 1, retirePhase: 1, confirmChoice: 0);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x76, 0.0, 0.0, -1.09);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_confirm_no_a_cancels_back_to_retire()
    {
        $this->setup(press: 4, ctrl: 0, pauseActive: 1, onRetire: 1, retirePhase: 1, confirmChoice: 1);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_retirePhase_8c18ad08'), 0);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0, 0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x77, 0.0, 0.0, -1.09);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_confirm_no_left_selects_yes()
    {
        $this->setup(press: 0x40, ctrl: 0, pauseActive: 1, onRetire: 1, retirePhase: 1, confirmChoice: 1);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_confirmChoice_8c18ad0c'), 0);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x77, 0.0, 0.0, -1.09);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_confirm_no_stick_left_selects_yes()
    {
        $this->setup(press: 0, ctrl: 0, pauseActive: 1, onRetire: 1, retirePhase: 1, confirmChoice: 1, x1: 0xffbf);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_confirmChoice_8c18ad0c'), 0);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 3, 0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x77, 0.0, 0.0, -1.09);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_confirm_no_idle_draws_its_mark()
    {
        $this->setup(press: 0, ctrl: 0, pauseActive: 1, onRetire: 1, retirePhase: 1, confirmChoice: 1);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x77, 0.0, 0.0, -1.09);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x74, 0.0, 0.0, -1.1);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_fading_holds_the_yes_mark()
    {
        $this->setup(press: 0, ctrl: 0, pauseActive: 1, onRetire: 1, retirePhase: 2, isFading: 1);
        $mark = $this->addressOf('_var_markTexlist_8c1bc418');

        $this->call('_pauseUpdate_8c0129cc');

        // The base mark is skipped on this path.
        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldCall('_SpriteDraw_8c014f54')->with($mark, 0x76, 0.0, 0.0, -1.09);
        $this->shouldCall('_njDrawPolygon');
        $this->shouldReturn(0);
    }

    public function test_retire_from_practice_reopens_the_lesson_list()
    {
        $this->setup(press: 0, ctrl: 0, pauseActive: 1, onRetire: 1, retirePhase: 2, isFading: 0, playMode: 1);
        $this->initUint32($this->addressOf('_var_practiceLesson_8c22640c'), 0x2b);

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_runSucceeded_8c1bb8dc'), 0);
        $this->shouldWriteLong($this->addressOf('_var_runReportPending_8c1bb8b8'), 0);
        $this->shouldWriteLong($this->addressOf('_var_runWasPractice_8c1bb8bc'), 0);
        $this->shouldCall('_ReplayMenuFreeSessionAssets_8c016182');
        $this->shouldWriteLong($this->addressOf('_var_menuState_8c1bc7a8') + 0x38, 0x2b);
        $this->shouldCall('_PracticeMenuReturn_8c01f21c');
        $this->shouldReturn(0);
    }

    public function test_retire_rolls_back_progress_and_returns_to_the_course_menu()
    {
        $this->setup(press: 0, ctrl: 0, pauseActive: 1, onRetire: 1, retirePhase: 2, isFading: 0, playMode: 0);
        $b8 = $this->addressOf('_var_eventFlagsSnapshot_8c1ba2b8');
        $cc = $this->addressOf('_var_profileFlagsSnapshot_8c1ba2cc');
        $prog = $this->addressOf('_var_progress_8c1ba1cc');
        for ($i = 0; $i < 5; $i++) {
            $this->initUint32($b8 + $i * 4, 0x100 + $i);
            $this->initUint32($cc + $i * 4, 0x200 + $i);
        }

        $this->call('_pauseUpdate_8c0129cc');

        $this->shouldCall('_RenderDrawFrame_8c022560');
        $this->shouldWriteLong($this->addressOf('_var_runSucceeded_8c1bb8dc'), 0);
        $this->shouldWriteLong($this->addressOf('_var_runReportPending_8c1bb8b8'), 0);
        $this->shouldWriteLong($this->addressOf('_var_runWasPractice_8c1bb8bc'), 0);
        $this->shouldCall('_ReplayMenuFreeSessionAssets_8c016182');
        for ($i = 0; $i < 5; $i++) {
            $this->shouldWriteLong($prog + 0x04 + $i * 4, 0x100 + $i);
            $this->shouldWriteLong($prog + 0x18 + $i * 4, 0x200 + $i);
        }
        $this->shouldCall('_CourseMenuReturn_8c017ef2');
        $this->shouldReturn(0);
    }

    private function setup(int $press, int $ctrl, int $vibport = 0, int $pauseActive = 0, int $settle = 0, int $onRetire = 0, int $y1 = 0, int $retirePhase = 0, int $confirmChoice = 0, int $x1 = 0, int $isFading = 0, int $playMode = 0): void
    {
        $this->setSize('_RenderDrawFrame_8c022560', 4);
        $this->setSize('_SpriteDraw_8c014f54', 4);
        $this->setSize('_njDrawPolygon', 4);
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_SndSetPaused_8c0107d2', 4);
        $this->setSize('_pdVibMxStop', 4);
        $this->setSize('_RenderStartFadeOut_8c022b60', 4);
        $this->setSize('_ReplayMenuFreeSessionAssets_8c016182', 4);
        $this->setSize('_CourseMenuReturn_8c017ef2', 4);
        $this->setSize('_PracticeMenuReturn_8c01f21c', 4);

        $this->setSize('_var_runSucceeded_8c1bb8dc', 4);
        $this->setSize('_var_runWasPractice_8c1bb8bc', 4);
        $this->setSize('_var_isFading_8c226568', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_practiceLesson_8c22640c', 4);
        $this->setSize('_var_menuState_8c1bc7a8', 0x80);
        $this->setSize('_var_progress_8c1ba1cc', 0xd8);
        $this->setSize('_var_eventFlagsSnapshot_8c1ba2b8', 0x14);
        $this->setSize('_var_profileFlagsSnapshot_8c1ba2cc', 0x14);

        $this->setSize('_var_pauseActive_8c1bb8cc', 4);
        $this->setSize('_var_runReportPending_8c1bb8b8', 4);

        $periph = $this->alloc(0x34);
        $this->initUint32($periph + 0x10, $press);
        $this->initUint16($periph + 0x1c, $x1);
        $this->initUint16($periph + 0x1e, $y1);
        $this->initUint32($this->addressOf('_var_peripheral_8c1ba358'), $periph);

        $this->initUint32($this->addressOf('_var_pauseActive_8c1bb8cc'), $pauseActive);
        $this->initUint32($this->addressOf('_var_pauseSettle_8c18ad04'), $settle);
        $this->initUint32($this->addressOf('_var_onRetire_8c18ad10'), $onRetire);
        $this->initUint32($this->addressOf('_var_retirePhase_8c18ad08'), $retirePhase);
        $this->initUint32($this->addressOf('_var_confirmChoice_8c18ad0c'), $confirmChoice);
        $this->initUint32($this->addressOf('_var_activeCtrlType_8c157a70'), $ctrl);
        $this->initUint32($this->addressOf('_var_messageBoxActive_8c22847c'), 0);
        $this->initUint32($this->addressOf('_var_vibport_8c1ba354'), $vibport);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x1234);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), $isFading);
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), $playMode);
    }
};
