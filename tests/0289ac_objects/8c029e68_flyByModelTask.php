<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // State offsets (byte), matching drawFlyByModel_8c029e46/rowFlyByTask_8c029e94's tests.
    const ST_CURRENT_FRAME = 0x58;
    const ST_FRAME_LIMIT = 0x5c;

    public function test_frame_not_lapsed_pushes_draw_call(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_CURRENT_FRAME, 0x40000000); // 2.0f
        $this->initUint32($state + self::ST_FRAME_LIMIT, 0x40800000); // 4.0f

        $this->call('_flyByModelTask_8c029e68')->with($task, $state);

        // 2.0f + 1.0f = 3.0f
        $this->shouldWriteLong($state + self::ST_CURRENT_FRAME, 0x40400000);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawFlyByModel_8c029e46'), $state);
    }

    public function test_frame_lapsed_exactly_at_limit_frees_task(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_CURRENT_FRAME, 0x40400000); // 3.0f
        $this->initUint32($state + self::ST_FRAME_LIMIT, 0x40800000); // 4.0f

        $this->call('_flyByModelTask_8c029e68')->with($task, $state);

        // 3.0f + 1.0f == 4.0f exactly: the boundary is inclusive (lapsed).
        $this->shouldWriteLong($state + self::ST_CURRENT_FRAME, 0x40800000);
        $this->shouldCall('_TaskUninstall_8c014b66')->with($task);
    }

    public function test_frame_lapsed_frees_task(): void
    {
        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $this->initUint32($state + self::ST_CURRENT_FRAME, 0x40800000); // 4.0f
        $this->initUint32($state + self::ST_FRAME_LIMIT, 0x40800000); // 4.0f

        $this->call('_flyByModelTask_8c029e68')->with($task, $state);

        // 4.0f + 1.0f = 5.0f
        $this->shouldWriteLong($state + self::ST_CURRENT_FRAME, 0x40a00000);
        $this->shouldCall('_TaskUninstall_8c014b66')->with($task);
    }
};
