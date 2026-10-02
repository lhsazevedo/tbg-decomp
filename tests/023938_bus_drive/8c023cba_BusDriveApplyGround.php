<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

/*
 * _BusDriveApplyGround_8c023cba(void): rebuilds the bus's steering-correction direction
 * (dir_x_0x29c/dir_z_0x2a0 and their scaled mirrors dir_x2_0x2ac/
 * dir_z2_0x2b0) from the 10 corner ground-probe results BusDriveSampleGround_8c023938 fills
 * into groundSamples_0x190, then always averages two pairs of those probes'
 * interpolated heights into posY_0x0f8/posHistory_0x100[0].y and fans those
 * out to the lane-offset posHistory entries. Called by BusInitStart_8c023610
 * (023310_bus_init) and BusTask_8c022bdc (022bdc) once per frame while
 * driving.
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
        $this->setSize('_njUnitMatrix', 4);
        $this->setSize('_njRotateY', 4);
        $this->setSize('_njCalcPoint', 4);
        $this->setSize('_var_scratchMatrix_8c1bc46c', 0x40);
        $this->setSize('_var_groundQueryPoint_8c1bc460', 0xc);
        $this->setSize('_GroundProbeInterpolateHeight_8c020f7e', 4);
    }

    private function groundSampleAttr(int $base, int $i): int
    {
        return $base + 0x190 + $i * 0x10;
    }

    private function posHistory(int $base, int $i): int
    {
        return $base + 0x100 + $i * 0xc;
    }

    // Seeds groundSamples_0x190[0..9].attr_0x00 all non-zero (all probes
    // hit), so only the caller's specific override(s) miss.
    private function seedAllHits(int $base): void
    {
        for ($i = 0; $i < 10; $i++) {
            $this->initUint32($this->groundSampleAttr($base, $i), 1);
        }
    }

    // The 4 tail GroundProbeInterpolateHeight calls + posY_0x0f8/
    // posHistory[0].y average + fan-out -- unconditional, common to every
    // path through the function.
    private function assertTail(int $base): void
    {
        $this->initUint32($this->posHistory($base, 2) + 4, $this->fdec(10.0)); // hist[2].y
        $this->initUint32($this->posHistory($base, 3) + 4, $this->fdec(20.0)); // hist[3].y
        $this->initUint32($this->posHistory($base, 4) + 4, $this->fdec(30.0)); // hist[4].y
        $this->initUint32($this->posHistory($base, 5) + 4, $this->fdec(40.0)); // hist[5].y

        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')
            ->with($this->groundSampleAttr($base, 0), $this->posHistory($base, 2));
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')
            ->with($this->groundSampleAttr($base, 1), $this->posHistory($base, 3));
        $this->shouldWriteFloat($base + 0xf8, 15.0); // posY_0x0f8 = (10+20)/2

        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')
            ->with($this->groundSampleAttr($base, 2), $this->posHistory($base, 4));
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')
            ->with($this->groundSampleAttr($base, 3), $this->posHistory($base, 5));
        $this->shouldWriteFloat($this->posHistory($base, 0) + 4, 35.0); // hist[0].y = (30+40)/2

        $this->shouldWriteFloat($this->posHistory($base, 7) + 4, 15.0);
        $this->shouldWriteFloat($this->posHistory($base, 6) + 4, 15.0);
        $this->shouldWriteFloat($this->posHistory($base, 9) + 4, 15.0);
        $this->shouldWriteFloat($this->posHistory($base, 8) + 4, 15.0);
        $this->shouldWriteFloat($this->posHistory($base, 11) + 4, 35.0);
        $this->shouldWriteFloat($this->posHistory($base, 10) + 4, 35.0);
    }

    public function test_forward_probes_miss_falls_back_to_last_heading(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');

        $this->seedAllHits($base);
        $this->initUint32($this->groundSampleAttr($base, 6), 0); // gs[6] miss
        $this->initUint32($base + 0x230, $this->fdec(0.6)); // headingX_0x230 (heading x)
        $this->initUint32($base + 0x238, $this->fdec(0.8)); // headingZ_0x238 (heading z)

        $this->call('_BusDriveApplyGround_8c023cba')->with();

        $this->shouldCall('_hitWall_8c023bea');
        $this->shouldWriteFloat($base + 0x2ac, 0.6); // dir_x2_0x2ac
        $this->shouldWriteFloat($base + 0x29c, 0.6); // dir_x_0x29c
        $this->shouldWriteFloat($base + 0x2b0, 0.8); // dir_z2_0x2b0
        $this->shouldWriteFloat($base + 0x2a0, 0.8); // dir_z_0x2a0

        $this->assertTail($base);
    }

    public function test_outer_probes_missing_divides_by_two(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $matrix = $this->addressOf('_var_scratchMatrix_8c1bc46c');
        $point = $this->addressOf('_var_groundQueryPoint_8c1bc460');

        $this->seedAllHits($base);
        $this->initUint32($this->groundSampleAttr($base, 4), 0); // gs[4] miss
        $this->initUint32($base + 0x250, 0x1234); // ang_0x250

        $this->call('_BusDriveApplyGround_8c023cba')->with();

        $this->shouldCall('_hitWall_8c023bea');
        $this->shouldWriteFloat($point + 0x0, 1.0); // sign (default +1.0)
        $this->shouldWriteFloat($point + 0x8, 0.0);
        $this->shouldCall('_njUnitMatrix')->with($matrix);
        $this->shouldCall('_njRotateY')->with($matrix, 0x1234);
        // njCalcPoint is mocked (no real body runs); fake its in-place
        // transform of *point so the code reads a known result back.
        $x = $this->fdec(3.0);
        $z = $this->fdec(4.0);
        $this->shouldCall('_njCalcPoint')->with($matrix, $point, $point)->do(function () use ($point, $x, $z) {
            $this->memory->writeUInt32($point + 0x0, U32::of($x));
            $this->memory->writeUInt32($point + 0x8, U32::of($z));
        });
        $this->shouldWriteFloat($base + 0x29c, 3.0);
        $this->shouldWriteFloat($base + 0x2a0, 4.0);
        $this->shouldWriteFloat($base + 0x2ac, 1.5); // /2.0
        $this->shouldWriteFloat($base + 0x2b0, 2.0); // /2.0

        $this->assertTail($base);
    }

    public function test_inner_probes_missing_divides_with_negative_sign(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $matrix = $this->addressOf('_var_scratchMatrix_8c1bc46c');
        $point = $this->addressOf('_var_groundQueryPoint_8c1bc460');

        $this->seedAllHits($base);
        $this->initUint32($this->groundSampleAttr($base, 5), 0); // gs[5] miss
        $this->initUint32($base + 0x250, 0x4321); // ang_0x250

        $this->call('_BusDriveApplyGround_8c023cba')->with();

        $this->shouldCall('_hitWall_8c023bea');
        $this->shouldWriteFloat($point + 0x0, -1.0); // sign (negated)
        $this->shouldWriteFloat($point + 0x8, 0.0);
        $this->shouldCall('_njUnitMatrix')->with($matrix);
        $this->shouldCall('_njRotateY')->with($matrix, 0x4321);
        $x = $this->fdec(6.0);
        $z = $this->fdec(8.0);
        $this->shouldCall('_njCalcPoint')->with($matrix, $point, $point)->do(function () use ($point, $x, $z) {
            $this->memory->writeUInt32($point + 0x0, U32::of($x));
            $this->memory->writeUInt32($point + 0x8, U32::of($z));
        });
        $this->shouldWriteFloat($base + 0x29c, 6.0);
        $this->shouldWriteFloat($base + 0x2a0, 8.0);
        $this->shouldWriteFloat($base + 0x2ac, 3.0); // /2.0
        $this->shouldWriteFloat($base + 0x2b0, 4.0); // /2.0

        $this->assertTail($base);
    }

    public function test_side_probes_missing_multiplies_by_two(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');
        $matrix = $this->addressOf('_var_scratchMatrix_8c1bc46c');
        $point = $this->addressOf('_var_groundQueryPoint_8c1bc460');

        $this->seedAllHits($base);
        $this->initUint32($this->groundSampleAttr($base, 2), 0); // gs[2] miss
        $this->initUint32($base + 0x250, 0x2222); // ang_0x250

        $this->call('_BusDriveApplyGround_8c023cba')->with();

        $this->shouldCall('_hitWall_8c023bea');
        $this->shouldWriteFloat($point + 0x0, 1.0); // sign stays default (+1.0)
        $this->shouldWriteFloat($point + 0x8, 0.0);
        $this->shouldCall('_njUnitMatrix')->with($matrix);
        $this->shouldCall('_njRotateY')->with($matrix, 0x2222);
        $x = $this->fdec(1.5);
        $z = $this->fdec(2.5);
        $this->shouldCall('_njCalcPoint')->with($matrix, $point, $point)->do(function () use ($point, $x, $z) {
            $this->memory->writeUInt32($point + 0x0, U32::of($x));
            $this->memory->writeUInt32($point + 0x8, U32::of($z));
        });
        $this->shouldWriteFloat($base + 0x29c, 1.5);
        $this->shouldWriteFloat($base + 0x2a0, 2.5);
        $this->shouldWriteFloat($base + 0x2ac, 3.0); // *2.0
        $this->shouldWriteFloat($base + 0x2b0, 5.0); // *2.0

        $this->assertTail($base);
    }

    public function test_all_probes_hit_keeps_previous_steering(): void
    {
        $this->resolveSymbols();
        $base = $this->addressOf('_var_busState_8c1bb9d0');

        $this->seedAllHits($base);

        $this->call('_BusDriveApplyGround_8c023cba')->with();

        // No decelerate call and no steering-field writes -- only the
        // unconditional tail runs.
        $this->assertTail($base);
    }
};
