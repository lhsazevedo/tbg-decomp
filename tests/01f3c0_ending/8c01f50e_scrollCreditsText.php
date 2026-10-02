<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * scrollCreditsText_8c01f50e(void): scrolls the double-buffered credit
 * textboxes up by 2px/frame (field_0x5c selects which of the pair is
 * "current"); wraps and loads the next credit page once the current box
 * scrolls fully off-screen. init_endingCreditsHead_8c04528c and
 * init_endingCreditsTail_8c045290 are laid out back to back in the real
 * binary, so counter_0x64 indexes across both as one 38-entry table.
 */
return new class extends TestCase {
    const MENU_STATE_SIZE = 0x7c;
    const FIELD_0X1C = 0x1c;
    const FIELD_0X54 = 0x54;
    const FIELD_0X58 = 0x58;
    const FIELD_0X5C = 0x5c;
    const STARTTIMER_0X64 = 0x64;
    const TEXTBOX_SIZE = 0x3c;
    const TEXTBOX_Y_0X04 = 0x04;

    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', self::MENU_STATE_SIZE);
        $this->setSize('_TxtPrepareTextBoxLayout_8c01543a', 4);
        $this->setSize('_TxtDrawTextBox_8c0155e0', 4);

        // var_messageTextBoxA/B are consecutive words in the real binary and
        // are external to this unit (defined in 0129cc_game.c) -- force the
        // same adjacency here since the linker doesn't place separately
        // declared symbols next to each other by default.
        $mbBase = $this->addressOf('_var_messageTextBoxA_8c1bc404');
        $this->rellocate('_var_messageTextBoxB_8c1bc408', $mbBase + 4);

        // init_endingCreditsHead_8c04528c/init_endingCreditsTail_8c045290 are defined by *this* unit (real
        // internal relocations, not test-relocatable), so no rellocate is
        // needed or possible here -- the C/asm each place them wherever
        // their own compiler/assembler naturally lays out consecutive
        // rodata declarations.
    }

    private function seedState(int $phase, int $idx, int $creditIndex, int $timerA, int $timerB): int
    {
        $base = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->initUint32($base + self::FIELD_0X1C, $phase);
        $this->initUint32($base + self::FIELD_0X54, $timerA);
        $this->initUint32($base + self::FIELD_0X58, $timerB);
        $this->initUint32($base + self::FIELD_0X5C, $idx);
        $this->initUint32($base + self::STARTTIMER_0X64, $creditIndex);
        return $base;
    }

    private function makeBoxes(): array
    {
        $box0 = $this->alloc(self::TEXTBOX_SIZE);
        $box1 = $this->alloc(self::TEXTBOX_SIZE);
        $this->initUint32($this->addressOf('_var_messageTextBoxA_8c1bc404'), $box0);
        $this->initUint32($this->addressOf('_var_messageTextBoxB_8c1bc408'), $box1);
        return [$box0, $box1];
    }

    // The credit text pointer is a compile-time constant, but not
    // expressible as an ASCII PHP literal (the strings are Shift-JIS) or
    // reachable via addressOf (anonymous string literals have no symbol) --
    // read it back at call time instead, inside the callback where
    // $this->memory is populated. $index is the combined table index
    // (0 = head[0], 1.. = tail[index - 1]).
    private function checkPrepareLayoutArgs(int $box, int $index): callable
    {
        $readAddr = $index === 0
            ? $this->addressOf('_init_endingCreditsHead_8c04528c')
            : $this->addressOf('_init_endingCreditsTail_8c045290') + ($index - 1) * 4;

        return function () use ($box, $index, $readAddr) {
            $r4 = $this->registers[4]->value;
            $r5 = $this->registers[5]->value;
            $expected = $this->memory->readUInt32($readAddr)->value;
            if ($r4 !== $box || $r5 !== $expected) {
                throw new \Exception(sprintf(
                    "Unexpected TxtPrepareTextBoxLayout args: box=0x%x text=0x%x, expecting box=0x%x text=0x%x (combined[%d])",
                    $r4, $r5, $box, $expected, $index
                ));
            }
        };
    }

    public function test_no_more_credits_only_scrolls_and_draws(): void
    {
        $this->resolveSymbols();
        $base = $this->seedState(1, 0, 2, 10, 20);
        [$box0, $box1] = $this->makeBoxes();
        $this->initUint32($box0 + self::TEXTBOX_Y_0X04, 100);
        $this->initUint32($box1 + self::TEXTBOX_Y_0X04, 200);

        $this->call('_scrollCreditsText_8c01f50e');

        $this->shouldWriteLong($base + self::FIELD_0X54, 11);
        $this->shouldWriteLong($base + self::FIELD_0X58, 21);
        $this->shouldCall('_TxtDrawTextBox_8c0155e0')->with($box0, 11)->andReturn(0);
        $this->shouldCall('_TxtDrawTextBox_8c0155e0')->with($box1, 21)->andReturn(5);
        $this->shouldReturn(5);
    }

    public function test_scroll_no_wrap_decrements_both_boxes(): void
    {
        $this->resolveSymbols();
        $base = $this->seedState(0, 0, 2, 0, 0);
        [$box0, $box1] = $this->makeBoxes();
        $this->initUint32($box0 + self::TEXTBOX_Y_0X04, 100);
        $this->initUint32($box1 + self::TEXTBOX_Y_0X04, 200);

        $this->call('_scrollCreditsText_8c01f50e');

        $this->shouldWriteLong($box0 + self::TEXTBOX_Y_0X04, 98);
        $this->shouldWriteLong($box1 + self::TEXTBOX_Y_0X04, 198);
        $this->shouldWriteLong($base + self::FIELD_0X54, 1);
        $this->shouldWriteLong($base + self::FIELD_0X58, 1);
        $this->shouldCall('_TxtDrawTextBox_8c0155e0')->with($box0, 1)->andReturn(0);
        $this->shouldCall('_TxtDrawTextBox_8c0155e0')->with($box1, 1)->andReturn(0);
    }

    public function test_current_box_wraps_and_loads_next_credit_idx0(): void
    {
        $this->resolveSymbols();
        $base = $this->seedState(0, 0, 2, 5, 9);
        [$box0, $box1] = $this->makeBoxes();
        // -478 - 2 = -480, not < -480 yet -- use -479 so post-decrement (-481) wraps.
        $this->initUint32($box0 + self::TEXTBOX_Y_0X04, -479 & 0xffffffff);
        $this->initUint32($box1 + self::TEXTBOX_Y_0X04, 50);

        $this->call('_scrollCreditsText_8c01f50e');

        $this->shouldWriteLong($box0 + self::TEXTBOX_Y_0X04, -481 & 0xffffffff);
        $this->shouldWriteLong($box1 + self::TEXTBOX_Y_0X04, 48);
        $this->shouldWriteLong($box0 + self::TEXTBOX_Y_0X04, (-481 + 960) & 0xffffffff);
        $this->shouldCall('_TxtPrepareTextBoxLayout_8c01543a')
            ->do($this->checkPrepareLayoutArgs($box0, 2))
            ->andReturn(1);
        $this->shouldWriteLong($base + self::FIELD_0X54, 0);
        $this->shouldWriteLong($base + self::FIELD_0X5C, 1);
        $this->shouldWriteLong($base + self::STARTTIMER_0X64, 3);
        $this->shouldWriteLong($base + self::FIELD_0X54, 1);
        $this->shouldWriteLong($base + self::FIELD_0X58, 10);
        $this->shouldCall('_TxtDrawTextBox_8c0155e0')->with($box0, 1)->andReturn(0);
        $this->shouldCall('_TxtDrawTextBox_8c0155e0')->with($box1, 10)->andReturn(0);
    }

    public function test_current_box_wraps_and_loads_next_credit_idx1(): void
    {
        $this->resolveSymbols();
        $base = $this->seedState(0, 1, 1, 5, 9);
        [$box0, $box1] = $this->makeBoxes();
        $this->initUint32($box0 + self::TEXTBOX_Y_0X04, 50);
        $this->initUint32($box1 + self::TEXTBOX_Y_0X04, -479 & 0xffffffff);

        // Index 1 spills past the head's single entry into the tail.
        $this->call('_scrollCreditsText_8c01f50e');

        $this->shouldWriteLong($box1 + self::TEXTBOX_Y_0X04, -481 & 0xffffffff);
        $this->shouldWriteLong($box0 + self::TEXTBOX_Y_0X04, 48);
        $this->shouldWriteLong($box1 + self::TEXTBOX_Y_0X04, (-481 + 960) & 0xffffffff);
        $this->shouldCall('_TxtPrepareTextBoxLayout_8c01543a')
            ->do($this->checkPrepareLayoutArgs($box1, 1))
            ->andReturn(1);
        $this->shouldWriteLong($base + self::FIELD_0X58, 0);
        $this->shouldWriteLong($base + self::FIELD_0X5C, 0);
        $this->shouldWriteLong($base + self::STARTTIMER_0X64, 2);
        $this->shouldWriteLong($base + self::FIELD_0X54, 6);
        $this->shouldWriteLong($base + self::FIELD_0X58, 1);
        $this->shouldCall('_TxtDrawTextBox_8c0155e0')->with($box0, 6)->andReturn(0);
        $this->shouldCall('_TxtDrawTextBox_8c0155e0')->with($box1, 1)->andReturn(0);
    }

    public function test_wrap_with_no_more_credits_latches_subState_0x1c(): void
    {
        $this->resolveSymbols();
        $base = $this->seedState(0, 0, 37, 0, 0);
        [$box0, $box1] = $this->makeBoxes();
        $this->initUint32($box0 + self::TEXTBOX_Y_0X04, -479 & 0xffffffff);
        $this->initUint32($box1 + self::TEXTBOX_Y_0X04, 50);

        $this->call('_scrollCreditsText_8c01f50e');

        $this->shouldWriteLong($box0 + self::TEXTBOX_Y_0X04, -481 & 0xffffffff);
        $this->shouldWriteLong($box1 + self::TEXTBOX_Y_0X04, 48);
        $this->shouldWriteLong($box0 + self::TEXTBOX_Y_0X04, (-481 + 960) & 0xffffffff);
        $this->shouldCall('_TxtPrepareTextBoxLayout_8c01543a')
            ->do($this->checkPrepareLayoutArgs($box0, 37))
            ->andReturn(0);
        $this->shouldWriteLong($base + self::FIELD_0X1C, 1);
        $this->shouldWriteLong($base + self::FIELD_0X54, 1);
        $this->shouldWriteLong($base + self::FIELD_0X58, 1);
        $this->shouldCall('_TxtDrawTextBox_8c0155e0')->with($box0, 1)->andReturn(0);
        $this->shouldCall('_TxtDrawTextBox_8c0155e0')->with($box1, 1)->andReturn(0);
    }

    public function test_timer_clamps_at_0xff(): void
    {
        $this->resolveSymbols();
        $base = $this->seedState(1, 0, 2, 0xff, 0xff);
        [$box0, $box1] = $this->makeBoxes();
        $this->initUint32($box0 + self::TEXTBOX_Y_0X04, 100);
        $this->initUint32($box1 + self::TEXTBOX_Y_0X04, 200);

        $this->call('_scrollCreditsText_8c01f50e');

        // The pre-increment writes 0x100 unconditionally, then the clamp
        // check overwrites it back down to 0xff.
        $this->shouldWriteLong($base + self::FIELD_0X54, 0x100);
        $this->shouldWriteLong($base + self::FIELD_0X54, 0xff);
        $this->shouldWriteLong($base + self::FIELD_0X58, 0x100);
        $this->shouldWriteLong($base + self::FIELD_0X58, 0xff);
        $this->shouldCall('_TxtDrawTextBox_8c0155e0')->with($box0, 0xff)->andReturn(0);
        $this->shouldCall('_TxtDrawTextBox_8c0155e0')->with($box1, 0xff)->andReturn(0);
    }
};
