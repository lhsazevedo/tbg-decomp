<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('f32')) {
    function f32(float $value): float
    {
        return unpack('f', pack('f', $value))[1];
    }
}

/*
 * _BusInitStart_8c023610(void): the per-run bootstrap for the player's bus.
 * Resets the ground-grid globals from var_currentCourse_8c1bb868, arms
 * BusTask_8c022bdc via TaskInstall_8c014ae8, binds the bus body model
 * (VehiclePartsBind_8c02786c), picks a (route, timeOfDay) content-swap pointer
 * and a ground-query dispatch table (normal vs. the Wangan-route/segment-10
 * *AtHeight variants), calls placeBus_8c023310 to place the bus, then
 * runs three AttrQueryFindPolygon_8c02e69c region lookups (seeded from the just-computed
 * posHistory_0x100[2]/[3] breadcrumbs) into junctionASlot_0x340/0x35c/0x378,
 * copying each 4-word hit into junctionARoadFlags_0x34c/0x368/0x384 or zeroing on a miss.
 * placeBus_8c023310 is mocked here; its own test covers its body.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_var_currentCourse_8c1bb868', 0x14);
        $this->setSize('_var_activeGroundGrid_8c2264d4', 4);
        $this->setSize('_var_activeAttrGrid_8c228b3c', 4);
        $this->setSize('_var_busDoorMotion_8c1bc410', 4);
        $this->setSize('_var_busDoorLastFrame_8c227db4', 4);
        $this->setSize('_var_tasks_8c1ba5e8', 4);
        $this->setSize('_BusTask_8c022bdc', 4);
        $this->setSize('_TaskInstall_8c014ae8', 4);
        $this->setSize('_var_routeModelSlots_8c1bbddc', 0x200);
        $this->setSize('_var_trafficModels_8c1bc3f4', 4);
        $this->setSize('_VehiclePartsBind_8c02786c', 4);
        $this->setSize('_var_timeOfDay_8c18ad20', 4);
        $this->setSize('_var_route_8c18ad1c', 4);
        $this->setSize('_var_currentSegment_8c228708', 4);
        $this->setSize('_GroundProbeFindPolygonAtHeight_8c020fe4', 4);
        $this->setSize('_AttrQueryFindConvexPolygonAtHeight_8c02eab4', 4);
        $this->setSize('_AttrQueryFindPolygonAtHeight_8c02ec50', 4);
        $this->setSize('_GroundQueryFindPolygon_8c020914', 4);
        $this->setSize('_AttrQueryFindConvexPolygon_8c02e51c', 4);
        $this->setSize('_AttrQueryFindPolygon_8c02e69c', 4);
        $this->setSize('_StopGetSegment_8c02cd6a', 4);
        $this->setSize('_BusDriveSampleGround_8c023938', 4);
        $this->setSize('_BusDriveApplyGround_8c023cba', 4);
        $this->setSize('_VehicleModelPlace_8c020594', 4);
        $this->setSize('_var_playerBus_8c1bbd9c', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_practiceRules_8c226410', 4);
        $this->setSize('_var_brakePressPeak_8c227d8c', 4);
    }

    /**
     * @param float[] $point2 posHistory_0x100[2] as [x, y, z]
     * @param float[] $point3 posHistory_0x100[3] as [x, y, z]
     */
    private function runAndAssert(
        int $route, int $timeOfDay, int $segmentId,
        int $rawTimeBits, float $expectVar8c227db4,
        int $swapPtrIndex, // which of the 5 field_0x04c.. slots gets the 0x37 override
        array $point2, array $point3,
        ?array $lookup1, ?array $lookup2, ?array $lookup3, // 4-int results, or null for a miss
        int $playMode, int $var8c226410, int $expectField3c4,
        bool $wanganAtHeight,
    ): void {
        $this->resolveSymbols();

        $base = $this->addressOf('_var_busState_8c1bb9d0');
        for ($off = 0; $off < 0x3cc; $off += 4) {
            $this->initUint32($base + $off, 0);
        }

        $course = $this->addressOf('_var_currentCourse_8c1bb868');
        $atariBus = 0xcafe0100;
        $lineBus = 0xcafe0200;
        $ukn = 0xcafe0300;
        $attrBus = 0xcafe0400;
        $this->initUint32($course + 0x04, $atariBus);
        $this->initUint32($course + 0x08, $lineBus);
        $this->initUint32($course + 0x0c, $ukn);
        $this->initUint32($course + 0x10, $attrBus);

        $timeStruct = $this->alloc(8);
        $this->initUint32($this->addressOf('_var_busDoorMotion_8c1bc410'), $timeStruct);
        $this->initUint32($timeStruct + 4, $rawTimeBits);

        $vehParts = $this->addressOf('_var_routeModelSlots_8c1bbddc') + 26 * 0x10;
        $vpField4 = 0xcafe0500;
        $vpField8 = 0xcafe0600;
        $this->initUint32($vehParts + 0x08, $vpField4);
        $this->initUint32($vehParts + 0x0c, $vpField8);

        $trafficModels = $this->alloc(0x48);
        $this->initUint32($this->addressOf('_var_trafficModels_8c1bc3f4'), $trafficModels);
        $this->initUint32($trafficModels + 0x44, 0xcafe0700);

        // 5 scratch cells for the field_0x04c.. pointer array.
        $slots = [];
        $slotOffsets = [0x04c, 0x050, 0x054, 0x058, 0x05c, 0x060];
        foreach ($slotOffsets as $i => $off) {
            $slots[$i] = $this->alloc(4);
            $this->initUint32($base + $off, $slots[$i]);
        }

        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), $timeOfDay);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), $route);
        $this->initUint32($this->addressOf('_var_currentSegment_8c228708'), $segmentId);

        for ($i = 0; $i < 3; $i++) {
            $this->initUint32($base + 0x100 + 24 + $i * 4, unpack('L', pack('f', $point2[$i]))[1]);
            $this->initUint32($base + 0x100 + 36 + $i * 4, unpack('L', pack('f', $point3[$i]))[1]);
        }

        $segment = $this->alloc(0x2c);
        $stopAreaId = 0x1234;
        $this->initUint16($segment + 0x02, $stopAreaId);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), $playMode);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), $var8c226410);

        $this->call('_BusInitStart_8c023610')->with();

        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', $atariBus);
        $this->shouldWriteLongTo('_var_activeAttrGrid_8c228b3c', $attrBus);
        $this->shouldWriteLongTo('_var_lineSegments_8c227d84', $lineBus);
        $this->shouldWriteLongTo('_var_lineNodes_8c227d88', $ukn);

        $this->shouldWriteFloat($this->addressOf('_var_busDoorLastFrame_8c227db4'), $expectVar8c227db4);

        $this->shouldCall('_TaskInstall_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba5e8'),
            $this->addressOf('_BusTask_8c022bdc'),
        );

        $this->shouldWriteLongTo('_var_playerBus_8c1bbd9c', $base);
        $this->shouldWriteLong($base + 0x004, $vpField4);
        $this->shouldWriteLong($base + 0x00c, $vpField8);
        $this->shouldWriteLong($base + 0x014, 0xcafe0700);
        $this->shouldCall('_VehiclePartsBind_8c02786c')->with($base, 0x1a);

        foreach ($slots as $slot) {
            $this->shouldWriteLong($slot, 0x3f);
        }
        $this->shouldWriteLong($slots[$swapPtrIndex], 0x37);

        if ($wanganAtHeight) {
            $this->shouldWriteLong($base + 0x2c8, $this->addressOf('_GroundProbeFindPolygonAtHeight_8c020fe4'));
            $this->shouldWriteLong($base + 0x2cc, $this->addressOf('_AttrQueryFindConvexPolygonAtHeight_8c02eab4'));
            $this->shouldWriteLong($base + 0x2d0, $this->addressOf('_AttrQueryFindPolygonAtHeight_8c02ec50'));
        } else {
            $this->shouldWriteLong($base + 0x2c8, $this->addressOf('_GroundQueryFindPolygon_8c020914'));
            $this->shouldWriteLong($base + 0x2cc, $this->addressOf('_AttrQueryFindConvexPolygon_8c02e51c'));
            $this->shouldWriteLong($base + 0x2d0, $this->addressOf('_AttrQueryFindPolygon_8c02e69c'));
        }

        $this->shouldCall('_placeBus_8c023310');

        $this->shouldWriteLong($base + 0x2e0, 0);
        $this->shouldWriteLong($base + 0x2ec, 0);
        $this->shouldWriteLong($base + 0x2f4, 0);
        $this->shouldWriteLong($base + 0x334, 0);

        $this->shouldCall('_StopGetSegment_8c02cd6a')->with($segmentId)->andReturn($segment);
        $this->shouldWriteLong($base + 0x33c, $stopAreaId);

        foreach ([0x344, 0x348, 0x360, 0x364, 0x37c, 0x380, 0x398, 0x39c, 0x3a8, 0x3ac] as $off) {
            $this->shouldWriteLong($base + $off, 0);
        }

        $this->shouldCall('_BusDriveSampleGround_8c023938');
        $this->shouldCall('_BusDriveApplyGround_8c023cba');

        $this->shouldCall('_VehicleModelPlace_8c020594')->with(
            ($this->addressOf('_var_busState_8c1bb9d0') + 0x084),
            $base,
        );

        $this->shouldCall('_AttrQueryFindPolygon_8c02e69c')
            ->with(f32($point2[0]), f32($point2[1]), f32($point2[2]), $base + 0x340)
            ->andReturn($lookup1 === null ? 0 : $this->allocInts($lookup1));
        if ($lookup1 !== null) {
            $this->shouldWriteLong($base + 0x34c, $lookup1[0]);
            $this->shouldWriteLong($base + 0x350, $lookup1[1]);
            $this->shouldWriteLong($base + 0x354, $lookup1[2]);
            $this->shouldWriteLong($base + 0x358, $lookup1[3]);
        } else {
            $this->shouldWriteLong($base + 0x34c, 0);
            $this->shouldWriteLong($base + 0x350, 0);
            $this->shouldWriteLong($base + 0x354, 0);
            $this->shouldWriteLong($base + 0x358, 0);
        }

        $this->shouldCall('_AttrQueryFindPolygon_8c02e69c')
            ->with(f32($point3[0]), f32($point3[1]), f32($point3[2]), $base + 0x35c)
            ->andReturn($lookup2 === null ? 0 : $this->allocInts($lookup2));
        if ($lookup2 !== null) {
            $this->shouldWriteLong($base + 0x368, $lookup2[0]);
            $this->shouldWriteLong($base + 0x36c, $lookup2[1]);
            $this->shouldWriteLong($base + 0x370, $lookup2[2]);
            $this->shouldWriteLong($base + 0x374, $lookup2[3]);
        } else {
            $this->shouldWriteLong($base + 0x368, 0);
            $this->shouldWriteLong($base + 0x36c, 0);
            $this->shouldWriteLong($base + 0x370, 0);
            $this->shouldWriteLong($base + 0x374, 0);
        }

        $this->shouldCall('_AttrQueryFindPolygon_8c02e69c')
            ->with(f32($point3[0]), f32($point3[1]), f32($point3[2]), $base + 0x378)
            ->andReturn($lookup3 === null ? 0 : $this->allocInts($lookup3));
        if ($lookup3 !== null) {
            $this->shouldWriteLong($base + 0x384, $lookup3[0]);
            $this->shouldWriteLong($base + 0x388, $lookup3[1]);
            $this->shouldWriteLong($base + 0x38c, $lookup3[2]);
            $this->shouldWriteLong($base + 0x390, $lookup3[3]);
        } else {
            $this->shouldWriteLong($base + 0x384, 0);
            $this->shouldWriteLong($base + 0x388, 0);
            $this->shouldWriteLong($base + 0x38c, 0);
            $this->shouldWriteLong($base + 0x390, 0);
        }

        foreach ([0x3a0, 0x3b0, 0x3b4, 0x3b8, 0x3bc, 0x3c0] as $off) {
            $this->shouldWriteLong($base + $off, 0);
        }

        $this->shouldWriteLong($base + 0x3c4, $expectField3c4);

        $this->shouldWriteLongTo('_var_brakePressPeak_8c227d8c', 0);
        $this->shouldWriteLong($base + 0x2e4, 0);
        $this->shouldWriteFloat($base + 0x2e8, 0.0);
        $this->shouldWriteLong($base + 0x3c8, 0);
    }

    /** @param int[] $values */
    private function allocInts(array $values): int
    {
        $addr = $this->alloc(count($values) * 4);
        foreach ($values as $i => $v) {
            $this->initUint32($addr + $i * 4, $v);
        }
        return $addr;
    }

    // Shinjuku route, day, non-Wangan/seg10 -> normal ground-query dispatch;
    // TOD day/evening -> field_0x04c array, route 0 -> slot 0 (field_0x04c
    // itself) gets the 0x37 override. All 3 lookups hit.
    public function test_normal_route_day_hits(): void
    {
        $this->runAndAssert(
            route: 0 /* ROUTE_SHINJUKU */, timeOfDay: 0 /* TIME_OF_DAY_DAY */,
            segmentId: 3,
            rawTimeBits: 1000, expectVar8c227db4: f32(1000.0 - 1.0),
            swapPtrIndex: 0,
            point2: [1.0, 2.0, 3.0], point3: [4.0, 5.0, 6.0],
            lookup1: [11, 12, 13, 14],
            lookup2: [21, 22, 23, 24],
            lookup3: [31, 32, 33, 34],
            playMode: 0 /* PLAY_MODE_NORMAL */, var8c226410: 0,
            expectField3c4: 1,
            wanganAtHeight: false,
        );
    }

    // Wangan route, segment 10, night -> *AtHeight dispatch; TOD night ->
    // field_0x058 array, route 1 -> slot 4 (field_0x05c) gets the override.
    // All 3 lookups miss. rawTimeBits negative to exercise the unsigned
    // conversion's +2^32 correction. Practice mode without the debug bit ->
    // doorRequest_0x3c4 = 0.
    public function test_wangan_segment10_night_misses(): void
    {
        $this->runAndAssert(
            route: 1 /* ROUTE_WANGAN */, timeOfDay: 2 /* TIME_OF_DAY_NIGHT */,
            segmentId: 10,
            rawTimeBits: -1000, expectVar8c227db4: f32((-1000 + 4294967296.0) - 1.0),
            swapPtrIndex: 4,
            point2: [-1.5, 0.5, 2.5], point3: [7.0, -3.0, 0.25],
            lookup1: null,
            lookup2: null,
            lookup3: null,
            playMode: 1 /* PLAY_MODE_PRACTICE */, var8c226410: 0,
            expectField3c4: 0,
            wanganAtHeight: true,
        );
    }
};
