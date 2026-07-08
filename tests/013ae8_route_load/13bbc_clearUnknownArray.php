<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_resets_handle_of_each_slot()
    {
        $count = 4;
        $base = $this->alloc($count * 0x10);

        $this->call('_clearModelSlots_8c013bbc')->with($base, $count);

        // Walks back to front, storing -1 at +8 of each 0x10-byte slot.
        for ($i = $count - 1; $i >= 0; $i--) {
            $this->shouldWriteLong($base + $i * 0x10 + 8, 0xffffffff);
        }
    }

    public function test_empty_writes_nothing()
    {
        $base = $this->alloc(0x10);

        $this->call('_clearModelSlots_8c013bbc')->with($base, 0);
    }
};
