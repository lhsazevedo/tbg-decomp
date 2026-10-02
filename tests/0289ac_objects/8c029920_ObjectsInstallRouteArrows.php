<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

if (!function_exists('fdec8c029920')) {
    function fdec8c029920(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    const ROUTE_SHINJUKU = 0;
    const ROUTE_WANGAN = 1;
    const ROUTE_OME = 2;

    protected function isAsmObject(): bool
    {
        return str_contains($this->objectFile, '/asm/');
    }

    /**
     * ObjectsInstallRouteArrows_8c029920's stack slots for TaskInstall's
     * installed_task / installed_state out-params. The two objects lay the
     * frame out differently.
     */
    private function outParams(): array
    {
        return $this->isAsmObject() ? [0xffffc0, 0xffffbc] : [0xffffc0, 0xffffbc];
    }

    private function setPoint(int $table, int $index, float $x, float $z, int $angleDeg): void
    {
        $entry = $table + $index * 0xc;
        $this->initUint32($entry + 0x0, fdec8c029920($x));
        $this->initUint32($entry + 0x4, fdec8c029920($z));
        $this->initUint32($entry + 0x8, $angleDeg);
    }

    /**
     * Sets up route/model boilerplate, overwrites the point table the given
     * route is expected to read from with a single, immediately-terminated
     * entry, and asserts the function still resolves
     * var_routeArrowNodes_8c228278 and skips TaskInstall. If the route's
     * switch case pointed at the wrong table symbol, the table left
     * unmodified (the real, non-terminated one) would still trigger a
     * TaskInstall call, exposing the mismatch.
     */
    private function assertNoPointsSkipsTaskInstall(int $route, string $tableSymbol): void
    {
        $routeModels = $this->alloc(0x28);
        $this->initUint32($this->addressOf('_var_routeModels_8c1bc3ec'), $routeModels);
        $node0 = 0x30006000;
        $this->initUint32($routeModels + 0x24, $node0); // index 9

        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), $route);

        $table = $this->addressOf($tableSymbol);
        $this->setPoint($table, 0, 0.0, 0.0, 0);

        $this->call('_ObjectsInstallRouteArrows_8c029920');

        $this->shouldWriteLongTo('_var_routeArrowNodes_8c228278', $node0);
        $this->shouldCall('_resolveObjectChildren_8c029868')->with($this->addressOf('_var_routeArrowNodes_8c228278'));
    }

    public function test_no_points_skips_task_install(): void
    {
        $this->assertNoPointsSkipsTaskInstall(self::ROUTE_SHINJUKU, '_init_shinjukuArrowPoints_8c046524');
    }

    public function test_wangan_no_points_skips_task_install(): void
    {
        $this->assertNoPointsSkipsTaskInstall(self::ROUTE_WANGAN, '_init_wanganArrowPoints_8c0465d8');
    }

    public function test_ome_no_points_skips_task_install(): void
    {
        $this->assertNoPointsSkipsTaskInstall(self::ROUTE_OME, '_init_omeArrowPoints_8c0466a4');
    }

    // The real, unmodified init_shinjukuArrowPoints_8c046524 (Shinjuku) table: {x, z, angleDeg}.
    const SHINJUKU_POINTS = [
        [4301.0, 4493.0, 68],
        [3898.0, 4387.0, 6],
        [3867.0, 3877.0, -18],
        [3979.0, 3709.0, 70],
        [3022.0, 3051.0, 18],
        [2905.0, 2484.0, 109],
        [2788.0, 2552.0, 20],
        [2311.0, 1604.0, 30],
        [2426.0, 772.0, 89],
        [2406.0, 850.0, -3],
        [1999.0, 848.0, 20],
        [640.0, 154.0, 90],
        [500.0, 154.0, 180],
        [537.0, 349.0, 90],
    ];

    public function test_installs_task_and_builds_matrices(): void
    {
        $routeModels = $this->alloc(0x28);
        $this->initUint32($this->addressOf('_var_routeModels_8c1bc3ec'), $routeModels);
        $node0 = 0x30006000;
        $this->initUint32($routeModels + 0x24, $node0); // index 9

        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), self::ROUTE_SHINJUKU);

        // Registers the real table's address so the simulator can read its
        // unmodified data; this test asserts against that real data below.
        $this->addressOf('_init_shinjukuArrowPoints_8c046524');

        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $point = $this->setSize('_var_groundQueryPoint_8c1bc460', 0xc);
        $this->initUint32($point + 0x4, fdec8c029920(0.0)); // y, read by the first iteration's query

        [$taskLocal, $stateLocal] = $this->outParams();
        $matrices = 0x30007000;
        $count = count(self::SHINJUKU_POINTS);

        $this->call('_ObjectsInstallRouteArrows_8c029920');

        $this->shouldWriteLongTo('_var_routeArrowNodes_8c228278', $node0);
        $this->shouldCall('_resolveObjectChildren_8c029868')->with($this->addressOf('_var_routeArrowNodes_8c228278'));

        $this->shouldCall('_TaskInstall_8c014ae8')->with(
            $this->addressOf('_var_tasks_8c1ba5e8'),
            $this->addressOf('_routeArrowTask_8c029904'),
            $taskLocal,
            $stateLocal,
            $count * 0x40,
        )->do(function ($params) use ($taskLocal, $stateLocal, $matrices) {
            $this->memory->writeUInt32($taskLocal, U32::of(0x30004000));
            $this->memory->writeUInt32($stateLocal, U32::of($matrices));
        });

        $this->shouldWriteLong(0x30004000 + 0x08, $count);
        $this->shouldWriteLong(0x30004000 + 0x0c, 0);

        $courseAddr = $this->setSize('_var_currentCourse_8c1bb868', 0x50);
        $this->initUint32($courseAddr + 0x28, 0x11111111); // atariHum_0x28
        $this->initUint32($courseAddr + 0x04, 0x22222222); // atariBus_0x04
        $fallbackHeight = 99.0;
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x0f8), fdec8c029920($fallbackHeight));

        // var_groundQueryPoint_8c1bc460.y persists across iterations, so track what it holds
        // whenever a case leaves it untouched (primary grid hit).
        $currentY = 0.0;

        foreach (self::SHINJUKU_POINTS as $i => [$x, $z, $angleDeg]) {
            $matrix = $matrices + $i * 0x40;

            $this->shouldWriteLong($point + 0x0, fdec8c029920($x));
            $this->shouldWriteLong($point + 0x8, fdec8c029920($z));

            switch ($i % 3) {
                case 0:
                    // Primary grid hit: height left untouched.
                    $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x11111111);
                    $this->shouldCall('_GroundQueryFindPolygon_8c020914')
                        ->with($x, $currentY, $z)
                        ->do(function () {
                            $this->memory->writeUInt32($this->registers[4]->value + 0xc, U32::of(1));
                        });
                    break;
                case 1:
                    // Primary grid miss, secondary grid hit: height filled by GroundProbeInterpolateHeight_8c020f7e.
                    $groundY = 42.0 + $i;
                    $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x11111111);
                    $this->shouldCall('_GroundQueryFindPolygon_8c020914')
                        ->with($x, $currentY, $z)
                        ->do(function () {
                            $this->memory->writeUInt32($this->registers[4]->value + 0xc, U32::of(0));
                        });
                    $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x22222222);
                    $this->shouldCall('_GroundQueryFindPolygon_8c020914')
                        ->with($x, $currentY, $z)
                        ->do(function () {
                            $this->memory->writeUInt32($this->registers[4]->value + 0xc, U32::of(1));
                        });
                    $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->do(function () use ($point, $groundY) {
                        $this->memory->writeUInt32($point + 0x4, U32::of(fdec8c029920($groundY)));
                    });
                    $currentY = $groundY;
                    break;
                default:
                    // Both grids miss: hardcoded fallback height.
                    $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x11111111);
                    $this->shouldCall('_GroundQueryFindPolygon_8c020914')
                        ->with($x, $currentY, $z)
                        ->do(function () {
                            $this->memory->writeUInt32($this->registers[4]->value + 0xc, U32::of(0));
                        });
                    $this->shouldWriteLongTo('_var_activeGroundGrid_8c2264d4', 0x22222222);
                    $this->shouldCall('_GroundQueryFindPolygon_8c020914')
                        ->with($x, $currentY, $z)
                        ->do(function () {
                            $this->memory->writeUInt32($this->registers[4]->value + 0xc, U32::of(0));
                        });
                    $currentY = $fallbackHeight;
                    $this->shouldWriteLong($point + 0x4, fdec8c029920($currentY));
                    break;
            }

            $this->shouldCall('_njUnitMatrix')->with($matrix);
            $this->shouldCall('_njTranslate')->with($matrix, $x, $currentY, $z);
            $this->shouldCall('_njRotateY')->with($matrix, (int) ($angleDeg * 65536.0 / 360.0));
        }
    }
};
