<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Dead code -- no caller anywhere in the tree, kept only for object parity
// with the original binary. Scales (to - from) so its length equals
// `step`, using njDistanceP2P for the true distance between the points.
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_njDistanceP2P', 4);
    }

    private function f32(float $value): float
    {
        return unpack('f', pack('f', $value))[1];
    }

    private function initFloat(int $address, float $value): void
    {
        $raw = unpack('L', pack('f', $value))[1];
        $this->initUint32($address, $raw);
    }

    public function test_scalesDeltaToStepLength(): void
    {
        $this->resolveSymbols();

        $from = $this->alloc(12);
        $to = $this->alloc(12);
        $out = $this->alloc(12);

        $this->initFloat($from + 0, 1.0);
        $this->initFloat($from + 4, 2.0);
        $this->initFloat($from + 8, 3.0);

        $this->initFloat($to + 0, 5.0);
        $this->initFloat($to + 4, 2.0);
        $this->initFloat($to + 8, 9.0);

        $this->call('_unusedStepToward_8c020676')->with(2.0, $from, $to, $out);

        $this->shouldCall('_njDistanceP2P')->with($from, $to)->andReturn(10.0);

        $scale = $this->f32(10.0 / 2.0); // dist / step

        $this->shouldWriteFloat($out + 0, $this->f32((5.0 - 1.0) / $scale));
        $this->shouldWriteFloat($out + 4, $this->f32((2.0 - 2.0) / $scale));
        $this->shouldWriteFloat($out + 8, $this->f32((9.0 - 3.0) / $scale));

        $this->shouldReturn(10.0);
    }
};
