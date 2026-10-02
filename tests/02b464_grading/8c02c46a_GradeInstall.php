<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_resets_state_and_installs_task(): void
    {
        $base = $this->addressOf('_var_runState_8c2285c4');

        $this->setSize('_var_tasks_8c1ba5e8', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_practiceRules_8c226410', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x400);
        $this->setSize('_var_driveMsgQueue_8c228564', 0x60);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 0);
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x358, 0);
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x34c, 0);
        $this->initUint32($this->addressOf('_var_busState_8c1bb9d0') + 0x368, 0);

        $this->call('_GradeInstall_8c02c46a');

        // GradeInstall_8c02c46a has an empty stack frame of its own
        // beyond the two TaskInstall out-params (installed_task, installed_state),
        // pushed right after the STS.L PR/ADD #-8,R15 prologue -- so their
        // addresses are the initial test stack pointer (16MiB-4, see
        // sh4objtest's Run) minus 12 and 8 respectively. Same layout in
        // both objects.
        $sp0 = 1024 * 1024 * 16 - 4;
        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba5e8'),
                $this->addressOf('_runPhaseTask_8c02c072'),
                $sp0 - 12, // &installed_task
                $sp0 - 8, // &installed_state
                0
            )
            ->andReturn(1);

        $this->shouldWriteLong($base + 0x00, 0); // var_runState_8c2285c4.runPhase_0x00
        $this->shouldWriteLong($base + 0x04, 0); // var_runState_8c2285c4.runPassed_0x04
        $this->shouldWriteLong($base + 0x20, 1); // var_runState_8c2285c4.stopPhase_0x20 (playMode != 1 -> 1)

        $this->shouldWriteLong($base + 0x2c, 0); // var_runState_8c2285c4.wrongLaneCount_0x2c
        $this->shouldWriteLong($base + 0x30, 0); // var_runState_8c2285c4.speedingCountdown_0x30
        $this->shouldWriteLong($base + 0x34, 0); // var_runState_8c2285c4.stopLineGraded_0x34

        $this->shouldWriteLong($base + 0x38, 0); // var_runState_8c2285c4.field_0x38[0]
        $this->shouldWriteLong($base + 0x40, 0); // var_runState_8c2285c4.field_0x38[2]
        $this->shouldWriteLong($base + 0x44, 0); // var_runState_8c2285c4.field_0x38[3]
        $this->shouldWriteLong($base + 0x48, 0); // var_runState_8c2285c4.field_0x38[4]
        $this->shouldWriteLong($base + 0x4c, 0); // var_runState_8c2285c4.field_0x38[5]
        $this->shouldWriteLong($base + 0x50, 0); // var_runState_8c2285c4.field_0x38[6]
        $this->shouldWriteLong($base + 0x54, -1); // var_runState_8c2285c4.field_0x38[7]

        $this->shouldWriteLong($base + 0x5c, 0); // var_runState_8c2285c4.hasMoved_0x5c
        $this->shouldWriteLong($base + 0x60, 0); // var_runState_8c2285c4.startIdleFrames_0x60
        $this->shouldWriteLong($base + 0x64, 1); // var_runState_8c2285c4.crossingState_0x64
        $this->shouldWriteLong($base + 0x68, 0); // var_runState_8c2285c4.crossingStopTimer_0x68
        $this->shouldWriteLong($base + 0x6c, 0); // var_runState_8c2285c4.lane_0x6c
        $this->shouldWriteLong($base + 0x70, 0); // var_runState_8c2285c4.field_0x70[0]
        $this->shouldWriteLong($base + 0x74, 0); // var_runState_8c2285c4.field_0x70[1]
        $this->shouldWriteLong($base + 0x78, 0); // var_runState_8c2285c4.field_0x70[2]

        $this->shouldWriteLong($base + 0x7c, 0); // var_runState_8c2285c4.instructionBonusPending_0x7c
        $this->shouldWriteLong($base + 0x88, 0); // var_runState_8c2285c4.firstUpshift_0x88
        $this->shouldWriteFloat($base + 0x90, 0.0); // var_runState_8c2285c4.brakeAverage_0x90
        $this->shouldWriteLong($base + 0x94, 0); // var_runState_8c2285c4.hardBrakeCooldown_0x94
        $this->shouldWriteLong($base + 0x98, 0); // var_runState_8c2285c4.swerveCountdown_0x98

        $this->shouldWriteLong($this->addressOf('_var_cooldownCollision_8c228690'), 0);
        $this->shouldWriteLong($this->addressOf('_var_cooldownOffCourse_8c228694'), 0);
        $this->shouldWriteLong($this->addressOf('_var_cooldownSignal_8c228698'), 0);
        $this->shouldWriteLong($this->addressOf('_var_cooldownLane_8c22869c'), 0);
        $this->shouldWriteLong($this->addressOf('_var_cooldownIntersection_8c2286a0'), 0);

        $this->shouldWriteLong($this->addressOf('_var_driveMsgQueue_8c228564') + 0x14, 0);
        $this->shouldWriteLong($this->addressOf('_var_driveMsgQueue_8c228564') + 0x2c, 0);
        $this->shouldWriteLong($this->addressOf('_var_driveMsgQueue_8c228564') + 0x44, 0);
        $this->shouldWriteLong($this->addressOf('_var_driveMsgQueue_8c228564') + 0x5c, 0);
    }
};
