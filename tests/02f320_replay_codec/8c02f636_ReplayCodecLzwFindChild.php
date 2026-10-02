<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Searches a parent's child chain for a node with the given byte.
return new class extends TestCase {
    public function test_finds_matching_node(): void
    {
        $this->resolveSymbols();

        $this->initUint16($this->addressOf('_var_dictChild_8c22bbae') + 10 * 2, 20); // bucket[10] = 20
        $this->initUint8($this->addressOf('_var_dictByte_8c228bae') + 20, 0x41);   // key(20) != target, chain continues
        $this->initUint16($this->addressOf('_var_dictNext_8c22dbae') + 20 * 2, 30); // next[20] = 30
        $this->initUint8($this->addressOf('_var_dictByte_8c228bae') + 30, 0x5A);   // key(30) == target

        $this->call('_lzwFindChild_8c02f636')->with(10, 0x5A);

        $this->shouldReturn(30);
    }

    public function test_returns_nil_when_absent(): void
    {
        $this->resolveSymbols();

        $this->initUint16($this->addressOf('_var_dictChild_8c22bbae') + 11 * 2, 0x1000); // empty bucket

        $this->call('_lzwFindChild_8c02f636')->with(11, 0x22);

        $this->shouldReturn(0x1000);
    }

    private function resolveSymbols(): void
    {
    }
};
