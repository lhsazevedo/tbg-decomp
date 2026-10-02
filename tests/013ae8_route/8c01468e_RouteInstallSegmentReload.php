<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    /** value at var_loadingResourceGroup_8c1bc3f8.tlist_0x00, bound by the texture calls */
    private const TLIST = 0x8c500000;

    /** Below the hardest difficulty: the segment boundary refunds 30 driver points. */
    public function test_refunds_driver_points_then_installs_task(): void
    {
        $this->resolveSizes();
        $points = $this->addressOf('_var_runState_8c2285c4');
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xc4, 0);
        $this->initUint32($points + 0x0c, 10);
        $this->initUint32($points + 0x10, 100);

        $this->call('_RouteInstallSegmentReload_8c01468e');

        $this->shouldWriteLong($points + 0x0c, 40);
        $this->expectInstallAndBind();
    }

    /** The refund clamps to var_runState_8c2285c4.driverPointsMax_0x10. */
    public function test_clamps_refund_to_the_run_maximum(): void
    {
        $this->resolveSizes();
        $points = $this->addressOf('_var_runState_8c2285c4');
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xc4, 0);
        $this->initUint32($points + 0x0c, 90);
        $this->initUint32($points + 0x10, 100);

        $this->call('_RouteInstallSegmentReload_8c01468e');

        $this->shouldWriteLong($points + 0x0c, 120);
        $this->shouldWriteLong($points + 0x0c, 100);
        $this->expectInstallAndBind();
    }

    /** Hardest difficulty outside practice: no refund. */
    public function test_no_refund_on_the_hardest_difficulty(): void
    {
        $this->resolveSizes();
        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc') + 0xc4, 5);
        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);

        $this->call('_RouteInstallSegmentReload_8c01468e');

        $this->expectInstallAndBind();
    }

    /** Common tail: install segmentReloadTask_8c014550, prime the queues, rebind the texture. */
    private function expectInstallAndBind(): void
    {
        $this->initUint32($this->addressOf('_var_loadingResourceGroup_8c1bc3f8'), self::TLIST);

        $installedTask = $this->alloc(0x20);
        $installedState = $this->alloc(0x1c);

        $this->shouldWriteLong($this->addressOf('_var_loadScreenActive_8c157a6c'), 1);
        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_segmentReloadTask_8c014550'),
                0xffffec,
                0xfffff0,
                0,
            )
            ->do(function ($params) use ($installedTask, $installedState) {
                $this->memory->writeUInt32($params[2], U32::of($installedTask));
                $this->memory->writeUInt32($params[3], U32::of($installedState));
            });
        $this->shouldWriteLong($installedTask + 0x08, 0);
        $this->shouldWriteLong($installedTask + 0x0c, 0);
        $this->shouldCall('_freeSegmentAssets_8c013f22');
        $this->shouldCall('_njGarbageTexture')->with($this->addressOf('_var_tex_8c157af8'), 0xc00);
        $this->shouldCall('_AsqInitQueues_8c011f36')->with(0x20, 0x800, 0x800, 0x40);
        $this->shouldCall('_njSetTexture')->with(self::TLIST);
        $this->shouldCall('_njLoadCacheTexture')->with(self::TLIST);
        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    private function resolveSizes(): void
    {
        foreach ([
            '_var_progress_8c1ba1cc' => 0xd2,
            '_var_playMode_8c1bb8d0' => 4,
            '_var_loadScreenActive_8c157a6c' => 4,
            '_var_loadingResourceGroup_8c1bc3f8' => 0x0c,
            '_var_tasks_8c1ba3c8' => 4,
            '_var_tex_8c157af8' => 4,
            '_njGarbageTexture' => 4,
            '_AsqInitQueues_8c011f36' => 4,
            '_njSetTexture' => 4,
            '_njLoadCacheTexture' => 4,
            '_njSetBackColor' => 4,
        ] as $sym => $size) {
            $this->setSize($sym, $size);
        }

        $this->setSize('_var_runState_8c2285c4', 0x9c);
    }
};
