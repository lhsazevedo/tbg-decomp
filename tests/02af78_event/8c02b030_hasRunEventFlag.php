<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _hasRunEventFlag_8c02b030(index): tests bit `index` of the single-word bitset
 * var_runEventFlags_8c1ba2b4 (see setRunEventFlag_8c02b022). Returns the masked word, not
 * normalized to 0/1.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_runEventFlags_8c1ba2b4', 4);
    }

    public function test_returns_nonzero_when_bit_set(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_runEventFlags_8c1ba2b4'), 0x20); // bit 5 set

        $this->call('_hasRunEventFlag_8c02b030')->with(5);

        $this->shouldReturn(0x20);
    }

    public function test_returns_zero_when_bit_clear(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_runEventFlags_8c1ba2b4'), 0xdf); // all but bit 5 set

        $this->call('_hasRunEventFlag_8c02b030')->with(5);

        $this->shouldReturn(0);
    }
};
