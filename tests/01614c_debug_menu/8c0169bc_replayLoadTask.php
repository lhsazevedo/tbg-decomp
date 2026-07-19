<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_no_vmu_selected_frees_task(): void
    {
        $this->setupExterns();
        $task = $this->alloc(0x20);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0xffffffff);

        $this->call('_replayLoadTask_8c0169bc')->with($task, 0);

        $this->shouldCall('_TaskFree_8c014b66')->with($task);
        $this->shouldCall('_FUN_8c01328c');
    }

    public function test_state0_not_connected_resets_and_returns_to_normal(): void
    {
        $this->setupExterns();
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 0);

        $bup = $this->alloc(0x5c);
        $this->initUint32($bup + 0x4c, 0); // Connect == 0

        $this->call('_replayLoadTask_8c0169bc')->with($task, 0);

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bup);
        $this->shouldWriteLongTo('_var_selectedVm_8c1ba34c', 0xffffffff);
        $this->shouldWriteLongTo('_var_playMode_8c1bb8d0', 0);
    }

    public function test_state0_connected_not_mounted_mounts_and_goes_state1(): void
    {
        $this->setupExterns();
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 0);

        $bup = $this->alloc(0x5c);
        $this->initUint32($bup + 0x4c, 1); // Connect != 0
        $this->initUint32($bup + 0x50, 0); // Work == NULL -> needs mount

        $this->call('_replayLoadTask_8c0169bc')->with($task, 0);

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bup);
        $this->shouldCall('_BupMount_8c014c00')->with(0);
        $this->shouldWriteLong($task + 0x08, 1);
    }

    public function test_state0_connected_and_mounted_goes_state2(): void
    {
        $this->setupExterns();
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 0);

        $bup = $this->alloc(0x5c);
        $this->initUint32($bup + 0x4c, 1);      // Connect != 0
        $this->initUint32($bup + 0x50, 0x1234); // Work != NULL -> already mounted

        $this->call('_replayLoadTask_8c0169bc')->with($task, 0);

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bup);
        $this->shouldWriteLong($task + 0x08, 2);
    }

    public function test_state1_not_ready_noop(): void
    {
        $this->setupExterns();
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 1);

        $bup = $this->alloc(0x5c);
        $this->initUint32($bup + 0x00, 0); // Ready == 0

        $this->call('_replayLoadTask_8c0169bc')->with($task, 0);

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bup);
    }

    public function test_state1_ready_goes_state2(): void
    {
        $this->setupExterns();
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 1);

        $bup = $this->alloc(0x5c);
        $this->initUint16($bup + 0x00, 1); // Ready != 0

        $this->call('_replayLoadTask_8c0169bc')->with($task, 0);

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bup);
        $this->shouldWriteLong($task + 0x08, 2);
    }

    public function test_state2_starts_load_and_falls_into_state3(): void
    {
        $this->setupExterns();
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 2);

        $malloc = $this->alloc(0x4000);

        $bup = $this->alloc(0x5c);
        $this->initUint32($bup + 0x08, 42);  // ProgressCount
        $this->initUint32($bup + 0x0c, 100); // ProgressMax

        $this->call('_replayLoadTask_8c0169bc')->with($task, 0);

        $this->shouldCall('_syMalloc')->with(0x4000)->andReturn($malloc);
        $this->shouldWriteLongTo('_var_demoBuf_8c1ba3c4', $malloc);
        $this->shouldCall('_BupLoad_8c014bc6')->with(0, "BUS_REPLAY", $malloc);
        $this->shouldWriteLong($task + 0x08, 3);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bup);
        $this->shouldCall('__divlu');
        $this->shouldCall('_njPrint')->variadic(1)->with(0x40008, "NOW LOADING...(%03d%%)", 42);
        $this->shouldCall('_buStat')->with(0)->andReturn(-1); // still busy
    }

    public function test_state3_still_loading_noop(): void
    {
        $this->setupExterns();
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 3);

        $bup = $this->alloc(0x5c);
        $this->initUint32($bup + 0x08, 10);
        $this->initUint32($bup + 0x0c, 100);

        $this->call('_replayLoadTask_8c0169bc')->with($task, 0);

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bup);
        $this->shouldCall('__divlu');
        $this->shouldCall('_njPrint')->variadic(1)->with(0x40008, "NOW LOADING...(%03d%%)", 10);
        $this->shouldCall('_buStat')->with(0)->andReturn(-1);
    }

    public function test_state3_finished_unpacks_and_exits(): void
    {
        $this->setupExterns();
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 3);

        $demoBuf = $this->alloc(0x20);
        $this->initUint32($this->addressOf('_var_demoBuf_8c1ba3c4'), $demoBuf);
        $this->initUint32($demoBuf + 0x00, 0x30);   // size
        $this->initUint32($demoBuf + 0x04, 7);      // courseId
        $this->initUint32($demoBuf + 0x08, 3);      // inputMapSel
        $this->initUint32($demoBuf + 0x0c, 0x1234); // seed

        $bup = $this->alloc(0x5c);
        $this->initUint32($bup + 0x08, 100);
        $this->initUint32($bup + 0x0c, 100);

        $this->call('_replayLoadTask_8c0169bc')->with($task, 0);

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bup);
        $this->shouldCall('__divlu');
        $this->shouldCall('_njPrint')->variadic(1)->with(0x40008, "NOW LOADING...(%03d%%)", 100);
        $this->shouldCall('_buStat')->with(0)->andReturn(0);
        $this->shouldWriteLongTo('_var_currentCourse_8c1bb868', 7);
        $this->shouldWriteLongTo('_var_inputMapSel_8c1bb8c8', 3);
        $this->shouldWriteLongTo('_var_seed_8c157a64', 0x1234);
        $this->shouldCall('_FUN_8c02f320');
        $this->shouldCall('_FUN_readDemo_8c02fa14')
            ->with($demoBuf + 0x10, $this->destOffset(), 0x30);
        $this->shouldCall('_syFree')->with($demoBuf);
        $this->shouldWriteLongTo('_var_demoBuf_8c1ba3c4', 0xffffffff);
        $this->shouldCall('_BupUnmount_8c014c46')->with(0);
        $this->shouldCall('_TaskFree_8c014b66')->with($task);
        $this->shouldCall('_FUN_8c01328c');
    }

    public function test_installer_no_vmu_selected_resets_play_mode(): void
    {
        $this->setupExterns();
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0xffffffff);

        $this->call('_startReplayLoad_8c016b4c');

        $this->shouldWriteLongTo('_var_playMode_8c1bb8d0', 0);
    }

    public function test_installer_starts_load_task(): void
    {
        $this->setupExterns();
        $task = $this->alloc(0x20);

        $this->call('_startReplayLoad_8c016b4c');

        $this->shouldWriteLongTo('_var_playMode_8c1bb8d0', 2);
        $this->shouldWriteLongTo('_var_8c1bb8d4', 0);
        $this->shouldCall('_TaskPush_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_replayLoadTask_8c0169bc'),
                $this->installerTaskOffset(),
                $this->installerStateOffset(),
                0,
            )
            ->do(function ($params) use ($task) {
                $this->memory->writeUInt32($params[2], U32::of($task));
            });
        $this->shouldWriteLong($task + 0x08, 0);
    }

    private function installerTaskOffset(): int
    {
        return 0xffffec;
    }

    private function installerStateOffset(): int
    {
        return 0xfffff0;
    }

    private function destOffset(): int
    {
        return str_contains($this->objectFile, '_src.obj') ? 0xffffd8 : 0xffffe4;
    }

    private function setupExterns(): void
    {
        $this->setSize('_var_selectedVm_8c1ba34c', 4);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_8c1bb8d4', 4);
        $this->setSize('_var_tasks_8c1ba3c8', 4);
        $this->setSize('_TaskPush_8c014ae8', 4);
        $this->setSize('_BupGetInfo_8c014bba', 4);
        $this->setSize('_BupMount_8c014c00', 4);
        $this->setSize('_BupLoad_8c014bc6', 4);
        $this->setSize('_TaskFree_8c014b66', 4);
        $this->setSize('_FUN_8c01328c', 4);
        $this->setSize('_var_demoBuf_8c1ba3c4', 4);
        $this->setSize('_var_demoBuffer_8c1bc828', 4);
        $this->setSize('_var_currentCourse_8c1bb868', 0x50);
        $this->setSize('_var_inputMapSel_8c1bb8c8', 4);
        $this->setSize('_var_seed_8c157a64', 4);
        $this->setSize('_buStat', 4);
        $this->setSize('_syFree', 4);
        $this->setSize('_syMalloc', 4);
        $this->setSize('_BupUnmount_8c014c46', 4);
        $this->setSize('_njPrint', 4);
        $this->setSize('__divlu', 4);
        $this->setSize('_FUN_8c02f320', 4);
        $this->setSize('_FUN_readDemo_8c02fa14', 4);
        $this->onCall('__divlu', function () {
            $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
        });
    }
};
