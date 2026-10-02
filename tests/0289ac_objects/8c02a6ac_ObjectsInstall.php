<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    // var_assetRequestSlots_8c228288 row layout (0x18 bytes per row).
    const ROW_SIZE = 0x18;
    const ROW_PVM = 0x00;
    const ROW_NJ = 0x04;
    const ROW_DAT = 0x08;
    const ROW_B14 = 0x14;
    const ROW_B15 = 0x15;
    const ROW_B16 = 0x16;
    const ROW_B17 = 0x17;

    // Task offsets.
    const TASK_FIELD_08 = 0x08;
    const TASK_FIELD_0C = 0x0c;

    // Installed task state offsets (word index * 4).
    const ST_10 = 0x40;
    const ST_11 = 0x44;
    const ST_12 = 0x48;
    const ST_15 = 0x54;
    const ST_16 = 0x58;
    const ST_17 = 0x5c;
    const ST_18 = 0x60;
    const ST_19 = 0x64;
    const ST_78 = 0x78;
    const ST_79 = 0x79;

    protected function isAsmObject(): bool
    {
        return str_contains($this->objectFile, '/asm/');
    }

    /**
     * ObjectsInstall_8c02a6ac's stack slots for TaskInstall's installed_task /
     * installed_state out-params (a single pair of locals reused across every
     * row and the closing TaskInstall call). The two objects lay the frame out
     * differently.
     */
    private function outParams(): array
    {
        return $this->isAsmObject() ? [0xffffd0, 0xffffcc] : [0xffffc8, 0xffffc4];
    }

    /** Allocates a fake asset handle with a frame count at offset +4. */
    private function makeHandle(int $frames): int
    {
        $handle = $this->alloc(8);
        $this->initUint32($handle + 4, $frames);
        return $handle;
    }

    private function makeTable(array $types): int
    {
        // The compiled object's relocations for every symbol the function
        // touches (across all branches) get resolved up front regardless of
        // which branch a given test actually takes, so these need an address
        // in every test, not just the ones that exercise their branch.
        $this->addressOf('_var_tasks_8c1bb448');
        $this->addressOf('_var_tasks_8c1ba5e8');

        $n = count($types);
        $table = $this->alloc(($n + 1) * 8);
        foreach ($types as $i => $type) {
            $this->initUint32($table + $i * 8, $type);
            $this->initUint32($table + $i * 8 + 4, 0);
        }
        $this->initUint32($table + $n * 8, -1);
        return $table;
    }

    public function test_sentinel_table_is_a_noop(): void
    {
        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), -1);

        $this->call('_ObjectsInstall_8c02a6ac');
    }

    public function test_empty_table_still_pushes_the_closing_group_task(): void
    {
        // A distinct case from test_sentinel_table_is_a_noop: here the table
        // pointer itself is valid, just with zero rows -- the loop body never
        // runs, but the closing TaskInstall must still fire.
        $table = $this->makeTable([]);
        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), $table);

        $this->call('_ObjectsInstall_8c02a6ac');

        $this->mockCloseTaskInstall();
    }

    public function test_unmatched_row_type_advances_without_a_task_install(): void
    {
        // Verified against the asm: the compare chain has no default/else --
        // an unmatched type (99) falls straight to rowIndex++ with no
        // TaskInstall at all, and the following real row (type 4) must still
        // read its own slot, not the skipped row's.
        $table = $this->makeTable([99, 4]);
        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), $table);

        $row1 = $this->addressOf('_var_assetRequestSlots_8c228288') + self::ROW_SIZE;
        $pvm1 = $this->makeHandle(0);
        $nj1 = $this->makeHandle(0);
        $this->initUint32($row1 + self::ROW_PVM, $pvm1);
        $this->initUint32($row1 + self::ROW_NJ, $nj1);
        $this->initUint8($row1 + self::ROW_B15, 0);

        $task1 = $this->alloc(0x20);
        $state1 = $this->alloc(0x7c);

        $this->call('_ObjectsInstall_8c02a6ac');

        [$taskLocal, $stateLocal] = $this->outParams();

        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1bb448'),
                $this->addressOf('_rowSimpleModelTask_8c02a1f0'),
                $taskLocal,
                $stateLocal,
                0x7c,
            )
            ->do(function () use ($task1, $state1) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($task1));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($state1));
            })
            ->andReturn(1);
        $this->shouldWriteLong($state1 + self::ST_10, $pvm1);
        $this->shouldWriteLong($state1 + self::ST_11, $nj1);
        $this->shouldWriteLong($task1 + self::TASK_FIELD_08, 0);
        $this->mockCloseTaskInstall();
    }

    public function test_type_0_installs_from_dat_handle(): void
    {
        $table = $this->makeTable([0]);
        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), $table);

        $row = $this->addressOf('_var_assetRequestSlots_8c228288');
        $dat = $this->makeHandle(5);
        $this->initUint32($row + self::ROW_DAT, $dat);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $closeTask = $this->alloc(0x20);
        $closeState = $this->alloc(4);

        $this->call('_ObjectsInstall_8c02a6ac');

        [$taskLocal, $stateLocal] = $this->outParams();

        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1bb448'),
                $this->addressOf('_rowFlyByTask_8c029e94'),
                $taskLocal,
                $stateLocal,
                0x7c,
            )
            ->do(function () use ($task, $state) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($task));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($state));
            })
            ->andReturn(1);
        $this->shouldWriteLong($state + self::ST_12, $dat);
        $this->shouldWriteFloat($state + self::ST_17, 4.0);
        $this->shouldWriteLong($task + self::TASK_FIELD_08, 0);

        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba5e8'),
                $this->addressOf('_execRowTaskGroupTask_8c02a60e'),
                $taskLocal,
                $stateLocal,
                0,
            )
            ->do(function () use ($closeTask, $closeState) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($closeTask));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($closeState));
            })
            ->andReturn(1);
    }

    public function test_type_1_seeds_dat_handle_via_initDatBlob_8c029f42(): void
    {
        $table = $this->makeTable([1]);
        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), $table);

        $row = $this->addressOf('_var_assetRequestSlots_8c228288');
        $pvm = $this->makeHandle(0);
        $nj = $this->makeHandle(0);
        $dat = $this->makeHandle(0);
        $this->initUint32($row + self::ROW_PVM, $pvm);
        $this->initUint32($row + self::ROW_NJ, $nj);
        $this->initUint32($row + self::ROW_DAT, $dat);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);

        $this->call('_ObjectsInstall_8c02a6ac');

        [$taskLocal, $stateLocal] = $this->outParams();

        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1bb448'),
                $this->addressOf('_rowDatTask_8c029fcc'),
                $taskLocal,
                $stateLocal,
                0x7c,
            )
            ->do(function () use ($task, $state) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($task));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($state));
            })
            ->andReturn(1);
        $this->shouldCall('_initDatBlob_8c029f42')->with($dat, $pvm, $nj);
        $this->shouldWriteLong($state + self::ST_12, $dat);
        $this->mockCloseTaskInstall();
    }

    public function test_type_2_seeds_pvm_nj_and_attr_bytes(): void
    {
        $table = $this->makeTable([2]);
        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), $table);

        $row = $this->addressOf('_var_assetRequestSlots_8c228288');
        $pvm = $this->makeHandle(0);
        $nj = $this->makeHandle(0);
        $this->initUint32($row + self::ROW_PVM, $pvm);
        $this->initUint32($row + self::ROW_NJ, $nj);
        $this->initUint8($row + self::ROW_B14, 0x11);
        $this->initUint8($row + self::ROW_B15, 0x22);
        $this->initUint8($row + self::ROW_B16, 0x33);
        $this->initUint8($row + self::ROW_B17, 0x44);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);

        $this->call('_ObjectsInstall_8c02a6ac');

        [$taskLocal, $stateLocal] = $this->outParams();

        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1bb448'),
                $this->addressOf('_rowModelTask_8c02a08a'),
                $taskLocal,
                $stateLocal,
                0x7c,
            )
            ->do(function () use ($task, $state) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($task));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($state));
            })
            ->andReturn(1);
        $this->shouldWriteLong($state + self::ST_15, 0);
        $this->shouldWriteLong($state + self::ST_10, $pvm);
        $this->shouldWriteLong($state + self::ST_11, $nj);
        $this->shouldWriteLong($task + self::TASK_FIELD_08, 0x11 << 0x18);
        $this->shouldWriteLong($task + self::TASK_FIELD_0C, 0x22);
        $this->shouldWriteByte($state + self::ST_78, 0x33);
        $this->shouldWriteByte($state + self::ST_79, 0x44);
        $this->mockCloseTaskInstall();
    }

    public function test_type_3_seeds_pvm_nj_dat_and_attr_bytes(): void
    {
        $table = $this->makeTable([3]);
        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), $table);

        $row = $this->addressOf('_var_assetRequestSlots_8c228288');
        $pvm = $this->makeHandle(0);
        $nj = $this->makeHandle(0);
        $dat = $this->makeHandle(9);
        $this->initUint32($row + self::ROW_PVM, $pvm);
        $this->initUint32($row + self::ROW_NJ, $nj);
        $this->initUint32($row + self::ROW_DAT, $dat);
        $this->initUint8($row + self::ROW_B14, 0x11);
        $this->initUint8($row + self::ROW_B15, 0x22);
        $this->initUint8($row + self::ROW_B16, 0x33);
        $this->initUint8($row + self::ROW_B17, 0x44);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);

        $this->call('_ObjectsInstall_8c02a6ac');

        [$taskLocal, $stateLocal] = $this->outParams();

        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1bb448'),
                $this->addressOf('_rowMotionModelTask_8c02a120'),
                $taskLocal,
                $stateLocal,
                0x7c,
            )
            ->do(function () use ($task, $state) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($task));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($state));
            })
            ->andReturn(1);
        $this->shouldWriteLong($state + self::ST_15, 0);
        $this->shouldWriteLong($state + self::ST_10, $pvm);
        $this->shouldWriteLong($state + self::ST_11, $nj);
        $this->shouldWriteLong($state + self::ST_12, $dat);
        $this->shouldWriteLong($state + self::ST_16, 0);
        $this->shouldWriteFloat($state + self::ST_17, 8.0);
        $this->shouldWriteLong($task + self::TASK_FIELD_08, 0x11 << 0x18);
        $this->shouldWriteLong($task + self::TASK_FIELD_0C, 0x22);
        $this->shouldWriteByte($state + self::ST_78, 0x33);
        $this->shouldWriteByte($state + self::ST_79, 0x44);
        $this->mockCloseTaskInstall();
    }

    public function test_type_4_seeds_pvm_and_nj(): void
    {
        $table = $this->makeTable([4]);
        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), $table);

        $row = $this->addressOf('_var_assetRequestSlots_8c228288');
        $pvm = $this->makeHandle(0);
        $nj = $this->makeHandle(0);
        $this->initUint32($row + self::ROW_PVM, $pvm);
        $this->initUint32($row + self::ROW_NJ, $nj);
        $this->initUint8($row + self::ROW_B15, 0x22);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);

        $this->call('_ObjectsInstall_8c02a6ac');

        [$taskLocal, $stateLocal] = $this->outParams();

        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1bb448'),
                $this->addressOf('_rowSimpleModelTask_8c02a1f0'),
                $taskLocal,
                $stateLocal,
                0x7c,
            )
            ->do(function () use ($task, $state) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($task));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($state));
            })
            ->andReturn(1);
        $this->shouldWriteLong($state + self::ST_10, $pvm);
        $this->shouldWriteLong($state + self::ST_11, $nj);
        $this->shouldWriteLong($task + self::TASK_FIELD_08, 0x22);
        $this->mockCloseTaskInstall();
    }

    public function test_type_5_seeds_pvm_nj_and_copies_extra(): void
    {
        $this->setSize('__quick_evn_mvn', 4);

        $table = $this->makeTable([5]);
        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), $table);

        $row = $this->addressOf('_var_assetRequestSlots_8c228288');
        $pvm = $this->makeHandle(0);
        $nj = $this->makeHandle(0);
        $this->initUint32($row + self::ROW_PVM, $pvm);
        $this->initUint32($row + self::ROW_NJ, $nj);
        $this->initUint32($row + 0x0c, 0x11223344);
        $this->initUint32($row + 0x10, 0x55667788);
        $this->initUint32($row + 0x14, 0x99aabbcc);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);

        $this->call('_ObjectsInstall_8c02a6ac');

        [$taskLocal, $stateLocal] = $this->outParams();

        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1bb448'),
                $this->addressOf('_rowMaterialModelTask_8c02a27c'),
                $taskLocal,
                $stateLocal,
                0x7c,
            )
            ->do(function () use ($task, $state) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($task));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($state));
            })
            ->andReturn(1);
        $this->shouldWriteLong($state + self::ST_10, $pvm);
        $this->shouldWriteLong($state + self::ST_11, $nj);
        // The trailing 8-byte struct copy compiles to a call to the SHC
        // runtime helper __quick_evn_mvn; its memory effect isn't visible to
        // this harness (see ObjectsStartAssetRequests_8c029ad4's type-5 case
        // for the same reason), so just assert the call.
        $this->shouldCall('__quick_evn_mvn');
        $this->shouldWriteLong($state + 0x74, 0); // ST_1D
        $this->shouldWriteLong($state + 0x70, 0); // ST_1C
        $this->shouldWriteLong($state + 0x6c, 0); // ST_1B
        $this->mockCloseTaskInstall();
    }

    /**
     * Type 6's TaskInstall call passes `state` itself (its stale value left
     * over from the previous row), not `&state`, as the installed_state
     * out-param -- a bug-for-bug match of the archived asm (Ghidra shows
     * `local_30` instead of `&local_30` here, unlike every other case). Since
     * nothing writes a fresh pointer back into the C-level `state` local,
     * every field write that follows still lands in the *previous* row's
     * state buffer, not the newly-installed task's own state -- and the newly
     * installed task's state pointer is used for nothing but overwriting the
     * previous buffer's first word. This only exercises meaningfully as the
     * second row, after a first row has left `state` pointing at a real
     * buffer.
     */
    public function test_type_6_picks_a_random_trailer_and_resolves_lamp_grandchildren(): void
    {
        $this->setSize('_AsqGetRandomInRangeA_8c012178', 4);

        $table = $this->makeTable([4, 6]);
        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), $table);

        $row0 = $this->addressOf('_var_assetRequestSlots_8c228288');
        $pvm0 = $this->makeHandle(0);
        $nj0 = $this->makeHandle(0);
        $this->initUint32($row0 + self::ROW_PVM, $pvm0);
        $this->initUint32($row0 + self::ROW_NJ, $nj0);
        $this->initUint8($row0 + self::ROW_B15, 0);

        $fumi00 = $this->makeHandle(9);
        $this->initUint32($this->addressOf('_var_fumiCloseMotion_8c228414'), $fumi00);

        $tra0 = $this->makeHandle(3);
        $tra1 = $this->makeHandle(7);
        $this->initUint32($this->addressOf('_var_fumiTrainMotionA_8c22842c'), $tra0);
        $this->initUint32($this->addressOf('_var_fumiTrainMotionB_8c228430'), $tra1);

        $lamp = 0x40404040;
        $this->initUint32($this->addressOf('_var_fumiLampModel_8c22841c'), $lamp);

        $task0 = $this->alloc(0x20);
        $state0 = $this->alloc(0x7c);
        $task1 = $this->alloc(0x20);

        $this->call('_ObjectsInstall_8c02a6ac');

        [$taskLocal, $stateLocal] = $this->outParams();

        // Row 0 (type 4): establishes `state` = state0 for real.
        $this->shouldCall('_TaskInstall_8c014ae8')
            ->do(function () use ($task0, $state0) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($task0));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($state0));
            })
            ->andReturn(1);
        $this->shouldWriteLong($state0 + self::ST_10, $pvm0);
        $this->shouldWriteLong($state0 + self::ST_11, $nj0);
        $this->shouldWriteLong($task0 + self::TASK_FIELD_08, 0);

        // Row 1 (type 6): installed_state == state0 (the stale value), so the
        // mocked TaskInstall "writes back" the new state pointer into *state0
        // -- but the C local `state` itself is left unchanged at state0.
        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1bb448'),
                $this->addressOf('_fumiCrossingTask_8c02a4f8'),
                $taskLocal,
                $state0,
                0x7c,
            )
            ->do(function () use ($task1) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($task1));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($task1));
            })
            ->andReturn(1);
        $this->shouldWriteLong($state0 + self::ST_15, 0);
        $this->shouldWriteLong($state0 + self::ST_16, 0);
        $this->shouldWriteFloat($state0 + self::ST_17, 8.0);
        $this->shouldWriteLong($state0 + self::ST_18, 0);
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(2)->andReturn(1);
        $this->shouldWriteLong($state0 + self::ST_12, $tra0);
        $this->shouldWriteFloat($state0 + self::ST_19, 2.0);
        $this->shouldWriteLongTo('_var_fumiLampNodes_8c228434', $lamp);
        $this->shouldCall('_resolveObjectGrandchildren_8c02a322')
            ->with($this->addressOf('_var_fumiLampNodes_8c228434'));
        $this->mockCloseTaskInstall();
    }

    public function test_multiple_rows_advance_the_row_index(): void
    {
        $table = $this->makeTable([4, 4]);
        $this->initUint32($this->addressOf('_var_assetRequestTable_8c228408'), $table);

        $row0 = $this->addressOf('_var_assetRequestSlots_8c228288');
        $row1 = $row0 + self::ROW_SIZE;
        $pvm0 = $this->makeHandle(0);
        $nj0 = $this->makeHandle(0);
        $pvm1 = $this->makeHandle(0);
        $nj1 = $this->makeHandle(0);
        $this->initUint32($row0 + self::ROW_PVM, $pvm0);
        $this->initUint32($row0 + self::ROW_NJ, $nj0);
        $this->initUint8($row0 + self::ROW_B15, 0);
        $this->initUint32($row1 + self::ROW_PVM, $pvm1);
        $this->initUint32($row1 + self::ROW_NJ, $nj1);
        $this->initUint8($row1 + self::ROW_B15, 0);

        $task0 = $this->alloc(0x20);
        $state0 = $this->alloc(0x7c);
        $task1 = $this->alloc(0x20);
        $state1 = $this->alloc(0x7c);

        $this->call('_ObjectsInstall_8c02a6ac');

        $this->shouldCall('_TaskInstall_8c014ae8')
            ->do(function () use ($task0, $state0) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($task0));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($state0));
            })
            ->andReturn(1);
        $this->shouldWriteLong($state0 + self::ST_10, $pvm0);
        $this->shouldWriteLong($state0 + self::ST_11, $nj0);
        $this->shouldWriteLong($task0 + self::TASK_FIELD_08, 0);

        $this->shouldCall('_TaskInstall_8c014ae8')
            ->do(function () use ($task1, $state1) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($task1));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($state1));
            })
            ->andReturn(1);
        $this->shouldWriteLong($state1 + self::ST_10, $pvm1);
        $this->shouldWriteLong($state1 + self::ST_11, $nj1);
        $this->shouldWriteLong($task1 + self::TASK_FIELD_08, 0);

        $this->mockCloseTaskInstall();
    }

    /** Expects the final "table drained" TaskInstall and lets it succeed harmlessly. */
    private function mockCloseTaskInstall(): void
    {
        [$taskLocal, $stateLocal] = $this->outParams();

        $closeTask = $this->alloc(0x20);
        $closeState = $this->alloc(4);
        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba5e8'),
                $this->addressOf('_execRowTaskGroupTask_8c02a60e'),
                $taskLocal,
                $stateLocal,
                0,
            )
            ->do(function () use ($closeTask, $closeState) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($closeTask));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($closeState));
            })
            ->andReturn(1);
    }
};
