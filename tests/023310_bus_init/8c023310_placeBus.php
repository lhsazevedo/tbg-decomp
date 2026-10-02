<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

/*
 * _placeBus_8c023310(void): resets var_busState_8c1bb9d0's driving-physics
 * fields for a fresh run and snaps its position/height to the current segment's
 * bus-stop spawn strip (StopGetArea_8c02cd7a), via a GroundQueryFindPolygon_8c020914
 * + GroundProbeInterpolateHeight_8c020f7e lookup at that (x, z). Also copies the
 * primary directional light's row from var_sceneParams_8c18ad24, seeds the
 * position-history breadcrumb, computes an initial heading from the stop's
 * direction vector via acosf, and (re)arms the debug/practice replay flags
 * (var_cameraMode_8c227d9c / var_cameraCueBusy_8c227dac) depending on var_playMode_8c1bb8d0.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_busState_8c1bb9d0', 0x3c8);
        $this->setSize('_var_currentSegment_8c228708', 4);
        $this->setSize('_var_sceneParams_8c18ad24', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_practiceRules_8c226410', 4);
        $this->setSize('_var_cameraMode_8c227d9c', 4);
        $this->setSize('_var_cameraCueBusy_8c227dac', 4);

        $this->setSize('_StopGetArea_8c02cd7a', 4);
        $this->setSize('_GroundQueryFindPolygon_8c020914', 4);
        $this->setSize('_GroundProbeInterpolateHeight_8c020f7e', 4);
        $this->setSize('_BusDriveSampleGround_8c023938', 4);
        $this->setSize('_acosf', 4);
    }

    // stop-area record: +4/+8 (x, z) origin, +0xc/+0x10 (dx, dz) direction.
    private function allocStopAreaRecord(float $x, float $z, float $dx, float $dz): int
    {
        $rec = $this->alloc(0x14);
        $this->initUint32($rec + 0x4, fdec($x));
        $this->initUint32($rec + 0x8, fdec($z));
        $this->initUint32($rec + 0xc, fdec($dx));
        $this->initUint32($rec + 0x10, fdec($dz));
        return $rec;
    }

    // CourseSceneParams: only rec0_0x0c[0] (offsets 0xc..0x1c) is read.
    private function allocSceneParams(array $row0): int
    {
        $params = $this->alloc(0x88);
        for ($i = 0; $i < 5; $i++) {
            $this->initUint32($params + 0xc + $i * 4, fdec($row0[$i]));
        }
        return $params;
    }

    /**
     * Runs the call and asserts the full, exactly-ordered write/call sequence.
     *
     * @param float[] $sceneRow 5 floats
     */
    private function runAndAssert(
        int $segmentId,
        float $stopX, float $stopZ, float $stopDx, float $stopDz,
        array $sceneRow,
        float $groundHeight,
        int $playMode,
        int $var8c226410,
        float $acosArg, float $acosResult,
        int $expectField25c, int $expectMirror,
        bool $expectNegate,
        int $expectVar8c227d9c, ?int $expectVar8c227dac,
    ): void {
        $this->resolveSymbols();

        $base = $this->addressOf('_var_busState_8c1bb9d0');

        $this->initUint32($this->addressOf('_var_currentSegment_8c228708'), $segmentId);
        $rec = $this->allocStopAreaRecord($stopX, $stopZ, $stopDx, $stopDz);

        $scene = $this->allocSceneParams($sceneRow);
        $this->initUint32($this->addressOf('_var_sceneParams_8c18ad24'), $scene);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), $playMode);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), $var8c226410);

        $this->call('_placeBus_8c023310')->with();

        $this->shouldCall('_StopGetArea_8c02cd7a')->with($segmentId)->andReturn($rec);

        // Physics fields zeroed up front.
        foreach ([0x064, 0x068, 0x06c, 0x070, 0x074, 0x078, 0x07c, 0x080] as $off) {
            $this->shouldWriteLong($base + $off, 0);
        }

        // Copy of the primary directional light's coefficient row.
        for ($i = 0; $i < 5; $i++) {
            $this->shouldWriteFloat($base + 0xc4 + $i * 4, $sceneRow[$i]);
        }

        $this->shouldWriteFloat($base + 0x0f4, $stopX);
        $this->shouldWriteFloat($base + 0x0fc, $stopZ);

        $resultPtr = null;
        $this->shouldCall('_GroundQueryFindPolygon_8c020914')
            ->with($stopX, 0.0, $stopZ)
            ->do(function () use (&$resultPtr) {
                $resultPtr = $this->registers[4]->value;
                $this->memory->writeUInt32($resultPtr + 0xc, U32::of(1));
            });
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')
            ->do(function () use (&$resultPtr, $base, $groundHeight) {
                if ($this->registers[4]->value !== $resultPtr || $this->registers[5]->value !== $base + 0x0f4) {
                    throw new RuntimeException(sprintf(
                        '_GroundProbeInterpolateHeight_8c020f7e: expected result=%08x point=%08x, got result=%08x point=%08x',
                        $resultPtr, $base + 0x0f4, $this->registers[4]->value, $this->registers[5]->value,
                    ));
                }
                $this->memory->writeUInt32($base + 0x0f8, U32::of(fdec($groundHeight)));
            });

        $this->shouldWriteFloat($base + 0x23c, 4.9);
        $this->shouldWriteFloat($base + 0x244, 2.5);
        $this->shouldWriteFloat($base + 0x248, 1.25);
        $this->shouldWriteFloat($base + 0x24c, 2.6);
        $this->shouldWriteLong($base + 0x258, 0);

        $this->shouldWriteLong($base + 0x25c, $expectField25c);
        $this->shouldWriteLong($base + 0x268, $expectMirror);

        $this->shouldWriteFloat($base + 0x270, 1.0);
        $this->shouldWriteLong($base + 0x26c, 0);

        $this->shouldWriteFloat($base + 0x278, $stopDz);
        $this->shouldWriteFloat($base + 0x274, $stopDx);

        $rec0X = $stopX - $stopDx * 4.9;
        $rec0Z = $stopZ - $stopDz * 4.9;
        $rec1X = $stopX - $stopDx * 8.0;
        $rec1Z = $stopZ - $stopDz * 8.0;

        $this->shouldWriteFloat($base + 0x100, $rec0X);
        $this->shouldWriteFloat($base + 0x108, $rec0Z);
        $this->shouldWriteFloat($base + 0x104, $groundHeight);

        $this->shouldWriteFloat($base + 0x10c, $rec1X);
        $this->shouldWriteFloat($base + 0x114, $rec1Z);
        $this->shouldWriteFloat($base + 0x110, $groundHeight);

        foreach ([0x11c, 0x128, 0x134, 0x140, 0x14c, 0x158, 0x164, 0x170, 0x17c, 0x188] as $off) {
            $this->shouldWriteFloat($base + $off, $groundHeight);
        }

        foreach ([
            0x198, 0x19c, 0x1a8, 0x1ac, 0x1b8, 0x1bc, 0x1c8, 0x1cc,
            0x1d8, 0x1dc, 0x1e8, 0x1ec, 0x1f8, 0x1fc, 0x208, 0x20c,
            0x218, 0x21c, 0x228, 0x22c,
        ] as $off) {
            $this->shouldWriteLong($base + $off, 0);
        }

        $this->shouldWriteFloat($base + 0x27c, 0.0);
        $this->shouldWriteFloat($base + 0x284, 0.0);
        $this->shouldWriteFloat($base + 0x288, 0.0);
        $this->shouldWriteFloat($base + 0x28c, 0.0);

        $this->shouldWriteLong($base + 0x2b4, 0);
        $this->shouldWriteLong($base + 0x2b8, $rec);
        $this->shouldWriteFloat($base + 0x2c0, 2.0);
        $this->shouldWriteFloat($base + 0x2bc, 2.0);
        $this->shouldWriteFloat($base + 0x2c4, 2.0);
        $this->shouldWriteLong($base + 0x2d4, 1);

        $this->shouldCall('_BusDriveSampleGround_8c023938');

        $this->shouldCall('_acosf')->with($acosArg)->andReturn($acosResult);

        $ang = (int)($acosResult * 65536.0 / 6.283184);
        if ($expectNegate) {
            $ang = -$ang;
        }

        $this->shouldWriteLong($base + 0x250, $ang);
        $this->shouldWriteLong($base + 0x254, $ang);

        if ($expectVar8c227dac !== null) {
            $this->shouldWriteLongTo('_var_cameraMode_8c227d9c', $expectVar8c227d9c);
            $this->shouldWriteLongTo('_var_cameraCueBusy_8c227dac', $expectVar8c227dac);
        } else {
            $this->shouldWriteLongTo('_var_cameraMode_8c227d9c', $expectVar8c227d9c);
        }
    }

    // Normal play mode (0): falls to the {signalSide_0x25c=0, mirror=3} arm.
    // dx < 0 -> posHistory[0].x (stopX - dx*4.9) > stopX -> no negate.
    public function test_normal_play_mode(): void
    {
        $this->runAndAssert(
            segmentId: 5,
            stopX: 10.0, stopZ: 20.0, stopDx: -1.0, stopDz: 0.5,
            sceneRow: [1.1, 2.2, 3.3, 4.4, 5.5],
            groundHeight: 0.75,
            playMode: 0 /* PLAY_MODE_NORMAL */, var8c226410: 0,
            acosArg: 0.5, acosResult: 1.0471975512 /* pi/3 */,
            expectField25c: 0, expectMirror: 3,
            expectNegate: false,
            expectVar8c227d9c: 0, expectVar8c227dac: 0,
        );
    }

    // Practice mode (1) with the debug bit (0x8) clear: takes the
    // {signalSide_0x25c=2, mirror=2} arm. dx > 0 -> negate branch taken.
    public function test_practice_mode_without_debug_bit(): void
    {
        $this->runAndAssert(
            segmentId: 8,
            stopX: -5.0, stopZ: 12.5, stopDx: 2.0, stopDz: -0.25,
            sceneRow: [0.0, -1.0, 0.5, 2.5, -3.5],
            groundHeight: -1.5,
            playMode: 1 /* PLAY_MODE_PRACTICE */, var8c226410: 0,
            acosArg: -0.25, acosResult: 1.8234765819,
            expectField25c: 2, expectMirror: 2,
            expectNegate: true,
            expectVar8c227d9c: 0, expectVar8c227dac: 0,
        );
    }

    // Practice mode (1) with the debug bit set: falls back to the
    // {signalSide_0x25c=0, mirror=3} arm, same as normal play.
    public function test_practice_mode_with_debug_bit(): void
    {
        $this->runAndAssert(
            segmentId: 3,
            stopX: 0.0, stopZ: 0.0, stopDx: 0.0, stopDz: 1.0,
            sceneRow: [9.0, 8.0, 7.0, 6.0, 5.0],
            groundHeight: 0.0,
            playMode: 1 /* PLAY_MODE_PRACTICE */, var8c226410: 0x8,
            acosArg: 1.0, acosResult: 0.0,
            expectField25c: 0, expectMirror: 3,
            expectNegate: false,
            expectVar8c227d9c: 0, expectVar8c227dac: 0,
        );
    }

    // Demo mode (2): var_cameraMode_8c227d9c = 5, var_cameraCueBusy_8c227dac untouched.
    public function test_demo_play_mode(): void
    {
        $this->runAndAssert(
            segmentId: 1,
            stopX: 3.0, stopZ: -3.0, stopDx: -0.5, stopDz: 0.5,
            sceneRow: [1.0, 1.0, 1.0, 1.0, 1.0],
            groundHeight: 2.0,
            playMode: 2 /* PLAY_MODE_DEMO */, var8c226410: 0,
            acosArg: 0.5, acosResult: 1.0471975512,
            expectField25c: 0, expectMirror: 3,
            expectNegate: false,
            expectVar8c227d9c: 5, expectVar8c227dac: null,
        );
    }
};
