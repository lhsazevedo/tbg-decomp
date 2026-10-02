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
 * _CinematicUpdateCamera_8c025906(void): sets up the live camera (var_camera_8c1bb904)
 * each frame during demo playback. var_cameraMode_8c227d9c==7 first re-derives the
 * bus's draw position (posX_0x2fc/posY_0x300/posZ_0x304) from var_demoShotPos_8c227e00
 * transformed by the bus's world matrix; then, for 5/6/7, points the camera
 * at that draw position, aims it at the bus's ground position, and updates
 * the moveDeltaX_0x308/moveDeltaZ_0x310 delta. Any other state is a no-op past the
 * camera setup.
 */
return new class extends TestCase {
    private function f32(float $value): float
    {
        return unpack('f', pack('f', $value))[1];
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_camera_8c1bb904', 0x40);
        $this->setSize('_var_cameraMode_8c227d9c', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_njInitCamera', 4);
        $this->setSize('_njSetCameraAngle', 4);
        $this->setSize('_njSetCameraDepth', 4);
        $this->setSize('_njCalcPoint', 4);
        $this->setSize('_njTranslateCameraPosition', 4);
        $this->setSize('_njPointCameraInterest', 4);

        // Pulled in by other not-yet-decompiled functions sharing this
        // object's literal pool.
        $this->setSize('_var_cabinCamera_8c1bb984', 0x40);
        $this->setSize('_var_demoShotId_8c227dd4', 4);
        $this->setSize('_var_route_8c18ad1c', 4);
        $this->setSize('_var_tasks_8c1ba5e8', 4);
        $this->setSize('__quick_odd_mvn', 4);
        $this->setSize('_MessageBoxSetText_8c02aefc', 4);
        $this->setSize('_MessageBoxDrawText_8c02af1c', 4);
        $this->setSize('_MessageBoxOpenTextbox_8c02ae3e', 4);
        $this->setSize('_BusCameraDrawBusModel_8c024bb8', 4);
        $this->setSize('_RenderQueueDraw_8c0223ea', 4);
        $this->setSize('_TaskInstall_8c014ae8', 4);
    }

    private function initBusStatePos(int $busState): void
    {
        $this->initUint32($busState + 0xf4, fdec(1.0));  // posX_0x0f4
        $this->initUint32($busState + 0xf8, fdec(2.0));  // posY_0x0f8
        $this->initUint32($busState + 0xfc, fdec(3.0));  // posZ_0x0fc
    }

    public function testModeSeven(): void
    {
        $this->resolveSymbols();

        $camera = $this->addressOf('_var_camera_8c1bb904');
        $busState = $this->addressOf('_var_busState_8c1bb9d0');
        $point = $this->addressOf('_var_demoShotPos_8c227e00');

        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 7);
        $this->initBusStatePos($busState);
        // Draw position, written by the mocked njCalcPoint below.
        $this->initUint32($busState + 0x2fc, fdec(10.0)); // posX_0x2fc
        $this->initUint32($busState + 0x300, fdec(20.0)); // posY_0x300
        $this->initUint32($busState + 0x304, fdec(30.0)); // posZ_0x304

        $this->call('_CinematicUpdateCamera_8c025906')->with();

        $this->shouldCall('_njInitCamera')->with($camera);
        $this->shouldCall('_njSetCameraAngle')->with($camera, 10194);
        $this->shouldCall('_njSetCameraDepth')->with($camera, -1.0, -300.0);
        $this->shouldCall('_njCalcPoint')->with($busState + 0x84, $point, $busState + 0x2fc);
        $this->shouldCall('_njTranslateCameraPosition')->with($camera, $this->f32(10.0), $this->f32(20.0), $this->f32(30.0));
        $this->shouldCall('_njPointCameraInterest')->with($camera, $this->f32(1.0), $this->f32(2.0), $this->f32(3.0));
        $this->shouldWriteFloat($busState + 0x308, $this->f32(9.0));  // posX_0x2fc - posX_0x0f4
        $this->shouldWriteFloat($busState + 0x310, $this->f32(27.0)); // posZ_0x304 - posZ_0x0fc
    }

    public function testModeFive(): void
    {
        $this->resolveSymbols();

        $camera = $this->addressOf('_var_camera_8c1bb904');
        $busState = $this->addressOf('_var_busState_8c1bb9d0');

        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 5);
        $this->initBusStatePos($busState);
        $this->initUint32($busState + 0x2fc, fdec(-4.0)); // posX_0x2fc
        $this->initUint32($busState + 0x300, fdec(5.0));  // posY_0x300
        $this->initUint32($busState + 0x304, fdec(-6.0)); // posZ_0x304

        $this->call('_CinematicUpdateCamera_8c025906')->with();

        $this->shouldCall('_njInitCamera')->with($camera);
        $this->shouldCall('_njSetCameraAngle')->with($camera, 10194);
        $this->shouldCall('_njSetCameraDepth')->with($camera, -1.0, -300.0);
        $this->shouldCall('_njTranslateCameraPosition')->with($camera, $this->f32(-4.0), $this->f32(5.0), $this->f32(-6.0));
        $this->shouldCall('_njPointCameraInterest')->with($camera, $this->f32(1.0), $this->f32(2.0), $this->f32(3.0));
        $this->shouldWriteFloat($busState + 0x308, $this->f32(-5.0)); // posX_0x2fc - posX_0x0f4
        $this->shouldWriteFloat($busState + 0x310, $this->f32(-9.0)); // posZ_0x304 - posZ_0x0fc
    }

    public function testOtherStateSkipsPositioning(): void
    {
        $this->resolveSymbols();

        $camera = $this->addressOf('_var_camera_8c1bb904');
        $busState = $this->addressOf('_var_busState_8c1bb9d0');

        $this->initUint32($this->addressOf('_var_cameraMode_8c227d9c'), 0);
        $this->initBusStatePos($busState);
        $this->initUint32($busState + 0x2fc, fdec(1.0));
        $this->initUint32($busState + 0x300, fdec(1.0));
        $this->initUint32($busState + 0x304, fdec(1.0));
        $this->initUint32($busState + 0x308, fdec(1.0));
        $this->initUint32($busState + 0x310, fdec(1.0));

        $this->call('_CinematicUpdateCamera_8c025906')->with();

        $this->shouldCall('_njInitCamera')->with($camera);
        $this->shouldCall('_njSetCameraAngle')->with($camera, 10194);
        $this->shouldCall('_njSetCameraDepth')->with($camera, -1.0, -300.0);
    }
};
