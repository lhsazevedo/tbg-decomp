<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // State offsets (byte), matching rowModelTask_8c02a08a/flyByModelTask_8c029e68's tests.
    const ST_15 = 0x54;
    const ST_CURRENT_FRAME = 0x58;
    const ST_FRAME_LIMIT = 0x5c;

    public function test_no_match_leaves_counter_at_zero(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x3bc), 0x11000000);

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 0x22000000);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_15, 0);

        $this->call('_rowMotionModelTask_8c02a120')->with($task, $state);
    }

    public function test_match_advances_counter(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x3bc), 0x11ffffff);

        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x08, 0x11000000);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_15, 0);

        $this->call('_rowMotionModelTask_8c02a120')->with($task, $state);

        $this->shouldWriteLong($state + self::ST_15, 1);
    }

    public function test_frame_not_lapsed_pushes_draw_call(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_15, 1);
        $this->initUint32($state + self::ST_CURRENT_FRAME, 0x40000000); // 2.0f
        $this->initUint32($state + self::ST_FRAME_LIMIT, 0x40800000); // 4.0f

        $this->call('_rowMotionModelTask_8c02a120')->with($task, $state);

        // 2.0f + 1.0f = 3.0f
        $this->shouldWriteLong($state + self::ST_CURRENT_FRAME, 0x40400000);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawRowMotionModel_8c02a0d6'), $state);
    }

    public function test_frame_lapsed_exactly_at_limit_frees_task(): void
    {
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x0c, 0);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_15, 1);
        $this->initUint32($state + self::ST_CURRENT_FRAME, 0x40400000); // 3.0f
        $this->initUint32($state + self::ST_FRAME_LIMIT, 0x40800000); // 4.0f

        $this->call('_rowMotionModelTask_8c02a120')->with($task, $state);

        // 3.0f + 1.0f == 4.0f exactly: not "<" the limit, so lapsed.
        $this->shouldWriteLong($state + self::ST_CURRENT_FRAME, 0x40800000);
        $this->shouldCall('_TaskUninstall_8c014b66')->with($task);
    }

    public function test_frame_lapsed_and_not_looping_frees_task(): void
    {
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x0c, 0);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_15, 1);
        $this->initUint32($state + self::ST_CURRENT_FRAME, 0x40800000); // 4.0f
        $this->initUint32($state + self::ST_FRAME_LIMIT, 0x40800000); // 4.0f

        $this->call('_rowMotionModelTask_8c02a120')->with($task, $state);

        // 4.0f + 1.0f = 5.0f
        $this->shouldWriteLong($state + self::ST_CURRENT_FRAME, 0x40a00000);
        $this->shouldCall('_TaskUninstall_8c014b66')->with($task);
    }

    public function test_frame_lapsed_and_looping_resets_frame(): void
    {
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x0c, 1);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_15, 1);
        $this->initUint32($state + self::ST_CURRENT_FRAME, 0x40800000); // 4.0f
        $this->initUint32($state + self::ST_FRAME_LIMIT, 0x40800000); // 4.0f

        $this->call('_rowMotionModelTask_8c02a120')->with($task, $state);

        // 4.0f + 1.0f = 5.0f
        $this->shouldWriteLong($state + self::ST_CURRENT_FRAME, 0x40a00000);
        $this->shouldWriteLong($state + self::ST_CURRENT_FRAME, 0);
    }
};
