<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private const SLOTS = 0x20;
    private const UNLOADED = 0xffffffff;

    public function test_loads_wanted_frees_dropped_keeps_loaded()
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_8c1bbddc');
        $basedir = $this->addressOf('_var_basedir_8c18ad6c');
        $this->seedAllUnloaded($base);

        // Default filename table.
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0);

        // slot 5: loaded but not requested -> freed.
        $this->initUint32($base + 5 * 0x10 + 0x8, 0x8c500000);
        $this->initUint32($base + 5 * 0x10 + 0xc, 0x8c500010);
        // slot 10: loaded and still requested -> kept untouched.
        $this->initUint32($base + 10 * 0x10 + 0x8, 0x8c600000);

        // slot 3 maps to init_8c043dc4 entries 6/7 ("3s_2do1_s.njd" / .pvm).

        // Request list: keep 10, load 3, terminate.
        $models = $this->alloc(3);
        $this->initUint8($models + 0, 10);
        $this->initUint8($models + 1, 3);
        $this->initUint8($models + 2, 0xff);

        $this->call('_syncRouteModelAssets_8c013c34')->with($models);

        $this->assertFlagsCleared($base);

        // Marking pass: requested for 10 and 3; slot 3 is unloaded so needsLoad too.
        $this->shouldWriteLong($base + 10 * 0x10 + 0x0, 1);
        $this->shouldWriteLong($base + 3 * 0x10 + 0x0, 1);
        $this->shouldWriteLong($base + 3 * 0x10 + 0x4, 1);

        // slot 3: request its files.
        $this->shouldCall('_AsqRequestNj_11492')
            ->with($basedir, "3s_2do1_s.njd", 0, $base + 3 * 0x10 + 0xc);
        $this->shouldCall('_AsqRequestPvm_11ac0')
            ->with($basedir, "3t_2do1_s.pvm", $base + 3 * 0x10 + 0x8, 0x60, 0);

        // slot 5: free it and mark unloaded.
        $this->shouldCall('_AsqReleaseAndFreeTexlist_11e3c')->with(0x8c500000);
        $this->shouldCall('_syFree')->with(0x8c500010);
        $this->shouldWriteLong($base + 5 * 0x10 + 0x8, self::UNLOADED);
    }

    public function test_uses_alternate_table_when_mode_is_2()
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_8c1bbddc');
        $basedir = $this->addressOf('_var_basedir_8c18ad6c');
        $this->seedAllUnloaded($base);

        // Mode 2 selects init_8c043ecc; slot 1 maps to entries 2/3.
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 2);

        // Load slot 1 only.
        $models = $this->alloc(2);
        $this->initUint8($models + 0, 1);
        $this->initUint8($models + 1, 0xff);

        $this->call('_syncRouteModelAssets_8c013c34')->with($models);

        $this->assertFlagsCleared($base);
        $this->shouldWriteLong($base + 1 * 0x10 + 0x0, 1);
        $this->shouldWriteLong($base + 1 * 0x10 + 0x4, 1);

        $this->shouldCall('_AsqRequestNj_11492')
            ->with($basedir, "3s_2do0_sn.njd", 0, $base + 1 * 0x10 + 0xc);
        $this->shouldCall('_AsqRequestPvm_11ac0')
            ->with($basedir, "3t_2do0_sn.pvm", $base + 1 * 0x10 + 0x8, 0x60, 0);
    }

    public function test_empty_list_only_clears_flags()
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_8c1bbddc');
        $this->seedAllUnloaded($base);

        // Nothing loaded, nothing requested: no requests, no frees.
        $models = $this->alloc(1);
        $this->initUint8($models + 0, 0xff);

        $this->call('_syncRouteModelAssets_8c013c34')->with($models);

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
        // _var_timeOfDay_8c18ad20 and _var_basedir_8c18ad6c are defined in the C object.
        $this->setSize('_var_8c1bbddc', self::SLOTS * 0x10);

        $this->setSize('_AsqRequestNj_11492', 4);
        $this->setSize('_AsqRequestPvm_11ac0', 4);
        $this->setSize('_AsqReleaseAndFreeTexlist_11e3c', 4);
        $this->setSize('_syFree', 4);
    }
};
