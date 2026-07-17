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

        $createdTaskLocal = 0xffffec;
        $createdStateLocal = 0xffffe8;
        $createdTask = $this->alloc(0x20);
        $createdState = $this->alloc(0x1c);

        $this->call('_CourseMenuPushDialogTask_8c0170c6')
            ->with(65, 0xcafe0002);

        $this->shouldCall('_TaskPush_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_dialogSequenceTask_8c016f98'),
                $createdTaskLocal,
                $createdStateLocal,
                0x18,
            )
            ->do(function ($params) use ($createdTask, $createdState) {
                $this->memory->writeUInt32($params[2], U32::of($createdTask));
                $this->memory->writeUInt32($params[3], U32::of($createdState));
            });

        $this->shouldWriteLong($createdTask + 0x18, 0xcafe0002);
        $this->shouldWriteLong($createdState + 0x00, 0);
        $this->shouldWriteLong($createdState + 0x04, $this->addressOf('_init_seqFreeRunChooseCourse_8c044bf8'));
        $this->shouldWriteLong($this->addressOf('_var_dialogSequenceIsActive_8c225fb4'), 1);
    }

    private function resolveSymbols()
    {
        $this->setSize('_var_tasks_8c1ba3c8', 4);
        $this->setSize('_var_dialogSequenceIsActive_8c225fb4', 4);
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
