<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;

return new class extends TestCase {
    // param 0: install the peripheral-support task and clear its auto-fire state.
    public function test_installs_peripheral_support_task()
    {
        $this->resolveSymbols();

        $this->call('_InputPushTask_8c0128cc')->with(0);

        $this->shouldCall('_TaskPush_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            $this->addressOf('_PspTask_8c012324'),
            $this->addressOf('_var_8c157a74'),
            new WildcardArgument, // create_state out-param (stack local)
            0
        );
        $this->shouldWriteLongTo('_var_8c157ae4', 0);
        $this->shouldWriteLongTo('_var_8c157ae8', 0);
        $this->shouldWriteLongTo('_var_8c157ad4', 0);
    }

    // param 1, var_inputMapSel_8c1bb8c8 == 0: queue inputTask_8c012504.
    public function test_installs_inputTask_8c012504_when_flag_clear()
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_inputMapSel_8c1bb8c8'), 0);

        $this->call('_InputPushTask_8c0128cc')->with(1);

        $this->shouldCall('_TaskPush_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            $this->addressOf('_inputTask_8c012504'),
            $this->addressOf('_var_8c157a74'),
            new WildcardArgument,
            0
        );
    }

    // param 1, var_inputMapSel_8c1bb8c8 != 0: queue inputTaskAlt_8c012718.
    public function test_installs_inputTaskAlt_8c012718_when_flag_set()
    {
        $this->resolveSymbols();
        $this->initUint32($this->addressOf('_var_inputMapSel_8c1bb8c8'), 1);

        $this->call('_InputPushTask_8c0128cc')->with(1);

        $this->shouldCall('_TaskPush_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            $this->addressOf('_inputTaskAlt_8c012718'),
            $this->addressOf('_var_8c157a74'),
            new WildcardArgument,
            0
        );
    }

    // Any other param falls through with no task queued and no state cleared.
    public function test_other_param_does_nothing()
    {
        $this->resolveSymbols();
        // Prologue loads this before the branch, so relocation must resolve.
        $this->setSize('_TaskPush_8c014ae8', 4);

        $this->call('_InputPushTask_8c0128cc')->with(2);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_tasks_8c1ba3c8', 4);
        $this->setSize('_var_8c157a74', 4);
        $this->setSize('_var_8c157ae4', 4);
        $this->setSize('_var_8c157ae8', 4);
        $this->setSize('_var_8c157ad4', 0x10);
        $this->setSize('_var_inputMapSel_8c1bb8c8', 4);
        $this->setSize('_PspTask_8c012324', 4);
    }
};
