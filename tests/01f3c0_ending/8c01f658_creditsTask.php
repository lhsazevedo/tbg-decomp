<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * creditsTask_8c01f658(void): per-frame ending state machine (TaskInstall
 * callback). See ENDING_TASK_STATE in the unit for the 8 states.
 */
return new class extends TestCase {
    const MENU_STATE_SIZE = 0x7c;
    const STATE_0X18 = 0x18;
    const FIELD_0X1C = 0x1c;
    const BUS_X_0X20 = 0x20;
    const FLAG_Y_0X24 = 0x24;
    const VELOCITY_X_0X30 = 0x30;
    const VELOCITY_Y_0X34 = 0x34;
    const FIELD_0X54 = 0x54;
    const FIELD_0X58 = 0x58;
    const FIELD_0X5C = 0x5c;
    const STARTTIMER_0X64 = 0x64;
    const LOGOTIMER_0X68 = 0x68;

    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', self::MENU_STATE_SIZE);
        $this->setSize('_var_dialogQueue_8c225fbc', 0x10);
        $this->setSize('_var_isFading_8c226568', 4);
        $this->setSize('_var_instructorDialogActive_8c225fb4', 4);
        $this->setSize('_init_adxPlaying_8c03bd80', 4);
        $this->setSize('_var_selectedVm_8c1ba34c', 4);
        $this->setSize('_var_messageTextBoxA_8c1bc404', 4);
        $this->setSize('_var_messageTextBoxB_8c1bc408', 4);
        $this->setSize('_RouteGetLatch_8c01432a', 4);
        $this->setSize('_AsqFreeQueues_8c011f7e', 4);
        $this->setSize('_SndPlayAdx_8c010cd6', 4);
        $this->setSize('_RenderStartFadeIn_8c022a9c', 4);
        $this->setSize('_CourseMenuInstallDialog_8c0170c6', 4);
        $this->setSize('_SndStartAdxFadeOut_8c010bae', 4);
        $this->setSize('_RenderStartFadeOut_8c022b60', 4);
        $this->setSize('_MessageBoxFreeTextboxes_8c02af32', 4);
        $this->setSize('_TxtInit_8c01524c', 4);
        $this->setSize('_TxtCreateTextBox_8c0152fc', 4);
        $this->setSize('_TxtPrepareTextBoxLayout_8c01543a', 4);
        $this->setSize('_ReplayMenuFreeSessionAssets_8c016182', 4);
        $this->setSize('_TitleStart_8c015fd6', 4);
        $this->setSize('_ResultsShowFailedRun_8c01e24e', 4);
    }

    private function setState(int $state): int
    {
        $base = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($base + self::STATE_0X18, $state);
        return $base;
    }

    // Empirically, creditsTask_8c01f658's own TxtCreateTextBox_8c0152fc call
    // sites push x2 last (closest to sp), then y2, then enable_offset
    // first -- the opposite order from 8c02ae3e_MessageBoxOpenTextbox.php's
    // helper, which checks a different caller's stack layout.
    private function stackArgsCheck(int $x2, int $y2, int $enableOffset): callable
    {
        $mask = 0xffffffff;
        $x2 &= $mask; $y2 &= $mask; $enableOffset &= $mask;
        return function () use ($x2, $y2, $enableOffset) {
            $sp = $this->registers[15]->value;
            $a = $this->memory->readUInt32($sp + 0)->value;
            $b = $this->memory->readUInt32($sp + 4)->value;
            $c = $this->memory->readUInt32($sp + 8)->value;
            if ($a !== $x2 || $b !== $y2 || $c !== $enableOffset) {
                throw new \Exception(sprintf(
                    "Unexpected stack args: got [%d, %d, %d], expecting x2=%d, y2=%d, enable_offset=%d",
                    $a, $b, $c, $x2, $y2, $enableOffset
                ));
            }
        };
    }

    // --- state 0: WAIT_PVM ---

    public function test_state0_pvm_not_ready_does_nothing(): void
    {
        $this->resolveSymbols();
        $this->setState(0);

        $this->call('_creditsTask_8c01f658');

        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(1);
    }

    public function test_state0_pvm_ready_starts_fade_in(): void
    {
        $this->resolveSymbols();
        $base = $this->setState(0);

        $this->call('_creditsTask_8c01f658');

        $this->shouldCall('_RouteGetLatch_8c01432a')->andReturn(0);
        $this->shouldCall('_AsqFreeQueues_8c011f7e');
        $this->shouldWriteLong($base + self::STATE_0X18, 1);
        $this->shouldWriteLong($base + self::FIELD_0X1C, 0);
        $this->shouldWriteFloat($base + self::BUS_X_0X20, 45.0);
        $this->shouldWriteFloat($base + self::FLAG_Y_0X24, 100.0);
        $this->shouldWriteFloat($base + self::VELOCITY_X_0X30, 1.0);
        $this->shouldWriteFloat($base + self::VELOCITY_Y_0X34, 0.0);
        $this->shouldCall('_SndPlayAdx_8c010cd6')->with(0, 1);
        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    // --- state 1: FADE_IN ---

    public function test_state1_still_fading_only_updates_overlay(): void
    {
        $this->resolveSymbols();
        $this->setState(1);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_creditsTask_8c01f658');

        $this->shouldCall('_updateDialogScene_8c01f42c');
    }

    public function test_state1_fade_done_pushes_dialog_and_advances(): void
    {
        $this->resolveSymbols();
        $base = $this->setState(1);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_var_dialogQueue_8c225fbc'), 2);
        $dialogList = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_endingVoiceList_8c226430'), $dialogList);

        $this->call('_creditsTask_8c01f658');

        $this->shouldCall('_CourseMenuInstallDialog_8c0170c6')->with(2, $dialogList);
        $this->shouldWriteLong($base + self::STATE_0X18, 2);
        $this->shouldCall('_updateDialogScene_8c01f42c');
    }

    // --- state 2: INSTRUCTOR_DIALOG ---

    public function test_state2_dialog_active_only_updates_overlay(): void
    {
        $this->resolveSymbols();
        $this->setState(2);
        $this->initUint32($this->addressOf('_var_instructorDialogActive_8c225fb4'), 1);

        $this->call('_creditsTask_8c01f658');

        $this->shouldCall('_updateDialogScene_8c01f42c');
    }

    public function test_state2_dialog_done_fades_out(): void
    {
        $this->resolveSymbols();
        $base = $this->setState(2);
        $this->initUint32($this->addressOf('_var_instructorDialogActive_8c225fb4'), 0);

        $this->call('_creditsTask_8c01f658');

        $this->shouldWriteLong($base + self::STATE_0X18, 3);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(0);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(1);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(30);
        $this->shouldCall('_updateDialogScene_8c01f42c');
    }

    // --- state 3: FADE_OUT_TO_CREDITS ---

    public function test_state3_still_fading_only_updates_overlay(): void
    {
        $this->resolveSymbols();
        $this->setState(3);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_creditsTask_8c01f658');

        $this->shouldCall('_updateDialogScene_8c01f42c');
    }

    public function test_state3_sound_module_busy_does_nothing(): void
    {
        $this->resolveSymbols();
        $this->setState(3);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_adxPlaying_8c03bd80'), 1);

        $this->call('_creditsTask_8c01f658');
    }

    public function test_state3_opens_credits_textboxes(): void
    {
        $this->resolveSymbols();
        $base = $this->setState(3);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_adxPlaying_8c03bd80'), 0);
        $box0 = $this->alloc(4);
        $box1 = $this->alloc(4);

        $this->call('_creditsTask_8c01f658');

        $this->shouldWriteLong($base + self::STATE_0X18, 4);
        $this->shouldWriteLong($base + self::LOGOTIMER_0X68, 0);
        $this->shouldCall('_MessageBoxFreeTextboxes_8c02af32');
        $this->shouldCall('_TxtInit_8c01524c');

        $this->shouldCall('_TxtCreateTextBox_8c0152fc')
            ->with(0, 480, -5.0, 640, 480)
            ->do($this->stackArgsCheck(0, 0, -1))
            ->andReturn($box0);
        $this->shouldWriteLongTo('_var_messageTextBoxA_8c1bc404', $box0);

        $this->shouldCall('_TxtCreateTextBox_8c0152fc')
            ->with(0, 960, -5.0, 640, 480)
            ->do($this->stackArgsCheck(0, 0, -1))
            ->andReturn($box1);
        $this->shouldWriteLongTo('_var_messageTextBoxB_8c1bc408', $box1);

        // The credit text pointers are compile-time constants, but not
        // expressible as ASCII PHP literals (Shift-JIS) or reachable via
        // addressOf (anonymous string literals have no symbol) -- read them
        // back from the array at call time instead.
        $credits0Addr = $this->addressOf('_init_endingCreditsHead_8c04528c');
        $credits1Addr = $this->addressOf('_init_endingCreditsTail_8c045290');
        $this->shouldCall('_TxtPrepareTextBoxLayout_8c01543a')
            ->do(function () use ($box0, $credits0Addr) {
                $r5 = $this->registers[5]->value;
                $expected = $this->memory->readUInt32($credits0Addr)->value;
                if ($this->registers[4]->value !== $box0 || $r5 !== $expected) {
                    throw new \Exception("Unexpected TxtPrepareTextBoxLayout args for box0");
                }
            })
            ->andReturn(1);
        $this->shouldCall('_TxtPrepareTextBoxLayout_8c01543a')
            ->do(function () use ($box1, $credits1Addr) {
                $r5 = $this->registers[5]->value;
                $expected = $this->memory->readUInt32($credits1Addr)->value;
                if ($this->registers[4]->value !== $box1 || $r5 !== $expected) {
                    throw new \Exception("Unexpected TxtPrepareTextBoxLayout args for box1");
                }
            })
            ->andReturn(1);

        $this->shouldWriteLong($base + self::FIELD_0X1C, 0);
        $this->shouldWriteLong($base + self::FIELD_0X5C, 0);
        $this->shouldWriteLong($base + self::STARTTIMER_0X64, 2);
        $this->shouldWriteLong($base + self::FIELD_0X54, 0);
        $this->shouldWriteLong($base + self::FIELD_0X58, 0);
        $this->shouldWriteLong($base + self::LOGOTIMER_0X68, 3240);
        $this->shouldCall('_SndPlayAdx_8c010cd6')->with(0, 12);
        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(30);
    }

    // --- state 4: CREDITS_INTRO ---

    public function test_state4_still_fading_stays_and_scrolls(): void
    {
        $this->resolveSymbols();
        $base = $this->setState(4);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);
        $this->initUint32($base + self::LOGOTIMER_0X68, 10);

        $this->call('_creditsTask_8c01f658');

        $this->shouldCall('_scrollCreditsText_8c01f50e')->andReturn(0);
        $this->shouldWriteLong($base + self::LOGOTIMER_0X68, 9);
    }

    public function test_state4_fade_done_advances_to_scroll(): void
    {
        $this->resolveSymbols();
        $base = $this->setState(4);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($base + self::LOGOTIMER_0X68, 10);

        $this->call('_creditsTask_8c01f658');

        $this->shouldWriteLong($base + self::STATE_0X18, 5);
        $this->shouldCall('_scrollCreditsText_8c01f50e')->andReturn(0);
        $this->shouldWriteLong($base + self::LOGOTIMER_0X68, 9);
    }

    // --- state 5: CREDITS_SCROLL ---

    public function test_state5_normal_frame_just_scrolls(): void
    {
        $this->resolveSymbols();
        $base = $this->setState(5);
        $this->initUint32($base + self::LOGOTIMER_0X68, 10);
        $this->initUint32($base + self::FIELD_0X1C, 0);

        $this->call('_creditsTask_8c01f658');

        $this->shouldWriteLong($base + self::LOGOTIMER_0X68, 9);
        $this->shouldCall('_scrollCreditsText_8c01f50e')->andReturn(0);
    }

    public function test_state5_timer_expires_pings_sound_and_resets(): void
    {
        $this->resolveSymbols();
        $base = $this->setState(5);
        $this->initUint32($base + self::LOGOTIMER_0X68, 0);
        $this->initUint32($base + self::FIELD_0X1C, 0);

        $this->call('_creditsTask_8c01f658');

        $this->shouldWriteLong($base + self::LOGOTIMER_0X68, 0xffffffff);
        $this->shouldCall('_SndPlayAdx_8c010cd6')->with(0, 11);
        $this->shouldWriteLong($base + self::LOGOTIMER_0X68, 2700);
        $this->shouldCall('_scrollCreditsText_8c01f50e')->andReturn(0);
    }

    public function test_state5_credits_done_advances_to_hold(): void
    {
        $this->resolveSymbols();
        $base = $this->setState(5);
        $this->initUint32($base + self::LOGOTIMER_0X68, 10);
        $this->initUint32($base + self::FIELD_0X1C, 1);

        $this->call('_creditsTask_8c01f658');

        $this->shouldWriteLong($base + self::LOGOTIMER_0X68, 9);
        $this->shouldWriteLong($base + self::STATE_0X18, 6);
        $this->shouldWriteLong($base + self::LOGOTIMER_0X68, 0);
        $this->shouldCall('_scrollCreditsText_8c01f50e')->andReturn(0);
    }

    // --- state 6: CREDITS_HOLD ---

    public function test_state6_still_holding_just_scrolls(): void
    {
        $this->resolveSymbols();
        $base = $this->setState(6);
        $this->initUint32($base + self::LOGOTIMER_0X68, 10);

        $this->call('_creditsTask_8c01f658');

        $this->shouldWriteLong($base + self::LOGOTIMER_0X68, 11);
        $this->shouldCall('_scrollCreditsText_8c01f50e')->andReturn(0);
    }

    public function test_state6_hold_expires_fades_out(): void
    {
        $this->resolveSymbols();
        $base = $this->setState(6);
        $this->initUint32($base + self::LOGOTIMER_0X68, 150);

        $this->call('_creditsTask_8c01f658');

        $this->shouldWriteLong($base + self::LOGOTIMER_0X68, 151);
        $this->shouldWriteLong($base + self::STATE_0X18, 7);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(0);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(1);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(60);
        $this->shouldCall('_scrollCreditsText_8c01f50e')->andReturn(0);
    }

    // --- state 7: EXIT ---

    public function test_state7_still_fading_only_scrolls(): void
    {
        $this->resolveSymbols();
        $this->setState(7);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_creditsTask_8c01f658');

        $this->shouldCall('_scrollCreditsText_8c01f50e')->andReturn(0);
    }

    public function test_state7_sound_module_busy_does_nothing(): void
    {
        $this->resolveSymbols();
        $this->setState(7);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_adxPlaying_8c03bd80'), 1);

        $this->call('_creditsTask_8c01f658');
    }

    public function test_state7_no_vm_selected_returns_to_title(): void
    {
        $this->resolveSymbols();
        $this->setState(7);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_adxPlaying_8c03bd80'), 0);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0xffffffff);

        $this->call('_creditsTask_8c01f658');

        $this->shouldCall('_ReplayMenuFreeSessionAssets_8c016182');
        $this->shouldCall('_TitleStart_8c015fd6')->with(0);
    }

    public function test_state7_vm_selected_shows_failed_run(): void
    {
        $this->resolveSymbols();
        $this->setState(7);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_adxPlaying_8c03bd80'), 0);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);

        $this->call('_creditsTask_8c01f658');

        $this->shouldCall('_ReplayMenuFreeSessionAssets_8c016182');
        $this->shouldCall('_ResultsShowFailedRun_8c01e24e');
    }
};
