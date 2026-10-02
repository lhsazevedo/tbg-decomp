<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// TrafficPathScanBuild_8c02f0c8(self, entry, firstScriptArg, flag, startProgress, window):
// walks startProgress units along the path records starting at
// firstScriptArg (no bounds check on this first walk -- a real asm quirk),
// then samples (x, z) positions every 5.0 units across the next `window`
// units into var_samples_8c228b48 (cursor/bound var_sampleCursor_8c228b9c/var_sampleEnd_8c228ba0). Returns
// the first sample's occupant: the player's bus sentinel (var_playerBus_8c1bbd9c) if
// within 2.5 of either of its two tracked points, else the first live,
// non-self task within 2.5 -- or NULL.
//
// Both real call sites pass startProgress = window = 8.0, so every test
// here samples at path-progress 8.0 and 13.0 (two points, 5.0 apart).

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_busState_8c1bb9d0', 0x400);
        $this->setSize('_var_playerBus_8c1bbd9c', 4);
        $this->setSize('_var_tasks_8c1bac28', 4 * 0x20);
    }

    private function initFloat(int $addr, float $value): void {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    // A PathRecord run of one long segment (never exhausted by the walk)
    // followed by a terminator, so the initial "walk to startProgress" loop
    // is a no-op and every sample is projected from the same segment:
    // pos = (remaining*dirX + x, remaining*dirZ + z).
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

    // entry with pathBlocks_0x304[0] = seg
    private function makeEntry(int $seg): int {
        $entry = $this->alloc(0x308);
        $this->initUint32($entry + 0x304, $seg);
        return $entry;
    }

    private function makeTask(int $index, int $action, int $state): int {
        $task = $this->addressOf('_var_tasks_8c1bac28') + $index * 0x20;
        $this->initUint32($task + 0x00, $action);
        $this->initUint32($task + 0x04, $state);
        return $task;
    }

    private function farBusState(): void {
        $this->initFloat($this->addressOf('_var_busState_8c1bb9d0') + 0xf4, -9999.0);
        $this->initFloat($this->addressOf('_var_busState_8c1bb9d0') + 0x100, -9999.0);
        $this->initFloat(($this->addressOf('_var_busState_8c1bb9d0') + 0x0fc), -9999.0);
        $this->initFloat(($this->addressOf('_var_busState_8c1bb9d0') + 0x108), -9999.0);
    }

    // No task in the way and the bus is far off: both samples are written
    // (progress 8.0 and 13.0), var_sampleEnd_8c228ba0 is set to the end of the
    // collected pairs, and the function returns NULL without ever setting
    // var_sampleCursor_8c228b9c.
    public function test_noMatch_returnsNull(): void {
        $this->resolveSymbols();

        $seg = $this->makeSeg(length: 1000.0, x: 0.0, z: 0.0, dirX: 1.0, dirZ: 0.0);
        $entry = $this->makeEntry($seg);
        $self = $this->alloc(4);
        $this->farBusState();
        $this->makeTask(0, 0, 0); // terminator

        $base = $this->addressOf('_var_samples_8c228b48');

        $this->call('_TrafficPathScanBuild_8c02f0c8')->with($self, $entry, $seg, 0, 8.0, 8.0);

        $this->shouldWriteFloat($base + 0x00, 8.0);
        $this->shouldWriteFloat($base + 0x04, 0.0);
        $this->shouldWriteFloat($base + 0x08, 13.0);
        $this->shouldWriteFloat($base + 0x0c, 0.0);
        $this->shouldWriteLongTo('_var_sampleEnd_8c228ba0', $base + 0x10);
        $this->shouldReturn(0);
    }

    // The first sample (8.0, 0.0) lands within 2.5 of the bus's primary
    // point (posX_0x0f4 / var_8c1bbacc): returns var_playerBus_8c1bbd9c and sets the
    // cursor to just past that first pair.
    public function test_matchesBusPrimaryPoint_returnsBusSentinel(): void {
        $this->resolveSymbols();

        $seg = $this->makeSeg(length: 1000.0, x: 0.0, z: 0.0, dirX: 1.0, dirZ: 0.0);
        $entry = $this->makeEntry($seg);
        $self = $this->alloc(4);
        $this->makeTask(0, 0, 0);

        $this->initFloat($this->addressOf('_var_busState_8c1bb9d0') + 0xf4, 8.0);
        $this->initFloat(($this->addressOf('_var_busState_8c1bb9d0') + 0x0fc), 0.0);
        $this->initFloat($this->addressOf('_var_busState_8c1bb9d0') + 0x100, -9999.0);
        $this->initFloat(($this->addressOf('_var_busState_8c1bb9d0') + 0x108), -9999.0);

        $bus = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_playerBus_8c1bbd9c'), $bus);

        $base = $this->addressOf('_var_samples_8c228b48');

        $this->call('_TrafficPathScanBuild_8c02f0c8')->with($self, $entry, $seg, 0, 8.0, 8.0);

        $this->shouldWriteFloat($base + 0x00, 8.0);
        $this->shouldWriteFloat($base + 0x04, 0.0);
        $this->shouldWriteFloat($base + 0x08, 13.0);
        $this->shouldWriteFloat($base + 0x0c, 0.0);
        $this->shouldWriteLongTo('_var_sampleEnd_8c228ba0', $base + 0x10);
        $this->shouldWriteLongTo('_var_sampleCursor_8c228b9c', $base + 0x08);
        $this->shouldReturn($bus);
    }

    // The second sample (13.0, 0.0) lands within 2.5 of the bus's secondary
    // point (frontPointX_0x100 / var_8c1bbad8) -- the first sample doesn't match
    // either bus point or any task.
    public function test_matchesBusSecondaryPoint_returnsBusSentinel(): void {
        $this->resolveSymbols();

        $seg = $this->makeSeg(length: 1000.0, x: 0.0, z: 0.0, dirX: 1.0, dirZ: 0.0);
        $entry = $this->makeEntry($seg);
        $self = $this->alloc(4);
        $this->makeTask(0, 0, 0);

        $this->initFloat($this->addressOf('_var_busState_8c1bb9d0') + 0xf4, -9999.0);
        $this->initFloat(($this->addressOf('_var_busState_8c1bb9d0') + 0x0fc), -9999.0);
        $this->initFloat($this->addressOf('_var_busState_8c1bb9d0') + 0x100, 13.0);
        $this->initFloat(($this->addressOf('_var_busState_8c1bb9d0') + 0x108), 0.0);

        $bus = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_playerBus_8c1bbd9c'), $bus);

        $base = $this->addressOf('_var_samples_8c228b48');

        $this->call('_TrafficPathScanBuild_8c02f0c8')->with($self, $entry, $seg, 0, 8.0, 8.0);

        $this->shouldWriteFloat($base + 0x00, 8.0);
        $this->shouldWriteFloat($base + 0x04, 0.0);
        $this->shouldWriteFloat($base + 0x08, 13.0);
        $this->shouldWriteFloat($base + 0x0c, 0.0);
        $this->shouldWriteLongTo('_var_sampleEnd_8c228ba0', $base + 0x10);
        $this->shouldWriteLongTo('_var_sampleCursor_8c228b9c', $base + 0x10);
        $this->shouldReturn($bus);
    }

    // A live task (not self, not the -1 sentinel) whose entry sits within
    // 2.5 of the first sample: returns that task's state pointer.
    public function test_matchesTask_returnsItsState(): void {
        $this->resolveSymbols();

        $seg = $this->makeSeg(length: 1000.0, x: 0.0, z: 0.0, dirX: 1.0, dirZ: 0.0);
        $entry = $this->makeEntry($seg);
        $self = $this->alloc(4);
        $this->farBusState();

        $candidateEntry = $this->alloc(0x100);
        $this->initFloat($candidateEntry + 0xf4, 8.0);
        $this->initFloat($candidateEntry + 0xfc, 0.0);

        $this->makeTask(0, -1, 0xdeadbeef); // sentinel, skipped
        $selfSlot = $this->makeTask(1, 1, 0xdeadbeef); // self, skipped
        $this->makeTask(2, 1, $candidateEntry);
        $this->makeTask(3, 0, 0);

        $base = $this->addressOf('_var_samples_8c228b48');

        $this->call('_TrafficPathScanBuild_8c02f0c8')->with($selfSlot, $entry, $seg, 0, 8.0, 8.0);

        $this->shouldWriteFloat($base + 0x00, 8.0);
        $this->shouldWriteFloat($base + 0x04, 0.0);
        $this->shouldWriteFloat($base + 0x08, 13.0);
        $this->shouldWriteFloat($base + 0x0c, 0.0);
        $this->shouldWriteLongTo('_var_sampleEnd_8c228ba0', $base + 0x10);
        $this->shouldWriteLongTo('_var_sampleCursor_8c228b9c', $base + 0x08);
        $this->shouldReturn($candidateEntry);
    }

    // window == 0.0: the sampling loop's condition (traveled < startProgress
    // + window) is false on entry, so no sample is ever written and the
    // function returns NULL without touching var_sampleEnd_8c228ba0/var_sampleCursor_8c228b9c at
    // all.
    public function test_zeroWindow_collectsNothing_returnsNull(): void {
        $this->resolveSymbols();

        $seg = $this->makeSeg(length: 1000.0, x: 0.0, z: 0.0, dirX: 1.0, dirZ: 0.0);
        $entry = $this->makeEntry($seg);
        $self = $this->alloc(4);

        $this->call('_TrafficPathScanBuild_8c02f0c8')->with($self, $entry, $seg, 0, 8.0, 0.0);

        $this->shouldReturn(0);
    }

    // The path runs out (pathBlocks_0x304[1] == -1) partway through the
    // sampling loop, after the first sample was already collected: the
    // second sample is never written, and the scan proceeds over just that
    // one collected pair.
    //
    // seg0's length (10.0) is chosen so the *first* walk-to-startProgress
    // loop is a no-op (10.0 > startProgress 8.0 -- its own condition never
    // fires, so it never risks dereferencing the -1 sentinel, matching the
    // real asm's unchecked first walk), while the *second* (sampling) walk
    // exhausts it on its second pass (remaining reaches 13.0 > 10.0),
    // hitting the terminator and then pathBlocks_0x304[1] == -1.
    public function test_pathExhaustedMidLoop_scansOnlyCollectedSample(): void {
        $this->resolveSymbols();

        $seg0 = $this->makeSeg(length: 10.0, x: 0.0, z: 0.0, dirX: 1.0, dirZ: 0.0);
        $entry = $this->makeEntry($seg0);
        $this->initUint32($entry + 0x304 + 4, -1); // pathBlocks_0x304[1]

        $self = $this->alloc(4);
        $this->farBusState();
        $this->makeTask(0, 0, 0);

        $base = $this->addressOf('_var_samples_8c228b48');

        $this->call('_TrafficPathScanBuild_8c02f0c8')->with($self, $entry, $seg0, 0, 8.0, 8.0);

        $this->shouldWriteFloat($base + 0x00, 8.0);
        $this->shouldWriteFloat($base + 0x04, 0.0);
        $this->shouldWriteLongTo('_var_sampleEnd_8c228ba0', $base + 0x08);
        $this->shouldReturn(0);
    }
};
