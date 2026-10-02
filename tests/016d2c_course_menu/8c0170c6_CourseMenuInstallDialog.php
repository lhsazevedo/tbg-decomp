<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    public function test_1()
    {
        $this->resolveSymbols();

        $installedTaskLocal = 0xffffec;
        $installedStateLocal = 0xffffe8;
        $installedTask = $this->alloc(0x20);
        $installedState = $this->alloc(0x1c);

        $this->call('_CourseMenuInstallDialog_8c0170c6')
            ->with(65, 0xcafe0002);

        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_instructorDialogTask_8c016f98'),
                $installedTaskLocal,
                $installedStateLocal,
                0x18,
            )
            ->do(function ($params) use ($installedTask, $installedState) {
                $this->memory->writeUInt32($params[2], U32::of($installedTask));
                $this->memory->writeUInt32($params[3], U32::of($installedState));
            });

        $this->shouldWriteLong($installedTask + 0x18, 0xcafe0002);
        $this->shouldWriteLong($installedState + 0x00, 0);
        $this->shouldWriteLong($installedState + 0x04, $this->addressOf('_init_seqFreeRunChooseCourse_8c044bf8'));
        $this->shouldWriteLong($this->addressOf('_var_instructorDialogActive_8c225fb4'), 1);
    }

    private function resolveSymbols()
    {
        $this->setSize('_var_tasks_8c1ba3c8', 4);
    }

    protected function isAsmObject(): bool
    {
        return str_contains($this->objectFile, '/asm/');
    }
};
