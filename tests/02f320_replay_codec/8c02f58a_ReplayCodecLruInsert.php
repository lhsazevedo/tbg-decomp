<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Inserts a node into the recency-ordered doubly linked list.
return new class extends TestCase {
    public function test_empty_list_becomes_singleton(): void
    {
        $this->resolveSymbols();

        $this->initUint16($this->addressOf('_var_lruHead_8c235bb0'), 0x1000);

        $this->call('_lruInsert_8c02f58a')->with(5, 0x1000);

        $this->shouldWriteWord($this->addressOf('_var_lruPrev_8c231bb0') + 5 * 2, 0x1000);
        $this->shouldWriteWord($this->addressOf('_var_lruNext_8c233bb0') + 5 * 2, 0x1000);
        $this->shouldWriteWord($this->addressOf('_var_lruTail_8c235bb2'), 5);
        $this->shouldWriteWord($this->addressOf('_var_lruHead_8c235bb0'), 5);
    }

    public function test_insert_at_tail_when_after_is_nil(): void
    {
        $this->resolveSymbols();

        $this->initUint16($this->addressOf('_var_lruHead_8c235bb0'), 1);
        $this->initUint16($this->addressOf('_var_lruTail_8c235bb2'), 3);

        $this->call('_lruInsert_8c02f58a')->with(5, 0x1000);

        $this->shouldWriteWord($this->addressOf('_var_lruNext_8c233bb0') + 5 * 2, 0x1000);
        $this->shouldWriteWord($this->addressOf('_var_lruPrev_8c231bb0') + 5 * 2, 3);
        $this->shouldWriteWord($this->addressOf('_var_lruNext_8c233bb0') + 3 * 2, 5);
        $this->shouldWriteWord($this->addressOf('_var_lruTail_8c235bb2'), 5);
    }

    public function test_insert_at_head_when_after_equals_head(): void
    {
        $this->resolveSymbols();

        $this->initUint16($this->addressOf('_var_lruHead_8c235bb0'), 3);

        $this->call('_lruInsert_8c02f58a')->with(5, 3);

        $this->shouldWriteWord($this->addressOf('_var_lruNext_8c233bb0') + 5 * 2, 3);
        $this->shouldWriteWord($this->addressOf('_var_lruPrev_8c231bb0') + 5 * 2, 0x1000);
        $this->shouldWriteWord($this->addressOf('_var_lruPrev_8c231bb0') + 3 * 2, 5);
        $this->shouldWriteWord($this->addressOf('_var_lruHead_8c235bb0'), 5);
    }

    public function test_insert_after_generic_node(): void
    {
        $this->resolveSymbols();

        $this->initUint16($this->addressOf('_var_lruHead_8c235bb0'), 1);
        $this->initUint16($this->addressOf('_var_lruPrev_8c231bb0') + 7 * 2, 9); // next[after]=9

        $this->call('_lruInsert_8c02f58a')->with(5, 7);

        $this->shouldWriteWord($this->addressOf('_var_lruNext_8c233bb0') + 5 * 2, 7);
        $this->shouldWriteWord($this->addressOf('_var_lruPrev_8c231bb0') + 5 * 2, 9);
        $this->shouldWriteWord($this->addressOf('_var_lruNext_8c233bb0') + 9 * 2, 5);
        $this->shouldWriteWord($this->addressOf('_var_lruPrev_8c231bb0') + 7 * 2, 5);
    }

    private function resolveSymbols(): void
    {
    }
};
