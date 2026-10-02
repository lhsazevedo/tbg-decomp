<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

// TrafficRunEntryScript_8c027012 is the per-tick script interpreter for one
// traffic entry: it walks opcodes starting at the entry's saved cursor
// (entry+0x2fc) until it hits a stopping condition, then writes the cursor
// back. See the function's header comment in 026710_traffic.c for the full
// opcode table.

return new class extends TestCase {
    private int $entry;

    private function isAsmObject(): bool {
        return str_contains($this->objectFile, '/asm/');
    }

    private function allocEntry(): void {
        $this->entry = $this->alloc(0x510);
    }

    // Allocates a flat run of 16-bit words (a script instruction stream) and
    // returns its address.
    private function makeScript(array $words): int {
        $addr = $this->alloc(count($words) * 2);
        foreach ($words as $i => $w) {
            $this->initUint16($addr + $i * 2, $w);
        }
        return $addr;
    }

    // Opcode 9 (end) with no prior spawn (opcode 0) this run: reports
    // failure and does not touch the cursor field at all.
    public function test_endWithoutSpawnReturnsZero(): void {
        $this->allocEntry();

        $script = $this->makeScript([9]);
        $this->initUint32($this->entry + 0x2fc, $script);

        $this->call('_TrafficRunEntryScript_8c027012')->with($this->entry);

        $this->shouldReturn(0);
    }

    // Opcode 0 (spawn) delegates to initEntryState_8c026748, passing
    // it the address of the interpreter's own cursor local by reference (the
    // same out-param initEntryState_8c026748's own test drives via
    // $scriptIp); the mock simulates that side effect by writing the new
    // cursor into the address it receives in R5. entry+0x2e8 is always
    // zeroed right after the call, regardless of scriptIp's outcome. The
    // spawn is transparent -- the loop re-reads at the new cursor without
    // yielding, and hitting opcode 9 immediately after now succeeds (the
    // entry spawned earlier in this same run) and halts with the cursor
    // parked at the (unconsumed) opcode 9 word.
    public function test_spawnThenEndReturnsOneAndAdvancesCursor(): void {
        $this->allocEntry();

        $script0 = $this->makeScript([0]);
        $script1 = $this->makeScript([9]);
        $this->initUint32($this->entry + 0x2fc, $script0);

        $this->call('_TrafficRunEntryScript_8c027012')->with($this->entry);

        $this->shouldCall('_initEntryState_8c026748')
            ->do(function () use ($script1) {
                $ipAddr = $this->registers[5]->value;
                $this->memory->writeUInt32($ipAddr, U32::of($script1));
            });
        $this->shouldWriteLong($this->entry + 0x2e8, 0);

        $this->shouldWriteLong($this->entry + 0x2fc, $script1);
        $this->shouldReturn(1);
    }

    // Opcode 1, before any spawn this run: marks the entry spawned (same
    // flag opcode 0 sets), advances to the next resolved path block
    // (entry+0x300 index into the entry+0x304 pointer array) and refreshes
    // entry+0x414 from the instruction's second argument word, then
    // continues to the next opcode -- so an immediately following opcode 9
    // now succeeds (spawned this run, by opcode 1 itself) and halts with
    // the cursor parked at the (unconsumed) opcode 9 word.
    public function test_opcode1AdvancesBlockAndMarksSpawned(): void {
        $this->allocEntry();

        $block1 = $this->alloc(4);

        $script = $this->makeScript([1, 0x1111, 0x8000, 9]);
        $this->initUint32($this->entry + 0x2fc, $script);
        $this->initUint32($this->entry + 0x300, 0); // current block index
        $this->initUint32($this->entry + 0x304, 0); // block 0 (unused here)
        $this->initUint32($this->entry + 0x308, $block1); // block 1

        $this->call('_TrafficRunEntryScript_8c027012')->with($this->entry);

        $this->shouldWriteLong($this->entry + 0x300, 1);
        $this->shouldWriteLong($this->entry + 0x2b8, $block1);
        $this->shouldWriteFloat($this->entry + 0x414, 0.5); // 0x8000 / 65536.0

        $this->shouldWriteLong($this->entry + 0x2fc, $script + 6);
        $this->shouldReturn(1);
    }

    // Opcode 1, once the entry has already spawned this run: instead of
    // advancing, it halts *without* consuming itself -- the cursor is
    // written back pointing at the still-unprocessed opcode 1 word, and no
    // entry fields from the opcode-1 handler are touched.
    public function test_opcode1AfterSpawnYieldsWithoutConsuming(): void {
        $this->allocEntry();

        $script0 = $this->makeScript([0]);
        $script1 = $this->makeScript([1, 0x1111, 0x8000]);
        $this->initUint32($this->entry + 0x2fc, $script0);

        $this->call('_TrafficRunEntryScript_8c027012')->with($this->entry);

        $this->shouldCall('_initEntryState_8c026748')
            ->do(function () use ($script1) {
                $ipAddr = $this->registers[5]->value;
                $this->memory->writeUInt32($ipAddr, U32::of($script1));
            });
        $this->shouldWriteLong($this->entry + 0x2e8, 0);

        $this->shouldWriteLong($this->entry + 0x2fc, $script1);
        $this->shouldReturn(1);
    }

    // Opcodes 2/3 both configure the 0x42c/0x430/0x434/0x438 family, using
    // 0x430 to record which of the two ran.
    public function test_opcode2ConfiguresFields(): void {
        $this->allocEntry();

        $script = $this->makeScript([2, 0x1234, 0x5678, 9]);
        $this->initUint32($this->entry + 0x2fc, $script);

        $this->call('_TrafficRunEntryScript_8c027012')->with($this->entry);

        $this->shouldWriteLong($this->entry + 0x42c, 1);
        $this->shouldWriteLong($this->entry + 0x430, 0);
        $this->shouldWriteLong($this->entry + 0x434, 0x1234);
        $this->shouldWriteLong($this->entry + 0x438, 0x5678);

        $this->shouldReturn(0);
    }

    public function test_opcode3ConfiguresFieldsWithFlagSet(): void {
        $this->allocEntry();

        $script = $this->makeScript([3, 0x1234, 0x5678, 9]);
        $this->initUint32($this->entry + 0x2fc, $script);

        $this->call('_TrafficRunEntryScript_8c027012')->with($this->entry);

        $this->shouldWriteLong($this->entry + 0x42c, 1);
        $this->shouldWriteLong($this->entry + 0x430, 1);
        $this->shouldWriteLong($this->entry + 0x434, 0x1234);
        $this->shouldWriteLong($this->entry + 0x438, 0x5678);

        $this->shouldReturn(0);
    }

    // Opcode 4 is a pure 3-word skip: no entry field is touched.
    public function test_opcode4Skips(): void {
        $this->allocEntry();

        $script = $this->makeScript([4, 0xaaaa, 0xbbbb, 9]);
        $this->initUint32($this->entry + 0x2fc, $script);

        $this->call('_TrafficRunEntryScript_8c027012')->with($this->entry);

        $this->shouldReturn(0);
    }

    public function test_opcode5ConfiguresFields(): void {
        $this->allocEntry();

        $script = $this->makeScript([5, 0x2222, 9]);
        $this->initUint32($this->entry + 0x2fc, $script);
        $this->initUint32($this->entry + 0x300, 7); // current block index

        $this->call('_TrafficRunEntryScript_8c027012')->with($this->entry);

        $this->shouldWriteLong($this->entry + 0x448, 2);
        $this->shouldWriteLong($this->entry + 0x450, 0x2222);
        $this->shouldWriteLong($this->entry + 0x454, 7);

        $this->shouldReturn(0);
    }

    public function test_opcode6ConfiguresFields(): void {
        $this->allocEntry();

        $script = $this->makeScript([6, 0x3333, 0x4444, 9]);
        $this->initUint32($this->entry + 0x2fc, $script);
        $this->initUint32($this->entry + 0x300, 3);

        $this->call('_TrafficRunEntryScript_8c027012')->with($this->entry);

        $this->shouldWriteLong($this->entry + 0x458, 2);
        $this->shouldWriteLong($this->entry + 0x45c, 0x3333);
        $this->shouldWriteLong($this->entry + 0x460, 0x4444);
        $this->shouldWriteLong($this->entry + 0x464, 3);

        $this->shouldReturn(0);
    }

    public function test_opcode7ConfiguresFields(): void {
        $this->allocEntry();

        $script = $this->makeScript([7, 0x5555, 9]);
        $this->initUint32($this->entry + 0x2fc, $script);
        $this->initUint32($this->entry + 0x300, 4);

        $this->call('_TrafficRunEntryScript_8c027012')->with($this->entry);

        $this->shouldWriteLong($this->entry + 0x468, 2);
        $this->shouldWriteLong($this->entry + 0x46c, 0x5555);
        $this->shouldWriteLong($this->entry + 0x470, 4);

        $this->shouldReturn(0);
    }

    // Opcode 8 additionally resolves its 3rd argument word through
    // var_cpuPathBlocks_8c227e1c -- the same array TrafficReadScriptArgs_8c026710 indexes
    // for opcode 1.
    public function test_opcode8ConfiguresFieldsAndResolvesArg(): void {
        $this->allocEntry();

        $resolved = $this->alloc(4 * 4);
        $this->initUint32($resolved + 2 * 4, 0x9999); // index 2
        $this->initUint32($this->addressOf('_var_cpuPathBlocks_8c227e1c'), $resolved);

        $script = $this->makeScript([8, 0x1111, 0x2222, 2, 9]);
        $this->initUint32($this->entry + 0x2fc, $script);
        $this->initUint32($this->entry + 0x300, 5);

        $this->call('_TrafficRunEntryScript_8c027012')->with($this->entry);

        $this->shouldWriteLong($this->entry + 0x474, 1);
        $this->shouldWriteLong($this->entry + 0x478, 0x1111);
        $this->shouldWriteLong($this->entry + 0x47c, 0x2222);
        $this->shouldWriteLong($this->entry + 0x484, 0x9999);
        $this->shouldWriteLong($this->entry + 0x480, 5);

        $this->shouldReturn(0);
    }

    // Opcode 10 places a fixed-position/fixed-heading parked vehicle and
    // delegates the rest of spawning to initEntryState_8c026748 (whose
    // own test proves it never touches the cursor when the entry's script
    // header word is 10, i.e. a parked vehicle -- the mock here reflects that by
    // not writing back through R5 at all). The instruction is only 3 words
    // long by its argument count, but per the real asm the cursor still ends
    // up parked at the *last* argument word (the angle) rather than past it
    // -- a genuine original-game quirk, preserved as-is.
    public function test_opcode10PlacesParked(): void {
        $this->allocEntry();

        $script = $this->makeScript([10, 100, 200, 0x4000]);
        $this->initUint32($this->entry + 0x2fc, $script);

        $this->call('_TrafficRunEntryScript_8c027012')->with($this->entry);

        $this->shouldWriteFloat($this->entry + 0xf4, 10.0); // 100 / 10.0
        $this->shouldWriteFloat($this->entry + 0xfc, 20.0); // 200 / 10.0
        $this->shouldWriteLong($this->entry + 0x250, 0x4000);
        $this->shouldWriteLong($this->entry + 0x254, 0x4000 + 0x8000);
        // initEntryState_8c026748 receives the address of the interpreter's
        // own cursor local (by reference), not its value -- that's a stack
        // slot, so it differs per object.
        $this->shouldCall('_initEntryState_8c026748')
            ->with($this->entry, $this->isAsmObject() ? 0xffffdc : 0xffffe0)
            ->do(function () use ($script) {
                $ip = $this->memory->readUInt32($this->registers[5]->value)->value;
                if ($ip !== $script + 6) {
                    throw new \Exception(sprintf(
                        'Expected *scriptIp to be 0x%x, got 0x%x',
                        $script + 6,
                        $ip
                    ));
                }
            });

        $this->shouldWriteLong($this->entry + 0x2fc, $script + 6);
        $this->shouldReturn(1);
    }
};
