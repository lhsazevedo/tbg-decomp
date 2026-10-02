<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// VM-GAME entry point: installs the state machine task and kicks off asset loads.
return new class extends TestCase {
    public function test_installs_and_requests()
    {
        $this->resolveSymbols();
        $task = $this->alloc(0x40);
        $ms = $this->addressOf('_var_menuState_8c1bc7a8');

        $this->call('_VmGameEnter_8c01c880')->with($task);

        $this->shouldCall('_TaskSwitch_8c014b3e')
            ->with($task, $this->addressOf('_vmGameTask_8c01bfec'));
        $this->shouldWriteLong($ms + 0x18, 0); // state_0x18
        $this->shouldWriteLong($ms + 0x38, 0); // selected_0x38
        $this->shouldCall('_AsqInitQueues_8c011f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_8c011f6c');
        $this->shouldCall('_CourseMenuRequestSysResgrp_8c018568')
            ->with($this->addressOf('_var_resourceGroup_8c2263a8'),
                   $this->addressOf('_init_vmGameResgrp_8c044e90'))
            ->andReturn(0);
        $this->shouldCall('_AsqRequestDat_8c011182')
            ->with('\\SYSTEM', 'PDAQUIZ.bin', $this->addressOf('_var_vmGameBuf_8c1bc454'))
            ->andReturn(0);
        $this->shouldCall('_RouteSetLatch_8c014330');
        $this->shouldCall('_AsqProcessQueues_8c011fe0')
            ->with($this->addressOf('_AsqNop_8c011120'), 0, 0, 0,
                   $this->addressOf('_RouteClearLatch_8c014322'));
        $this->shouldCall('_MessageBoxSetText_8c02aefc')
            ->with("")
            ->andReturn(0);
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x7c);
        $this->setSize('_TaskSwitch_8c014b3e', 4);
        $this->setSize('_AsqInitQueues_8c011f36', 4);
        $this->setSize('_AsqResetQueues_8c011f6c', 4);
        $this->setSize('_CourseMenuRequestSysResgrp_8c018568', 4);
        $this->setSize('_AsqRequestDat_8c011182', 4);
        $this->setSize('_RouteSetLatch_8c014330', 4);
        $this->setSize('_AsqProcessQueues_8c011fe0', 4);
        $this->setSize('_AsqNop_8c011120', 4);
        $this->setSize('_RouteClearLatch_8c014322', 4);
        $this->setSize('_MessageBoxSetText_8c02aefc', 4);
    }
};
