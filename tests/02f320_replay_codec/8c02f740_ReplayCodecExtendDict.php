<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Extends the LZW dictionary for each new input byte: reuses an existing
// child code if present, otherwise allocates one (from the free pool or by
// evicting the LRU tail) and links it into its parent's child chain and recency list.
return new class extends TestCase {
    public function test_nil_parent_code_returns_immediately(): void
    {
        $this->resolveSymbols();

        $symbols = $this->alloc(2);
        $this->initUint16($symbols, 0x41);

        $this->call('_extendDict_8c02f740')->with($symbols, 1, 0x1000, 0);
    }

    public function test_zero_count_returns_immediately(): void
    {
        $this->resolveSymbols();

        $symbols = $this->alloc(2);
        $this->initUint16($symbols, 0x41);

        $this->call('_extendDict_8c02f740')->with($symbols, 0, 5, 0);
    }

    public function test_pos_overflow_returns_immediately(): void
    {
        $this->resolveSymbols();

        $symbols = $this->alloc(2);
        $this->initUint16($symbols, 0x41);

        // pos=100 -> increments to 101 > 100 on first iteration.
        $this->call('_extendDict_8c02f740')->with($symbols, 1, 5, 100);
    }

    public function test_existing_child_reused_no_allocation(): void
    {
        $this->resolveSymbols();

        $symbols = $this->alloc(2);
        $this->initUint16($symbols, 0x41);

        $this->call('_extendDict_8c02f740')->with($symbols, 1, 5, 0);

        $this->shouldCall('_lzwFindChild_8c02f636')->with(5, 0x41)->andReturn(20);
    }

    public function test_allocates_new_code_from_free_pool(): void
    {
        $this->resolveSymbols();

        $this->initUint16($this->addressOf('_var_nextCode_8c231bae'), 100);
        $this->initUint16($this->addressOf('_var_lruHead_8c235bb0'), 7); // recency head

        $symbols = $this->alloc(2);
        $this->initUint16($symbols, 0x41);

        // parentCode=5 (<=0xff) -> before = recency head (7).
        $this->call('_extendDict_8c02f740')->with($symbols, 1, 5, 0);

        $this->shouldCall('_lzwFindChild_8c02f636')->with(5, 0x41)->andReturn(0x1000);
        $this->shouldWriteWord($this->addressOf('_var_nextCode_8c231bae'), 101);
        $this->shouldCall('_lzwInsertChild_8c02f668')->with(5, 100, 0x41);
        $this->shouldCall('_lruInsert_8c02f58a')->with(100, 7);
    }

    public function test_allocates_by_evicting_lru_tail(): void
    {
        $this->resolveSymbols();

        $this->initUint16($this->addressOf('_var_nextCode_8c231bae'), 0x1000); // pool exhausted
        $this->initUint16($this->addressOf('_var_lruTail_8c235bb2'), 55); // recency tail
        $this->initUint16($this->addressOf('_var_lruNext_8c233bb0') + 300 * 2, 40); // next[parentCode]

        $symbols = $this->alloc(2);
        $this->initUint16($symbols, 0x41);

        // parentCode=300 (>0xff) -> before = next[parentCode] (40).
        $this->call('_extendDict_8c02f740')->with($symbols, 1, 300, 0);

        $this->shouldCall('_lzwFindChild_8c02f636')->with(300, 0x41)->andReturn(0x1000);
        $this->shouldCall('_lruUnlink_8c02f556')->with(55);
        $this->shouldCall('_lzwRemoveChild_8c02f6ac')->with(55);
        $this->shouldCall('_lzwInsertChild_8c02f668')->with(300, 55, 0x41);
        $this->shouldCall('_lruInsert_8c02f58a')->with(55, 40);
    }

    public function test_returns_when_evicting_current_parent(): void
    {
        $this->resolveSymbols();

        $this->initUint16($this->addressOf('_var_nextCode_8c231bae'), 0x1000);
        $this->initUint16($this->addressOf('_var_lruTail_8c235bb2'), 5); // tail == parentCode

        $symbols = $this->alloc(2);
        $this->initUint16($symbols, 0x41);

        $this->call('_extendDict_8c02f740')->with($symbols, 1, 5, 0);

        $this->shouldCall('_lzwFindChild_8c02f636')->with(5, 0x41)->andReturn(0x1000);
    }

    private function resolveSymbols(): void
    {
    }
};
