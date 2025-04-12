<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_init_state_waits_for_pvm_bool()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0);
        $task = 0xbebacafe;

        $this->shouldCall('_getUknPvmBool_8c01432a')->andReturn(1);

        $this->singleCall('_MainMenuTask_8c019e98')->with($task)->run();
    }

    public function test_init_state_advances_to_next_state()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0);

        $this->shouldCall('_getUknPvmBool_8c01432a')->andReturn(0);
        $this->shouldCall('_AsqFreeQueues_11f7e');
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 1);
        $this->shouldCall('_push_fadein_8c022a9c')->with(10);

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_fade_in_state_waits_for_fade_completion()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 0);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 0,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_fade_in_state_advances_to_next_state()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 0);
        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 0,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_idle_state_no_input_remains_idle()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 0);
        $this->initUint32($this->addressOf('_var_peripheral_8c1ba35c') + 0x10, 0);
        $this->initUint32(
            $this->addressOf('_var_midiHandles_8c0fcd28'), 0xbeef0000
        );

        // $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 3, 0);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 0,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_idle_state_input_left_on_first_option_remains_idle()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 0);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 0);
        $this->initUint32($this->addressOf('_var_peripheral_8c1ba35c') + 0x10, 0x40);
        $this->initUint32(
            $this->addressOf('_var_midiHandles_8c0fcd28'), 0xbeef0000
        );

        // $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 3, 0);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 0,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_idle_state_input_right_on_last_option_remains_idle()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 0);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 3);
        $this->initUint32($this->addressOf('_var_peripheral_8c1ba35c') + 0x10, 0x80);
        $this->initUint32(
            $this->addressOf('_var_midiHandles_8c0fcd28'), 0xbeef0000
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 0,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_idle_state_input_left_on_third_option_moves_left()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 0);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 2);
        $this->initUint32($this->addressOf('_var_peripheral_8c1ba35c') + 0x10, 0x40);
        $this->initUint32(
            $this->addressOf('_var_midiHandles_8c0fcd28'), 0xbeef0000
        );

        $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 3, 0);
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1);
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x18, 4);
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x64, 0);
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x68, 0);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 0,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_idle_state_input_right_on_third_option_moves_right()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 0);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 2);
        $this->initUint32($this->addressOf('_var_peripheral_8c1ba35c') + 0x10, 0x80);
        $this->initUint32(
            $this->addressOf('_var_midiHandles_8c0fcd28'), 0xbeef0000
        );

        $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 3, 0);
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x38, 3);
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x18, 3);
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x64, 0);
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x68, 0);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 0,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_idle_state_input_a_on_free_run_option_activates_free_run()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 0);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1);
        $this->initUint32(
            $this->addressOf('_var_peripheral_8c1ba35c') + 0x10, 0x04
        );
        $this->initUint32(
            $this->addressOf('_var_midiHandles_8c0fcd28'), 0xbeef0000
        );

        $this->shouldCall('_sdMidiPlay')->with(0xbeef0000, 1, 0, 0);
        $this->shouldWrite($this->addressOf('_menuState_8c1bc7a8') + 0x18, 5);
        $this->shouldCall('_push_fadeout_8c022b60')->with(10);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 0,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_animating_right_state_animates_frame_0_from_story_to_free_run()
    {
        $this->resolveSymbols();

        // State MAIN_MENU_STATE_ANIMATING_RIGHT
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 3);
        // Story mode selected
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 0);

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 0);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x64, 0);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 0);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 1);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x64, 1);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 1);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 1,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_animating_right_state_animates_frame_1_from_story_to_free_run()
    {
        $this->resolveSymbols();

        // State MAIN_MENU_STATE_ANIMATING_RIGHT
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 3);
        // Story mode selected
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 0);

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x64, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 1);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 2);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 1,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_animating_right_state_animates_frame_2_from_story_to_free_run()
    {
        $this->resolveSymbols();

        // State MAIN_MENU_STATE_ANIMATING_RIGHT
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 3);
        // Story mode selected
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 0);

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x64, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 2);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 3);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 1,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_animating_right_state_animates_frame_3_from_story_to_free_run()
    {
        $this->resolveSymbols();

        // State MAIN_MENU_STATE_ANIMATING_RIGHT
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 3);
        // Story mode selected
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 0);

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x64, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 3);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 4);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 1,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_animating_right_state_animates_frame_4_from_story_to_free_run()
    {
        $this->resolveSymbols();

        // State MAIN_MENU_STATE_ANIMATING_RIGHT
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 3);
        // Story mode selected
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 0);

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x64, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 4);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 2);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x64, 2);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 5);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 2,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }




    public function test_animating_left_state_animates_frame_0_from_story_to_free_run()
    {
        $this->resolveSymbols();

        // State MAIN_MENU_STATE_ANIMATING_LEFT
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 4);
        // Free run mode selected
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1);

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 2);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x64, 0);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 0);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 1);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x64, 1);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 1);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 1,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_animating_left_state_animates_frame_1_from_story_to_free_run()
    {
        $this->resolveSymbols();

        // State MAIN_MENU_STATE_ANIMATING_LEFT
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 4);
        // Free run mode selected
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1);

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x64, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 1);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 2);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 1,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_animating_left_state_animates_frame_2_from_story_to_free_run()
    {
        $this->resolveSymbols();

        // State MAIN_MENU_STATE_ANIMATING_LEFT
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 4);
        // Free run mode selected
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1);

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x64, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 2);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 3);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 1,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_animating_left_state_animates_frame_3_from_story_to_free_run()
    {
        $this->resolveSymbols();

        // State MAIN_MENU_STATE_ANIMATING_LEFT
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 4);
        // Free run mode selected
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1);

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x64, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 3);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 4);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 1,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_animating_left_state_animates_frame_4_from_story_to_free_run()
    {
        $this->resolveSymbols();

        // State MAIN_MENU_STATE_ANIMATING_LEFT
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 4);
        // Free run mode selected
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1);

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x64, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 4);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 0);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x64, 2);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 5);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 0,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_animating_left_state_animates_from_story_to_free_run()
    {
        $this->resolveSymbols();

        // State MAIN_MENU_STATE_ANIMATING_LEFT
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 4);
        // Free run mode selected
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1);

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 2);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x64, 0);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x68, 0);

        $this->call('_MainMenuTask_8c019e98')->with(0xbebacafe);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 1);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x64, 1);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 1);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 1,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );


        $this->call('_MainMenuTask_8c019e98')->with(0xbebacafe);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 2);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 1,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );


        $this->call('_MainMenuTask_8c019e98')->with(0xbebacafe);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 3);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 1,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );


        $this->call('_MainMenuTask_8c019e98')->with(0xbebacafe);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 4);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 1,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->call('_MainMenuTask_8c019e98')->with(0xbebacafe);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 0);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x64, 2);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x68, 5);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 2);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 0,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );
    }

    public function test_selected_state_waits_for_fade()
    {
        $this->resolveSymbols();

        // State MAIN_MENU_STATE_SELECTED
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 5);
        // Free run mode selected
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5C, 1);

        $this->initUint32($this->addressOf('_isFading_8c226568'), 1);

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x65 + 1,
            0.0,
            0.0,
            -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            0x64,
            0.0,
            0.0,
            -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00,
            0x2d,
            0.0,
            0.0,
            -7.0
        );

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_selected_state_opens_free_run_menu()
    {
        $this->resolveSymbols();

        // State MAIN_MENU_STATE_SELECTED
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 5);
        // Free run mode selected
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 1);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5C, 1);

        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        $init_8c044c08 = $this->alloc(64 * 4);
        $this->initUint32($this->addressOf('_init_8c044c08'), $init_8c044c08);
        $this->initUint32($init_8c044c08 + 2 * 4, 0xcafe0002);
        // Used as index for init_8c044c08
        $this->initUint32($this->addressOf('_var_dialog_8c225fbc'), 2);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x3c, 2);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x40, 0);
        $this->shouldWriteLongTo('_var_game_mode_8c1bb8fc', 1);
        $this->shouldWriteLongTo('_var_8c1bb8c0', 1);

        $this->shouldCall('_CourseMenuSwitchFromTask_8c017e18');

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_selected_state_opens_story_menu()
    {
        $this->resolveSymbols();

        // State MAIN_MENU_STATE_SELECTED
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 5);
        // Story mode selected
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 0);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x5C, 1);

        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        $init_8c044c08 = $this->alloc(64 * 4);
        $this->initUint32($this->addressOf('_init_8c044c08'), $init_8c044c08);
        $this->initUint32($init_8c044c08 + 2 * 4, 0xcafe0002);
        // Used as index for init_8c044c08
        $this->initUint32($this->addressOf('_var_dialog_8c225fbc'), 2);

        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x3c, 2);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x40, 0);
        $this->shouldWriteLongTo('_var_game_mode_8c1bb8fc', 0);
        $this->shouldWriteLongTo('_var_8c1bb8c0', 1);

        $this->shouldCall('_CourseMenuSwitchFromTask_8c017e18')->with(0xbebacafe);

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_selected_state_opens_options_menu()
    {
        $this->resolveSymbols();

        // State MAIN_MENU_STATE_SELECTED
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 5);
        // Option selected
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 2);

        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        $this->shouldCall('_FUN_8c01b122')->with(0xbebacafe);

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_selected_state_opens_vm_game_options_menu()
    {
        $this->resolveSymbols();

        // State MAIN_MENU_STATE_SELECTED
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 5);
        // VM Game selected
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 3);

        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        $this->shouldCall('_FUN_8c01c880')->with(0xbebacafe);

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    public function test_selected_state_ignores_other_options()
    {
        $this->resolveSymbols();

        // State MAIN_MENU_STATE_SELECTED
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x18, 5);
        // VM Game selected
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x38, 4);

        $this->initUint32($this->addressOf('_isFading_8c226568'), 0);

        $this->singleCall('_MainMenuTask_8c019e98')->with(0xbebacafe)->run();
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_menuState_8c1bc7a8', 0x6c);
        $this->setSize('_isFading_8c226568', 4);
        $this->setSize('_var_peripheral_8c1ba35c', 52 * 2);
        $this->setSize('_init_8c044c08', 4);
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0xbeef0000);

        // Functions
        $this->setSize('_drawSprite_8c014f54', 0x4);
        $this->setSize('_sdMidiPlay', 0x4);
        $this->setSize('_CourseMenuSwitchFromTask_8c017e18', 0x4);
        $this->setSize('_FUN_8c017420', 0x4);
        $this->setSize('_FUN_8c017d54', 0x4);
        $this->setSize('_FUN_8c01b122', 0x4);
    }
};
