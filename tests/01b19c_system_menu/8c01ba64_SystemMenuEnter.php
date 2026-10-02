<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_switches_into_save_load_flow(): void
    {
        $this->setSize('_init_saveNames_8c044d50', 0x2c); // char*[11]

        $task = 0x8ce00000;
        $menuState = $this->addressOf('_var_menuState_8c1bc7a8');
        $saveNames = $this->addressOf('_init_saveNames_8c044d50');

        $this->initUint32($saveNames + 2 * 4, 0x8c440000);
        $this->initUint32($this->addressOf('_var_saveSlot_8c1ba350'), 2);
        $this->initUint32($this->addressOf('_var_selectedVm_8c1ba34c'), 1);

        $this->call('_SystemMenuEnter_8c01ba64')->with($task);

        $this->shouldCall('_TaskSwitch_8c014b3e')
            ->with($task, $this->addressOf('_menuTask_8c01b3ac'));
        $this->shouldWriteLong($menuState + 0x18, 0);
        $this->shouldWriteLong($menuState + 0x38, 0);
        $this->shouldCall('_syMalloc')->with(0x600)->andReturn(0x8c500000);
        $this->shouldWriteLong($this->addressOf('_var_saveBuf_8c1ba2e0'), 0x8c500000);
        $this->shouldCall('_VmSelectUpdateStatus_8c01967c')->with(1, 0x8c440000, 3);
        $this->shouldCall('_MessageBoxSetText_8c02aefc')->with("");
        $this->shouldCall('_RenderStartFadeIn_8c022a9c')->with(10);
        $this->shouldCall('_AsqInitQueues_8c011f36')->with(8, 0, 0, 8);
        $this->shouldCall('_AsqResetQueues_8c011f6c');
        $this->shouldCall('_AsqRequestDat_8c011182')
            ->with("\\SYSTEM", "bus_mem.VMI", $this->addressOf('_var_vmuIconFileBuf_8c1ba344'));
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
