<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_initializes_course_menu_task()
    {
        $this->resolveSymbols();

        // Allocate task parameter
        $task = $this->alloc(0x20);

        $this->call('_CourseMenuFUN_8c0184cc')->with($task);

        // Step 1: Garbage texture
        $this->shouldCall('_njGarbageTexture')->with(
            $this->addressOf('_var_tex_8c157af8'),
            0xc00
        );

        // Step 2: Set task action to course confirm menu task
        $this->shouldCall('_setTaskAction_8c014b3e')->with(
            $task,
            $this->addressOf('_CourseConfirmMenuTask_8c0181b6')
        );

        // Step 3: Initialize menuState fields
        $menuStateBase = $this->addressOf('_menuState_8c1bc7a8');
        $this->shouldWriteLong($menuStateBase + 0x18, 0);
        $this->shouldWriteLong($menuStateBase + 0x38, 0);

        // Step 4: Initialize asset queues
        $this->shouldCall('_AsqInitQueues_11f36')->with(8, 0, 0, 8);

        // Step 5: Reset asset queues
        $this->shouldCall('_AsqResetQueues_11f6c');

        // Step 6: Request course parts resource group
        $this->shouldCall('_CourseMenuRequestSysResgrp_8c018568')->with(
            $menuStateBase + 0x0c,
            $this->addressOf('_init_8c044d40')
        );

        // Step 7: Set unknown PVM boolean
        $this->shouldCall('_setUknPvmBool_8c014330');

        // Step 8: Process asset queues
        $this->shouldCall('_AsqProcessQueues_11fe0')->with(
            $this->addressOf('_AsqNop_11120'),
            0,
            0,
            0,
            $this->addressOf('_resetUknPvmBool_8c014322')
        );

        // Step 9: Reset menuState state field again
        $this->shouldWriteLong($menuStateBase + 0x18, 0);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_menuState_8c1bc7a8', 0x6c);
        $this->setSize('_var_tex_8c157af8', 0xc00);

        // Function pointers
        $this->setSize('_AsqNop_11120', 0x4);
        $this->setSize('_resetUknPvmBool_8c014322', 0x4);

        // Resource group pointer
        $this->setSize('_init_8c044d40', 0x8);
    }
};

