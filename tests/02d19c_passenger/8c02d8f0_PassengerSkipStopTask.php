<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_TaskUninstall_8c014b66', 4);
        $this->setSize('_BusCameraApplyMode_8c024f32', 4);
        $this->setSize('_var_isFading_8c226568', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_var_cameraMode_8c227d9c', 4);
        $this->setSize('_var_cameraCueState_8c227da4', 4);
        $this->setSize('_var_viewLayout_8c22655c', 4);
    }

    public function test_not_fading_resets_state_and_frees_task(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);

        $task = $this->alloc(0x20);
        $state = $this->alloc(4);

        $this->call('_PassengerSkipStopTask_8c02d8f0')->with($task, $state);

        $this->shouldWriteLong($this->addressOf('_var_busState_8c1bb9d0') + 0x2b4, 1);
        $this->shouldWriteLongTo('_var_viewLayout_8c22655c', 0);
        $this->shouldWriteLongTo('_var_cameraMode_8c227d9c', 2);
        $this->shouldWriteLongTo('_var_cameraCueState_8c227da4', 0);
        $this->shouldCall('_BusCameraApplyMode_8c024f32');
        $this->shouldCall('_TaskUninstall_8c014b66')->with($task);
    }

    public function test_fading_does_nothing(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $task = $this->alloc(0x20);
        $state = $this->alloc(4);

        $this->call('_PassengerSkipStopTask_8c02d8f0')->with($task, $state);
    }
};
