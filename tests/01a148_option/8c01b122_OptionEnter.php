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
     * Reinstall the top-menu task with the cursor on `row`: install
     * topMenuTask_8c01b00a, reset the phase to 0, repoint the SETTING toggle
     * pointer at its backing array, seed the cursor row, then kick the fade-in.
     */
    private function arrange(int $row): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_var_progress_8c1ba1cc', 0xe8);
        $this->setSize('_TaskReinstall_8c014b3e', 4);
        $this->setSize('_RenderStartFadeIn_8c022a9c', 4);

        $task = $this->alloc(0x20);
        $this->call('_OptionEnter_8c01b122')->with($task, $row);

        $this->shouldCall('_TaskReinstall_8c014b3e')->with($task, $this->addressOf('_topMenuTask_8c01b00a'));
        $this->shouldWriteLong($this->menu(self::STATE), 0);
        $this->shouldWriteLong($this->menu(self::SELECTED), $row);
        $this->shouldWriteLong($this->addressOf('_var_settingValues_8c226074'),
            $this->addressOf('_var_progress_8c1ba1cc') + 0xc4); // difficulty_0xc4, the first SETTING row
        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
    }

    public function test_setting_row()
    {
        $this->arrange(0);
    }

    public function test_key_configure_row()
    {
        $this->arrange(1);
    }
};
