<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// TrafficResolvePathBlocks_8c026710 walks a stage script (an array of ushort
// words) starting 2 bytes in, decoding one instruction at a time. Each
// instruction's word count comes from init_scriptOpWords_8c0460bc, indexed by the
// instruction's own opcode word. Opcode 1 additionally resolves its operand
// (the instruction's second word) through the var_cpuPathBlocks_8c227e1c pointer table and
// appends the resolved value to entry+0x304. Opcode 9 terminates the scan and
// the output array is closed off with a -1 sentinel.

return new class extends TestCase {
    public function test_emptyScriptWritesOnlyTerminator(): void {
        $entry = $this->alloc(0x310);
        $script = $this->alloc(4);

        // Header word (never read) + immediate terminator.
        $this->initUint16($script + 0, 0);
        $this->initUint16($script + 2, 9);

        $this->call('_TrafficResolvePathBlocks_8c026710')->with($entry, $script);

        $this->shouldWriteLong($entry + 0x304, 0xffffffff);
    }

    public function test_skipsNonPushOpcodeThenResolvesPush(): void {
        $entry = $this->alloc(0x310);
        $script = $this->alloc(12);

        $table = $this->alloc(4 * 4);
        $this->initUint32($table + 0 * 4, 0x11111111);
        $this->initUint32($table + 1 * 4, 0x22222222);
        $this->initUint32($table + 2 * 4, 0x33333333);
        $this->initUint32($table + 3 * 4, 0x44444444);
        $this->initUint32($this->addressOf('_var_cpuPathBlocks_8c227e1c'), $table);

        // Header word (never read).
        $this->initUint16($script + 0, 0);
        // Opcode 0: a one-word instruction that isn't a push -- must not
        // write anything to the output array.
        $this->initUint16($script + 2, 0);
        // Opcode 1: a three-word push instruction; operand selects table[2].
        $this->initUint16($script + 4, 1);
        $this->initUint16($script + 6, 2);
        $this->initUint16($script + 8, 0); // unused third word of the instruction
        // Terminator.
        $this->initUint16($script + 10, 9);

        $this->call('_TrafficResolvePathBlocks_8c026710')->with($entry, $script);

        $this->shouldWriteLong($entry + 0x304, 0x33333333);
        $this->shouldWriteLong($entry + 0x308, 0xffffffff);
    }
};
