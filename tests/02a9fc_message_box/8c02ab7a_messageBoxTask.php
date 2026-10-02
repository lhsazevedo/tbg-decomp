<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // messageBoxTask state offsets (word index * 4).
    const ST_PHASE = 0x00;
    const ST_PAGE_COUNT = 0x04;
    const ST_PAGE_INDEX = 0x08;
    const ST_FRAME_COUNTER = 0x0c;
    const ST_ROW = 0x10;
    const ST_IDS = 0x14;
    const ST_MSG = 0x18;

    private function allocPeripheral(int $on = 0, int $press = 0)
    {
        $periph = $this->alloc(0x34);
        $this->initUint32($periph + 0x08, $on);
        $this->initUint32($periph + 0x10, $press);
        $this->initUint32($this->addressOf('_var_peripheral_8c1ba358'), $periph);
        return $periph;
    }

    // A 0xffff-terminated ids array with no entries, so the shared draw tail
    // falls straight through to menuTextboxText/SndPollVoiceEnd_8c0106ac.
    private function allocEmptyIds(): int
    {
        $ids = $this->alloc(2);
        $this->initUint16($ids, 0xffff);
        return $ids;
    }

    private function allocState(
        int $phase,
        int $pageCount = 0,
        int $pageIndex = 0,
        int $frameCounter = 0,
    ): int {
        $state = $this->alloc(0x1c);
        $this->initUint32($state + self::ST_PHASE, $phase);
        $this->initUint32($state + self::ST_PAGE_COUNT, $pageCount);
        $this->initUint32($state + self::ST_PAGE_INDEX, $pageIndex);
        $this->initUint32($state + self::ST_FRAME_COUNTER, $frameCounter);
        $this->initUint32($state + self::ST_ROW, 0);
        $this->initUint32($state + self::ST_IDS, $this->allocEmptyIds());
        $this->initUint32($state + self::ST_MSG, 0);
        $this->initUint32($this->addressOf('_var_messageAssetCount_8c228514'), 0);
        // The peripheral pointer is loaded unconditionally near the top of
        // the function, even on paths that never read through it.
        $this->allocPeripheral();
        return $state;
    }

    private function expectDrawEmpty(int $state, int $pageIndex): void
    {
        $this->shouldWriteFloat($this->addressOf('_init_msgScroll_8c04ab3c') + 0x24, -3.0);
        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with($pageIndex);
        $this->shouldCall('_SndPollVoiceEnd_8c0106ac');
    }

    public function test_phase0_swaps_in_message_and_waits(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->alloc(0x1c);
        $this->initUint32($state + self::ST_PHASE, 0);

        // Row = {ids*, groupIndex}.
        $idsArray = $this->alloc(2);
        $this->initUint16($idsArray, 0xffff);
        $row = $this->alloc(8);
        $this->initUint32($row, $idsArray);
        $this->initUint32($row + 0x04, 0);
        $this->initUint32($state + self::ST_ROW, $row);

        // var_messageTextDat_8c228518 groups table: index 0 -> msgData.
        $string = $this->allocString('Hello');
        $msgData = $this->alloc(8);
        $this->initUint32($msgData, $string);
        $this->initUint32($msgData + 0x04, 0x1234);
        $groupsTable = $this->alloc(4);
        $this->initUint32($groupsTable, $msgData);
        $this->initUint32($this->addressOf('_var_messageTextDat_8c228518'), $groupsTable);

        $this->allocPeripheral(on: 0, press: 0);
        $this->initUint32($this->addressOf('_var_messageAssetCount_8c228514'), 0);

        $this->call('_messageBoxTask_8c02ab7a')->with($task, $state);

        $this->shouldWriteLong($state + self::ST_IDS, $idsArray);
        $this->shouldWriteLong($state + self::ST_MSG, $msgData);

        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with($string)->andReturn(5);
        $this->shouldWriteLong($state + self::ST_PAGE_COUNT, 5);
        $this->shouldCall('_SndPlayAdx_8c010cd6')->with(2, 0x1234);

        $this->shouldWriteLong($state + self::ST_PAGE_INDEX, 1);
        $this->shouldWriteLong($state + self::ST_FRAME_COUNTER, 0);
        $this->shouldWriteLong($state + self::ST_PHASE, 2);

        // Wait: no button press, frame counter increments and stays < 3.
        $this->shouldWriteLong($state + self::ST_FRAME_COUNTER, 1);

        // Draw: pr is reset unconditionally, then ids array is immediately
        // 0xffff-terminated.
        $this->shouldWriteFloat($this->addressOf('_init_msgScroll_8c04ab3c') + 0x24, -3.0);
        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(1);
        $this->shouldCall('_SndPollVoiceEnd_8c0106ac');
    }

    public function test_phase1_reswaps_current_message(): void
    {
        // Phase 1 re-swaps in the message state[6] already points at
        // (used when advancing to the next message within the same group),
        // skipping the state[5]/state[6] row lookup that phase 0 does.
        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 1);

        $string = $this->allocString('World');
        $msgData = $this->alloc(8);
        $this->initUint32($msgData, $string);
        $this->initUint32($msgData + 0x04, 0x5678);
        $this->initUint32($state + self::ST_MSG, $msgData);

        $this->call('_messageBoxTask_8c02ab7a')->with($task, $state);

        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with($string)->andReturn(3);
        $this->shouldWriteLong($state + self::ST_PAGE_COUNT, 3);
        $this->shouldCall('_SndPlayAdx_8c010cd6')->with(2, 0x5678);

        $this->shouldWriteLong($state + self::ST_PAGE_INDEX, 1);
        $this->shouldWriteLong($state + self::ST_FRAME_COUNTER, 0);
        $this->shouldWriteLong($state + self::ST_PHASE, 2);

        $this->shouldWriteLong($state + self::ST_FRAME_COUNTER, 1);
        $this->expectDrawEmpty($state, 1);
    }

    public function test_phase2_button_press_starts_fast_forward(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 2, pageCount: 5, pageIndex: 1, frameCounter: 0);
        $this->allocPeripheral(on: 0, press: 4 /* PDD_DGT_TA */);

        $this->call('_messageBoxTask_8c02ab7a')->with($task, $state);

        $this->shouldWriteLong($state + self::ST_FRAME_COUNTER, 99);
        $this->shouldWriteLong($state + self::ST_PHASE, 3);
        $this->shouldCall('_SndStopAdx_8c010ca6')->with(1);
        $this->shouldWriteLong($state + self::ST_FRAME_COUNTER, 100);
        $this->shouldWriteLong($state + self::ST_PAGE_INDEX, 2);
        $this->shouldWriteLong($state + self::ST_FRAME_COUNTER, 0);
        $this->expectDrawEmpty($state, 2);
    }

    public function test_phase2_no_press_keeps_waiting(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 2, pageCount: 5, pageIndex: 1, frameCounter: 1);
        $this->allocPeripheral(on: 0, press: 0);

        $this->call('_messageBoxTask_8c02ab7a')->with($task, $state);

        $this->shouldWriteLong($state + self::ST_FRAME_COUNTER, 2);
        $this->expectDrawEmpty($state, 1);
    }

    public function test_phase2_page_advance_ends_message(): void
    {
        $task = $this->alloc(0x20);
        // frameCounter=2 -> increments to 3 (not < 3); pageIndex=4,
        // pageCount=5 -> increments to 5 (not < 5) -> ends the message.
        $state = $this->allocState(phase: 2, pageCount: 5, pageIndex: 4, frameCounter: 2);
        $this->allocPeripheral(on: 0, press: 0);

        $this->call('_messageBoxTask_8c02ab7a')->with($task, $state);

        $this->shouldWriteLong($state + self::ST_FRAME_COUNTER, 3);
        $this->shouldWriteLong($state + self::ST_PAGE_INDEX, 5);
        $this->shouldWriteLong($state + self::ST_PHASE, 4);
        $this->expectDrawEmpty($state, 5);
    }

    public function test_phase2_page_advance_continues(): void
    {
        // frameCounter reaches 3; pageIndex=1, pageCount=5 -> still < 5,
        // frameCounter resets to 0 and waiting continues.
        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 2, pageCount: 5, pageIndex: 1, frameCounter: 2);
        $this->allocPeripheral(on: 0, press: 0);

        $this->call('_messageBoxTask_8c02ab7a')->with($task, $state);

        $this->shouldWriteLong($state + self::ST_FRAME_COUNTER, 3);
        $this->shouldWriteLong($state + self::ST_PAGE_INDEX, 2);
        $this->shouldWriteLong($state + self::ST_FRAME_COUNTER, 0);
        $this->expectDrawEmpty($state, 2);
    }

    public function test_phase3_button_released_returns_to_wait(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 3, pageCount: 5, pageIndex: 1, frameCounter: 99);
        $this->allocPeripheral(on: 0, press: 0);

        $this->call('_messageBoxTask_8c02ab7a')->with($task, $state);

        $this->shouldWriteLong($state + self::ST_PHASE, 2);
        $this->expectDrawEmpty($state, 1);
    }

    public function test_phase3_button_held_fast_forwards(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 3, pageCount: 5, pageIndex: 1, frameCounter: 99);
        $this->allocPeripheral(on: 4 /* PDD_DGT_TA */, press: 0);

        $this->call('_messageBoxTask_8c02ab7a')->with($task, $state);

        $this->shouldWriteLong($state + self::ST_PAGE_INDEX, 3);
        $this->expectDrawEmpty($state, 3);
    }

    public function test_phase3_button_held_ends_message(): void
    {
        // pageIndex=4, pageCount=5 -> +2 = 6, not < 5 -> ends the message.
        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 3, pageCount: 5, pageIndex: 4, frameCounter: 99);
        $this->allocPeripheral(on: 4 /* PDD_DGT_TA */, press: 0);

        $this->call('_messageBoxTask_8c02ab7a')->with($task, $state);

        $this->shouldWriteLong($state + self::ST_PAGE_INDEX, 6);
        $this->shouldWriteLong($state + self::ST_PHASE, 4);
        $this->expectDrawEmpty($state, 6);
    }

    public function test_phase4_no_press_just_draws(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 4, pageCount: 5, pageIndex: 1, frameCounter: 0);
        $this->allocPeripheral(on: 0, press: 0);

        $this->call('_messageBoxTask_8c02ab7a')->with($task, $state);

        $this->expectDrawEmpty($state, 1);
    }

    public function test_phase4_press_advances_to_next_message_in_group(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 4, pageCount: 5, pageIndex: 1, frameCounter: 0);
        $this->allocPeripheral(on: 0, press: 4 /* PDD_DGT_TA */);

        // Current group entry {string, value}, followed by another
        // non-empty entry.
        $nextString = $this->allocString('More');
        $msgData = $this->alloc(0x10);
        $this->initUint32($msgData, $this->allocString('Current'));
        $this->initUint32($msgData + 0x04, 0);
        $this->initUint32($msgData + 0x08, $nextString);
        $this->initUint32($msgData + 0x0c, 0);
        $this->initUint32($state + self::ST_MSG, $msgData);

        $this->call('_messageBoxTask_8c02ab7a')->with($task, $state);

        $this->shouldCall('_SndStopAdx_8c010ca6')->with(1);
        $this->shouldWriteLong($state + self::ST_MSG, $msgData + 0x08);
        $this->shouldWriteLong($state + self::ST_PHASE, 1);
        $this->expectDrawEmpty($state, 1);
    }

    public function test_phase4_press_advances_to_next_group_in_row(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 4, pageCount: 5, pageIndex: 1, frameCounter: 0);
        $this->allocPeripheral(on: 0, press: 4 /* PDD_DGT_TA */);

        // Current group's next entry is the empty-string terminator.
        $msgData = $this->alloc(0x10);
        $this->initUint32($msgData, $this->allocString('Current'));
        $this->initUint32($msgData + 0x04, 0);
        $this->initUint32($msgData + 0x08, $this->allocString(''));
        $this->initUint32($msgData + 0x0c, 0);
        $this->initUint32($state + self::ST_MSG, $msgData);

        // Row's next entry is a real ids pointer (not -1).
        $row = $this->alloc(0x10);
        $this->initUint32($row, 0);
        $this->initUint32($row + 0x04, 0);
        $this->initUint32($row + 0x08, 0x11223344);
        $this->initUint32($row + 0x0c, 0);
        $this->initUint32($state + self::ST_ROW, $row);

        $this->call('_messageBoxTask_8c02ab7a')->with($task, $state);

        $this->shouldCall('_SndStopAdx_8c010ca6')->with(1);
        $this->shouldWriteLong($state + self::ST_MSG, $msgData + 0x08);
        $this->shouldWriteLong($state + self::ST_ROW, $row + 0x08);
        $this->shouldWriteLong($state + self::ST_PHASE, 0);
        $this->expectDrawEmpty($state, 1);
    }

    public function test_phase4_press_ends_row(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 4, pageCount: 5, pageIndex: 1, frameCounter: 0);
        $this->allocPeripheral(on: 0, press: 4 /* PDD_DGT_TA */);

        $msgData = $this->alloc(0x10);
        $this->initUint32($msgData, $this->allocString('Current'));
        $this->initUint32($msgData + 0x04, 0);
        $this->initUint32($msgData + 0x08, $this->allocString(''));
        $this->initUint32($msgData + 0x0c, 0);
        $this->initUint32($state + self::ST_MSG, $msgData);

        // Row's next entry is the -1 sentinel.
        $row = $this->alloc(0x10);
        $this->initUint32($row, 0);
        $this->initUint32($row + 0x04, 0);
        $this->initUint32($row + 0x08, -1);
        $this->initUint32($row + 0x0c, 0);
        $this->initUint32($state + self::ST_ROW, $row);

        $this->call('_messageBoxTask_8c02ab7a')->with($task, $state);

        $this->shouldCall('_SndStopAdx_8c010ca6')->with(1);
        $this->shouldWriteLong($state + self::ST_MSG, $msgData + 0x08);
        $this->shouldWriteLong($state + self::ST_ROW, $row + 0x08);
        $this->shouldWriteLongTo('_var_fadeRequest_8c226564', 2 /* FADE_REQUEST_OUT */);
        $this->shouldWriteLong($state + self::ST_PHASE, 5);
        $this->expectDrawEmpty($state, 1);
    }

    public function test_phase5_still_fading_falls_through_to_draw(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 5, pageIndex: 3);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $this->call('_messageBoxTask_8c02ab7a')->with($task, $state);

        $this->expectDrawEmpty($state, 3);
    }

    public function test_phase5_fade_complete_frees_task_and_cleans_up(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 5);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);

        $this->call('_messageBoxTask_8c02ab7a')->with($task, $state);

        $this->shouldCall('_MessageBoxFreeAssets_8c02adee');
        $this->shouldCall('_TaskKill_8c014b66')->with($task);
        $this->shouldCall('_EventApplyFlags_8c02b292');
        $this->shouldCall('_RouteStartModelLoadPass_8c013d78');
        $this->shouldWriteLongTo('_var_fadeRequest_8c226564', 1 /* FADE_REQUEST_IN */);
        $this->shouldWriteLongTo('_var_sceneVisible_8c226560', 1);
        $this->shouldWriteLongTo('_var_messageBoxActive_8c22847c', 0);
    }

    public function test_unknown_phase_falls_through_to_draw(): void
    {
        // Any phase outside 0-5 goes straight to the shared draw tail
        // untouched (dead state in practice, but matches the original).
        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 6, pageIndex: 2);

        $this->call('_messageBoxTask_8c02ab7a')->with($task, $state);

        $this->expectDrawEmpty($state, 2);
    }

    public function test_draw_matches_asset_and_draws_scroll(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 4, pageCount: 5, pageIndex: 1, frameCounter: 0);
        $this->allocPeripheral(on: 0, press: 0);

        // ids array: one id, then terminator.
        $ids = $this->alloc(4);
        $this->initUint16($ids, 0x0007);
        $this->initUint16($ids + 2, 0xffff);
        $this->initUint32($state + self::ST_IDS, $ids);

        // Dedup table: a single matching entry.
        $entries = $this->addressOf('_var_messageAssets_8c228484');
        $this->initUint32($entries, 7);
        $pvm = $this->alloc(4);
        $dat = $this->alloc(4);
        $this->initUint32($entries + 0x04, $pvm);
        $this->initUint32($entries + 0x08, $dat);
        $this->initUint32($this->addressOf('_var_messageAssetCount_8c228514'), 1);

        $this->call('_messageBoxTask_8c02ab7a')->with($task, $state);

        $this->shouldWriteFloat($this->addressOf('_init_msgScroll_8c04ab3c') + 0x24, -3.0);
        $this->shouldWriteLong($this->addressOf('_init_msgScroll_8c04ab3c') + 0x0c, $pvm);
        $this->shouldWriteLong($this->addressOf('_init_msgScroll_8c04ab3c') + 0x10, $dat);
        $this->shouldCall('_njDrawScroll')->with($this->addressOf('_init_msgScroll_8c04ab3c'));
        $this->shouldWriteFloat($this->addressOf('_init_msgScroll_8c04ab3c') + 0x24, -2.9);
        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(1);
        $this->shouldCall('_SndPollVoiceEnd_8c0106ac');
    }

    public function test_draw_with_no_matching_asset_leaves_pvm_dat_stale(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 4, pageCount: 5, pageIndex: 1, frameCounter: 0);
        $this->allocPeripheral(on: 0, press: 0);

        // ids array: one id that matches nothing in the dedup table.
        $ids = $this->alloc(4);
        $this->initUint16($ids, 0x0007);
        $this->initUint16($ids + 2, 0xffff);
        $this->initUint32($state + self::ST_IDS, $ids);

        // Dedup table: no entries at all.
        $this->initUint32($this->addressOf('_var_messageAssetCount_8c228514'), 0);

        // Sentinel pvm/dat: proves the "no match" path leaves them untouched.
        $pvmField = $this->addressOf('_init_msgScroll_8c04ab3c') + 0x0c;
        $datField = $this->addressOf('_init_msgScroll_8c04ab3c') + 0x10;
        $this->initUint32($pvmField, 0xdeadbeef);
        $this->initUint32($datField, 0xcafef00d);

        $this->call('_messageBoxTask_8c02ab7a')->with($task, $state);

        $this->shouldWriteFloat($this->addressOf('_init_msgScroll_8c04ab3c') + 0x24, -3.0);
        $this->shouldCall('_njDrawScroll')
            ->with($this->addressOf('_init_msgScroll_8c04ab3c'))
            ->do(function () use ($pvmField, $datField) {
                $pvm = $this->memory->readUInt32($pvmField)->value;
                $dat = $this->memory->readUInt32($datField)->value;
                if ($pvm !== 0xdeadbeef || $dat !== 0xcafef00d) {
                    throw new RuntimeException(sprintf(
                        'pvm/dat were overwritten (%08x/%08x) despite no dedup match',
                        $pvm, $dat,
                    ));
                }
            });
        $this->shouldWriteFloat($this->addressOf('_init_msgScroll_8c04ab3c') + 0x24, -2.9);
        $this->shouldCall('_MessageBoxMenuTextboxText_8c02af1c')->with(1);
        $this->shouldCall('_SndPollVoiceEnd_8c0106ac');
    }
};
