<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_var_mirrorCamera_8c1bb944', 64);
        $this->setSize('_var_camera_8c1bb904', 64);
        $this->setSize('_var_cabinCamera_8c1bb984', 76);
        $this->setSize('_var_busStopTexlist_8c1bc424', 4);
        $this->setSize('_njUserClipping', 4);
        $this->setSize('_njControl3D', 4);
        $this->setSize('_njSetScreen', 4);
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njRenderTextureNumG', 4);
        $this->setSize('_njDrawTexture', 4);
        $this->setSize('_njDrawPolygon', 4);
        $this->setSize('_njSetCamera', 4);
        $this->setSize('_SpriteDraw_8c014f54', 4);
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

        $this->call('_RenderDrawFrame_8c022560');

        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_clipCabin_8c045598'));
    }

    public function test_unknown_phase_does_nothing(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_sceneVisible_8c226560'), 0);
        $this->initUint32($this->addressOf('_var_fadePhase_8c227d7c'), 4);
        $this->initUint32($this->addressOf('_var_fadeRequest_8c226564'), 1);

        $this->call('_RenderDrawFrame_8c022560');

        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_clipCabin_8c045598'));
    }

    // Unlike RenderDrawFrameMainOnly_8c022910, this one has no var_isFading_8c226568
    // guard: a cleared flag must not short-cut either ramp.
    public function test_fade_in_ignores_cleared_isFading(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_sceneVisible_8c226560'), 0);
        $this->initUint32($this->addressOf('_var_fadePhase_8c227d7c'), 1);
        $this->initUint32($this->addressOf('_var_fadeRequest_8c226564'), 1);
        $this->initUint32($this->addressOf('_var_fadeProgress_8c227d80'), 0xff000000);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_var_fadeCompleteCallback_8c22656c'), 0xffffffff);

        $init8c0455a8 = $this->addressOf('_init_fadeQuad_8c0455a8');
        [$init8c0455b4, $init8c0455c4, $init8c0455d4, $init8c0455e4] = $this->colAddresses($init8c0455a8);

        $this->call('_RenderDrawFrame_8c022560');

        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_clipCabin_8c045598'));
        $this->shouldWriteLong($this->addressOf('_var_fadeProgress_8c227d80'), 0xfac00000);
        $this->shouldWriteLong($init8c0455b4, 0xfa000000);
        $this->shouldWriteLong($init8c0455c4, 0xfa000000);
        $this->shouldWriteLong($init8c0455d4, 0xfa000000);
        $this->shouldWriteLong($init8c0455e4, 0xfa000000);
        $this->shouldCall('_njDrawPolygon')->with($init8c0455a8, 4, 1);
    }

    public function test_fade_out_ignores_cleared_isFading(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_sceneVisible_8c226560'), 0);
        $this->initUint32($this->addressOf('_var_fadePhase_8c227d7c'), 2);
        $this->initUint32($this->addressOf('_var_fadeRequest_8c226564'), 2);
        $this->initUint32($this->addressOf('_var_fadeProgress_8c227d80'), 0);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 0);
        $this->initUint32($this->addressOf('_var_fadeCompleteCallback_8c22656c'), 0xffffffff);

        $init8c0455a8 = $this->addressOf('_init_fadeQuad_8c0455a8');
        [$init8c0455b4, $init8c0455c4, $init8c0455d4, $init8c0455e4] = $this->colAddresses($init8c0455a8);

        $this->call('_RenderDrawFrame_8c022560');

        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_clipCabin_8c045598'));
        $this->shouldWriteLong($this->addressOf('_var_fadeProgress_8c227d80'), 0x44000);
        $color = (0x44000 << 8) & 0xff000000;
        $this->shouldWriteLong($init8c0455b4, $color);
        $this->shouldWriteLong($init8c0455c4, $color);
        $this->shouldWriteLong($init8c0455d4, $color);
        $this->shouldWriteLong($init8c0455e4, $color);
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

        $this->call('_RenderDrawFrame_8c022560');

        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_clipCabin_8c045598'));
        $this->shouldWriteLong($this->addressOf('_var_fadePhase_8c227d7c'), 0);
        $this->shouldWriteLong($this->addressOf('_var_fadeRequest_8c226564'), 0);
        $this->shouldWriteLong($this->addressOf('_var_isFading_8c226568'), 0);
    }
};
