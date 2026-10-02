<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

/*
 * _CinematicStartTour_8c025af4(void): opens the caption textbox and arms
 * demoShotTask_8c0259e8 via TaskSpawn_8c014ae8, after selecting this route's
 * shot table into var_demoShots_8c227e0c from
 * init_demoShotsShinjuku_8c045674/init_demoShotsWangan_8c045b60/
 * init_demoShotsOme_8c045ee4.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_cameraMode_8c227d9c', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_njCalcPoint', 4);
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

    private function runForRoute(int $route, string $expectedTable): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), $route);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0xc);

        $this->call('_CinematicStartTour_8c025af4');

        $this->shouldWriteLongTo('_var_demoShots_8c227e0c', $this->addressOf($expectedTable));

        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba5e8'),
                $this->addressOf('_demoShotTask_8c0259e8'),
            )
            ->do(function () use ($task, $state) {
                // Verify the stack-passed alloc_size (5th arg) directly,
                // since with() only checks the two leading register args.
                $sizeArg = $this->memory->readUInt32($this->registers[15]->value);
                if (!$sizeArg->equals(0xc)) {
                    throw new \Exception(
                        "Unexpected TaskSpawn alloc_size $sizeArg, expecting 0xc"
                    );
                }
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($task));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($state));
            })
            ->andReturn(1);

        $this->shouldWriteLong($state + 0x00, 0); // state->phase_0x00 = 0

        $this->shouldCall('_MessageBoxOpenTextbox_8c02ae3e')->with(0x20, 0x180, -1.0, 0x023e, 0x40, 0, 0, -1);

        $this->shouldWriteLongTo('_var_demoShotRearm_8c227e10', 1);
    }

    public function test_shinjuku_route(): void
    {
        $this->runForRoute(0, '_init_demoShotsShinjuku_8c045674');
    }

    public function test_wangan_route(): void
    {
        $this->runForRoute(1, '_init_demoShotsWangan_8c045b60');
    }

    public function test_ome_route(): void
    {
        $this->runForRoute(2, '_init_demoShotsOme_8c045ee4');
    }
};
