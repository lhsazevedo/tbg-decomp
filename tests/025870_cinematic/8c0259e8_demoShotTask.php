<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

if (!function_exists('fdec')) {
    function fdec(float $value): int
    {
        return unpack('L', pack('f', $value))[1];
    }
}

/*
 * _demoShotTask_8c0259e8(Task *task, void *state): the attract-mode camera
 * tour armed by CinematicStartTour_8c025af4. Its trigger is the camera cue in the
 * top byte of var_busState_8c1bb9d0.markDriveFlags_0x3b0. Phase 0 watches for that id to
 * change (or var_demoShotRearm_8c227e10 to force a re-trigger of the last
 * one): looks up the matching shot in var_demoShots_8c227e0c, switches the demo
 * camera into record.kind+5 (delegating the actual position update to
 * applyShotPosition_8c0258ba), opens (or clears) the message box for the record's name,
 * and moves to phase 1. Phase 1 waits for the id to drop back to 0. Every
 * call then advances the textbox's reveal counter (if a box is open) and
 * reschedules BusCameraDrawBusModel_8c024bb8 on draw layer 0.
 */
return new class extends TestCase {
    // State offsets (word index * 4).
    const ST_PHASE = 0x00;
    const ST_REVEAL = 0x04;
    const ST_HANDLE = 0x08;

    private function f32(float $value): float
    {
        return unpack('f', pack('f', $value))[1];
    }

    private function oddMvn(): Closure
    {
        return function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8(
                    $dst->value + $i, $this->readUInt8($src->value + $i)
                );
            }
        };
    }

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
        $this->setSize('_MessageBoxSetText_8c02aefc', 4);
        $this->setSize('_MessageBoxDrawText_8c02af1c', 4);
        $this->setSize('_MessageBoxOpenTextbox_8c02ae3e', 4);
        $this->setSize('_BusCameraDrawBusModel_8c024bb8', 4);
        $this->setSize('_RenderQueueDraw_8c0223ea', 4);
        $this->setSize('_TaskInstall_8c014ae8', 4);
    }

    /** @param array<array{0:int,1:float,2:float,3:float,4:string}> $records */
    private function buildTable(array $records): int
    {
        $base = $this->alloc(count($records) * 0x14);

        foreach ($records as $i => [$kind, $x, $y, $z, $name]) {
            $rec = $base + $i * 0x14;
            $this->initUint32($rec + 0x00, $kind);
            $this->initUint32($rec + 0x04, fdec($x));
            $this->initUint32($rec + 0x08, fdec($y));
            $this->initUint32($rec + 0x0c, fdec($z));
            $this->initUint32($rec + 0x10, $this->allocString($name));
        }

        return $base;
    }

    private function allocState(int $phase, int $reveal = 0, int $handle = 0): int
    {
        $state = $this->alloc(0xc);
        $this->initUint32($state + self::ST_PHASE, $phase);
        $this->initUint32($state + self::ST_REVEAL, $reveal);
        $this->initUint32($state + self::ST_HANDLE, $handle);
        return $state;
    }

    private function setMarker(int $stopId): void
    {
        $this->initUint32(($this->addressOf('_var_busState_8c1bb9d0') + 0x3b0), $stopId << 24);
    }

    public function test_phase0_nothing_pending_is_noop(): void
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 0);

        $this->setMarker(0);
        $this->initUint32($this->addressOf('_var_demoShotRearm_8c227e10'), 0);

        $this->call('_demoShotTask_8c0259e8')->with($task, $state);

        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(0, $this->addressOf('_BusCameraDrawBusModel_8c024bb8'), 0);
    }

    public function test_phase0_new_stop_opens_box_and_positions_state5(): void
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 0);

        $table = $this->buildTable([
            [9, 0.0, 0.0, 0.0, ''], // index 0: unused (a 0 marker byte means "nothing pending")
            [0, 10.0, 2.5, -30.0, 'Ichinoseki'],
        ]);
        $this->initUint32($this->addressOf('_var_demoShots_8c227e0c'), $table);
        $this->initUint32($this->addressOf('_var_demoShotId_8c227dd4'), -1);
        $this->initUint32($this->addressOf('_var_demoShotRearm_8c227e10'), 0);
        $this->setMarker(1); // marker 1 -> shot 1

        $busState = $this->addressOf('_var_busState_8c1bb9d0');
        $this->initUint32($busState + 0xf8, fdec(5.0)); // posY_0x0f8

        $this->call('_demoShotTask_8c0259e8')->with($task, $state);

        $this->shouldWriteLongTo('_var_demoShotId_8c227dd4', 1);
        $this->shouldWriteLongTo('_var_cameraMode_8c227d9c', 5); // kind(0) + 5
        $this->shouldCall('__quick_odd_mvn')->do($this->oddMvn());
        $this->shouldCall('_MessageBoxSetText_8c02aefc')
            ->with('Ichinoseki')
            ->andReturn(42);
        $this->shouldWriteLong($state + self::ST_HANDLE, 42);
        $this->shouldWriteLong($state + self::ST_REVEAL, 0);
        // applyShotPosition_8c0258ba positions the bus draw point for the new camera
        // state; its own behavior (per var_cameraMode_8c227d9c) is covered by
        // 8c0258ba_FUN.php.
        $this->shouldCall('_applyShotPosition_8c0258ba');
        $this->shouldWriteLong($state + self::ST_PHASE, 1);
        $this->shouldWriteLong($state + self::ST_REVEAL, 1);
        $this->shouldCall('_MessageBoxDrawText_8c02af1c')->with(0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(0, $this->addressOf('_BusCameraDrawBusModel_8c024bb8'), 0);
    }

    public function test_phase0_new_stop_with_empty_name_skips_box(): void
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 0);

        $table = $this->buildTable([
            [9, 0.0, 0.0, 0.0, ''], // index 0: unused (a 0 marker byte means "nothing pending")
            [2, 1.0, 2.0, 3.0, ''],
        ]);
        $this->initUint32($this->addressOf('_var_demoShots_8c227e0c'), $table);
        $this->initUint32($this->addressOf('_var_demoShotId_8c227dd4'), -1);
        $this->initUint32($this->addressOf('_var_demoShotRearm_8c227e10'), 0);
        $this->setMarker(1);

        $this->call('_demoShotTask_8c0259e8')->with($task, $state);

        $this->shouldWriteLongTo('_var_demoShotId_8c227dd4', 1);
        $this->shouldWriteLongTo('_var_cameraMode_8c227d9c', 7); // kind(2) + 5; state7 is a no-op in applyShotPosition_8c0258ba
        $this->shouldCall('__quick_odd_mvn')->do($this->oddMvn());
        $this->shouldWriteLong($state + self::ST_HANDLE, 0);
        $this->shouldCall('_applyShotPosition_8c0258ba');
        $this->shouldWriteLong($state + self::ST_PHASE, 1);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(0, $this->addressOf('_BusCameraDrawBusModel_8c024bb8'), 0);
    }

    public function test_phase0_same_stop_id_is_noop(): void
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 0, handle: 0);

        $this->initUint32($this->addressOf('_var_demoShotId_8c227dd4'), 3);
        $this->initUint32($this->addressOf('_var_demoShotRearm_8c227e10'), 0);
        $this->setMarker(3); // same as the cached var_demoShotId_8c227dd4 -> no cut

        $this->call('_demoShotTask_8c0259e8')->with($task, $state);

        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(0, $this->addressOf('_BusCameraDrawBusModel_8c024bb8'), 0);
    }

    public function test_phase0_rearm_forces_last_stop_and_positions_state6(): void
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 0);

        $table = $this->buildTable([
            [0, 0.0, 0.0, 0.0, ''],
            [0, 0.0, 0.0, 0.0, ''],
            [0, 0.0, 0.0, 0.0, ''],
            [0, 0.0, 0.0, 0.0, ''],
            [0, 0.0, 0.0, 0.0, ''],
            [1, 40.0, 5.0, -10.0, 'Nishi-Nippori'],
        ]);
        $this->initUint32($this->addressOf('_var_demoShots_8c227e0c'), $table);
        // var_demoShotRearm_8c227e10 forces a cut to the pre-seeded shot (index 5),
        // even though the marker byte is currently 0.
        $this->initUint32($this->addressOf('_var_demoShotId_8c227dd4'), 5);
        $this->initUint32($this->addressOf('_var_demoShotRearm_8c227e10'), 1);
        $this->setMarker(0);

        $this->call('_demoShotTask_8c0259e8')->with($task, $state);

        $this->shouldWriteLongTo('_var_demoShotId_8c227dd4', -1);
        $this->shouldWriteLongTo('_var_demoShotRearm_8c227e10', 0);
        $this->shouldWriteLongTo('_var_demoShotId_8c227dd4', 5);
        $this->shouldWriteLongTo('_var_cameraMode_8c227d9c', 6); // kind(1) + 5
        $this->shouldCall('__quick_odd_mvn')->do($this->oddMvn());
        $this->shouldCall('_MessageBoxSetText_8c02aefc')
            ->with('Nishi-Nippori')
            ->andReturn(7);
        $this->shouldWriteLong($state + self::ST_HANDLE, 7);
        $this->shouldWriteLong($state + self::ST_REVEAL, 0);
        // applyShotPosition_8c0258ba positions the bus draw point for state6 (transformed
        // by the world matrix via njCalcPoint); its own behavior is covered
        // by 8c0258ba_FUN.php.
        $this->shouldCall('_applyShotPosition_8c0258ba');
        $this->shouldWriteLong($state + self::ST_PHASE, 1);
        $this->shouldWriteLong($state + self::ST_REVEAL, 1);
        $this->shouldCall('_MessageBoxDrawText_8c02af1c')->with(0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(0, $this->addressOf('_BusCameraDrawBusModel_8c024bb8'), 0);
    }

    public function test_phase1_marker_cleared_resets_to_phase0(): void
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 1, handle: 0);

        $this->setMarker(0);

        $this->call('_demoShotTask_8c0259e8')->with($task, $state);

        $this->shouldWriteLong($state + self::ST_PHASE, 0);
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(0, $this->addressOf('_BusCameraDrawBusModel_8c024bb8'), 0);
    }

    public function test_phase1_marker_still_set_stays_and_advances_reveal(): void
    {
        $this->resolveSymbols();

        $task = $this->alloc(0x20);
        $state = $this->allocState(phase: 1, reveal: 3, handle: 9);

        $this->setMarker(1);

        $this->call('_demoShotTask_8c0259e8')->with($task, $state);

        // No phase write: the strict expectation order below would fail if
        // one were emitted here.
        $this->shouldWriteLong($state + self::ST_REVEAL, 4);
        $this->shouldCall('_MessageBoxDrawText_8c02af1c')->with(2); // 4 >> 1
        $this->shouldCall('_RenderQueueDraw_8c0223ea')->with(0, $this->addressOf('_BusCameraDrawBusModel_8c024bb8'), 0);
    }
};
