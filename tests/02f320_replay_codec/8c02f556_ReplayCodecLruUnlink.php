<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Swaps a node's position with its sibling in the parent/child index
// tables, or promotes it when it is already the "escape" node.
return new class extends TestCase {
    public function test_promotes_escape_node(): void
    {
        $this->resolveSymbols();

        $this->initUint16($this->addressOf('_var_lruTail_8c235bb2'), 5);
        $this->initUint16($this->addressOf('_var_lruPrev_8c231bb0') + 5 * 2, 42);

        $this->call('_lruUnlink_8c02f556')->with(5);

        $this->shouldWriteWord($this->addressOf('_var_lruTail_8c235bb2'), 42);
        $this->shouldWriteWord($this->addressOf('_var_lruNext_8c233bb0') + 42 * 2, 0x1000);
    }

    public function test_swaps_sibling_nodes(): void
    {
        $this->resolveSymbols();

        $this->initUint16($this->addressOf('_var_lruTail_8c235bb2'), 99);
        $this->initUint16($this->addressOf('_var_lruNext_8c233bb0') + 7 * 2, 3);
        $this->initUint16($this->addressOf('_var_lruPrev_8c231bb0') + 7 * 2, 11);

        $this->call('_lruUnlink_8c02f556')->with(7);

        $this->shouldWriteWord($this->addressOf('_var_lruPrev_8c231bb0') + 3 * 2, 11);
        $this->shouldWriteWord($this->addressOf('_var_lruNext_8c233bb0') + 11 * 2, 3);
    }

    private function resolveSymbols(): void
    {
    }
};
