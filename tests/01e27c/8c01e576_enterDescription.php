<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_installs_description_reveal_task()
    {
        $this->resolveSymbols();

        // Allocate task parameter
        $task = $this->alloc(0x20);

        $this->call('_enterDescription_8c01e576')->with($task);

        // Step 1: Install the description-reveal task action
        $this->shouldCall('_TaskSwitch_8c014b3e')->with(
            $task,
            $this->addressOf('_lessonDescriptionTask_8c01e27c')
        );

        // Step 2: Reset menuState's shared state field
        $menuStateBase = $this->addressOf('_var_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x18, 0);

        // Step 3: Initialize asset queues
        $this->shouldCall('_AsqInitQueues_8c011f36')->with(8, 0, 0, 8);

        // Step 4: Reset asset queues
        $this->shouldCall('_AsqResetQueues_8c011f6c');

        // Step 5: Request the course's sys resource group
        $this->shouldCall('_CourseMenuRequestSysResgrp_8c018568')->with(
            $menuStateBase + 0x0c,
            $this->addressOf('_init_practice02ResourceGroup_8c044284')
        );

        // Step 6: Set unknown PVM boolean
        $this->shouldCall('_RouteSetLatch_8c014330');

        // Step 7: Process asset queues
        $this->shouldCall('_AsqProcessQueues_8c011fe0')->with(
            $this->addressOf('_AsqNop_8c011120'),
            0,
            0,
            0,
            $this->addressOf('_RouteClearLatch_8c014322')
        );
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x6c);

        // Function pointers
        $this->setSize('_AsqNop_8c011120', 0x4);
        $this->setSize('_RouteClearLatch_8c014322', 0x4);
    }
};
