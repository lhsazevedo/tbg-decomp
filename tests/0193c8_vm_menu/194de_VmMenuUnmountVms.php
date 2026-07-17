<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_unmountsConnectedVms()
    {
        $this->resolveSymbols();

        $createdTask = 0xfffff0;
        $createdState = 0xfffff4;

        $task = 0xbebacafe;

        $this->shouldCall('_TaskPush_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            $this->addressOf('_taskUnmountVms_8c01946a'),
            $createdTask,
            $createdState,
            0,
        );
        $this->shouldWriteLongTo('_var_8c22606c', 1);

        $this->singleCall('_VmMenuUnmountVms_8c0194de')->with($task)->run();
    }

    private function resolveSymbols(): void
    {
        // Functions
        $this->setSize('_BupGetInfo_8c014bba', 0x4);
        $this->setSize('_BupUnmount_8c014c46', 0x4);
    }

    private function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
