<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private const SLOTS = 0x41;
    private const UNLOADED = 0xffffffff;

    public function test_requests_wanted_releases_dropped_keeps_loaded()
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_pedestrianAssets_8c1bbfdc');
        $basedir = $this->addressOf('_var_commonDir_8c18ad6c');
        $this->seedAllUnloaded($base);

        // slot 5: loaded but not requested -> released.
        $this->initUint32($base + 5 * 0x10 + 0x8, 0x8c500000);
        // slot 10: loaded and still requested -> kept untouched.
        $this->initUint32($base + 10 * 0x10 + 0x8, 0x8c600000);

        // slot 0's pvm filename is init_pedestrianPvmNames_8c043fd8[0] == "MOV01.pvm".

        // Request list: keep 10, load 0, terminate.
        $models = $this->alloc(3);
        $this->initUint8($models + 0, 10);
        $this->initUint8($models + 1, 0);
        $this->initUint8($models + 2, 0xff);

        $this->call('_syncPedestrianAssets_8c013df6')->with($models);

        $this->assertFlagsCleared($base);

        // Marking pass: slot 10 requested (loaded, no needsLoad); slot 0 requested
        // and unloaded so needsLoad too.
        $this->shouldWriteLong($base + 10 * 0x10 + 0x0, 1);
        $this->shouldWriteLong($base + 0 * 0x10 + 0x0, 1);
        $this->shouldWriteLong($base + 0 * 0x10 + 0x4, 1);

        // slot 0: request its pvm.
        $this->shouldCall('_AsqRequestPvm_8c011ac0')
            ->with($basedir, "MOV01.pvm", $base + 0 * 0x10 + 0x8, 2, 0);

        // slot 5: release and mark unloaded.
        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0x8c500000);
        $this->shouldWriteLong($base + 5 * 0x10 + 0x8, self::UNLOADED);
    }

    public function test_empty_list_only_clears_flags()
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_pedestrianAssets_8c1bbfdc');
        $this->seedAllUnloaded($base);

        $models = $this->alloc(1);
        $this->initUint8($models + 0, 0xff);

        $this->call('_syncPedestrianAssets_8c013df6')->with($models);

        $this->assertFlagsCleared($base);
    }

    /** Every slot starts unloaded (texlist == -1) so nothing is freed by default. */
    private function seedAllUnloaded(int $base): void
    {
        for ($s = 0; $s < self::SLOTS; $s++) {
            $this->initUint32($base + $s * 0x10 + 0x8, self::UNLOADED);
        }
    }

    /** First pass zeroes requested/needsLoad of all slots, front to back. */
    private function assertFlagsCleared(int $base): void
    {
        for ($s = 0; $s < self::SLOTS; $s++) {
            $this->shouldWriteLong($base + $s * 0x10 + 0x0, 0);
            $this->shouldWriteLong($base + $s * 0x10 + 0x4, 0);
        }
    }

    private function resolveSymbols(): void
    {
        // _var_commonDir_8c18ad6c is defined in the C object.
        $this->setSize('_var_pedestrianAssets_8c1bbfdc', self::SLOTS * 0x10);
        $this->setSize('_AsqRequestPvm_8c011ac0', 4);
        $this->setSize('_AsqReleaseAndFreeTexlist_8c011e3c', 4);
    }
};
