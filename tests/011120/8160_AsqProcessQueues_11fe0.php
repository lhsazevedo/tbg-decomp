<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_basic()
    {
        $state = $this->alloc(0x18);
        $stateLocal = $this->isAsmObject() ? 0xffffdc : 0xffffdc;

        $this->shouldCall('_pushTask_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_task_processQueues_8c011e80'),
                0xffffe0,
                $stateLocal,
                0x18
            )
            ->do(function ($params) use ($state) {
                $this->memory->writeUint32($params[3], U32::of($state));
            });

        $this->shouldWrite($state + 0x00, 0);
        $this->shouldWrite($state + 0x08, 0xcafe0002);
        $this->shouldWrite($state + 0x0c, 0xcafe0003);
        $this->shouldWrite($state + 0x10, 0xcafe0004);
        $this->shouldWrite($state + 0x14, 0xcafe0005);
        $this->shouldWrite($state + 0x04, 0xcafe0001);

        $this->shouldCall('_sortAndLoadDatQueue_8c011310');

        $this->singleCall('_AsqProcessQueues_8c011fe0')
            ->with(0xcafe0001, 0xcafe0002, 0xcafe0003, 0xcafe0004, 0xcafe0005)
            ->run();
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
