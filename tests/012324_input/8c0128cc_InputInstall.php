<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // param 0: install the menu input task and clear its stick and repeat state.
    public function test_installs_menu_input_task()
    {
        $this->resolveSymbols();

        $this->call('_InputInstall_8c0128cc')->with(0);

        // InputInstall_8c0128cc's frame holds a single local (installed_state),
        // pushed just above TaskInstall's 0 stack arg; its address is the initial
        // test stack pointer (16MiB-4, see sh4objtest's Run) minus 16. Same
        // layout in both objects, and int param goes in R4 so it adds no
        // stack args of its own.
        $sp0 = 1024 * 1024 * 16 - 4;
        $this->shouldCall('_TaskInstall_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            $this->addressOf('_inputMenuTask_8c012324'),
            $this->addressOf('_var_pushedTask_8c157a74'),
            $sp0 - 16, // &installed_state (stack local; InputInstall_8c0128cc's only local, sole word pushed just above TaskInstall's stack arg)
            0
        );
        $this->shouldWriteLongTo('_var_stickLatchX_8c157ae4', 0);
        $this->shouldWriteLongTo('_var_stickLatchY_8c157ae8', 0);
        $this->shouldWriteLongTo('_var_keyRepeat_8c157ad4', 0);
    }

    public function test_installs_the_manual_input_task()
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 0);

        $this->call('_InputInstall_8c0128cc')->with(1);

        // See test_installs_menu_input_task for the &installed_state
        // derivation -- same frame layout regardless of branch taken.
        $sp0 = 1024 * 1024 * 16 - 4;
        $this->shouldCall('_TaskInstall_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            $this->addressOf('_inputManualTask_8c012504'),
            $this->addressOf('_var_pushedTask_8c157a74'),
            $sp0 - 16, // &installed_state
            0
        );
    }

    public function test_installs_the_auto_input_task()
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_driveMode_8c1bb8c8'), 1);

        $this->call('_InputInstall_8c0128cc')->with(1);

        // See test_installs_menu_input_task for the &installed_state
        // derivation -- same frame layout regardless of branch taken.
        $sp0 = 1024 * 1024 * 16 - 4;
        $this->shouldCall('_TaskInstall_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            $this->addressOf('_inputAutoTask_8c012718'),
            $this->addressOf('_var_pushedTask_8c157a74'),
            $sp0 - 16, // &installed_state
            0
        );
    }

    // Any other param falls through with no task queued and no state cleared.
    public function test_other_param_does_nothing()
    {
        $this->resolveSymbols();
        // Prologue loads this before the branch, so relocation must resolve.
        $this->setSize('_TaskInstall_8c014ae8', 4);

        $this->call('_InputInstall_8c0128cc')->with(2);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_tasks_8c1ba3c8', 4);
        $this->setSize('_var_pushedTask_8c157a74', 4);
        $this->setSize('_var_driveMode_8c1bb8c8', 4);
    }
};
