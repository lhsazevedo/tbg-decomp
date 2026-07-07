<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private const STRIDE = 0x2c;

    public function test_frees_pairs_and_hands_off_when_both_flags_set()
    {
        $entry = $this->setup(2, 1, 1);
        $this->call('_freeSelectedEntryPairs_8c013f22');
        $this->shouldCall('_AsqFreeNjPvmPairs_120fe')->with($this->addressOf('_var_8c1bc3f0'));
        $this->shouldCall('_FUN_8c021a24');
    }

    public function test_only_frees_pairs_when_handoff_flag_clear()
    {
        $this->setup(0, 1, 0);
        $this->call('_freeSelectedEntryPairs_8c013f22');
        $this->shouldCall('_AsqFreeNjPvmPairs_120fe')->with($this->addressOf('_var_8c1bc3f0'));
    }

    public function test_only_hands_off_when_free_flag_clear()
    {
        $this->setup(0, 0, 1);
        $this->call('_freeSelectedEntryPairs_8c013f22');
        $this->shouldCall('_FUN_8c021a24');
    }

    public function test_does_nothing_when_both_flags_clear()
    {
        $this->setup(0, 0, 0);
        $this->call('_freeSelectedEntryPairs_8c013f22');
    }

    /**
     * Seed the entry array so var_8c228708 selects entry $index, whose ukn_0x28
     * (gates the pair free) and ukn_0x0c (gates the handoff) hold $f28 / $f0c.
     */
    private function setup(int $index, int $f28, int $f0c): int
    {
        // _var_currentCourse_8c18ad18 is defined in the C object; the other two are imported.
        $this->setSize('_var_8c228708', 4);
        $this->setSize('_var_8c1bc3f0', 4);
        $this->setSize('_AsqFreeNjPvmPairs_120fe', 4);
        $this->setSize('_FUN_8c021a24', 4);

        $holder = $this->alloc(0x10);
        $array = $this->alloc(4 * self::STRIDE);
        $entry = $array + $index * self::STRIDE;

        // var_currentCourse_8c18ad18 -> holder; holder+8 -> entry array base.
        $this->initUint32($this->addressOf('_var_currentCourse_8c18ad18'), $holder);
        $this->initUint32($holder + 8, $array);
        $this->initUint32($this->addressOf('_var_8c228708'), $index);

        $this->initUint32($entry + 0x28, $f28);
        $this->initUint32($entry + 0xc, $f0c);

        return $entry;
    }
};
