<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;

return new class extends TestCase {
    public function test_init_state_waits_for_ukn_pvm_bool(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 0);

        $this->call('_CourseConfirmMenuTask_8c0181b6');

        $this->shouldCall('_getUknPvmBool_8c01432a')->andReturn(1);
    }

    public function test_init_state_advances(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 0);

        $this->call('_CourseConfirmMenuTask_8c0181b6');

        $this->shouldCall('_getUknPvmBool_8c01432a')->andReturn(0);
        $this->shouldCall('_AsqFreeQueues_11f7e');
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 1);
        $this->shouldCall('_push_fadein_8c022a9c')->with(10);
        $this->shouldCall('_snd_8c010cd6')->with(0, 15);
    }

    public function test_fade_in_state_advances_when_fade_complete(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 1);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);
        // field10_0x38 used for the second draw (id = field10_0x38 + 2)
        $this->initMenuStateUint32(0x38, 0);
        // field_0x50 influences the first draw sprite index (id = field_0x50 / 3)
        $this->initMenuStateUint32(0x50, 12); // expect 12/3 = 4

        $this->call('_CourseConfirmMenuTask_8c0181b6');

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2);
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            4,
            0.0,
            0.0,
            -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            2,
            376.0,
            378.0,
            -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0,
            0.0,
            0.0,
            -7.0
        );
    }

    public function test_fade_in_state_waits_for_fade(): void
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 1);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);
        $this->initMenuStateUint32(0x38, 0); // selected_0x38
        $this->initMenuStateUint32(0x50, 12); // field_0x50

        $this->call('_CourseConfirmMenuTask_8c0181b6');

        // Still fading ¨ remain in state 1 and render epilogue
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            4,
            0.0,
            0.0,
            -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            2,
            376.0,
            378.0,
            -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0,
            0.0,
            0.0,
            -7.0
        );
    }

    public function test_dialog_state_advances_on_yes_and_fadeout(): void
    {
        $this->resolveSymbols();

        // Enter state 2 (dialog/prompt)
        $this->initMenuStateUint32(0x18, 2);
        $this->initMenuStateUint32(0x38, 0); // selected_0x38
        $this->initMenuStateUint32(0x50, 12); // field_0x50 ¨ preview sprite 12/3 = 4

        $this->call('_CourseConfirmMenuTask_8c0181b6');

        // Prompt returns YES (1) ¨ advance to state 3 and fade out
        $this->shouldCall('_promptHandleBinary_16caa')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x38
        )->andReturn(1);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 3);
        $this->shouldCall('_push_fadeout_8c022b60')->with(10);

        // Epilogue draws
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            4,
            0.0,
            0.0,
            -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            2,
            376.0,
            378.0,
            -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0,
            0.0,
            0.0,
            -7.0
        );
    }

    public function test_dialog_state_on_no_go_to_main_menu_and_fadeout(): void
    {
        $this->resolveSymbols();

        // Enter state 2 (dialog/prompt)
        $this->initMenuStateUint32(0x18, 2);
        $this->initMenuStateUint32(0x38, 0); // selected_0x38
        $this->initMenuStateUint32(0x50, 12); // field_0x50 ¨ preview sprite 12/3 = 4

        $this->call('_CourseConfirmMenuTask_8c0181b6');

        // Prompt returns NO/Cancel (2) ¨ set state 7, do two FUN_8c010bae calls, then fade out
        $this->shouldCall('_promptHandleBinary_16caa')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x38
        )->andReturn(2);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 7);
        $this->shouldCall('_FUN_8c010bae')->with(0);
        $this->shouldCall('_FUN_8c010bae')->with(1);
        $this->shouldCall('_push_fadeout_8c022b60')->with(10);

        // Epilogue draws
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            4,
            0.0,
            0.0,
            -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            2,
            376.0,
            378.0,
            -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0,
            0.0,
            0.0,
            -7.0
        );
    }

    public function test_dialog_state_waits_when_prompt_returns_0(): void
    {
        $this->resolveSymbols();

        // Enter state 2 (dialog/prompt)
        $this->initMenuStateUint32(0x18, 2);
        $this->initMenuStateUint32(0x38, 0); // selected_0x38
        $this->initMenuStateUint32(0x50, 12); // field_0x50 ¨ preview sprite 12/3 = 4

        $this->call('_CourseConfirmMenuTask_8c0181b6');

        // Prompt returns 0 ¨ stay in state 2; no fade out; just epilogue draws
        $this->shouldCall('_promptHandleBinary_16caa')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x38
        )->andReturn(0);

        // Epilogue draws
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            4,
            0.0,
            0.0,
            -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            2,
            376.0,
            378.0,
            -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0,
            0.0,
            0.0,
            -7.0
        );
    }

    public function test_idle_state_advances_to_animating_when_fade_complete(): void
    {
        $this->resolveSymbols();

        // Enter state 3 (idle)
        $this->initMenuStateUint32(0x18, 3);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);
        $this->initMenuStateUint32(0x38, 0); // selected_0x38
        $this->initMenuStateUint32(0x50, 12); // field_0x50 ¨ preview sprite 12/3 = 4

        $this->call('_CourseConfirmMenuTask_8c0181b6');

        // When fade finished ¨ state=4 and push fade-in(0x14)
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 4);
        $this->shouldCall('_push_fadein_8c022a9c')->with(20);

        // Epilogue draws
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            4,
            0.0,
            0.0,
            -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            2,
            376.0,
            378.0,
            -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0,
            0.0,
            0.0,
            -7.0
        );
    }

    public function test_idle_state_waits_when_fading(): void
    {
        $this->resolveSymbols();

        // Enter state 3 (idle) but still fading
        $this->initMenuStateUint32(0x18, 3);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);
        $this->initMenuStateUint32(0x38, 0); // selected_0x38
        $this->initMenuStateUint32(0x50, 12); // field_0x50 ¨ preview sprite 12/3 = 4

        $this->call('_CourseConfirmMenuTask_8c0181b6');

        // Still fading ¨ remain in state 3; epilogue draws occur
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            4,
            0.0,
            0.0,
            -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            2,
            376.0,
            378.0,
            -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0,
            0.0,
            0.0,
            -7.0
        );
    }

    public function test_animating_state_advances_when_fade_complete(): void
    {
        $this->resolveSymbols();

        // Enter state 4 (animating/fade-in)
        $this->initMenuStateUint32(0x18, 4);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);
        $this->initMenuStateUint32(0x38, 0); // selected_0x38
        $this->initMenuStateUint32(0x50, 12); // field_0x50 ¨ preview sprite 12/3 = 4

        $this->call('_CourseConfirmMenuTask_8c0181b6');

        // Fade complete ¨ advance to state 5, reset logo_timer_0x68 to 0
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 5);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 0);

        // In original assembly, state 4 tail-calls to drawRouteInfo
        // instead of running epilogue rendering
        $this->shouldCall('_drawRouteInfo_8c018118');
    }

    public function test_animating_state_waits_when_fading(): void
    {
        $this->resolveSymbols();

        // Enter state 4 (animating) but still fading
        $this->initMenuStateUint32(0x18, 4);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);
        $this->initMenuStateUint32(0x38, 0); // selected_0x38
        $this->initMenuStateUint32(0x50, 12); // field_0x50 ¨ preview sprite 12/3 = 4

        $this->call('_CourseConfirmMenuTask_8c0181b6');

        // Still fading ¨ remain in state 4
        // In original assembly, state 4 tail-calls to drawRouteInfo regardless
        $this->shouldCall('_drawRouteInfo_8c018118');
    }

    public function test_logo_timer_state_increments_timer(): void
    {
        $this->resolveSymbols();

        // Enter state 5 (logo timer)
        $this->initMenuStateUint32(0x18, 5);
        $this->initMenuStateUint32(0x68, 10); // logo_timer_0x68 = 10

        $this->call('_CourseConfirmMenuTask_8c0181b6');

        // Timer increments from 10 to 11
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 11);
        // State 5 calls drawRouteInfo
        $this->shouldCall('_drawRouteInfo_8c018118');
    }

    public function test_logo_timer_state_advances_when_timer_exceeds_threshold(): void
    {
        $this->resolveSymbols();

        // Enter state 5 with timer at threshold (30)
        $this->initMenuStateUint32(0x18, 5);
        $this->initMenuStateUint32(0x68, 30); // logo_timer_0x68 = 30

        $this->call('_CourseConfirmMenuTask_8c0181b6');

        // Timer increments from 30 to 31, which is > 30
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 31);
        // Then transition to state 6
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 6);
        $this->shouldCall('_FUN_8c010bae')->with(0);
        $this->shouldCall('_FUN_8c010bae')->with(1);
        $this->shouldCall('_push_fadeout_8c022b60')->with(20);
        // State 5 calls drawRouteInfo
        $this->shouldCall('_drawRouteInfo_8c018118');
    }

    public function test_start_loading_state_waits_when_fading(): void
    {
        $this->resolveSymbols();

        // Enter state 6 but still fading
        $this->initMenuStateUint32(0x18, 6);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);

        $this->call('_CourseConfirmMenuTask_8c0181b6');

        // Still fading ¨ call drawRouteInfo and return
        $this->shouldCall('_drawRouteInfo_8c018118');
    }

    public function test_start_loading_state_waits_when_init_busy(): void
    {
        $this->resolveSymbols();

        // Enter state 6, not fading, but init_8c03bd80 is busy
        $this->initMenuStateUint32(0x18, 6);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 1); // busy

        $this->call('_CourseConfirmMenuTask_8c0181b6');

        // init_8c03bd80 is busy ¨ just return (no calls)
    }

    public function test_start_loading_state_initializes_game(): void
    {
        $this->resolveSymbols();

        // Enter state 6, not fading, init not busy
        $this->initMenuStateUint32(0x18, 6);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0); // not busy
        $this->initMenuStateUint32(0x50, 12); // field_0x50 = 12

        // Initialize player progress struct with course data
        $this->setSize('_var_progress_8c1ba1cc', 0x94);
        // Course index will be 12/3 = 4, so we need course[4].field_0x02
        // Courses start at offset 0x44, each is 8 bytes
        // course[4] is at 0x44 + 4*8 = 0x44 + 32 = 0x64
        // field_0x02 is at 0x64 + 2 = 0x66
        $this->initUint8($this->addressOf('_var_progress_8c1ba1cc') + 0x66, 0); // not unlocked

        // Initialize progress->days_0x00
        $progressBase = $this->addressOf('_var_progress_8c1ba1cc');
        $this->initUint32($progressBase + 0x00, 5); // days = 5

        // Initialize lookup table byte at init_8c044d10[days - 1]
        // Note: Ghidra shows PTR_PTR_8c044d0c + days + 3, which equals init_8c044d10 + days - 1
        $this->initUint8($this->addressOf('_init_8c044d10') + 5 - 1, 7);

        // Initialize progress->field_0x04 data (two arrays of 5 uint32 values)
        for ($i = 0; $i < 5; $i++) {
            $this->initUint32($progressBase + 0x04 + $i * 4, 0x1000 + $i); // First array
            $this->initUint32($progressBase + 0x04 + 0x14 + $i * 4, 0x2000 + $i); // Second array
        }

        $this->call('_CourseConfirmMenuTask_8c0181b6');

        // Step 1: Initialize game systems
        $this->shouldCall('_FUN_8c016182');

        // Step 2: Check if course is unlocked (read course[4].field_0x02)
        // Course is not unlocked (we init'd it to 0), so write 1 to var_8c1bb8e0
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8e0'), 1);
        $this->shouldWriteByte($this->addressOf('_var_progress_8c1ba1cc') + 0x66, 1);

        // Step 3: Initialize various game state variables
        $this->shouldWriteLongTo('_var_8c1bb8e8', 0);
        $this->shouldWriteLongTo('_var_8c1bb8e4', 0);
        $this->shouldWriteLongTo('_var_8c1bb8f0', 0);
        $this->shouldWriteLongTo('_var_8c1bb8ec', 0x1d);
        $this->shouldWriteLongTo('_var_8c1bb8f4', 0);

        // Step 4: Copy progress data to two pointer arrays (5 uint32 values each)
        for ($i = 0; $i < 5; $i++) {
            // Copy from progress->field_0x04 to var_8c1ba2b8
            $this->shouldWriteLong(
                $this->addressOf('_var_8c1ba2b8') + $i * 4,
                0x1000 + $i
            );
            // Copy from progress->field_0x04 + 0x14 to var_8c1ba2cc
            $this->shouldWriteLong(
                $this->addressOf('_var_8c1ba2cc') + $i * 4,
                0x2000 + $i
            );
        }

        // Step 5: Update menuState.field_0x50 by adding lookup table value
        // field_0x50 += init_8c044d10[days - 1]
        // (Ghidra decompiled as PTR_PTR_8c044d0c + days + 3, which equals init_8c044d10 + days - 1)
        $menuStateBase = $this->addressOf('_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x50, 12 + 7); // write sum (19)

        // Step 6: Initialize game and push loading task
        $this->shouldCall('_pushLoadingTask_8c013310')->with(19);
    }

    public function test_start_loading_state_initializes_game_when_course_already_unlocked(): void
    {
        $this->resolveSymbols();

        // Enter state 6, not fading, init not busy
        $this->initMenuStateUint32(0x18, 6);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0); // not busy
        $this->initMenuStateUint32(0x50, 12); // field_0x50 = 12

        // Initialize player progress struct with course data
        $this->setSize('_var_progress_8c1ba1cc', 0x94);
        // Course index will be 12/3 = 4, so we need course[4].field_0x02
        // Courses start at offset 0x44, each is 8 bytes
        // course[4] is at 0x44 + 4*8 = 0x44 + 32 = 0x64
        // field_0x02 is at 0x64 + 2 = 0x66
        $this->initUint8($this->addressOf('_var_progress_8c1ba1cc') + 0x66, 1); // already unlocked

        // Initialize progress->days_0x00
        $progressBase = $this->addressOf('_var_progress_8c1ba1cc');
        $this->initUint32($progressBase + 0x00, 5); // days = 5

        // Initialize lookup table byte at init_8c044d10[days - 1]
        $this->initUint8($this->addressOf('_init_8c044d10') + 5 - 1, 7);

        // Initialize progress->field_0x04 data (two arrays of 5 uint32 values)
        for ($i = 0; $i < 5; $i++) {
            $this->initUint32($progressBase + 0x04 + $i * 4, 0x1000 + $i); // First array
            $this->initUint32($progressBase + 0x04 + 0x14 + $i * 4, 0x2000 + $i); // Second array
        }

        $this->call('_CourseConfirmMenuTask_8c0181b6');

        // Step 1: call FUN_8c016182
        $this->shouldCall('_FUN_8c016182');

        // Step 2: Course is already unlocked (field_0x02 == 1), so write 0 to var_8c1bb8e0
        // No write to course[4].field_0x02
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8e0'), 0);

        // Step 3: Initialize various game state variables (this always happens)
        $this->shouldWriteLongTo('_var_8c1bb8e8', 0);
        $this->shouldWriteLongTo('_var_8c1bb8e4', 0);
        $this->shouldWriteLongTo('_var_8c1bb8f0', 0);
        $this->shouldWriteLongTo('_var_8c1bb8ec', 0x1d);
        $this->shouldWriteLongTo('_var_8c1bb8f4', 0);

        // Step 4: Copy progress data to two arrays (5 uint32 values each)
        for ($i = 0; $i < 5; $i++) {
            $this->shouldWriteLong(
                $this->addressOf('_var_8c1ba2b8') + $i * 4,
                0x1000 + $i
            );
            $this->shouldWriteLong(
                $this->addressOf('_var_8c1ba2cc') + $i * 4,
                0x2000 + $i
            );
        }

        // Step 5: Update menuState.field_0x50 by adding lookup table value
        $menuStateBase = $this->addressOf('_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x50, 12 + 7); // write sum (19)

        // Step 6: Initialize game and push loading task
        $this->shouldCall('_pushLoadingTask_8c013310')->with(19);
    }

    public function test_return_to_menu_state_waits_when_fading(): void
    {
        $this->resolveSymbols();

        // Enter state 7 but still fading
        $this->initMenuStateUint32(0x18, 7);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);
        $this->initMenuStateUint32(0x38, 0); // selected_0x38
        $this->initMenuStateUint32(0x50, 12); // field_0x50

        $this->call('_CourseConfirmMenuTask_8c0181b6');

        // Still fading ¨ render epilogue (no state transition)
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            4,
            0.0,
            0.0,
            -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            2,
            376.0,
            378.0,
            -4.0
        );
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0,
            0.0,
            0.0,
            -7.0
        );
    }

    public function test_return_to_menu_state_waits_when_init_busy(): void
    {
        $this->resolveSymbols();

        // Enter state 7, not fading, but init_8c03bd80 is busy
        $this->initMenuStateUint32(0x18, 7);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 1); // busy

        $this->call('_CourseConfirmMenuTask_8c0181b6');

        // init_8c03bd80 is busy ¨ just return (no rendering)
    }

    public function test_return_to_menu_state_transitions_back_to_main_menu(): void
    {
        $this->resolveSymbols();

        // Allocate Task structure (param_2)
        $task = $this->alloc(0x20);

        // Enter state 7, not fading, init not busy
        $this->initMenuStateUint32(0x18, 7);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0); // not busy
        $this->initUint32($this->addressOf('_var_game_mode_8c1bb8fc'), 0); // story mode
        $this->initMenuStateUint32(0x38, 0); // selected_0x38
        $this->initMenuStateUint32(0x50, 12); // field_0x50

        // Initialize var_dialogQueue_8c225fbc[0]
        $this->initUint32($this->addressOf('_var_dialogQueue_8c225fbc'), 2);

        // Initialize init_dialogSequences_8c044c08[2] + 4 (dialog sequence data)
        $dialogSeqBase = $this->addressOf('_init_dialogSequences_8c044c08');
        $this->initUint32($dialogSeqBase + 2 * 8 + 4, 0x12345678);

        $this->call('_CourseConfirmMenuTask_8c0181b6')->with($task);

        // Step 1: Free resource group
        $this->shouldCall('_CourseMenuFreeResourceGroup_8c0185c4')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c
        );

        // Step 2: Set var_8c225fb0 = 0xffffffff
        $this->shouldWriteLongTo('_var_8c225fb0', 0xffffffff);

        // Step 3: Call helper that handles all menu transition logic
        // (game mode check, task action, dialog setup, resource loading, etc.)
        $this->shouldCall('_CourseMenuSwitchFromTask_8c017e18')->with($task);
    }

    private function initMenuStateUint32($offset, $value) {
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + $offset, $value);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_menuState_8c1bc7a8', 0x84);
        $this->setSize('_isFading_8c226568', 4);
        $this->setSize('_FUN_8c010bae', 4);
        $this->setSize('_init_8c03bd80', 4);
        $this->setSize('_drawSprite_8c014f54', 4);
        $this->setSize('_handleBinaryPrompt_8c016caa', 4); // decompiled alias
        $this->setSize('_promptHandleBinary_16caa', 4);    // original symbol name
        $this->setSize('_push_fadeout_8c022b60', 4);
        $this->setSize('__divls', 4);
        $this->setSize('_FUN_8c016182', 4);
        $this->setSize('_pushLoadingTask_8c013310', 4);
        $this->setSize('_var_8c1bb8e0', 4);
        $this->setSize('_var_8c1bb8e4', 4);
        $this->setSize('_var_8c1bb8e8', 4);
        $this->setSize('_var_8c1bb8ec', 4);
        $this->setSize('_var_8c1bb8f0', 4);
        $this->setSize('_var_8c1bb8f4', 4);
        $this->setSize('_var_8c1ba2b8', 0x14); // 5 uint32 values
        $this->setSize('_var_8c1ba2cc', 0x14); // 5 uint32 values
        $this->setSize('_PTR_PTR_8c044d0c', 32); // Lookup table for days
        $this->setSize('_init_8c044d10', 30);
        $this->setSize('_var_game_mode_8c1bb8fc', 4);
        $this->setSize('_var_8c225fb0', 4);
        $this->setSize('_var_dialogQueue_8c225fbc', 0x10); // Array of dialog queue
        $this->setSize('_init_dialogSequences_8c044c08', 0x100); // Dialog sequences data
    }
};
