<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private const STRIDE = 0x2c;

    public function test_empty_entry_calls_hook_and_clears_pair_id()
    {
        [$entry] = $this->setup(1, 0);

        $this->call('_syncSegmentModels_8c013f78');

        // ukn_0x1c is 0; then no pairs -> var_currentTileRegionList_8c226534 becomes -1.
        $this->shouldCall('_FUN_8c029ad4')->with(0);
        $this->shouldWriteLong($this->addressOf('_var_currentTileRegionList_8c226534'), 0xffffffff);
    }

    public function test_publishes_fog_params()
    {
        [$entry] = $this->setup(1, 0);

        // field_0x00 = 0 -> var_fogParam_8c226504 = -1, var_fogParam_8c226508 = -1/2 = 0 (toward zero).
        $fog = $this->alloc(0x14);
        $this->initUint32($fog + 0x00, 0);
        $this->initUint32($fog + 0x04, 0x12345678);
        $this->initUint32($entry + 0x24, $fog);

        $this->call('_syncSegmentModels_8c013f78');

        $this->shouldWriteLong($this->addressOf('_var_fogParams_8c18ad28'), $fog);
        $this->shouldWriteLong($this->addressOf('_var_fogParam_8c226504'), 0xffffffff);
        $this->shouldWriteLong($this->addressOf('_var_fogParam_8c226508'), 0);
        $this->shouldWriteLong($this->addressOf('_var_fogParam_8c227dd0'), 0x12345678);

        $this->shouldCall('_FUN_8c029ad4')->with(0);
        $this->shouldWriteLong($this->addressOf('_var_currentTileRegionList_8c226534'), 0xffffffff);
    }

    public function test_requests_nj_pvm_pairs()
    {
        [$entry] = $this->setup(1, 0);

        $pairs = $this->alloc(0x10);
        $this->initUint32($entry + 0x28, $pairs);

        $this->call('_syncSegmentModels_8c013f78');

        $this->shouldCall('_AsqRequestModels_12030')
            ->with($this->addressOf('_var_commonDir_8c18ad6c'), $pairs, 0x10)
            ->andReturn(0x8c990000);
        $this->shouldWriteLong($this->addressOf('_var_segmentModels_8c1bc3f0'), 0x8c990000);

        $this->shouldCall('_FUN_8c029ad4')->with(0);
        $this->shouldWriteLong($this->addressOf('_var_currentTileRegionList_8c226534'), 0xffffffff);
    }

    public function test_non_demo_syncs_entry_models()
    {
        [$entry] = $this->setup(1, 0);
        // bb900 == 0 -> normal (non-demo) model path.

        $models = 0x8cab0000;
        $this->initUint32($entry + 0x10, $models);

        $this->call('_syncSegmentModels_8c013f78');

        $this->shouldWriteLong($this->addressOf('_var_routeModelIndexes_8c18adb0'), $models);
        $this->shouldCall('_syncRouteModelAssets_8c013c34')->with($models);

        $this->shouldCall('_FUN_8c029ad4')->with(0);
        $this->shouldWriteLong($this->addressOf('_var_currentTileRegionList_8c226534'), 0xffffffff);
    }

    public function test_demo_uses_fixed_list_and_runs_tail()
    {
        [$entry] = $this->setup(1, 1, 0);
        // bb900 != 0, demo inactive -> demo branch.

        $models = 0x8cab0000;
        $this->initUint32($entry + 0x10, $models);

        $this->call('_syncSegmentModels_8c013f78');

        // The entry list is still latched, but the fixed demo list is synced.
        $this->shouldWriteLong($this->addressOf('_var_routeModelIndexes_8c18adb0'), $models);
        $this->shouldCall('_syncRouteModelAssets_8c013c34')
            ->with($this->addressOf('_init_8c043fd4'));

        $this->shouldCall('_FUN_8c029ad4')->with(0);
        $this->shouldWriteLong($this->addressOf('_var_currentTileRegionList_8c226534'), 0xffffffff);
        $this->shouldCall('_FUN_8c02aa36');
    }

    public function test_reconciles_second_table()
    {
        [$entry] = $this->setup(1, 0);

        $list = 0x8cac0000;
        $this->initUint32($entry + 0x18, $list);

        $this->call('_syncSegmentModels_8c013f78');

        $this->shouldCall('_syncPedestrianAssets_8c013df6')->with($list);

        $this->shouldCall('_FUN_8c029ad4')->with(0);
        $this->shouldWriteLong($this->addressOf('_var_currentTileRegionList_8c226534'), 0xffffffff);
    }

    public function test_requests_four_dat_files()
    {
        [$entry] = $this->setup(1, 0);

        $names = $this->alloc(4 * 4);
        for ($i = 0; $i < 4; $i++) {
            $this->initUint32($names + $i * 4, 0x8cd00000 + $i);
        }
        $this->initUint32($entry + 0x0c, 0x21);   // pair id, non-zero
        $this->initUint32($entry + 0x20, $names);

        $this->call('_syncSegmentModels_8c013f78');

        $this->shouldCall('_FUN_8c029ad4')->with(0);
        $this->shouldWriteLong($this->addressOf('_var_currentTileRegionList_8c226534'), 0x21);

        $basedir = $this->addressOf('_var_datDir_8c18ad2c');
        $dest = $this->addressOf('_var_datFiles_8c18adb4');
        for ($i = 0; $i < 4; $i++) {
            $this->shouldCall('_AsqRequestDat_11182')
                ->with($basedir, 0x8cd00000 + $i, $dest + $i * 4)
                ->andReturn(0);
        }
    }

    /**
     * Seed var_currentCourseConfig_8c18ad18 so var_currentSegment_8c228708 selects entry $index (zeroed), and set
     * the demo-gating globals. Returns [entryAddr, arrayBase].
     */
    private function setup(int $index, int $bb900, int $demo = 0): array
    {
        $this->setSize('_var_currentSegment_8c228708', 4);
        $this->setSize('_var_cutsceneActive_8c1bb900', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_fogParam_8c226504', 4);
        $this->setSize('_var_fogParam_8c226508', 4);
        $this->setSize('_var_fogParam_8c227dd0', 4);
        $this->setSize('_var_currentTileRegionList_8c226534', 4);
        $this->setSize('_var_segmentModels_8c1bc3f0', 4);
        $this->setSize('_AsqRequestModels_12030', 4);
        $this->setSize('_AsqRequestDat_11182', 4);
        $this->setSize('_FUN_8c029ad4', 4);
        $this->setSize('_FUN_8c02aa36', 4);

        $holder = $this->alloc(0x10);
        $array = $this->alloc(8 * self::STRIDE);
        $entry = $array + $index * self::STRIDE;

        // alloc memory is uninitialized; zero the selected entry.
        for ($off = 0; $off < self::STRIDE; $off += 4) {
            $this->initUint32($entry + $off, 0);
        }

        $this->initUint32($this->addressOf('_var_currentCourseConfig_8c18ad18'), $holder);
        $this->initUint32($holder + 8, $array);
        $this->initUint32($this->addressOf('_var_currentSegment_8c228708'), $index);
        $this->initUint32($this->addressOf('_var_cutsceneActive_8c1bb900'), $bb900);
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), $demo);

        return [$entry, $array];
    }
};
