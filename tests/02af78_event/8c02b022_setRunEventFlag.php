<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _setRunEventFlag_8c02b022(index): sets bit `index` of the single-word bitset
 * var_runEventFlags_8c1ba2b4 -- per-day scratch flags (cleared each scan) that
 * let an event fired on one segment gate a later segment's pick.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_runEventFlags_8c1ba2b4', 4);
    }

    public function test_sets_bit(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_runEventFlags_8c1ba2b4'), 0);

        $this->call('_setRunEventFlag_8c02b022')->with(5);

        $this->shouldWriteLongTo('_var_runEventFlags_8c1ba2b4', 0x20); // bit 5
    }

    public function test_ors_into_existing_bits(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_runEventFlags_8c1ba2b4'), 0x1);

        $this->call('_setRunEventFlag_8c02b022')->with(2);

        $this->shouldWriteLongTo('_var_runEventFlags_8c1ba2b4', 0x5); // 0x1 | 0x4
    }
};
