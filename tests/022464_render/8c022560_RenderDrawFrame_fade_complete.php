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

    public function test_fade_in_completes_without_callback(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_sceneVisible_8c226560'), 0);
        $this->initUint32($this->addressOf('_var_fadePhase_8c227d7c'), 1);
        $this->initUint32($this->addressOf('_var_fadeRequest_8c226564'), 1);
        $this->initUint32($this->addressOf('_var_fadeProgress_8c227d80'), 0x5400000);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);
        $this->initUint32($this->addressOf('_var_fadeCompleteCallback_8c22656c'), 0xffffffff);

        $this->call('_RenderDrawFrame_8c022560');

        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_clipCabin_8c045598'));
        $this->shouldWriteLong($this->addressOf('_var_fadeProgress_8c227d80'), 0x1000000);
        $this->shouldWriteLong($this->addressOf('_var_fadePhase_8c227d7c'), 0);
        $this->shouldWriteLong($this->addressOf('_var_fadeRequest_8c226564'), 0);
        $this->shouldWriteLong($this->addressOf('_var_isFading_8c226568'), 0);
    }

    public function test_fade_in_completes_and_fires_callback(): void {
        $this->resolveSymbols();
        $this->setSize('_callback', 4);

        $this->initUint32($this->addressOf('_var_sceneVisible_8c226560'), 0);
        $this->initUint32($this->addressOf('_var_fadePhase_8c227d7c'), 1);
        $this->initUint32($this->addressOf('_var_fadeRequest_8c226564'), 1);
        $this->initUint32($this->addressOf('_var_fadeProgress_8c227d80'), 0x5400000);
        $this->initUint32($this->addressOf('_var_isFading_8c226568'), 1);
        $this->initUint32($this->addressOf('_var_fadeCompleteCallback_8c22656c'), $this->addressOf('_callback'));

        $this->call('_RenderDrawFrame_8c022560');

        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_clipCabin_8c045598'));
        $this->shouldWriteLong($this->addressOf('_var_fadeProgress_8c227d80'), 0x1000000);
        $this->shouldWriteLong($this->addressOf('_var_fadePhase_8c227d7c'), 0);
        $this->shouldWriteLong($this->addressOf('_var_fadeRequest_8c226564'), 0);
        $this->shouldWriteLong($this->addressOf('_var_isFading_8c226568'), 0);
        $this->shouldCall('_callback');
        $this->shouldWriteLong($this->addressOf('_var_fadeCompleteCallback_8c22656c'), 0xffffffff);
    }

    public function test_held_fade_completes_immediately(): void {
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
