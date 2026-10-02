<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function taskInstallCheck(int $tasksAddr, int $actionAddr): callable
    {
        return function () use ($tasksAddr, $actionAddr) {
            $r4 = $this->registers[4]->value;
            $r5 = $this->registers[5]->value;
            $allocSize = $this->memory->readUInt32($this->registers[15]->value)->value;
            if ($r4 !== $tasksAddr || $r5 !== $actionAddr || $allocSize !== 0) {
                throw new \Exception(sprintf(
                    "Unexpected TaskInstall args: tasks=0x%x action=0x%x allocSize=%d",
                    $r4, $r5, $allocSize
                ));
            }
        };
    }

    public function test_installs_task_and_resets_scratch_state(): void
    {
        $this->setSize('_var_tasks_8c1ba5e8', 4);
        // var_tachoNeedleVerts_8c226478 follows var_hudState_8c22643c in section B,
        // so assertions reach both off one base.
        $base = $this->addressOf('_var_hudState_8c22643c');

        $this->setSize('_var_runState_8c2285c4', 0x9c);
        $this->initUint32($this->addressOf('_var_runState_8c2285c4') + 0x0c, 42);

        $this->call('_HudInstall_8c02018c');

        $this->shouldCall('_TaskInstall_8c014ae8')->do($this->taskInstallCheck(
            $this->addressOf('_var_tasks_8c1ba5e8'),
            $this->addressOf('_updateTask_8c01ff48'),
        ));

        $this->shouldWriteLong($base + 0x00, 0);
        $this->shouldWriteLong($base + 0x04, 0);
        $this->shouldWriteLong($base + 0x10, 0);
        $this->shouldWriteLong($base + 0x14, -1); // var_hudState_8c22643c.driveMarkIcon_0x14
        $this->shouldWriteLong($base + 0x18, 0); // var_hudState_8c22643c.blinkTimer_0x18
        $this->shouldWriteFloat($base + 0x20, 42.0); // var_hudState_8c22643c.pointsMeter_0x1c.lastSample_0x04
        $this->shouldWriteFloat($base + 0x1c, 42.0); // var_hudState_8c22643c.pointsMeter_0x1c.displayedValue_0x00
        $this->shouldWriteFloat($base + 0x28, 1.0); // var_hudState_8c22643c.pointsMeter_0x1c.field_0x0c
        $this->shouldWriteFloat($base + 0x2c, 0.0); // var_hudState_8c22643c.engineRpm_0x2c
        $this->shouldWriteLong($base + 0x30, 0); // var_hudState_8c22643c.field_0x30
        $this->shouldWriteLong($base + 0x34, 0); // var_hudState_8c22643c.gearLatch_0x34
        $this->shouldWriteLong($base + 0x38, 0); // var_hudState_8c22643c.laneLatch_0x38
        $this->shouldWriteLong($base + 0x48, 0xffff0000); // var_tachoNeedleVerts_8c226478[0].color
        $this->shouldWriteLong($base + 0x58, 0xffff0000); // var_tachoNeedleVerts_8c226478[1].color
        $this->shouldWriteLong($base + 0x68, 0xffff0000); // var_tachoNeedleVerts_8c226478[2].color
    }
};
