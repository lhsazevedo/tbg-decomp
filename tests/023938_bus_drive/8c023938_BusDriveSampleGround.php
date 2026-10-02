<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _BusDriveSampleGround_8c023938(void): computes 10 corner/lookahead ground-sample points
 * around the bus from its heading (posHistory_0x100[0] vs posX_0x0f4/
 * posZ_0x0fc) and queries each through the ground-query callback stored at
 * groundProbeFn_0x2c8 (GroundQueryFindPolygon_8c020914 or a GroundProbe* variant),
 * filling groundSamples_0x190. Also reseeds posHistory_0x100[0]/[1] (the
 * only two entries not probed) and the heading unit vector
 * headingDirX_0x274/headingDirZ_0x278. Called once per frame while driving, by both
 * placeBus_8c023310/BusInitStart_8c023610 (023310_bus_init) and
 * BusTask_8c022bdc (022bdc).
 */
return new class extends TestCase {
    private function f32(float $value): float
    {
        return unpack('f', pack('f', $value))[1];
    }

    private function fdec(float $value): int
    {
        return unpack('L', pack('f', $value))[1];
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3c8);
        $this->setSize('_njSqrt', 4);
        $this->setSize('_GroundQueryFindPolygon_8c020914', 4);
    }

    public function test_computes_corner_samples_from_a_straight_ahead_heading(): void
    {
        $this->resolveSymbols();

        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $query = $this->addressOf('_GroundQueryFindPolygon_8c020914');

        // Straight-ahead heading: posHistory[0] is 10 units further along +z
        // than the bus's current position, so dx=0, dz=10 -- this keeps the
        // whole computation free of irrational float roundoff.
        $this->initUint32($base + 0xf4, $this->fdec(100.0)); // posX_0x0f4
        $this->initUint32($base + 0xf8, $this->fdec(5.0));   // posY_0x0f8
        $this->initUint32($base + 0xfc, $this->fdec(200.0)); // posZ_0x0fc
        $this->initUint32($base + 0x100, $this->fdec(100.0)); // posHistory[0].x
        $this->initUint32($base + 0x104, $this->fdec(7.0));   // posHistory[0].y
        $this->initUint32($base + 0x108, $this->fdec(210.0)); // posHistory[0].z

        // posHistory[2..11].y: read verbatim as each probe's y (ignored by
        // the callee, but the exact bits must still match).
        for ($i = 2; $i <= 11; $i++) {
            $this->initUint32($base + 0x100 + $i * 0xc + 4, $this->fdec(7.0));
        }

        $this->initUint32($base + 0x2c8, $query); // groundProbeFn_0x2c8

        $this->call('_BusDriveSampleGround_8c023938')->with();

        $this->shouldCall('_njSqrt')->with(100.0)->andReturn(10.0);

        // ndx=0, ndz=1 -- heading fields and posHistory[0]/[1].
        $this->shouldWriteFloat($base + 0x274, 0.0);   // headingDirX_0x274 = ndx
        $this->shouldWriteFloat($base + 0x278, 1.0);   // headingDirZ_0x278 = ndz
        $this->shouldWriteFloat($base + 0x230, 0.0);   // headingX_0x230 = ndx*4.9
        $this->shouldWriteFloat($base + 0x238, 4.9);   // headingZ_0x238 = ndz*4.9
        $this->shouldWriteFloat($base + 0x100, 100.0); // posHistory[0].x
        $this->shouldWriteFloat($base + 0x108, $this->f32(200.0 + 4.9)); // posHistory[0].z
        $this->shouldWriteFloat($base + 0x10c, 100.0); // posHistory[1].x
        $this->shouldWriteFloat($base + 0x114, 208.0); // posHistory[1].z

        // lat=1.25 (ndz*1.25), lon=0 (ndx*1.25).
        $this->shouldWriteFloat($base + 0x118, 98.75);  // posHistory[2].x
        $this->shouldWriteFloat($base + 0x120, 200.0);  // posHistory[2].z
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(98.75, 7.0, 200.0, $base + 0x190);

        $this->shouldWriteFloat($base + 0x124, 101.25); // posHistory[3].x
        $this->shouldWriteFloat($base + 0x12c, 200.0);  // posHistory[3].z
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(101.25, 7.0, 200.0, $base + 0x1a0);

        $this->shouldWriteFloat($base + 0x130, 98.75);  // posHistory[4].x
        $this->shouldWriteFloat($base + 0x138, 208.0);  // posHistory[4].z
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(98.75, 7.0, 208.0, $base + 0x1b0);

        $this->shouldWriteFloat($base + 0x13c, 101.25); // posHistory[5].x
        $this->shouldWriteFloat($base + 0x144, 200.0);  // posHistory[5].z
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(101.25, 7.0, 200.0, $base + 0x1c0);

        $this->shouldWriteFloat($base + 0x178, 98.75);  // posHistory[10].x
        $this->shouldWriteFloat($base + 0x180, 208.0);  // posHistory[10].z
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(98.75, 7.0, 208.0, $base + 0x210);

        $this->shouldWriteFloat($base + 0x184, 101.25); // posHistory[11].x
        $this->shouldWriteFloat($base + 0x18c, 208.0);  // posHistory[11].z
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(101.25, 7.0, 208.0, $base + 0x220);

        $baseZ = $this->f32(200.0 - $this->f32(1.0 * 2.6));

        $this->shouldWriteFloat($base + 0x148, 98.75);  // posHistory[6].x
        $this->shouldWriteFloat($base + 0x150, $baseZ); // posHistory[6].z
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(98.75, 5.0, $baseZ, $base + 0x1d0);

        $this->shouldWriteFloat($base + 0x154, 101.25); // posHistory[7].x
        $this->shouldWriteFloat($base + 0x15c, $baseZ); // posHistory[7].z
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(101.25, 5.0, $baseZ, $base + 0x1e0);

        // lat/=2 -> 0.625, lon/=2 -> 0.
        $this->shouldWriteFloat($base + 0x160, 99.375);  // posHistory[8].x
        $this->shouldWriteFloat($base + 0x168, $baseZ);  // posHistory[8].z
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(99.375, 7.0, $baseZ, $base + 0x1f0);

        $this->shouldWriteFloat($base + 0x16c, 100.625); // posHistory[9].x
        $this->shouldWriteFloat($base + 0x174, $baseZ);  // posHistory[9].z
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')->with(100.625, 7.0, $baseZ, $base + 0x200);
    }
};
