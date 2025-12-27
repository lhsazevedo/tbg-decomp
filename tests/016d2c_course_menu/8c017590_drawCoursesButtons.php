<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

return new Class extends TestCase {
    public function test_mode_0()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_game_mode_8c1bb8fc'), 0);

        $this->initMenuStateUint32(0x20, fdec(64.0));
        $this->initMenuStateUint32(0x24, fdec(42.0));
        $this->initMenuStateUint32(0x48, 1);

        $this->initCourseMenuButtons([[1, 21], [1, 32]]);
        $this->initVarCourseSettings8c1ba1cc([[1, 0], [2, 0]]);

        $this->call('_drawCourseButtons_8c017590');

        // Unknown sprite
        $this->shouldDrawSprite(0x18, 64.0, 42.0, -3.0);

        // Unknown sprites: first loop
        $this->shouldDrawSprite(21, 0.0, 0.0, -4.0);
        $this->shouldDrawSprite(32, 0.0, 0.0, -4.0);

        // Unknown sprites: second loop
        $this->shouldDrawSprite(0x17, 240.0, 106.0, -3.5);
        $this->shouldDrawSprite(0x16, 333.0, 106.0, -3.5);
    }

    public function test_mode_1_no_first_sprite()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_game_mode_8c1bb8fc'), 1);

        $this->initMenuStateUint32(0x20, fdec(64.0));
        $this->initMenuStateUint32(0x24, fdec(42.0));
        $this->initMenuStateUint32(0x48, 0);

        $this->initCourseMenuButtons([[1, 21], [1, 32]]);
        $this->initVarCourseSettings8c1ba1cc([[0, 1], [0, 2]]);

        $this->call('_drawCourseButtons_8c017590');

        // Unknown sprite
        // $this->shouldDrawSprite(0x18, 64.0, 42.0, -3.0);

        // Unknown sprites: first loop
        $this->shouldDrawSprite(21, 0.0, 0.0, -4.0);
        $this->shouldDrawSprite(32, 0.0, 0.0, -4.0);

        // Unknown sprites: second loop
        $this->shouldDrawSprite(0x17, 240.0, 106.0, -3.5);
        $this->shouldDrawSprite(0x16, 333.0, 106.0, -3.5);
    }

    public function test_skips_button_when_unlocked_but_sprite_is_zero()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_game_mode_8c1bb8fc'), 0);

        $this->initMenuStateUint32(0x20, fdec(64.0));
        $this->initMenuStateUint32(0x24, fdec(42.0));
        $this->initMenuStateUint32(0x48, 0);

        // First button: unlocked=1, sprite=0 (should skip)
        // Second button: unlocked=1, sprite=21 (should draw)
        $this->initCourseMenuButtons([[1, 0], [1, 21]]);
        $this->initVarCourseSettings8c1ba1cc([[0, 0], [0, 0]]);

        $this->call('_drawCourseButtons_8c017590');

        // First button should be skipped (unlocked but sprite=0)
        // Only second button should draw
        $this->shouldDrawSprite(21, 0.0, 0.0, -4.0);
    }

    public function test_skips_button_when_locked_even_if_sprite_is_set()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_game_mode_8c1bb8fc'), 0);

        $this->initMenuStateUint32(0x20, fdec(64.0));
        $this->initMenuStateUint32(0x24, fdec(42.0));
        $this->initMenuStateUint32(0x48, 0);

        // First button: unlocked=0, sprite=21 (should skip)
        // Second button: unlocked=1, sprite=32 (should draw)
        $this->initCourseMenuButtons([[0, 21], [1, 32]]);
        $this->initVarCourseSettings8c1ba1cc([[0, 0], [0, 0]]);

        $this->call('_drawCourseButtons_8c017590');

        // First button should be skipped (locked even though sprite is set)
        // Only second button should draw
        $this->shouldDrawSprite(32, 0.0, 0.0, -4.0);
    }

    public function test_skips_all_buttons_when_none_meet_criteria()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_game_mode_8c1bb8fc'), 0);

        $this->initMenuStateUint32(0x20, fdec(64.0));
        $this->initMenuStateUint32(0x24, fdec(42.0));
        $this->initMenuStateUint32(0x48, 0);

        // Mix of locked/zero sprite buttons - all should skip
        $this->initCourseMenuButtons([[0, 0], [1, 0], [0, 21]]);
        $this->initVarCourseSettings8c1ba1cc([[0, 0], [0, 0], [0, 0]]);

        $this->call('_drawCourseButtons_8c017590');

        // No button sprites should be drawn (all skipped by the condition)
        // No shouldDrawSprite calls expected for the first loop
    }

    private function initMenuStateUint32($offset, $value) {
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + $offset, $value);
    }

    private function initCourseMenuButtons(array $values) {
        $base = $this->addressOf('_init_courseMenuButtons_8c04442c');
        array_push($values, ...array_fill(0, 15 - count($values), [0, 0]));

        foreach ($values as $i => $struct) {
            $this->initUint32($base + $i * 0x1c + 0x04, $struct[0]); // unlocked_0x04
            $this->initUint32($base + $i * 0x1c + 0x10, $struct[1]); // spriteNo_0x10
        }
    }

    private function initVarCourseSettings8c1ba1cc(array $values)
    {
        array_push($values, ...array_fill(0, 9 - count($values), [0, 0]));

        foreach ($values as $index => $value) {
            $this->initUint8(
                $this->addressOf('_var_progress_8c1ba1cc') + 0x44 + $index * 8 + 3,
                $value[0],
            );
            $this->initUint8(
                $this->addressOf('_var_progress_8c1ba1cc') + 0x44 + $index * 8 + 4,
                $value[1],
            );
        }
    }

    private function shouldDrawSprite(int $spriteNo, float $x, float $y, float $priority) {
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            $spriteNo,
            $x,
            $y,
            $priority,
        );
    }

    private function resolveSymbols() {
        $this->setSize('_drawSprite_8c014f54', 0x04);
        $this->setSize('_menuState_8c1bc7a8', 0x84);
        $this->setSize('_init_courseMenuButtons_8c04442c', 0x1c * 15);
        $this->setSize('_var_progress_8c1ba1cc', 0x94);
        $this->setSize('__modls', 0x04);
        $this->setSize('__divls', 0x04);
    }
};
