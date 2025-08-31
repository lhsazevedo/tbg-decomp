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

        $text1 = $this->allocString('Hey there!');
        $dialogs1 = $this->alloc(0x8 * 3);
        $this->initUint32($dialogs1 + 0x00, $text1);
        $this->initUint32($dialogs1 + 0x04, 1);

        $text2 = $this->allocString('Hey there!');
        $dialogs2 = $this->alloc(0x8 * 3);
        $this->initUint32($dialogs2 + 0x00, $text2);
        $this->initUint32($dialogs2 + 0x04, 42);

        $this->setSize('_init_8c044c08', 0x8);
        $this->initUint32($this->addressOf('_init_8c044c08') + 0x00, $dialogs1);
        $this->initUint32($this->addressOf('_init_8c044c08') + 0x04, $dialogs2);

        $createdTaskLocal = 0xffffec;
        $createdStateLocal = 0xffffe8;
        $createdTask = $this->alloc(0x20);
        $createdState = $this->alloc(0x1c);

        $this->call('_pushDialogTask_8c0170c6')
            ->with(1, 0xcafe0002);


        $this->shouldCall('_pushTask_8c014ae8')
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
        $this->shouldWriteLong($createdState + 0x04, $dialogs2);
        $this->shouldWriteLong($this->addressOf('_var_8c225fb4'), 1);
    }

    private function resolveSymbols()
    {
        $this->setSize('_var_tasks_8c1ba3c8', 4);
        $this->setSize('_var_8c225fb4', 4);
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
