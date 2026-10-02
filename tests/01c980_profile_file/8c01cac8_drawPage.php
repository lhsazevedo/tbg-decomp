<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * drawPage_8c01cac8: for the currently selected grid slot,
 * walks its progress-flag list and draws a checkmark (sprite 9) per flag
 * currently set, then the slot's bio art and the nav-option sprites.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_SpriteDraw_8c014f54', 4);
        $this->setSize('_EventHasProfileProgressFlag_8c02aff0', 4);
    }

    private function selectSlot(int $row, int $col): void
    {
        $menuState = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($menuState + 0x40, $row);
        $this->initUint32($menuState + 0x3c, $col);
    }

    private function setSelectedOption(int $value): void
    {
        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x38, $value);
    }

    public function test_no_flags_set_draws_no_checkmarks(): void
    {
        $this->resolveSymbols();

        // Slot (row 0, col 0) list: 0x32, 0x33, 0x7c, 0x34, 0x7d, 0x35, 0x7e, 0x3b.
        $this->selectSlot(0, 0);
        $this->setSelectedOption(0);

        foreach ([0x32, 0x33, 0x7c, 0x34, 0x7d, 0x35, 0x7e, 0x3b] as $flag) {
            $this->shouldCall('_EventHasProfileProgressFlag_8c02aff0')->with($flag)->andReturn(0);
        }

        $this->expectNavOptionsAndFrame(0);

        $this->singleCall('_drawPage_8c01cac8')->run();
    }

    public function test_draws_checkmark_per_set_flag_in_order(): void
    {
        $this->resolveSymbols();

        // Slot (row 0, col 0) list: 0x32, 0x33, 0x7c, 0x34, 0x7d, 0x35, 0x7e, 0x3b.
        $this->selectSlot(0, 0);
        $this->setSelectedOption(0);

        $resGroup = $this->addressOf('_var_menuState_8c1bc7a8') + 0x0c;
        $flags = [0x32, 0x33, 0x7c, 0x34, 0x7d, 0x35, 0x7e, 0x3b];
        $hits = [1 => true, 4 => true];

        foreach ($flags as $i => $flag) {
            $isHit = $hits[$i] ?? false;
            $this->shouldCall('_EventHasProfileProgressFlag_8c02aff0')->with($flag)->andReturn($isHit ? 1 : 0);
            if ($isHit) {
                $this->shouldCall('_SpriteDraw_8c014f54')->with(
                    $resGroup, 9,
                    96.0 + $i * 38.0, 267.0,
                    -2.0
                );
            }
        }

        // Any hit at all draws the slot's bio art (sprite = grid column).
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $this->addressOf('_var_resourceGroup_8c2263a8'), 0,
            0.0, 0.0, -3.0
        );

        $this->expectNavOptionsAndFrame(0);

        $this->singleCall('_drawPage_8c01cac8')->run();
    }

    public function test_checkmark_row_wraps_after_twelve_columns(): void
    {
        $this->resolveSymbols();

        // Slot (row 5, col 2) list has 36 flags -- long enough to wrap rows
        // (x exceeds 515.0 after the 12th column).
        $this->selectSlot(5, 2);
        $this->setSelectedOption(0);

        $flags = [
            0x40, 0x41, 0x73, 0x85, 0x12, 0x42, 0x4d, 0x74, 0x13, 0x1e, 0x1c, 0x29,
            0x43, 0x4e, 0x44, 0x45, 0x86, 0x72, 0x14, 0x20, 0x21, 0x60, 0x5c, 0x4f,
            0x2a, 0x87, 0x15, 0x1f, 0x10, 0x11, 0x61, 0x5e, 0x92, 0x8f, 0x2b, 0x27,
        ];

        $resGroup = $this->addressOf('_var_menuState_8c1bc7a8') + 0x0c;
        $x = 96.0;
        $y = 267.0;
        foreach ($flags as $i => $flag) {
            $this->shouldCall('_EventHasProfileProgressFlag_8c02aff0')->with($flag)->andReturn(1);
            $this->shouldCall('_SpriteDraw_8c014f54')->with($resGroup, 9, $x, $y, -2.0);
            $x += 38.0;
            if ($x > 515.0) {
                $x = 96.0;
                $y += 34.0;
            }
        }

        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $this->addressOf('_var_resourceGroup_8c2263a8'), 2,
            0.0, 0.0, -3.0
        );

        $this->expectNavOptionsAndFrame(0);

        $this->singleCall('_drawPage_8c01cac8')->run();
    }

    public function test_exit_option_draws_sprite_eleven(): void
    {
        $this->resolveSymbols();

        $this->selectSlot(0, 0);
        $this->setSelectedOption(4);

        foreach ([0x32, 0x33, 0x7c, 0x34, 0x7d, 0x35, 0x7e, 0x3b] as $flag) {
            $this->shouldCall('_EventHasProfileProgressFlag_8c02aff0')->with($flag)->andReturn(0);
        }

        $this->expectNavOptionsAndFrame(4);

        $this->singleCall('_drawPage_8c01cac8')->run();
    }

    private function expectNavOptionsAndFrame(int $selectedOption): void
    {
        $resGroup = $this->addressOf('_var_menuState_8c1bc7a8') + 0x0c;

        if ($selectedOption < 4) { // PAGE_OPTION_EXIT
            $this->shouldCall('_SpriteDraw_8c014f54')->with(
                $resGroup, $selectedOption + 5, 0.0, 0.0, -3.0
            );
            $this->shouldCall('_SpriteDraw_8c014f54')->with(
                $resGroup, 10, 0.0, 0.0, -3.0
            );
        } else {
            $this->shouldCall('_SpriteDraw_8c014f54')->with(
                $resGroup, 11, 0.0, 0.0, -3.0
            );
        }

        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $resGroup, 4, 0.0, 0.0, -4.0
        );
    }
};
