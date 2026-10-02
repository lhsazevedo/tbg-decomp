<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// Unlinks a dictionary node from its parent's child chain.
return new class extends TestCase {
    public function test_remove_head_of_bucket(): void
    {
        $this->resolveSymbols();

        // node 20 is the head of parent(10)'s bucket, next=30
        $this->initUint16($this->addressOf('_var_dictPrev_8c22fbae') + 20 * 2, 0x1000);
        $this->initUint16($this->addressOf('_var_dictNext_8c22dbae') + 20 * 2, 30);
        $this->initUint16($this->addressOf('_var_dictParent_8c229bae') + 20 * 2, 10);

        $this->call('_lzwRemoveChild_8c02f6ac')->with(20);

        $this->shouldWriteWord($this->addressOf('_var_dictChild_8c22bbae') + 10 * 2, 30);
        $this->shouldWriteWord($this->addressOf('_var_dictPrev_8c22fbae') + 30 * 2, 0x1000);
    }

    public function test_remove_middle_of_bucket(): void
    {
        $this->resolveSymbols();

        // node 20 sits between prev=15 and next=30
        $this->initUint16($this->addressOf('_var_dictPrev_8c22fbae') + 20 * 2, 15);
        $this->initUint16($this->addressOf('_var_dictNext_8c22dbae') + 20 * 2, 30);

        $this->call('_lzwRemoveChild_8c02f6ac')->with(20);

        $this->shouldWriteWord($this->addressOf('_var_dictNext_8c22dbae') + 15 * 2, 30);
        $this->shouldWriteWord($this->addressOf('_var_dictPrev_8c22fbae') + 30 * 2, 15);
    }

    public function test_remove_tail_of_bucket(): void
    {
        $this->resolveSymbols();

        $this->initUint16($this->addressOf('_var_dictPrev_8c22fbae') + 20 * 2, 15);
        $this->initUint16($this->addressOf('_var_dictNext_8c22dbae') + 20 * 2, 0x1000);

        $this->call('_lzwRemoveChild_8c02f6ac')->with(20);

        $this->shouldWriteWord($this->addressOf('_var_dictNext_8c22dbae') + 15 * 2, 0x1000);
    }

    private function resolveSymbols(): void
    {
    }
};
