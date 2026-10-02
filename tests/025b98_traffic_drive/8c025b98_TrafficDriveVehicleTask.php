<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('f32')) {
    function f32(float $value): float {
        return unpack('f', pack('f', $value))[1];
    }
}
if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

// TrafficDriveVehicleTask_8c025b98: TaskAction for a moving CPU vehicle.
// See the function's header comment in 025b98_traffic_drive.c for the full
// driveState breakdown.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_njSqrt', 4);
        $this->setSize('_TaskUninstall_8c014b66', 4);
        $this->setSize('_TrafficUpdateHeading_8c026bc4', 4);
        $this->setSize('_CollisionFindTaskHit_8c02e400', 4);
        $this->setSize('_BusDrawPlaceEntity_8c027c3c', 4);
        $this->setSize('_GeomDistanceXZ_8c02081c', 4);
        $this->setSize('_TrafficLookaheadScan_8c02dfca', 4);
        $this->setSize('_TrafficPathScanFirst_8c02f0c8', 4);
        $this->setSize('_BusDrawFadeLights_8c028022', 4);
        $this->setSize('_TrafficAdvanceOnPath_8c026ca2', 4);
        $this->setSize('_TrafficRunEntryScript_8c027012', 4);
        $this->setSize('_TrafficResolvePathBlocks_8c026710', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_var_activeTrafficPreset_8c227e14', 4);
        $this->setSize('_var_timeOfDay_8c18ad20', 4);
        $this->setSize('_var_playerBus_8c1bbd9c', 4);
        $this->setSize('_var_driveCueState_8c2264b8', 0x1c);

        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x0f4), fdec(0.0));
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x0fc), fdec(0.0));
        $this->initUint32($this->addressOf('_var_activeTrafficPreset_8c227e14'), 5);
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0); // TIME_OF_DAY_DAY
        $this->initUint32($this->addressOf('_var_playerBus_8c1bbd9c'), 0); // no player bus sentinel match
    }

    // Zeroed 0x514-byte entry with every field this function reads
    // initialized to a harmless default; each test overrides only the
    // fields relevant to its path.
    private function allocEntry(): int {
        $entry = $this->alloc(0x514);
        $this->doNotRandomizeMemory();
        return $entry;
    }

    // ---- driveState == 1 (knockback push) ----

    // Clear probes, no collision: applies the push and keeps knockback
    // running (speed stays > 0), so the driveState==2 finalize block does
    // not run. heading = dx (dirX * old speed), matching
    // TrafficDriveParkedTask_8c02656a's leftover convention.
    public function test_knockbackClearPathKeepsPushing(): void {
        $this->resolveSymbols();
        $task = $this->alloc(4);
        $entry = $this->allocEntry();
        $this->initUint32($entry + 0x2b4, 1); // driveState = knockback
        $this->initUint32($entry + 0x27c, fdec(2.0)); // speed
        $this->initUint32($entry + 0x190, 1); // probe0.attr
        $this->initUint32($entry + 0x1a0, 1); // probe1.attr
        $this->initUint32($entry + 0x1b0, 1); // probe2.attr
        $this->initUint32($entry + 0x29c, fdec(1.0)); // dirX
        $this->initUint32($entry + 0x2a0, fdec(0.5)); // dirZ
        $this->initUint32($entry + 0xf4, fdec(10.0)); // posX
        $this->initUint32($entry + 0xfc, fdec(0.0));  // posZ
        $this->initUint32($entry + 0x100, fdec(12.0)); // frontPointX_0x100
        $this->initUint32($entry + 0x108, fdec(0.0));  // frontPointZ_0x108

        $this->call('_TrafficDriveVehicleTask_8c025b98')->with($task, $entry);

        $this->shouldWriteLong($entry + 0x080, 0); // cleared unconditionally at top

        $this->shouldCall('_CollisionFindTaskHit_8c02e400')->with($task, $entry)->andReturn(0);

        // dx = 2.0*1.0 = 2.0, dz = 2.0*0.5 = 1.0
        $this->shouldWriteFloat($entry + 0xf4, 12.0);
        $this->shouldWriteFloat($entry + 0xfc, 1.0);
        $this->shouldWriteFloat($entry + 0x100, 14.0);
        $this->shouldWriteFloat($entry + 0x108, 1.0);
        // speed -= 0.1 => 1.9, still > 0: no driveState/projectDistance/probe reset here.

        $this->shouldCall('_TrafficUpdateHeading_8c026bc4')->with(2.0, $entry);

        // shared tail: ring buffer (all zero) + speed diff (1.9 - 2.0)
        $this->shouldWriteFloat($entry + 0x280, 0.0);
        $this->shouldWriteFloat($entry + 0x284, 0.0);
        $this->shouldWriteFloat($entry + 0x288, 0.0);
        $this->shouldWriteFloat($entry + 0x28c, 0.0); // slot 3 re-stored unchanged
        $this->shouldWriteLong($entry + 0x080, 0); // |= extraLightFlags_0x510 (0); speed != 0 so no |=1
        $this->shouldWriteFloat($entry + 0x27c, f32(1.9));

        $this->shouldCall('_njSqrt')->with(f32(12.0 * 12.0 + 1.0 * 1.0))->andReturn(12.04);
        $this->shouldWriteFloat($entry + 0x490, f32(12.04));
        $this->shouldWriteLong($entry + 0x268, 0);
        $this->shouldCall('_BusDrawPlaceEntity_8c027c3c')->with($entry, -0.1);
    }

    // Blocked probe: the push contributes nothing (speed forced to 0),
    // which immediately ends the knockback: ground-snaps projectDistance,
    // reverts to driveState 2 and clears the 3 probe counts.
    public function test_knockbackBlockedProbeEndsPush(): void {
        $this->resolveSymbols();
        $task = $this->alloc(4);
        $entry = $this->allocEntry();
        $this->initUint32($entry + 0x2b4, 1);
        $this->initUint32($entry + 0x27c, fdec(2.0));
        $this->initUint32($entry + 0x190, 1);
        $this->initUint32($entry + 0x1a0, 0); // closed
        $this->initUint32($entry + 0x1b0, 1);

        $this->call('_TrafficDriveVehicleTask_8c025b98')->with($task, $entry);

        $this->shouldWriteLong($entry + 0x080, 0); // cleared unconditionally at top

        $this->shouldWriteLong($entry + 0x2b4, 2);
        $this->shouldCall('_GeomDistanceXZ_8c02081c')->with($entry + 0xf4, $entry + 0xec)->andReturn(3.5);
        $this->shouldWriteFloat($entry + 0x2c4, 3.5);
        $this->shouldWriteLong($entry + 0x19c, 0);
        $this->shouldWriteLong($entry + 0x1ac, 0);
        $this->shouldWriteLong($entry + 0x1bc, 0);

        // speed forced to 0.0 this frame -> TrafficUpdateHeading is skipped
        // (heading's uninitialized-register value is functionally unused
        // by the callee, and untestable, so this path avoids triggering
        // the call entirely).
        $this->shouldWriteFloat($entry + 0x280, 0.0);
        $this->shouldWriteFloat($entry + 0x284, 0.0);
        $this->shouldWriteFloat($entry + 0x288, 0.0);
        $this->shouldWriteFloat($entry + 0x28c, 0.0); // slot 3 re-stored unchanged
        $this->shouldWriteLong($entry + 0x080, 1); // speed == 0 -> |= 1
        $this->shouldWriteLong($entry + 0x080, 1); // |= extraLightFlags_0x510 (0), unchanged
        $this->shouldWriteFloat($entry + 0x27c, 0.0);

        $this->shouldCall('_njSqrt')->andReturn(10.0);
        $this->shouldWriteFloat($entry + 0x490, 10.0);
        $this->shouldWriteLong($entry + 0x268, 0);
        $this->shouldCall('_BusDrawPlaceEntity_8c027c3c');
    }

    // ---- driveState == 3 (waiting for the reloaded script's spawn box) ----

    public function test_reloadWaitBlockedReturnsWithoutRunningTail(): void {
        $this->resolveSymbols();
        $task = $this->alloc(4);
        $entry = $this->allocEntry();
        $this->initUint32($entry + 0x2b4, 3);
        $this->initUint32($entry + 0x304, 0x1234); // pathBlocks_0x304[0]

        $this->call('_TrafficDriveVehicleTask_8c025b98')->with($task, $entry);

        $this->shouldWriteLong($entry + 0x080, 0); // cleared unconditionally at top

        $this->shouldCall('_TrafficPathScanFirst_8c02f0c8')
            ->with($task, $entry, 0x1234, 0, 0.0, 8.0)
            ->andReturn(0xabc);
        // blocked: returns immediately, no tail bookkeeping at all.
    }

    public function test_reloadWaitClearRunsScriptAndReturns(): void {
        $this->resolveSymbols();
        $task = $this->alloc(4);
        $entry = $this->allocEntry();
        $this->initUint32($entry + 0x2b4, 3);
        $this->initUint32($entry + 0x304, 0x1234);

        $this->call('_TrafficDriveVehicleTask_8c025b98')->with($task, $entry);

        $this->shouldWriteLong($entry + 0x080, 0); // cleared unconditionally at top

        $this->shouldCall('_TrafficPathScanFirst_8c02f0c8')
            ->with($task, $entry, 0x1234, 0, 0.0, 8.0)
            ->andReturn(0);
        $this->shouldCall('_TrafficRunEntryScript_8c027012')->with($entry)->andReturn(1);
        // returns right after, no tail bookkeeping.
    }

    // ---- driveState == 0 (normal driving) ----

    // speed is nonzero (so the junction lookup runs its normal call path
    // rather than the speed==0 uninitialized-signalId quirk -- see
    // test_unknownDriveStateSkipsToTail for that one), and every speed
    // constraint is slack enough that integrated speed still lands back at
    // the same value: exercises the junction query, the always-on obstacle
    // scan, all the light/yield sub-state machines left idle, the min-speed
    // selection, and the position/path-distance integration of driveState 0.
    // TrafficRunEntryScript_8c027012 reports "nothing left to do" and this
    // isn't the demo-reload case, so the frame ends by unconditionally
    // freeing the task -- real asm behavior, skipping the shared ring-
    // buffer/blinker/render tail entirely (see the function's header
    // comment). That tail is exercised instead by the driveState==1 and
    // unknown-driveState tests below, whose paths don't run into this
    // function's two genuinely-uninitialized-register reads (this
    // function's own is the speed==0 junction-skip case).
    public function test_drivingScriptExhaustedFreesTask(): void {
        $this->resolveSymbols();
        $this->setSize('_AttrQueryFindConvexPolygon_8c02e51c', 4);
        $task = $this->alloc(4);
        $entry = $this->allocEntry();
        $this->initUint32($entry + 0x2b4, 0); // driveState
        $this->initUint32($entry + 0x27c, fdec(1.0)); // speed
        $this->initUint32($entry + 0x2cc, $this->addressOf('_AttrQueryFindConvexPolygon_8c02e51c'));
        $this->initUint32($entry + 0xf4, fdec(1.0));  // posX
        $this->initUint32($entry + 0xf8, fdec(2.0));  // posY
        $this->initUint32($entry + 0xfc, fdec(3.0));  // posZ
        $this->initUint32($entry + 0x41c, fdec(1.0)); // lookahead base
        $this->initUint32($entry + 0x414, fdec(1.0)); // laneLimit input
        $this->initUint32($entry + 0x418, fdec(1.0)); // laneLimit input
        $this->initUint32($entry + 0x290, fdec(1.0)); // accel rate (unused: already at laneLimit)
        $this->initUint32($entry + 0x2f4, 9); // preset != active (not the demo-reload case)
        $this->initUint32($entry + 0x48c, 0); // != 3 (not the demo-reload case)

        $this->call('_TrafficDriveVehicleTask_8c025b98')->with($task, $entry);

        $this->shouldWriteLong($entry + 0x080, 0); // cleared unconditionally at top

        $this->shouldCall('_AttrQueryFindConvexPolygon_8c02e51c')->with(1.0, 2.0, 3.0, $entry + 0x404)->andReturn(0);
        $this->shouldWriteLong($entry + 0x2dc, 0);
        $this->shouldWriteLong($entry + 0x410, -1); // unaff_r8 = -1 on a miss
        $this->shouldWriteLong($entry + 0x50c, 0);

        $this->shouldCall('_TrafficLookaheadScan_8c02dfca')->with($task, $entry, f32(1.0 * 36.0 + 1.0))->andReturn(0);
        $this->shouldWriteLong($entry + 0x424, 0);
        $this->shouldWriteLong($entry + 0x2d4, 0);

        $this->shouldWriteLong($entry + 0x428, 0);
        $this->shouldWriteFloat($entry + 0x418, 9999.0);

        // speed(1.0) == laneLimit(1.0): path advance/script run.
        $this->shouldWriteFloat($entry + 0x2bc, 1.0); // pathDistance += speed
        $this->shouldWriteFloat($entry + 0x2c0, 1.0); // pathDistanceCopy += speed
        $this->shouldWriteFloat($entry + 0x4ec, -1.0); // lookaheadCacheLen_0x4ec -= speed

        $this->shouldCall('_TrafficAdvanceOnPath_8c026ca2')->andReturn(0);
        $this->shouldCall('_TrafficRunEntryScript_8c027012')->with($entry)->andReturn(0);
        $this->shouldCall('_TaskUninstall_8c014b66')->with($task);
        // returns immediately: no ring-buffer/blinker/render tail at all.
    }

    // ---- driveState not in {0,1,2,3} (else/default) ----

    // Falls straight to the shared tail without running any driveState
    // logic at all: speed_0x27c is read once at entry and passed through
    // unchanged. Kept at 0 here so the (functionally unused, and in this
    // path genuinely uninitialized in the original) heading argument to
    // TrafficUpdateHeading_8c026bc4 never has to be asserted.
    public function test_unknownDriveStateSkipsToTail(): void {
        $this->resolveSymbols();
        $task = $this->alloc(4);
        $entry = $this->allocEntry();
        $this->initUint32($entry + 0x2b4, 9); // not 0/1/2/3
        $this->initUint32($entry + 0x27c, fdec(0.0));

        $this->call('_TrafficDriveVehicleTask_8c025b98')->with($task, $entry);

        $this->shouldWriteLong($entry + 0x080, 0); // cleared unconditionally at top

        $this->shouldWriteFloat($entry + 0x280, 0.0);
        $this->shouldWriteFloat($entry + 0x284, 0.0);
        $this->shouldWriteFloat($entry + 0x288, 0.0);
        $this->shouldWriteFloat($entry + 0x28c, 0.0); // slot 3 re-stored unchanged
        $this->shouldWriteLong($entry + 0x080, 1); // speed == 0 -> |= 1
        $this->shouldWriteLong($entry + 0x080, 1); // |= extraLightFlags_0x510 (0), unchanged
        $this->shouldWriteFloat($entry + 0x27c, 0.0);

        $this->shouldCall('_njSqrt')->andReturn(0.0);
        $this->shouldWriteFloat($entry + 0x490, 0.0);
        $this->shouldWriteLong($entry + 0x268, 0);
        $this->shouldCall('_BusDrawPlaceEntity_8c027c3c')->with($entry, 0.0);
    }
};
