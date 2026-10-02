<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// TrafficBlinkTurnSignal_8c026f7e increments the entry's frame counter (entry+0x260) and, once
// every 32 counts (the pre-increment value's bit 0x10 is 0 for counter%32 in
// 0..15), ORs a flag into entry+0x80: bit 2 when entry+0x47c is 0, bit 4
// otherwise.
return new class extends TestCase {
    public function test_bit0x10Set_onlyIncrementsCounter(): void {
        $entry = $this->alloc(0x480);
        $this->initUint32($entry + 0x260, 0x10); // bit 0x10 set -> skip flag update
        $this->initUint32($entry + 0x47c, 0);
        $this->initUint32($entry + 0x80, 0);

        $this->call('_TrafficBlinkTurnSignal_8c026f7e')->with($entry);

        $this->shouldWriteLong($entry + 0x260, 0x11);
    }

    public function test_bit0x10Clear_flagZero_orsBit2(): void {
        $entry = $this->alloc(0x480);
        $this->initUint32($entry + 0x260, 4); // bit 0x10 clear
        $this->initUint32($entry + 0x47c, 0);
        $this->initUint32($entry + 0x80, 0x1); // pre-existing bit must be preserved

        $this->call('_TrafficBlinkTurnSignal_8c026f7e')->with($entry);

        $this->shouldWriteLong($entry + 0x260, 5);
        $this->shouldWriteLong($entry + 0x80, 0x1 | 2);
    }

    public function test_bit0x10Clear_flagNonZero_orsBit4(): void {
        $entry = $this->alloc(0x480);
        $this->initUint32($entry + 0x260, 0); // bit 0x10 clear
        $this->initUint32($entry + 0x47c, 7); // non-zero flag
        $this->initUint32($entry + 0x80, 0x1);

        $this->call('_TrafficBlinkTurnSignal_8c026f7e')->with($entry);

        $this->shouldWriteLong($entry + 0x260, 1);
        $this->shouldWriteLong($entry + 0x80, 0x1 | 4);
    }
};
