<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _EventScanCandidates_8c02b03c(void): scans the active route's EventEntry
 * table (init_shinjukuEvents_8c04b1f0/abb0/b920, selected by var_route_8c18ad1c) for entries
 * whose timeOfDay_0x00 matches var_timeOfDay_8c18ad20, whose dayMask_0x04
 * contains the current PlayerProgress.days_0x00 value (packed as 5-bit
 * slots), and whose conditions_0x08 (packed 10-bit {mode:2,flag:8} codes,
 * mode 0 = must-not-have, mode 1 = must-have, tested via
 * hasProgressFlag_8c02afbe) are all satisfied. Matching entries' table index
 * is appended to var_eventCandidates_8c228520, counted by var_eventCandidateCount_8c228560. Always resets
 * var_eventCandidateCount_8c228560 to 0 first; does nothing else during PLAY_MODE_PRACTICE.
 */
return new class extends TestCase {
    const ENTRY_SIZE = 0x10;
    const TIME_OF_DAY_0X00 = 0x00;
    const DAY_MASK_0X04 = 0x04;
    const CONDITIONS_0X08 = 0x08;

    private function resolveSymbols(): void
    {
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_runEventFlags_8c1ba2b4', 4);
        $this->setSize('_var_route_8c18ad1c', 4);
        $this->setSize('_var_timeOfDay_8c18ad20', 4);
        $this->setSize('_var_progress_8c1ba1cc', 0xd2);
        $this->setSize('_var_routeEvents_8c22851c', 4);
        $this->setSize('_var_eventCandidates_8c228520', 0x40);
        $this->setSize('_var_eventCandidateCount_8c228560', 4);
    }

    private function initEntry(string $symbol, int $index, int $timeOfDay, int $dayMask, int $conditions): int
    {
        $base = $this->addressOf($symbol) + $index * self::ENTRY_SIZE;
        $this->initUint16($base + self::TIME_OF_DAY_0X00, $timeOfDay);
        $this->initUint32($base + self::DAY_MASK_0X04, $dayMask);
        $this->initUint32($base + self::CONDITIONS_0X08, $conditions);
        return $base;
    }

    private function initSentinel(string $symbol, int $index): void
    {
        $base = $this->addressOf($symbol) + $index * self::ENTRY_SIZE;
        $this->initUint16($base + self::TIME_OF_DAY_0X00, 0xffff);
    }

    private function shouldResetAtStart(): void
    {
        $this->shouldWriteLongTo('_var_eventCandidateCount_8c228560', 0);
        $this->shouldWriteLongTo('_var_runEventFlags_8c1ba2b4', 0);
    }

    public function test_skips_everything_during_practice_mode(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1); // PLAY_MODE_PRACTICE

        $this->call('_EventScanCandidates_8c02b03c');

        $this->shouldWriteLongTo('_var_eventCandidateCount_8c228560', 0);
        $this->forceStop();
    }

    public function test_adds_candidate_when_time_of_day_day_and_conditions_match(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0); // PLAY_MODE_NORMAL
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 0); // ROUTE_SHINJUKU
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0); // TIME_OF_DAY_DAY
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 5); // days_0x00

        $this->initEntry('_init_shinjukuEvents_8c04b1f0', 0, 0, 5, 0);
        $this->initSentinel('_init_shinjukuEvents_8c04b1f0', 1);

        $this->call('_EventScanCandidates_8c02b03c');

        $this->shouldResetAtStart();
        $this->shouldWriteLongTo('_var_routeEvents_8c22851c', $this->addressOf('_init_shinjukuEvents_8c04b1f0'));
        $this->shouldWriteLong($this->addressOf('_var_eventCandidates_8c228520'), 0);
        $this->shouldWriteLongTo('_var_eventCandidateCount_8c228560', 1);
    }

    public function test_selects_wangan_table_when_route_is_wangan(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 1); // ROUTE_WANGAN
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 0);

        $this->initSentinel('_init_wanganEvents_8c04abb0', 0);

        $this->call('_EventScanCandidates_8c02b03c');

        $this->shouldResetAtStart();
        $this->shouldWriteLongTo('_var_routeEvents_8c22851c', $this->addressOf('_init_wanganEvents_8c04abb0'));
    }

    public function test_selects_ome_table_when_route_is_ome(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 2); // ROUTE_OME
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 0);

        $this->initSentinel('_init_omeEvents_8c04b920', 0);

        $this->call('_EventScanCandidates_8c02b03c');

        $this->shouldResetAtStart();
        $this->shouldWriteLongTo('_var_routeEvents_8c22851c', $this->addressOf('_init_omeEvents_8c04b920'));
    }

    public function test_skips_entry_with_mismatched_time_of_day(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 0);
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0); // TIME_OF_DAY_DAY

        $this->initEntry('_init_shinjukuEvents_8c04b1f0', 0, 1, 0, 0); // entry is TIME_OF_DAY_EVENING
        $this->initSentinel('_init_shinjukuEvents_8c04b1f0', 1);

        $this->call('_EventScanCandidates_8c02b03c');

        $this->shouldResetAtStart();
        $this->shouldWriteLongTo('_var_routeEvents_8c22851c', $this->addressOf('_init_shinjukuEvents_8c04b1f0'));
    }

    public function test_skips_entry_with_no_matching_day(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 0);
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 99); // days_0x00

        $this->initEntry('_init_shinjukuEvents_8c04b1f0', 0, 0, 5, 0); // only accepts day 5
        $this->initSentinel('_init_shinjukuEvents_8c04b1f0', 1);

        $this->call('_EventScanCandidates_8c02b03c');

        $this->shouldResetAtStart();
        $this->shouldWriteLongTo('_var_routeEvents_8c22851c', $this->addressOf('_init_shinjukuEvents_8c04b1f0'));
    }

    public function test_skips_entry_when_forbidden_flag_is_set(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 0);
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 5); // days_0x00

        // condition code: mode 0 (must-not-have), flag index 3
        $this->initEntry('_init_shinjukuEvents_8c04b1f0', 0, 0, 5, 3);
        $this->initSentinel('_init_shinjukuEvents_8c04b1f0', 1);

        $this->call('_EventScanCandidates_8c02b03c');

        $this->shouldResetAtStart();
        $this->shouldWriteLongTo('_var_routeEvents_8c22851c', $this->addressOf('_init_shinjukuEvents_8c04b1f0'));
        $this->shouldCall('_hasProgressFlag_8c02afbe')->with(3)->andReturn(1);
    }

    public function test_adds_candidate_when_forbidden_flag_is_not_set(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 0);
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 5); // days_0x00

        // condition code: mode 0 (must-not-have), flag index 3
        $this->initEntry('_init_shinjukuEvents_8c04b1f0', 0, 0, 5, 3);
        $this->initSentinel('_init_shinjukuEvents_8c04b1f0', 1);

        $this->call('_EventScanCandidates_8c02b03c');

        $this->shouldResetAtStart();
        $this->shouldWriteLongTo('_var_routeEvents_8c22851c', $this->addressOf('_init_shinjukuEvents_8c04b1f0'));
        $this->shouldCall('_hasProgressFlag_8c02afbe')->with(3)->andReturn(0);
        $this->shouldWriteLong($this->addressOf('_var_eventCandidates_8c228520'), 0);
        $this->shouldWriteLongTo('_var_eventCandidateCount_8c228560', 1);
    }

    public function test_skips_entry_when_required_flag_is_missing(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 0);
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 5); // days_0x00

        // condition code: mode 1 (must-have), flag index 3 -> 0x103
        $this->initEntry('_init_shinjukuEvents_8c04b1f0', 0, 0, 5, 0x103);
        $this->initSentinel('_init_shinjukuEvents_8c04b1f0', 1);

        $this->call('_EventScanCandidates_8c02b03c');

        $this->shouldResetAtStart();
        $this->shouldWriteLongTo('_var_routeEvents_8c22851c', $this->addressOf('_init_shinjukuEvents_8c04b1f0'));
        $this->shouldCall('_hasProgressFlag_8c02afbe')->with(3)->andReturn(0);
    }

    public function test_adds_candidate_when_required_flag_is_present(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 0);
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0);
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 5); // days_0x00

        // condition code: mode 1 (must-have), flag index 3 -> 0x103
        $this->initEntry('_init_shinjukuEvents_8c04b1f0', 0, 0, 5, 0x103);
        $this->initSentinel('_init_shinjukuEvents_8c04b1f0', 1);

        $this->call('_EventScanCandidates_8c02b03c');

        $this->shouldResetAtStart();
        $this->shouldWriteLongTo('_var_routeEvents_8c22851c', $this->addressOf('_init_shinjukuEvents_8c04b1f0'));
        $this->shouldCall('_hasProgressFlag_8c02afbe')->with(3)->andReturn(1);
        $this->shouldWriteLong($this->addressOf('_var_eventCandidates_8c228520'), 0);
        $this->shouldWriteLongTo('_var_eventCandidateCount_8c228560', 1);
    }
};
