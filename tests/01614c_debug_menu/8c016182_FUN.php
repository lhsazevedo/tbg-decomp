<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_full_teardown_frees_every_owned_resource()
    {
        $this->setSize('_syFree', 4);
        $this->setSize('_var_currentCourse_8c1bb868', 0x50);
        $cc = $this->addressOf('_var_currentCourse_8c1bb868');
        $demoBuf = $this->addressOf('_var_demoBuf_8c1ba3c4');
        $var454 = $this->addressOf('_var_8c1bc454');

        $this->initUint32($this->addressOf('_var_vibport_8c1ba354'), 5);
        for ($i = 0; $i < 19; $i++) {
            $this->initUint32($cc + 4 + 4 * $i, 0x2000 + $i);
        }
        $this->initUint32($demoBuf, 0x3000);
        $this->initUint32($var454, 0x4000);

        $this->call('_FUN_8c016182');

        $this->shouldCall('_FUN_8c010c7c');
        $this->shouldCall('_sdMidiStopAll');
        $this->shouldCall('_pdVibMxStop')->with(5);
        $this->shouldCall('_VibClear_8c010fbe');
        $this->shouldCall('_FUN_8c01614c');
        $this->shouldCall('_TaskFreeGroup_8c014ab4')->with($this->addressOf('_var_tasks_8c1ba3c8'));
        $this->shouldCall('_FUN_8c02adee');
        $this->shouldCall('_FUN_8c029cfe');
        $this->shouldCall('_RouteLoadFreePedestrianAssets_8c013ee4');
        $this->shouldCall('_RouteLoadFreeAllRouteModels_8c013dae');
        $this->shouldCall('_AsqFreeModels_8c0120fe')->with($this->addressOf('_var_routeModels_8c1bc3ec'));
        $this->shouldCall('_AsqFreeModels_8c0120fe')->with($this->addressOf('_var_segmentModels_8c1bc3f0'));
        $this->shouldCall('_AsqFreeModels_8c0120fe')->with($this->addressOf('_var_trafficModels_8c1bc3f4'));
        $this->shouldCall('_FUN_8c021724');
        $this->shouldCall('_RouteLoadFreeVehicleAssets_8c013b5a');

        // slot 2 is not freed here
        for ($i = 0; $i < 19; $i++) {
            if ($i == 2) continue;
            $this->shouldCall('_syFree')->with(0x2000 + $i);
        }
        $this->shouldWriteLong($cc + 4, -1);

        $this->shouldCall('_syFree')->with(0x3000);
        $this->shouldWriteLong($demoBuf, -1);
        $this->shouldCall('_syFree')->with(0x4000);
        $this->shouldWriteLong($var454, -1);

        $this->shouldCall('_RgFreeResourceGroups_8c016108');
        $this->shouldCall('_FUN_8c0187d0');
        $this->shouldCall('_VmMenuFreeAndClear_8c019504');
    }

    public function test_skips_frees_when_handles_already_released()
    {
        $this->setSize('_syFree', 4);
        $this->setSize('_var_currentCourse_8c1bb868', 0x50);
        $cc = $this->addressOf('_var_currentCourse_8c1bb868');

        $this->initUint32($this->addressOf('_var_vibport_8c1ba354'), -1);
        $this->initUint32($cc + 4, -1);
        $this->initUint32($this->addressOf('_var_demoBuf_8c1ba3c4'), -1);
        $this->initUint32($this->addressOf('_var_8c1bc454'), -1);

        $this->call('_FUN_8c016182');

        $this->shouldCall('_FUN_8c010c7c');
        $this->shouldCall('_sdMidiStopAll');
        $this->shouldCall('_VibClear_8c010fbe');
        $this->shouldCall('_FUN_8c01614c');
        $this->shouldCall('_TaskFreeGroup_8c014ab4')->with($this->addressOf('_var_tasks_8c1ba3c8'));
        $this->shouldCall('_FUN_8c02adee');
        $this->shouldCall('_FUN_8c029cfe');
        $this->shouldCall('_RouteLoadFreePedestrianAssets_8c013ee4');
        $this->shouldCall('_RouteLoadFreeAllRouteModels_8c013dae');
        $this->shouldCall('_AsqFreeModels_8c0120fe')->with($this->addressOf('_var_routeModels_8c1bc3ec'));
        $this->shouldCall('_AsqFreeModels_8c0120fe')->with($this->addressOf('_var_segmentModels_8c1bc3f0'));
        $this->shouldCall('_AsqFreeModels_8c0120fe')->with($this->addressOf('_var_trafficModels_8c1bc3f4'));
        $this->shouldCall('_FUN_8c021724');
        $this->shouldCall('_RouteLoadFreeVehicleAssets_8c013b5a');
        $this->shouldCall('_RgFreeResourceGroups_8c016108');
        $this->shouldCall('_FUN_8c0187d0');
        $this->shouldCall('_VmMenuFreeAndClear_8c019504');
    }
};
