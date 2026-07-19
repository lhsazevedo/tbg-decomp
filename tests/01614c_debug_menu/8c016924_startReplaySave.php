<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    const REPLAY_BUFFER_CAPACITY = 54000;
    const REPLAY_BUFFER_SIZE = self::REPLAY_BUFFER_CAPACITY * 8;

    public function test_no_vmu_selected_deactivates_task(): void
    {
        $this->resolveSymbols();
        $task = $this->alloc(0x20);

        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0xffffffff);
        $this->initUint32(
            $this->addressOf('_var_demoCursor_8c225fa8'),
            $this->addressOf('_var_demoBuffer_8c1bc828')
        );

        $this->call('_startReplaySave_8c016924');

        $this->shouldCall('_TaskPush_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_replaySaveTask_8c0167ca'),
                $this->createdTaskOffset(),
                $this->createStateOffset(),
                0,
            )
            ->do(function ($params) use ($task) {
                $this->memory->writeUInt32($params[2], U32::of($task));
            });
        $this->shouldWriteLong($task + 0x08, 0);
    }

    public function test_buffer_full_deactivates_task(): void
    {
        $this->resolveSymbols();
        $task = $this->alloc(0x20);

        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $end = $this->addressOf('_var_demoBuffer_8c1bc828') + self::REPLAY_BUFFER_SIZE;
        $this->initUint32($this->addressOf('_var_demoCursor_8c225fa8'), $end);

        $this->call('_startReplaySave_8c016924');

        $this->shouldCall('_TaskPush_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_replaySaveTask_8c0167ca'),
                $this->createdTaskOffset(),
                $this->createStateOffset(),
                0,
            )
            ->do(function ($params) use ($task) {
                $this->memory->writeUInt32($params[2], U32::of($task));
            });
        $this->shouldWriteLong($task + 0x08, 0);
    }

    public function test_packs_recorded_buffer_and_starts_task(): void
    {
        $this->resolveSymbols();
        $task = $this->alloc(0x20);

        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);

        $bufBase = $this->addressOf('_var_demoBuffer_8c1bc828');
        $cursor = $bufBase + 0x28; // 5 records recorded (5 * 8 bytes)
        $this->initUint32($this->addressOf('_var_demoCursor_8c225fa8'), $cursor);

        $this->initUint32($this->addressOf('_var_8c228ba4'), 0x99);
        $this->initUint32($this->addressOf('_var_currentCourse_8c1bb868') + 0x00, 7); // courseId
        $this->initUint32($this->addressOf('_var_inputMapSel_8c1bb8c8'), 3);
        $this->initUint32($this->addressOf('_var_seed_8c157a64'), 0x1234);

        $malloc = $this->alloc(0x38); // 0x28 recorded + 0x10 header

        $this->call('_startReplaySave_8c016924');

        $this->shouldCall('_TaskPush_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_replaySaveTask_8c0167ca'),
                $this->createdTaskOffset(),
                $this->createStateOffset(),
                0,
            )
            ->do(function ($params) use ($task) {
                $this->memory->writeUInt32($params[2], U32::of($task));
            });

        $this->shouldCall('_syMalloc')->with(0x38)->andReturn($malloc);
        $this->shouldWriteLongTo('_var_demoBuf_8c1ba3c4', $malloc);
        $this->shouldCall('_FUN_8c02f320');
        $this->shouldCall('_FUN_8c02f934')->with($bufBase, $this->destOffset(), 0x38);
        $this->shouldWriteLong($malloc + 0x00, 0x99);
        $this->shouldWriteLong($malloc + 0x04, 7);
        $this->shouldWriteLong($malloc + 0x08, 3);
        $this->shouldWriteLong($malloc + 0x0c, 0x1234);
        $this->shouldWriteLong($task + 0x08, 1);
        $this->shouldWriteLong($task + 0x0c, 0);
    }

    private function createdTaskOffset(): int
    {
        return str_contains($this->objectFile, '_src.obj') ? 0xffffe0 : 0xffffe4;
    }

    private function createStateOffset(): int
    {
        return str_contains($this->objectFile, '_src.obj') ? 0xffffe8 : 0xffffec;
    }

    private function destOffset(): int
    {
        return str_contains($this->objectFile, '_src.obj') ? 0xffffe4 : 0xffffe8;
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_tasks_8c1ba3c8', 4);
        $this->setSize('_var_selectedVm_8c1ba34c', 4);
        $this->setSize('_var_demoBuf_8c1ba3c4', 4);
        $this->setSize('_var_8c228ba4', 4);
        $this->setSize('_var_currentCourse_8c1bb868', 0x50);
        $this->setSize('_var_inputMapSel_8c1bb8c8', 4);
        $this->setSize('_var_seed_8c157a64', 4);
        $this->setSize('_syMalloc', 4);
        $this->setSize('_FUN_8c02f320', 4);
        $this->setSize('_FUN_8c02f934', 4);
        /*
         * startReplaySave_8c016924's real end-of-buffer bound check reuses &var_demoCursor_8c225fa8
         * as a linker-layout coincidence (base + REPLAY_BUFFER_CAPACITY*8 lands exactly on
         * the next symbol) -- same pattern as DebugMenuDemoRecordTask_8c01677e (see
         * reference_linker_coincidence_bound memory). Allocate adjacently to reproduce it.
         */
        $this->setSize('_var_demoBuffer_8c1bc828', self::REPLAY_BUFFER_SIZE);
        $this->setSize('_var_demoCursor_8c225fa8', 4);
    }
};
