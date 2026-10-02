<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

// initEntryState_8c026748 runs one "spawn" (opcode 0) or "place
// parked vehicle" (opcode 10) script instruction for a traffic entry: entry+0x2f8
// is the script's own base pointer, so *entry->0x2f8 is the script's header
// word -- 10 marks a fixed-angle parked vehicle and
// anything else a path-following vehicle. For a vehicle it walks the path
// (entry+0x304 onward, an inline array of per-block path pointers indexed by
// the segment index at entry+0x300; each block is a run of {len,x,y,dx,dy}
// records terminated by a len==0 record) to the entry's starting distance
// (entry+0x2e8), resolves world position/heading from the current record,
// looks up ground height, fills in the vehicle's dimension/animation state
// from its variant table (entry+0x2e0 indexes init_variantDims_8c0460c8/cc/d0/d4, a
// 16-row x4-float table), and spawns its driving task via TrafficLookaheadInit_8c02df3c.

return new class extends TestCase {
    private int $entry;
    private int $scriptIp;
    private int $scriptBuf;
    private int $pathArr;
    private int $sceneParams;
    private int $typeVal;

    private function isAsmObject(): bool {
        return str_contains($this->objectFile, '/asm/');
    }

    private function resolveSymbols(): void {
        $this->setSize('_var_timeOfDay_8c18ad20', 4);
        $this->setSize('_var_sceneParams_8c18ad24', 4);
        $this->setSize('_GroundQueryFindPolygon_8c020914', 4);
        $this->setSize('_GroundProbeInterpolateHeight_8c020f7e', 4);
        $this->setSize('_AsqGetRandomA_8c012166', 4);
        $this->setSize('_TrafficLookaheadInit_8c02df3c', 4);
    }

    private function allocEntry(): void {
        $this->entry = $this->alloc(0x510);

        // Type discriminant: entry+0x2f8 holds a pointer to the script's
        // header word (never 10 in this test -- a vehicle, not a parked one).
        $this->typeVal = $this->alloc(2);
        $this->initUint16($this->typeVal, 5);
        $this->initUint32($this->entry + 0x2f8, $this->typeVal);

        // Script bytecode buffer: scriptIp holds its address; the function
        // reads a ushort 6 bytes in (skip 2-byte header + 4 more) for the
        // initial speed variance.
        $this->scriptBuf = $this->alloc(16);
        $this->initUint16($this->scriptBuf + 6, 0x8000);
        $this->scriptIp = $this->alloc(4);
        $this->initUint32($this->scriptIp, $this->scriptBuf);

        // Single path record with a length longer than the starting
        // distance, so the segment-advance loop never iterates.
        $this->pathArr = $this->alloc(20);
        $this->initUint32($this->pathArr + 0, fdec(10.0));  // len
        $this->initUint32($this->pathArr + 4, fdec(100.0)); // x
        $this->initUint32($this->pathArr + 8, fdec(200.0)); // y
        $this->initUint32($this->pathArr + 12, fdec(0.25)); // dx
        $this->initUint32($this->pathArr + 16, fdec(0.75)); // dy
        $this->initUint32($this->entry + 0x304, $this->pathArr);

        $this->initUint32($this->entry + 0x2e8, fdec(0.0));  // starting distance
        $this->initUint32($this->entry + 0x2ec, fdec(5.0));  // x offset
        $this->initUint32($this->entry + 0x2f0, fdec(7.0));  // y offset
        $this->initUint32($this->entry + 0x2e0, 0);          // variant index
        $this->initUint32($this->entry + 0xf8, fdec(3.5));   // pre-existing height

        // CourseSceneParams: only rec0_0x0c rows 1 and 2 (offsets 0x20, 0x34)
        // are read by this path.
        $this->sceneParams = $this->alloc(0x88);
        $vals1 = [1.1, 2.2, 3.3, 4.4, 5.5];
        foreach ($vals1 as $i => $v) {
            $this->initUint32($this->sceneParams + 0x20 + $i * 4, fdec($v));
        }
        $vals2 = [6.6, 7.7, 8.8, 9.9, 11.0];
        foreach ($vals2 as $i => $v) {
            $this->initUint32($this->sceneParams + 0x34 + $i * 4, fdec($v));
        }
        $this->initUint32($this->addressOf('_var_sceneParams_8c18ad24'), $this->sceneParams);
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0); // TIME_OF_DAY_DAY
    }

    public function test_vehiclePathNoSegmentAdvance(): void {
        $this->resolveSymbols();
        $this->allocEntry();

        $this->call('_initEntryState_8c026748')
            ->with($this->entry, $this->scriptIp);

        // entry+0x300 = 0
        $this->shouldWriteLong($this->entry + 0x300, 0);

        // Unconditional zeroing.
        foreach ([0x408, 0x40c, 0x504, 0x508, 0x50c, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c, 0x80] as $off) {
            $this->shouldWriteLong($this->entry + $off, 0);
        }

        // Default (non-night) scene-params color copy.
        $this->shouldWriteFloat($this->entry + 0xd8, 6.6);
        $this->shouldWriteFloat($this->entry + 0xdc, 7.7);
        $this->shouldWriteFloat($this->entry + 0xe0, 8.8);
        $this->shouldWriteFloat($this->entry + 0xe4, 9.9);
        $this->shouldWriteFloat($this->entry + 0xe8, 11.0);
        $this->shouldWriteFloat($this->entry + 0xc4, 1.1);
        $this->shouldWriteFloat($this->entry + 0xc8, 2.2);
        $this->shouldWriteFloat($this->entry + 0xcc, 3.3);
        $this->shouldWriteFloat($this->entry + 0xd0, 4.4);
        $this->shouldWriteFloat($this->entry + 0xd4, 5.5);

        // Position resolved from the path record (dist == 2.0 exactly, so
        // the (dist - 2.0) multiplier term is zero).
        $this->shouldWriteFloat($this->entry + 0xf4, 105.0);
        $this->shouldWriteFloat($this->entry + 0xfc, 207.0);

        /* both objects now place groundBuf at the same stack slot */
        $groundBuf = 0xffffbc;
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(105.0, 0.0, 207.0, $groundBuf);
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->with($groundBuf, $this->entry + 0xf4);

        // Variant-table dimensions (variant 0).
        $this->shouldWriteFloat($this->entry + 0x23c, 2.42);
        $this->shouldWriteFloat($this->entry + 0x240, 3.0500002);
        $this->shouldWriteFloat($this->entry + 0x244, 1.67);
        $this->shouldWriteFloat($this->entry + 0x248, 0.835);
        $this->shouldWriteFloat($this->entry + 0x24c, 0.65);
        $this->shouldWriteLong($this->entry + 0x258, 0);
        $this->shouldWriteLong($this->entry + 0x268, 0);
        $this->shouldWriteFloat($this->entry + 0x270, 1.0);
        $this->shouldWriteLong($this->entry + 0x26c, 0);

        // Heading basis vectors come from the path record's dx/dy.
        $this->shouldWriteFloat($this->entry + 0x278, 0.75);
        $this->shouldWriteFloat($this->entry + 0x274, 0.25);

        $this->shouldWriteFloat($this->entry + 0x100, 104.395);
        $this->shouldWriteFloat($this->entry + 0x108, 205.185);
        $this->shouldWriteFloat($this->entry + 0x104, 3.5);
        $this->shouldWriteFloat($this->entry + 0x11c, 3.5);
        $this->shouldWriteFloat($this->entry + 0x128, 3.5);

        foreach ([0x198, 0x19c, 0x1a8, 0x1ac, 0x1b8, 0x1bc, 0x1c8, 0x1cc, 0x2b4] as $off) {
            $this->shouldWriteLong($this->entry + $off, 0);
        }

        $this->shouldWriteLong($this->entry + 0x2b8, $this->pathArr);
        $this->shouldWriteFloat($this->entry + 0x2c0, 2.0);
        $this->shouldWriteFloat($this->entry + 0x2bc, 2.0);
        $this->shouldWriteFloat($this->entry + 0x2c4, 2.0);

        foreach ([0x2d4, 0x2d8, 0x2dc] as $off) {
            $this->shouldWriteLong($this->entry + $off, 0);
        }

        // Vehicle tail: speed variance, random offset, task spawn.
        $this->shouldWriteFloat($this->entry + 0x414, 0.5);
        $this->shouldWriteFloat($this->entry + 0x27c, 0.25);
        $this->shouldWriteLong($this->entry + 0x284, 0);
        $this->shouldWriteLong($this->entry + 0x288, 0);
        $this->shouldWriteLong($this->entry + 0x28c, 0);
        $this->shouldWriteFloat($this->entry + 0x290, 0.01);
        $this->shouldWriteFloat($this->entry + 0x418, 9999.0);
        $this->shouldWriteLong($this->entry + 0x424, 0);
        $this->shouldWriteLong($this->entry + 0x428, 0);
        $this->shouldCall('_AsqGetRandomA_8c012166')->andReturn(0x4000);
        $this->shouldWriteFloat($this->entry + 0x41c, 1.25);
        $this->shouldWriteLong($this->entry + 0x42c, 0);
        $this->shouldWriteLong($this->entry + 0x448, 0);
        $this->shouldWriteLong($this->entry + 0x450, 0xffffffff);
        $this->shouldWriteLong($this->entry + 0x458, 0);
        $this->shouldWriteLong($this->entry + 0x498, 0);
        $this->shouldWriteLong($this->entry + 0x468, 0);
        $this->shouldWriteLong($this->entry + 0x474, 0);
        $this->shouldWriteLong($this->scriptIp, $this->scriptBuf + 8);
        $this->shouldWriteFloat($this->entry + 0x490, 9999.0);
        $this->shouldWriteLong($this->entry + 0x4ec, 0);
        $this->shouldWriteLong($this->entry + 0x4f4, $this->pathArr);
        $this->shouldWriteLong($this->entry + 0x4f8, 0);
        $this->shouldWriteFloat($this->entry + 0x4fc, 4.5);
        $this->shouldCall('_TrafficLookaheadInit_8c02df3c')->with($this->entry);
    }

    // Second path-block record has len == 0.0, so the segment-advance loop
    // switches entry+0x304's block index once before settling.
    public function test_vehiclePathAdvancesSegment(): void {
        $this->resolveSymbols();

        $this->entry = $this->alloc(0x510);
        $this->typeVal = $this->alloc(2);
        $this->initUint16($this->typeVal, 5);
        $this->initUint32($this->entry + 0x2f8, $this->typeVal);

        // scriptCursor starts at scriptBuf+2; one block switch adds 6, so the
        // final speed-variance ushort read lands at scriptBuf+12.
        $this->scriptBuf = $this->alloc(20);
        $this->initUint16($this->scriptBuf + 12, 0x8000);
        $this->scriptIp = $this->alloc(4);
        $this->initUint32($this->scriptIp, $this->scriptBuf);

        // Block 0: one real record (len 1.0) followed by a len==0 sentinel.
        $block0 = $this->alloc(40);
        $this->initUint32($block0 + 0, fdec(1.0));  // len
        $this->initUint32($block0 + 4, fdec(0.0));
        $this->initUint32($block0 + 8, fdec(0.0));
        $this->initUint32($block0 + 12, fdec(0.0));
        $this->initUint32($block0 + 16, fdec(0.0));
        $this->initUint32($block0 + 20, fdec(0.0));  // sentinel len == 0
        $this->initUint32($block0 + 24, fdec(0.0));
        $this->initUint32($block0 + 28, fdec(0.0));
        $this->initUint32($block0 + 32, fdec(0.0));
        $this->initUint32($block0 + 36, fdec(0.0));

        // Block 1: len 5.0 > remaining distance, so the loop stops here.
        $block1 = $this->alloc(20);
        $this->initUint32($block1 + 0, fdec(5.0));   // len
        $this->initUint32($block1 + 4, fdec(50.0));  // x
        $this->initUint32($block1 + 8, fdec(60.0));  // y
        $this->initUint32($block1 + 12, fdec(0.1));  // dx
        $this->initUint32($block1 + 16, fdec(0.2));  // dy

        // entry+0x304 onward is itself an inline array of per-block
        // pointers (the same region TrafficResolvePathBlocks_8c026710 fills in
        // from the entry's script), indexed directly by the segment index.
        $this->initUint32($this->entry + 0x304, $block0);
        $this->initUint32($this->entry + 0x308, $block1);

        $this->initUint32($this->entry + 0x2e8, fdec(0.0));  // starting distance -> dist == 2.0
        $this->initUint32($this->entry + 0x2ec, fdec(5.0));
        $this->initUint32($this->entry + 0x2f0, fdec(7.0));
        $this->initUint32($this->entry + 0x2e0, 0);
        $this->initUint32($this->entry + 0xf8, fdec(3.5));

        $this->sceneParams = $this->alloc(0x88);
        foreach ([1.1, 2.2, 3.3, 4.4, 5.5] as $i => $v) {
            $this->initUint32($this->sceneParams + 0x20 + $i * 4, fdec($v));
        }
        foreach ([6.6, 7.7, 8.8, 9.9, 11.0] as $i => $v) {
            $this->initUint32($this->sceneParams + 0x34 + $i * 4, fdec($v));
        }
        $this->initUint32($this->addressOf('_var_sceneParams_8c18ad24'), $this->sceneParams);
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0);

        $this->call('_initEntryState_8c026748')
            ->with($this->entry, $this->scriptIp);

        $this->shouldWriteLong($this->entry + 0x300, 0);

        // Segment switch, mid-loop (happens before the unconditional zeroing
        // that follows the whole path-walk block).
        $this->shouldWriteLong($this->entry + 0x300, 1);

        foreach ([0x408, 0x40c, 0x504, 0x508, 0x50c, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c, 0x80] as $off) {
            $this->shouldWriteLong($this->entry + $off, 0);
        }

        $this->shouldWriteFloat($this->entry + 0xd8, 6.6);
        $this->shouldWriteFloat($this->entry + 0xdc, 7.7);
        $this->shouldWriteFloat($this->entry + 0xe0, 8.8);
        $this->shouldWriteFloat($this->entry + 0xe4, 9.9);
        $this->shouldWriteFloat($this->entry + 0xe8, 11.0);
        $this->shouldWriteFloat($this->entry + 0xc4, 1.1);
        $this->shouldWriteFloat($this->entry + 0xc8, 2.2);
        $this->shouldWriteFloat($this->entry + 0xcc, 3.3);
        $this->shouldWriteFloat($this->entry + 0xd0, 4.4);
        $this->shouldWriteFloat($this->entry + 0xd4, 5.5);

        // dist == 1.0 after crossing into block 1.
        $this->shouldWriteFloat($this->entry + 0xf4, 54.9);
        $this->shouldWriteFloat($this->entry + 0xfc, 66.8);

        /* both objects now place groundBuf at the same stack slot */
        $groundBuf = 0xffffbc;
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(54.9, 0.0, 66.8, $groundBuf);
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->with($groundBuf, $this->entry + 0xf4);

        $this->shouldWriteFloat($this->entry + 0x23c, 2.42);
        $this->shouldWriteFloat($this->entry + 0x240, 3.0500002);
        $this->shouldWriteFloat($this->entry + 0x244, 1.67);
        $this->shouldWriteFloat($this->entry + 0x248, 0.835);
        $this->shouldWriteFloat($this->entry + 0x24c, 0.65);
        $this->shouldWriteLong($this->entry + 0x258, 0);
        $this->shouldWriteLong($this->entry + 0x268, 0);
        $this->shouldWriteFloat($this->entry + 0x270, 1.0);
        $this->shouldWriteLong($this->entry + 0x26c, 0);

        $this->shouldWriteFloat($this->entry + 0x278, 0.2);
        $this->shouldWriteFloat($this->entry + 0x274, 0.1);

        $this->shouldWriteFloat($this->entry + 0x100, 54.658);
        $this->shouldWriteFloat($this->entry + 0x108, 66.316001);
        $this->shouldWriteFloat($this->entry + 0x104, 3.5);
        $this->shouldWriteFloat($this->entry + 0x11c, 3.5);
        $this->shouldWriteFloat($this->entry + 0x128, 3.5);

        foreach ([0x198, 0x19c, 0x1a8, 0x1ac, 0x1b8, 0x1bc, 0x1c8, 0x1cc, 0x2b4] as $off) {
            $this->shouldWriteLong($this->entry + $off, 0);
        }

        $this->shouldWriteLong($this->entry + 0x2b8, $block1);
        $this->shouldWriteFloat($this->entry + 0x2c0, 1.0);
        $this->shouldWriteFloat($this->entry + 0x2bc, 1.0);
        $this->shouldWriteFloat($this->entry + 0x2c4, 2.0);

        foreach ([0x2d4, 0x2d8, 0x2dc] as $off) {
            $this->shouldWriteLong($this->entry + $off, 0);
        }

        $this->shouldWriteFloat($this->entry + 0x414, 0.5);
        $this->shouldWriteFloat($this->entry + 0x27c, 0.25);
        $this->shouldWriteLong($this->entry + 0x284, 0);
        $this->shouldWriteLong($this->entry + 0x288, 0);
        $this->shouldWriteLong($this->entry + 0x28c, 0);
        $this->shouldWriteFloat($this->entry + 0x290, 0.01);
        $this->shouldWriteFloat($this->entry + 0x418, 9999.0);
        $this->shouldWriteLong($this->entry + 0x424, 0);
        $this->shouldWriteLong($this->entry + 0x428, 0);
        $this->shouldCall('_AsqGetRandomA_8c012166')->andReturn(0x4000);
        $this->shouldWriteFloat($this->entry + 0x41c, 1.25);
        $this->shouldWriteLong($this->entry + 0x42c, 0);
        $this->shouldWriteLong($this->entry + 0x448, 0);
        $this->shouldWriteLong($this->entry + 0x450, 0xffffffff);
        $this->shouldWriteLong($this->entry + 0x458, 0);
        $this->shouldWriteLong($this->entry + 0x498, 0);
        $this->shouldWriteLong($this->entry + 0x468, 0);
        $this->shouldWriteLong($this->entry + 0x474, 0);
        $this->shouldWriteLong($this->scriptIp, $this->scriptBuf + 14);
        $this->shouldWriteFloat($this->entry + 0x490, 9999.0);
        $this->shouldWriteLong($this->entry + 0x4ec, 0);
        $this->shouldWriteLong($this->entry + 0x4f4, $block1);
        $this->shouldWriteLong($this->entry + 0x4f8, 1);
        $this->shouldWriteFloat($this->entry + 0x4fc, 3.5);
        $this->shouldCall('_TrafficLookaheadInit_8c02df3c')->with($this->entry);
    }

    // A parked entry (script header word == 10): the path-walk and
    // vehicle tail are skipped entirely, heading comes from njCos/njSin of
    // entry+0x250, and the tail speed-variance/task-spawn block never runs.
    public function test_parkedDefaultColors(): void {
        $this->resolveSymbols();
        $this->setSize('_njCos', 4);
        $this->setSize('_njSin', 4);

        $this->entry = $this->alloc(0x510);
        $this->typeVal = $this->alloc(2);
        $this->initUint16($this->typeVal, 10);
        $this->initUint32($this->entry + 0x2f8, $this->typeVal);

        $this->scriptIp = $this->alloc(4);
        $this->initUint32($this->scriptIp, 0x1234); // never read for a parked entry

        $this->initUint32($this->entry + 0x2e0, 0); // variant index
        $this->initUint32($this->entry + 0x250, 0x2000); // raw angle
        $this->initUint32($this->entry + 0xf4, fdec(10.0)); // pre-set by the caller
        $this->initUint32($this->entry + 0xfc, fdec(20.0));
        $this->initUint32($this->entry + 0xf8, fdec(3.0));

        $this->sceneParams = $this->alloc(0x88);
        foreach ([1.1, 2.2, 3.3, 4.4, 5.5] as $i => $v) {
            $this->initUint32($this->sceneParams + 0x20 + $i * 4, fdec($v));
        }
        foreach ([6.6, 7.7, 8.8, 9.9, 11.0] as $i => $v) {
            $this->initUint32($this->sceneParams + 0x34 + $i * 4, fdec($v));
        }
        $this->initUint32($this->addressOf('_var_sceneParams_8c18ad24'), $this->sceneParams);
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0); // TIME_OF_DAY_DAY

        $this->call('_initEntryState_8c026748')
            ->with($this->entry, $this->scriptIp);

        // The path-walk if(type != 10) block is skipped entirely -- no
        // entry+0x300 write here.
        foreach ([0x408, 0x40c, 0x504, 0x508, 0x50c, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c, 0x80] as $off) {
            $this->shouldWriteLong($this->entry + $off, 0);
        }

        $this->shouldWriteFloat($this->entry + 0xd8, 6.6);
        $this->shouldWriteFloat($this->entry + 0xdc, 7.7);
        $this->shouldWriteFloat($this->entry + 0xe0, 8.8);
        $this->shouldWriteFloat($this->entry + 0xe4, 9.9);
        $this->shouldWriteFloat($this->entry + 0xe8, 11.0);
        $this->shouldWriteFloat($this->entry + 0xc4, 1.1);
        $this->shouldWriteFloat($this->entry + 0xc8, 2.2);
        $this->shouldWriteFloat($this->entry + 0xcc, 3.3);
        $this->shouldWriteFloat($this->entry + 0xd0, 4.4);
        $this->shouldWriteFloat($this->entry + 0xd4, 5.5);

        // entry+0xf4/0xfc are left as the caller set them.
        /* both objects now place groundBuf at the same stack slot */
        $groundBuf = 0xffffbc;
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(10.0, 0.0, 20.0, $groundBuf);
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->with($groundBuf, $this->entry + 0xf4);

        $this->shouldWriteFloat($this->entry + 0x23c, 2.42);
        $this->shouldWriteFloat($this->entry + 0x240, 3.0500002);
        $this->shouldWriteFloat($this->entry + 0x244, 1.67);
        $this->shouldWriteFloat($this->entry + 0x248, 0.835);
        $this->shouldWriteFloat($this->entry + 0x24c, 0.65);
        $this->shouldWriteLong($this->entry + 0x258, 0);
        $this->shouldWriteLong($this->entry + 0x268, 0);
        $this->shouldWriteFloat($this->entry + 0x270, 1.0);
        $this->shouldWriteLong($this->entry + 0x26c, 0);

        $this->shouldCall('_njCos')->with(0x2000)->andReturn(0.5);
        $this->shouldWriteFloat($this->entry + 0x278, 0.5);
        $this->shouldCall('_njSin')->with(0x2000)->andReturn(0.25);
        $this->shouldWriteFloat($this->entry + 0x274, 0.25);

        $this->shouldWriteFloat($this->entry + 0x100, 9.395);
        $this->shouldWriteFloat($this->entry + 0x108, 18.79);
        $this->shouldWriteFloat($this->entry + 0x104, 3.0);
        $this->shouldWriteFloat($this->entry + 0x11c, 3.0);
        $this->shouldWriteFloat($this->entry + 0x128, 3.0);

        foreach ([0x198, 0x19c, 0x1a8, 0x1ac, 0x1b8, 0x1bc, 0x1c8, 0x1cc, 0x2b4] as $off) {
            $this->shouldWriteLong($this->entry + $off, 0);
        }

        // entry+0x2b8 (the segment pointer) is never set on a parked entry's
        // path -- the real asm stores whatever R11 held at function entry
        // (a genuinely uninitialized general-purpose register), and
        // sh4objtest randomizes unset registers, so this write's value is
        // not reproducible. Stop here; the remaining writes (seg/dist
        // leftovers plus the harmless entry+0x27c = 0) carry no further
        // observable behavior for a parked entry.
        $this->forceStop();
    }

    // Night + a parked entry + a hit junction lookup: colors come from the
    // shared night-color globals instead of CourseSceneParams.
    public function test_parkedNightJunctionColors(): void {
        $this->resolveSymbols();
        $this->setSize('_njCos', 4);
        $this->setSize('_njSin', 4);
        $this->setSize('_AttrQueryFindConvexPolygon_8c02e51c', 4);
        $this->setSize('_var_nightLightIntensityOn_8c1bbdb0', 8);
        $this->setSize('_var_nightLightColorOn_8c1bbdd0', 12);

        $this->entry = $this->alloc(0x510);
        $this->typeVal = $this->alloc(2);
        $this->initUint16($this->typeVal, 10);
        $this->initUint32($this->entry + 0x2f8, $this->typeVal);

        $this->scriptIp = $this->alloc(4);
        $this->initUint32($this->scriptIp, 0x1234);

        $this->initUint32($this->entry + 0x2e0, 0);
        $this->initUint32($this->entry + 0x250, 0x2000);
        $this->initUint32($this->entry + 0xf4, fdec(10.0));
        $this->initUint32($this->entry + 0xfc, fdec(20.0));
        $this->initUint32($this->entry + 0xf8, fdec(3.0));

        $this->initUint32($this->addressOf('_var_nightLightIntensityOn_8c1bbdb0') + 0, fdec(0.1));
        $this->initUint32($this->addressOf('_var_nightLightIntensityOn_8c1bbdb0') + 4, fdec(0.2));
        $this->initUint32($this->addressOf('_var_nightLightColorOn_8c1bbdd0') + 0, fdec(0.3));
        $this->initUint32($this->addressOf('_var_nightLightColorOn_8c1bbdd0') + 4, fdec(0.4));
        $this->initUint32($this->addressOf('_var_nightLightColorOn_8c1bbdd0') + 8, fdec(0.5));

        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 2); // TIME_OF_DAY_NIGHT

        $junction = $this->alloc(8);
        $this->initUint32($junction + 4, 1); // hit

        $this->call('_initEntryState_8c026748')
            ->with($this->entry, $this->scriptIp);

        foreach ([0x408, 0x40c, 0x504, 0x508, 0x50c, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c, 0x80] as $off) {
            $this->shouldWriteLong($this->entry + $off, 0);
        }

        $this->shouldCall('_AttrQueryFindConvexPolygon_8c02e51c')->with(10.0, 3.0, 20.0, $this->entry + 0x404)->andReturn($junction);

        $this->shouldWriteFloat($this->entry + 0xc4, 0.1);
        $this->shouldWriteFloat($this->entry + 0xc8, 0.2);
        $this->shouldWriteFloat($this->entry + 0xcc, 0.3);
        $this->shouldWriteFloat($this->entry + 0xd0, 0.4);
        $this->shouldWriteFloat($this->entry + 0xd4, 0.5);

        /* both objects now place groundBuf at the same stack slot */
        $groundBuf = 0xffffbc;
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(10.0, 0.0, 20.0, $groundBuf);
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->with($groundBuf, $this->entry + 0xf4);

        $this->shouldWriteFloat($this->entry + 0x23c, 2.42);
        $this->shouldWriteFloat($this->entry + 0x240, 3.0500002);
        $this->shouldWriteFloat($this->entry + 0x244, 1.67);
        $this->shouldWriteFloat($this->entry + 0x248, 0.835);
        $this->shouldWriteFloat($this->entry + 0x24c, 0.65);
        $this->shouldWriteLong($this->entry + 0x258, 0);
        $this->shouldWriteLong($this->entry + 0x268, 0);
        $this->shouldWriteFloat($this->entry + 0x270, 1.0);
        $this->shouldWriteLong($this->entry + 0x26c, 0);

        $this->shouldCall('_njCos')->with(0x2000)->andReturn(0.5);
        $this->shouldWriteFloat($this->entry + 0x278, 0.5);
        $this->shouldCall('_njSin')->with(0x2000)->andReturn(0.25);
        $this->shouldWriteFloat($this->entry + 0x274, 0.25);

        $this->shouldWriteFloat($this->entry + 0x100, 9.395);
        $this->shouldWriteFloat($this->entry + 0x108, 18.79);
        $this->shouldWriteFloat($this->entry + 0x104, 3.0);
        $this->shouldWriteFloat($this->entry + 0x11c, 3.0);
        $this->shouldWriteFloat($this->entry + 0x128, 3.0);

        foreach ([0x198, 0x19c, 0x1a8, 0x1ac, 0x1b8, 0x1bc, 0x1c8, 0x1cc, 0x2b4] as $off) {
            $this->shouldWriteLong($this->entry + $off, 0);
        }

        // See test_parkedDefaultColors: entry+0x2b8 onward reads a
        // genuinely uninitialized register on a parked entry's path, which
        // sh4objtest randomizes -- stop before it.
        $this->forceStop();
    }

    // Night + parked entry but the junction lookup misses (field_0x04 == 0):
    // must reject the night-color branch and fall back to CourseSceneParams,
    // proving the AND-chain is not short-circuited away entirely.
    public function test_parkedNightJunctionMissFallsBackToSceneParams(): void {
        $this->resolveSymbols();
        $this->setSize('_njCos', 4);
        $this->setSize('_njSin', 4);
        $this->setSize('_AttrQueryFindConvexPolygon_8c02e51c', 4);

        $this->entry = $this->alloc(0x510);
        $this->typeVal = $this->alloc(2);
        $this->initUint16($this->typeVal, 10);
        $this->initUint32($this->entry + 0x2f8, $this->typeVal);

        $this->scriptIp = $this->alloc(4);
        $this->initUint32($this->scriptIp, 0x1234);

        $this->initUint32($this->entry + 0x2e0, 0);
        $this->initUint32($this->entry + 0x250, 0x2000);
        $this->initUint32($this->entry + 0xf4, fdec(10.0));
        $this->initUint32($this->entry + 0xfc, fdec(20.0));
        $this->initUint32($this->entry + 0xf8, fdec(3.0));

        $this->sceneParams = $this->alloc(0x88);
        foreach ([1.1, 2.2, 3.3, 4.4, 5.5] as $i => $v) {
            $this->initUint32($this->sceneParams + 0x20 + $i * 4, fdec($v));
        }
        foreach ([6.6, 7.7, 8.8, 9.9, 11.0] as $i => $v) {
            $this->initUint32($this->sceneParams + 0x34 + $i * 4, fdec($v));
        }
        $this->initUint32($this->addressOf('_var_sceneParams_8c18ad24'), $this->sceneParams);
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 2); // TIME_OF_DAY_NIGHT

        $this->call('_initEntryState_8c026748')
            ->with($this->entry, $this->scriptIp);

        foreach ([0x408, 0x40c, 0x504, 0x508, 0x50c, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c, 0x80] as $off) {
            $this->shouldWriteLong($this->entry + $off, 0);
        }

        // Junction lookup misses -> null result, night-color branch rejected.
        $this->shouldCall('_AttrQueryFindConvexPolygon_8c02e51c')->with(10.0, 3.0, 20.0, $this->entry + 0x404)->andReturn(0);

        $this->shouldWriteFloat($this->entry + 0xd8, 6.6);
        $this->shouldWriteFloat($this->entry + 0xdc, 7.7);
        $this->shouldWriteFloat($this->entry + 0xe0, 8.8);
        $this->shouldWriteFloat($this->entry + 0xe4, 9.9);
        $this->shouldWriteFloat($this->entry + 0xe8, 11.0);
        $this->shouldWriteFloat($this->entry + 0xc4, 1.1);
        $this->shouldWriteFloat($this->entry + 0xc8, 2.2);
        $this->shouldWriteFloat($this->entry + 0xcc, 3.3);
        $this->shouldWriteFloat($this->entry + 0xd0, 4.4);
        $this->shouldWriteFloat($this->entry + 0xd4, 5.5);

        /* both objects now place groundBuf at the same stack slot */
        $groundBuf = 0xffffbc;
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(10.0, 0.0, 20.0, $groundBuf);
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->with($groundBuf, $this->entry + 0xf4);

        $this->shouldWriteFloat($this->entry + 0x23c, 2.42);
        $this->shouldWriteFloat($this->entry + 0x240, 3.0500002);
        $this->shouldWriteFloat($this->entry + 0x244, 1.67);
        $this->shouldWriteFloat($this->entry + 0x248, 0.835);
        $this->shouldWriteFloat($this->entry + 0x24c, 0.65);
        $this->shouldWriteLong($this->entry + 0x258, 0);
        $this->shouldWriteLong($this->entry + 0x268, 0);
        $this->shouldWriteFloat($this->entry + 0x270, 1.0);
        $this->shouldWriteLong($this->entry + 0x26c, 0);

        $this->shouldCall('_njCos')->with(0x2000)->andReturn(0.5);
        $this->shouldWriteFloat($this->entry + 0x278, 0.5);
        $this->shouldCall('_njSin')->with(0x2000)->andReturn(0.25);
        $this->shouldWriteFloat($this->entry + 0x274, 0.25);

        $this->shouldWriteFloat($this->entry + 0x100, 9.395);
        $this->shouldWriteFloat($this->entry + 0x108, 18.79);
        $this->shouldWriteFloat($this->entry + 0x104, 3.0);
        $this->shouldWriteFloat($this->entry + 0x11c, 3.0);
        $this->shouldWriteFloat($this->entry + 0x128, 3.0);

        foreach ([0x198, 0x19c, 0x1a8, 0x1ac, 0x1b8, 0x1bc, 0x1c8, 0x1cc, 0x2b4] as $off) {
            $this->shouldWriteLong($this->entry + $off, 0);
        }

        // See test_parkedDefaultColors: entry+0x2b8 onward reads a
        // genuinely uninitialized register on a parked entry's path, which
        // sh4objtest randomizes -- stop before it.
        $this->forceStop();
    }
};
