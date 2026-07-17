<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private const SLOTS = 0x41;
    private const UNLOADED = 0xffffffff;

    public function test_frees_loaded_slots_marks_them_unloaded()
    {
        $this->setSize('_var_pedestrianAssets_8c1bbfdc', self::SLOTS * 0x10);
        $this->setSize('_AsqReleaseAndFreeTexlist_8c011e3c', 4);

        $base = $this->addressOf('_var_pedestrianAssets_8c1bbfdc');
        $this->seedAllUnloaded($base);

        // slot 3 and slot 40 are loaded -> released in slot order.
        $this->initUint32($base + 3 * 0x10 + 0x8, 0x8c500000);
        $this->initUint32($base + 40 * 0x10 + 0x8, 0x8c700000);

        $this->call('_RouteLoadFreePedestrianAssets_8c013ee4');

        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0x8c500000);
        $this->shouldWriteLong($base + 3 * 0x10 + 0x8, self::UNLOADED);

        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0x8c700000);
        $this->shouldWriteLong($base + 40 * 0x10 + 0x8, self::UNLOADED);
    }

    public function test_all_unloaded_does_nothing()
    {
        $this->setSize('_var_pedestrianAssets_8c1bbfdc', self::SLOTS * 0x10);
        $this->setSize('_AsqReleaseAndFreeTexlist_8c011e3c', 4);

        $base = $this->addressOf('_var_pedestrianAssets_8c1bbfdc');
        $this->seedAllUnloaded($base);

        $this->call('_RouteLoadFreePedestrianAssets_8c013ee4');
    }

    /** Every slot starts unloaded (texlist == -1) so nothing is freed. */
    private function seedAllUnloaded(int $base): void
    {
        for ($s = 0; $s < self::SLOTS; $s++) {
            $this->initUint32($base + $s * 0x10 + 0x8, self::UNLOADED);
        }
    }
};
