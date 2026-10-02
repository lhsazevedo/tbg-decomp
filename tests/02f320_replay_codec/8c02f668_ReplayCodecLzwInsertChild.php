<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Adds a dictionary node as the new head of its parent's child chain.
return new class extends TestCase {
    public function test_insert_into_empty_bucket(): void
    {
        $this->resolveSymbols();

        $this->initUint16($this->addressOf('_var_dictChild_8c22bbae') + 10 * 2, 0x1000); // bucket[10] empty

        $this->call('_lzwInsertChild_8c02f668')->with(10, 20, 0x41);

        $this->shouldWriteByte($this->addressOf('_var_dictByte_8c228bae') + 20, 0x41);
        $this->shouldWriteWord($this->addressOf('_var_dictParent_8c229bae') + 20 * 2, 10);
        $this->shouldWriteWord($this->addressOf('_var_dictPrev_8c22fbae') + 20 * 2, 0x1000);
        $this->shouldWriteWord($this->addressOf('_var_dictChild_8c22bbae') + 20 * 2, 0x1000);
        $this->shouldWriteWord($this->addressOf('_var_dictNext_8c22dbae') + 20 * 2, 0x1000);
        $this->shouldWriteWord($this->addressOf('_var_dictChild_8c22bbae') + 10 * 2, 20);
    }

    public function test_insert_into_nonempty_bucket(): void
    {
        $this->resolveSymbols();

        $this->initUint16($this->addressOf('_var_dictChild_8c22bbae') + 10 * 2, 30); // bucket[10] = 30

        $this->call('_lzwInsertChild_8c02f668')->with(10, 20, 0x42);

        $this->shouldWriteByte($this->addressOf('_var_dictByte_8c228bae') + 20, 0x42);
        $this->shouldWriteWord($this->addressOf('_var_dictParent_8c229bae') + 20 * 2, 10);
        $this->shouldWriteWord($this->addressOf('_var_dictPrev_8c22fbae') + 20 * 2, 0x1000);
        $this->shouldWriteWord($this->addressOf('_var_dictChild_8c22bbae') + 20 * 2, 0x1000);
        $this->shouldWriteWord($this->addressOf('_var_dictNext_8c22dbae') + 20 * 2, 30);
        $this->shouldWriteWord($this->addressOf('_var_dictPrev_8c22fbae') + 30 * 2, 20);
        $this->shouldWriteWord($this->addressOf('_var_dictChild_8c22bbae') + 10 * 2, 20);
    }

    private function resolveSymbols(): void
    {
    }
};
