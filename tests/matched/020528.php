<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function testOnlyRunsWhen8c1bb8d0IsNot2() {
        $this->shouldReadSymbolOffset('_var_playMode_8c1bb8d0', 0, 2);

        $this->singleCall('_FUN_8c020528')
            ->run();
    }

    public function testA() {
        $this->shouldReadSymbolOffset('_var_playMode_8c1bb8d0', 0, 1);

        $tasksPtr = $this->alloc(8 * 4 * 16);
        $this->rellocate('_var_tasks_8c1ba5e8', $tasksPtr);

        $actionPtr = $this->alloc(4);
        $this->rellocate('_FUN_8c020214', $actionPtr);

        // TODO: Implement a way to expect local vars (stack) as parameters
        $this->shouldCall('_TaskPush_8c014ae8')
            ->with($tasksPtr, $actionPtr, 0xFFFFEB, 0xFFFFEF, 0);

        $structPtr = $this->alloc(0x1c);
        $this->rellocate('_var_8c2264b8', $structPtr);

        $this->shouldWrite($structPtr, 0);

        $this->shouldCall('_AsqGetRandomInRangeB_8c0121be')->with(300)->andReturn(0xbebacafe);
        $this->shouldWrite($structPtr + 0x04, 0xbebacb94);
        $this->shouldWrite($structPtr + 0x08, 3);
        $this->shouldWrite($structPtr + 0x0c, 1);
        $this->shouldWrite($structPtr + 0x14, 0);
        $this->shouldWrite($structPtr + 0x18, 0);

        $this->singleCall('_FUN_8c020528')
            ->run();
    }
};
