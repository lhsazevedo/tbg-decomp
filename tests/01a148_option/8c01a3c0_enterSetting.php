<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    const STATE = 0x18;       // var_menuState_8c1bc7a8.state_0x18 offset
    const SELECTED = 0x38;    // var_menuState_8c1bc7a8.selected_0x38 offset

    private function menu(int $off): int
    {
        return $this->addressOf('_var_menuState_8c1bc7a8') + $off;
    }

    /*
     * Install the SETTING task, reset the phase and cursor to 0, kick the fade-in.
     */
    public function test_switch_to_setting()
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_TaskReinstall_8c014b3e', 4);
        $this->setSize('_RenderStartFadeIn_8c022a9c', 4);

        $task = $this->alloc(0x20);
        $this->call('_enterSetting_8c01a3c0')->with($task);

        $this->shouldCall('_TaskReinstall_8c014b3e')
            ->with($task, $this->addressOf('_settingTask_8c01a148'));
        $this->shouldWriteLong($this->menu(self::STATE), 0);
        $this->shouldWriteLong($this->menu(self::SELECTED), 0);
        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }
};
