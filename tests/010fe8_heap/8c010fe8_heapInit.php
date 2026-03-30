<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_large_region() {
        $heap = $this->alloc(0x1000);
        $heapEnd = $heap + 0x1000;

        $this->call('_heapInit_8c010fe8')->with($heap, $heapEnd);

        $this->shouldWriteLongTo('_var_heapHead_8c157a58', $heap);
        $this->shouldWriteLong($heap + 0x0c, 0);
        $this->shouldWriteLong($heap + 0x08, 127);
        $this->shouldWriteLong($heap, $heap);
        $this->shouldWriteLong($heap + 0x04, $heapEnd - 0x20);

        $tail = $heapEnd - 0x20;

        $this->shouldWriteLongTo('_var_heapTail_8c157a5c', $tail);
        $this->shouldWriteLong($tail + 0x0c, 0);
        $this->shouldWriteLong($tail + 0x08, 0);
        $this->shouldWriteLong($tail, $heap);
        $this->shouldWriteLong($tail + 0x04, 0);
    }

    public function test_single_chunk_region() {
        $heap = $this->alloc(0x20);
        $heapEnd = $heap + 0x20;

        $this->call('_heapInit_8c010fe8')->with($heap, $heapEnd);

        $this->shouldWriteLongTo('_var_heapHead_8c157a58', $heap);
        $this->shouldWriteLong($heap + 0x0c, 0);
        $this->shouldWriteLong($heap + 0x08, 0);
        $this->shouldWriteLong($heap, $heap);
        $this->shouldWriteLong($heap + 0x04, $heap);

        $this->shouldWriteLongTo('_var_heapTail_8c157a5c', $heap);
        $this->shouldWriteLong($heap + 0x0c, 0);
        $this->shouldWriteLong($heap + 0x08, 0);
        $this->shouldWriteLong($heap, $heap);
        $this->shouldWriteLong($heap + 0x04, 0);
    }

    public function test_two_chunk_region() {
        $heap = $this->alloc(0x40);
        $heapEnd = $heap + 0x40;
        $tail = $heapEnd - 0x20;

        $this->call('_heapInit_8c010fe8')->with($heap, $heapEnd);

        $this->shouldWriteLongTo('_var_heapHead_8c157a58', $heap);
        $this->shouldWriteLong($heap + 0x0c, 0);
        $this->shouldWriteLong($heap + 0x08, 1);
        $this->shouldWriteLong($heap, $heap);
        $this->shouldWriteLong($heap + 0x04, $tail);

        $this->shouldWriteLongTo('_var_heapTail_8c157a5c', $tail);
        $this->shouldWriteLong($tail + 0x0c, 0);
        $this->shouldWriteLong($tail + 0x08, 0);
        $this->shouldWriteLong($tail, $heap);
        $this->shouldWriteLong($tail + 0x04, 0);
    }

    public function test_three_chunk_region() {
        $heap = $this->alloc(0x60);
        $heapEnd = $heap + 0x60;
        $tail = $heapEnd - 0x20;

        $this->call('_heapInit_8c010fe8')->with($heap, $heapEnd);

        $this->shouldWriteLongTo('_var_heapHead_8c157a58', $heap);
        $this->shouldWriteLong($heap + 0x0c, 0);
        $this->shouldWriteLong($heap + 0x08, 2);
        $this->shouldWriteLong($heap, $heap);
        $this->shouldWriteLong($heap + 0x04, $tail);

        $this->shouldWriteLongTo('_var_heapTail_8c157a5c', $tail);
        $this->shouldWriteLong($tail + 0x0c, 0);
        $this->shouldWriteLong($tail + 0x08, 0);
        $this->shouldWriteLong($tail, $heap);
        $this->shouldWriteLong($tail + 0x04, 0);
    }

    public function test_unaligned_region_truncates_units() {
        $heap = $this->alloc(0x50);
        $heapEnd = $heap + 0x50;
        $tail = $heapEnd - 0x20;

        $this->call('_heapInit_8c010fe8')->with($heap, $heapEnd);

        $this->shouldWriteLongTo('_var_heapHead_8c157a58', $heap);
        $this->shouldWriteLong($heap + 0x0c, 0);
        $this->shouldWriteLong($heap + 0x08, 1);
        $this->shouldWriteLong($heap, $heap);
        $this->shouldWriteLong($heap + 0x04, $tail);

        $this->shouldWriteLongTo('_var_heapTail_8c157a5c', $tail);
        $this->shouldWriteLong($tail + 0x0c, 0);
        $this->shouldWriteLong($tail + 0x08, 0);
        $this->shouldWriteLong($tail, $heap);
        $this->shouldWriteLong($tail + 0x04, 0);
    }
};
