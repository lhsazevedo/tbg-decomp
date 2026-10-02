<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _BusLineAdvance_8c02412c(void): advances the bus along its mapped route line by the
 * distance accumulated in busState.lineSegmentRemaining_0x2bc/lineSegmentProgress_0x2c0, switching route
 * segments via the var_lineNodes_8c227d88 node table (indexed by currentLineNodeIdx_0x33c) when the
 * current segment's point list (var_lineSegments_8c227d84[idx].points_0x00, LinePoint[])
 * is exhausted. Writes the resulting waypoint to laneTargetX_0x0ec/laneTargetZ_0x0f0,
 * offsets the bus's actual position from it along the normalized direction
 * by the lane offset laneOffset_0x2c4, and recomputes the secondary heading
 * angle targetHeadingAngle_0x254 (acosf-based, same shape as
 * TrafficAdvanceOnPath_8c026ca2/TrafficUpdateHeading_8c026bc4 in
 * 026710_traffic). Always returns 1.
 *
 * Test values are chosen to be exactly representable in float32 (integers
 * and halves) throughout, since sh4objtest's FR registers carry double
 * precision between instructions and only round to float32 on a memory
 * store or char*-style read -- an inexact intermediate (e.g. 20.0 * 0.6f)
 * would make the asm and C paths drift by an ULP or two before the next
 * store, even though both are "correct".
 */
