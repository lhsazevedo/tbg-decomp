<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _pickSegmentEvent_8c02b170(void): filters the candidate table indices
 * in var_eventCandidates_8c228520 (built by scanEventCandidates_8c02b03c) down to the ones
 * whose segmentId_0x02 matches var_currentSegment_8c228708 and whose
 * conditions_0x08 var_runEventFlags_8c1ba2b4-bit checks (mode 2 = must-not-have, mode 3 =
 * must-have, tested via hasRunEventFlag_8c02b030) also pass. Picks one at random into
 * var_selectedEventEntry_8c228478 and sets var_cutsceneActive_8c1bb900 = 1;
 * otherwise (or if skipped) sets var_cutsceneActive_8c1bb900 = 0. Skipped
 * entirely outside PLAY_MODE_NORMAL, while var_gameMode_8c1bb8fc != 0, or
 * while var_8c2285dc > var_8c2285d8.
 */
return new class extends TestCase {
    const ENTRY_SIZE = 0x10;
    const SEGMENT_ID_0X02 = 0x02;
    const CONDITIONS_0X08 = 0x08;

    private function resolveSymbols(): void
    {
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_gameMode_8c1bb8fc', 4);
        $this->setSize('_var_8c2285dc', 4);
        $this->setSize('_var_8c2285d8', 4);
        $this->setSize('_var_eventCandidates_8c228520', 0x40);
        $this->setSize('_var_routeEvents_8c22851c', 4);
        $this->setSize('_var_currentSegment_8c228708', 4);
        $this->setSize('_var_eventCandidateCount_8c228560', 4);
        $this->setSize('_var_cutsceneActive_8c1bb900', 4);
        $this->setSize('_var_selectedEventEntry_8c228478', 4);
        $this->setSize('_AsqGetRandomInRangeB_8c0121be', 4);
    }

    private function initGuardsOpen(): void
    {
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0); // PLAY_MODE_NORMAL
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 0);
        $this->initUint32($this->addressOf('_var_8c2285dc'), 1);
        $this->initUint32($this->addressOf('_var_8c2285d8'), 1);
    }

    private function initEntry(int $table, int $index, int $segmentId, int $conditions): void
    {
        $base = $table + $index * self::ENTRY_SIZE;
        $this->initUint16($base + self::SEGMENT_ID_0X02, $segmentId);
        $this->initUint32($base + self::CONDITIONS_0X08, $conditions);
    }

    private function initCandidates(array $tableIndices): int
    {
        $table = $this->alloc((($tableIndices === [] ? 0 : max($tableIndices)) + 1) * self::ENTRY_SIZE);
        $this->initUint32($this->addressOf('_var_routeEvents_8c22851c'), $table);

        $this->initUint32($this->addressOf('_var_eventCandidateCount_8c228560'), count($tableIndices));
        $base = $this->addressOf('_var_eventCandidates_8c228520');
        foreach ($tableIndices as $i => $tableIndex) {
            $this->initUint32($base + $i * 4, $tableIndex);
        }

        return $table;
    }

    public function test_clears_cutscene_when_not_normal_play_mode(): void
    {
        $this->resolveSymbols();

        $this->initGuardsOpen();
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 2); // PLAY_MODE_DEMO

        $this->call('_pickSegmentEvent_8c02b170');

        $this->shouldWriteLongTo('_var_cutsceneActive_8c1bb900', 0);
        $this->forceStop();
    }

    public function test_clears_cutscene_when_game_mode_active(): void
    {
        $this->resolveSymbols();

        $this->initGuardsOpen();
        $this->initUint32($this->addressOf('_var_gameMode_8c1bb8fc'), 1);

        $this->call('_pickSegmentEvent_8c02b170');

        $this->shouldWriteLongTo('_var_cutsceneActive_8c1bb900', 0);
        $this->forceStop();
    }

    public function test_clears_cutscene_when_gate_not_satisfied(): void
    {
        $this->resolveSymbols();

        $this->initGuardsOpen();
        $this->initUint32($this->addressOf('_var_8c2285dc'), 2);
        $this->initUint32($this->addressOf('_var_8c2285d8'), 1);

        $this->call('_pickSegmentEvent_8c02b170');

        $this->shouldWriteLongTo('_var_cutsceneActive_8c1bb900', 0);
        $this->forceStop();
    }

    public function test_clears_cutscene_when_no_candidates(): void
    {
        $this->resolveSymbols();

        $this->initGuardsOpen();
        $this->initCandidates([]);
        $this->initUint32($this->addressOf('_var_currentSegment_8c228708'), 0);

        $this->call('_pickSegmentEvent_8c02b170');

        $this->shouldWriteLongTo('_var_cutsceneActive_8c1bb900', 0);
    }

    public function test_skips_candidate_with_mismatched_segment(): void
    {
        $this->resolveSymbols();

        $this->initGuardsOpen();
        $table = $this->initCandidates([0]);
        $this->initEntry($table, 0, 7, 0);
        $this->initUint32($this->addressOf('_var_currentSegment_8c228708'), 9);

        $this->call('_pickSegmentEvent_8c02b170');

        $this->shouldWriteLongTo('_var_cutsceneActive_8c1bb900', 0);
    }

    public function test_selects_candidate_with_matching_segment_and_no_conditions(): void
    {
        $this->resolveSymbols();

        $this->initGuardsOpen();
        $table = $this->initCandidates([3]);
        $this->initEntry($table, 3, 9, 0);
        $this->initUint32($this->addressOf('_var_currentSegment_8c228708'), 9);

        $this->call('_pickSegmentEvent_8c02b170');

        $this->shouldWriteLongTo('_var_cutsceneActive_8c1bb900', 1);
        $this->shouldCall('_AsqGetRandomInRangeB_8c0121be')->with(1)->andReturn(0);
        $this->shouldWriteLongTo('_var_selectedEventEntry_8c228478', 3);
    }

    public function test_skips_candidate_when_forbidden_bit_is_set(): void
    {
        $this->resolveSymbols();

        $this->initGuardsOpen();
        $table = $this->initCandidates([0]);
        // condition code: mode 2 (must-not-have), flag index 4 -> 0x204
        $this->initEntry($table, 0, 9, 0x204);
        $this->initUint32($this->addressOf('_var_currentSegment_8c228708'), 9);

        $this->call('_pickSegmentEvent_8c02b170');

        $this->shouldCall('_hasRunEventFlag_8c02b030')->with(4)->andReturn(1);
        $this->shouldWriteLongTo('_var_cutsceneActive_8c1bb900', 0);
    }

    public function test_selects_candidate_when_forbidden_bit_is_not_set(): void
    {
        $this->resolveSymbols();

        $this->initGuardsOpen();
        $table = $this->initCandidates([0]);
        // condition code: mode 2 (must-not-have), flag index 4 -> 0x204
        $this->initEntry($table, 0, 9, 0x204);
        $this->initUint32($this->addressOf('_var_currentSegment_8c228708'), 9);

        $this->call('_pickSegmentEvent_8c02b170');

        $this->shouldCall('_hasRunEventFlag_8c02b030')->with(4)->andReturn(0);
        $this->shouldWriteLongTo('_var_cutsceneActive_8c1bb900', 1);
        $this->shouldCall('_AsqGetRandomInRangeB_8c0121be')->with(1)->andReturn(0);
        $this->shouldWriteLongTo('_var_selectedEventEntry_8c228478', 0);
    }

    public function test_selects_candidate_when_required_bit_is_present(): void
    {
        $this->resolveSymbols();

        $this->initGuardsOpen();
        $table = $this->initCandidates([0]);
        // condition code: mode 3 (must-have), flag index 4 -> 0x304
        $this->initEntry($table, 0, 9, 0x304);
        $this->initUint32($this->addressOf('_var_currentSegment_8c228708'), 9);

        $this->call('_pickSegmentEvent_8c02b170');

        $this->shouldCall('_hasRunEventFlag_8c02b030')->with(4)->andReturn(1);
        $this->shouldWriteLongTo('_var_cutsceneActive_8c1bb900', 1);
        $this->shouldCall('_AsqGetRandomInRangeB_8c0121be')->with(1)->andReturn(0);
        $this->shouldWriteLongTo('_var_selectedEventEntry_8c228478', 0);
    }
};
