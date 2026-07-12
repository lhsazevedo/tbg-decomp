<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    public function test_drawFirstOption()
    {
        $this->resolveSymbols();

        $task = 0xbebacafe;

        $this->shouldCall('_setTaskAction_8c014b3e')->with($task, $this->addressOf('_VmMenuTask_198a0'));
        $this->shouldWriteLong($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 0);
        $this->shouldWriteLong($this->addressOf('_var_menuState_8c1bc7a8') + 0x38, 0);
        $this->shouldWriteLong($this->addressOf('_var_menuState_8c1bc7a8') + 0x68, 0);

        $this->singleCall('_VmMenuSwitchFromTask_8c019e44')->with($task)->run();
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x6c);

        // Functions
        $this->setSize('_setTaskAction_8c014b3e', 0x4);
    }
};