return new class extends TestCase {
    private function fdec(float $value): int
    {
        return unpack('L', pack('f', $value))[1];
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_var_lineSegments_8c227d84', 4);
        $this->setSize('_var_lineNodes_8c227d88', 4);
        $this->setSize('_njSqrt', 4);
        $this->setSize('_acosf', 4);
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

    // segs[idx]: {LinePoint *points_0x00; float length_0x04;}
    private function allocSegs(int $count): int
    {
        return $this->alloc($count * 8);
    }

    private function setSeg(int $segs, int $idx, int $points): void
    {
        $this->initUint32($segs + $idx * 8, $points);
        $this->initUint32($segs + $idx * 8 + 4, 0); // length_0x04, unread by this function
    }

    // nodes[idx]: {u16 leftLane; u16 rightLane; u16 alt[3]; u16 fallback;}
    private function allocNodes(int $count): int
    {
        return $this->alloc($count * 0xc);
    }

    private function setNodeAlt(int $nodes, int $idx, int $slot, int $value): void
    {
        $this->initUint16($nodes + $idx * 0xc + 4 + $slot * 2, $value);
    }

    private function busState(): int
    {
        return $this->addressOf('_var_busState_8c1bb9d0');
    }

    public function test_found_in_current_segment_no_switch(): void
    {
        $this->resolveSymbols();

        $base = $this->busState();

        $point = $this->allocPoint(100.0, 5.0, 7.0, 0.5, 0.5);
        $this->initUint32($base + 0x2b8, $point); // currentLinePointPtr_0x2b8: current point*
        $this->initUint32($base + 0x2bc, $this->fdec(20.0)); // lineSegmentRemaining_0x2bc: remaining

        $this->initUint32($base + 0x0f4, $this->fdec(20.0)); // posX_0x0f4
        $this->initUint32($base + 0x0fc, $this->fdec(27.0)); // posZ_0x0fc
        $this->initUint32($base + 0x2c4, $this->fdec(4.0)); // laneOffset_0x2c4: lane offset

        $this->call('_BusLineAdvance_8c02412c')->with();

        // point->len_0x00 (100) > remaining (20): found immediately, no
        // segment switch -- currentLinePointPtr_0x2b8 keeps pointing at the same record.
        $this->shouldWriteLong($base + 0x2b8, $point);
        $this->shouldWriteFloat($base + 0x2bc, 20.0);

        // computedX = 5.0 + 20*0.5 = 15.0; computedZ = 7.0 + 20*0.5 = 17.0
        $this->shouldWriteFloat($base + 0x0ec, 15.0);
        $this->shouldWriteFloat($base + 0x0f0, 17.0);

        // dx = 20-15 = 5.0; dz = 27-17 = 10.0; distSq = 25+100 = 125
        $this->shouldCall('_njSqrt')->with(125.0)->andReturn(5.0);

        // dxN = 5.0/5.0 = 1.0; dzN = 10.0/5.0 = 2.0
        // posX = 15.0 + 1.0*4.0 = 19.0; posZ = 17.0 + 2.0*4.0 = 25.0
        $this->shouldWriteFloat($base + 0x0f4, 19.0);
        $this->shouldWriteFloat($base + 0x0fc, 25.0);

        $this->shouldCall('_acosf')->with(2.0)->andReturn(1.0);

        // dxN (1.0) > 0: no sign flip.
        $this->shouldWriteLong($base + 0x254, (int)((1.0 * 65536.0) / 6.283184051513672));
    }

    public function test_negative_dxN_flips_angle_sign(): void
    {
        $this->resolveSymbols();

        $base = $this->busState();

        $point = $this->allocPoint(100.0, 5.0, 7.0, 0.5, 0.5);
        $this->initUint32($base + 0x2b8, $point);
        $this->initUint32($base + 0x2bc, $this->fdec(20.0));

        // posX below computedX (15.0) so dx (and dxN) end up negative.
        $this->initUint32($base + 0x0f4, $this->fdec(10.0)); // posX_0x0f4
        $this->initUint32($base + 0x0fc, $this->fdec(27.0)); // posZ_0x0fc
        $this->initUint32($base + 0x2c4, $this->fdec(4.0));

        $this->call('_BusLineAdvance_8c02412c')->with();

        $this->shouldWriteLong($base + 0x2b8, $point);
        $this->shouldWriteFloat($base + 0x2bc, 20.0);

        $this->shouldWriteFloat($base + 0x0ec, 15.0);
        $this->shouldWriteFloat($base + 0x0f0, 17.0);

        // dx = 10-15 = -5.0; dz = 27-17 = 10.0; distSq = 25+100 = 125
        $this->shouldCall('_njSqrt')->with(125.0)->andReturn(5.0);

        // dxN = -1.0; dzN = 2.0
        $this->shouldWriteFloat($base + 0x0f4, 11.0);  // 15.0 + (-1.0)*4.0
        $this->shouldWriteFloat($base + 0x0fc, 25.0);  // 17.0 + 2.0*4.0

        $this->shouldCall('_acosf')->with(2.0)->andReturn(1.0);

        // dxN (-1.0) <= 0: sign flip.
        $this->shouldWriteLong($base + 0x254, -(int)((1.0 * 65536.0) / 6.283184051513672));
    }

    public function test_segment_exhausted_switches_via_left_signal(): void
    {
        $this->resolveSymbols();

        $base = $this->busState();

        // First segment's point list: a single record whose len (5.0) is
        // <= remaining (20.0), so it's consumed, and the record after it
        // (sentinel len==0.0) forces a segment switch.
        $p0 = $this->allocPoint(5.0, 0.0, 0.0, 0.0, 0.0);
        $this->allocPoint(0.0, 0.0, 0.0, 0.0, 0.0); // sentinel, right after p0

        $this->initUint32($base + 0x2b8, $p0);
        $this->initUint32($base + 0x2bc, $this->fdec(20.0));
        $this->initUint32($base + 0x33c, 7);  // currentLineNodeIdx_0x33c: current node index
        $this->initUint32($base + 0x25c, 1);  // signalSide_0x25c selects nodes[idx].alt[1]

        $nodes = $this->allocNodes(8);
        $this->setNodeAlt($nodes, 7, 1, 3);      // alt[1] -> segment 3
        $this->initUint32($this->addressOf('_var_lineNodes_8c227d88'), $nodes);

        // Segment 3's point list: found immediately (len 100 > remaining).
        $point3 = $this->allocPoint(100.0, 9.0, 11.0, 0.0, 0.0);
        $segs = $this->allocSegs(4);
        $this->setSeg($segs, 3, $point3);
        $this->initUint32($this->addressOf('_var_lineSegments_8c227d84'), $segs);

        $this->initUint32($base + 0x0f4, $this->fdec(9.0)); // posX_0x0f4
        $this->initUint32($base + 0x0fc, $this->fdec(11.0)); // posZ_0x0fc
        $this->initUint32($base + 0x2c4, $this->fdec(0.0)); // no lane offset

        $this->call('_BusLineAdvance_8c02412c')->with();

        // remaining after consuming p0: 20.0 - 5.0 = 15.0, carried as
        // lineSegmentProgress_0x2c0 at the moment of the switch.
        $this->shouldWriteLong($base + 0x33c, 3);
        $this->shouldWriteFloat($base + 0x2c0, 15.0);

        $this->shouldWriteLong($base + 0x2b8, $point3);
        $this->shouldWriteFloat($base + 0x2bc, 15.0);

        // computedX = 9.0 + 15*0.0 = 9.0; computedZ = 11.0 + 15*0.0 = 11.0
        $this->shouldWriteFloat($base + 0x0ec, 9.0);
        $this->shouldWriteFloat($base + 0x0f0, 11.0);

        // dx = 9-9 = 0.0; dz = 11-11 = 0.0
        $this->shouldCall('_njSqrt')->with(0.0)->andReturn(1.0);

        $this->shouldWriteFloat($base + 0x0f4, 9.0);
        $this->shouldWriteFloat($base + 0x0fc, 11.0);

        $this->shouldCall('_acosf')->with(0.0)->andReturn(1.0);

        // dxN (0.0) is not > 0: sign flip applies too.
        $this->shouldWriteLong($base + 0x254, -(int)((1.0 * 65536.0) / 6.283184051513672));
    }

    public function test_segment_exhausted_switches_via_right_signal(): void
    {
        $this->resolveSymbols();

        $base = $this->busState();

        $p0 = $this->allocPoint(5.0, 0.0, 0.0, 0.0, 0.0);
        $this->allocPoint(0.0, 0.0, 0.0, 0.0, 0.0); // sentinel

        $this->initUint32($base + 0x2b8, $p0);
        $this->initUint32($base + 0x2bc, $this->fdec(20.0));
        $this->initUint32($base + 0x33c, 7);
        $this->initUint32($base + 0x25c, 2); // right signal: alt[1] skipped, alt[2] selected

        $nodes = $this->allocNodes(8);
        $this->setNodeAlt($nodes, 7, 1, 0xffff); // alt[1]: no connection
        $this->setNodeAlt($nodes, 7, 2, 5);       // alt[2] -> segment 5
        $this->initUint32($this->addressOf('_var_lineNodes_8c227d88'), $nodes);

        $point5 = $this->allocPoint(100.0, 9.0, 11.0, 0.0, 0.0);
        $segs = $this->allocSegs(6);
        $this->setSeg($segs, 5, $point5);
        $this->initUint32($this->addressOf('_var_lineSegments_8c227d84'), $segs);

        $this->initUint32($base + 0x0f4, $this->fdec(9.0));
        $this->initUint32($base + 0x0fc, $this->fdec(11.0));
        $this->initUint32($base + 0x2c4, $this->fdec(0.0));

        $this->call('_BusLineAdvance_8c02412c')->with();

        $this->shouldWriteLong($base + 0x33c, 5);
        $this->shouldWriteFloat($base + 0x2c0, 15.0);

        $this->shouldWriteLong($base + 0x2b8, $point5);
        $this->shouldWriteFloat($base + 0x2bc, 15.0);

        $this->shouldWriteFloat($base + 0x0ec, 9.0);
        $this->shouldWriteFloat($base + 0x0f0, 11.0);

        $this->shouldCall('_njSqrt')->with(0.0)->andReturn(1.0);

        $this->shouldWriteFloat($base + 0x0f4, 9.0);
        $this->shouldWriteFloat($base + 0x0fc, 11.0);

        $this->shouldCall('_acosf')->with(0.0)->andReturn(1.0);

        $this->shouldWriteLong($base + 0x254, -(int)((1.0 * 65536.0) / 6.283184051513672));
    }

    public function test_segment_exhausted_falls_back_to_alt0(): void
    {
        $this->resolveSymbols();

        $base = $this->busState();

        $p0 = $this->allocPoint(5.0, 0.0, 0.0, 0.0, 0.0);
        $this->allocPoint(0.0, 0.0, 0.0, 0.0, 0.0); // sentinel

        $this->initUint32($base + 0x2b8, $p0);
        $this->initUint32($base + 0x2bc, $this->fdec(20.0));
        $this->initUint32($base + 0x33c, 7);
        $this->initUint32($base + 0x25c, 0); // no signal: alt[1] and alt[2] both skipped

        $nodes = $this->allocNodes(8);
        $this->setNodeAlt($nodes, 7, 0, 2);      // alt[0] (default) -> segment 2
        $this->setNodeAlt($nodes, 7, 1, 0xffff);
        $this->setNodeAlt($nodes, 7, 2, 0xffff);
        $this->initUint32($this->addressOf('_var_lineNodes_8c227d88'), $nodes);

        $point2 = $this->allocPoint(100.0, 9.0, 11.0, 0.0, 0.0);
        $segs = $this->allocSegs(3);
        $this->setSeg($segs, 2, $point2);
        $this->initUint32($this->addressOf('_var_lineSegments_8c227d84'), $segs);

        $this->initUint32($base + 0x0f4, $this->fdec(9.0));
        $this->initUint32($base + 0x0fc, $this->fdec(11.0));
        $this->initUint32($base + 0x2c4, $this->fdec(0.0));

        $this->call('_BusLineAdvance_8c02412c')->with();

        $this->shouldWriteLong($base + 0x33c, 2);
        $this->shouldWriteFloat($base + 0x2c0, 15.0);

        $this->shouldWriteLong($base + 0x2b8, $point2);
        $this->shouldWriteFloat($base + 0x2bc, 15.0);

        $this->shouldWriteFloat($base + 0x0ec, 9.0);
        $this->shouldWriteFloat($base + 0x0f0, 11.0);

        $this->shouldCall('_njSqrt')->with(0.0)->andReturn(1.0);

        $this->shouldWriteFloat($base + 0x0f4, 9.0);
        $this->shouldWriteFloat($base + 0x0fc, 11.0);

        $this->shouldCall('_acosf')->with(0.0)->andReturn(1.0);

        $this->shouldWriteLong($base + 0x254, -(int)((1.0 * 65536.0) / 6.283184051513672));
    }
};
