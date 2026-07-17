<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    /* ---------------- State 0: INIT ---------------- */

    public function test_init_waits_for_pvm_bool()
    {
        $this->resolveSymbols();
        $this->seedState(0);

        $this->shouldCall('_RouteLoadIsPvmReady_8c01432a')->andReturn(1);

        // Returns early, no draw / no selected write.
        $this->singleCall('_albumMenuTask_8c01d300')->with(0xbebacafe, 0)->run();
    }

    public function test_init_advances_to_fade_in()
    {
        $this->resolveSymbols();
        $this->seedState(0);

        $this->shouldCall('_RouteLoadIsPvmReady_8c01432a')->andReturn(0);
        $this->shouldCall('_AsqFreeQueues_8c011f7e');
        $this->shouldWriteLong($this->m(0x18), 1);
        $this->shouldCall('_SndProc_8c010cd6')->with(0, 0x10);
        $this->shouldCall('_push_fadein_8c022a9c')->with(10);

        $this->singleCall('_albumMenuTask_8c01d300')->with(0xbebacafe, 0)->run();
    }

    /* ---------------- State 1: FADE_IN ---------------- */

    public function test_fade_in_waits_while_fading()
    {
        $this->resolveSymbols();
        $this->seedState(1);
        $this->seedSlot(0);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);
        $this->noLetters();

        $this->shouldCall('_albumDrawGrid_8c01d290');
        $this->shouldWriteLong($this->m(0x38), 0);

        $this->singleCall('_albumMenuTask_8c01d300')->with(0xbebacafe, 0)->run();
    }

    public function test_fade_in_with_pending_goes_to_idle()
    {
        $this->resolveSymbols();
        $this->seedState(1);
        $this->seedSlot(0);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->noLetters();

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 1);

        $this->shouldWriteLong($this->m(0x18), 3);
        $this->shouldCall('_albumDrawGrid_8c01d290');
        $this->shouldWriteLong($this->m(0x38), 0);

        $this->singleCall('_albumMenuTask_8c01d300')->with($task, 0)->run();
    }

    public function test_fade_in_without_pending_goes_to_dialog()
    {
        $this->resolveSymbols();
        $this->seedState(1);
        $this->seedSlot(0);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->noLetters();

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 0);

        $this->shouldWriteLong($this->m(0x18), 2);
        $this->shouldCall('_albumDrawGrid_8c01d290');
        $this->shouldWriteLong($this->m(0x38), 0);

        $this->singleCall('_albumMenuTask_8c01d300')->with($task, 0)->run();
    }

    /* ---------------- State 2: DIALOG ---------------- */

    public function test_dialog_a_plays_blocked_sound()
    {
        $this->resolveSymbols();
        $this->seedState(2);
        $this->seedSlot(0);
        $this->seedPress(0x04); // TA
        $this->noLetters();

        $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 2, 0);
        $this->shouldCall('_albumDrawGrid_8c01d290');
        $this->shouldWriteLong($this->m(0x38), 0);

        $this->singleCall('_albumMenuTask_8c01d300')->with(0xbebacafe, 0)->run();
    }

    public function test_dialog_b_starts_fade_out()
    {
        $this->resolveSymbols();
        $this->seedState(2);
        $this->seedSlot(0);
        $this->seedPress(0x02); // TB
        $this->noLetters();

        $this->shouldWriteLong($this->m(0x18), 6);
        $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 1, 0);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(0);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(1);
        $this->shouldCall('_push_fadeout_8c022b60')->with(10);
        $this->shouldCall('_albumDrawGrid_8c01d290');
        $this->shouldWriteLong($this->m(0x38), 0);

        $this->singleCall('_albumMenuTask_8c01d300')->with(0xbebacafe, 0)->run();
    }

    /* ---------------- State 3: IDLE ---------------- */

    public function test_idle_no_input_draws_cursor_and_grid()
    {
        $this->resolveSymbols();
        $this->seedState(3);
        $this->seedSlot(0);
        $this->seedPress(0);
        $this->noLetters();
        $this->seedCursor(0.0, 0.0);

        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->m(0x0c), 0xd, 0.0, 0.0, -2.0);
        $this->shouldCall('_albumDrawGrid_8c01d290');
        $this->shouldWriteLong($this->m(0x38), 0);

        $this->singleCall('_albumMenuTask_8c01d300')->with(0xbebacafe, 0)->run();
    }

    public function test_idle_right_animates_to_next_slot()
    {
        $this->resolveSymbols();
        $this->seedState(3);
        $this->seedSlot(0);
        $this->seedPress(0x80); // KR
        $this->seedLetters([0, 1, 0, 0, 0, 0]);
        $this->seedCursor(0.0, 0.0);
        // init_8c045170[1] = (60, 66)
        $this->seedCursorTarget(1, 60.0, 66.0);

        $this->shouldWriteLong($this->m(0x18), 4);
        $this->shouldWriteFloat($this->m(0x28), 60.0);
        $this->shouldWriteFloat($this->m(0x2c), 66.0);
        $this->shouldWriteFloat($this->m(0x30), 10.0); // (60 - 0) / 6
        $this->shouldWriteFloat($this->m(0x34), 11.0); // (66 - 0) / 6
        $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 3, 0);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->m(0x0c), 0xd, 0.0, 0.0, -2.0);
        $this->shouldCall('_albumDrawGrid_8c01d290');
        $this->shouldWriteLong($this->m(0x38), 1);

        $this->singleCall('_albumMenuTask_8c01d300')->with(0xbebacafe, 0)->run();
    }

    public function test_idle_down_picks_closest_letter_tie_lowest()
    {
        $this->resolveSymbols();
        $this->seedState(3);
        $this->seedSlot(1);
        $this->seedPress(0x20); // KD
        // Bottom row letters at slots 3 and 5; both 1 away from target col 4 -> pick 3.
        $this->seedLetters([0, 0, 0, 1, 0, 1]);
        $this->seedCursor(0.0, 0.0);
        // init_8c045170[3] = (300, 180)
        $this->seedCursorTarget(3, 300.0, 180.0);

        $this->shouldWriteLong($this->m(0x18), 4);
        $this->shouldWriteFloat($this->m(0x28), 300.0);
        $this->shouldWriteFloat($this->m(0x2c), 180.0);
        $this->shouldWriteFloat($this->m(0x30), 50.0); // 300 / 6
        $this->shouldWriteFloat($this->m(0x34), 30.0); // 180 / 6
        $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 3, 0);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->m(0x0c), 0xd, 0.0, 0.0, -2.0);
        $this->shouldCall('_albumDrawGrid_8c01d290');
        $this->shouldWriteLong($this->m(0x38), 3);

        $this->singleCall('_albumMenuTask_8c01d300')->with(0xbebacafe, 0)->run();
    }

    public function test_idle_up_picks_closest_letter_tie_lowest()
    {
        $this->resolveSymbols();
        $this->seedState(3);
        $this->seedSlot(4);
        $this->seedPress(0x10); // KU
        // Top row letters at slots 0 and 2; both 1 away from target col 1 -> pick 0.
        $this->seedLetters([1, 0, 1, 0, 1, 0]);
        $this->seedCursor(0.0, 0.0);
        // init_8c045170[0] = (6, 12)
        $this->seedCursorTarget(0, 6.0, 12.0);

        $this->shouldWriteLong($this->m(0x18), 4);
        $this->shouldWriteFloat($this->m(0x28), 6.0);
        $this->shouldWriteFloat($this->m(0x2c), 12.0);
        $this->shouldWriteFloat($this->m(0x30), 1.0); // 6 / 6
        $this->shouldWriteFloat($this->m(0x34), 2.0); // 12 / 6
        $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 3, 0);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->m(0x0c), 0xd, 0.0, 0.0, -2.0);
        $this->shouldCall('_albumDrawGrid_8c01d290');
        $this->shouldWriteLong($this->m(0x38), 0);

        $this->singleCall('_albumMenuTask_8c01d300')->with(0xbebacafe, 0)->run();
    }

    public function test_idle_a_opens_letter()
    {
        $this->resolveSymbols();
        $this->seedState(3);
        $this->seedSlot(2);
        $this->seedPress(0x04); // TA
        $this->seedCursor(0.0, 0.0);

        // state -> VIEWING (5): albumDrawGrid_8c01d290 is still called but draws nothing.
        $this->shouldWriteLong($this->m(0x18), 5);
        $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 0, 0);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->m(0x0c), 0xd, 0.0, 0.0, -2.0);
        $this->shouldCall('_albumDrawGrid_8c01d290');
        $this->shouldWriteLong($this->m(0x38), 2);

        $this->singleCall('_albumMenuTask_8c01d300')->with(0xbebacafe, 0)->run();
    }

    public function test_idle_b_leaves_album()
    {
        $this->resolveSymbols();
        $this->seedState(3);
        $this->seedSlot(0);
        $this->seedPress(0x02); // TB
        $this->noLetters();
        $this->seedCursor(0.0, 0.0);

        $this->shouldWriteLong($this->m(0x18), 6);
        $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 1, 0);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(0);
        $this->shouldCall('_SndStartAdxFadeOut_8c010bae')->with(1);
        $this->shouldCall('_push_fadeout_8c022b60')->with(10);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->m(0x0c), 0xd, 0.0, 0.0, -2.0);
        $this->shouldCall('_albumDrawGrid_8c01d290');
        $this->shouldWriteLong($this->m(0x38), 0);

        $this->singleCall('_albumMenuTask_8c01d300')->with(0xbebacafe, 0)->run();
    }

    /* ---------------- State 4: ANIMATING ---------------- */

    public function test_animating_still_running()
    {
        $this->resolveSymbols();
        $this->seedState(4);
        $this->seedSlot(1);
        $this->noLetters();
        $this->seedCursor(0.0, 0.0);

        $this->shouldCall('_CourseMenuInterpolateCursor_8c016d2c')->andReturn(0);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->m(0x0c), 0xd, 0.0, 0.0, -2.0);
        $this->shouldCall('_albumDrawGrid_8c01d290');
        $this->shouldWriteLong($this->m(0x38), 1);

        $this->singleCall('_albumMenuTask_8c01d300')->with(0xbebacafe, 0)->run();
    }

    public function test_animating_reaches_target_returns_to_idle()
    {
        $this->resolveSymbols();
        $this->seedState(4);
        $this->seedSlot(1);
        $this->noLetters();
        $this->seedCursor(0.0, 0.0);

        $this->shouldCall('_CourseMenuInterpolateCursor_8c016d2c')->andReturn(1);
        $this->shouldWriteLong($this->m(0x18), 3);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->m(0x0c), 0xd, 0.0, 0.0, -2.0);
        $this->shouldCall('_albumDrawGrid_8c01d290');
        $this->shouldWriteLong($this->m(0x38), 1);

        $this->singleCall('_albumMenuTask_8c01d300')->with(0xbebacafe, 0)->run();
    }

    /* ---------------- State 5: VIEWING ---------------- */

    public function test_viewing_draws_letter()
    {
        $this->resolveSymbols();
        $this->seedState(5);
        $this->seedSlot(2);
        $this->seedPress(0);

        // albumDrawGrid_8c01d290 is still called but skips the grid in state 5.
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->m(0x0c), 2 + 7, 0.0, 0.0, -3.0);
        $this->shouldCall('_albumDrawGrid_8c01d290');
        $this->shouldWriteLong($this->m(0x38), 2);

        $this->singleCall('_albumMenuTask_8c01d300')->with(0xbebacafe, 0)->run();
    }

    public function test_viewing_a_returns_to_idle_and_draws_grid()
    {
        $this->resolveSymbols();
        $this->seedState(5);
        $this->seedSlot(2);
        $this->seedPress(0x04); // TA
        $this->seedLetters([1, 0, 0, 0, 0, 0]);

        // state -> IDLE (3) before the trailing grid draw, so the grid IS drawn.
        $this->shouldWriteLong($this->m(0x18), 3);
        $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 0, 0);
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($this->m(0x0c), 2 + 7, 0.0, 0.0, -3.0);
        $this->shouldCall('_albumDrawGrid_8c01d290');
        $this->shouldWriteLong($this->m(0x38), 2);

        $this->singleCall('_albumMenuTask_8c01d300')->with(0xbebacafe, 0)->run();
    }

    /* ---------------- State 6: FADE_OUT ---------------- */

    public function test_fade_out_waits_while_fading()
    {
        $this->resolveSymbols();
        $this->seedState(6);
        $this->seedSlot(0);
        $this->noLetters();
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->shouldCall('_albumDrawGrid_8c01d290');
        $this->shouldWriteLong($this->m(0x38), 0);

        $this->singleCall('_albumMenuTask_8c01d300')->with(0xbebacafe, 0)->run();
    }

    public function test_fade_out_waits_for_resource_release()
    {
        $this->resolveSymbols();
        $this->seedState(6);
        $this->seedSlot(0);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 1);

        // Returns early: no draw / no selected write.
        $this->singleCall('_albumMenuTask_8c01d300')->with(0xbebacafe, 0)->run();
    }

    public function test_fade_out_completes_and_switches_screen()
    {
        $this->resolveSymbols();
        $this->seedState(6);
        $this->seedSlot(0);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0);

        $this->shouldWriteLong($this->m(0x3c), 1);
        $this->shouldWriteLong($this->m(0x40), 1);
        $this->shouldWriteFloat($this->m(0x20), 0.0);
        $this->shouldCall('_FUN_8c016182');
        $this->shouldCall('_CourseMenuFUN_8c017ef2');

        $this->singleCall('_albumMenuTask_8c01d300')->with(0xbebacafe, 0)->run();
    }

    /* ---------------- Default ---------------- */

    public function test_unknown_state_just_draws()
    {
        $this->resolveSymbols();
        $this->seedState(7);
        $this->seedSlot(0);
        $this->noLetters();

        $this->shouldCall('_albumDrawGrid_8c01d290');
        $this->shouldWriteLong($this->m(0x38), 0);

        $this->singleCall('_albumMenuTask_8c01d300')->with(0xbebacafe, 0)->run();
    }

    /* ---------------- Helpers ---------------- */

    private function m(int $offset): int
    {
        return $this->addressOf('_var_menuState_8c1bc7a8') + $offset;
    }

    private function seedState(int $state): void
    {
        $this->initUint32($this->m(0x18), $state);
    }

    private function seedSlot(int $slot): void
    {
        $this->initUint32($this->m(0x38), $slot);
    }

    private function seedPress(int $press): void
    {
        $this->initUint32($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, $press);
    }

    private function seedCursor(float $x, float $y): void
    {
        $this->initUint32($this->m(0x20), fdec($x));
        $this->initUint32($this->m(0x24), fdec($y));
    }

    private function seedCursorTarget(int $slot, float $x, float $y): void
    {
        $base = $this->addressOf('_init_8c045170') + $slot * 8;
        $this->initUint32($base + 0, fdec($x));
        $this->initUint32($base + 4, fdec($y));
    }

    private function seedLetters(array $letters): void
    {
        $base = $this->addressOf('_var_progress_8c1ba1cc') + 0x2c;
        foreach ($letters as $i => $value) {
            $this->initUint32($base + $i * 4, $value);
        }
    }

    private function noLetters(): void
    {
        $this->seedLetters([0, 0, 0, 0, 0, 0]);
    }

    private function expectGrid(array $letters): void
    {
        $rg = $this->m(0x0c);
        foreach ($letters as $i => $value) {
            if ($value) {
                $this->shouldCall('_TxtDrawSprite_8c014f54')->with($rg, $i + 1, 0.0, 0.0, -4.0);
            }
        }
        $this->shouldCall('_TxtDrawSprite_8c014f54')->with($rg, 0, 0.0, 0.0, -5.0);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_var_progress_8c1ba1cc', 0x94);
        $this->setSize('_var_peripherals_8c1ba35c', 52 * 2);
        $this->setSize('_init_8c045170', 0x30);
        $this->setSize('_var_isFading_8c226568', 4);
        $this->setSize('_init_8c03bd80', 4);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0xbeef0000);

        // Functions
        $this->setSize('_TxtDrawSprite_8c014f54', 0x4);
        $this->setSize('_sdMidiPlay', 0x4);
        $this->setSize('_RouteLoadIsPvmReady_8c01432a', 0x4);
        $this->setSize('_AsqFreeQueues_8c011f7e', 0x4);
        $this->setSize('_SndProc_8c010cd6', 0x4);
        $this->setSize('_push_fadein_8c022a9c', 0x4);
        $this->setSize('_push_fadeout_8c022b60', 0x4);
        $this->setSize('_SndStartAdxFadeOut_8c010bae', 0x4);
        $this->setSize('_FUN_8c016182', 0x4);
        $this->setSize('_CourseMenuInterpolateCursor_8c016d2c', 0x4);
        $this->setSize('_CourseMenuFUN_8c017ef2', 0x4);
    }
};
