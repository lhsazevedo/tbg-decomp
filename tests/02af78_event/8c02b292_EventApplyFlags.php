<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _EventApplyFlags_8c02b292(void): applies the actions_0x0c codes of
 * the EventEntry chosen by EventPickForSegment_8c02b170
 * (var_selectedEventEntry_8c228478, indexing into the table pointed to by
 * var_routeEvents_8c22851c). Each packed 10-bit code either sets a progress flag
 * (setProgressFlag_8c02af78, mode clear) or a var_runEventFlags_8c1ba2b4 bit
 * (setRunEventFlag_8c02b022, mode 0x200), until a 0/padding (0x3ff) slot ends the list.
 */
return new class extends TestCase {
    const ENTRY_SIZE = 0x10;
    const ACTIONS_0X0C = 0x0c;

    private function resolveSymbols(): void
    {
        $this->setSize('_var_routeEvents_8c22851c', 4);
        $this->setSize('_var_selectedEventEntry_8c228478', 4);
    }

    private function initEntry(int $index, int $actions): int
    {
        $table = $this->alloc(($index + 1) * self::ENTRY_SIZE);
        $this->initUint32($table + $index * self::ENTRY_SIZE + self::ACTIONS_0X0C, $actions);
        $this->initUint32($this->addressOf('_var_routeEvents_8c22851c'), $table);
        $this->initUint32($this->addressOf('_var_selectedEventEntry_8c228478'), $index);
        return $table;
    }

    public function test_sets_progress_flag_for_mode_clear(): void
    {
        $this->resolveSymbols();

        // action code: mode clear, flag index 5
        $this->initEntry(0, 5);

        $this->call('_EventApplyFlags_8c02b292');

        $this->shouldCall('_setProgressFlag_8c02af78')->with(5);
    }

    public function test_sets_8c1ba2b4_bit_for_mode_0x200(): void
    {
        $this->resolveSymbols();

        // action code: mode 0x200, flag index 7
        $this->initEntry(0, 0x207);

        $this->call('_EventApplyFlags_8c02b292');

        $this->shouldCall('_setRunEventFlag_8c02b022')->with(7);
    }

    public function test_applies_multiple_packed_actions_in_order(): void
    {
        $this->resolveSymbols();

        // two codes: flag 2 (mode clear), then var_runEventFlags_8c1ba2b4 bit 9 (mode 0x200)
        $this->initEntry(0, (0x209 << 10) | 2);

        $this->call('_EventApplyFlags_8c02b292');

        $this->shouldCall('_setProgressFlag_8c02af78')->with(2);
        $this->shouldCall('_setRunEventFlag_8c02b022')->with(9);
    }

    public function test_skips_padding_slot(): void
    {
        $this->resolveSymbols();

        // padding (0x3ff), then flag 4 (mode clear)
        $this->initEntry(0, (4 << 10) | 0x3ff);

        $this->call('_EventApplyFlags_8c02b292');

        $this->shouldCall('_setProgressFlag_8c02af78')->with(4);
    }

    public function test_does_nothing_when_no_actions(): void
    {
        $this->resolveSymbols();

        $this->initEntry(0, 0);

        $this->call('_EventApplyFlags_8c02b292');
    }

    public function test_uses_selected_index_into_table(): void
    {
        $this->resolveSymbols();

        // entry at table index 2 has the actions; other slots are padding
        $table = $this->alloc(3 * self::ENTRY_SIZE);
        $this->initUint32($table + 0 * self::ENTRY_SIZE + self::ACTIONS_0X0C, 0x3ff);
        $this->initUint32($table + 1 * self::ENTRY_SIZE + self::ACTIONS_0X0C, 0x3ff);
        $this->initUint32($table + 2 * self::ENTRY_SIZE + self::ACTIONS_0X0C, 6);
        $this->initUint32($this->addressOf('_var_routeEvents_8c22851c'), $table);
        $this->initUint32($this->addressOf('_var_selectedEventEntry_8c228478'), 2);

        $this->call('_EventApplyFlags_8c02b292');

        $this->shouldCall('_setProgressFlag_8c02af78')->with(6);
    }
};
