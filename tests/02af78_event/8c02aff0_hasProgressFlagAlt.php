<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _hasProgressFlagAlt_8c02aff0(index): tests unlock bit `index` in the
 * field_0x18 bitset of var_progress_8c1ba1cc (see setProgressFlag_8c02af78).
 * Returns the masked word, not normalized to 0/1.
 */
return new class extends TestCase {
    const FIELD_0X18 = 0x18;

    private function resolveSymbols(): void
    {
        $this->setSize('_var_progress_8c1ba1cc', 0xd2);
    }

    public function test_returns_nonzero_when_bit_set(): void
    {
        $this->resolveSymbols();
        $p = $this->addressOf('_var_progress_8c1ba1cc');

        $this->initUint32($p + self::FIELD_0X18, 0x20); // bit 5 set

        $this->call('_hasProgressFlagAlt_8c02aff0')->with(5);

        $this->shouldReturn(0x20);
    }

    public function test_returns_zero_when_bit_clear(): void
    {
        $this->resolveSymbols();
        $p = $this->addressOf('_var_progress_8c1ba1cc');

        $this->initUint32($p + self::FIELD_0X18, 0xdf); // all but bit 5 set

        $this->call('_hasProgressFlagAlt_8c02aff0')->with(5);

        $this->shouldReturn(0);
    }

    public function test_reads_correct_word(): void
    {
        $this->resolveSymbols();
        $p = $this->addressOf('_var_progress_8c1ba1cc');

        $this->initUint32($p + self::FIELD_0X18, 0);
        $this->initUint32($p + self::FIELD_0X18 + 4, 0x2); // index 33 -> word 1, bit 1

        $this->call('_hasProgressFlagAlt_8c02aff0')->with(33);

        $this->shouldReturn(0x2);
    }
};
