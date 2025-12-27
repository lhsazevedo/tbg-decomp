<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_draws_story_menu_hud_elements(): void
    {
        $this->resolveSymbols();

        $progressBase = $this->addressOf('_var_progress_8c1ba1cc');
        $menuBase = $this->addressOf('_menuState_8c1bc7a8');
        $timeTable = $this->addressOf('_init_routeInfoTime_8c044d2e');

        $dayCount = 123;
        $row = 1;
        $column = 4;
        $index = $row * 6 + ($column - 2) * 2; // mirrors in-game computation

        $this->initUint32($progressBase + 0x00, $dayCount);
        $this->initUint32($menuBase + 0x3c, $column);
        $this->initUint32($menuBase + 0x40, $row);

        $this->initUint8($timeTable + $index, 0x2a);      // hour
        $this->initUint8($timeTable + $index + 1, 0x37);  // minute

        $this->call('_drawRouteInfo_8c018118');

        $this->shouldCall('_drawFixedInteger_8c01803e')->with(219.0, 108.0, $dayCount, 0);
        $this->shouldCall('_getWeekDayIndex_8c016ed2')->andReturn(5);
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $menuBase + 0x0c,
            5 + 0x16,
            281.0,
            110.0,
            -4.0,
        );
        $this->shouldCall('_drawFixedInteger_8c01803e')->with(421.0, 108.0, 0x2a, 2);
        $this->shouldCall('_drawFixedInteger_8c01803e')->with(471.0, 108.0, 0x37, 2);
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $menuBase + 0x0c,
            $row + 9,
            0.0,
            0.0,
            -7.0,
        );
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_menuState_8c1bc7a8', 0x6c);
        $this->setSize('_var_progress_8c1ba1cc', 0x94);
        $this->setSize('_init_routeInfoTime_8c044d2e', 0x20);
    }
};
