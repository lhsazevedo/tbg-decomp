<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /* State 0: first file is missing on the VMU -> request it and advance to state 1. */
    public function test_state0_missing_file_loads_and_goes_state1(): void
    {
        $this->setupExterns();

        $buf = $this->alloc(0x1000);
        $this->initUint32($this->addressOf('_var_8c225fe0'), $buf);
        $this->initUint32($this->addressOf('_var_8c22600c'), 0);

        $name0 = $this->allocString("SAVE1");
        $names = $this->alloc(0x8);
        $this->initUint32($names + 0, $name0);
        $this->initUint32($names + 4, $this->allocString(""));

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 0);
        $this->initUint32($task + 0x0c, 5);
        $this->initUint32($task + 0x18, $names);

        $this->call('_VmLoadTask_8c018644')->with($task);

        $this->shouldCall('_buIsExistFile')->with(0, $name0)->andReturn(0);
        $this->shouldCall('_BupLoad_8c014bc6')->with(0, $name0, $buf);
        $this->shouldWriteLongTo('_var_8c225fe4', 5);
        $this->shouldWriteLongTo('_var_8c22600c', 1);
        $this->shouldWriteLong($task + 0x18, $names + 4);
        $this->shouldWriteLong($task + 0x08, 1);
        $this->shouldWriteLong($task + 0x0c, 6);
    }

    /* State 0: an already-present file (err -0xfb) is skipped; the next missing one loads. */
    public function test_state0_skips_present_then_loads_next(): void
    {
        $this->setupExterns();

        $buf = $this->alloc(0x1000);
        $this->initUint32($this->addressOf('_var_8c225fe0'), $buf);
        $this->initUint32($this->addressOf('_var_8c22600c'), 0);

        $name0 = $this->allocString("SAVE1");
        $name1 = $this->allocString("SAVE2");
        $names = $this->alloc(0xc);
        $this->initUint32($names + 0, $name0);
        $this->initUint32($names + 4, $name1);
        $this->initUint32($names + 8, $this->allocString(""));

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 0);
        $this->initUint32($task + 0x0c, 5);
        $this->initUint32($task + 0x18, $names);

        $this->call('_VmLoadTask_8c018644')->with($task);

        $this->shouldCall('_buIsExistFile')->with(0, $name0)->andReturn(-0xfb);
        $this->shouldWriteLong($task + 0x0c, 6);
        $this->shouldCall('_buIsExistFile')->with(0, $name1)->andReturn(0);
        $this->shouldCall('_BupLoad_8c014bc6')->with(0, $name1, $buf);
        $this->shouldWriteLongTo('_var_8c225fe4', 6);
        $this->shouldWriteLongTo('_var_8c22600c', 1);
        $this->shouldWriteLong($task + 0x18, $names + 8);
        $this->shouldWriteLong($task + 0x08, 1);
        $this->shouldWriteLong($task + 0x0c, 7);
    }

    /* State 0: an unexpected buIsExistFile error frees the task and reports failure (2). */
    public function test_state0_error_frees_task(): void
    {
        $this->setupExterns();

        $this->initUint32($this->addressOf('_var_8c22600c'), 0);

        $name0 = $this->allocString("SAVE1");
        $names = $this->alloc(0x8);
        $this->initUint32($names + 0, $name0);
        $this->initUint32($names + 4, $this->allocString(""));

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 0);
        $this->initUint32($task + 0x18, $names);

        $this->call('_VmLoadTask_8c018644')->with($task);

        $this->shouldCall('_buIsExistFile')->with(0, $name0)->andReturn(5);
        $this->shouldCall('_TaskFree_8c014b66')->with($task);
        $this->shouldWriteLongTo('_var_8c226010', 2);
    }

    /* State 0: empty list terminator -> all files present, report success (1). */
    public function test_state0_list_end_frees_task_success(): void
    {
        $this->setupExterns();

        $names = $this->alloc(0x4);
        $this->initUint32($names + 0, $this->allocString(""));

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 0);
        $this->initUint32($task + 0x18, $names);

        $this->call('_VmLoadTask_8c018644')->with($task);

        $this->shouldCall('_TaskFree_8c014b66')->with($task);
        $this->shouldWriteLongTo('_var_8c226010', 1);
    }

    /* Unknown state -> dispatch falls through with no side effects. */
    public function test_unknown_state_noop(): void
    {
        $this->setupExterns();

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 2);

        $this->call('_VmLoadTask_8c018644')->with($task);
    }

    /* State 1: drive still busy (buStat != 0) -> no-op. */
    public function test_state1_busy_noop(): void
    {
        $this->setupExterns();

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 1);

        $this->call('_VmLoadTask_8c018644')->with($task);

        $this->shouldCall('_buStat')->with(0)->andReturn(1);
    }

    /* State 1: drive ready but last op errored -> free task and report failure (2). */
    public function test_state1_error_frees_task(): void
    {
        $this->setupExterns();

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 1);

        $this->call('_VmLoadTask_8c018644')->with($task);

        $this->shouldCall('_buStat')->with(0)->andReturn(0);
        $this->shouldCall('_buGetLastError')->with(0)->andReturn(5);
        $this->shouldCall('_TaskFree_8c014b66')->with($task);
        $this->shouldWriteLongTo('_var_8c226010', 2);
    }

    /* State 1: load complete -> analyze image, copy header, advance buffer, back to state 0. */
    public function test_state1_success_analyzes_and_resets(): void
    {
        $this->setupExterns();

        $buf = $this->alloc(0x1000);
        $this->initUint32($this->addressOf('_var_8c225fe0'), $buf);

        $src = $this->alloc(0x100);
        $this->initUint32($this->addressOf('_var_8c1ba33c'), $src);

        $mem = $this->alloc(0x100);

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 1);

        $this->call('_VmLoadTask_8c018644')->with($task);

        $this->shouldCall('_buStat')->with(0)->andReturn(0);
        $this->shouldCall('_buGetLastError')->with(0)->andReturn(0);
        $this->shouldCall('_syMalloc')->with(0xe8)->andReturn($mem);
        $this->shouldWriteLongTo('_var_8c1ba348', $mem);
        $this->shouldCall('_buAnalyzeBackupFileImage')->with($this->addressOf('_var_8c1ba2e4'), $buf);
        $this->shouldCall('_njMemCopy')->with($mem, $src, 0xe8);
        $this->shouldCall('_njMemCopy')->with($buf, $mem, 0xe8);
        $this->shouldCall('_syFree')->with($mem);
        $this->shouldWriteLongTo('_var_8c1ba348', 0xffffffff);
        $this->shouldWriteLongTo('_var_8c225fe0', $buf + 0x600);
        $this->shouldWriteLong($task + 0x08, 0);
    }

    private function setupExterns(): void
    {
        $this->setSize('_var_selectedVm_8c1ba34c', 4);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->setSize('_var_8c225fe0', 4);
        $this->setSize('_var_8c225fe4', 0x28);
        $this->setSize('_var_8c22600c', 4);
        $this->setSize('_var_8c226010', 4);
        $this->setSize('_var_8c1ba348', 4);
        $this->setSize('_var_8c1ba2e4', 0x58);
        $this->setSize('_var_8c1ba33c', 8);
        $this->setSize('_buIsExistFile', 4);
        $this->setSize('_buStat', 4);
        $this->setSize('_buGetLastError', 4);
        $this->setSize('_buAnalyzeBackupFileImage', 4);
        $this->setSize('_BupLoad_8c014bc6', 4);
        $this->setSize('_TaskFree_8c014b66', 4);
        $this->setSize('_syMalloc', 4);
        $this->setSize('_syFree', 4);
        $this->setSize('_njMemCopy', 4);
    }
};
