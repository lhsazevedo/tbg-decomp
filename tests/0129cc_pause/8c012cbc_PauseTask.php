<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_reset_requested_with_queues_idle_returns_to_title()
    {
        $this->setup(resetRequested: 1, unk157a7c: 0, queuesInitialized: 0);

        $this->call('_PauseTask_8c012cbc');

        $this->shouldCall('_FUN_8c016182');
        $this->shouldCall('_TitlePushTitle_8c015fd6')->with(1);
    }

    public function test_reset_requested_but_unk157a7c_set_runs_menu_instead()
    {
        $this->setup(resetRequested: 1, unk157a7c: 1, queuesInitialized: 0);

        $this->call('_PauseTask_8c012cbc');

        $this->shouldCall('_update_8c0129cc')->andReturn(0);
    }

    public function test_reset_requested_but_queues_not_idle_runs_menu_instead()
    {
        $this->setup(resetRequested: 1, unk157a7c: 0, queuesInitialized: 1);

        $this->call('_PauseTask_8c012cbc');

        $this->shouldCall('_update_8c0129cc')->andReturn(0);
    }

    public function test_menu_still_closed_does_nothing_further()
    {
        $this->setup();

        $this->call('_PauseTask_8c012cbc');

        $this->shouldCall('_update_8c0129cc')->andReturn(0);
    }

    public function test_menu_just_opened_resets_render_list_and_runs_tasks()
    {
        $this->setup();

        $this->call('_PauseTask_8c012cbc');

        $this->shouldCall('_update_8c0129cc')->andReturn(1);
        $this->shouldCall('_FUN_8c02239c');
        $this->shouldCall('_TaskExecGroup_8c014b42')->with($this->addressOf('_var_tasks_8c1ba5e8'));
        $this->shouldCall('_FUN_8c022560');
    }

    private function setup(
        int $resetRequested = 0,
        int $unk157a7c = 0,
        int $queuesInitialized = 0,
    ): void {
        $this->setSize('_FUN_8c016182', 4);
        $this->setSize('_TitlePushTitle_8c015fd6', 4);
        $this->setSize('_FUN_8c02239c', 4);
        $this->setSize('_TaskExecGroup_8c014b42', 4);
        $this->setSize('_FUN_8c022560', 4);
        $this->setSize('_var_tasks_8c1ba5e8', 4);

        $this->initUint32($this->addressOf('_var_resetRequested_8c157a78'), $resetRequested);
        $this->initUint32($this->addressOf('_var_8c157a7c'), $unk157a7c);
        $this->initUint32($this->addressOf('_var_queuesAreInitialized_8c157a60'), $queuesInitialized);
    }
};
