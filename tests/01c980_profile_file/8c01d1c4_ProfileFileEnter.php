<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * ProfileFileEnter_8c01d1c4: installs menuTask_8c01ccec,
 * resets the grid cursor to slot (0,0), refreshes unlock flags, and kicks
 * off the async load of the overview page's resource group.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_TaskReinstall_8c014b3e', 4);
        $this->setSize('_CourseMenuFreeResourceGroup_8c0185c4', 4);
        $this->setSize('_njGarbageTexture', 4);
        $this->setSize('_var_tex_8c157af8', 4);
        $this->setSize('_AsqInitQueues_8c011f36', 4);
        $this->setSize('_AsqResetQueues_8c011f6c', 4);
        $this->setSize('_CourseMenuRequestSysResgrp_8c018568', 4);
        $this->setSize('_RouteSetLatch_8c014330', 4);
        $this->setSize('_AsqProcessQueues_8c011fe0', 4);
    }

    private function menu(int $off): int
    {
        return $this->addressOf('_var_menuState_8c1bc7a8') + $off;
    }

    private function floatToUint32(float $value): int
    {
        return unpack('L', pack('f', $value))[1];
    }

    public function test_installs_task_and_requests_overview_page(): void
    {
        $this->resolveSymbols();
        $task = $this->alloc(0x20);

        $this->call('_ProfileFileEnter_8c01d1c4')->with($task);
        $this->shouldCall('_TaskReinstall_8c014b3e')->with(
            $task, $this->addressOf('_menuTask_8c01ccec')
        );
        $this->shouldWriteLong($this->menu(0x18), 0);
        $this->shouldWriteLong($this->menu(0x3c), 0);
        $this->shouldWriteLong($this->menu(0x40), 0);
        $this->shouldWriteLong($this->menu(0x20), $this->floatToUint32(96.0));
        $this->shouldWriteLong($this->menu(0x24), $this->floatToUint32(128.0));

        $this->shouldCall('_ProfileFileUpdateUnlocks_8c01c980');

        $this->shouldCall('_CourseMenuFreeResourceGroup_8c0185c4')->with(
            $this->menu(0x00)
        );
        $this->shouldCall('_njGarbageTexture')->with(
            $this->addressOf('_var_tex_8c157af8'), 0xc00
        );
        $this->shouldCall('_AsqInitQueues_8c011f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_8c011f6c');
        $this->shouldCall('_CourseMenuRequestSysResgrp_8c018568')->with(
            $this->menu(0x0c), $this->addressOf('_init_profileResgrps_8c0450d8')
        );
        $this->shouldCall('_RouteSetLatch_8c014330');
        $this->shouldCall('_AsqProcessQueues_8c011fe0')->with(
            $this->addressOf('_AsqNop_8c011120'),
            0, 0, 0,
            $this->addressOf('_RouteClearLatch_8c014322'),
        );
    }
};
