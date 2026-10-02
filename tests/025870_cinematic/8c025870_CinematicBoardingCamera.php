<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _CinematicBoardingCamera_8c025870(void): aims the fade camera (var_cabinCamera_8c1bb984)
 * down the aisle for the passenger boarding shot. Unconditional
 * straight-line function -- one path.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_var_cabinCamera_8c1bb984', 0x40);
        $this->setSize('_njInitCamera', 4);
        $this->setSize('_njSetCameraAngle', 4);
        $this->setSize('_njSetCameraDepth', 4);
        $this->setSize('_njTranslateCameraPosition', 4);
        $this->setSize('_njPointCameraInterest', 4);
    }

    public function test(): void
    {
        $this->resolveSymbols();

        $this->call('_CinematicBoardingCamera_8c025870')->with();

        $this->shouldCall('_njInitCamera')->with($this->addressOf('_var_cabinCamera_8c1bb984'));
        $this->shouldCall('_njSetCameraAngle')->with($this->addressOf('_var_cabinCamera_8c1bb984'), 12743);
        $this->shouldCall('_njSetCameraDepth')->with($this->addressOf('_var_cabinCamera_8c1bb984'), -1.0, -15.0);
        $this->shouldCall('_njTranslateCameraPosition')->with($this->addressOf('_var_cabinCamera_8c1bb984'), -0.2, 1.8, -1.6);
        $this->shouldCall('_njPointCameraInterest')->with($this->addressOf('_var_cabinCamera_8c1bb984'), 0.0, 1.8, 3.0);
    }
};
