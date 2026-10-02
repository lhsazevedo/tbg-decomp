<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_does_nothing_when_both_groups_already_freed()
    {
        $this->setSize('_TaskUninstallGroup_8c014ab4', 4);
        $this->setSize('_syFree', 4);

        $this->initUint32($this->addressOf('_var_trafficSignalTasks_8c227e20'), 0xffffffff);
        $this->initUint32($this->addressOf('_var_trafficSignalFrames_8c227e24'), 0xffffffff);

        $this->call('_SignalFree_8c0288be');
    }

    public function test_frees_task_group_when_allocated()
    {
        $this->setSize('_TaskUninstallGroup_8c014ab4', 4);
        $this->setSize('_syFree', 4);

        $tasks = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_trafficSignalTasks_8c227e20'), $tasks);
        $this->initUint32($this->addressOf('_var_trafficSignalFrames_8c227e24'), 0xffffffff);

        $this->call('_SignalFree_8c0288be');

        $this->shouldCall('_TaskUninstallGroup_8c014ab4')->with($tasks);
        $this->shouldCall('_syFree')->with($tasks);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalTasks_8c227e20'), 0xffffffff);
    }

    public function test_frees_object_tables_when_allocated()
    {
        $this->setSize('_TaskUninstallGroup_8c014ab4', 4);
        $this->setSize('_syFree', 4);

        $slots = $this->alloc(4);
        $table = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_trafficSignalTasks_8c227e20'), 0xffffffff);
        $this->initUint32($this->addressOf('_var_trafficSignalFrames_8c227e24'), $slots);
        $this->initUint32($this->addressOf('_var_trafficSignalStates_8c227e28'), $table);

        $this->call('_SignalFree_8c0288be');

        $this->shouldCall('_syFree')->with($slots);
        $this->shouldCall('_syFree')->with($table);
        $this->shouldWriteLong($this->addressOf('_var_trafficSignalFrames_8c227e24'), 0xffffffff);
    }
};
