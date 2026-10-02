<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

// TrafficInit_8c02769e is the traffic subsystem's setup/init entry point:
// caches two per-course table pointers, picks a per-route table, optionally
// caches a pair of CourseSceneParams rows (+ their per-20-frame deltas) for
// the night route, then installs updateTask_8c0275d4. See the
// function's header comment in 026710_traffic.c for the full breakdown.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_var_currentCourse_8c1bb868', 0x50);
        $this->setSize('_var_route_8c18ad1c', 4);
        $this->setSize('_var_timeOfDay_8c18ad20', 4);
        $this->setSize('_var_sceneParams_8c18ad24', 4);
        $this->setSize('_var_tasks_8c1ba5e8', 4);
        $this->setSize('_var_signalGroups_8c228b40', 4);
        $this->setSize('_TaskInstall_8c014ae8', 4);
        $this->setSize('_SignalClearCrossingOccupied_8c028958', 4);
        $this->setSize('_init_signalGroupsWangan_8c04c980', 4);
        $this->setSize('_init_signalGroupsShinjuku_8c04caec', 4);
        $this->setSize('_init_signalGroupsOme_8c04cd38', 4);

        $this->setSize('_var_nightLightIntensityStep_8c1bbda0', 2 * 4);
        $this->setSize('_var_nightLightIntensityOff_8c1bbda8', 2 * 4);
        $this->setSize('_var_nightLightIntensityOn_8c1bbdb0', 2 * 4);
        $this->setSize('_var_nightLightColorStep_8c1bbdb8', 3 * 4);
        $this->setSize('_var_nightLightColorOff_8c1bbdc4', 3 * 4);
        $this->setSize('_var_nightLightColorOn_8c1bbdd0', 3 * 4);
    }

    private function initFloat(int $addr, float $value): void {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    private function shouldWriteFloatAt(int $addr, float $value): void {
        $this->shouldWriteFloat($addr, $value);
    }

    // rec0_0x0c is float[3][5] starting at var_sceneParams_8c18ad24+0x0c;
    // row 1 starts at +0x20, row 2 at +0x34.
    private function setSceneRow(int $scene, int $row, array $values): void {
        $base = $scene + 0x0c + $row * 5 * 4;
        foreach ($values as $i => $v) {
            $this->initFloat($base + $i * 4, $v);
        }
    }

    // &task and &state are TrafficInit_8c02769e's own stack locals; both
    // objects place them at the same slots.
    private function mockTaskInstall(int $task, int $state): void {
        $this->shouldCall('_TaskInstall_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba5e8'),
                $this->addressOf('_updateTask_8c0275d4'),
                0xffffd0,
                0xffffd4,
                0,
            )
            ->do(function () use ($task, $state) {
                $this->memory->writeUInt32($this->registers[6]->value, U32::of($task));
                $this->memory->writeUInt32($this->registers[7]->value, U32::of($state));
            })
            ->andReturn(1);
    }

    // ROUTE_WANGAN (1) selects init_signalGroupsWangan_8c04c980; timeOfDay != NIGHT skips the
    // scene-row caching entirely.
    public function test_routeWangan_dayOrEvening_skipsSceneCache(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 1); // ROUTE_WANGAN
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 0); // TIME_OF_DAY_DAY

        // macCpu1_0x24 doubles as the per-demo script table -- point it at a
        // real allocation, since TrafficInit_8c02769e itself both caches this
        // field into var_trafficPresetTable_8c227e18 AND immediately indexes through it below.
        $table = $this->alloc(4 * 4);
        $this->initUint32($table + 2 * 4, 0x99990000);

        $course = $this->addressOf('_var_currentCourse_8c1bb868');
        $this->initUint32($course + 0x1c, 0x11110000); // lineCpu_0x1c
        $this->initUint32($course + 0x24, $table); // macCpu1_0x24

        $this->initUint32($this->addressOf('_var_activeTrafficPreset_8c227e14'), 2);

        $task = $this->alloc(0x20);
        $state = $this->alloc(4);

        $this->call('_TrafficInit_8c02769e')->with();

        $this->shouldWriteLong($this->addressOf('_var_cpuPathBlocks_8c227e1c'), 0x11110000);
        $this->shouldWriteLong($this->addressOf('_var_trafficPresetTable_8c227e18'), $table);

        $this->shouldWriteLong($this->addressOf('_var_signalGroups_8c228b40'), $this->addressOf('_init_signalGroupsWangan_8c04c980'));

        $this->mockTaskInstall($task, $state);
        $this->shouldWriteLong($task + 0x18, 0x99990000);
        $this->shouldWriteLong($task + 0x08, 0);
        $this->shouldWriteLong($task + 0x0c, 1);

        $this->shouldCall('_SignalClearCrossingOccupied_8c028958');
    }

    // ROUTE_SHINJUKU (0) selects init_signalGroupsShinjuku_8c04caec.
    public function test_routeShinjuku_selectsCaec(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 0); // ROUTE_SHINJUKU
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 1); // TIME_OF_DAY_EVENING

        $table = $this->alloc(4);
        $this->initUint32($table, 0);

        $course = $this->addressOf('_var_currentCourse_8c1bb868');
        $this->initUint32($course + 0x1c, 0);
        $this->initUint32($course + 0x24, $table);

        $this->initUint32($this->addressOf('_var_activeTrafficPreset_8c227e14'), 0);

        $task = $this->alloc(0x20);
        $state = $this->alloc(4);

        $this->call('_TrafficInit_8c02769e')->with();

        $this->shouldWriteLong($this->addressOf('_var_cpuPathBlocks_8c227e1c'), 0);
        $this->shouldWriteLong($this->addressOf('_var_trafficPresetTable_8c227e18'), $table);

        $this->shouldWriteLong($this->addressOf('_var_signalGroups_8c228b40'), $this->addressOf('_init_signalGroupsShinjuku_8c04caec'));

        $this->mockTaskInstall($task, $state);
        $this->shouldWriteLong($task + 0x18, 0);
        $this->shouldWriteLong($task + 0x08, 0);
        $this->shouldWriteLong($task + 0x0c, 1);

        $this->shouldCall('_SignalClearCrossingOccupied_8c028958');
    }

    // ROUTE_OME (2) selects init_signalGroupsOme_8c04cd38 AND (timeOfDay == NIGHT) triggers
    // the scene-row cache + delta computation.
    public function test_routeOme_night_cachesSceneRowsAndDeltas(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 2); // ROUTE_OME
        $this->initUint32($this->addressOf('_var_timeOfDay_8c18ad20'), 2); // TIME_OF_DAY_NIGHT

        $table = $this->alloc(4);
        $this->initUint32($table, 0);

        $course = $this->addressOf('_var_currentCourse_8c1bb868');
        $this->initUint32($course + 0x1c, 0x33330000);
        $this->initUint32($course + 0x24, $table);

        $this->initUint32($this->addressOf('_var_activeTrafficPreset_8c227e14'), 0);

        $scene = $this->alloc(0x80);
        $this->initUint32($this->addressOf('_var_sceneParams_8c18ad24'), $scene);
        // row1 (index 1): the "day" cache; row2 (index 2): the "night" cache.
        $this->setSceneRow($scene, 1, [10.0, 20.0, 100.0, 200.0, 300.0]);
        $this->setSceneRow($scene, 2, [50.0, 60.0, 400.0, 500.0, 600.0]);

        $task = $this->alloc(0x20);
        $state = $this->alloc(4);

        $this->call('_TrafficInit_8c02769e')->with();

        $this->shouldWriteLong($this->addressOf('_var_cpuPathBlocks_8c227e1c'), 0x33330000);
        $this->shouldWriteLong($this->addressOf('_var_trafficPresetTable_8c227e18'), $table);

        $this->shouldWriteLong($this->addressOf('_var_signalGroups_8c228b40'), $this->addressOf('_init_signalGroupsOme_8c04cd38'));

        $b0 = $this->addressOf('_var_nightLightIntensityOn_8c1bbdb0');
        $d0 = $this->addressOf('_var_nightLightColorOn_8c1bbdd0');
        $a8 = $this->addressOf('_var_nightLightIntensityOff_8c1bbda8');
        $c4 = $this->addressOf('_var_nightLightColorOff_8c1bbdc4');
        $a0 = $this->addressOf('_var_nightLightIntensityStep_8c1bbda0');
        $b8 = $this->addressOf('_var_nightLightColorStep_8c1bbdb8');

        $this->shouldWriteFloatAt($b0 + 0, 50.0);
        $this->shouldWriteFloatAt($b0 + 4, 60.0);
        $this->shouldWriteFloatAt($d0 + 0, 400.0);
        $this->shouldWriteFloatAt($d0 + 4, 500.0);
        $this->shouldWriteFloatAt($d0 + 8, 600.0);
        $this->shouldWriteFloatAt($a8 + 0, 10.0);
        $this->shouldWriteFloatAt($a8 + 4, 20.0);
        $this->shouldWriteFloatAt($c4 + 0, 100.0);
        $this->shouldWriteFloatAt($c4 + 4, 200.0);
        $this->shouldWriteFloatAt($c4 + 8, 300.0);

        $this->shouldWriteFloatAt($a0 + 0, (50.0 - 10.0) / 20.0);
        $this->shouldWriteFloatAt($a0 + 4, (60.0 - 20.0) / 20.0);
        $this->shouldWriteFloatAt($b8 + 0, (400.0 - 100.0) / 20.0);
        $this->shouldWriteFloatAt($b8 + 4, (500.0 - 200.0) / 20.0);
        $this->shouldWriteFloatAt($b8 + 8, (600.0 - 300.0) / 20.0);

        $this->mockTaskInstall($task, $state);
        $this->shouldWriteLong($task + 0x18, 0);
        $this->shouldWriteLong($task + 0x08, 0);
        $this->shouldWriteLong($task + 0x0c, 1);

        $this->shouldCall('_SignalClearCrossingOccupied_8c028958');
    }
};
