<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * enqueueTask_8c0221d0(Task *task, void *state): the TaskAction
 * TileDrawInstall_8c0222dc installs. Computes the three fade light directions by
 * broadcasting one scalar CourseSceneParams component into a 3-vector and
 * transforming it with njCalcVector under whichever camera is current (as
 * coded -- reads dir1_0x48[0]/dir2_0x68[0] three times each rather than
 * [0]/[1]/[2], not obviously intentional but preserved), then queues
 * drawTileGrid_8c021b9c/drawTileGridMirror_8c021ec4 as this frame's tile-grid draws for draw layers
 * 0/1, and TileStreamDrawTile_8c021b34 (the current tile, `state`) for both
 * layers.
 */
return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_njSetCamera', 4);
        $this->setSize('_var_camera_8c1bb904', 0x40);
        $this->setSize('_var_mirrorCamera_8c1bb944', 0x40);
        $this->setSize('_njCalcVector', 4);
        $this->setSize('_var_sceneParams_8c18ad24', 4);
        $this->setSize('_RenderQueueDraw2_8c022420', 4);
        $this->setSize('_RenderQueueDraw_8c0223ea', 4);
        $this->setSize('_TileStreamDrawTile_8c021b34', 4);
    }

    /** Raw uint32 bits of a float, for initUint32. */
    private function f(float $v): int
    {
        return unpack('L', pack('f', $v))[1];
    }

    public function test_computes_light_dirs_and_queues_draw_calls(): void
    {
        $this->resolveSymbols();

        // CourseSceneParams: dir1_0x48[3] at +0x48, dir2_0x68[3] at +0x68.
        $sceneParams = $this->alloc(0x88);
        $this->initUint32($sceneParams + 0x48, $this->f(21.0));
        $this->initUint32($sceneParams + 0x68, $this->f(42.0));
        $this->initUint32($this->addressOf('_var_sceneParams_8c18ad24'), $sceneParams);

        $tileIndex = $this->alloc(8);
        $this->initUint32($tileIndex + 0, 7); // width
        $this->initUint32($tileIndex + 4, 5); // height
        $this->initUint32($this->addressOf('_var_tileLayerIndexes_8c22650c'), $tileIndex);

        $task = $this->alloc(4);
        $state = $this->alloc(8);

        $this->call('_enqueueTask_8c0221d0')->with($task, $state);

        $camera0 = $this->addressOf('_var_camera_8c1bb904');
        $camera1 = $this->addressOf('_var_mirrorCamera_8c1bb944');
        $easyDir = $this->addressOf('_var_easyLightDir_8c226538');
        $dir0 = $this->addressOf('_var_simpleLightDir_8c2264d8');
        $dir1 = $this->addressOf('_var_mirrorSimpleLightDir_8c2264e4');

        $this->shouldCall('_njSetCamera')->with($camera0);

        $this->shouldWriteFloat($easyDir + 0, 21.0);
        $this->shouldWriteFloat($easyDir + 4, 21.0);
        $this->shouldWriteFloat($easyDir + 8, 21.0);
        $this->shouldCall('_njCalcVector')->with(0, $easyDir, $easyDir);

        $this->shouldWriteFloat($dir0 + 0, 42.0);
        $this->shouldWriteFloat($dir0 + 4, 42.0);
        $this->shouldWriteFloat($dir0 + 8, 42.0);
        $this->shouldCall('_njCalcVector')->with(0, $dir0, $dir0);

        $this->shouldCall('_njSetCamera')->with($camera1);

        $this->shouldWriteFloat($dir1 + 0, 42.0);
        $this->shouldWriteFloat($dir1 + 4, 42.0);
        $this->shouldWriteFloat($dir1 + 8, 42.0);
        $this->shouldCall('_njCalcVector')->with(0, $dir1, $dir1);

        $this->shouldCall('_RenderQueueDraw2_8c022420')->with(0, $this->addressOf('_drawTileGrid_8c021b9c'), 7, 5);
        $this->shouldCall('_RenderQueueDraw2_8c022420')->with(1, $this->addressOf('_drawTileGridMirror_8c021ec4'), 7, 5);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(0, $this->addressOf('_TileStreamDrawTile_8c021b34'), $state);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(1, $this->addressOf('_TileStreamDrawTile_8c021b34'), $state);
    }
};
