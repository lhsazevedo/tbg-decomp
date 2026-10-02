<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    // Task offsets.
    const TASK_FIELD_08 = 0x08;

    // State offsets (word index * 4), matching ObjectsInstall_8c02a6ac's test.
    const ST_10 = 0x40;
    const ST_11 = 0x44;
    const ST_12 = 0x48;
    const ST_16 = 0x58;
    const ST_17 = 0x5c;

    // var_routeModelSlots_8c1bbddc entry layout (0x10 bytes per slot).
    const SLOT_SIZE = 0x10;
    const SLOT_TEXLIST = 0x08;
    const SLOT_NJ = 0x0c;
    const SLOTS = 0x20;

    /**
     * rowFlyByTask_8c029e94's stack slots for TaskInstall's installed_task /
     * installed_state out-params. Unlike most functions in this unit, the two
     * objects happen to lay these out identically.
     */
    private function outParams(): array
    {
        return [0xffffe8, 0xffffdc];
    }

    public function test_countdown_not_lapsed_just_decrements(): void
    {
        $task = $this->alloc(0x20);
        $this->initUint32($task + self::TASK_FIELD_08, 1);
        $state = $this->alloc(0x7c);

        $this->call('_rowFlyByTask_8c029e94')->with($task, $state);

        $this->shouldWriteLong($task + self::TASK_FIELD_08, 0);
    }

    public function test_countdown_lapsed_installs_flyby_and_rearms(): void
    {
        $this->setSize('_var_routeModelSlots_8c1bbddc', self::SLOTS * self::SLOT_SIZE);
        $slots = $this->addressOf('_var_routeModelSlots_8c1bbddc');

        $texlist = $this->alloc(4);
        $nj = $this->alloc(4);
        // Index picked = AsqGetRandomInRangeA_8c012178(0xd) * 2 + 1.
        $this->initUint32($slots + 1 * self::SLOT_SIZE + self::SLOT_TEXLIST, $texlist);
        $this->initUint32($slots + 1 * self::SLOT_SIZE + self::SLOT_NJ, $nj);

        $task = $this->alloc(0x20);
        $this->initUint32($task + self::TASK_FIELD_08, 0);
        $state = $this->alloc(0x7c);
        $dat = $this->alloc(4);
        $this->initUint32($state + self::ST_12, $dat);
        // rowFlyByTask_8c029e94 copies this field verbatim (it's a raw bit pattern to
        // this function, whatever float value the installer put there).
        $frameLimit = 0x40800000; // 4.0f
        $this->initUint32($state + self::ST_17, $frameLimit);

        $newTask = $this->alloc(0x20);
        $newState = $this->alloc(0x7c);

        $this->call('_rowFlyByTask_8c029e94')->with($task, $state);

        [$taskLocal, $stateLocal] = $this->outParams();

        $this->shouldWriteLong($task + self::TASK_FIELD_08, 0xffffffff);
        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1bb448'),
                $this->addressOf('_flyByModelTask_8c029e68'),
                $taskLocal,
                $stateLocal,
                0x7c,
            )
            ->do(function () use ($newTask, $newState) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($newTask));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($newState));
            })
            ->andReturn(1);
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(0xd)->andReturn(0);
        $this->shouldWriteLong($newState + self::ST_10, $texlist);
        $this->shouldWriteLong($newState + self::ST_11, $nj);
        $this->shouldWriteLong($newState + self::ST_12, $dat);
        $this->shouldWriteLong($newState + self::ST_16, 0);
        $this->shouldWriteLong($newState + self::ST_17, $frameLimit);
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(0x30)->andReturn(5);
        $this->shouldWriteLong($task + self::TASK_FIELD_08, 5 + 0x10);
    }

    public function test_lapsed_retries_random_slot_until_a_loaded_one_is_found(): void
    {
        $this->setSize('_var_routeModelSlots_8c1bbddc', self::SLOTS * self::SLOT_SIZE);
        $slots = $this->addressOf('_var_routeModelSlots_8c1bbddc');

        // Index 1 (from random 0) is unloaded (-1); index 3 (from random 1) is loaded.
        $this->initUint32($slots + 1 * self::SLOT_SIZE + self::SLOT_TEXLIST, 0xffffffff);
        $this->initUint32($slots + 1 * self::SLOT_SIZE + self::SLOT_NJ, 0);
        $texlist = $this->alloc(4);
        $nj = $this->alloc(4);
        $this->initUint32($slots + 3 * self::SLOT_SIZE + self::SLOT_TEXLIST, $texlist);
        $this->initUint32($slots + 3 * self::SLOT_SIZE + self::SLOT_NJ, $nj);

        $task = $this->alloc(0x20);
        $this->initUint32($task + self::TASK_FIELD_08, 0);
        $state = $this->alloc(0x7c);
        $dat = $this->alloc(4);
        $this->initUint32($state + self::ST_12, $dat);
        $frameLimit = 0x40000000; // 2.0f
        $this->initUint32($state + self::ST_17, $frameLimit);

        $newTask = $this->alloc(0x20);
        $newState = $this->alloc(0x7c);

        $this->call('_rowFlyByTask_8c029e94')->with($task, $state);

        [$taskLocal, $stateLocal] = $this->outParams();

        $this->shouldWriteLong($task + self::TASK_FIELD_08, 0xffffffff);
        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1bb448'),
                $this->addressOf('_flyByModelTask_8c029e68'),
                $taskLocal,
                $stateLocal,
                0x7c,
            )
            ->do(function () use ($newTask, $newState) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($newTask));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($newState));
            })
            ->andReturn(1);
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(0xd)->andReturn(0);
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(0xd)->andReturn(1);
        $this->shouldWriteLong($newState + self::ST_10, $texlist);
        $this->shouldWriteLong($newState + self::ST_11, $nj);
        $this->shouldWriteLong($newState + self::ST_12, $dat);
        $this->shouldWriteLong($newState + self::ST_16, 0);
        $this->shouldWriteLong($newState + self::ST_17, $frameLimit);
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(0x30)->andReturn(0);
        $this->shouldWriteLong($task + self::TASK_FIELD_08, 0x10);
    }

    public function test_lapsed_taskinstall_failure_still_rearms(): void
    {
        $task = $this->alloc(0x20);
        $this->initUint32($task + self::TASK_FIELD_08, 0);
        $state = $this->alloc(0x7c);

        $this->call('_rowFlyByTask_8c029e94')->with($task, $state);

        [$taskLocal, $stateLocal] = $this->outParams();

        $this->shouldWriteLong($task + self::TASK_FIELD_08, 0xffffffff);
        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1bb448'),
                $this->addressOf('_flyByModelTask_8c029e68'),
                $taskLocal,
                $stateLocal,
                0x7c,
            )
            ->andReturn(0);
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(0x30)->andReturn(3);
        $this->shouldWriteLong($task + self::TASK_FIELD_08, 3 + 0x10);
    }
};
