<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /** value at var_loadingResourceGroup_8c1bc3f8.tlist_0x00, passed to the nj texture calls */
    private const TLIST = 0x8c500000;

    /** State 0: reset queues, load the route models, start the first pass. */
    public function test_state0_starts_load_pass(): void
    {
        $this->resolveSizes();
        $task = $this->makeTask(0, 0);

        $this->call('_unknownRouteLoadTask_8c014784')->with($task, 0);

        $this->shouldCall('_AsqResetQueues_11f6c');
        $this->shouldCall('_njSetTexture')->with(self::TLIST);
        $this->shouldCall('_njLoadCacheTexture')->with(self::TLIST);
        $this->shouldCall('_loadRouteModels_8c014088');
        $this->shouldCall('_resetPvmReady_8c014322');
        $this->shouldCall('_AsqProcessQueues_11fe0')->with(
            $this->addressOf('_AsqNop_11120'),
            0, 0, 0,
            $this->addressOf('_setPvmReady_8c014330'),
        );
        $this->shouldWriteLong($task + 0x08, 1);

        $this->expectLoadingAnimation($task);
    }

    /** State 1, pvm not ready yet: only the loading animation runs. */
    public function test_state1_waits_and_draws(): void
    {
        $this->resolveSizes();
        $task = $this->makeTask(1, 0);

        $this->call('_unknownRouteLoadTask_8c014784')->with($task, 0);

        $this->shouldCall('_isPvmReady_8c01432a')->andReturn(0);
        $this->expectLoadingAnimation($task);
    }

    /** State 1, pvm ready: run the post-load setup, start the second pass, draw.
     *  Unlike routeLoadTask_8c014338, AsqProcessQueues gets 0 for the second arg. */
    public function test_state1_runs_post_load(): void
    {
        $this->resolveSizes();

        $base = $this->addressOf('_var_currentCourse_8c1bb868');
        $this->initUint32($base + 0x24, 0x8cd00008);   // slots_0x04[8]
        $this->initUint32($base + 0x30, 0x8cd0000b);   // slots_0x04[11]
        $this->initUint32($base + 0x34, 0x8cd0000c);   // slots_0x04[12]

        $task = $this->makeTask(1, 0);

        $this->call('_unknownRouteLoadTask_8c014784')->with($task, 0);

        $this->shouldCall('_isPvmReady_8c01432a')->andReturn(1);
        $this->shouldCall('_FUN_8c02175a');
        $this->shouldCall('_FUN_8c026da4')->with(0x8cd00008);
        $this->shouldCall('_FUN_8c028de8')->with(0x8cd0000b);
        $this->shouldCall('_FUN_8c028dd0')->with(0x8cd0000c);
        $this->shouldCall('_FUN_8c02caba');
        $this->shouldCall('_FUN_8c02b170');
        $this->shouldCall('_AsqResetQueues_11f6c');
        $this->shouldCall('_syncSegmentModels_8c013f78');
        $this->shouldCall('_resetPvmReady_8c014322');
        $this->shouldCall('_AsqProcessQueues_11fe0')->with(
            $this->addressOf('_AsqNop_11120'),
            0,
            $this->addressOf('_FUN_8c02190a'),
            0,
            $this->addressOf('_setPvmReady_8c014330'),
        );
        $this->shouldWriteLong($task + 0x08, 2);

        $this->expectLoadingAnimation($task);
    }

    /** State 2, pvm ready: advance to the idle state, no drawing. */
    public function test_state2_advances_when_ready(): void
    {
        $this->resolveSizes();
        $task = $this->makeTask(2, 0);

        $this->call('_unknownRouteLoadTask_8c014784')->with($task, 0);

        $this->shouldCall('_isPvmReady_8c01432a')->andReturn(1);
        $this->shouldWriteLong($task + 0x08, 3);
    }

    /** State 2, pvm not ready: keep drawing the loading animation. */
    public function test_state2_waits_and_draws(): void
    {
        $this->resolveSizes();
        $task = $this->makeTask(2, 0);

        $this->call('_unknownRouteLoadTask_8c014784')->with($task, 0);

        $this->shouldCall('_isPvmReady_8c01432a')->andReturn(0);
        $this->expectLoadingAnimation($task);
    }

    /** State 3: idle one frame. */
    public function test_state3_idles(): void
    {
        $this->resolveSizes();
        $task = $this->makeTask(3, 0);

        $this->call('_unknownRouteLoadTask_8c014784')->with($task, 0);

        $this->shouldWriteLong($task + 0x08, 4);
    }

    /** State 4: tear down, bind the interior texture, hand off to the input task. */
    public function test_state4_teardown_and_handoff(): void
    {
        $this->resolveSizes();
        $interior = 0x8c600000;
        $this->initUint32($this->addressOf('_var_loadingResourceGroup_8c1bc3f8'), self::TLIST);
        $this->initUint32($this->addressOf('_var_interiorTexlist_8c1bc438'), $interior);
        $task = $this->makeTask(4, 0);

        $this->call('_unknownRouteLoadTask_8c014784')->with($task, 0);

        $this->shouldCall('_freeTask_8c014b66')->with($task);
        $this->shouldCall('_AsqFreeQueues_11f7e');
        $this->shouldWriteLong($this->addressOf('_var_loadScreenActive_8c157a6c'), 0);
        $this->shouldCall('_njReleaseTexture')->with(self::TLIST);
        $this->shouldCall('_njSetTexture')->with($interior);
        $this->shouldCall('_njLoadCacheTexture')->with($interior);
        $this->shouldCall('_FUN_8c01306e');
        $this->shouldCall('_dispatchInputTask_8c012970');
    }

    /** Unexpected state: falls through the switch to just the animation. */
    public function test_default_state_draws(): void
    {
        $this->resolveSizes();
        $task = $this->makeTask(5, 0);

        $this->call('_unknownRouteLoadTask_8c014784')->with($task, 0);

        $this->expectLoadingAnimation($task);
    }

    /**
     * The loading animation: a base sprite plus one of six cycling frames.
     * With field_0x0c == 0 the frame index is __modls(0, 6) + 1 == 1.
     */
    private function expectLoadingAnimation(int $task): void
    {
        $this->onCall('__modls', function () {
            $this->setRegister(0, $this->getRegister(1)->mod($this->getRegister(0)));
        });

        $res = $this->addressOf('_var_loadingResourceGroup_8c1bc3f8');
        $this->shouldCall('_drawSprite_8c014f54')->with($res, 0, 0.0, 0.0, -5.0);
        $this->shouldWriteLong($task + 0x0c, 1);
        $this->shouldCall('__modls');
        $this->shouldCall('_drawSprite_8c014f54')->with($res, 1, 0.0, 0.0, -4.0);
    }

    private function makeTask(int $state, int $frame): int
    {
        $task = $this->alloc(0x20);
        for ($off = 0; $off < 0x20; $off += 4) {
            $this->initUint32($task + $off, 0);
        }
        $this->initUint32($task + 0x08, $state);
        $this->initUint32($task + 0x0c, $frame);
        $this->initUint32($this->addressOf('_var_loadingResourceGroup_8c1bc3f8'), self::TLIST);

        return $task;
    }

    private function resolveSizes(): void
    {
        foreach ([
            '_var_loadingResourceGroup_8c1bc3f8' => 0x0c,
            '_var_loadScreenActive_8c157a6c' => 4,
            '_var_currentCourse_8c1bb868' => 0x50,
            '_var_interiorTexlist_8c1bc438' => 4,
            '_AsqResetQueues_11f6c' => 4,
            '_AsqFreeQueues_11f7e' => 4,
            '_AsqProcessQueues_11fe0' => 4,
            '_AsqNop_11120' => 4,
            '_njSetTexture' => 4,
            '_njLoadCacheTexture' => 4,
            '_njReleaseTexture' => 4,
            '_drawSprite_8c014f54' => 4,
            '_dispatchInputTask_8c012970' => 4,
            '_freeTask_8c014b66' => 4,
            '_FUN_8c01306e' => 4,
            '_FUN_8c02175a' => 4,
            '_FUN_8c026da4' => 4,
            '_FUN_8c028de8' => 4,
            '_FUN_8c028dd0' => 4,
            '_FUN_8c02caba' => 4,
            '_FUN_8c02b170' => 4,
            '_FUN_8c02190a' => 4,
            '__modls' => 4,
        ] as $sym => $size) {
            $this->setSize($sym, $size);
        }
    }
};
