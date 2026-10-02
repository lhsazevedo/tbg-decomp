<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_draws_at_start_on_step_zero()
    {
        $this->drawsAt(0, 10.0, 20.0);
    }

    public function test_draws_part_way_on_middle_step()
    {
        $this->drawsAt(1, 35.0, 70.0);
    }

    public function test_draws_at_end_on_last_step()
    {
        $this->drawsAt(4, 110.0, 220.0);
    }

    private function drawsAt(int $step, float $x, float $y): void
    {
        $res = $this->alloc(0xc);

        $this->call('_drawSpriteLerp_8c014ff6')
            ->with(10.0, 20.0, 2.5, 110.0, 220.0, 4, $step, $res, 7);

        $this->shouldCall('_SpriteDraw_8c014f54')
            ->with($res, 7, $x, $y, 2.5);
    }
};
