<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new Class extends TestCase {
    public function test_init_state_waits_for_ukn_pvm_bool()
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 0);

        $this->call('_CourseMenuStoryMenuTask_8c017718');

        $this->shouldCall('_getUknPvmBool_8c01432a')->andReturn(1);
    }

    public function test_init_state_advances()
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 0);

        $this->call('_CourseMenuStoryMenuTask_8c017718');

        $this->shouldCall('_getUknPvmBool_8c01432a')->andReturn(0);
        $this->shouldCall('_AsqFreeQueues_11f7e');
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 1);
        $this->shouldCall('_FUN_8c010d8a');
        $this->shouldCall('_snd_8c010cd6')->with(0, 15);
        $this->shouldCall('_push_fadein_8c022a9c')->with(10);
    }

    public function test_fade_in_state_waits_for_fade()
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 1);
        $this->initMenuStateUint32(0x60, 42);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);
        $this->initUint32($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 21);

        $this->call('_CourseMenuStoryMenuTask_8c017718');

        $this->shouldRenderFrame(
            spriteNo: 42,
            textboxIndex: 21,
            menuTextboxReturns: 1,
        );
    }

    public function test_fade_in_state_advances()
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 1);
        $this->initMenuStateUint32(0x60, 42);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 21);
        $this->initUint32($this->addressOf('_var_dialogQueue_8c225fbc') + 4 * 0, 32);

        $this->call('_CourseMenuStoryMenuTask_8c017718');

        $this->shouldCall('_CourseMenuPushDialogTask_8c0170c6')->with(32);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2);

        $this->shouldRenderFrame(
            spriteNo: 42,
            textboxIndex: 21,
            menuTextboxReturns: 1,
        );
    }

    public function test_dialog_state_waits_while_dialog_running()
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 2);
        $this->initMenuStateUint32(0x60, 42);

        // Dialog is still running => should just break (no state advance, no new dialog)
        $this->initUint32($this->addressOf('_var_dialogSequenceIsActive_8c225fb4'), 1);

        $this->initUint32($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 21);

        $this->call('_CourseMenuStoryMenuTask_8c017718');

        // End-of-frame rendering should always run
        $this->shouldRenderFrame(
            spriteNo: 42,
            textboxIndex: 21,
            menuTextboxReturns: 1,
        );
    }

    public function test_dialog_state_writes_buttons_when_current_is_unlock()
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 2);
        $this->initMenuStateUint32(0x60, 42);

        $this->initUint32($this->addressOf('_var_dialogSequenceIsActive_8c225fb4'), 0);

        $this->initUint32($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 21);

        // Dialog sequence table
        $seqBase = $this->addressOf('_var_dialogQueue_8c225fbc');
        $this->initUint32($seqBase + 0, 0x0d); // SEQ_COURSE_UNLOCKED
        $this->initUint32($seqBase + 4, -1);

        // Seed midi handle used for course unlock jingle
        $midiBase = $this->addressOf('_var_midiHandles_8c0fcd28');
        $this->initUint32($midiBase + 5 * 4, 0x12345678);

        // Source bytes for course button values:
        $this->seedCourseButtonValues(0xA1);

        $task = $this->alloc(0x10);
        $this->initUint32($task + 0x08, 0);

        $this->call('_CourseMenuStoryMenuTask_8c017718')->with($task, 0);

        $this->shouldCall('_CourseMenuApplyUnlocks_8c0173e6');

        $btnBase = $this->addressOf('_init_courseMenuButtons_8c04442c');

        $this->shouldWriteCourseButtonValues(0xA1);

        // Jingle after loop completes
        $this->shouldCall('_sdMidiPlay')->with(0x12345678, 1, 0x16, 0);

        // Increment dialogSequenceIndex
        $this->shouldWriteLong($task + 0x08, 1);

        // Last sequence: advance state and swap message box
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 3);
        $this->shouldCall('_swapMessageBoxFor_8c02aefc')->with($this->addressOf('_const_8c03628c'));

        // Epilogue rendering
        $this->shouldRenderFrame(
            spriteNo: 42,
            textboxIndex: 21,
            menuTextboxReturns: 1,
        );
    }

    public function test_dialog_state_pushes_next_sequence_when_there_is_more()
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 2);
        $this->initMenuStateUint32(0x60, 42);

        $this->initUint32($this->addressOf('_var_dialogSequenceIsActive_8c225fb4'), 0);
        $this->initUint32($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 21);

        // Dialog sequences:
        // current != SEQ_COURSE_UNLOCKED to avoid unlock branch,
        // next != -1 to indicate there's more dialogs,
        // next also != SEQ_COURSE_UNLOCKED so no midiReset call.
        $seqBase = $this->addressOf('_var_dialogQueue_8c225fbc');
        $this->initUint32($seqBase + 0, 0x05); // current
        $this->initUint32($seqBase + 4, 0x22); // next (pushed)

        $task = $this->alloc(0x10);
        $this->initUint32($task + 0x08, 0);

        $this->call('_CourseMenuStoryMenuTask_8c017718')->with($task, 0);

        // Effects (no course-unlock branch, just advance + push next)
        $this->shouldWriteLong($task + 0x08, 1);                // ++dialogSequenceIndex
        $this->shouldCall('_CourseMenuPushDialogTask_8c0170c6')->with(0x22, 0); // start next sequence

        $this->shouldRenderFrame(
            spriteNo: 42,
            textboxIndex: 21,
            menuTextboxReturns: 1
        );
    }

    public function test_dialog_state_triggers_midi_reset_when_next_is_unlock()
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 2);
        $this->initMenuStateUint32(0x60, 42);

        $this->initUint32($this->addressOf('_var_dialogSequenceIsActive_8c225fb4'), 0);

        $this->initUint32($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 21);

        // Dialog sequences:
        // current != SEQ_COURSE_UNLOCKED (skip unlock branch),
        // next == SEQ_COURSE_UNLOCKED (so midiResetFxAndPlay should trigger).
        $seqBase = $this->addressOf('_var_dialogQueue_8c225fbc');
        $this->initUint32($seqBase + 0, 0x05); // current (non-unlock)
        $this->initUint32($seqBase + 4, 0x0d); // next is SEQ_COURSE_UNLOCKED

        $task = $this->alloc(0x10);
        $this->initUint32($task + 0x08, 0); // dialogSequenceIndex

        $this->call('_CourseMenuStoryMenuTask_8c017718')->with($task, 0);

        // Effects:
        // No course-unlock processing for the "current" (we deliberately avoided it)
        // Increment index, then push next dialog, then reset+play midi because next == unlock
        $this->shouldWriteLong($task + 0x08, 1);
        $this->shouldCall('_CourseMenuPushDialogTask_8c0170c6')->with(0x0d, 0);
        $this->shouldCall('_midiResetFxAndPlay_8c010846')->with(0, 0);

        $this->shouldRenderFrame(
            spriteNo: 42,
            textboxIndex: 21,
            menuTextboxReturns: 1
        );
    }

    public function test_dialog_state_when_current_is_unlock_next_is_normal_dialog()
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 2);
        $this->initMenuStateUint32(0x60, 42);

        // Dialog finished (so we advance this frame)
        $this->initUint32($this->addressOf('_var_dialogSequenceIsActive_8c225fb4'), 0);

        $this->initUint32($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 21);

        // Dialog sequences: current == unlock, next == non-unlock
        $seqBase = $this->addressOf('_var_dialogQueue_8c225fbc');
        $this->initUint32($seqBase + 0, 0x0d); // SEQ_COURSE_UNLOCKED
        $this->initUint32($seqBase + 4, 0x22); // next sequence (not unlock)

        // Seed midi handle used for unlock jingle
        $midiBase = $this->addressOf('_var_midiHandles_8c0fcd28');
        $this->initUint32($midiBase + 5 * 4, 0x12345678);

        $this->seedCourseButtonValues(0xB0);

        // Allocate Task and set dialogSequenceIndex = 0
        $task = $this->alloc(0x10);
        $this->initUint32($task + 0x08, 0);

        $this->call('_CourseMenuStoryMenuTask_8c017718')->with($task, 0);

        // Current == unlock: do the loop + jingle
        $this->shouldCall('_CourseMenuApplyUnlocks_8c0173e6');

        $this->shouldWriteCourseButtonValues(0xB0);

        $this->shouldCall('_sdMidiPlay')->with(0x12345678, 1, 0x16, 0);

        // Advance dialog index
        $this->shouldWriteLong($task + 0x08, 1);

        // Next is not -1: push next dialog (not unlock, so no midiReset)
        $this->shouldCall('_CourseMenuPushDialogTask_8c0170c6')->with(0x22, 0);

        $this->shouldRenderFrame(
            spriteNo: 42,
            textboxIndex: 21,
            menuTextboxReturns: 1
        );
    }

    public function test_dialog_state_when_current_is_unlock_next_is_also_unlock()
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 2);
        $this->initMenuStateUint32(0x60, 42);
        $this->initUint32($this->addressOf('_var_dialogSequenceIsActive_8c225fb4'), 0);
        $this->initUint32($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 21);

        // Dialog sequences: current == unlock, next == unlock
        $seqBase = $this->addressOf('_var_dialogQueue_8c225fbc');
        $this->initUint32($seqBase + 0, 0x0d); // SEQ_COURSE_UNLOCKED
        $this->initUint32($seqBase + 4, 0x0d); // next also unlock

        // Seed midi handle used for unlock jingle
        $midiBase = $this->addressOf('_var_midiHandles_8c0fcd28');
        $this->initUint32($midiBase + 5 * 4, 0x12345678);

        $this->seedCourseButtonValues(0xC0);

        // Allocate Task and set dialogSequenceIndex = 0
        $task = $this->alloc(0x10);
        $this->initUint32($task + 0x08, 0);

        $this->call('_CourseMenuStoryMenuTask_8c017718')->with($task, 0);

        // Current == unlock: run loop + jingle
        $this->shouldCall('_CourseMenuApplyUnlocks_8c0173e6');

        $this->shouldWriteCourseButtonValues(0xC0);

        $this->shouldCall('_sdMidiPlay')->with(0x12345678, 1, 0x16, 0);

        // Advance dialog index
        $this->shouldWriteLong($task + 0x08, 1);

        // Next == unlock: push dialog, then midiResetFxAndPlay
        $this->shouldCall('_CourseMenuPushDialogTask_8c0170c6')->with(0x0d, 0);
        $this->shouldCall('_midiResetFxAndPlay_8c010846')->with(0, 0);

        $this->shouldRenderFrame(
            spriteNo: 42,
            textboxIndex: 21,
            menuTextboxReturns: 1
        );
    }

    public function test_idle_state_handles_cursor_and_renders_frame()
    {
        $this->resolveSymbols();
        $this->initMenuStateUint32(0x18, 3);
        $this->initMenuStateUint32(0x60, 42);
        $this->initUint32($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 21);

        $task = $this->alloc(0x10);
        $this->initUint32($task + 0x08, 0);

        $this->call('_CourseMenuStoryMenuTask_8c017718')->with($task, 0);

        $this->shouldCall('_handleCourseMenuInput_8c017126');
        $this->shouldRenderFrame(spriteNo: 42, textboxIndex: 21, menuTextboxReturns: 1);
    }

    public function test_animating_state_waits_until_interpolation_finishes()
    {
        $this->resolveSymbols();

        // Enter ANIMATING state
        $this->initMenuStateUint32(0x18, 4);
        // ResourceGroupB sprite index + textbox index for epilogue
        $this->initMenuStateUint32(0x60, 42);
        $this->initUint32($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 21);

        // Allocate task (unused by this branch)
        $task = $this->alloc(0x10);

        // Invoke
        $this->call('_CourseMenuStoryMenuTask_8c017718')->with($task, 0);

        // ANIMATING branch: interpolate returns false -> remain in state 4
        $this->shouldCall('_CourseMenuInterpolateCursor_8c016d2c')->andReturn(0);

        // Epilogue rendering
        $this->shouldRenderFrame(
            spriteNo: 42,
            textboxIndex: 21,
            menuTextboxReturns: 1
        );
    }

    public function test_animating_state_advances_to_idle_when_interpolation_finishes()
    {
        $this->resolveSymbols();

        // Enter ANIMATING state
        $this->initMenuStateUint32(0x18, 4);
        // ResourceGroupB sprite index + textbox index for epilogue
        $this->initMenuStateUint32(0x60, 42);
        $this->initUint32($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 21);

        // Allocate task (unused by this branch)
        $task = $this->alloc(0x10);

        // Invoke
        $this->call('_CourseMenuStoryMenuTask_8c017718')->with($task, 0);

        // ANIMATING branch: interpolate returns true -> write state = IDLE (3)
        $this->shouldCall('_CourseMenuInterpolateCursor_8c016d2c')->andReturn(1);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 3);

        // Epilogue rendering
        $this->shouldRenderFrame(
            spriteNo: 42,
            textboxIndex: 21,
            menuTextboxReturns: 1
        );
    }

    public function test_course_selected_state_increments_timer_and_waits_until_threshold()
    {
        $this->resolveSymbols();

        // Enter COURSE_SELECTED state
        $this->initMenuStateUint32(0x18, 5);
        // 8 -> 9 (still <= 10, so no fade)
        $this->initMenuStateUint32(0x68, 8);

        // Epilogue inputs
        $this->initMenuStateUint32(0x60, 42);
        $this->initUint32($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 21);

        $task = $this->alloc(0x10);

        $this->call('_CourseMenuStoryMenuTask_8c017718')->with($task, 0);

        $base = $this->addressOf('_menuState_8c1bc7a8');

        // timer++
        $this->shouldWriteLong($base + 0x68, 9);
        // shared epilogue: field_0x48 = (logo_timer & 1) -> 9 & 1 = 1
        $this->shouldWriteLong($base + 0x48, 1);

        $this->shouldRenderFrame(42, 21, 1);
    }

    public function test_course_selected_state_advances_to_fade_out_after_threshold()
    {
        $this->resolveSymbols();

        // Enter COURSE_SELECTED state
        $this->initMenuStateUint32(0x18, 5);
        // 10 -> 11 (> 10) triggers fade out
        $this->initMenuStateUint32(0x68, 10);

        // Epilogue inputs
        $this->initMenuStateUint32(0x60, 42);
        $this->initUint32($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 21);

        $task = $this->alloc(0x10);

        $this->call('_CourseMenuStoryMenuTask_8c017718')->with($task, 0);

        $base = $this->addressOf('_menuState_8c1bc7a8');

        // timer++
        $this->shouldWriteLong($base + 0x68, 11);
        $this->shouldWriteLong($base + 0x18, 6);
        // state -> FADE_OUT and fadeout(10)
        $this->shouldCall('_push_fadeout_8c022b60')->with(10);

        $this->shouldWriteLong($base + 0x48, 1);
        $this->shouldRenderFrame(42, 21, 1);
    }

    public function test_fade_out_state_waits_for_fade()
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 6);
        $this->initMenuStateUint32(0x68, 7);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);
        $this->initMenuStateUint32(0x60, 42);
        $this->initUint32($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 21);

        $this->call('_CourseMenuStoryMenuTask_8c017718');

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 8);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x48, 0);
        $this->shouldRenderFrame(
            spriteNo: 42,
            textboxIndex: 21,
            menuTextboxReturns: 1,
        );
    }

    public function test_fade_out_state_waits_for_init_8c03bd80()
    {
        $this->resolveSymbols();

        $this->initMenuStateUint32(0x18, 6);
        $this->initMenuStateUint32(0x68, 7);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);
        $this->initMenuStateUint32(0x60, 42);
        $this->initUint32($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 21);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 1);

        $this->call('_CourseMenuStoryMenuTask_8c017718');
    }

    public function test_fade_out_state_happy_path_without_free()
    {
        $this->resolveSymbols();

        // Enter FADE_OUT
        $this->initMenuStateUint32(0x18, 6);
        // No fade in progress, init flag cleared
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0);

        // Choose coordinates that DISABLE the free path: field_0x3c == 1 && field_0x40 == 0
        $this->initMenuStateUint32(0x3c, 1);
        $this->initMenuStateUint32(0x40, 0);

        // Prepare the selected button entry: index = 0*5 + 1 = 1
        $btnBase = $this->addressOf('_init_courseMenuButtons_8c04442c');
        $idx = 1;
        $cb = $this->addressOf('_dummyCallback');

        // Seed function pointer and payload value read by code
        $this->initUint32($btnBase + $idx * 0x1c + 0x14, $cb);       // .field_0x14 (callback)
        $this->initUint32($btnBase + $idx * 0x1c + 0x18, 0xbebacafe); // .field_0x18 (copied to menuState.field_0x50)

        // Allocate task (passed to callback)
        $task = $this->alloc(0x10);

        // Invoke frame
        $this->call('_CourseMenuStoryMenuTask_8c017718')->with($task, 0);

        $base = $this->addressOf('_menuState_8c1bc7a8');

        // No freeResourceGroup path here

        // selected = 0
        $this->shouldWriteLong($base + 0x38, 0);

        // field_0x50 <- btn[idx].field_0x18
        $this->shouldWriteLong($base + 0x50, 0xbebacafe);

        // Flip globals
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8dc'), 1);
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8b8'), 0);
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8bc'), 1);

        $this->shouldCall('_dummyCallback')->with($task);
    }

    public function test_fade_out_state_happy_path_with_free()
    {
        $this->resolveSymbols();

        // Enter FADE_OUT and ensure wefre allowed to proceed
        $this->initMenuStateUint32(0x18, 6);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0);

        // Coordinates that TRIGGER the free path: field_0x3c != 1 (e.g., 2) || field_0x40 != 0
        $this->initMenuStateUint32(0x3c, 2);
        $this->initMenuStateUint32(0x40, 0);

        // Button index = 0*5 + 2 = 2
        $btnBase = $this->addressOf('_init_courseMenuButtons_8c04442c');
        $idx     = 2;

        // Seed function pointer and payload value read from the button
        $cb = $this->addressOf('_dummyCallback');
        $this->initUint32($btnBase + $idx * 0x1c + 0x14, $cb);        // .field_0x14 (callback)
        $this->initUint32($btnBase + $idx * 0x1c + 0x18, 0xCAFEBABE); // .field_0x18 (copied to menuState.field_0x50)

        // Allocate task (passed to callback)
        $task = $this->alloc(0x10);

        // Invoke
        $this->call('_CourseMenuStoryMenuTask_8c017718')->with($task, 0);

        $base = $this->addressOf('_menuState_8c1bc7a8');

        // Conditional free branch
        $this->shouldCall('_CourseMenuFreeResourceGroup_8c0185c4')->with($base + 0x0c);
        $this->shouldWriteLong($this->addressOf('_var_currentSysResGroupInfo_8c225fb0'), -1);

        // selected = 0
        $this->shouldWriteLong($base + 0x38, 0);

        // field_0x50 <- btn[idx].field_0x18
        $this->shouldWriteLong($base + 0x50, 0xCAFEBABE);

        // Flip globals
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8dc'), 1);
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8b8'), 0);
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8bc'), 1);

        // Indirect callback invoked with task
        $this->shouldCall('_dummyCallback')->with($task);
    }

    public function test_fade_out_to_main_menu_waits_while_fading()
    {
        $this->resolveSymbols();

        // Enter FADE_OUT_TO_MAIN_MENU
        $this->initMenuStateUint32(0x18, 7);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1); // still fading -> break

        // Epilogue inputs
        $this->initMenuStateUint32(0x60, 42);
        $this->initUint32($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 21);

        $task = $this->alloc(0x10);

        $this->call('_CourseMenuStoryMenuTask_8c017718')->with($task, 0);

        // No state writes or menu switch while fading; epilogue rendering runs
        $this->shouldRenderFrame(
            spriteNo: 42,
            textboxIndex: 21,
            menuTextboxReturns: 1
        );
    }

    public function test_fade_out_to_main_menu_waits_for_init_8c03bd80()
    {
        $this->resolveSymbols();

        // Enter FADE_OUT_TO_MAIN_MENU
        $this->initMenuStateUint32(0x18, 7);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0); // fade done
        $this->initUint32($this->addressOf('_init_8c03bd80'), 1);

        // Epilogue inputs
        $this->initMenuStateUint32(0x60, 42);
        $this->initUint32($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 21);

        $task = $this->alloc(0x10);

        $this->call('_CourseMenuStoryMenuTask_8c017718')->with($task, 0);
    }

    public function test_fade_out_to_main_menu()
    {
        $this->resolveSymbols();

        // Enter FADE_OUT_TO_MAIN_MENU
        $this->initMenuStateUint32(0x18, 7);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0); // fade finished
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0);

        // (Epilogue inputs would be irrelevant?this path returns early)

        $task = $this->alloc(0x10);

        $this->call('_CourseMenuStoryMenuTask_8c017718')->with($task, 0);

        // Writes and call in order, then return (no epilogue rendering)
        $this->shouldWriteLong($this->addressOf('_var_8c1bb8b8'), 0);
        $this->shouldCall('_MainMenuSwitchFromTask_8c01a09a')->with($task);
    }

    private function initMenuStateUint32($offset, $value) {
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + $offset, $value);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_menuState_8c1bc7a8', 0x6c);
        $this->setSize('_var_dialogQueue_8c225fbc', 4 * 4);
        $this->setSize('_init_8c03bd80', 4);
        $this->setSize('_isFading_8c226568', 4);
        $this->setSize('_var_dialogSequenceIsActive_8c225fb4', 4); // dialog-running flag
        $this->setSize('_const_8c03628c', 4);
        $this->setSize('_init_courseMenuButtons_8c04442c', 0x1c * 15);
        $this->setSize('_var_progress_8c1ba1cc', 0x94);
    }

    /**
     * Seed 9 course button values in var_progress_8c1ba1cc.courses_0x44.
     * Values will be base, base+1, c base+8.
     */
    private function seedCourseButtonValues(int $baseVal): void
    {
        $src = $this->addressOf('_var_progress_8c1ba1cc') + 0x44;
        for ($i = 0; $i < 9; $i++) {
            $this->initUint8($src + $i * 8, $baseVal + $i);
        }
    }

    /**
     * Assert that init_courseMenuButtons[2..4,7..9,12..14].field_0x04
     * got written with base, base+1, c base+8.
     */
    private function shouldWriteCourseButtonValues(int $baseVal): void
    {
        $btnBase = $this->addressOf('_init_courseMenuButtons_8c04442c');
        $indices = [2, 3, 4, 7, 8, 9, 12, 13, 14];

        foreach ($indices as $i => $idx) {
            $this->shouldWriteLong($btnBase + $idx * 0x1c + 0x04, $baseVal + $i);
        }
    }

    private function shouldRenderFrame(int $spriteNo, int $textboxIndex, int $menuTextboxReturns = 1): void
    {
        $this->shouldCall('_CourseMenuDrawDateAndExp_8c016ee6');
        $this->shouldCall('_drawCourseButtons_8c017590');
        $this->shouldDrawSprite(0x0c, 0x0a, 0.0, 0.0, -5.0);
        $this->shouldDrawSprite(0x00, 0x2b, 0.0, 0.0, -4.0);
        $this->shouldCall('_menuTextboxText_8c02af1c')->with($textboxIndex)->andReturn($menuTextboxReturns);
        if ($menuTextboxReturns) {
            $this->shouldDrawSprite(0x00, 1, 0.0, 0.0, -5.0);
        }
        $this->shouldDrawSprite(0x0c, $spriteNo, 0.0, 0.0, -6.0);
        $this->shouldDrawSprite(0x00, 0x00, 0.0, 0.0, -7.0);
        $this->shouldCall('_AsqGetRandomA_12166');
    }

    private function shouldDrawSprite(
        int $resourceGroup,
        int $spriteNo,
        float $x,
        float $y,
        float $priority,
    ) {
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + $resourceGroup,
            $spriteNo,
            $x,
            $y,
            $priority,
        );
    }
};
