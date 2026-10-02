<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    const PRESS = 0x10;      // var_peripherals_8c1ba35c[0].press offset
    const STATE = 0x18;      // var_menuState_8c1bc7a8.state_0x18 offset
    const SELECTED = 0x38;   // .selected_0x38
    const FIELD70 = 0x70;    // .returnAction_0x70 (pending task ptr)
    const FIELD74 = 0x74;    // .returnActionArg_0x74 (pending task arg)
    const TA = 1 << 2;       // confirm
    const TB = 1 << 1;       // cancel
    const KU = 1 << 4;       // prev row
    const KD = 1 << 5;       // next row
    const MIDI = 0xd1d1d1d1;

    private int $task = 0;

    private function menu(int $off): int
    {
        return $this->addressOf('_var_menuState_8c1bc7a8') + $off;
    }

    private function arrange(int $press, int $state, int $selected,
                             int $isFading = 0, int $field70 = 0, int $field74 = 0): void
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_var_isFading_8c226568', 4);
        $this->setSize('_SpriteDraw_8c014f54', 4);
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_RenderStartFadeOut_8c022b60', 4);
        // _MainMenuEnter_8c01a09a and the three switch-in wrappers are
        // auto-allocated via the init_topMenuActions_8c044e28 dispatch table's relocations.

        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + self::PRESS, $press);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), self::MIDI);
        $this->initUint32($this->menu(self::STATE), $state);
        $this->initUint32($this->menu(self::SELECTED), $selected);
        $this->initUint32($this->menu(self::FIELD70), $field70);
        $this->initUint32($this->menu(self::FIELD74), $field74);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), $isFading);

        $this->task = $this->alloc(0x20);
        $this->call('_topMenuTask_8c01b00a')->with($this->task);
    }

    /* Highlight sprite for the selected row (selected+0x1a), the row-label sheet
     * (0x19), and the frame overlay (group A, sprite 0). */
    private function shouldDraw(int $selected): void
    {
        $gB = $this->menu(0x0c);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($gB, $selected + 0x1a, 0.0, 0.0, -4.0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($gB, 0x19, 0.0, 0.0, -5.0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with($this->menu(0x00), 0, 0.0, 0.0, -7.0);
    }

    public function test_phase0_waits_while_fading()
    {
        $this->arrange(press: 0, state: 0, selected: 0, isFading: 1);
        $this->shouldDraw(0);
    }

    public function test_phase0_advances_when_fade_done()
    {
        $this->arrange(press: 0, state: 0, selected: 0, isFading: 0);
        $this->shouldWriteLong($this->menu(self::STATE), 1);
        $this->shouldDraw(0);
    }

    public function test_navigate_next_row()
    {
        $this->arrange(press: self::KD, state: 1, selected: 0);
        $this->shouldWriteLong($this->menu(self::SELECTED), 1);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldDraw(1);
    }

    public function test_navigate_next_wraps_to_setting()
    {
        $this->arrange(press: self::KD, state: 1, selected: 3);
        $this->shouldWriteLong($this->menu(self::SELECTED), 4);
        $this->shouldWriteLong($this->menu(self::SELECTED), 0);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldDraw(0);
    }

    public function test_navigate_prev_row()
    {
        $this->arrange(press: self::KU, state: 1, selected: 1);
        $this->shouldWriteLong($this->menu(self::SELECTED), 0);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldDraw(0);
    }

    public function test_navigate_prev_wraps_to_return()
    {
        $this->arrange(press: self::KU, state: 1, selected: 0);
        $this->shouldWriteLong($this->menu(self::SELECTED), -1);
        $this->shouldWriteLong($this->menu(self::SELECTED), 3);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 3, 0);
        $this->shouldDraw(3);
    }

    public function test_confirm_setting_row()
    {
        $this->arrange(press: self::TA, state: 1, selected: 0);
        $this->shouldWriteLong($this->menu(self::STATE), 2);
        $this->shouldWriteLong($this->menu(self::FIELD70), $this->addressOf('_enterSetting_8c01a3c0'));
        $this->shouldWriteLong($this->menu(self::FIELD74), 2);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 0, 0);
        $this->shouldDraw(0);
    }

    public function test_confirm_audio_row()
    {
        $this->arrange(press: self::TA, state: 1, selected: 2);
        $this->shouldWriteLong($this->menu(self::STATE), 2);
        $this->shouldWriteLong($this->menu(self::FIELD70), $this->addressOf('_enterAudio_8c01afd8'));
        $this->shouldWriteLong($this->menu(self::FIELD74), 2);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 0, 0);
        $this->shouldDraw(2);
    }

    public function test_confirm_return_row()
    {
        // RETURN is table[3] = MainMenuEnter; still a confirm (sound 0).
        $this->arrange(press: self::TA, state: 1, selected: 3);
        $this->shouldWriteLong($this->menu(self::STATE), 2);
        $this->shouldWriteLong($this->menu(self::FIELD70), $this->addressOf('_MainMenuEnter_8c01a09a'));
        $this->shouldWriteLong($this->menu(self::FIELD74), 2);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 0, 0);
        $this->shouldDraw(3);
    }

    public function test_cancel_returns_to_main_menu()
    {
        $this->arrange(press: self::TB, state: 1, selected: 1);
        $this->shouldWriteLong($this->menu(self::STATE), 2);
        $this->shouldWriteLong($this->menu(self::FIELD70), $this->addressOf('_MainMenuEnter_8c01a09a'));
        $this->shouldWriteLong($this->menu(self::FIELD74), 2);
        $this->shouldCall('_RenderStartFadeOut_8c022b60')->with(10);
        $this->shouldCall('_sdMidiPlay')->with(self::MIDI, 1, 1, 0);
        $this->shouldDraw(1);
    }

    public function test_phase2_hands_off_when_fade_done()
    {
        // returnAction_0x70 holds the pending task (here MainMenu); called with (task, arg=2).
        $this->arrange(press: 0, state: 2, selected: 3, isFading: 0,
                       field70: $this->addressOf('_MainMenuEnter_8c01a09a'), field74: 2);
        $this->shouldCall('_MainMenuEnter_8c01a09a')->with($this->task, 2);
    }

    public function test_phase2_keeps_fading()
    {
        $this->arrange(press: 0, state: 2, selected: 1, isFading: 1);
        $this->shouldDraw(1);
    }
};
