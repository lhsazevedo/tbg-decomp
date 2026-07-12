<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_releases_when_loaded()
    {
        $this->resolveSymbols();

        // Seed the texlist pointer and its texture count.
        $texlist = $this->alloc(0x8);
        $this->initUint32($texlist + 0x4, 3);  // nbTexture
        $this->initUint32($this->addressOf('_var_interiorTexlist_8c1bc438'), $texlist);

        // Distinct pointer values so we can assert exact free args.
        $this->initUint32($this->addressOf('_var_interiorNj_8c1bc43c'), 0x8c200010);
        $this->initUint32($this->addressOf('_var_frontTexlist_8c1bc430'), 0x8c200020);
        $this->initUint32($this->addressOf('_var_frontNj_8c1bc434'), 0x8c200030);
        $this->initUint32($this->addressOf('_var_markTexlist_8c1bc418'), 0x8c200040);
        $this->initUint32($this->addressOf('_var_busStopTexlist_8c1bc424'), 0x8c200050);

        $this->call('_freeVehicleAssets_8c013b5a');

        $this->shouldCall('_njSetTexture')->with($texlist);
        $this->shouldCall('_njReleaseCacheTextureNum')->with(0);
        $this->shouldCall('_njReleaseCacheTextureNum')->with(1);
        $this->shouldCall('_njReleaseCacheTextureNum')->with(2);

        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with($texlist);
        $this->shouldCall('_syFree')->with(0x8c200010);
        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0x8c200020);
        $this->shouldCall('_syFree')->with(0x8c200030);
        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0x8c200040);
        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0x8c200050);

        $this->shouldWriteLongTo('_var_interiorTexlist_8c1bc438', 0xffffffff);
    }

    public function test_releases_with_no_textures()
    {
        $this->resolveSymbols();

        // nbTexture = 0: njSetTexture runs but the release loop does not.
        $texlist = $this->alloc(0x8);
        $this->initUint32($texlist + 0x4, 0);
        $this->initUint32($this->addressOf('_var_interiorTexlist_8c1bc438'), $texlist);

        $this->initUint32($this->addressOf('_var_interiorNj_8c1bc43c'), 0x8c200010);
        $this->initUint32($this->addressOf('_var_frontTexlist_8c1bc430'), 0x8c200020);
        $this->initUint32($this->addressOf('_var_frontNj_8c1bc434'), 0x8c200030);
        $this->initUint32($this->addressOf('_var_markTexlist_8c1bc418'), 0x8c200040);
        $this->initUint32($this->addressOf('_var_busStopTexlist_8c1bc424'), 0x8c200050);

        $this->call('_freeVehicleAssets_8c013b5a');

        $this->shouldCall('_njSetTexture')->with($texlist);

        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with($texlist);
        $this->shouldCall('_syFree')->with(0x8c200010);
        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0x8c200020);
        $this->shouldCall('_syFree')->with(0x8c200030);
        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0x8c200040);
        $this->shouldCall('_AsqReleaseAndFreeTexlist_8c011e3c')->with(0x8c200050);

        $this->shouldWriteLongTo('_var_interiorTexlist_8c1bc438', 0xffffffff);
    }

    public function test_skips_when_unloaded()
    {
        $this->resolveSymbols();

        // Sentinel -1: nothing is released.
        $this->initUint32($this->addressOf('_var_interiorTexlist_8c1bc438'), 0xffffffff);

        $this->call('_freeVehicleAssets_8c013b5a');
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_interiorTexlist_8c1bc438', 4);
        $this->setSize('_var_interiorNj_8c1bc43c', 4);
        $this->setSize('_var_frontTexlist_8c1bc430', 4);
        $this->setSize('_var_frontNj_8c1bc434', 4);
        $this->setSize('_var_markTexlist_8c1bc418', 4);
        $this->setSize('_var_busStopTexlist_8c1bc424', 4);

        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njReleaseCacheTextureNum', 4);
        $this->setSize('_AsqReleaseAndFreeTexlist_8c011e3c', 4);
        $this->setSize('_syFree', 4);
    }
};
