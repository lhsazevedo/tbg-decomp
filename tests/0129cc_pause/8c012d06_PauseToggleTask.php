<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_reset_requested_with_queues_idle_returns_to_title()
    {
        $this->setup(resetRequested: 1, unk157a7c: 0, queuesInitialized: 0);

        $this->call('_PauseToggleTask_8c012d06');

        $this->shouldCall('_FUN_8c016182');
        $this->shouldCall('_TitlePushTitle_8c015fd6')->with(1);
    }

    public function test_reset_requested_but_unk157a7c_set_runs_normally()
    {
        $this->setup(resetRequested: 1, unk157a7c: 1, queuesInitialized: 0, pauseActive: 1);

        $this->call('_PauseToggleTask_8c012d06');

        $this->shouldCall('_FUN_8c022910');
    }

    public function test_reset_requested_but_queues_not_idle_runs_normally()
    {
        $this->setup(resetRequested: 1, unk157a7c: 0, queuesInitialized: 1, pauseActive: 1);

        $this->call('_PauseToggleTask_8c012d06');

        $this->shouldCall('_FUN_8c022910');
    }

    public function test_start_pressed_while_unpaused_toggles_paused_and_skips_tasks()
    {
        $this->setup(press: 8, pauseActive: 0);

        $this->call('_PauseToggleTask_8c012d06');

        $this->shouldWriteTo('_var_pauseActive_8c1bb8cc', 1);
        $this->shouldCall('_FUN_8c022910');
    }

    public function test_start_pressed_while_paused_toggles_unpaused_and_runs_tasks()
    {
        $this->setup(press: 8, pauseActive: 1);

        $this->call('_PauseToggleTask_8c012d06');

        $this->shouldWriteTo('_var_pauseActive_8c1bb8cc', 0);
        $this->shouldCall('_FUN_8c02239c');
        $this->shouldCall('_TaskExecGroup_8c014b42')->with($this->addressOf('_var_tasks_8c1ba5e8'));
        $this->shouldCall('_FUN_8c022910');
    }

    public function test_start_not_pressed_while_unpaused_runs_tasks()
    {
        $this->setup(press: 0, pauseActive: 0);

        $this->call('_PauseToggleTask_8c012d06');

        $this->shouldCall('_FUN_8c02239c');
        $this->shouldCall('_TaskExecGroup_8c014b42')->with($this->addressOf('_var_tasks_8c1ba5e8'));
        $this->shouldCall('_FUN_8c022910');
    }

    public function test_start_not_pressed_while_paused_skips_tasks()
    {
        $this->setup(press: 0, pauseActive: 1);

        $this->call('_PauseToggleTask_8c012d06');

        $this->shouldCall('_FUN_8c022910');
    }

    private function setup(
        int $resetRequested = 0,
        int $unk157a7c = 0,
        int $queuesInitialized = 0,
        int $press = 0,
        int $pauseActive = 0,
    ): void {
        $this->setSize('_FUN_8c016182', 4);
        $this->setSize('_TitlePushTitle_8c015fd6', 4);
        $this->setSize('_FUN_8c02239c', 4);
        $this->setSize('_TaskExecGroup_8c014b42', 4);
        $this->setSize('_FUN_8c022910', 4);
        $this->setSize('_var_tasks_8c1ba5e8', 4);

        $periph = $this->alloc(0x34);
        $this->initUint32($periph + 0x10, $press);
        $this->initUint32($this->addressOf('_var_peripheral_8c1ba358'), $periph);

        $this->initUint32($this->addressOf('_var_resetRequested_8c157a78'), $resetRequested);
        $this->initUint32($this->addressOf('_var_8c157a7c'), $unk157a7c);
        $this->initUint32($this->addressOf('_var_queuesAreInitialized_8c157a60'), $queuesInitialized);
        $this->initUint32($this->addressOf('_var_pauseActive_8c1bb8cc'), $pauseActive);
    }
};
