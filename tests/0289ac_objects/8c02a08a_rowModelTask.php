<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // State offsets, matching ObjectsInstall_8c02a6ac's test.
    const ST_15 = 0x54;

    public function test_no_match_leaves_counter_at_zero(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x3bc), 0x11000000);

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 0x22000000);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_15, 0);

        $this->call('_rowModelTask_8c02a08a')->with($task, $state);
    }

    public function test_match_advances_counter(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x3bc), 0x11ffffff);

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 0x11000000);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_15, 0);

        $this->call('_rowModelTask_8c02a08a')->with($task, $state);

        $this->shouldWriteLong($state + self::ST_15, 1);
    }

    public function test_counter_one_pushes_draw_calls(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_15, 1);

        $this->call('_rowModelTask_8c02a08a')->with($task, $state);

        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawRowModel_8c02a048'), $state);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(1, $this->addressOf('_drawRowModel_8c02a048'), $state);
    }
};
