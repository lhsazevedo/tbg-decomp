<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    // BusState offsets used by rowMaterialModelTask_8c02a27c's distance-fade calc.
    const BS_POS_X_0F4 = 0xf4;
    const BS_POS_Z_0FC = 0xfc;
    const BS_POS_X_2FC = 0x2fc;
    const BS_POS_Z_304 = 0x304;

    // ObjectAssetType5Extra {posX, posZ}, at state[0x13] per
    // ObjectsInstall_8c02a6ac's test.
    const ST_POS_X = 0x4c;
    const ST_POS_Z = 0x50;

    // Material-alpha float output.
    const ST_1A = 0x68;

    private function initFloat(int $addr, float $value): void
    {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    public function test_low_quality_zeroes_alpha(): void
    {
        $this->setSize('_var_cameraMode_8c227d9c', 4);
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 1);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);

        $this->call('_rowMaterialModelTask_8c02a27c')->with($task, $state);

        $this->shouldWriteFloat($state + self::ST_1A, 0.0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawRowMaterialModel_8c02a206'), $state);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(1, $this->addressOf('_drawRowMaterialModel_8c02a206'), $state);
    }

    public function test_far_distance_zeroes_alpha(): void
    {
        $this->setSize('_var_cameraMode_8c227d9c', 4);
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 2);
        $this->setSize('_njSqrt', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x2fc + 8);
        $busBase = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initFloat($busBase + self::BS_POS_X_0F4, 0.0);
        $this->initFloat($busBase + self::BS_POS_Z_0FC, 0.0);
        $this->initFloat($busBase + self::BS_POS_X_2FC, 0.0);
        $this->initFloat($busBase + self::BS_POS_Z_304, 0.0);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $this->initFloat($state + self::ST_POS_X, -25.0);
        $this->initFloat($state + self::ST_POS_Z, 0.0);

        $this->call('_rowMaterialModelTask_8c02a27c')->with($task, $state);

        // dx = avg(0, 0) - (-25) = 25, dz = 0; distSq = 625, sqrt = 25 > 20
        $this->shouldCall('_njSqrt')->with(625.0)->andReturn(25.0);
        $this->shouldWriteFloat($state + self::ST_1A, 0.0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawRowMaterialModel_8c02a206'), $state);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(1, $this->addressOf('_drawRowMaterialModel_8c02a206'), $state);
    }

    public function test_mid_distance_ramps_alpha(): void
    {
        $this->setSize('_var_cameraMode_8c227d9c', 4);
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 2);
        $this->setSize('_njSqrt', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x2fc + 8);
        $busBase = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initFloat($busBase + self::BS_POS_X_0F4, 0.0);
        $this->initFloat($busBase + self::BS_POS_Z_0FC, 0.0);
        $this->initFloat($busBase + self::BS_POS_X_2FC, 0.0);
        $this->initFloat($busBase + self::BS_POS_Z_304, 0.0);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $this->initFloat($state + self::ST_POS_X, -15.0);
        $this->initFloat($state + self::ST_POS_Z, 0.0);

        $this->call('_rowMaterialModelTask_8c02a27c')->with($task, $state);

        // dx = 15, dz = 0; distSq = 225, sqrt = 15; (15 - 10) * 0.05 - 0.5 = -0.25
        $this->shouldCall('_njSqrt')->with(225.0)->andReturn(15.0);
        $this->shouldWriteFloat($state + self::ST_1A, -0.25);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawRowMaterialModel_8c02a206'), $state);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(1, $this->addressOf('_drawRowMaterialModel_8c02a206'), $state);
    }

    public function test_averaging_and_both_axes_contribute(): void
    {
        $this->setSize('_var_cameraMode_8c227d9c', 4);
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 2);
        $this->setSize('_njSqrt', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x2fc + 8);
        $busBase = $this->addressOf('_var_busState_8c1bb9d0');
        // Distinct X pair averaging to 15.0; distinct Z pair averaging to 20.0.
        // A bug that forgot the /2.0 or read the wrong field would change
        // either the njSqrt argument or the resulting alpha.
        $this->initFloat($busBase + self::BS_POS_X_0F4, 10.0);
        $this->initFloat($busBase + self::BS_POS_X_2FC, 20.0);
        $this->initFloat($busBase + self::BS_POS_Z_0FC, 15.0);
        $this->initFloat($busBase + self::BS_POS_Z_304, 25.0);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $this->initFloat($state + self::ST_POS_X, 0.0);
        $this->initFloat($state + self::ST_POS_Z, 0.0);

        $this->call('_rowMaterialModelTask_8c02a27c')->with($task, $state);

        // dx = avg(10,20) - 0 = 15, dz = avg(15,25) - 0 = 20; distSq = 225+400 = 625
        $this->shouldCall('_njSqrt')->with(625.0)->andReturn(25.0);
        $this->shouldWriteFloat($state + self::ST_1A, 0.0); // dist 25 > 20: zeroed
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawRowMaterialModel_8c02a206'), $state);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(1, $this->addressOf('_drawRowMaterialModel_8c02a206'), $state);
    }

    public function test_both_dx_and_dz_contribute_to_distance(): void
    {
        $this->setSize('_var_cameraMode_8c227d9c', 4);
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 2);
        $this->setSize('_njSqrt', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x2fc + 8);
        $busBase = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initFloat($busBase + self::BS_POS_X_0F4, 0.0);
        $this->initFloat($busBase + self::BS_POS_Z_0FC, 0.0);
        $this->initFloat($busBase + self::BS_POS_X_2FC, 0.0);
        $this->initFloat($busBase + self::BS_POS_Z_304, 0.0);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        // 9-12-15 triangle: dx=9, dz=12, dist=15 -- a swapped X/Z field would
        // still give distSq=225 here (9^2+12^2==12^2+9^2), so pin the njSqrt
        // argument split too, not just the final dist.
        $this->initFloat($state + self::ST_POS_X, -9.0);
        $this->initFloat($state + self::ST_POS_Z, -12.0);

        $this->call('_rowMaterialModelTask_8c02a27c')->with($task, $state);

        $this->shouldCall('_njSqrt')->with(225.0)->andReturn(15.0);
        // (15 - 10) * 0.05 - 0.5 = -0.25
        $this->shouldWriteFloat($state + self::ST_1A, -0.25);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawRowMaterialModel_8c02a206'), $state);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(1, $this->addressOf('_drawRowMaterialModel_8c02a206'), $state);
    }

    public function test_near_distance_clamps_alpha(): void
    {
        $this->setSize('_var_cameraMode_8c227d9c', 4);
        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 2);
        $this->setSize('_njSqrt', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x2fc + 8);
        $busBase = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initFloat($busBase + self::BS_POS_X_0F4, 0.0);
        $this->initFloat($busBase + self::BS_POS_Z_0FC, 0.0);
        $this->initFloat($busBase + self::BS_POS_X_2FC, 0.0);
        $this->initFloat($busBase + self::BS_POS_Z_304, 0.0);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0x7c);
        $this->initFloat($state + self::ST_POS_X, -3.0);
        $this->initFloat($state + self::ST_POS_Z, 0.0);

        $this->call('_rowMaterialModelTask_8c02a27c')->with($task, $state);

        // dx = 3, dz = 0; distSq = 9, sqrt = 3; (3 - 10) clamps to 0: 0 * 0.05 - 0.5 = -0.5
        $this->shouldCall('_njSqrt')->with(9.0)->andReturn(3.0);
        $this->shouldWriteFloat($state + self::ST_1A, -0.5);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(0, $this->addressOf('_drawRowMaterialModel_8c02a206'), $state);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')
            ->with(1, $this->addressOf('_drawRowMaterialModel_8c02a206'), $state);
    }
};
