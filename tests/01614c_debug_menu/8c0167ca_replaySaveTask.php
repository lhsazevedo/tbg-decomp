<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_inactive_frees_task_and_returns_to_course_menu(): void
    {
        $this->setupExterns();
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 0);

        $this->call('_replaySaveTask_8c0167ca')->with($task, 0);

        $this->shouldCall('_TaskFree_8c014b66')->with($task);
        $this->shouldCall('_CourseMenuFUN_8c017ef2');
    }

    public function test_state0_not_connected_deactivates(): void
    {
        $this->setupExterns();
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 1);
        $this->initUint32($task + 0x0c, 0);

        $bup = $this->alloc(0x5c);
        $this->initUint32($bup + 0x4c, 0); // Connect == 0

        $this->call('_replaySaveTask_8c0167ca')->with($task, 0);

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bup);
        $this->shouldWriteLong($task + 0x08, 0);
    }

    public function test_state0_connected_not_mounted_mounts_and_goes_state1(): void
    {
        $this->setupExterns();
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 1);
        $this->initUint32($task + 0x0c, 0);

        $bup = $this->alloc(0x5c);
        $this->initUint32($bup + 0x4c, 1); // Connect != 0
        $this->initUint32($bup + 0x50, 0); // Work == NULL -> needs mount

        $this->call('_replaySaveTask_8c0167ca')->with($task, 0);

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bup);
        $this->shouldCall('_BupMount_8c014c00')->with(0);
        $this->shouldWriteLong($task + 0x0c, 1);
    }

    public function test_state0_connected_and_mounted_goes_state2(): void
    {
        $this->setupExterns();
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 1);
        $this->initUint32($task + 0x0c, 0);

        $bup = $this->alloc(0x5c);
        $this->initUint32($bup + 0x4c, 1);       // Connect != 0
        $this->initUint32($bup + 0x50, 0x1234);  // Work != NULL -> already mounted

        $this->call('_replaySaveTask_8c0167ca')->with($task, 0);

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bup);
        $this->shouldWriteLong($task + 0x0c, 2);
    }

    public function test_state1_not_ready_noop(): void
    {
        $this->setupExterns();
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 1);
        $this->initUint32($task + 0x0c, 1);

        $bup = $this->alloc(0x5c);
        $this->initUint32($bup + 0x00, 0); // Ready == 0

        $this->call('_replaySaveTask_8c0167ca')->with($task, 0);

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bup);
    }

    public function test_state1_ready_goes_state2(): void
    {
        $this->setupExterns();
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 1);
        $this->initUint32($task + 0x0c, 1);

        $bup = $this->alloc(0x5c);
        $this->initUint16($bup + 0x00, 1); // Ready != 0

        $this->call('_replaySaveTask_8c0167ca')->with($task, 0);

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bup);
        $this->shouldWriteLong($task + 0x0c, 2);
    }

    public function test_state2_starts_save_and_falls_into_state3(): void
    {
        $this->setupExterns();
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 1);
        $this->initUint32($task + 0x0c, 2);

        $demoBuf = $this->addressOf('_var_demoBuf_8c1ba3c4') + 0x100; // fake buffer address
        $this->initUint32($this->addressOf('_var_demoBuf_8c1ba3c4'), $demoBuf);
        $this->initUint32($this->addressOf('_var_8c228ba4'), 1000);

        $bup = $this->alloc(0x5c);
        $this->initUint32($bup + 0x08, 42);  // ProgressCount
        $this->initUint32($bup + 0x0c, 100); // ProgressMax

        $this->call('_replaySaveTask_8c0167ca')->with($task, 0);

        $this->shouldCall('_BupSave_8c014bcc')->with(0, "BUS_REPLAY", $demoBuf, 2);
        $this->shouldWriteLong($task + 0x0c, 3);
        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bup);
        $this->shouldCall('__divlu');
        $this->shouldCall('_njPrint')->variadic(1)->with(0x40008, "NOW SAVING...(%03d%%)", 42);
        $this->shouldCall('_buStat')->with(0)->andReturn(-1); // still busy
    }

    public function test_state3_still_saving_noop(): void
    {
        $this->setupExterns();
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 1);
        $this->initUint32($task + 0x0c, 3);

        $bup = $this->alloc(0x5c);
        $this->initUint32($bup + 0x08, 10);
        $this->initUint32($bup + 0x0c, 100);

        $this->call('_replaySaveTask_8c0167ca')->with($task, 0);

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bup);
        $this->shouldCall('__divlu');
        $this->shouldCall('_njPrint')->variadic(1)->with(0x40008, "NOW SAVING...(%03d%%)", 10);
        $this->shouldCall('_buStat')->with(0)->andReturn(-1);
    }

    public function test_state3_finished_frees_and_exits(): void
    {
        $this->setupExterns();
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 1);
        $this->initUint32($task + 0x0c, 3);

        $demoBuf = $this->addressOf('_var_demoBuf_8c1ba3c4') + 0x100;
        $this->initUint32($this->addressOf('_var_demoBuf_8c1ba3c4'), $demoBuf);

        $bup = $this->alloc(0x5c);
        $this->initUint32($bup + 0x08, 100);
        $this->initUint32($bup + 0x0c, 100);

        $this->call('_replaySaveTask_8c0167ca')->with($task, 0);

        $this->shouldCall('_BupGetInfo_8c014bba')->with(0)->andReturn($bup);
        $this->shouldCall('__divlu');
        $this->shouldCall('_njPrint')->variadic(1)->with(0x40008, "NOW SAVING...(%03d%%)", 100);
        $this->shouldCall('_buStat')->with(0)->andReturn(0);
        $this->shouldCall('_syFree')->with($demoBuf);
        $this->shouldWriteLongTo('_var_demoBuf_8c1ba3c4', 0xffffffff);
        $this->shouldCall('_BupUnmount_8c014c46')->with(0);
        $this->shouldCall('_TaskFree_8c014b66')->with($task);
        $this->shouldCall('_CourseMenuFUN_8c017ef2');
    }

    private function setupExterns(): void
    {
        $this->setSize('_var_selectedVm_8c1ba34c', 4);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 0);
        $this->setSize('_BupGetInfo_8c014bba', 4);
        $this->setSize('_BupMount_8c014c00', 4);
        $this->setSize('_TaskFree_8c014b66', 4);
        $this->setSize('_CourseMenuFUN_8c017ef2', 4);
        $this->setSize('_var_demoBuf_8c1ba3c4', 4);
        $this->setSize('_var_8c228ba4', 4);
        $this->setSize('_buStat', 4);
        $this->setSize('_syFree', 4);
        $this->setSize('_BupUnmount_8c014c46', 4);
        $this->setSize('_njPrint', 4);
        $this->setSize('__divlu', 4);
        $this->onCall('__divlu', function () {
            $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
        });
    }
};
