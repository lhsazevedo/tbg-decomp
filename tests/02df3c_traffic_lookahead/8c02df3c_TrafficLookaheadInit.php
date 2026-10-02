<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// TrafficLookaheadInit_8c02df3c(entry): seeds entry->lookaheadPoints_0x49c
// (0x304 is pathBlocks, 0x49c the cache, 0x4ec the odometer, 0x4f4/0x4fc
// the path cursor/distance, 0x4f8 the pathBlocks index) from a standing
// start, walking the path 5.0 units at a time out to 25.0 units.

return new class extends TestCase {
    private function initFloat(int $addr, float $value): void {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    // A PathRecord run of one long segment followed by a terminator.
    private function makeSeg(float $length, float $x, float $z, float $dirX, float $dirZ): int {
        $seg = $this->alloc(0x14 * 2);
        $this->initFloat($seg + 0x00, $length);
        $this->initFloat($seg + 0x04, $x);
        $this->initFloat($seg + 0x08, $z);
        $this->initFloat($seg + 0x0c, $dirX);
        $this->initFloat($seg + 0x10, $dirZ);
        $this->initFloat($seg + 0x14, 0.0); // terminator (length == 0)
        return $seg;
    }

    private function makeEntry(): int {
        return $this->alloc(0x514);
    }

    // A single segment long enough that it's never exhausted: every point
    // is projected from the same record, and lookaheadCacheLen_0x4ec climbs straight
    // from 0.0 to 25.0 in 5.0 steps, one point per step, then the loop's
    // top-of-loop check exits and writes the sentinel/cursor/distance.
    public function test_singleLongSegment_fillsFivePoints(): void {

        $seg = $this->makeSeg(length: 1000.0, x: 0.0, z: 0.0, dirX: 1.0, dirZ: 0.0);
        $entry = $this->makeEntry();
        $this->initUint32($entry + 0x4f4, $seg);
        $this->initFloat($entry + 0x4fc, 0.0);
        $this->initFloat($entry + 0x4ec, 0.0);

        $out = $entry + 0x49c;

        $this->call('_TrafficLookaheadInit_8c02df3c')->with($entry);

        $this->shouldWriteFloat($out + 0x00, 0.0);
        $this->shouldWriteFloat($out + 0x04, 0.0);
        $this->shouldWriteFloat($entry + 0x4ec, 5.0);

        $this->shouldWriteFloat($out + 0x08, 5.0);
        $this->shouldWriteFloat($out + 0x0c, 0.0);
        $this->shouldWriteFloat($entry + 0x4ec, 10.0);

        $this->shouldWriteFloat($out + 0x10, 10.0);
        $this->shouldWriteFloat($out + 0x14, 0.0);
        $this->shouldWriteFloat($entry + 0x4ec, 15.0);

        $this->shouldWriteFloat($out + 0x18, 15.0);
        $this->shouldWriteFloat($out + 0x1c, 0.0);
        $this->shouldWriteFloat($entry + 0x4ec, 20.0);

        $this->shouldWriteFloat($out + 0x20, 20.0);
        $this->shouldWriteFloat($out + 0x24, 0.0);
        $this->shouldWriteFloat($entry + 0x4ec, 25.0);

        $this->shouldWriteFloat($out + 0x28, 9999.0);
        $this->shouldWriteLong($entry + 0x4f4, $seg);
        $this->shouldWriteFloat($entry + 0x4fc, 25.0);
    }

    // entry->lookaheadCursor_0x4f4 already holds the -1 "path exhausted" sentinel
    // (never resolved), so the very first inner-loop check ends the walk
    // immediately: lookaheadCacheLen_0x4ec advances by 5.0, the cache's only entry is
    // the 9999.0 terminator, and the cursor/distance are stored back
    // unchanged.
    public function test_sentinelCursor_endsImmediately(): void {

        $entry = $this->makeEntry();
        $this->initUint32($entry + 0x4f4, 0xffffffff);
        $this->initFloat($entry + 0x4fc, 3.0);
        $this->initFloat($entry + 0x4ec, 0.0);

        $out = $entry + 0x49c;

        $this->call('_TrafficLookaheadInit_8c02df3c')->with($entry);

        $this->shouldWriteFloat($entry + 0x4ec, 5.0);
        $this->shouldWriteFloat($out + 0x00, 9999.0);
        $this->shouldWriteLong($entry + 0x4f4, 0xffffffff);
        $this->shouldWriteFloat($entry + 0x4fc, 3.0);
    }

    // The current segment (length 5.0) is exhausted by a starting distance
    // of 6.0: the walk subtracts the segment's length, advances past its
    // terminator, bumps lookaheadCursorBlock_0x4f8 (the pathBlocks index) from 0 to 1,
    // and re-resolves the cursor from pathBlocks_0x304[1] before writing
    // its first point.
    public function test_segmentExhausted_advancesViaResolvedArgs(): void {

        $seg0 = $this->makeSeg(length: 5.0, x: 0.0, z: 0.0, dirX: 1.0, dirZ: 0.0);
        $seg1 = $this->makeSeg(length: 1000.0, x: 50.0, z: 0.0, dirX: 1.0, dirZ: 0.0);

        $entry = $this->makeEntry();
        $this->initUint32($entry + 0x4f4, $seg0);
        $this->initFloat($entry + 0x4fc, 6.0);
        $this->initFloat($entry + 0x4ec, 20.0); // one point left to fill (< 25.0)
        $this->initUint32($entry + 0x4f8, 0);
        $this->initUint32($entry + 0x304 + 4, $seg1); // pathBlocks_0x304[1]

        $out = $entry + 0x49c;

        $this->call('_TrafficLookaheadInit_8c02df3c')->with($entry);

        $this->shouldWriteLong($entry + 0x4f8, 1);
        // point projected from seg1 at the remaining distance (6.0 - 5.0 = 1.0):
        // x = 1.0*1.0 + 50.0 = 51.0, z = 1.0*0.0 + 0.0 = 0.0
        $this->shouldWriteFloat($out + 0x00, 51.0);
        $this->shouldWriteFloat($out + 0x04, 0.0);
        $this->shouldWriteFloat($entry + 0x4ec, 25.0);

        $this->shouldWriteFloat($out + 0x08, 9999.0);
        $this->shouldWriteLong($entry + 0x4f4, $seg1);
        $this->shouldWriteFloat($entry + 0x4fc, 6.0);
    }
};
