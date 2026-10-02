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
 * _applyShotPosition_8c0258ba(void): positions the bus's draw position
 * (posX_0x2fc/posY_0x300/posZ_0x304) for var_cameraMode_8c227d9c==5 (pin to the fixed
 * world point var_demoShotPos_8c227e00, y relative to ground) or ==6 (transform
 * var_demoShotPos_8c227e00 by the bus's world matrix instead). No-op otherwise.
 */
return new class extends TestCase {
    private function f32(float $value): float
    {
        return unpack('f', pack('f', $value))[1];
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_cameraMode_8c227d9c', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_njCalcPoint', 4);

        // Pulled in by other not-yet-decompiled functions sharing this
        // object's literal pool.
        $this->setSize('_var_cabinCamera_8c1bb984', 0x40);
        $this->setSize('_var_camera_8c1bb904', 0x40);
        $this->setSize('_var_demoShotId_8c227dd4', 4);
        $this->setSize('_var_route_8c18ad1c', 4);
        $this->setSize('_var_tasks_8c1ba5e8', 4);
        $this->setSize('_njInitCamera', 4);
        $this->setSize('_njSetCameraAngle', 4);
        $this->setSize('_njSetCameraDepth', 4);
        $this->setSize('_njTranslateCameraPosition', 4);
        $this->setSize('_njPointCameraInterest', 4);
        $this->setSize('__quick_odd_mvn', 4);
        $this->setSize('_MessageBoxSwapFor_8c02aefc', 4);
        $this->setSize('_MessageBoxMenuTextboxText_8c02af1c', 4);
        $this->setSize('_MessageBoxOpenTextbox_8c02ae3e', 4);
        $this->setSize('_BusCameraDrawBusModel_8c024bb8', 4);
        $this->setSize('_RenderQueueDraw_8c0223ea', 4);
        $this->setSize('_TaskSpawn_8c014ae8', 4);
    }

    public function testFixedWorldPoint(): void
    {
        $this->resolveSymbols();

        $busState = $this->addressOf('_var_busState_8c1bb9d0');
        $point = $this->addressOf('_var_demoShotPos_8c227e00');

        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 5);
        $this->initUint32($point, fdec(10.0));
        $this->initUint32($point + 4, fdec(2.5));
        $this->initUint32($point + 8, fdec(-30.0));
        $this->initUint32($busState + 0xf8, fdec(5.0)); // posY_0x0f8

        $this->call('_applyShotPosition_8c0258ba')->with();

        $this->shouldWriteFloat($busState + 0x2fc, $this->f32(10.0));   // posX_0x2fc = point.x
        $this->shouldWriteFloat($busState + 0x300, $this->f32(7.5));    // posY_0x300 = posY_0x0f8 + point.y
        $this->shouldWriteFloat($busState + 0x304, $this->f32(-30.0));  // posZ_0x304 = point.z
    }

    public function testTransformByWorldMatrix(): void
    {
        $this->resolveSymbols();

        $busState = $this->addressOf('_var_busState_8c1bb9d0');
        $point = $this->addressOf('_var_demoShotPos_8c227e00');

        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 6);

        $this->call('_applyShotPosition_8c0258ba')->with();

        $this->shouldCall('_njCalcPoint')->with($busState + 0x84, $point, $busState + 0x2fc);
    }

    public function testOtherStateIsNoop(): void
    {
        $this->resolveSymbols();

        $busState = $this->addressOf('_var_busState_8c1bb9d0');

        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 7);
        $this->initUint32($busState + 0x2fc, fdec(1.0));
        $this->initUint32($busState + 0x300, fdec(2.0));
        $this->initUint32($busState + 0x304, fdec(3.0));

        $this->call('_applyShotPosition_8c0258ba')->with();
    }
};
