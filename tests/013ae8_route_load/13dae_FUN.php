<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private const SLOTS = 0x20;
    private const UNLOADED = 0xffffffff;

    public function test_frees_loaded_slots_marks_them_unloaded()
    {
        $this->setSize('_var_routeModelSlots_8c1bbddc', self::SLOTS * 0x10);
        $this->setSize('_AsqReleaseAndFreeTexlist_11e3c', 4);
        $this->setSize('_syFree', 4);

        $base = $this->addressOf('_var_routeModelSlots_8c1bbddc');
        $this->seedAllUnloaded($base);

        // slot 2 and slot 7 are loaded -> freed in slot order.
        $this->initUint32($base + 2 * 0x10 + 0x8, 0x8c500000);
        $this->initUint32($base + 2 * 0x10 + 0xc, 0x8c500010);
        $this->initUint32($base + 7 * 0x10 + 0x8, 0x8c700000);
        $this->initUint32($base + 7 * 0x10 + 0xc, 0x8c700010);

        $this->call('_freeAllRouteModels_8c013dae');

        $this->shouldCall('_AsqReleaseAndFreeTexlist_11e3c')->with(0x8c500000);
        $this->shouldCall('_syFree')->with(0x8c500010);
        $this->shouldWriteLong($base + 2 * 0x10 + 0x8, self::UNLOADED);

        $this->shouldCall('_AsqReleaseAndFreeTexlist_11e3c')->with(0x8c700000);
        $this->shouldCall('_syFree')->with(0x8c700010);
        $this->shouldWriteLong($base + 7 * 0x10 + 0x8, self::UNLOADED);
    }

    public function test_all_unloaded_does_nothing()
    {
        $this->setSize('_var_routeModelSlots_8c1bbddc', self::SLOTS * 0x10);
        $this->setSize('_AsqReleaseAndFreeTexlist_11e3c', 4);
        $this->setSize('_syFree', 4);

        $base = $this->addressOf('_var_routeModelSlots_8c1bbddc');
        $this->seedAllUnloaded($base);

        $this->call('_freeAllRouteModels_8c013dae');
    }

    /** Every slot starts unloaded (texlist == -1) so nothing is freed. */
    private function seedAllUnloaded(int $base): void
    {
        for ($s = 0; $s < self::SLOTS; $s++) {
            $this->initUint32($base + $s * 0x10 + 0x8, self::UNLOADED);
        }
    }
};
