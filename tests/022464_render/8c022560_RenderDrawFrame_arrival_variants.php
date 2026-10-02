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

    public function test_arrival_variant_1(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_sceneVisible_8c226560'), 1);
        $this->initUint32($this->addressOf('_var_viewLayout_8c22655c'), 0);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x268), 1);
        $this->initUint32($this->addressOf('_var_fadePhase_8c227d7c'), 0);
        $this->initUint32($this->addressOf('_var_fadeRequest_8c226564'), 0);

        $camera1 = $this->addressOf('_var_mirrorCamera_8c1bb944');
        $cameraDefault = $this->addressOf('_var_camera_8c1bb904');

        $this->call('_RenderDrawFrame_8c022560');

        $this->shouldCall('_njControl3D')->with(0x100);
        $this->shouldCall('_njUserClipping')->with(2, $this->addressOf('_init_clipMirrorView_8c045578'));
        $this->shouldCall('_njSetScreen')->with($this->addressOf('_init_screenMirror_8c0455fc'));
        $this->shouldWriteLong($this->addressOf('_var_drawCamera_8c226558'), $camera1);
        $this->shouldCall('_drawLayer_8c022464')->with(1);
        $this->shouldCall('_njSetTexture')->with($this->addressOf('_init_renderTexlist_8c03bf44'));
        $this->shouldCall('_njRenderTextureNumG')->with(999);
        $this->shouldCall('_njUserClipping')->with(2, $this->addressOf('_init_clipMirrorLeft_8c045558'));
        $this->shouldCall('_njDrawTexture')->with($this->addressOf('_init_mirrorQuadLeft_8c045438'), 4, 999, 0);
        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_clipMirrorLeft_8c045558'));
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $this->addressOf('_var_busStopTexlist_8c1bc424'), 0x27, 0.0, 0.0, -1.17
        );
        $this->shouldCall('_njUserClipping')->with(3, $this->addressOf('_init_clipMirrorLeft_8c045558'));
        $this->shouldCall('_njSetScreen')->with($this->addressOf('_init_screenFull_8c0455e8'));
        $this->shouldWriteLong($this->addressOf('_var_drawCamera_8c226558'), $cameraDefault);
        $this->shouldCall('_drawLayer_8c022464')->with(0);
        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_clipCabin_8c045598'));
    }

    public function test_arrival_variant_2(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_sceneVisible_8c226560'), 1);
        $this->initUint32($this->addressOf('_var_viewLayout_8c22655c'), 0);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x268), 2);
        $this->initUint32($this->addressOf('_var_fadePhase_8c227d7c'), 0);
        $this->initUint32($this->addressOf('_var_fadeRequest_8c226564'), 0);

        $camera1 = $this->addressOf('_var_mirrorCamera_8c1bb944');
        $cameraDefault = $this->addressOf('_var_camera_8c1bb904');

        $this->call('_RenderDrawFrame_8c022560');

        $this->shouldCall('_njControl3D')->with(0x100);
        $this->shouldCall('_njUserClipping')->with(2, $this->addressOf('_init_clipMirrorView_8c045578'));
        $this->shouldCall('_njSetScreen')->with($this->addressOf('_init_screenMirror_8c0455fc'));
        $this->shouldWriteLong($this->addressOf('_var_drawCamera_8c226558'), $camera1);
        $this->shouldCall('_drawLayer_8c022464')->with(1);
        $this->shouldCall('_njSetTexture')->with($this->addressOf('_init_renderTexlist_8c03bf44'));
        $this->shouldCall('_njRenderTextureNumG')->with(999);
        $this->shouldCall('_njUserClipping')->with(2, $this->addressOf('_init_clipMirrorRight_8c045568'));
        $this->shouldCall('_njDrawTexture')->with($this->addressOf('_init_mirrorQuadRight_8c045498'), 4, 999, 0);
        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_clipMirrorRight_8c045568'));
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $this->addressOf('_var_busStopTexlist_8c1bc424'), 0x29, 0.0, 0.0, -1.17
        );
        $this->shouldCall('_njUserClipping')->with(3, $this->addressOf('_init_clipMirrorRight_8c045568'));
        $this->shouldCall('_njSetScreen')->with($this->addressOf('_init_screenFull_8c0455e8'));
        $this->shouldWriteLong($this->addressOf('_var_drawCamera_8c226558'), $cameraDefault);
        $this->shouldCall('_drawLayer_8c022464')->with(0);
        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_clipCabin_8c045598'));
    }

    public function test_arrival_unknown_variant_skips_texture_draw(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_sceneVisible_8c226560'), 1);
        $this->initUint32($this->addressOf('_var_viewLayout_8c22655c'), 0);
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x268), 3); // MIRROR_DOOR: drawn, but no label sprite
        $this->initUint32($this->addressOf('_var_fadePhase_8c227d7c'), 0);
        $this->initUint32($this->addressOf('_var_fadeRequest_8c226564'), 0);

        $camera1 = $this->addressOf('_var_mirrorCamera_8c1bb944');
        $cameraDefault = $this->addressOf('_var_camera_8c1bb904');

        $this->call('_RenderDrawFrame_8c022560');

        $this->shouldCall('_njControl3D')->with(0x100);
        $this->shouldCall('_njUserClipping')->with(2, $this->addressOf('_init_clipMirrorView_8c045578'));
        $this->shouldCall('_njSetScreen')->with($this->addressOf('_init_screenMirror_8c0455fc'));
        $this->shouldWriteLong($this->addressOf('_var_drawCamera_8c226558'), $camera1);
        $this->shouldCall('_drawLayer_8c022464')->with(1);
        $this->shouldCall('_njSetTexture')->with($this->addressOf('_init_renderTexlist_8c03bf44'));
        $this->shouldCall('_njRenderTextureNumG')->with(999);
        $this->shouldCall('_njSetScreen')->with($this->addressOf('_init_screenFull_8c0455e8'));
        $this->shouldWriteLong($this->addressOf('_var_drawCamera_8c226558'), $cameraDefault);
        $this->shouldCall('_drawLayer_8c022464')->with(0);
        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_clipCabin_8c045598'));
    }

    public function test_arrival_group1(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_sceneVisible_8c226560'), 1);
        $this->initUint32($this->addressOf('_var_viewLayout_8c22655c'), 1);
        $this->initUint32($this->addressOf('_var_fadePhase_8c227d7c'), 0);
        $this->initUint32($this->addressOf('_var_fadeRequest_8c226564'), 0);

        $camera1 = $this->addressOf('_var_mirrorCamera_8c1bb944');
        $camera2 = $this->addressOf('_var_cabinCamera_8c1bb984');
        $cameraDefault = $this->addressOf('_var_camera_8c1bb904');

        $this->call('_RenderDrawFrame_8c022560');

        $this->shouldCall('_njControl3D')->with(0x100);
        $this->shouldCall('_njUserClipping')->with(2, $this->addressOf('_init_clipMirrorViewTall_8c045588'));
        $this->shouldCall('_njSetScreen')->with($this->addressOf('_init_screenMirrorTall_8c045610'));
        $this->shouldWriteLong($this->addressOf('_var_drawCamera_8c226558'), $camera1);
        $this->shouldCall('_drawLayer_8c022464')->with(1);
        $this->shouldCall('_njSetTexture')->with($this->addressOf('_init_renderTexlist_8c03bf44'));
        $this->shouldCall('_njRenderTextureNumG')->with(999);
        $this->shouldCall('_njUserClipping')->with(2, $this->addressOf('_init_clipCabin_8c045598'));
        $this->shouldCall('_njSetScreen')->with($this->addressOf('_init_screenCabin_8c045624'));
        $this->shouldWriteLong($this->addressOf('_var_drawCamera_8c226558'), $camera2);
        $this->shouldCall('_drawLayer_8c022464')->with(2);
        $this->shouldCall('_njUserClipping')->with(3, $this->addressOf('_init_clipCabin_8c045598'));
        $this->shouldCall('_njSetScreen')->with($this->addressOf('_init_screenFull_8c0455e8'));
        $this->shouldCall('_njDrawTexture')->with($this->addressOf('_init_mirrorQuadTall_8c0454f8'), 4, 999, 0);
        $this->shouldWriteLong($this->addressOf('_var_drawCamera_8c226558'), $cameraDefault);
        $this->shouldCall('_drawLayer_8c022464')->with(0);
        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_clipCabin_8c045598'));
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $this->addressOf('_var_busStopTexlist_8c1bc424'), 0x28, 0.0, 0.0, -1.17
        );
        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_clipCabin_8c045598'));
    }

    public function test_arrival_group2(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_sceneVisible_8c226560'), 1);
        $this->initUint32($this->addressOf('_var_viewLayout_8c22655c'), 2);
        $this->initUint32($this->addressOf('_var_fadePhase_8c227d7c'), 0);
        $this->initUint32($this->addressOf('_var_fadeRequest_8c226564'), 0);

        $camera1 = $this->addressOf('_var_mirrorCamera_8c1bb944');
        $cameraDefault = $this->addressOf('_var_camera_8c1bb904');

        $this->call('_RenderDrawFrame_8c022560');

        $this->shouldCall('_njControl3D')->with(0x100);
        $this->shouldCall('_njUserClipping')->with(2, $this->addressOf('_init_clipMirrorViewTall_8c045588'));
        $this->shouldCall('_njSetScreen')->with($this->addressOf('_init_screenMirrorTall_8c045610'));
        $this->shouldWriteLong($this->addressOf('_var_drawCamera_8c226558'), $camera1);
        $this->shouldCall('_drawLayer_8c022464')->with(1);
        $this->shouldCall('_njSetTexture')->with($this->addressOf('_init_renderTexlist_8c03bf44'));
        $this->shouldCall('_njRenderTextureNumG')->with(999);
        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_clipMirrorViewTall_8c045588'));
        $this->shouldCall('_njSetScreen')->with($this->addressOf('_init_screenFull_8c0455e8'));
        $this->shouldCall('_njDrawTexture')->with($this->addressOf('_init_mirrorQuadTall_8c0454f8'), 4, 999, 0);
        $this->shouldCall('_SpriteDraw_8c014f54')->with(
            $this->addressOf('_var_busStopTexlist_8c1bc424'), 0x2a, 0.0, 0.0, -1.17
        );
        $this->shouldWriteLong($this->addressOf('_var_drawCamera_8c226558'), $cameraDefault);
        $this->shouldCall('_drawLayer_8c022464')->with(0);
        $this->shouldCall('_njUserClipping')->with(0, $this->addressOf('_init_clipCabin_8c045598'));
    }
};
