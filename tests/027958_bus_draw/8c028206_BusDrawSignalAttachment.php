<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /** Allocates a TrafficSignal-sized buffer with drawA_0xc8 wired to
     * $drawA and lamps_0x10[0] (pre-seeded to $initialFlags)/tlist_0xb4/
     * model_0xb8 wired to fresh scratch targets. */
    private function makeObj(int $drawA, int $initialFlags): array
    {
        $obj = $this->alloc(0xd4);
        $this->doNotRandomizeMemory();

        $this->initUint32($obj + 0xc8, $drawA);

        $frame0 = $this->alloc(4); // evalflags is the first field
        $this->initUint32($frame0, $initialFlags);
        $this->initUint32($obj + 0x10, $frame0);

        $tlist = $this->alloc(4);
        $model = $this->alloc(4);
        $this->initUint32($obj + 0xb4, $tlist);
        $this->initUint32($obj + 0xb8, $model);

        return [$obj, $frame0, $tlist, $model];
    }

    private function assertDraw(int $matrix, int $tlist, int $model): void
    {
        $this->shouldCall('_njMultiMatrix')->with(0, $matrix);
        $this->shouldCall('_njSetTexture')->with($tlist);
        $this->shouldCall('_njCnkSimpleDrawObject')->with($model);
    }

    public function test_drawA_zero_sets_hide_bit(): void
    {
        [$obj, $frame0, $tlist, $model] = $this->makeObj(0, 0x37); // bit 3 clear
        $matrix = $this->alloc(0x40);

        $this->call('_BusDrawSignalAttachment_8c028206')->with($obj, $matrix);

        $this->shouldWriteLong($frame0, 0x3f); // 0x37 | 8
        $this->assertDraw($matrix, $tlist, $model);
    }

    public function test_drawA_nonzero_clears_hide_bit(): void
    {
        [$obj, $frame0, $tlist, $model] = $this->makeObj(1, 0x3f); // bit 3 set
        $matrix = $this->alloc(0x40);

        $this->call('_BusDrawSignalAttachment_8c028206')->with($obj, $matrix);

        $this->shouldWriteLong($frame0, 0x37); // 0x3f & ~8
        $this->assertDraw($matrix, $tlist, $model);
    }
};
