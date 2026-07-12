<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _setProgressFlag_8c02af78(index): sets unlock bit `index` in the two paired
 * bitsets of the progress struct at var_progress_8c1ba1cc+0x04 and +0x18.
 *
 *   word = index >> 5;   mask = 1 << (index & 0x1f);
 *   field_0x04[word] |= mask;
 *   field_0x18[word] |= mask;
 */
return new class extends TestCase {
    const FIELD_0X04 = 0x04;
    const FIELD_0X18 = 0x18;

    private function resolveSymbols(): void
    {
        $this->setSize('_var_progress_8c1ba1cc', 0xd2);
    }

    public function test_sets_bit_in_first_word(): void
    {
        $this->resolveSymbols();
        $p = $this->addressOf('_var_progress_8c1ba1cc');

        $this->initUint32($p + self::FIELD_0X04, 0);
        $this->initUint32($p + self::FIELD_0X18, 0);

        $this->call('_setProgressFlag_8c02af78')->with(5);

        // index 5 -> word 0, mask 1 << 5 = 0x20
        $this->shouldWriteLong($p + self::FIELD_0X04, 0x20);
        $this->shouldWriteLong($p + self::FIELD_0X18, 0x20);
    }

    public function test_sets_bit_in_second_word(): void
    {
        $this->resolveSymbols();
        $p = $this->addressOf('_var_progress_8c1ba1cc');

        $this->initUint32($p + self::FIELD_0X04 + 4, 0);
        $this->initUint32($p + self::FIELD_0X18 + 4, 0);

        $this->call('_setProgressFlag_8c02af78')->with(33);

        // index 33 -> word 1 (offset +4), mask 1 << 1 = 0x2
        $this->shouldWriteLong($p + self::FIELD_0X04 + 4, 0x2);
        $this->shouldWriteLong($p + self::FIELD_0X18 + 4, 0x2);
    }

    public function test_ors_into_existing_bits(): void
    {
        $this->resolveSymbols();
        $p = $this->addressOf('_var_progress_8c1ba1cc');

        $this->initUint32($p + self::FIELD_0X04, 0x1);
        $this->initUint32($p + self::FIELD_0X18, 0x1);

        $this->call('_setProgressFlag_8c02af78')->with(2);

        // mask 1 << 2 = 0x4, OR'd onto the pre-existing 0x1
        $this->shouldWriteLong($p + self::FIELD_0X04, 0x5);
        $this->shouldWriteLong($p + self::FIELD_0X18, 0x5);
    }
};
