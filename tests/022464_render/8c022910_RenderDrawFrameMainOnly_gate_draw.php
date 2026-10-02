<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_camera_8c1bb904', 64);
        $this->setSize('_njUserClipping', 4);
        $this->setSize('_njControl3D', 4);
        $this->setSize('_njSetScreen', 4);
        $this->setSize('_njDrawPolygon', 4);
    }

    public function test_draws_plain_fade_overlay_when_gate_open(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_sceneVisible_8c226560'), 1);
        $this->initUint32($this->addressOf('_var_fadePhase_8c227d7c'), 0);
        $this->initUint32($this->addressOf('_var_fadeRequest_8c226564'), 0);

        $var8c1bb904 = $this->addressOf('_var_camera_8c1bb904');

        $this->call('_RenderDrawFrameMainOnly_8c022910');

        $this->shouldCall('_njControl3D')->with(0x100);
        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_clipMirrorView_8c045578'));
        $this->shouldCall('_njSetScreen')->with($this->addressOf('_init_screenFull_8c0455e8'));
        $this->shouldWriteLong($this->addressOf('_var_drawCamera_8c226558'), $var8c1bb904);
        $this->shouldCall('_drawLayer_8c022464')->with(0);
    }
};
