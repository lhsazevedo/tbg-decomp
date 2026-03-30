<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // No merge: chunk is head (no backward) and next is tail (no forward).
    // Only the used flag is cleared.
    public function test_free_no_merge(): void
    {
        $heap = $this->alloc(0x40);
        $tail = $heap + 0x20;

        $this->initUint32($this->addressOf('_var_heapHead_8c157a58'), $heap);
        $this->initUint32($this->addressOf('_var_heapTail_8c157a5c'), $tail);

        // Head chunk: used, next -> tail
        $this->initUint32($heap + 0x00, $heap);
        $this->initUint32($heap + 0x04, $tail);
        $this->initUint32($heap + 0x08, 1);
        $this->initUint32($heap + 0x0c, 1);

        // Tail sentinel
        $this->initUint32($tail + 0x00, $heap);
        $this->initUint32($tail + 0x04, 0);
        $this->initUint32($tail + 0x08, 0);
        $this->initUint32($tail + 0x0c, 0);

        $this->call('_heapFree_8c0110c4')->with($heap + 0x20);

        $this->shouldWriteLong($heap + 0x0c, 0);

        $this->shouldReturn(0);
    }

    // Forward merge only: chunk is head (no backward), next is free and not tail.
    // chunk absorbs next: units added, next pointer and following prev updated.
    public function test_free_forward_merge(): void
    {
        $heap = $this->alloc(0x80);
        $chunk1 = $heap + 0x20;
        $tail = $heap + 0x60;

        $this->initUint32($this->addressOf('_var_heapHead_8c157a58'), $heap);
        $this->initUint32($this->addressOf('_var_heapTail_8c157a5c'), $tail);

        // Head chunk: used, 1 unit, next -> chunk1
        $this->initUint32($heap + 0x00, $heap);
        $this->initUint32($heap + 0x04, $chunk1);
        $this->initUint32($heap + 0x08, 1);
        $this->initUint32($heap + 0x0c, 1);

        // Chunk 1: free, 2 units, next -> tail
        $this->initUint32($chunk1 + 0x00, $heap);
        $this->initUint32($chunk1 + 0x04, $tail);
        $this->initUint32($chunk1 + 0x08, 2);
        $this->initUint32($chunk1 + 0x0c, 0);

        // Tail sentinel
        $this->initUint32($tail + 0x00, $chunk1);
        $this->initUint32($tail + 0x04, 0);
        $this->initUint32($tail + 0x08, 0);
        $this->initUint32($tail + 0x0c, 0);

        $this->call('_heapFree_8c0110c4')->with($heap + 0x20);

        // Clear used flag
        $this->shouldWriteLong($heap + 0x0c, 0);

        // Forward merge: absorb chunk1 into head
        $this->shouldWriteLong($heap + 0x04, $tail);     // next = chunk1->next
        $this->shouldWriteLong($heap + 0x08, 3);         // units = 1 + 2
        $this->shouldWriteLong($tail + 0x00, $heap);     // tail->prev = head

        $this->shouldReturn(0);
    }

    // Backward merge only: next is tail (no forward), chunk is not head and prev is free.
    // prev absorbs chunk: prev's next and units updated, tail's prev set to prev.
    public function test_free_backward_merge(): void
    {
        $heap = $this->alloc(0x60);
        $chunk = $heap + 0x20;
        $tail = $heap + 0x40;

        $this->initUint32($this->addressOf('_var_heapHead_8c157a58'), $heap);
        $this->initUint32($this->addressOf('_var_heapTail_8c157a5c'), $tail);

        // Head chunk: free, 1 unit, next -> chunk
        $this->initUint32($heap + 0x00, $heap);
        $this->initUint32($heap + 0x04, $chunk);
        $this->initUint32($heap + 0x08, 1);
        $this->initUint32($heap + 0x0c, 0);

        // Chunk to free: used, 1 unit, next -> tail
        $this->initUint32($chunk + 0x00, $heap);
        $this->initUint32($chunk + 0x04, $tail);
        $this->initUint32($chunk + 0x08, 1);
        $this->initUint32($chunk + 0x0c, 1);

        // Tail sentinel
        $this->initUint32($tail + 0x00, $chunk);
        $this->initUint32($tail + 0x04, 0);
        $this->initUint32($tail + 0x08, 0);
        $this->initUint32($tail + 0x0c, 0);

        $this->call('_heapFree_8c0110c4')->with($chunk + 0x20);

        // Clear used flag
        $this->shouldWriteLong($chunk + 0x0c, 0);

        // Backward merge: prev (head) absorbs chunk
        $this->shouldWriteLong($heap + 0x04, $tail);     // prev->next = chunk->next
        $this->shouldWriteLong($heap + 0x08, 2);         // prev->units = 1 + 1
        $this->shouldWriteLong($tail + 0x00, $heap);     // tail->prev = head

        $this->shouldReturn(0);
    }

    // Forward then backward: free chunk is sandwiched between two free chunks.
    // Forward merge first (absorbs next), then backward merge (prev absorbs chunk+next).
    public function test_free_both_merges(): void
    {
        $heap = $this->alloc(0x80);
        $chunk = $heap + 0x20;
        $chunk2 = $heap + 0x40;
        $tail = $heap + 0x60;

        $this->initUint32($this->addressOf('_var_heapHead_8c157a58'), $heap);
        $this->initUint32($this->addressOf('_var_heapTail_8c157a5c'), $tail);

        // Head chunk: free, 1 unit, next -> chunk
        $this->initUint32($heap + 0x00, $heap);
        $this->initUint32($heap + 0x04, $chunk);
        $this->initUint32($heap + 0x08, 1);
        $this->initUint32($heap + 0x0c, 0);

        // Chunk to free: used, 1 unit, next -> chunk2
        $this->initUint32($chunk + 0x00, $heap);
        $this->initUint32($chunk + 0x04, $chunk2);
        $this->initUint32($chunk + 0x08, 1);
        $this->initUint32($chunk + 0x0c, 1);

        // Chunk 2: free, 1 unit, next -> tail
        $this->initUint32($chunk2 + 0x00, $chunk);
        $this->initUint32($chunk2 + 0x04, $tail);
        $this->initUint32($chunk2 + 0x08, 1);
        $this->initUint32($chunk2 + 0x0c, 0);

        // Tail sentinel
        $this->initUint32($tail + 0x00, $chunk2);
        $this->initUint32($tail + 0x04, 0);
        $this->initUint32($tail + 0x08, 0);
        $this->initUint32($tail + 0x0c, 0);

        $this->call('_heapFree_8c0110c4')->with($chunk + 0x20);

        // Clear used flag
        $this->shouldWriteLong($chunk + 0x0c, 0);

        // Forward merge: chunk absorbs chunk2
        $this->shouldWriteLong($chunk + 0x04, $tail);    // next = chunk2->next
        $this->shouldWriteLong($chunk + 0x08, 2);        // units = 1 + 1
        $this->shouldWriteLong($tail + 0x00, $chunk);    // tail->prev = chunk

        // Backward merge: head absorbs chunk (which now spans chunk+chunk2)
        $this->shouldWriteLong($heap + 0x04, $tail);     // prev->next = chunk->next
        $this->shouldWriteLong($heap + 0x08, 3);         // prev->units = 1 + 2
        $this->shouldWriteLong($tail + 0x00, $heap);     // tail->prev = head

        $this->shouldReturn(0);
    }
};
