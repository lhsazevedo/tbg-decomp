<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value): int
    {
        return unpack('L', pack('f', $value))[1];
    }
}

/*
 * BusDrawPlaceEntity_8c027c3c(TrafficEntry *entity, float accel): called once
 * per frame per traffic entity by TrafficDriveVehicleTask_8c025b98 (025b98). See
 * 027958_bus_draw.h for the full description; this test covers the forward and
 * mirror cone tests, the ground-probe-pointer realign trigger, the suspension
 * lean easing, and the ground re-probe/grid-swap block.
 */
return new class extends TestCase {
    private function f32(float $value): float
    {
        return unpack('f', pack('f', $value))[1];
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_njSqrt', 4);
        $this->setSize('_var_activeGroundGrid_8c2264d4', 4);
        $this->setSize('_var_currentCourse_8c1bb868', 0x50);
        $this->setSize('_RenderQueueDraw2_8c022420', 4);
        $this->setSize('_GroundProbeInterpolateHeight_8c020f7e', 4);
        $this->setSize('_VehicleModelPlace_8c020594', 4);
        $this->setSize('_njMultiMatrix', 4);
        $this->setSize('_GroundProbeTrackPolygonAtHeight_8c021290', 4);
    }

    /** Sets up var_busState_8c1bb9d0's near-cone (move-delta) and
     * far-cone (mirror) reference fields. */
    private function makeBus(): int
    {
        $this->resolveSymbols();
        $bus = $this->addressOf('_var_busState_8c1bb9d0');
        $this->doNotRandomizeMemory();

        $this->initUint32($bus + 0x2fc, fdec(0.0)); // posX_0x2fc
        $this->initUint32($bus + 0x304, fdec(0.0)); // posZ_0x304
        $this->initUint32($bus + 0x308, fdec(1.0)); // move-delta x
        $this->initUint32($bus + 0x310, fdec(0.0)); // move-delta z
        $this->initUint32($bus + 0x314, fdec(1.0)); // move-delta magnitude

        $this->initUint32($bus + 0x318, fdec(0.0)); // mirror x
        $this->initUint32($bus + 0x320, fdec(0.0)); // mirror z
        $this->initUint32($bus + 0x324, fdec(1.0)); // mirror dir x
        $this->initUint32($bus + 0x32c, fdec(0.0)); // mirror dir z
        $this->initUint32($bus + 0x330, fdec(1.0)); // mirror dir magnitude

        return $bus;
    }

    /** Allocates an entity far from both the near and far cone references
     * (posX_0xf4 = 1000), with the ground-realign block fully skipped
     * (field_0x2c8 unrelated, busDistance_0x490 already close). */
    private function makeIdleEntity(): int
    {
        $entity = $this->alloc(0x514);
        $this->doNotRandomizeMemory();

        $this->initUint32($entity + 0xf4, fdec(1000.0)); // posX_0xf4
        $this->initUint32($entity + 0xfc, fdec(0.0));    // posZ_0xfc
        $this->initUint32($entity + 0x2c8, 0);            // field_0x2c8 (probe fn)
        $this->initUint32($entity + 0x490, fdec(100.0));  // busDistance_0x490 (>= 85.333336)

        return $entity;
    }

    public function test_near_registers_draw_call(): void
    {
        $bus = $this->makeBus();
        $entity = $this->makeIdleEntity();
        $this->initUint32($entity + 0xf4, fdec(-10.0)); // dist 10, dot 1.0

        $this->call('_BusDrawPlaceEntity_8c027c3c')->with($entity, 0.0);

        $this->shouldCall('_njSqrt')->with($this->f32(100.0))->andReturn(10.0);
        $this->shouldCall('_RenderQueueDraw2_8c022420')->with(0, $this->addressOf('_drawAhead_8c027a88'), $entity, 0);
        $this->forceStop();
    }

    public function test_near_skipped_when_too_far(): void
    {
        $bus = $this->makeBus();
        $entity = $this->makeIdleEntity();
        $this->initUint32($entity + 0xf4, fdec(-250.0)); // dist 250 >= 200

        $this->call('_BusDrawPlaceEntity_8c027c3c')->with($entity, 0.0);

        $this->shouldCall('_njSqrt')->with($this->f32(62500.0))->andReturn(250.0);
        $this->shouldCall('_njSqrt')->with($this->f32(62500.0))->andReturn(250.0);
        $this->shouldWriteLong($entity + 0x494, 0);
    }

    public function test_near_skipped_when_outside_cone(): void
    {
        $bus = $this->makeBus();
        $entity = $this->makeIdleEntity();
        $this->initUint32($entity + 0xf4, fdec(10.0)); // dot -1.0, dist 10
        $this->initUint32($bus + 0x318, fdec(10010.0)); // keep the far test out of range too (dist 10000)

        $this->call('_BusDrawPlaceEntity_8c027c3c')->with($entity, 0.0);

        $this->shouldCall('_njSqrt')->with($this->f32(100.0))->andReturn(10.0);
        $this->shouldCall('_njSqrt')->with($this->f32(100000000.0))->andReturn(10000.0);
        $this->shouldWriteLong($entity + 0x494, 0);
    }

    public function test_far_lod1_when_past_28m(): void
    {
        $bus = $this->makeBus();
        $entity = $this->makeIdleEntity();
        // Near stays out of range (dist 1000); far: dist 30, dot 1.0.
        $this->initUint32($bus + 0x318, fdec(970.0));

        $this->call('_BusDrawPlaceEntity_8c027c3c')->with($entity, 0.0);

        $this->shouldCall('_njSqrt')->with($this->f32(1000000.0))->andReturn(1000.0);
        $this->shouldCall('_njSqrt')->with($this->f32(900.0))->andReturn(30.0);
        $this->shouldCall('_RenderQueueDraw2_8c022420')->with(1, $this->addressOf('_drawMirror_8c027bac'), $entity, 1);
        $this->forceStop();
    }

    public function test_far_lod0_marks_between_15_and_28m(): void
    {
        $bus = $this->makeBus();
        $entity = $this->makeIdleEntity();
        $this->initUint32($bus + 0x318, fdec(980.0)); // far dist 20

        $this->call('_BusDrawPlaceEntity_8c027c3c')->with($entity, 0.0);

        $this->shouldCall('_njSqrt')->with($this->f32(1000000.0))->andReturn(1000.0);
        $this->shouldCall('_njSqrt')->with($this->f32(400.0))->andReturn(20.0);
        $this->shouldWriteLong($entity + 0x268, 1);
        $this->shouldCall('_RenderQueueDraw2_8c022420')->with(1, $this->addressOf('_drawMirror_8c027bac'), $entity, 0);
        $this->forceStop();
    }

    public function test_far_lod0_no_mark_under_15m(): void
    {
        $bus = $this->makeBus();
        $entity = $this->makeIdleEntity();
        $this->initUint32($bus + 0x318, fdec(990.0)); // far dist 10

        $this->call('_BusDrawPlaceEntity_8c027c3c')->with($entity, 0.0);

        $this->shouldCall('_njSqrt')->with($this->f32(1000000.0))->andReturn(1000.0);
        $this->shouldCall('_njSqrt')->with($this->f32(100.0))->andReturn(10.0);
        $this->shouldCall('_RenderQueueDraw2_8c022420')->with(1, $this->addressOf('_drawMirror_8c027bac'), $entity, 0);
        $this->forceStop();
    }

    public function test_ground_probe_pointer_match_forces_realign(): void
    {
        $bus = $this->makeBus();
        $entity = $this->makeIdleEntity();
        $this->initUint32($entity + 0x2c8, $this->addressOf('_GroundProbeTrackPolygonAtHeight_8c021290'));
        $this->initUint32($entity + 0x494, 5); // nonzero, proves the write below is real

        $this->call('_BusDrawPlaceEntity_8c027c3c')->with($entity, 0.0);

        $this->shouldCall('_njSqrt')->with($this->f32(1000000.0))->andReturn(1000.0);
        $this->shouldCall('_njSqrt')->with($this->f32(1000000.0))->andReturn(1000.0);
        $this->shouldWriteLong($entity + 0x494, 0);
        $this->forceStop();
    }

    public function test_skips_realign_block_when_far_and_already_aligned(): void
    {
        $bus = $this->makeBus();
        $entity = $this->makeIdleEntity();
        // Near/far both out of range; field_0x2c8 doesn't match; busDistance_0x490
        // already below the 85.333336 threshold -> outer condition false.

        $this->call('_BusDrawPlaceEntity_8c027c3c')->with($entity, 0.0);

        $this->shouldCall('_njSqrt')->with($this->f32(1000000.0))->andReturn(1000.0);
        $this->shouldCall('_njSqrt')->with($this->f32(1000000.0))->andReturn(1000.0);
        $this->shouldWriteLong($entity + 0x494, 0);
    }

    public function test_probe_skipped_when_aligned_and_stationary(): void
    {
        $bus = $this->makeBus();
        $entity = $this->makeIdleEntity();
        $this->initUint32($entity + 0xf4, fdec(-10.0)); // near registers -> outer condition true
        $this->initUint32($entity + 0x494, 1);           // already aligned
        $this->initUint32($entity + 0x27c, fdec(0.0));   // stationary -> inner condition false
        $this->initUint32($entity + 0x070, 0);
        $this->initUint32($entity + 0x250, 0); // heading_0x250
        $this->initUint32($entity + 0x254, 0); // headingAlt_0x254
        $this->initUint32($entity + 0x07c, 0);
        $this->initUint32($entity + 0x078, 0);

        $this->call('_BusDrawPlaceEntity_8c027c3c')->with($entity, 0.0);

        $this->shouldCall('_njSqrt')->with($this->f32(100.0))->andReturn(10.0);
        $this->shouldCall('_RenderQueueDraw2_8c022420')->with(0, $this->addressOf('_drawAhead_8c027a88'), $entity, 0);
        $this->shouldCall('_njSqrt')->with($this->f32(100.0))->andReturn(10.0);

        // Suspension-lean easing still runs (all deltas are 0 here).
        $this->shouldWriteLong($entity + 0x070, 0);
        $this->shouldWriteLong($entity + 0x258, 0);
        $this->shouldWriteLong($entity + 0x074, 0);
        $this->shouldWriteLong($entity + 0x07c, 0);
        $this->shouldWriteLong($entity + 0x078, 0);

        // Falls through past the (skipped) probe block: groundAligned_0x494 reset.
        $this->shouldWriteLong($entity + 0x494, 0);
    }

    public function test_full_realign_advances_lean_and_reprobes_ground(): void
    {
        $bus = $this->makeBus();
        $entity = $this->makeIdleEntity();
        $this->initUint32($entity + 0xf4, fdec(-10.0)); // near registers, dist 10
        $this->initUint32($entity + 0x2c8, $this->addressOf('_GroundProbeTrackPolygonAtHeight_8c021290'));
        $this->initUint32($entity + 0x2b4, 0); // driveState != 1: no grid swap

        $this->initUint32($entity + 0x27c, fdec(0.5));   // speed
        $this->initUint32($entity + 0x070, 1000);
        $this->initUint32($entity + 0x250, 100); // heading_0x250
        $this->initUint32($entity + 0x254, 150); // headingAlt_0x254
        $this->initUint32($entity + 0x07c, 0);
        $this->initUint32($entity + 0x078, 0);

        $this->initUint32($entity + 0x118, fdec(1.0));
        $this->initUint32($entity + 0x11c, fdec(2.0));
        $this->initUint32($entity + 0x120, fdec(3.0));
        $this->initUint32($entity + 0x124, fdec(4.0));
        $this->initUint32($entity + 0x128, fdec(5.0));
        $this->initUint32($entity + 0x12c, fdec(6.0));
        $this->initUint32($entity + 0x100, fdec(7.0));
        $this->initUint32($entity + 0x104, fdec(8.0));
        $this->initUint32($entity + 0x108, fdec(9.0));

        $this->call('_BusDrawPlaceEntity_8c027c3c')->with($entity, 0.01);

        $this->shouldCall('_njSqrt')->with($this->f32(100.0))->andReturn(10.0);
        $this->shouldCall('_RenderQueueDraw2_8c022420')->with(0, $this->addressOf('_drawAhead_8c027a88'), $entity, 0);

        // Second (far) njSqrt: same entity position, dist 10 from the
        // default (0,0) mirror reference -- fails the tight cone test.
        $this->shouldCall('_njSqrt')->with($this->f32(100.0))->andReturn(10.0);

        // field_0x2c8 == GroundProbeTrackPolygonAtHeight_8c021290: registered
        // stays true and groundAligned_0x494 is reset.
        $this->shouldWriteLong($entity + 0x494, 0);

        // Suspension-lean easing.
        $this->shouldWriteLong($entity + 0x070, 1000 - (int)(0.5 * 65536.0)); // 1000 - 32768
        $this->shouldWriteLong($entity + 0x258, 50);
        $this->shouldWriteLong($entity + 0x074, 50);
        $this->shouldWriteLong($entity + 0x07c, -25); // -(50*0.5) clamped within +-0xb6/+-0x2d8
        $this->shouldWriteLong($entity + 0x078, 137); // (int)(0.01*45000)=450, clamped to +-0x89=137

        // Ground re-probe (groundAligned_0x494 == 0 -> probes fire).
        $probeA = $entity + 0x190;
        $probeB = $entity + 0x1a0;
        $probeC = $entity + 0x1b0;
        $this->shouldCall('_GroundProbeTrackPolygonAtHeight_8c021290')->with(1.0, 2.0, 3.0, $probeA);
        $this->shouldCall('_GroundProbeTrackPolygonAtHeight_8c021290')->with(4.0, 5.0, 6.0, $probeB);
        $this->shouldCall('_GroundProbeTrackPolygonAtHeight_8c021290')->with(7.0, 8.0, 9.0, $probeC);

        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->with($probeA, $entity + 0x118);
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->with($probeB, $entity + 0x124);
        $this->shouldWriteLong($entity + 0xf8, fdec((2.0 + 5.0) / 2.0)); // posY_0xf8
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->with($probeC, $entity + 0x100);

        $this->shouldCall('_VehicleModelPlace_8c020594')->with($entity + 0x84, $entity);
        $this->shouldWriteLong($entity + 0x494, 1);
    }

    public function test_grid_swap_when_field_0x2b4_is_1(): void
    {
        $bus = $this->makeBus();
        $entity = $this->makeIdleEntity();
        // Near/far both stay out of range; force entry via busDistance_0x490.
        $this->initUint32($entity + 0x490, fdec(0.0));
        $this->initUint32($entity + 0x2b4, 1);
        $this->initUint32($entity + 0x2c8, $this->addressOf('_njMultiMatrix'));

        $this->initUint32($entity + 0x27c, fdec(0.0));
        $this->initUint32($entity + 0x070, 0);
        $this->initUint32($entity + 0x250, 0);
        $this->initUint32($entity + 0x254, 0);
        $this->initUint32($entity + 0x07c, 0);
        $this->initUint32($entity + 0x078, 0);

        $this->initUint32($entity + 0x118, fdec(1.0));
        $this->initUint32($entity + 0x11c, fdec(2.0));
        $this->initUint32($entity + 0x120, fdec(3.0));
        $this->initUint32($entity + 0x124, fdec(4.0));
        $this->initUint32($entity + 0x128, fdec(5.0));
        $this->initUint32($entity + 0x12c, fdec(6.0));
        $this->initUint32($entity + 0x100, fdec(7.0));
        $this->initUint32($entity + 0x104, fdec(8.0));
        $this->initUint32($entity + 0x108, fdec(9.0));

        $busGrid = $this->alloc(4);
        $cpuGrid = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_currentCourse_8c1bb868') + 0x04, $busGrid); // atariBus_0x04
        $this->initUint32($this->addressOf('_var_currentCourse_8c1bb868') + 0x18, $cpuGrid); // atariCpu_0x18

        $this->call('_BusDrawPlaceEntity_8c027c3c')->with($entity, 0.0);

        $this->shouldCall('_njSqrt')->with($this->f32(1000000.0))->andReturn(1000.0);
        $this->shouldCall('_njSqrt')->with($this->f32(1000000.0))->andReturn(1000.0);
        $this->shouldWriteLong($entity + 0x070, 0);
        $this->shouldWriteLong($entity + 0x258, 0);
        $this->shouldWriteLong($entity + 0x074, 0);
        $this->shouldWriteLong($entity + 0x07c, 0);
        $this->shouldWriteLong($entity + 0x078, 0);

        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', $busGrid);

        $probeA = $entity + 0x190;
        $probeB = $entity + 0x1a0;
        $probeC = $entity + 0x1b0;
        $this->shouldCall('_njMultiMatrix')->with(1.0, 2.0, 3.0, $probeA);
        $this->shouldCall('_njMultiMatrix')->with(4.0, 5.0, 6.0, $probeB);
        $this->shouldCall('_njMultiMatrix')->with(7.0, 8.0, 9.0, $probeC);

        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->with($probeA, $entity + 0x118);
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->with($probeB, $entity + 0x124);
        $this->shouldWriteLong($entity + 0xf8, fdec((2.0 + 5.0) / 2.0));
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->with($probeC, $entity + 0x100);

        $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', $cpuGrid);

        $this->shouldCall('_VehicleModelPlace_8c020594')->with($entity + 0x84, $entity);
        $this->shouldWriteLong($entity + 0x494, 1);
    }
};
