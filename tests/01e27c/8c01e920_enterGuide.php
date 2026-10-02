<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_resgrp_ready_reprocesses_and_returns_to_description()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $this->call('_enterGuide_8c01e920')->with($task);

        $this->shouldCall('_TaskSwitch_8c014b3e')->with(
            $task,
            $this->addressOf('_lessonGuideTask_8c01e63c')
        );

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x40, 0);

        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("");
        $this->shouldCall('_AsqInitQueues_8c011f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_8c011f6c');
        $this->shouldCall('_CourseMenuRequestSysResgrp_8c018568')->with(
            $menuStateBase + 0x0c,
            $this->addressOf('_init_practice01ResourceGroup_8c044274')
        )->andReturn(1);

        $this->shouldCall('_RouteSetLatch_8c014330');
        $this->shouldCall('_AsqProcessQueues_8c011fe0')->with(
            $this->addressOf('_AsqNop_8c011120'),
            0,
            0,
            0,
            $this->addressOf('_RouteClearLatch_8c014322')
        );

        $this->shouldWriteLong($menuStateBase + 0x18, 0);
    }

    public function test_resgrp_not_ready_frees_queues_and_fades_in()
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $this->call('_enterGuide_8c01e920')->with($task);

        $this->shouldCall('_TaskSwitch_8c014b3e')->with(
            $task,
            $this->addressOf('_lessonGuideTask_8c01e63c')
        );

        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x40, 0);

        $this->shouldCall('_MessageBoxSwapFor_8c02aefc')->with("");
        $this->shouldCall('_AsqInitQueues_8c011f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_8c011f6c');
        $this->shouldCall('_CourseMenuRequestSysResgrp_8c018568')->with(
            $menuStateBase + 0x0c,
            $this->addressOf('_init_practice01ResourceGroup_8c044274')
        )->andReturn(0);

        $this->shouldCall('_AsqFreeQueues_8c011f7e');
        $this->shouldWriteLong($menuStateBase + 0x18, 1);
        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x84);
        $this->setSize('_TaskSwitch_8c014b3e', 4);
        $this->setSize('_MessageBoxSwapFor_8c02aefc', 4);
        $this->setSize('_AsqInitQueues_8c011f36', 4);
        $this->setSize('_AsqResetQueues_8c011f6c', 4);
        $this->setSize('_CourseMenuRequestSysResgrp_8c018568', 4);
        $this->setSize('_RouteSetLatch_8c014330', 4);
        $this->setSize('_AsqProcessQueues_8c011fe0', 4);
        $this->setSize('_AsqNop_8c011120', 4);
        $this->setSize('_RouteClearLatch_8c014322', 4);
        $this->setSize('_AsqFreeQueues_8c011f7e', 4);
        $this->setSize('_RenderStartFadeIn_8c022a9c', 4);
    }
};
