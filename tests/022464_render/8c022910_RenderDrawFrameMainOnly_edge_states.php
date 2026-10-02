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

    // Each vertex's .col field is written via displacement addressing from
    // init_fadeQuad_8c0455a8 (offsets 0xc/0x1c/0x2c/0x3c);
    // no separate symbol exists for them in either object.
    private function colAddresses(int $init8c0455a8): array {
        return [$init8c0455a8 + 12, $init8c0455a8 + 28, $init8c0455a8 + 44, $init8c0455a8 + 60];
    }

    public function test_idle_ignores_unknown_request(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_sceneVisible_8c226560'), 0);
        $this->initUint32($this->addressOf('_var_fadePhase_8c227d7c'), 0);
        $this->initUint32($this->addressOf('_var_fadeRequest_8c226564'), 3);

        $this->call('_RenderDrawFrameMainOnly_8c022910');
    }

    public function test_unknown_phase_does_nothing(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_sceneVisible_8c226560'), 0);
        $this->initUint32($this->addressOf('_var_fadePhase_8c227d7c'), 4);
        $this->initUint32($this->addressOf('_var_fadeRequest_8c226564'), 1);

        $this->call('_RenderDrawFrameMainOnly_8c022910');
    }

    public function test_fade_out_reaches_threshold_and_holds(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_sceneVisible_8c226560'), 0);
        $this->initUint32($this->addressOf('_var_fadePhase_8c227d7c'), 2);
        $this->initUint32($this->addressOf('_var_fadeRequest_8c226564'), 2);
        $this->initUint32($this->addressOf('_var_fadeProgress_8c227d80'), 0xfc0000);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);

        $init8c0455a8 = $this->addressOf('_init_fadeQuad_8c0455a8');
        [$init8c0455b4, $init8c0455c4, $init8c0455d4, $init8c0455e4] = $this->colAddresses($init8c0455a8);

        $this->call('_RenderDrawFrameMainOnly_8c022910');

        $this->shouldWriteLong($this->addressOf('_var_fadeProgress_8c227d80'), 0x1004000);
        $this->shouldWriteLong($this->addressOf('_var_fadePhase_8c227d7c'), 3);
        $this->shouldWriteLong($this->addressOf('_var_sceneVisible_8c226560'), 0);
        $this->shouldWriteLong($this->addressOf('_var_fadeProgress_8c227d80'), 0xff0000);
        $this->shouldWriteLong($init8c0455b4, 0xff000000);
        $this->shouldWriteLong($init8c0455c4, 0xff000000);
        $this->shouldWriteLong($init8c0455d4, 0xff000000);
        $this->shouldWriteLong($init8c0455e4, 0xff000000);
        $this->shouldCall('_njDrawPolygon')->with($init8c0455a8, 4, 1);
    }

    // The held phase completes without touching the fade quad.
    public function test_held_phase_does_not_draw(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_sceneVisible_8c226560'), 0);
        $this->initUint32($this->addressOf('_var_fadePhase_8c227d7c'), 3);
        $this->initUint32($this->addressOf('_var_fadeRequest_8c226564'), 2);
        $this->initUint32($this->addressOf('_var_fadeProgress_8c227d80'), 0xff0000);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);
        $this->initUint32($this->addressOf('_var_fadeCompleteCallback_8c22656c'), 0xffffffff);

        $this->call('_RenderDrawFrameMainOnly_8c022910');

        $this->shouldWriteLong($this->addressOf('_var_fadePhase_8c227d7c'), 0);
        $this->shouldWriteLong($this->addressOf('_var_fadeRequest_8c226564'), 0);
        $this->shouldWriteLong($this->addressOf('_var_isFading_8c226568'), 0);
    }
};
