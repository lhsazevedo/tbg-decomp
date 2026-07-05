<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function initHeap(int $heap, int $tail): void
    {
        $this->initUint32($this->addressOf('_var_heapHead_8c157a58'), $heap);
        $this->initUint32($this->addressOf('_var_heapTail_8c157a5c'), $tail);

        // Head chunk: free, points to tail
        $this->initUint32($heap + 0x00, $heap);
        $this->initUint32($heap + 0x04, $tail);
        $this->initUint32($heap + 0x08, ($tail - $heap) / 0x20);
        $this->initUint32($heap + 0x0c, 0);

        // Tail chunk: sentinel
        $this->initUint32($tail + 0x00, $heap);
        $this->initUint32($tail + 0x04, 0);
        $this->initUint32($tail + 0x08, 0);
        $this->initUint32($tail + 0x0c, 0);
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }

    // remainder->units_0x08 = ((int)oldNext - (int)remainder) / sizeof(HeapChunk).
    // The asm object (matches the original binary) calls __divls; the C
    // recompile folds the constant power-of-two division into shifts.
    public function test_alloc_splits_head_chunk(): void
    {
        $this->setSize('__divls', 4);

        if ($this->isAsmObject()) {
            $this->onCall('__divls', function () {
                $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
            });
        }

        $heap = $this->alloc(0x100);
        $tail = $heap + 0xe0;
        $this->initHeap($heap, $tail);

        $this->call('_heapAlloc_8c01102a')->with(0x20);

        $remainder = $heap + 0x40;

        $this->shouldWriteLong($heap + 0x08, 2);
        $this->shouldWriteLong($heap + 0x04, $remainder);
        $this->shouldWriteLong($heap + 0x0c, 1);

        $this->shouldWriteLong($remainder + 0x00, $heap);
        $this->shouldWriteLong($remainder + 0x04, $tail);
        if ($this->isAsmObject()) {
            $this->shouldCall('__divls');
        }
        $this->shouldWriteLong($remainder + 0x08, 5);
        $this->shouldWriteLong($remainder + 0x0c, 0);

        $this->shouldWriteLong($tail + 0x00, $remainder);

        $this->shouldReturn($heap + 0x20);
    }

    public function test_oom_calls_hook(): void
    {
        $this->setSize('__divls', 4);

        $heap = $this->alloc(0x40);
        $tail = $heap + 0x20;

        $this->initUint32($this->addressOf('_var_heapHead_8c157a58'), $heap);
        $this->initUint32($this->addressOf('_var_heapTail_8c157a5c'), $tail);

        // Head chunk: used, advances to tail
        $this->initUint32($heap + 0x00, $heap);
        $this->initUint32($heap + 0x04, $tail);
        $this->initUint32($heap + 0x08, 1);
        $this->initUint32($heap + 0x0c, 1);

        // Tail sentinel
        $this->initUint32($tail + 0x00, $heap);
        $this->initUint32($tail + 0x04, 0);
        $this->initUint32($tail + 0x08, 0);
        $this->initUint32($tail + 0x0c, 0);

        $this->call('_heapAlloc_8c01102a')->with(0x20);

        $this->shouldCall('_heapAllocOutOfMemoryHook_8c01102a');
        $this->forceStop();
    }

    // Exact fit: neededUnits == chunk->units, so remainder == oldNext and no split runs.
    public function test_alloc_exact_fit_no_remainder(): void
    {
        $this->setSize('__divls', 4);

        $heap = $this->alloc(0x100);
        $tail = $heap + 0xe0;
        $this->initHeap($heap, $tail);

        $this->call('_heapAlloc_8c01102a')->with(0xc0);

        $this->shouldWriteLong($heap + 0x08, 7);
        $this->shouldWriteLong($heap + 0x04, $tail);
        $this->shouldWriteLong($heap + 0x0c, 1);

        $this->shouldReturn($heap + 0x20);
    }

    // Walk skips a used chunk and allocates from the next free one.
    public function test_alloc_skips_used_chunk(): void
    {
        $this->setSize('__divls', 4);

        if ($this->isAsmObject()) {
            $this->onCall('__divls', function () {
                $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
            });
        }

        $heap = $this->alloc(0x100);
        $chunk1 = $heap + 0x40;
        $tail = $heap + 0xe0;

        $this->initUint32($this->addressOf('_var_heapHead_8c157a58'), $heap);
        $this->initUint32($this->addressOf('_var_heapTail_8c157a5c'), $tail);

        // Chunk 0: used, 2 units
        $this->initUint32($heap + 0x00, $heap);
        $this->initUint32($heap + 0x04, $chunk1);
        $this->initUint32($heap + 0x08, 2);
        $this->initUint32($heap + 0x0c, 1);

        // Chunk 1: free, 5 units
        $this->initUint32($chunk1 + 0x00, $heap);
        $this->initUint32($chunk1 + 0x04, $tail);
        $this->initUint32($chunk1 + 0x08, 5);
        $this->initUint32($chunk1 + 0x0c, 0);

        // Tail sentinel
        $this->initUint32($tail + 0x00, $chunk1);
        $this->initUint32($tail + 0x04, 0);
        $this->initUint32($tail + 0x08, 0);
        $this->initUint32($tail + 0x0c, 0);

        $this->call('_heapAlloc_8c01102a')->with(0x20);

        $remainder = $chunk1 + 0x40;

        $this->shouldWriteLong($chunk1 + 0x08, 2);
        $this->shouldWriteLong($chunk1 + 0x04, $remainder);
        $this->shouldWriteLong($chunk1 + 0x0c, 1);

        $this->shouldWriteLong($remainder + 0x00, $chunk1);
        $this->shouldWriteLong($remainder + 0x04, $tail);
        if ($this->isAsmObject()) {
            $this->shouldCall('__divls');
        }
        $this->shouldWriteLong($remainder + 0x08, 3);
        $this->shouldWriteLong($remainder + 0x0c, 0);

        $this->shouldWriteLong($tail + 0x00, $remainder);

        $this->shouldReturn($chunk1 + 0x20);
    }

    // Remainder is adjacent to a free chunk; they are coalesced into one.
    public function test_alloc_coalesces_remainder_with_next_free(): void
    {
        $this->setSize('__divls', 4);

        if ($this->isAsmObject()) {
            $this->onCall('__divls', function () {
                $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
            });
        }

        $heap = $this->alloc(0x100);
        $chunk1 = $heap + 0x60;
        $tail = $heap + 0xa0;

        $this->initUint32($this->addressOf('_var_heapHead_8c157a58'), $heap);
        $this->initUint32($this->addressOf('_var_heapTail_8c157a5c'), $tail);

        // Chunk 0: free, 3 units
        $this->initUint32($heap + 0x00, $heap);
        $this->initUint32($heap + 0x04, $chunk1);
        $this->initUint32($heap + 0x08, 3);
        $this->initUint32($heap + 0x0c, 0);

        // Chunk 1: free, 2 units
        $this->initUint32($chunk1 + 0x00, $heap);
        $this->initUint32($chunk1 + 0x04, $tail);
        $this->initUint32($chunk1 + 0x08, 2);
        $this->initUint32($chunk1 + 0x0c, 0);

        // Tail sentinel
        $this->initUint32($tail + 0x00, $chunk1);
        $this->initUint32($tail + 0x04, 0);
        $this->initUint32($tail + 0x08, 0);
        $this->initUint32($tail + 0x0c, 0);

        $this->call('_heapAlloc_8c01102a')->with(0x20);

        $remainder = $heap + 0x40;

        $this->shouldWriteLong($heap + 0x08, 2);
        $this->shouldWriteLong($heap + 0x04, $remainder);
        $this->shouldWriteLong($heap + 0x0c, 1);

        $this->shouldWriteLong($remainder + 0x00, $heap);
        $this->shouldWriteLong($remainder + 0x04, $chunk1);
        if ($this->isAsmObject()) {
            $this->shouldCall('__divls');
        }
        $this->shouldWriteLong($remainder + 0x08, 1);
        $this->shouldWriteLong($remainder + 0x0c, 0);

        $this->shouldWriteLong($chunk1 + 0x00, $remainder);

        // Coalesce: remainder absorbs chunk1
        $this->shouldWriteLong($remainder + 0x04, $tail);
        $this->shouldWriteLong($remainder + 0x08, 3);
        $this->shouldWriteLong($tail + 0x00, $remainder);

        $this->shouldReturn($heap + 0x20);
    }
};
