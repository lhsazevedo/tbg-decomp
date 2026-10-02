<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _BusDriveFindLaneTarget_8c023e7e(void): walks the current route line segment forward by the
 * bus's per-frame move distance to find the next lane-crossing point, then
 * confirms it against the crossing segment via IntersectSegments_8c0206f0.
 * Advances through var_lineNodes_8c227d88's linked segment records and indexes
 * var_lineSegments_8c227d84 for each segment's point list. Called once per frame by
 * BusTask_8c022bdc (022bdc).
 */
return new class extends TestCase {
    private function fdec(float $value): int
    {
        return unpack('L', pack('f', $value))[1];
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3c8);
        $this->setSize('_var_lineSegments_8c227d84', 4);
        $this->setSize('_var_lineNodes_8c227d88', 4);
        $this->setSize('_var_crossingIntersectPoint_8c1bc458', 4);
        $this->setSize('_var_crossingIntersectPointZ_8c1bc45c', 4);
        $this->setSize('_IntersectSegments_8c0206f0', 4);
        $this->setSize('_VecXZCross_8c0207fa', 4);
        $this->setSize('_GeomDistanceXZ_8c02081c', 4);
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_var_midiHandles_8c0fcd28', 0x20);
    }

    // segs[idx]: {LinePoint *points_0x00; float length_0x04;}
    private function allocSegs(int $count): int
    {
        return $this->alloc($count * 8);
    }

    private function setSeg(int $segs, int $idx, int $points, float $length): void
    {
        $this->initUint32($segs + $idx * 8, $points);
        $this->initUint32($segs + $idx * 8 + 4, $this->fdec($length));
    }

    // nodes[idx]: {u16 leftLane; u16 rightLane; u16 pad[3]; u16 fallbackNext;}
    private function allocNodes(int $count): int
    {
        return $this->alloc($count * 0xc);
    }

    private function setNodeFwd(int $nodes, int $idx, int $value): void
    {
        $this->initUint16($nodes + $idx * 0xc, $value);
    }

    private function setNodeFallback(int $nodes, int $idx, int $value): void
    {
        $this->initUint16($nodes + $idx * 0xc + 0xa, $value);
    }

    // VecXZCross_8c0207fa's 3rd arg is the candidate point on our own stack, whose
    // address we can't predict -- verify params 1/2 and read back param 3's
    // pointee instead of asserting a literal address.
    private function shouldCallFun0207fa(int $base, float $candX, float $candZ, float $return): void
    {
        $this->shouldCall('_VecXZCross_8c0207fa')->do(function () use ($base, $candX, $candZ, $return) {
            if ($this->registers[4]->value !== $base + 0xf4) {
                throw new RuntimeException(sprintf(
                    'VecXZCross_8c0207fa param1: expected %08x, got %08x', $base + 0xf4, $this->registers[4]->value,
                ));
            }
            if ($this->registers[5]->value !== $base + 0xec) {
                throw new RuntimeException(sprintf(
                    'VecXZCross_8c0207fa param2: expected %08x, got %08x', $base + 0xec, $this->registers[5]->value,
                ));
            }
            $candAddr = $this->registers[6]->value;
            $x = unpack('f', pack('L', $this->memory->readUInt32($candAddr)->value))[1];
            $z = unpack('f', pack('L', $this->memory->readUInt32($candAddr + 4)->value))[1];
            if ($x !== $candX || $z !== $candZ) {
                throw new RuntimeException(sprintf(
                    'VecXZCross_8c0207fa param3: expected (%f, %f), got (%f, %f)', $candX, $candZ, $x, $z,
                ));
            }
            $this->setFloatRegister(0, $return);
        });
    }

    // A LinePoint: {float len; float x; float z; float dx; float dz;}
    private function allocPoint(float $len, float $x, float $z, float $dx, float $dz): int
    {
        $p = $this->alloc(0x14);
        $this->initUint32($p + 0x00, $this->fdec($len));
        $this->initUint32($p + 0x04, $this->fdec($x));
        $this->initUint32($p + 0x08, $this->fdec($z));
        $this->initUint32($p + 0x0c, $this->fdec($dx));
        $this->initUint32($p + 0x10, $this->fdec($dz));
        return $p;
    }

    public function test_ends_replay_when_arrival_confirmed(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');

        $this->initUint32($base + 0x334, 1);
        $this->initUint32($base + 0x2c4, $this->fdec(2.0));

        $this->call('_BusDriveFindLaneTarget_8c023e7e')->with();

        $this->shouldWriteLong($base + 0x25c, 0);
        $this->shouldWriteLong($base + 0x268, 0); // mirror_0x268
        $this->shouldWriteLong($base + 0x334, 0);
    }

    public function test_pending_arrival_not_yet_confirmed_is_a_no_op(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');

        $this->initUint32($base + 0x334, 1);
        $this->initUint32($base + 0x2c4, $this->fdec(1.0)); // not 2.0

        $this->call('_BusDriveFindLaneTarget_8c023e7e')->with();

        // No writes and no calls at all.
    }

    public function test_already_flagged_off_route_is_a_no_op(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');

        $this->initUint32($base + 0x334, 0);
        $this->initUint32($base + 0x338, 2);

        $this->call('_BusDriveFindLaneTarget_8c023e7e')->with();
    }

    public function test_no_forward_segment_flags_off_route(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $nodes = $this->allocNodes(4);

        $this->initUint32($base + 0x334, 0);
        $this->initUint32($base + 0x338, 0); // forward mode
        $this->initUint32($base + 0x33c, 2); // current segment index
        $this->initUint32($this->addressOf('_var_lineNodes_8c227d88'), $nodes);
        $this->setNodeFwd($nodes, 2, 0xffff);

        $this->call('_BusDriveFindLaneTarget_8c023e7e')->with();

        $this->shouldWriteLong($base + 0x338, 2);
    }

    public function test_reaches_end_of_walked_segment_before_any_crossing(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $nodes = $this->allocNodes(8);
        $segs = $this->allocSegs(8);

        // point[0].len (100.0) exceeds remaining (50.0), so the forward walk
        // doesn't advance -- segEnd stays equal to seg on entry to the loop.
        $point = $this->allocPoint(100.0, 10.0, 20.0, 1.0, 0.0);
        $this->setSeg($segs, 7, $point, 50.0);
        $this->setSeg($segs, 5, 0, 25.0); // current segment's length (denominator)

        $this->initUint32($base + 0x334, 0);
        $this->initUint32($base + 0x338, 0); // forward mode
        $this->initUint32($base + 0x33c, 5); // current segment index
        $this->initUint32($base + 0x27c, $this->fdec(0.0)); // speed_0x27c
        $this->initUint32($base + 0x2c0, $this->fdec(25.0)); // lineSegmentProgress_0x2c0
        $this->initUint32($this->addressOf('_var_lineNodes_8c227d88'), $nodes);
        $this->initUint32($this->addressOf('_var_lineSegments_8c227d84'), $segs);
        $this->setNodeFwd($nodes, 5, 7);

        $this->call('_BusDriveFindLaneTarget_8c023e7e')->with();

        // remaining = 0*128 + segs[7].length(50) * lineSegmentProgress_0x2c0(25) / segs[5].length(25) = 50.0
        // cand = remaining*dx + x, remaining*dz + z = (60.0, 20.0)
        $this->shouldCallFun0207fa($base, 60.0, 20.0, -5.0); // side <= 0, not killed in forward mode
        $this->shouldWriteFloat($base + 0xec, 60.0); // laneTargetX_0x0ec = cand.x
        $this->shouldWriteFloat($base + 0xf0, 20.0); // laneTargetZ_0x0f0 = cand.z
        $this->shouldCall('_GeomDistanceXZ_8c02081c')->do(function () use ($base) {
            if ($this->registers[4]->value !== $base + 0xf4) {
                throw new RuntimeException('GeomDistanceXZ_8c02081c param1: unexpected address');
            }
            $candAddr = $this->registers[5]->value;
            $x = unpack('f', pack('L', $this->memory->readUInt32($candAddr)->value))[1];
            $z = unpack('f', pack('L', $this->memory->readUInt32($candAddr + 4)->value))[1];
            if ($x !== 60.0 || $z !== 20.0) {
                throw new RuntimeException('GeomDistanceXZ_8c02081c param2: unexpected candidate point');
            }
            $this->setFloatRegister(0, 99.0);
        });
        $this->shouldWriteFloat($base + 0x2c4, 99.0);
        $this->shouldWriteLong($base + 0x334, 1);
        $this->shouldWriteLong($base + 0x33c, 7);
        $this->shouldWriteFloat($base + 0x2bc, 50.0); // lineSegmentRemaining_0x2bc = remaining
        $this->shouldWriteFloat($base + 0x2c0, 50.0); // lineSegmentProgress_0x2c0 = traveled
        $this->shouldWriteLong($base + 0x2b8, $point); // currentLinePointPtr_0x2b8 = segEnd
    }

    public function test_side_check_kills_track_before_any_crossing(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $nodes = $this->allocNodes(8);
        $segs = $this->allocSegs(8);

        $point = $this->allocPoint(100.0, 10.0, 20.0, 1.0, 0.0);
        $this->setSeg($segs, 7, $point, 50.0);
        $this->setSeg($segs, 5, 0, 25.0);

        $this->initUint32($base + 0x334, 0);
        $this->initUint32($base + 0x338, 0); // forward mode: killed when side > 0
        $this->initUint32($base + 0x33c, 5);
        $this->initUint32($base + 0x27c, $this->fdec(0.0));
        $this->initUint32($base + 0x2c0, $this->fdec(25.0));
        $this->initUint32($this->addressOf('_var_lineNodes_8c227d88'), $nodes);
        $this->initUint32($this->addressOf('_var_lineSegments_8c227d84'), $segs);
        $this->setNodeFwd($nodes, 5, 7);

        $this->call('_BusDriveFindLaneTarget_8c023e7e')->with();

        $this->shouldCallFun0207fa($base, 60.0, 20.0, 5.0); // side > 0 -- kills the track in forward mode

        $this->shouldWriteLong($base + 0x338, 2);
    }
};
