<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    private function f(float $value): int
    {
        return unpack('L', pack('f', $value))[1];
    }

    protected function isAsmObject(): bool
    {
        return str_contains($this->objectFile, '/asm/');
    }

    /**
     * SignalInit_8c02845a's stack slots for TaskSpawn's created_task / create_state
     * out-params. The two objects lay the frame out differently.
     */
    private function outParams(): array
    {
        return $this->isAsmObject() ? [0xffffcc, 0xffffc8] : [0xffffd0, 0xffffcc];
    }

    /** Fills in TaskSpawn's two out-params. */
    private function fillTaskSpawn(int $task, int $state): Closure
    {
        return function ($params) use ($task, $state) {
            $this->memory->writeUInt32($params[2], U32::of($task));
            $this->memory->writeUInt32($params[3], U32::of($state));
        };
    }

    /** Checks _quick_odd_mvn's R1/R2/R0 operands: dest, src, 12 bytes. */
    private function assertOddMvn(int $dest, int $src): Closure
    {
        return function () use ($dest, $src) {
            $got = sprintf(
                'dest=%08x src=%08x n=%d',
                $this->getRegister(1)->value,
                $this->getRegister(2)->value,
                $this->getRegister(0)->value,
            );
            $want = sprintf('dest=%08x src=%08x n=12', $dest, $src);
            if ($got !== $want) {
                throw new RuntimeException("_quick_odd_mvn: expected $want, got $got");
            }
        };
    }

    public function test_empty_table_allocates_and_returns_early()
    {
        // Table with no entries: first entry's type terminates immediately.
        $table = $this->alloc(0xe * 4);
        $this->initUint32($table + 0x00, 0);
        $course = $this->setSize('_var_currentCourse_8c1bb868', 0x50);
        $this->initUint32($course + 0x38, $table); // macSignal_0x38

        $flags = $this->alloc(4);
        $this->initUint32($flags, 0);
        $slots = $this->alloc(4);
        $flagsVar = $this->addressOf('_var_trafficSignalFrames_8c227e24');
        $slotsVar = $this->addressOf('_var_trafficSignalStates_8c227e28');

        $this->call('_SignalInit_8c02845a');

        // maxId == 0, so both buffers are (0 + 1) * 4 bytes.
        $this->shouldCall('_syMalloc')->with(4)->andReturn($flags);
        $this->shouldWriteLong($flagsVar, $flags);
        $this->shouldCall('_syMalloc')->with(4)->andReturn($slots);
        $this->shouldWriteLong($slotsVar, $slots);
        $this->shouldCall('_TrafficMarkSignalIdsInUse_8c026dcc')->with(0);
    }

    public function test_no_flagged_ids_returns_before_task_group()
    {
        // One type 1 entry: the marking loop skips it, so every flag stays 0.
        $table = $this->alloc(2 * 0xe * 4);
        $this->initUint32($table + 0x00, 1); // type
        $this->initUint32($table + 0x04, 2); // id
        $this->initUint32($table + 0x08, 0); // linked id
        $this->initUint32($table + 0xe * 4, 0); // terminator
        $course = $this->setSize('_var_currentCourse_8c1bb868', 0x50);
        $this->initUint32($course + 0x38, $table); // macSignal_0x38

        $flags = $this->alloc(3 * 4);
        $this->initUint32($flags + 0x00, 0);
        $this->initUint32($flags + 0x04, 0);
        $this->initUint32($flags + 0x08, 0);
        $slots = $this->alloc(3 * 4);

        $this->call('_SignalInit_8c02845a');

        // maxId == 2, so both buffers are (2 + 1) * 4 bytes.
        $this->shouldCall('_syMalloc')->with(0xc)->andReturn($flags);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalFrames_8c227e24'), $flags);
        $this->shouldCall('_syMalloc')->with(0xc)->andReturn($slots);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalStates_8c227e28'), $slots);
        $this->shouldCall('_TrafficMarkSignalIdsInUse_8c026dcc')->with(2);
    }

    public function test_flagged_id_allocates_group_but_spawns_nothing()
    {
        // Type 2 entry flags its *linked* id (0), not its own id (1), so both
        // spawn loops skip it while the count still reaches 1.
        $table = $this->alloc(2 * 0xe * 4);
        $this->initUint32($table + 0x00, 2); // type
        $this->initUint32($table + 0x04, 1); // id
        $this->initUint32($table + 0x08, 0); // linked id
        $this->initUint32($table + 0xe * 4, 0); // terminator
        $course = $this->setSize('_var_currentCourse_8c1bb868', 0x50);
        $this->initUint32($course + 0x38, $table); // macSignal_0x38

        $flags = $this->alloc(2 * 4);
        $this->initUint32($flags + 0x00, 0);
        $this->initUint32($flags + 0x04, 0);
        $slots = $this->alloc(2 * 4);
        $group = $this->alloc(2 * 0x20);
        $this->addressOf('_var_groundQueryPoint_8c1bc460'); // seeds the carried "linked" pointer
        $this->addressOf('_var_routeModels_8c1bc3ec');
        $this->addressOf('_var_tasks_8c1ba5e8');

        $this->call('_SignalInit_8c02845a');

        $this->shouldCall('_syMalloc')->with(8)->andReturn($flags);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalFrames_8c227e24'), $flags);
        $this->shouldCall('_syMalloc')->with(8)->andReturn($slots);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalStates_8c227e28'), $slots);
        $this->shouldCall('_TrafficMarkSignalIdsInUse_8c026dcc')->with(1);
        $this->shouldWriteLong($flags + 0x00, 1); // marking loop flags linked id 0

        // count == 1, so the group holds (1 + 1) * 0x20 bytes.
        $this->shouldCall('_syMalloc')->with(0x40)->andReturn($group);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalTasks_8c227e20'), $group);
        [$taskLocal, $stateLocal] = $this->outParams();
        $this->shouldCall('_TaskInitGroup_8c014a9c')->with($group, 1);
        $this->shouldCall('_TaskSpawn_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba5e8'),
            $this->addressOf('_execGroupTask_8c0283e8'),
            $taskLocal,
            $stateLocal,
            0,
        );
    }

    public function test_spawns_traffic_signal_without_matrices()
    {
        // entry0 (type 2) exists only to flag entry1's id; entry1 is the type 1
        // traffic signal that actually spawns. Both positions are 0, so neither
        // matrix block runs.
        $table = $this->alloc(3 * 0xe * 4);
        $e0 = $table;
        $e1 = $table + 0xe * 4;
        $this->initUint32($e0 + 0x00, 2); // type
        $this->initUint32($e0 + 0x04, 3); // id
        $this->initUint32($e0 + 0x08, 1); // linked id -> flags entry1's id
        $this->initUint32($e1 + 0x00, 1); // type
        $this->initUint32($e1 + 0x04, 1); // id
        $this->initUint32($e1 + 0x08, 5); // linked id, seeds state[1]
        $this->initUint32($e1 + 0x10, 0); // posA.x == 0 -> skip matrix A
        $this->initUint32($e1 + 0x20, 0); // posB.x == 0 -> skip matrix B
        $this->initUint32($e1 + 0x2c, 10); // durations[0]
        $this->initUint32($e1 + 0x30, 20); // durations[1]
        $this->initUint32($e1 + 0x34, 30); // durations[2]
        $this->initUint32($table + 2 * 0xe * 4, 0); // terminator
        $course = $this->setSize('_var_currentCourse_8c1bb868', 0x50);
        $this->initUint32($course + 0x38, $table); // macSignal_0x38

        $flags = $this->alloc(4 * 4);
        for ($i = 0; $i < 4; $i++) {
            $this->initUint32($flags + $i * 4, 0);
        }
        $slots = $this->alloc(4 * 4);
        $group = $this->alloc(2 * 0x20);
        $this->addressOf('_var_groundQueryPoint_8c1bc460');
        $this->addressOf('_var_tasks_8c1ba5e8');

        // routeModels[1] heads a ->0x2c ->0x30 ->0x30 chain read into state[4..6].
        $models = $this->alloc(8 * 4);
        $objB = $this->alloc(0x30);
        $c1 = $this->alloc(0x34);
        $c2 = $this->alloc(0x34);
        $this->initUint32($models + 0x00, 0xaaaa0000);
        $this->initUint32($models + 0x04, $objB);
        $this->initUint32($objB + 0x2c, $c1);
        $this->initUint32($c1 + 0x30, $c2);
        $this->initUint32($c2 + 0x30, 0xcccc0002);
        $this->initUint32($this->addressOf('_var_routeModels_8c1bc3ec'), $models);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0xd4);

        $this->call('_SignalInit_8c02845a');

        $this->shouldCall('_syMalloc')->with(0x10)->andReturn($flags);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalFrames_8c227e24'), $flags);
        $this->shouldCall('_syMalloc')->with(0x10)->andReturn($slots);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalStates_8c227e28'), $slots);
        $this->shouldCall('_TrafficMarkSignalIdsInUse_8c026dcc')->with(3);
        $this->shouldWriteLong($flags + 1 * 4, 1);

        $this->shouldCall('_syMalloc')->with(0x40)->andReturn($group);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalTasks_8c227e20'), $group);
        $this->shouldCall('_TaskInitGroup_8c014a9c')->with($group, 1);

        // The out-params are stack locals whose addresses differ per object, so
        // read them out of R6/R7 ($this is the simulator inside do()).
        [$taskLocal, $stateLocal] = $this->outParams();
        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with(
                $group,
                $this->addressOf('_headTask_8c028258'),
                $taskLocal,
                $stateLocal,
                0xd4,
            )
            ->do($this->fillTaskSpawn($task, $state));

        $this->shouldWriteLong($state + 0x00 * 4, 1);          // id
        $this->shouldWriteLong($state + 0x01 * 4, 5);          // linked id
        $this->shouldWriteLong($state + 0x02 * 4, $e1 + 0xb * 4); // &durations
        // posA -> state[7..9], posB -> state[10..12]. This SHC runtime routine
        // takes dest/src/count in R1/R2/R0, which no DSL convention covers, so
        // check the registers by hand.
        $this->shouldCall('__quick_odd_mvn')
            ->do($this->assertOddMvn($state + 0x1c, $e1 + 0x10));
        $this->shouldCall('__quick_odd_mvn')
            ->do($this->assertOddMvn($state + 0x28, $e1 + 0x20));
        $this->shouldWriteLong($state + 0x2d * 4, 0xaaaa0000);
        $this->shouldWriteLong($state + 0x2e * 4, $objB);
        $this->shouldWriteLong($state + 0x04 * 4, $c1);
        $this->shouldWriteLong($state + 0x05 * 4, $c2);
        $this->shouldWriteLong($state + 0x06 * 4, 0xcccc0002);
        $this->shouldWriteLong($state + 0x03 * 4, 0);          // durations[0] > linked id
        $this->shouldWriteLong($state + 0x31 * 4, 0);
        $this->shouldWriteLong($state + 0x30 * 4, 0);
        $this->shouldWriteLong($state + 0x2f * 4, 0);
        $this->shouldWriteLong($flags + 1 * 4, 0);
        $this->shouldWriteLong($slots + 1 * 4, $state);

        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba5e8'),
                $this->addressOf('_execGroupTask_8c0283e8'),
                $taskLocal,
                $stateLocal,
                0,
            )
            ->do($this->fillTaskSpawn($task, $state));
    }

    public function test_spawns_traffic_signal_with_matrix_a()
    {
        $table = $this->alloc(3 * 0xe * 4);
        $e0 = $table;
        $e1 = $table + 0xe * 4;
        $this->initUint32($e0 + 0x00, 2);
        $this->initUint32($e0 + 0x04, 3);
        $this->initUint32($e0 + 0x08, 1);
        $this->initUint32($e1 + 0x00, 1);
        $this->initUint32($e1 + 0x04, 1);
        $this->initUint32($e1 + 0x08, 5);
        $this->initUint32($e1 + 0x0c, 16384); // rotA
        $this->initUint32($e1 + 0x10, $this->f(100.0)); // posA.x != 0 -> matrix A
        $this->initUint32($e1 + 0x20, 0); // posB.x == 0 -> skip matrix B
        $this->initUint32($e1 + 0x2c, 10);
        $this->initUint32($e1 + 0x30, 20);
        $this->initUint32($e1 + 0x34, 30);
        $this->initUint32($table + 2 * 0xe * 4, 0);
        $course = $this->setSize('_var_currentCourse_8c1bb868', 0x50);
        $this->initUint32($course + 0x38, $table); // macSignal_0x38

        $flags = $this->alloc(4 * 4);
        for ($i = 0; $i < 4; $i++) {
            $this->initUint32($flags + $i * 4, 0);
        }
        $slots = $this->alloc(4 * 4);
        $group = $this->alloc(2 * 0x20);
        $this->addressOf('_var_tasks_8c1ba5e8');

        // _quick_odd_mvn is mocked, so this keeps the values njTranslate reads.
        $point = $this->setSize('_var_groundQueryPoint_8c1bc460', 0xc);
        $this->initUint32($point + 0x0, $this->f(1.0));
        $this->initUint32($point + 0x4, $this->f(2.0));
        $this->initUint32($point + 0x8, $this->f(3.0));

        $models = $this->alloc(8 * 4);
        $objB = $this->alloc(0x30);
        $c1 = $this->alloc(0x34);
        $c2 = $this->alloc(0x34);
        $this->initUint32($models + 0x00, 0xaaaa0000);
        $this->initUint32($models + 0x04, $objB);
        $this->initUint32($objB + 0x2c, $c1);
        $this->initUint32($c1 + 0x30, $c2);
        $this->initUint32($c2 + 0x30, 0xcccc0002);
        $this->initUint32($this->addressOf('_var_routeModels_8c1bc3ec'), $models);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0xd4);

        $this->call('_SignalInit_8c02845a');

        $this->shouldCall('_syMalloc')->with(0x10)->andReturn($flags);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalFrames_8c227e24'), $flags);
        $this->shouldCall('_syMalloc')->with(0x10)->andReturn($slots);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalStates_8c227e28'), $slots);
        $this->shouldCall('_TrafficMarkSignalIdsInUse_8c026dcc')->with(3);
        $this->shouldWriteLong($flags + 1 * 4, 1);

        $this->shouldCall('_syMalloc')->with(0x40)->andReturn($group);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalTasks_8c227e20'), $group);
        $this->shouldCall('_TaskInitGroup_8c014a9c')->with($group, 1);

        [$taskLocal, $stateLocal] = $this->outParams();
        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with(
                $group,
                $this->addressOf('_headTask_8c028258'),
                $taskLocal,
                $stateLocal,
                0xd4,
            )
            ->do($this->fillTaskSpawn($task, $state));

        $this->shouldWriteLong($state + 0x00 * 4, 1);
        $this->shouldWriteLong($state + 0x01 * 4, 5);
        $this->shouldWriteLong($state + 0x02 * 4, $e1 + 0xb * 4);
        $this->shouldCall('__quick_odd_mvn')
            ->do($this->assertOddMvn($state + 0x1c, $e1 + 0x10));
        $this->shouldCall('__quick_odd_mvn')
            ->do($this->assertOddMvn($state + 0x28, $e1 + 0x20));
        $this->shouldWriteLong($state + 0x2d * 4, 0xaaaa0000);
        $this->shouldWriteLong($state + 0x2e * 4, $objB);
        $this->shouldWriteLong($state + 0x04 * 4, $c1);
        $this->shouldWriteLong($state + 0x05 * 4, $c2);
        $this->shouldWriteLong($state + 0x06 * 4, 0xcccc0002);

        // Matrix A: state + 0xd * 4 == state + 0x34
        $this->shouldCall('__quick_odd_mvn')
            ->do($this->assertOddMvn($point, $e1 + 0x10));
        $this->shouldCall('_snapPointToGround_8c02840c');
        $this->shouldCall('_njUnitMatrix')->with($state + 0x34);
        $this->shouldCall('_njTranslate')->with($state + 0x34, 1.0, 2.0, 3.0);
        // rotA is degrees; njRotateY takes BAMS, so 16384 * 65536 / 360.
        $this->shouldCall('_njRotateY')->with($state + 0x34, 2982616);

        $this->shouldWriteLong($state + 0x03 * 4, 0);
        $this->shouldWriteLong($state + 0x31 * 4, 0);
        $this->shouldWriteLong($state + 0x30 * 4, 0);
        $this->shouldWriteLong($state + 0x2f * 4, 0);
        $this->shouldWriteLong($flags + 1 * 4, 0);
        $this->shouldWriteLong($slots + 1 * 4, $state);

        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba5e8'),
                $this->addressOf('_execGroupTask_8c0283e8'),
                $taskLocal,
                $stateLocal,
                0,
            )
            ->do($this->fillTaskSpawn($task, $state));
    }

    public function test_spawns_traffic_signal_with_matrix_b_and_advances_frame()
    {
        $table = $this->alloc(3 * 0xe * 4);
        $e0 = $table;
        $e1 = $table + 0xe * 4;
        $this->initUint32($e0 + 0x00, 2);
        $this->initUint32($e0 + 0x04, 3);
        $this->initUint32($e0 + 0x08, 1);
        $this->initUint32($e1 + 0x00, 1);
        $this->initUint32($e1 + 0x04, 1);
        $this->initUint32($e1 + 0x08, 25); // linked id seeds state[1], enough for one frame step
        $this->initUint32($e1 + 0x10, 0);  // posA.x == 0 -> skip matrix A
        $this->initUint32($e1 + 0x1c, 32768); // rotB
        $this->initUint32($e1 + 0x20, $this->f(100.0)); // posB.x != 0 -> matrix B
        $this->initUint32($e1 + 0x2c, 10);
        $this->initUint32($e1 + 0x30, 20);
        $this->initUint32($e1 + 0x34, 30);
        $this->initUint32($table + 2 * 0xe * 4, 0);
        $course = $this->setSize('_var_currentCourse_8c1bb868', 0x50);
        $this->initUint32($course + 0x38, $table); // macSignal_0x38

        $flags = $this->alloc(4 * 4);
        for ($i = 0; $i < 4; $i++) {
            $this->initUint32($flags + $i * 4, 0);
        }
        $slots = $this->alloc(4 * 4);
        $group = $this->alloc(2 * 0x20);
        $this->addressOf('_var_tasks_8c1ba5e8');

        $point = $this->setSize('_var_groundQueryPoint_8c1bc460', 0xc);
        $this->initUint32($point + 0x0, $this->f(1.0));
        $this->initUint32($point + 0x4, $this->f(2.0));
        $this->initUint32($point + 0x8, $this->f(3.0));

        $models = $this->alloc(8 * 4);
        $objB = $this->alloc(0x30);
        $c1 = $this->alloc(0x34);
        $c2 = $this->alloc(0x34);
        $this->initUint32($models + 0x00, 0xaaaa0000);
        $this->initUint32($models + 0x04, $objB);
        $this->initUint32($objB + 0x2c, $c1);
        $this->initUint32($c1 + 0x30, $c2);
        $this->initUint32($c2 + 0x30, 0xcccc0002);
        $this->initUint32($this->addressOf('_var_routeModels_8c1bc3ec'), $models);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0xd4);

        $this->call('_SignalInit_8c02845a');

        $this->shouldCall('_syMalloc')->with(0x10)->andReturn($flags);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalFrames_8c227e24'), $flags);
        $this->shouldCall('_syMalloc')->with(0x10)->andReturn($slots);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalStates_8c227e28'), $slots);
        $this->shouldCall('_TrafficMarkSignalIdsInUse_8c026dcc')->with(3);
        $this->shouldWriteLong($flags + 1 * 4, 1);

        $this->shouldCall('_syMalloc')->with(0x40)->andReturn($group);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalTasks_8c227e20'), $group);
        $this->shouldCall('_TaskInitGroup_8c014a9c')->with($group, 1);

        [$taskLocal, $stateLocal] = $this->outParams();
        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with(
                $group,
                $this->addressOf('_headTask_8c028258'),
                $taskLocal,
                $stateLocal,
                0xd4,
            )
            ->do($this->fillTaskSpawn($task, $state));

        $this->shouldWriteLong($state + 0x00 * 4, 1);
        $this->shouldWriteLong($state + 0x01 * 4, 25);
        $this->shouldWriteLong($state + 0x02 * 4, $e1 + 0xb * 4);
        $this->shouldCall('__quick_odd_mvn')
            ->do($this->assertOddMvn($state + 0x1c, $e1 + 0x10));
        $this->shouldCall('__quick_odd_mvn')
            ->do($this->assertOddMvn($state + 0x28, $e1 + 0x20));
        $this->shouldWriteLong($state + 0x2d * 4, 0xaaaa0000);
        $this->shouldWriteLong($state + 0x2e * 4, $objB);
        $this->shouldWriteLong($state + 0x04 * 4, $c1);
        $this->shouldWriteLong($state + 0x05 * 4, $c2);
        $this->shouldWriteLong($state + 0x06 * 4, 0xcccc0002);

        // Matrix B: state + 0x1d * 4 == state + 0x74
        $this->shouldCall('__quick_odd_mvn')
            ->do($this->assertOddMvn($point, $e1 + 0x20));
        $this->shouldCall('_snapPointToGround_8c02840c');
        $this->shouldCall('_njUnitMatrix')->with($state + 0x74);
        $this->shouldCall('_njTranslate')->with($state + 0x74, 1.0, 2.0, 3.0);
        $this->shouldCall('_njRotateY')->with($state + 0x74, 5965232);

        // durations {10, 20, 30} vs state[1] == 25: one frame consumed.
        $this->shouldWriteLong($state + 0x03 * 4, 0);
        $this->shouldWriteLong($state + 0x01 * 4, 15);
        $this->shouldWriteLong($state + 0x03 * 4, 1);
        $this->shouldWriteLong($state + 0x31 * 4, 0);
        $this->shouldWriteLong($state + 0x30 * 4, 0);
        $this->shouldWriteLong($state + 0x2f * 4, 0);
        $this->shouldWriteLong($flags + 1 * 4, 1);
        $this->shouldWriteLong($slots + 1 * 4, $state);

        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba5e8'),
                $this->addressOf('_execGroupTask_8c0283e8'),
                $taskLocal,
                $stateLocal,
                0,
            )
            ->do($this->fillTaskSpawn($task, $state));
    }

    public function test_spawns_linked_object_of_type_2()
    {
        $this->runLinkedObjectCase(2);
    }

    public function test_spawns_linked_object_of_type_3()
    {
        $this->runLinkedObjectCase(3);
    }

    public function test_spawns_linked_object_of_type_4()
    {
        $this->runLinkedObjectCase(4);
    }

    public function test_linked_object_starts_open_when_gate_is_set()
    {
        $this->runLinkedObjectCase(2, 1);
    }

    public function test_link_search_skips_free_slots_and_the_task_it_just_pushed()
    {
        [$flags, $slots, $objC, $c1c] = $this->seedLinkedObjectFixture();

        // Slot 0 is free (-1) and slot 1 is the task just spawned; the search has
        // to walk past both to reach the match in slot 2.
        $group = $this->alloc(4 * 0x20);
        $task = $group + 0x20;
        $linked = $this->alloc(0xd4);
        $this->initUint32($linked + 0x00 * 4, 1);  // id matches eA[2]
        $this->initUint32($linked + 0x01 * 4, 10); // threshold, > state[1]
        $this->initUint32($linked + 0x03 * 4, 0);
        $this->initUint32($group + 0x00, 0xffffffff);
        $this->initUint32($group + 0x20, 1);
        $this->initUint32($group + 0x40, 1);
        $this->initUint32($group + 0x44, $linked);
        $this->initUint32($group + 0x60, 0);

        $state = $this->alloc(0xd4);

        $this->call('_SignalInit_8c02845a');

        $this->expectLinkedPrologue($flags, $slots, $group);
        [$taskLocal, $stateLocal] = $this->outParams();
        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with(
                $group,
                $this->addressOf('_attachmentTask_8c02833c'),
                $taskLocal,
                $stateLocal,
                0xd4,
            )
            ->do($this->fillTaskSpawn($task, $state));

        $this->shouldWriteLong($state + 0x34 * 4, $linked); // search hit, past both skips
        $this->expectLinkedBody($state, $linked, $objC, $c1c, $flags, 0);
        $this->expectGroupTaskSpawn($task, $state);
    }

    public function test_link_search_without_a_match_keeps_the_last_slot_it_examined()
    {
        [$flags, $slots, $objC, $c1c] = $this->seedLinkedObjectFixture();

        // Slot 0's id does not match, so the search falls off the end. The asm
        // loads the candidate state before comparing ids, so it stays latched.
        $group = $this->alloc(2 * 0x20);
        $other = $this->alloc(0xd4);
        $this->initUint32($other + 0x00 * 4, 99); // id does NOT match eA[2]
        $this->initUint32($other + 0x01 * 4, 10);
        $this->initUint32($other + 0x03 * 4, 0);
        $this->initUint32($group + 0x00, 1);
        $this->initUint32($group + 0x04, $other);
        $this->initUint32($group + 0x20, 0);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0xd4);

        $this->call('_SignalInit_8c02845a');

        $this->expectLinkedPrologue($flags, $slots, $group);
        [$taskLocal, $stateLocal] = $this->outParams();
        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with(
                $group,
                $this->addressOf('_attachmentTask_8c02833c'),
                $taskLocal,
                $stateLocal,
                0xd4,
            )
            ->do($this->fillTaskSpawn($task, $state));

        // No attachedTo write: that only happens on the matching break.
        $this->expectLinkedBody($state, $other, $objC, $c1c, $flags, 0);
        $this->expectGroupTaskSpawn($task, $state);
    }

    public function test_link_search_that_skips_every_slot_writes_through_the_carried_pointer()
    {
        [$flags, $slots, $objC, $c1c, $carried] = $this->seedLinkedObjectFixture();

        // The only slot is free, so nothing is ever latched and the back-link
        // still points at the initial var_groundQueryPoint_8c1bc460.
        $group = $this->alloc(2 * 0x20);
        $this->initUint32($group + 0x00, 0xffffffff);
        $this->initUint32($group + 0x20, 0);

        $this->initUint32($carried + 0x01 * 4, 10);
        $this->initUint32($carried + 0x03 * 4, 0);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0xd4);

        $this->call('_SignalInit_8c02845a');

        $this->expectLinkedPrologue($flags, $slots, $group);
        [$taskLocal, $stateLocal] = $this->outParams();
        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with(
                $group,
                $this->addressOf('_attachmentTask_8c02833c'),
                $taskLocal,
                $stateLocal,
                0xd4,
            )
            ->do($this->fillTaskSpawn($task, $state));

        $this->expectLinkedBody($state, $carried, $objC, $c1c, $flags, 0);
        $this->expectGroupTaskSpawn($task, $state);
    }

    /**
     * Table/flag/model state shared by the link-search cases: one type 2 entry
     * that flags its own id, plus an unflagged type 1 entry both loops skip.
     */
    private function seedLinkedObjectFixture(): array
    {
        $table = $this->alloc(3 * 0xe * 4);
        $eA = $table;
        $eB = $table + 0xe * 4;
        $this->initUint32($eA + 0x00, 2); // type
        $this->initUint32($eA + 0x04, 1); // id
        $this->initUint32($eA + 0x08, 1); // linked id (also what the search matches)
        $this->initUint32($eA + 0x2c, 5); // [0xb] seeds state[1]
        $this->initUint32($eB + 0x00, 1);
        $this->initUint32($eB + 0x04, 2);
        $this->initUint32($eB + 0x08, 0);
        $this->initUint32($table + 2 * 0xe * 4, 0);
        $course = $this->setSize('_var_currentCourse_8c1bb868', 0x50);
        $this->initUint32($course + 0x38, $table); // macSignal_0x38

        $flags = $this->alloc(3 * 4);
        for ($i = 0; $i < 3; $i++) {
            $this->initUint32($flags + $i * 4, 0);
        }
        $slots = $this->alloc(3 * 4);

        $models = $this->alloc(8 * 4);
        $objC = $this->alloc(0x30);
        $c1c = $this->alloc(0x34);
        $this->initUint32($models + 2 * 4, 0xbbbb0000);
        $this->initUint32($models + 3 * 4, $objC);
        $this->initUint32($objC + 0x2c, $c1c);
        $this->initUint32($this->addressOf('_var_routeModels_8c1bc3ec'), $models);

        // Sized as a TrafficSignal: the no-match path back-links through it.
        $carried = $this->setSize('_var_groundQueryPoint_8c1bc460', 0xd4);

        return [$flags, $slots, $objC, $c1c, $carried];
    }

    /** Allocation and marking effects up to the linked-object TaskSpawn. */
    private function expectLinkedPrologue(int $flags, int $slots, int $group): void
    {
        $this->shouldCall('_syMalloc')->with(0xc)->andReturn($flags);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalFrames_8c227e24'), $flags);
        $this->shouldCall('_syMalloc')->with(0xc)->andReturn($slots);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalStates_8c227e28'), $slots);
        $this->shouldCall('_TrafficMarkSignalIdsInUse_8c026dcc')->with(2);
        $this->shouldWriteLong($flags + 1 * 4, 1);
        $this->shouldCall('_syMalloc')->with(0x40)->andReturn($group);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalTasks_8c227e20'), $group);
        $this->shouldCall('_TaskInitGroup_8c014a9c')->with($group, 1);
    }

    /** Type 2 state fill and back-link, whichever object the search left latched. */
    private function expectLinkedBody(
        int $state,
        int $linked,
        int $objC,
        int $c1c,
        int $flags,
        int $frame,
    ): void {
        $this->shouldWriteLong($state + 0x00 * 4, 1);
        $this->shouldWriteLong($state + 0x01 * 4, 5);
        $this->shouldWriteLong($state + 0x2d * 4, 0xbbbb0000);
        $this->shouldWriteLong($state + 0x2e * 4, $objC);
        $this->shouldWriteLong($linked + 0x2f * 4, $state);
        $this->shouldWriteLong($state + 0x04 * 4, $c1c);
        $this->shouldWriteLong($state + 0x03 * 4, $frame);
        $this->shouldWriteLong($flags + 1 * 4, $frame);
    }

    /** The group task installed once both spawn loops finish. */
    private function expectGroupTaskSpawn(int $task, int $state): void
    {
        [$taskLocal, $stateLocal] = $this->outParams();
        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba5e8'),
                $this->addressOf('_execGroupTask_8c0283e8'),
                $taskLocal,
                $stateLocal,
                0,
            )
            ->do($this->fillTaskSpawn($task, $state));
    }

    /**
     * Types 2/3/4 differ only in which routeModels pair they take and which
     * back-link slot of the linked task they store themselves in.
     */
    private function runLinkedObjectCase(int $type, int $gate = 0): void
    {
        $modelIndex = 2 * ($type - 1);
        $backlinkIndex = 0x2f + ($type - 2);

        // entryA flags its own id, so it is picked up by the second spawn
        // loop; entryB is never flagged and is skipped by both loops.
        $table = $this->alloc(3 * 0xe * 4);
        $eA = $table;
        $eB = $table + 0xe * 4;
        $this->initUint32($eA + 0x00, $type);
        $this->initUint32($eA + 0x04, 1); // id
        $this->initUint32($eA + 0x08, 1); // linked id (also what the search matches)
        $this->initUint32($eA + 0x2c, 5); // [0xb] seeds state[1]
        $this->initUint32($eB + 0x00, 1);
        $this->initUint32($eB + 0x04, 2);
        $this->initUint32($eB + 0x08, 0);
        $this->initUint32($table + 2 * 0xe * 4, 0);
        $course = $this->setSize('_var_currentCourse_8c1bb868', 0x50);
        $this->initUint32($course + 0x38, $table); // macSignal_0x38

        $flags = $this->alloc(3 * 4);
        for ($i = 0; $i < 3; $i++) {
            $this->initUint32($flags + $i * 4, 0);
        }
        $slots = $this->alloc(3 * 4);
        $this->addressOf('_var_groundQueryPoint_8c1bc460');

        // The task the search should link to: state[0] matches entryA's linked id.
        $linked = $this->alloc(0xd4);
        $this->initUint32($linked + 0x00 * 4, 1);  // id matches eA[2]
        $this->initUint32($linked + 0x01 * 4, 10); // threshold, > state[1]
        $this->initUint32($linked + 0x03 * 4, $gate);

        // Task group slot 0 points at $linked; slot 1 terminates the search.
        $group = $this->alloc(2 * 0x20);
        $this->initUint32($group + 0x00, 1);
        $this->initUint32($group + 0x04, $linked);
        $this->initUint32($group + 0x20, 0);

        $models = $this->alloc(8 * 4);
        $objC = $this->alloc(0x30);
        $c1c = $this->alloc(0x34);
        $this->initUint32($models + $modelIndex * 4, 0xbbbb0000);
        $this->initUint32($models + ($modelIndex + 1) * 4, $objC);
        $this->initUint32($objC + 0x2c, $c1c);
        $this->initUint32($this->addressOf('_var_routeModels_8c1bc3ec'), $models);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0xd4);

        $this->call('_SignalInit_8c02845a');

        $this->shouldCall('_syMalloc')->with(0xc)->andReturn($flags);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalFrames_8c227e24'), $flags);
        $this->shouldCall('_syMalloc')->with(0xc)->andReturn($slots);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalStates_8c227e28'), $slots);
        $this->shouldCall('_TrafficMarkSignalIdsInUse_8c026dcc')->with(2);
        $this->shouldWriteLong($flags + 1 * 4, 1);

        $this->shouldCall('_syMalloc')->with(0x40)->andReturn($group);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalTasks_8c227e20'), $group);
        $this->shouldCall('_TaskInitGroup_8c014a9c')->with($group, 1);

        [$taskLocal, $stateLocal] = $this->outParams();
        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with(
                $group,
                $this->addressOf('_attachmentTask_8c02833c'),
                $taskLocal,
                $stateLocal,
                0xd4,
            )
            ->do($this->fillTaskSpawn($task, $state));

        $this->shouldWriteLong($state + 0x34 * 4, $linked); // search hit
        $this->shouldWriteLong($state + 0x00 * 4, 1);
        $this->shouldWriteLong($state + 0x01 * 4, 5);
        $this->shouldWriteLong($state + 0x2d * 4, 0xbbbb0000);
        $this->shouldWriteLong($state + 0x2e * 4, $objC);
        $this->shouldWriteLong($linked + $backlinkIndex * 4, $state);
        $this->shouldWriteLong($state + 0x04 * 4, $c1c);
        // state[3] is 0 only when the gate is clear and 5 < 10.
        $frame = $gate === 0 ? 0 : 1;
        $this->shouldWriteLong($state + 0x03 * 4, $frame);
        $this->shouldWriteLong($flags + 1 * 4, $frame);

        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba5e8'),
                $this->addressOf('_execGroupTask_8c0283e8'),
                $taskLocal,
                $stateLocal,
                0,
            )
            ->do($this->fillTaskSpawn($task, $state));
    }
};
