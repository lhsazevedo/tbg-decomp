<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /** Allocates a TrafficSignal-sized buffer with frame_0x0c wired to
     * $frame and lamps_0x10[0..2]/tlist_0xb4/model_0xb8 wired to fresh
     * scratch targets. */
    private function makeObj(int $frame): array
    {
        $obj = $this->alloc(0xd4);
        $this->doNotRandomizeMemory();

        $this->initUint32($obj + 0x0c, $frame);

        $frames = [];
        foreach ([0x10, 0x14, 0x18] as $i => $off) {
            $t = $this->alloc(4); // evalflags is the first field
            $this->initUint32($obj + $off, $t);
            $frames[$i] = $t;
        }

        $tlist = $this->alloc(4);
        $model = $this->alloc(4);
        $this->initUint32($obj + 0xb4, $tlist);
        $this->initUint32($obj + 0xb8, $model);

        return [$obj, $frames, $tlist, $model];
    }

    private function assertDraw(int $matrix, int $tlist, int $model): void
    {
        $this->shouldCall('_njMultiMatrix')->with(0, $matrix);
        $this->shouldCall('_njSetTexture')->with($tlist);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($model);
    }

    public function test_frame_1(): void
    {
        [$obj, $frames, $tlist, $model] = $this->makeObj(1);
        $matrix = $this->alloc(0x40);

        $this->call('_BusDrawSignal_8c0281ac')->with($obj, $matrix);

        $this->shouldWriteLong($frames[0], 0x37);
        $this->shouldWriteLong($frames[1], 0x3f);
        $this->shouldWriteLong($frames[2], 0x3f);
        $this->assertDraw($matrix, $tlist, $model);
    }

    public function test_frame_2(): void
    {
        [$obj, $frames, $tlist, $model] = $this->makeObj(2);
        $matrix = $this->alloc(0x40);

        $this->call('_BusDrawSignal_8c0281ac')->with($obj, $matrix);

        $this->shouldWriteLong($frames[0], 0x3f);
        $this->shouldWriteLong($frames[1], 0x37);
        $this->shouldWriteLong($frames[2], 0x3f);
        $this->assertDraw($matrix, $tlist, $model);
    }

    public function test_frame_0(): void
    {
        [$obj, $frames, $tlist, $model] = $this->makeObj(0);
        $matrix = $this->alloc(0x40);

        $this->call('_BusDrawSignal_8c0281ac')->with($obj, $matrix);

        $this->shouldWriteLong($frames[0], 0x3f);
        $this->shouldWriteLong($frames[1], 0x3f);
        $this->shouldWriteLong($frames[2], 0x37);
        $this->assertDraw($matrix, $tlist, $model);
    }

    public function test_frame_default_no_frame_writes(): void
    {
        [$obj, $frames, $tlist, $model] = $this->makeObj(5);
        $matrix = $this->alloc(0x40);

        $this->call('_BusDrawSignal_8c0281ac')->with($obj, $matrix);

        $this->assertDraw($matrix, $tlist, $model);
    }
};
