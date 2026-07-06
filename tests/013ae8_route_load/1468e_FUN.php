<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    /** value at var_8c1bc3f8.tlist_0x00, bound by the texture calls */
    private const TLIST = 0x8c500000;

    /** Outside a demo (flag < 2): bump the fade counter, no clamp needed. */
    public function test_bumps_counter_then_installs_task(): void
    {
        $this->resolveSizes();
        $counter = $this->addressOf('_var_8c2285c4');
        $this->initUint32($this->addressOf('_var_8c1ba290'), 0);
        $this->initUint32($counter + 0x0c, 10);
        $this->initUint32($counter + 0x10, 100);

        $this->call('_pushInteriorLoadTask_8c01468e');

        $this->shouldWriteLong($counter + 0x0c, 40);
        $this->expectInstallAndBind();
    }

    /** Bumping past the cap clamps the counter to var_8c2285c4[4]. */
    public function test_clamps_counter_to_cap(): void
    {
        $this->resolveSizes();
        $counter = $this->addressOf('_var_8c2285c4');
        $this->initUint32($this->addressOf('_var_8c1ba290'), 0);
        $this->initUint32($counter + 0x0c, 90);
        $this->initUint32($counter + 0x10, 100);

        $this->call('_pushInteriorLoadTask_8c01468e');

        $this->shouldWriteLong($counter + 0x0c, 120);
        $this->shouldWriteLong($counter + 0x0c, 100);
        $this->expectInstallAndBind();
    }

    /** During a demo (flag >= 2, not the demo-active value): skip the counter. */
    public function test_skips_counter_during_demo(): void
    {
        $this->resolveSizes();
        $this->initUint32($this->addressOf('_var_8c1ba290'), 5);
        $this->initUint32($this->addressOf('_var_demo_8c1bb8d0'), 0);

        $this->call('_pushInteriorLoadTask_8c01468e');

        $this->expectInstallAndBind();
    }

    /** Common tail: install task_interiorLoad_8c014550, prime the queues, rebind the texture. */
    private function expectInstallAndBind(): void
    {
        $this->initUint32($this->addressOf('_var_8c1bc3f8'), self::TLIST);

        $createdTask = $this->alloc(0x20);
        $createdState = $this->alloc(0x1c);

        $this->shouldWriteLong($this->addressOf('_var_8c157a6c'), 1);
        $this->shouldCall('_pushTask_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba3c8'),
                $this->addressOf('_task_interiorLoad_8c014550'),
                0xffffec,
                0xfffff0,
                0,
            )
            ->do(function ($params) use ($createdTask, $createdState) {
                $this->memory->writeUInt32($params[2], U32::of($createdTask));
                $this->memory->writeUInt32($params[3], U32::of($createdState));
            });
        $this->shouldWriteLong($createdTask + 0x08, 0);
        $this->shouldWriteLong($createdTask + 0x0c, 0);
        $this->shouldCall('_freeSelectedEntryPairs_8c013f22');
        $this->shouldCall('_njGarbageTexture')->with($this->addressOf('_var_tex_8c157af8'), 0xc00);
        $this->shouldCall('_AsqInitQueues_11f36')->with(0x20, 0x800, 0x800, 0x40);
        $this->shouldCall('_njSetTexture')->with(self::TLIST);
        $this->shouldCall('_njLoadCacheTexture')->with(self::TLIST);
        $this->shouldCall('_njSetBackColor')->with(0xff418dff, 0xff418dff, 0xff418dff);
    }

    private function resolveSizes(): void
    {
        foreach ([
            '_var_8c1ba290' => 1,
            '_var_demo_8c1bb8d0' => 4,
            '_var_8c2285c4' => 0x14,
            '_var_8c157a6c' => 4,
            '_var_8c1bc3f8' => 0x0c,
            '_var_tasks_8c1ba3c8' => 4,
            '_var_tex_8c157af8' => 4,
            '_njGarbageTexture' => 4,
            '_AsqInitQueues_11f36' => 4,
            '_njSetTexture' => 4,
            '_njLoadCacheTexture' => 4,
            '_njSetBackColor' => 4,
        ] as $sym => $size) {
            $this->setSize($sym, $size);
        }
    }

    protected function isAsmObject(): bool
    {
        return str_ends_with($this->objectFile, '_src.obj');
    }
};
