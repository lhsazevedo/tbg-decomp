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

// TrafficDriveParkedTask_8c02656a: TaskAction for a parked vehicle.
// See the function's header comment in 025b98_traffic_drive.c for the full
// branch breakdown.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_njSqrt', 4);
        $this->setSize('_TaskKill_8c014b66', 4);
        $this->setSize('_TrafficUpdateHeading_8c026bc4', 4);
        $this->setSize('_CollisionFindTaskHit_8c02e400', 4);
        $this->setSize('_BusDrawPlaceEntity_8c027c3c', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_var_activeTrafficPreset_8c227e14', 4);

        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x0f4), fdec(0.0));
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x0fc), fdec(0.0));
        $this->initUint32($this->addressOf('_var_activeTrafficPreset_8c227e14'), 5);
    }

    private function allocEntry(): int {
        $entry = $this->alloc(0x514);
        $this->initUint32($entry + 0x190, 1); // groundProbe[0].attr_0x00
        $this->initUint32($entry + 0x1a0, 1); // groundProbe[1].attr_0x00
        $this->initUint32($entry + 0x1b0, 1); // groundProbe[2].attr_0x00
        $this->initUint32($entry + 0x19c, 0); // groundProbe[0].count_0x0c
        $this->initUint32($entry + 0x1ac, 0); // groundProbe[1].count_0x0c
        $this->initUint32($entry + 0x1bc, 0); // groundProbe[2].count_0x0c
        $this->initUint32($entry + 0x29c, fdec(1.0)); // dirX
        $this->initUint32($entry + 0x2a0, fdec(0.5)); // dirZ
        $this->initUint32($entry + 0x2b4, 0); // knockbackActive
        $this->initUint32($entry + 0xf4, fdec(10.0));  // posX
        $this->initUint32($entry + 0xfc, fdec(0.0));   // posZ
        $this->initUint32($entry + 0x100, fdec(12.0)); // frontPointX_0x100
        $this->initUint32($entry + 0x108, fdec(0.0));  // frontPointZ_0x108
        $this->initUint32($entry + 0x27c, fdec(0.0));  // speed
        $this->initUint32($entry + 0x2f4, 5);          // preset id
        return $entry;
    }

    // knockbackActive == 0: skips the whole push block entirely, goes
    // straight to the distance check. Far (> 200) and mismatched preset ->
    // frees the task.
    public function test_inactiveFarMismatchedPresetFrees(): void {
        $this->resolveSymbols();
        $task = $this->alloc(4);
        $entry = $this->allocEntry();
        $this->initUint32($entry + 0xf4, fdec(1000.0));
        $this->initUint32($entry + 0x2f4, 9); // != activeTrafficPreset (5)

        $this->call('_TrafficDriveParkedTask_8c02656a')->with($task, $entry);

        $this->shouldCall('_njSqrt')->with(f32(1000.0 * 1000.0))->andReturn(1000.0);
        $this->shouldWriteFloat($entry + 0x490, 1000.0);
        $this->shouldCall('_TaskKill_8c014b66')->with($task);
    }

    // knockbackActive == 0, far but matching preset -> still draws.
    public function test_inactiveFarMatchedPresetDraws(): void {
        $this->resolveSymbols();
        $task = $this->alloc(4);
        $entry = $this->allocEntry();
        $this->initUint32($entry + 0xf4, fdec(1000.0));

        $this->call('_TrafficDriveParkedTask_8c02656a')->with($task, $entry);

        $this->shouldCall('_njSqrt')->with(f32(1000.0 * 1000.0))->andReturn(1000.0);
        $this->shouldWriteFloat($entry + 0x490, 1000.0);
        $this->shouldCall('_BusDrawPlaceEntity_8c027c3c')->with($entry, 0.0);
    }

    // knockbackActive == 0, close (< 200): always draws, regardless of
    // preset.
    public function test_inactiveNearDraws(): void {
        $this->resolveSymbols();
        $task = $this->alloc(4);
        $entry = $this->allocEntry();
        $this->initUint32($entry + 0x2f4, 9); // != activeTrafficPreset (5)

        $this->call('_TrafficDriveParkedTask_8c02656a')->with($task, $entry);

        $this->shouldCall('_njSqrt')->with(f32(10.0 * 10.0))->andReturn(10.0);
        $this->shouldWriteFloat($entry + 0x490, 10.0);
        $this->shouldCall('_BusDrawPlaceEntity_8c027c3c')->with($entry, 0.0);
    }

    // knockbackActive == 1, all 3 ground probes open and no collision:
    // applies the push using the pre-decrement speed, ramps speed down by
    // 0.1, then re-derives heading with the leftover dirX*speed value.
    public function test_activeClearPathAppliesPush(): void {
        $this->resolveSymbols();
        $task = $this->alloc(4);
        $entry = $this->allocEntry();
        $this->initUint32($entry + 0x2b4, 1);
        $this->initUint32($entry + 0x27c, fdec(2.0)); // speed

        $this->call('_TrafficDriveParkedTask_8c02656a')->with($task, $entry);

        $this->shouldCall('_CollisionFindTaskHit_8c02e400')->with($task, $entry)->andReturn(0);

        // dx = 1.0*2.0 = 2.0, dz = 0.5*2.0 = 1.0
        $this->shouldWriteFloat($entry + 0xf4, 12.0);   // posX += dx
        $this->shouldWriteFloat($entry + 0xfc, 1.0);    // posZ += dz
        $this->shouldWriteFloat($entry + 0x100, 14.0);  // frontPointX_0x100 += dx
        $this->shouldWriteFloat($entry + 0x108, 1.0);   // frontPointZ_0x108 += dz
        $this->shouldWriteFloat($entry + 0x27c, f32(1.9)); // speed -= 0.1

        // leftover = dx = 2.0
        $this->shouldCall('_TrafficUpdateHeading_8c026bc4')->with(2.0, $entry);

        $this->shouldCall('_njSqrt')->with(f32(12.0 * 12.0 + 1.0 * 1.0))->andReturn(12.04);
        $this->shouldWriteFloat($entry + 0x490, f32(12.04));
        $this->shouldCall('_BusDrawPlaceEntity_8c027c3c')->with($entry, 0.0);
    }

    // knockbackActive == 1, a ground probe is closed (attr_0x00 == 0):
    // undoes one step using speed+0.1, then unconditionally ends the push
    // (speed pinned to 0, knockbackActive and all 3 probe counts cleared).
    // CollisionFindTaskHit is never called since the probe gate already
    // failed.
    public function test_activeBlockedProbeCancelsPush(): void {
        $this->resolveSymbols();
        $task = $this->alloc(4);
        $entry = $this->allocEntry();
        $this->initUint32($entry + 0x2b4, 1);
        $this->initUint32($entry + 0x27c, fdec(2.0)); // speed
        $this->initUint32($entry + 0x1a0, 0); // groundProbe[1].attr_0x00 closed

        $this->call('_TrafficDriveParkedTask_8c02656a')->with($task, $entry);

        // speed += 0.1 => 2.1; dx = 1.0*2.1 = 2.1, dz = 0.5*2.1 = 1.05
        $dx = f32(1.0 * f32(2.1));
        $dz = f32(0.5 * f32(2.1));
        $this->shouldWriteFloat($entry + 0xf4, f32(10.0 - $dx));  // posX -= dx
        $this->shouldWriteFloat($entry + 0xfc, f32(0.0 - $dz));   // posZ -= dz
        $this->shouldWriteFloat($entry + 0x100, f32(12.0 - $dx)); // frontPointX_0x100 -= dx
        $this->shouldWriteFloat($entry + 0x108, f32(0.0 - $dz));  // frontPointZ_0x108 -= dz

        $this->shouldWriteLong($entry + 0x2b4, 0);
        $this->shouldWriteLong($entry + 0x19c, 0);
        $this->shouldWriteLong($entry + 0x1ac, 0);
        $this->shouldWriteLong($entry + 0x1bc, 0);
        $this->shouldWriteFloat($entry + 0x27c, 0.0);

        // leftover = dz
        $this->shouldCall('_TrafficUpdateHeading_8c026bc4')->with($dz, $entry);

        $newPosX = f32(10.0 - $dx);
        $newPosZ = f32(0.0 - $dz);
        $this->shouldCall('_njSqrt')->with(f32($newPosX * $newPosX + $newPosZ * $newPosZ))->andReturn(9.5);
        $this->shouldWriteFloat($entry + 0x490, 9.5);
        $this->shouldCall('_BusDrawPlaceEntity_8c027c3c')->with($entry, 0.0);
    }

    // knockbackActive == 1, a live collision hit (all probes open but
    // CollisionFindTaskHit returns non-null): also takes the cancel-push
    // branch, same as a closed probe.
    public function test_activeCollisionCancelsPush(): void {
        $this->resolveSymbols();
        $task = $this->alloc(4);
        $entry = $this->allocEntry();
        $this->initUint32($entry + 0x2b4, 1);
        $this->initUint32($entry + 0x27c, fdec(2.0)); // speed

        $this->call('_TrafficDriveParkedTask_8c02656a')->with($task, $entry);

        $this->shouldCall('_CollisionFindTaskHit_8c02e400')->with($task, $entry)->andReturn(0x12345);

        $dx = f32(1.0 * f32(2.1));
        $dz = f32(0.5 * f32(2.1));
        $this->shouldWriteFloat($entry + 0xf4, f32(10.0 - $dx));
        $this->shouldWriteFloat($entry + 0xfc, f32(0.0 - $dz));
        $this->shouldWriteFloat($entry + 0x100, f32(12.0 - $dx));
        $this->shouldWriteFloat($entry + 0x108, f32(0.0 - $dz));

        $this->shouldWriteLong($entry + 0x2b4, 0);
        $this->shouldWriteLong($entry + 0x19c, 0);
        $this->shouldWriteLong($entry + 0x1ac, 0);
        $this->shouldWriteLong($entry + 0x1bc, 0);
        $this->shouldWriteFloat($entry + 0x27c, 0.0);

        $this->shouldCall('_TrafficUpdateHeading_8c026bc4')->with($dz, $entry);

        $newPosX = f32(10.0 - $dx);
        $newPosZ = f32(0.0 - $dz);
        $this->shouldCall('_njSqrt')->with(f32($newPosX * $newPosX + $newPosZ * $newPosZ))->andReturn(9.5);
        $this->shouldWriteFloat($entry + 0x490, 9.5);
        $this->shouldCall('_BusDrawPlaceEntity_8c027c3c')->with($entry, 0.0);
    }

    // Clear-path push whose ramp-down brings speed to exactly 0 also ends
    // the push (same cleanup as the blocked/collision paths).
    public function test_activePushSpeedHitsZeroClearsState(): void {
        $this->resolveSymbols();
        $task = $this->alloc(4);
        $entry = $this->allocEntry();
        $this->initUint32($entry + 0x2b4, 1);
        $this->initUint32($entry + 0x27c, fdec(0.1)); // speed, decrements to exactly 0

        $this->call('_TrafficDriveParkedTask_8c02656a')->with($task, $entry);

        $this->shouldCall('_CollisionFindTaskHit_8c02e400')->with($task, $entry)->andReturn(0);

        $dx = f32(1.0 * 0.1);
        $dz = f32(0.5 * 0.1);
        $this->shouldWriteFloat($entry + 0xf4, f32(10.0 + $dx));
        $this->shouldWriteFloat($entry + 0xfc, f32(0.0 + $dz));
        $this->shouldWriteFloat($entry + 0x100, f32(12.0 + $dx));
        $this->shouldWriteFloat($entry + 0x108, f32(0.0 + $dz));

        $this->shouldWriteLong($entry + 0x2b4, 0);
        $this->shouldWriteLong($entry + 0x19c, 0);
        $this->shouldWriteLong($entry + 0x1ac, 0);
        $this->shouldWriteLong($entry + 0x1bc, 0);
        $this->shouldWriteFloat($entry + 0x27c, 0.0);

        $this->shouldCall('_TrafficUpdateHeading_8c026bc4')->with($dx, $entry);

        $newPosX = f32(10.0 + $dx);
        $newPosZ = f32(0.0 + $dz);
        $this->shouldCall('_njSqrt')->with(f32($newPosX * $newPosX + $newPosZ * $newPosZ))->andReturn(10.05);
        $this->shouldWriteFloat($entry + 0x490, 10.05);
        $this->shouldCall('_BusDrawPlaceEntity_8c027c3c')->with($entry, 0.0);
    }
};
