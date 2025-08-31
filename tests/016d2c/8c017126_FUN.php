<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    public function test_does_nothing_without_inputs()
    {
        // -- Arrange ------------------
        $this->resolveSymbols();

        // Nothing pressed.
        $this->initUint32($this->addressOf('_var_peripheral_8c1ba35c') + 16, 0);

        // -- Act ----------------------
        $this->call('_FUN_handleCourseMenuCursor_8c017126');

        // -- Assert -------------------
        // No asserts for this test
    }

    private function initCourseMenuButtonArray()
    {
        $runStructBytes = [
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x42, 0x00, 0x00, 0xda, 0x42, 0x00, 0x00, 0x00, 0x00, 0x14, 0xf1, 0x01, 0x8c, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdc, 0x42, 0x00, 0x00, 0xda, 0x42, 0x00, 0x00, 0x00, 0x00, 0x64, 0xba, 0x01, 0x8c, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x43, 0x00, 0x00, 0xda, 0x42, 0x0b, 0x00, 0x00, 0x00, 0xcc, 0x84, 0x01, 0x8c, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8e, 0x43, 0x00, 0x00, 0xda, 0x42, 0x0c, 0x00, 0x00, 0x00, 0xcc, 0x84, 0x01, 0x8c, 0x03, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0xbc, 0x43, 0x00, 0x00, 0xda, 0x42, 0x0d, 0x00, 0x00, 0x00, 0xcc, 0x84, 0x01, 0x8c, 0x06, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x42, 0x00, 0x00, 0x36, 0x43, 0x00, 0x00, 0x00, 0x00, 0xc4, 0xd1, 0x01, 0x8c, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xda, 0x42, 0x00, 0x00, 0x36, 0x43, 0x00, 0x00, 0x00, 0x00, 0xe2, 0xd6, 0x01, 0x8c, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x43, 0x00, 0x00, 0x36, 0x43, 0x0e, 0x00, 0x00, 0x00, 0xcc, 0x84, 0x01, 0x8c, 0x09, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0x8e, 0x43, 0x00, 0x00, 0x36, 0x43, 0x0f, 0x00, 0x00, 0x00, 0xcc, 0x84, 0x01, 0x8c, 0x0c, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbd, 0x43, 0x00, 0x00, 0x36, 0x43, 0x10, 0x00, 0x00, 0x00, 0xcc, 0x84, 0x01, 0x8c, 0x0f, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x42, 0x00, 0x00, 0x80, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd8, 0x42, 0x00, 0x00, 0x80, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x43, 0x00, 0x00, 0x80, 0x43, 0x11, 0x00, 0x00, 0x00, 0xcc, 0x84, 0x01, 0x8c, 0x12, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0x8e, 0x43, 0x00, 0x00, 0x80, 0x43, 0x12, 0x00, 0x00, 0x00, 0xcc, 0x84, 0x01, 0x8c, 0x15, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbd, 0x43, 0x00, 0x00, 0x80, 0x43, 0x13, 0x00, 0x00, 0x00, 0xcc, 0x84, 0x01, 0x8c, 0x18, 0x00, 0x00, 0x00,
        ];

        foreach ($runStructBytes as $key => $value) {
            $this->initUint8(
                $this->addressOf('_init_courseMenuButtons_8c04442c') + $key, $value
            );
        }
    }

    private function initCursor(int $x, int $y)
    {
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x3c, $x);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x40, $y);
    }

    public function test_it_selects_an_option()
    {
        // -- Arrange ------------------
        $this->resolveSymbols();

        $this->initCourseMenuButtonArray();
        // MIDI handles
        $this->initUint32(
            $this->addressOf('_var_midiHandles_8c0fcd28'), 0xd1d1d1d1
        );

        // Practice option selected
        $this->initCursor(0, 0);

        // A button pressed
        $this->initUint32(
            $this->addressOf('_var_peripheral_8c1ba35c') + 16, 1 << 2
        );

        // -- Act ----------------------
        $this->call('_FUN_handleCourseMenuCursor_8c017126');

        // -- Assert -------------------
        $this->shouldCall('_FUN_8c010bae')->with(0);
        $this->shouldCall('_FUN_8c010bae')->with(1);
        $this->shouldCall('_sdMidiPlay')->with(0xd1d1d1d1, 1, 0, 0);
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x18, 5
        );
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x68, 0
        );
    }

    public function test_it_selects_a_course()
    {
        // -- Arrange ------------------
        $this->resolveSymbols();

        $this->initCourseMenuButtonArray();
        // MIDI handles
        $this->initUint32(
            $this->addressOf('_var_midiHandles_8c0fcd28'), 0xd1d1d1d1
        );

        // Course option selected
        $this->initCursor(2, 0);

        // A button pressed
        $this->initUint32(
            $this->addressOf('_var_peripheral_8c1ba35c') + 16, 1 << 2
        );

        // -- Act ----------------------
        $this->call('_FUN_handleCourseMenuCursor_8c017126');

        // -- Assert -------------------
        $this->shouldCall('_FUN_8c010bae')->with(0);
        $this->shouldCall('_FUN_8c010bae')->with(1);
        $this->shouldCall('_sdMidiPlay')->with(0xd1d1d1d1, 1, 0, 0);
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x18, 5
        );
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x68, 0
        );
    }

    public function test_it_moves_cursor_up()
    {
        // -- Arrange ------------------
        $this->resolveSymbols();

        $this->initCourseMenuButtonArray();
        // MIDI handles
        $this->initUint32(
            $this->addressOf('_var_midiHandles_8c0fcd28'), 0xd1d1d1d1
        );

        // Event option selected
        $this->initCursor(0, 1);

        // Up button pressed
        $this->initUint32(
            $this->addressOf('_var_peripheral_8c1ba35c') + 16, 1 << 4
        );

        // -- Act ----------------------
        $this->call('_FUN_handleCourseMenuCursor_8c017126');

        // -- Assert -------------------
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x40, 0
        );
        $this->shouldCall('_cursorOffTarget_8c016dc6')->andReturn(1);
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x18, 4
        );
    }

    public function test_it_loops_cursor_up_and_skips_hidden_options()
    {
        // -- Arrange ------------------
        $this->resolveSymbols();

        $this->initCourseMenuButtonArray();
        // MIDI handles
        $this->initUint32(
            $this->addressOf('_var_midiHandles_8c0fcd28'), 0xd1d1d1d1
        );

        // Event option selected
        $this->initCursor(0, 0);

        // Up button pressed
        $this->initUint32(
            $this->addressOf('_var_peripheral_8c1ba35c') + 16, 1 << 4
        );

        // -- Act ----------------------
        $this->call('_FUN_handleCourseMenuCursor_8c017126');

        // -- Assert -------------------
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x40, -1
        );
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x40, 2
        );
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x40, 1
        );
        $this->shouldCall('_cursorOffTarget_8c016dc6')->andReturn(1);
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x18, 4
        );
    }

    public function test_it_moves_cursor_down()
    {
        // -- Arrange ------------------
        $this->resolveSymbols();

        $this->initCourseMenuButtonArray();
        // MIDI handles
        $this->initUint32(
            $this->addressOf('_var_midiHandles_8c0fcd28'), 0xd1d1d1d1
        );

        // Event option selected
        $this->initCursor(0, 0);

        // Down button pressed
        $this->initUint32(
            $this->addressOf('_var_peripheral_8c1ba35c') + 16, 1 << 5
        );

        // -- Act ----------------------
        $this->call('_FUN_handleCourseMenuCursor_8c017126');

        // -- Assert -------------------
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x40, 1
        );
        $this->shouldCall('_cursorOffTarget_8c016dc6')->andReturn(1);
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x18, 4
        );
    }

    public function test_it_loops_cursor_down_and_skips_hidden_options()
    {
        // -- Arrange ------------------
        $this->resolveSymbols();

        $this->initCourseMenuButtonArray();
        // MIDI handles
        $this->initUint32(
            $this->addressOf('_var_midiHandles_8c0fcd28'), 0xd1d1d1d1
        );

        // Event option selected
        $this->initCursor(0, 1);

        // Down button pressed
        $this->initUint32(
            $this->addressOf('_var_peripheral_8c1ba35c') + 16, 1 << 5
        );

        // -- Act ----------------------
        $this->call('_FUN_handleCourseMenuCursor_8c017126');

        // -- Assert -------------------
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x40, 2
        );
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x40, 3
        );
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x40, 0
        );
        $this->shouldCall('_cursorOffTarget_8c016dc6')->andReturn(1);
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x18, 4
        );
    }

    public function test_it_moves_cursor_left()
    {
        // -- Arrange ------------------
        $this->resolveSymbols();

        $this->initCourseMenuButtonArray();
        // MIDI handles
        $this->initUint32(
            $this->addressOf('_var_midiHandles_8c0fcd28'), 0xd1d1d1d1
        );

        // Event option selected
        $this->initCursor(1, 0);

        // Left button pressed
        $this->initUint32(
            $this->addressOf('_var_peripheral_8c1ba35c') + 16, 1 << 6
        );

        // -- Act ----------------------
        $this->call('_FUN_handleCourseMenuCursor_8c017126');

        // -- Assert -------------------
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x3c, 0
        );
        $this->shouldCall('_cursorOffTarget_8c016dc6')->andReturn(1);
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x18, 4
        );
    }

    public function test_it_loops_cursor_left_and_skips_hidden_options()
    {
        // -- Arrange ------------------
        $this->resolveSymbols();

        $this->initCourseMenuButtonArray();
        // MIDI handles
        $this->initUint32(
            $this->addressOf('_var_midiHandles_8c0fcd28'), 0xd1d1d1d1
        );

        // Event option selected
        $this->initCursor(2, 2);

        // Left button pressed
        $this->initUint32(
            $this->addressOf('_var_peripheral_8c1ba35c') + 16, 1 << 6
        );

        // -- Act ----------------------
        $this->call('_FUN_handleCourseMenuCursor_8c017126');

        // -- Assert -------------------
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x3c, 1
        );
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x3c, 0
        );
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x3c, -1
        );
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x3c, 4
        );
        $this->shouldCall('_cursorOffTarget_8c016dc6')->andReturn(1);
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x18, 4
        );
    }

    public function test_it_moves_cursor_right()
    {
        // -- Arrange ------------------
        $this->resolveSymbols();

        $this->initCourseMenuButtonArray();
        // MIDI handles
        $this->initUint32(
            $this->addressOf('_var_midiHandles_8c0fcd28'), 0xd1d1d1d1
        );

        // Event option selected
        $this->initCursor(0, 0);

        // Right button pressed
        $this->initUint32(
            $this->addressOf('_var_peripheral_8c1ba35c') + 16, 1 << 7
        );

        // -- Act ----------------------
        $this->call('_FUN_handleCourseMenuCursor_8c017126');

        // -- Assert -------------------
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x3c, 1
        );
        $this->shouldCall('_cursorOffTarget_8c016dc6')->andReturn(1);
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x18, 4
        );
    }

    public function test_it_loops_cursor_right_and_skips_hidden_options()
    {
        // -- Arrange ------------------
        $this->resolveSymbols();

        $this->initCourseMenuButtonArray();
        // MIDI handles
        $this->initUint32(
            $this->addressOf('_var_midiHandles_8c0fcd28'), 0xd1d1d1d1
        );

        // Event option selected
        $this->initCursor(4, 2);

        // Right button pressed
        $this->initUint32(
            $this->addressOf('_var_peripheral_8c1ba35c') + 16, 1 << 7
        );

        // -- Act ----------------------
        $this->call('_FUN_handleCourseMenuCursor_8c017126');

        // -- Assert -------------------
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x3c, 5
        );
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x3c, 0
        );
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x3c, 1
        );
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x3c, 2
        );
        $this->shouldCall('_cursorOffTarget_8c016dc6')->andReturn(1);
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x18, 4
        );
    }

    public function test_it_does_not_changes_the_state_when_cursor_is_on_target()
    {
        // -- Arrange ------------------
        $this->resolveSymbols();

        $this->initCourseMenuButtonArray();
        // MIDI handles
        $this->initUint32(
            $this->addressOf('_var_midiHandles_8c0fcd28'), 0xd1d1d1d1
        );

        // Event option selected
        $this->initCursor(0, 1);

        // Up button pressed
        $this->initUint32(
            $this->addressOf('_var_peripheral_8c1ba35c') + 16, 1 << 4
        );

        // -- Act ----------------------
        $this->call('_FUN_handleCourseMenuCursor_8c017126');

        // -- Assert -------------------
        $this->shouldWriteLong(
            $this->addressOf('_menuState_8c1bc7a8') + 0x40, 0
        );
        $this->shouldCall('_cursorOffTarget_8c016dc6')->andReturn(0);
    }

    public function resolveSymbols()
    {
        $this->setSize('_var_peripheral_8c1ba35c', 52 * 2);
        $this->setSize('_init_courseMenuButtons_8c04442c', 0x1c * 15);
        $this->setSize('_menuState_8c1bc7a8', 0x6c);
    }
};
