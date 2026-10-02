<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

// spawnEntry_8c0272b8 spawns one traffic entry for a script's
// opcode-0 header word (typeCode), script pointer, and route progress. See
// the function's header comment in 026710_traffic.c for the full field/
// branch breakdown.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_TaskInstall_8c014ae8', 4);
        $this->setSize('_TaskUninstall_8c014b66', 4);
        $this->setSize('_AsqGetRandomA_8c012166', 4);
        $this->setSize('_VehiclePartsBind_8c02786c', 4);
        $this->setSize('_TrafficPathScanFirst_8c02f0c8', 4);
        $this->setSize('_GroundProbeTrackPolygon_8c020b6c', 4);
        $this->setSize('_AttrQueryFindConvexPolygon_8c02e51c', 4);
        $this->setSize('_GroundProbeTrackPolygonAtHeight_8c021290', 4);
        $this->setSize('_AttrQueryFindConvexPolygonAtHeight_8c02eab4', 4);
        $this->setSize('_TrafficDriveVehicleTask_8c025b98', 4);
        $this->setSize('_TrafficDriveParkedTask_8c02656a', 4);
        $this->setSize('_var_tasks_8c1bac28', 4);
        $this->setSize('_var_routeModelSlots_8c1bbddc', 0x20 * 0x10);
        $this->setSize('_var_trafficModels_8c1bc3f4', 4);
        $this->setSize('_var_progress_8c1ba1cc', 0xe8);
        $this->setSize('_var_route_8c18ad1c', 4);
        $this->setSize('_var_timeOfDay_8c18ad20', 4);
    }

    // ModelSlot is {requested, needsLoad, texlist, njDest}, 0x10 bytes/slot.
    private function setModelSlot(int $slot, int $texlist, int $njDest): void {
        $base = $this->addressOf('_var_routeModelSlots_8c1bbddc') + $slot * 0x10;
        $this->initUint32($base + 0x08, $texlist);
        $this->initUint32($base + 0x0c, $njDest);
    }

    // LoadedModel is {texlist, njDest}, 8 bytes/record.
    private function setTrafficModel(int $bodyType, int $njDest): int {
        $table = $this->alloc(11 * 8);
        $this->initUint32($this->addressOf('_var_trafficModels_8c1bc3f4'), $table);
        $this->initUint32($table + $bodyType * 8 + 4, $njDest);
        return $table;
    }

    private function isAsmObject(): bool {
        return str_contains($this->objectFile, '/asm/');
    }

    // &task and &entryVoid are spawnEntry_8c0272b8's own stack locals; both
    // objects happen to place &task at the same slot, but &entryVoid differs.
    private function mockTaskInstall(int $action, int $task, int $entry): void {
        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1bac28'),
                $action,
                0xffffcc,
                $this->isAsmObject() ? 0xffffc8 : 0xffffd0,
                0x514,
            )
            ->do(function () use ($task, $entry) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($task));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($entry));
            })
            ->andReturn(1);
    }

    // typeCode 0x1c's day mask (per-route/time-of-day bit in init_dayMasks_8c046208)
    // is clear for (route=SHINJUKU, timeOfDay=DAY) regardless of the day
    // count -- the entry is skipped entirely: no TaskInstall, still returns 1.
    public function test_dayGateBlocksSpawn(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 1); // days_0x00
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 0);     // ROUTE_SHINJUKU
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0); // TIME_OF_DAY_DAY

        $script = $this->alloc(2);
        $this->initUint16($script, 10);

        $this->call('_spawnEntry_8c0272b8')->with(0x1c, 1.5, $script);

        $this->shouldReturn(1);
    }

    // Same typeCode, but (route=SHINJUKU, timeOfDay=EVENING) has bit 15 set
    // in init_dayMasks_8c046208, and days_0x00=16 sets that same bit in the day mask
    // -- the day gate passes and the entry is spawned as a parked vehicle
    // (script header word == 10): TrafficDriveParkedTask_8c02656a's task action, entry+0x2e4=1,
    // and the moving-vehicle-only block (TrafficResolvePathBlocks_8c026710,
    // TrafficPathScanFirst_8c02f0c8, entry+0x2e8/0x2ec/0x2f0, TrafficAdvanceOnPath_8c026ca2)
    // is skipped entirely.
    public function test_dayGateAllowsSpawn_parked(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_progress_8c1ba1cc'), 16); // days_0x00
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 0);     // ROUTE_SHINJUKU
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 1); // TIME_OF_DAY_EVENING

        $task = $this->alloc(0x20);
        $entry = $this->alloc(0x520);
        $this->setModelSlot(0x1c, 0x11110000, 0x22220000);
        $this->setModelSlot(0x1d, 0x33330000, 0x44440000);
        $trafficModelTable = $this->setTrafficModel(0x09, 0x55550000); // init_variantBodyType_8c04622c[0xe]==9
        $this->initUint32($this->addressOf('_var_activeTrafficPreset_8c227e14'), 0x77);

        $script = $this->alloc(2);
        $this->initUint16($script, 10);

        $this->call('_spawnEntry_8c0272b8')->with(0x1c, 1.5, $script);

        $this->mockTaskInstall($this->addressOf('_TrafficDriveParkedTask_8c02656a'), $task, $entry);
        $this->shouldWriteLong($entry + 0x2e4, 1);
        $this->shouldWriteLong($entry + 0x48c, 3); // 0x8000 clear
        $this->shouldWriteLong($entry + 0x2c8, $this->addressOf('_GroundProbeTrackPolygon_8c020b6c'));
        $this->shouldWriteLong($entry + 0x2cc, $this->addressOf('_AttrQueryFindConvexPolygon_8c02e51c'));
        $this->shouldWriteLong($entry + 0x2e0, 0xe); // 0x1c >> 1

        $this->shouldWriteLong($entry + 0x510, 0);

        $this->shouldWriteLong($entry + 0x2f4, 0x77);
        $this->shouldWriteLong($entry + 0x2f8, $script);
        $this->shouldWriteLong($entry + 0x2fc, $script);

        $this->shouldWriteLong($entry + 4, 0x11110000);
        $this->shouldWriteLong($entry + 0xc, 0x22220000);
        $this->shouldWriteLong($entry + 8, 0x33330000);
        $this->shouldWriteLong($entry + 0x10, 0x44440000);

        $this->shouldWriteLong($entry + 0x14, 0x55550000);

        $this->shouldCall('_VehiclePartsBind_8c02786c')->with($entry, 0x1c);
        $this->shouldCall('_TrafficRunEntryScript_8c027012')->with($entry)->andReturn(1);
        $this->shouldCall('_TrafficUpdateHeading_8c026bc4')->with(1.5, $entry);

        $this->shouldReturn(1);
    }

    // A typeCode outside the 0x1c/0x1e day-gated range spawns a moving
    // vehicle (*script != 10): TaskInstall uses TrafficDriveVehicleTask_8c025b98 (entry+0x2e4=0),
    // and the full moving-vehicle setup runs, including the taxi/tour-bus
    // random flag (typeCode 0x14, odd random -> entry+0x510 |= 0x40).
    public function test_movingVehicleFullPath(): void {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $entry = $this->alloc(0x520);
        $this->setModelSlot(0x14, 0x66660000, 0x77770000);
        $this->setModelSlot(0x15, 0x88880000, 0x99990000);
        $this->setTrafficModel(0x06, 0xaaaa0000); // init_variantBodyType_8c04622c[0xa]==6
        $this->initUint32($this->addressOf('_var_activeTrafficPreset_8c227e14'), 0x88);

        $script = $this->alloc(2);
        $this->initUint16($script, 5); // anything other than 10
        $this->initUint32($entry + 0x304, 0x12345678); // first resolved script arg

        $this->call('_spawnEntry_8c0272b8')->with(0x14, 2.5, $script);

        $this->mockTaskInstall($this->addressOf('_TrafficDriveVehicleTask_8c025b98'), $task, $entry);
        $this->shouldWriteLong($entry + 0x2e4, 0);
        $this->shouldWriteLong($entry + 0x48c, 3);
        $this->shouldWriteLong($entry + 0x2c8, $this->addressOf('_GroundProbeTrackPolygon_8c020b6c'));
        $this->shouldWriteLong($entry + 0x2cc, $this->addressOf('_AttrQueryFindConvexPolygon_8c02e51c'));
        $this->shouldWriteLong($entry + 0x2e0, 0xa); // 0x14 >> 1

        $this->shouldWriteLong($entry + 0x510, 0);
        $this->shouldCall('_AsqGetRandomA_8c012166')->andReturn(1); // odd -> flag set
        $this->shouldWriteLong($entry + 0x510, 0x40);

        $this->shouldCall('_TrafficResolvePathBlocks_8c026710')->with($entry, $script);
        $this->shouldCall('_TrafficPathScanFirst_8c02f0c8')->with($task, $entry, 0x12345678, 0, 2.5, 8.0)->andReturn(0);

        $this->shouldWriteFloat($entry + 0x2e8, 2.5);
        $this->shouldCall('_AsqGetRandomA_8c012166')->andReturn(0);
        $this->shouldWriteFloat($entry + 0x2ec, -0.5); // 0/65536 - 0.5
        $this->shouldCall('_AsqGetRandomA_8c012166')->andReturn(32768);
        $this->shouldWriteFloat($entry + 0x2f0, 0.0); // 32768/65536 - 0.5

        $this->shouldWriteLong($entry + 0x2f4, 0x88);
        $this->shouldWriteLong($entry + 0x2f8, $script);
        $this->shouldWriteLong($entry + 0x2fc, $script);

        $this->shouldWriteLong($entry + 4, 0x66660000);
        $this->shouldWriteLong($entry + 0xc, 0x77770000);
        $this->shouldWriteLong($entry + 8, 0x88880000);
        $this->shouldWriteLong($entry + 0x10, 0x99990000);

        $this->shouldWriteLong($entry + 0x14, 0xaaaa0000);

        $this->shouldCall('_VehiclePartsBind_8c02786c')->with($entry, 0x14);
        $this->shouldCall('_TrafficRunEntryScript_8c027012')->with($entry)->andReturn(1);
        $this->shouldCall('_TrafficAdvanceOnPath_8c026ca2')->with(2.5, $entry)->andReturn(1);
        $this->shouldCall('_TrafficUpdateHeading_8c026bc4')->with(2.5, $entry);

        $this->shouldReturn(1);
    }

    // TaskInstall failing (no task allocated) aborts immediately: no further
    // field writes or calls, and nothing to free.
    public function test_taskInstallFailureReturnsZero(): void {
        $this->resolveSymbols();

        $script = $this->alloc(2);
        $this->initUint16($script, 5);

        $this->call('_spawnEntry_8c0272b8')->with(0x02, 1.0, $script);

        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1bac28'),
                $this->addressOf('_TrafficDriveVehicleTask_8c025b98'),
                0xffffcc,
                $this->isAsmObject() ? 0xffffc8 : 0xffffd0,
                0x514,
            )
            ->andReturn(0);

        $this->shouldReturn(0);
    }

    // An unloaded model slot (texlist_0x08 == -1) frees the just-allocated
    // task and skips the rest of the setup -- but per real asm behavior it
    // still reports success (return 1), not failure.
    public function test_unloadedModelSlotFreesTaskButStillReturnsOne(): void {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $entry = $this->alloc(0x520);
        $this->setModelSlot(0x02, -1, 0);

        $script = $this->alloc(2);
        $this->initUint16($script, 5);

        $this->call('_spawnEntry_8c0272b8')->with(0x02, 1.0, $script);

        $this->mockTaskInstall($this->addressOf('_TrafficDriveVehicleTask_8c025b98'), $task, $entry);
        $this->shouldWriteLong($entry + 0x2e4, 0);
        $this->shouldWriteLong($entry + 0x48c, 3);
        $this->shouldWriteLong($entry + 0x2c8, $this->addressOf('_GroundProbeTrackPolygon_8c020b6c'));
        $this->shouldWriteLong($entry + 0x2cc, $this->addressOf('_AttrQueryFindConvexPolygon_8c02e51c'));
        $this->shouldWriteLong($entry + 0x2e0, 0x01);

        $this->shouldCall('_TaskUninstall_8c014b66')->with($task);

        $this->shouldReturn(1);
    }

    // TrafficPathScanFirst_8c02f0c8 rejecting the entry (moving vehicle only) also frees the
    // task and returns 0, without ever reaching the field writes that follow
    // it.
    public function test_scriptRejectedByPlacementFreesTaskAndReturnsZero(): void {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $entry = $this->alloc(0x520);
        $this->setModelSlot(0x02, 0x66660000, 0x77770000);
        $this->setModelSlot(0x03, 0x88880000, 0x99990000);

        $script = $this->alloc(2);
        $this->initUint16($script, 5);
        $this->initUint32($entry + 0x304, 0xdeadbeef);

        $this->call('_spawnEntry_8c0272b8')->with(0x02, 1.0, $script);

        $this->mockTaskInstall($this->addressOf('_TrafficDriveVehicleTask_8c025b98'), $task, $entry);
        $this->shouldWriteLong($entry + 0x2e4, 0);
        $this->shouldWriteLong($entry + 0x48c, 3);
        $this->shouldWriteLong($entry + 0x2c8, $this->addressOf('_GroundProbeTrackPolygon_8c020b6c'));
        $this->shouldWriteLong($entry + 0x2cc, $this->addressOf('_AttrQueryFindConvexPolygon_8c02e51c'));
        $this->shouldWriteLong($entry + 0x2e0, 0x01);
        $this->shouldWriteLong($entry + 0x510, 0);

        $this->shouldCall('_TrafficResolvePathBlocks_8c026710')->with($entry, $script);
        $this->shouldCall('_TrafficPathScanFirst_8c02f0c8')->with($task, $entry, 0xdeadbeef, 0, 1.0, 8.0)->andReturn(1);
        $this->shouldCall('_TaskUninstall_8c014b66')->with($task);

        $this->shouldReturn(0);
    }
};
