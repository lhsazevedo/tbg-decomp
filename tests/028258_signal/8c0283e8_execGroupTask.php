<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_does_nothing_before_the_run_starts()
    {
        $this->setSize('_var_runState_8c2285c4', 0x9c);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4'), 0);

        $this->call('_execGroupTask_8c0283e8')->with(0, 0);
    }

    public function test_pushes_light_fade_and_execs_group_once_driving()
    {
        $this->setSize('_var_runState_8c2285c4', 0x9c);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4'), 1);

        $tasks = $this->addressOf('_var_trafficSignalTasks_8c227e20');
        $this->initUint32($tasks, 0xdeadbeef);

        $this->setSize('_RenderQueueDraw_8c0223ea', 4);
        $this->setSize('_TaskRunGroup_8c014b42', 4);

        $this->call('_execGroupTask_8c0283e8')->with(0, 0);

        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_setLightCallback_8c0283d4'), 0);
        $this->shouldCall('_TaskRunGroup_8c014b42')->with(0xdeadbeef);
    }
};
