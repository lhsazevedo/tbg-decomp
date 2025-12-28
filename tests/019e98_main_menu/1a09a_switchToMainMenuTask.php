<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_basic()
    {
        $this->resolveSymbols();

        $task = 0xbebacafe;

        $this->shouldCall('_setTaskAction_8c014b3e')->with($task, $this->addressOf('_MainMenuTask_8c019e98'));
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x18, 0);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x38, 0);
        $this->shouldWriteLong($this->addressOf('_menuState_8c1bc7a8') + 0x5c, 0);
        $this->shouldCall('_AsqInitQueues_11f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_11f6c');
        $this->shouldCall('_CourseMenuRequestSysResgrp_8c018568')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c,
            $this->addressOf('_init_mainMenuResourceGroup_8c044264'),
        );
        $this->shouldCall('_setUknPvmBool_8c014330');
        $this->shouldCall('_AsqProcessQueues_11fe0')->with(
            $this->addressOf('_AsqNop_11120'),
            0,
            0,
            0,
            $this->addressOf('_resetUknPvmBool_8c014322')
        );

        $this->singleCall('_MainMenuSwitchFromTask_8c01a09a')->with($task)->run();
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_menuState_8c1bc7a8', 0x6c);

        // Functions
        //$this->setSize('_setTaskAction_8c014b3e', 0x4);
    }
};
