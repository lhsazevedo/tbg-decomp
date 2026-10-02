<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// updateTask_8c0275d4 is a hidden (never directly called) TaskAction
// -- its address is only taken via a .DATA.L pool entry and passed to
// TaskInstall_8c014ae8, installed with no extra state, so its own Task struct
// doubles as the state. See the function's header comment in
// 026710_traffic.c for the full field/branch breakdown.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_runState_8c2285c4', 0x9c);
        $this->setSize('_var_currentCourse_8c1bb868', 0x50);
        $this->setSize('_var_activeGroundGrid_8c2264d4', 4);
        $this->setSize('_var_activeAttrGrid_8c228b3c', 4);
        $this->setSize('_SignalClearCrossingOccupied_8c028958', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_RenderQueueDraw_8c0223ea', 4);
        $this->setSize('_TaskRunGroup_8c014b42', 4);
        $this->setSize('_var_tasks_8c1bac28', 4);
    }

    private function makeTask(int $field8, int $fieldC, int $queuedItem18): int {
        // Task = {action, state, field_0x08, field_0x0c, field_0x10,
        // field_0x14, queuedItem_0x18, field_0x1c}
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, $field8);
        $this->initUint32($task + 0x0c, $fieldC);
        $this->initUint32($task + 0x18, $queuedItem18);
        return $task;
    }

    // Record = {typeCode:u16, threshold:u16, script:u32, progress:float}, 0xc bytes.
    private function makeRecord(int $typeCode, int $threshold, int $script, float $progress): int {
        $rec = $this->alloc(0xc);
        $this->initUint16($rec + 0, $typeCode);
        $this->initUint16($rec + 2, $threshold);
        $this->initUint32($rec + 4, $script);
        $this->initFloat($rec + 8, $progress);
        return $rec;
    }

    private function initFloat(int $addr, float $value): void {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    // Before the run starts (var_runState_8c2285c4.runPhase_0x00 == 0) the function is a
    // pure no-op: no ground-grid selection, no demo-entry bookkeeping, no
    // record processing, no lighting/task-group calls.
    public function test_noOpBeforeTheRunStarts(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_runState_8c2285c4'), 0);

        $task = $this->makeTask(0, 0, 0);

        $this->call('_updateTask_8c0275d4')->with($task, 0);

        // No writes, no calls -- reaching the epilogue directly.
    }

    // Run under way, field_0x0c == 0 and no demo pending (bits 8-15 of
    // busState.scenePresetIds_0x3bc clear): arms field_0x0c = 2. The
    // record's script dword is 0, so the record block is skipped entirely.
    public function test_driving_armsWaitingState_noRecord(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_runState_8c2285c4'), 1);

        $course = $this->addressOf('_var_currentCourse_8c1bb868');
        $this->initUint32($course + 0x18, 0x11110000); // atariCpu_0x18
        $this->initUint32($course + 0x20, 0x22220000); // attrCpu_0x20

        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x3bc), 0);

        $rec = $this->makeRecord(0, 0, 0, 0.0); // script == 0 -> record block skipped
        $task = $this->makeTask(0, 0, $rec);

        $this->call('_updateTask_8c0275d4')->with($task, 0);

        $this->shouldWriteLong($this->addressOf('_var_activeGroundGrid_8c2264d4'), 0x11110000);
        $this->shouldWriteLong($this->addressOf('_var_activeAttrGrid_8c228b3c'), 0x22220000);
        $this->shouldCall('_SignalClearCrossingOccupied_8c028958');
        $this->shouldWriteLong($this->addressOf('_var_occupiedGroup_8c228b44'), 0xffffffff);

        $this->shouldWriteLong($task + 0x0c, 2);

        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(0, $this->addressOf('_applyLighting_8c02756a'), 0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(1, $this->addressOf('_applyLighting_8c02756a'), 1);
        $this->shouldCall('_TaskRunGroup_8c014b42')->with($this->addressOf('_var_tasks_8c1bac28'));
    }

    // field_0x0c != 0 and a *different* demo id is now pending: latches the
    // new id into var_activeTrafficPreset_8c227e14, refreshes the task's script
    // cursor from var_trafficPresetTable_8c227e18[demoId], and resets field_0x0c to 0. The
    // freshly-loaded record's script is again 0, so no spawn.
    public function test_demoSwitch_refreshesScriptCursor(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_runState_8c2285c4'), 1);

        $course = $this->addressOf('_var_currentCourse_8c1bb868');
        $this->initUint32($course + 0x18, 0);
        $this->initUint32($course + 0x20, 0);

        // demo id 3 pending in bits 8-15
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x3bc), 3 << 8);
        $this->initUint32($this->addressOf('_var_activeTrafficPreset_8c227e14'), 1); // currently 1, differs from 3

        $table = $this->alloc(4 * 4);
        $newRec = $this->makeRecord(0, 0, 0, 0.0); // script == 0
        $this->initUint32($table + 3 * 4, $newRec);
        $this->initUint32($this->addressOf('_var_trafficPresetTable_8c227e18'), $table);

        $oldRec = $this->makeRecord(0, 0, 0, 0.0);
        $task = $this->makeTask(0, 1, $oldRec); // field_0x0c == 1 (nonzero)

        $this->call('_updateTask_8c0275d4')->with($task, 0);

        $this->shouldWriteLong($this->addressOf('_var_activeGroundGrid_8c2264d4'), 0);
        $this->shouldWriteLong($this->addressOf('_var_activeAttrGrid_8c228b3c'), 0);
        $this->shouldCall('_SignalClearCrossingOccupied_8c028958');
        $this->shouldWriteLong($this->addressOf('_var_occupiedGroup_8c228b44'), 0xffffffff);

        $this->shouldWriteLong($this->addressOf('_var_activeTrafficPreset_8c227e14'), 3);
        $this->shouldWriteLong($task + 0x18, $newRec);
        $this->shouldWriteLong($task + 0x0c, 0);

        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(0, $this->addressOf('_applyLighting_8c02756a'), 0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(1, $this->addressOf('_applyLighting_8c02756a'), 1);
        $this->shouldCall('_TaskRunGroup_8c014b42')->with($this->addressOf('_var_tasks_8c1bac28'));
    }

    // A record with a nonzero script whose counter (field_0x08) has not yet
    // exceeded the threshold: the counter is incremented and written back,
    // but the record is not consumed (no spawn, no cursor advance).
    public function test_record_counterNotYetPastThreshold(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_runState_8c2285c4'), 1);
        $course = $this->addressOf('_var_currentCourse_8c1bb868');
        $this->initUint32($course + 0x18, 0);
        $this->initUint32($course + 0x20, 0);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x3bc), 0);

        $rec = $this->makeRecord(0x14, 5, 0xdeadbeef, 1.5);
        $task = $this->makeTask(3, 5, $rec); // old counter 3 < threshold 5; field_0x0c == 5 (avoid the arm/demo-switch branches)

        $this->call('_updateTask_8c0275d4')->with($task, 0);

        $this->shouldWriteLong($this->addressOf('_var_activeGroundGrid_8c2264d4'), 0);
        $this->shouldWriteLong($this->addressOf('_var_activeAttrGrid_8c228b3c'), 0);
        $this->shouldCall('_SignalClearCrossingOccupied_8c028958');
        $this->shouldWriteLong($this->addressOf('_var_occupiedGroup_8c228b44'), 0xffffffff);

        $this->shouldWriteLong($task + 0x08, 4);

        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(0, $this->addressOf('_applyLighting_8c02756a'), 0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(1, $this->addressOf('_applyLighting_8c02756a'), 1);
        $this->shouldCall('_TaskRunGroup_8c014b42')->with($this->addressOf('_var_tasks_8c1bac28'));
    }

    // Counter passes the threshold and field_0x0c == 1 with a zero progress
    // -- real asm quirk: (field_0x0c != 1 && progress != 0) is false, so
    // spawnEntry_8c0272b8 IS called (the OR's second operand), and
    // its nonzero return advances the cursor and resets the counter.
    public function test_record_thresholdPassed_spawnsViaSpawnEntry(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_runState_8c2285c4'), 1);
        $course = $this->addressOf('_var_currentCourse_8c1bb868');
        $this->initUint32($course + 0x18, 0);
        $this->initUint32($course + 0x20, 0);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x3bc), 0);

        $rec = $this->makeRecord(0x14, 2, 0xdeadbeef, 0.0);
        $task = $this->makeTask(3, 1, $rec); // old counter 3 > threshold 2; field_0x0c == 1

        $this->call('_updateTask_8c0275d4')->with($task, 0);

        $this->shouldWriteLong($this->addressOf('_var_activeGroundGrid_8c2264d4'), 0);
        $this->shouldWriteLong($this->addressOf('_var_activeAttrGrid_8c228b3c'), 0);
        $this->shouldCall('_SignalClearCrossingOccupied_8c028958');
        $this->shouldWriteLong($this->addressOf('_var_occupiedGroup_8c228b44'), 0xffffffff);

        $this->shouldWriteLong($task + 0x08, 4);

        $this->shouldCall('_spawnEntry_8c0272b8')->with(0x14, 0.0, 0xdeadbeef)->andReturn(1);

        $this->shouldWriteLong($task + 0x18, $rec + 0xc);
        $this->shouldWriteLong($task + 0x08, 0);

        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(0, $this->addressOf('_applyLighting_8c02756a'), 0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(1, $this->addressOf('_applyLighting_8c02756a'), 1);
        $this->shouldCall('_TaskRunGroup_8c014b42')->with($this->addressOf('_var_tasks_8c1bac28'));
    }

    // Counter passes the threshold, field_0x0c != 1, and the record's own
    // progress is nonzero: the "already spawned" shortcut takes over --
    // spawnEntry_8c0272b8 is never called, yet the cursor still
    // advances and the counter still resets (real asm quirk).
    public function test_record_thresholdPassed_shortcutSkipsSpawnCall(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_runState_8c2285c4'), 1);
        $course = $this->addressOf('_var_currentCourse_8c1bb868');
        $this->initUint32($course + 0x18, 0);
        $this->initUint32($course + 0x20, 0);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x3bc), 0);

        $rec = $this->makeRecord(0x14, 2, 0xdeadbeef, 1.5); // progress != 0
        $task = $this->makeTask(3, 5, $rec); // old counter 3 > threshold 2; field_0x0c == 5 (!= 1, and != 0 to avoid the arm branch)

        $this->call('_updateTask_8c0275d4')->with($task, 0);

        $this->shouldWriteLong($this->addressOf('_var_activeGroundGrid_8c2264d4'), 0);
        $this->shouldWriteLong($this->addressOf('_var_activeAttrGrid_8c228b3c'), 0);
        $this->shouldCall('_SignalClearCrossingOccupied_8c028958');
        $this->shouldWriteLong($this->addressOf('_var_occupiedGroup_8c228b44'), 0xffffffff);

        $this->shouldWriteLong($task + 0x08, 4);

        // spawnEntry_8c0272b8 NOT called.

        $this->shouldWriteLong($task + 0x18, $rec + 0xc);
        $this->shouldWriteLong($task + 0x08, 0);

        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(0, $this->addressOf('_applyLighting_8c02756a'), 0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(1, $this->addressOf('_applyLighting_8c02756a'), 1);
        $this->shouldCall('_TaskRunGroup_8c014b42')->with($this->addressOf('_var_tasks_8c1bac28'));
    }
};
