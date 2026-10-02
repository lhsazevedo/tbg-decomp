<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_increments_counter_and_forwards_draw_call(): void
    {
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x0c, 4);
        $state = $this->alloc(0x10);

        $this->call('_routeArrowTask_8c029904')->with($task, $state);

        $this->shouldWriteLong($task + 0x0c, 5);
        $this->shouldCall('_RenderQueueDraw2_8c022420')
            ->with(0, $this->addressOf('_drawRouteArrows_8c029878'), $task, $state);
    }

    public function test_counter_wraps_with_plain_addition(): void
    {
        // -1 + 1 == 0: plain integer wraparound, no masking, mirroring the
        // asm's bare ADD #1.
        $task = $this->alloc(0x20);
        $this->initUint32($task + 0x0c, 0xffffffff);
        $state = $this->alloc(0x10);

        $this->call('_routeArrowTask_8c029904')->with($task, $state);

        $this->shouldWriteLong($task + 0x0c, 0);
        $this->shouldCall('_RenderQueueDraw2_8c022420')
            ->with(0, $this->addressOf('_drawRouteArrows_8c029878'), $task, $state);
    }
};
