<?php declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_pushes_results_task_and_requests_resources(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x6c);

        $this->call('_installResults_8c01df8e');

        $this->shouldCall('_InputInstall_8c0128cc')->with(0);

        // &installed_task and &installed_state are installResults_8c01df8e's
        // own stack locals; both TaskInstall calls share the same slots, and
        // both objects place them the same.
        $this->shouldCall('_TaskInstall_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            $this->addressOf('_GameSoftResetTask_8c012f44'),
            0xfffff0,
            0xfffff4,
            0,
        );
        $this->shouldCall('_TaskInstall_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba3c8'),
            $this->addressOf('_resultsTask_8c01d8e0'),
            0xfffff0,
            0xfffff4,
            0,
        );

        $this->shouldWriteLong($this->addressOf('_var_menuState_8c1bc7a8') + 0x18, 0);

        $this->shouldCall('_njGarbageTexture')->with(
            $this->addressOf('_var_tex_8c157af8'),
            0xc00,
        );

        $this->shouldCall('_MessageBoxOpenTextbox_8c02ae3e')->with(0x20, 0x180, -2.0, 0x240, 0x40, 0, 0, -1);

        $this->shouldCall('_AsqInitQueues_8c011f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_8c011f6c');

        $this->shouldCall('_CourseMenuRequestSysResgrp_8c018568')->with(
            $this->addressOf('_var_menuState_8c1bc7a8') + 0x0c,
            $this->addressOf('_init_titleResourceGroup_8c044254'),
        );

        $this->shouldCall('_CourseMenuRequestCommonResources_8c01852c');

        $this->shouldCall('_AsqRequestDat_8c011182')->with(
            '\\SYSTEM',
            'bus_mem.VMI',
            $this->addressOf('_var_vmuIconFileBuf_8c1ba344'),
        );

        $this->shouldCall('_RouteSetLatch_8c014330');

        $this->shouldCall('_AsqProcessQueues_8c011fe0')->with(
            $this->addressOf('_AsqNop_8c011120'),
            0,
            0,
            0,
            $this->addressOf('_RouteClearLatch_8c014322'),
        );
    }
};
