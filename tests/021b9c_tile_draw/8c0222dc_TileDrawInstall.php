<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_tasks_8c1ba5e8', 4);
        $this->setSize('_TaskInstall_8c014ae8', 4);
        $this->setSize('_var_segmentModels_8c1bc3f0', 4);
        $this->setSize('_var_sceneParams_8c18ad24', 4);
    }

    private function toRaw(float $v): int {
        return unpack('L', pack('f', $v))[1];
    }

    public function test_captures_scene_state(): void {
        $this->resolveSymbols();

        $state = $this->alloc(0x20);

        $model = $this->alloc(8);
        $this->initUint32($model, 0x11223344);
        $this->initUint32($model + 4, 0x55667788);
        $this->initUint32($this->addressOf('_var_segmentModels_8c1bc3f0'), $model);

        $sceneParams = $this->alloc(0x88);
        $rec1 = [1.0, 2.0, 3.0, 4.0, 5.0];
        $rec2 = [6.0, 7.0, 8.0, 9.0, 10.0];
        foreach ($rec1 as $i => $v) {
            $this->initUint32($sceneParams + 0x54 + $i * 4, $this->toRaw($v));
        }
        foreach ($rec2 as $i => $v) {
            $this->initUint32($sceneParams + 0x74 + $i * 4, $this->toRaw($v));
        }
        $this->initUint32($this->addressOf('_var_sceneParams_8c18ad24'), $sceneParams);

        $this->call('_TileDrawInstall_8c0222dc');

        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba5e8'),
                $this->addressOf('_enqueueTask_8c0221d0'),
                0xfffff4, // &task on the stack
                0xfffff0, // &state on the stack
                8,
            )
            ->do(function ($params) use ($state) {
                $this->memory->writeUInt32($params[3], U32::of($state));
            });
        $this->shouldWriteLong($state, 0x11223344);
        $this->shouldWriteLong($state + 4, 0x55667788);

        $this->shouldWriteFloat($this->addressOf('_var_easyLightIntensity_8c226544'), 1.0);
        $this->shouldWriteFloat($this->addressOf('_var_easyLightIntensity_8c226544') + 4, 2.0);
        $this->shouldWriteFloat($this->addressOf('_var_easyLightColor_8c22654c'), 3.0);
        $this->shouldWriteFloat($this->addressOf('_var_easyLightColor_8c22654c') + 4, 4.0);
        $this->shouldWriteFloat($this->addressOf('_var_easyLightColor_8c22654c') + 8, 5.0);

        $this->shouldWriteFloat($this->addressOf('_var_simpleLightIntensity_8c2264f0'), 6.0);
        $this->shouldWriteFloat($this->addressOf('_var_simpleLightIntensity_8c2264f0') + 4, 7.0);
        $this->shouldWriteFloat($this->addressOf('_var_simpleLightColor_8c2264f8'), 8.0);
        $this->shouldWriteFloat($this->addressOf('_var_simpleLightColor_8c2264f8') + 4, 9.0);
        $this->shouldWriteFloat($this->addressOf('_var_simpleLightColor_8c2264f8') + 8, 10.0);
    }
};
