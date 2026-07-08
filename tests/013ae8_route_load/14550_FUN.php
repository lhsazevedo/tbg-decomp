<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    /** value at var_loadingResourceGroup_8c1bc3f8.tlist_0x00, passed to the nj texture calls */
    private const TLIST = 0x8c500000;

    /** State 0: build the interior queue, start the pass, draw. */
    public function test_state0_starts_pass(): void
    {
        $this->resolveSizes();
        $task = $this->makeTask(0, 0);

        $this->call('_unknownSegmentReloadTask_8c014550')->with($task, 0);

        $this->shouldCall('_FUN_8c02b170');
        $this->shouldCall('_AsqResetQueues_11f6c');
        $this->shouldCall('_syncSegmentModels_8c013f78');
        $this->shouldCall('_resetPvmReady_8c014322');
        $this->shouldCall('_AsqProcessQueues_11fe0')->with(
            $this->addressOf('_AsqNop_11120'),
            $this->addressOf('_FUN_8c021810'),
            $this->addressOf('_FUN_8c02190a'),
            0,
            $this->addressOf('_setPvmReady_8c014330'),
        );
        $this->shouldWriteLong($task + 0x08, 1);

        $this->expectLoadingAnimation($task);
    }

    /** State 1, pvm not ready: only the loading animation runs. */
    public function test_state1_waits_and_draws(): void
    {
        $this->resolveSizes();
        $task = $this->makeTask(1, 0);

        $this->call('_unknownSegmentReloadTask_8c014550')->with($task, 0);

        $this->shouldCall('_isPvmReady_8c01432a')->andReturn(0);
        $this->expectLoadingAnimation($task);
    }

    /** State 1, pvm ready: advance, no drawing. */
    public function test_state1_advances_when_ready(): void
    {
        $this->resolveSizes();
        $task = $this->makeTask(1, 0);

        $this->call('_unknownSegmentReloadTask_8c014550')->with($task, 0);

        $this->shouldCall('_isPvmReady_8c01432a')->andReturn(1);
        $this->shouldWriteLong($task + 0x08, 2);
    }

    /** State 2: advance to teardown, no drawing. */
    public function test_state2_advances(): void
    {
        $this->resolveSizes();
        $task = $this->makeTask(2, 0);

        $this->call('_unknownSegmentReloadTask_8c014550')->with($task, 0);

        $this->shouldWriteLong($task + 0x08, 3);
    }

    /** State 3: tear down, bind the interior texture, hand off to the input task. */
    public function test_state3_teardown_and_handoff(): void
    {
        $this->resolveSizes();
        $interior = 0x8c600000;
        $this->initUint32($this->addressOf('_var_interiorTexlist_8c1bc438'), $interior);
        $task = $this->makeTask(3, 0);

        $this->call('_unknownSegmentReloadTask_8c014550')->with($task, 0);

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
        $task = $this->makeTask(4, 0);

        $this->call('_unknownSegmentReloadTask_8c014550')->with($task, 0);

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
            '_FUN_8c02b170' => 4,
            '_FUN_8c021810' => 4,
            '_FUN_8c02190a' => 4,
            '__modls' => 4,
        ] as $sym => $size) {
            $this->setSize($sym, $size);
        }
    }
};
