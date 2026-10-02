<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

// StopSpawnInit_8c02d968: course-start setup for the bus-stop passenger subsystem.
// See src/02d968_stop_spawn.h for what it does. Covers the demo-mode early return, route
// dispatch, waiting-passenger spawning (PassengerBoardTask_8c02d21c), the scripted
// schedule's unmatched-segment spawn (PassengerSeatedTask_8c02d5ca), the Fisher-Yates
// shuffle (_quick_evn_mvn), and the shuffled matched-segment spawn
// (PassengerExitTask_8c02d46c).

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_TaskSpawn_8c014ae8', 4);
        $this->setSize('_PassengerSkipStopTask_8c02d8f0', 4);
        $this->setSize('_var_tasks_8c1ba5e8', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_practiceRules_8c226410', 4);
        $this->setSize('_var_runState_8c2285c4', 0x9c);
        // Loaded into R12 unconditionally at function entry by the asm
        // object even though this test's path never uses it.
        $this->setSize('_var_exitSpot1_8c228934', 12);
    }

    // playMode == 1 (demo) and bit 3 of var_practiceRules_8c226410 clear: spawns the
    // "return to demo" task and sets var_runState_8c2285c4.runPhase_0x00 = 2, without doing
    // any of the normal course-start spawning.
    public function test_demoModeEarlyReturn(): void {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 1);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 0);

        $task = $this->alloc(0x20);
        $state = $this->alloc(0x20);

        $this->call('_StopSpawnInit_8c02d968');

        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with(
                $this->addressOf('_var_tasks_8c1ba5e8'),
                $this->addressOf('_PassengerSkipStopTask_8c02d8f0'),
            )
            ->do(function () use ($task, $state) {
                $this->memory->writeUInt32($this->getRegister(6)->value, U32::of($task));
                $this->memory->writeUInt32($this->getRegister(7)->value, U32::of($state));
            })
            ->andReturn(1);

        $this->shouldWriteLong($this->addressOf('_var_runState_8c2285c4'), 2);

        $this->forceStop();
    }

    private function f32(float $value): float {
        return unpack('f', pack('f', $value))[1];
    }

    private function initFloat(int $addr, float $value): void {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    // Not demo mode, no waiting passengers and no scripted schedule slots in
    // use: exercises the main task spawn, the interior texture/camera setup
    // calls, the route-dependent anchor-point branch (all six interior
    // points plus both njCalcPoint transforms), and the (empty) spawn
    // loops. $route selects ROUTE_SHINJUKU (0)/ROUTE_WANGAN (1) vs
    // ROUTE_OME (2); $pts gives the six points' expected (x, y, z) in
    // var_boardSpot1_8c228928/910/91c/934/940/94c order. A null $pts (for a route
    // value with no dispatch arm) skips the whole anchor-point block.
    private function runEmptyScheduleRoute(int $route, ?array $pts): void {
        $this->setSize('_TaskSpawn_8c014ae8', 4);
        $this->setSize('_PassengerStopSceneTask_8c02d644', 4);
        $this->setSize('_var_tasks_8c1ba5e8', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_practiceRules_8c226410', 4);
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njLoadCacheTexture', 4);
        $this->setSize('_CinematicBoardingCamera_8c025870', 4);
        $this->setSize('_var_interiorTexlist_8c1bc438', 4);
        $this->setSize('_var_route_8c18ad1c', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_njCalcPoint', 4);
        $this->setSize('_var_boardSpot1_8c228928', 12);
        $this->setSize('_var_boardSpot2_8c228910', 12);
        $this->setSize('_var_boardSpot3_8c22891c', 12);
        $this->setSize('_var_exitSpot1_8c228934', 12);
        $this->setSize('_var_exitSpot2_8c228940', 12);
        $this->setSize('_var_exitSpot3_8c22894c', 12);
        $this->setSize('_var_passengerFadeColor_8c228960', 20);
        $this->setSize('_var_passengersFadedOut_8c22895c', 4);
        $this->setSize('_var_seats_8c228718', 31 * 4);
        $this->setSize('_var_waitingPassengerCount_8c228794', 4);
        $this->setSize('_syMalloc', 4);
        $this->setSize('_TaskInitGroup_8c014a9c', 4);
        $this->setSize('_var_stopTaskGroup_8c2288f8', 4);
        $this->setSize('_syFree', 4);
        $this->setSize('_rand', 4);
        $this->setSize('_AsqGetRandomInRangeA_8c012178', 4);
        $this->setSize('_var_waitingPassengers_8c228798', 20 * 16);
        $this->setSize('_var_passengerCount_8c1bb8e4', 4);
        $this->setSize('_init_seatPositions_8c04c3e4', 8 * 31);
        $this->setSize('_var_currentSegment_8c228708', 4);
        $this->setSize('_PassengerSeatedTask_8c02d5ca', 4);
        $this->setSize('_PassengerBoardTask_8c02d21c', 4);
        $this->setSize('_PassengerExitTask_8c02d46c', 4);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0); // not demo mode
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), $route);
        $this->initUint32($this->addressOf('_var_waitingPassengerCount_8c228794'), 0); // no waiting passengers

        $interiorTexlist = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_interiorTexlist_8c1bc438'), $interiorTexlist);

        $scheduleBase = $this->addressOf('_var_seats_8c228718');
        for ($i = 0; $i < 31; $i++) {
            $this->initUint32($scheduleBase + $i * 4, 0xffffffff); // unused
        }

        $mainState = $this->alloc(8);
        $matchedBuf = $this->alloc(0xf8);
        $group = $this->alloc(0x20);

        $this->call('_StopSpawnInit_8c02d968');

        $mainTask = $this->alloc(0x20);
        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with($this->addressOf('_var_tasks_8c1ba5e8'), $this->addressOf('_PassengerStopSceneTask_8c02d644'))
            ->do(function () use ($mainTask, $mainState) {
                $this->memory->writeUInt32($this->getRegister(6)->value, U32::of($mainTask));
                $this->memory->writeUInt32($this->getRegister(7)->value, U32::of($mainState));
            })
            ->andReturn(1);
        $this->shouldWriteLong($mainState + 0, 0);
        $this->shouldWriteLong($mainState + 4, 0);

        $this->shouldCall('_njSetTexture')->with($interiorTexlist);
        $this->shouldCall('_njLoadCacheTexture')->with($interiorTexlist);
        $this->shouldCall('_CinematicBoardingCamera_8c025870');

        // Six bus-interior anchor points, each written x, then y, then z
        // (verified against the real instruction order for ROUTE_SHINJUKU,
        // which does not match Ghidra's statement order for this block).
        // A route with no dispatch arm (e.g. an out-of-range value) skips
        // this whole block, including both njCalcPoint transforms.
        if ($pts !== null) {
            $matrix = ($this->addressOf('_var_busState_8c1bb9d0') + 0x084);
            $addrs = [
                $this->addressOf('_var_boardSpot1_8c228928'),
                $this->addressOf('_var_boardSpot2_8c228910'),
                $this->addressOf('_var_boardSpot3_8c22891c'),
                $this->addressOf('_var_exitSpot1_8c228934'),
                $this->addressOf('_var_exitSpot2_8c228940'),
                $this->addressOf('_var_exitSpot3_8c22894c'),
            ];

            foreach ($addrs as $i => $addr) {
                [$x, $y, $z] = $pts[$i];
                $this->shouldWriteFloat($addr + 0, $this->f32($x));
                $this->shouldWriteFloat($addr + 4, $this->f32($y));
                $this->shouldWriteFloat($addr + 8, $this->f32($z));
                if ($i === 0 || $i === 5) {
                    $this->shouldCall('_njCalcPoint')->with($matrix, $addr, $addr);
                }
            }
        }

        $p960 = $this->addressOf('_var_passengerFadeColor_8c228960');
        $this->shouldWriteLongTo('_var_passengersFadedOut_8c22895c', 0);
        $this->shouldWriteFloat($p960 + 0, $this->f32(0.0));
        $this->shouldWriteFloat($p960 + 4, $this->f32(1.0));
        $this->shouldWriteFloat($p960 + 8, $this->f32(1.0));
        $this->shouldWriteFloat($p960 + 0xc, $this->f32(1.0));

        $this->shouldCall('_syMalloc')->with(0x20)->andReturn($group);
        $this->shouldWriteLongTo('_var_stopTaskGroup_8c2288f8', $group);
        $this->shouldCall('_TaskInitGroup_8c014a9c')->with($group, 0);

        $this->shouldCall('_syMalloc')->with(0xf8)->andReturn($matchedBuf);
        $this->shouldCall('_syFree')->with($matchedBuf);
    }

    public function test_emptyScheduleShinjukuRoute(): void {
        $this->runEmptyScheduleRoute(0, [ // ROUTE_SHINJUKU
            [-1.6, 0.0, 0.9],    // var_boardSpot1_8c228928
            [0.15, 0.68, 0.27],  // var_boardSpot2_8c228910
            [0.128, 0.68, 0.76], // var_boardSpot3_8c22891c
            [0.35, 0.68, 3.0],   // var_exitSpot1_8c228934
            [1.0, 0.35, 2.8],    // var_exitSpot2_8c228940
            [-1.39, 0.0, 2.63],  // var_exitSpot3_8c22894c
        ]);
    }

    public function test_emptyScheduleWanganRoute(): void {
        // ROUTE_WANGAN takes the same dispatch arm as ROUTE_SHINJUKU.
        $this->runEmptyScheduleRoute(1, [
            [-1.6, 0.0, 0.9],
            [0.15, 0.68, 0.27],
            [0.128, 0.68, 0.76],
            [0.35, 0.68, 3.0],
            [1.0, 0.35, 2.8],
            [-1.39, 0.0, 2.63],
        ]);
    }

    public function test_emptyScheduleOmeRoute(): void {
        $this->runEmptyScheduleRoute(2, [ // ROUTE_OME
            [-1.39, 0.0, 2.63],  // var_boardSpot1_8c228928
            [1.0, 0.35, 2.8],    // var_boardSpot2_8c228910
            [0.35, 0.68, 3.0],   // var_boardSpot3_8c22891c
            [0.128, 0.68, 0.76], // var_exitSpot1_8c228934
            [0.15, 0.68, 0.27],  // var_exitSpot2_8c228940
            [-1.6, 0.0, 0.9],    // var_exitSpot3_8c22894c
        ]);
    }

    // A route value with no dispatch arm (only SHINJUKU/WANGAN/OME exist in
    // practice, but the C preserves the original branch structure) skips
    // the whole anchor-point block entirely.
    public function test_emptyScheduleUnknownRoute(): void {
        $this->runEmptyScheduleRoute(3, null);
    }

    // One already-picked waiting passenger (var_waitingPassengerCount_8c228794 == 1), no scripted
    // schedule slots: exercises the PassengerBoardTask_8c02d21c spawn loop's per-iteration
    // field writes and rand()-derived random offsets.
    public function test_oneWaitingPassenger(): void {
        $this->setSize('_TaskSpawn_8c014ae8', 4);
        $this->setSize('_PassengerStopSceneTask_8c02d644', 4);
        $this->setSize('_PassengerBoardTask_8c02d21c', 4);
        $this->setSize('_var_tasks_8c1ba5e8', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_practiceRules_8c226410', 4);
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njLoadCacheTexture', 4);
        $this->setSize('_CinematicBoardingCamera_8c025870', 4);
        $this->setSize('_var_interiorTexlist_8c1bc438', 4);
        $this->setSize('_var_route_8c18ad1c', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_njCalcPoint', 4);
        $this->setSize('_var_boardSpot1_8c228928', 12);
        $this->setSize('_var_boardSpot2_8c228910', 12);
        $this->setSize('_var_boardSpot3_8c22891c', 12);
        $this->setSize('_var_exitSpot1_8c228934', 12);
        $this->setSize('_var_exitSpot2_8c228940', 12);
        $this->setSize('_var_exitSpot3_8c22894c', 12);
        $this->setSize('_var_passengerFadeColor_8c228960', 20);
        $this->setSize('_var_passengersFadedOut_8c22895c', 4);
        $this->setSize('_var_seats_8c228718', 31 * 4);
        $this->setSize('_var_waitingPassengerCount_8c228794', 4);
        $this->setSize('_var_waitingPassengers_8c228798', 20);
        $this->setSize('_var_passengerCount_8c1bb8e4', 4);
        $this->setSize('_syMalloc', 4);
        $this->setSize('_TaskInitGroup_8c014a9c', 4);
        $this->setSize('_var_stopTaskGroup_8c2288f8', 4);
        $this->setSize('_syFree', 4);
        $this->setSize('_rand', 4);
        $this->setSize('__quick_odd_mvn', 4);

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 3); // skip anchor points
        $this->initUint32($this->addressOf('_var_waitingPassengerCount_8c228794'), 1);
        $this->initUint32($this->addressOf('_var_passengerCount_8c1bb8e4'), 5);

        $scheduleBase = $this->addressOf('_var_seats_8c228718');
        for ($i = 0; $i < 31; $i++) {
            $this->initUint32($scheduleBase + $i * 4, 0xffffffff);
        }

        // WaitingPassengerSlot: spot_0x00, NJS_POINT3 pos_0x04, index_0x10.
        $slotBase = $this->addressOf('_var_waitingPassengers_8c228798');
        $spot = $this->alloc(4);
        $this->initUint32($slotBase + 0x00, $spot);
        $this->initFloat($slotBase + 0x04, 1.5);
        $this->initFloat($slotBase + 0x08, 0.0);
        $this->initFloat($slotBase + 0x0c, -2.5);
        $this->initUint8($spot, 2); // *(char*)spot -> init_passengerVoiceVariant_8c04c4dc index

        $interiorTexlist = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_interiorTexlist_8c1bc438'), $interiorTexlist);

        $mainState = $this->alloc(8);
        $mainTask = $this->alloc(0x20);
        $group = $this->alloc(0x20);
        $matchedBuf = $this->alloc(0xf8);
        $state = $this->alloc(0x38);
        $task = $this->alloc(0x20);

        $this->call('_StopSpawnInit_8c02d968');

        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with($this->addressOf('_var_tasks_8c1ba5e8'), $this->addressOf('_PassengerStopSceneTask_8c02d644'))
            ->do(function () use ($mainTask, $mainState) {
                $this->memory->writeUInt32($this->getRegister(6)->value, U32::of($mainTask));
                $this->memory->writeUInt32($this->getRegister(7)->value, U32::of($mainState));
            })
            ->andReturn(1);
        $this->shouldWriteLong($mainState + 0, 0);
        $this->shouldWriteLong($mainState + 4, 0);

        $this->shouldCall('_njSetTexture')->with($interiorTexlist);
        $this->shouldCall('_njLoadCacheTexture')->with($interiorTexlist);
        $this->shouldCall('_CinematicBoardingCamera_8c025870');

        $p960 = $this->addressOf('_var_passengerFadeColor_8c228960');
        $this->shouldWriteLongTo('_var_passengersFadedOut_8c22895c', 0);
        $this->shouldWriteFloat($p960 + 0, $this->f32(0.0));
        $this->shouldWriteFloat($p960 + 4, $this->f32(1.0));
        $this->shouldWriteFloat($p960 + 8, $this->f32(1.0));
        $this->shouldWriteFloat($p960 + 0xc, $this->f32(1.0));

        $this->shouldCall('_syMalloc')->with((1 + 0 + 1) * 0x20)->andReturn($group);
        $this->shouldWriteLongTo('_var_stopTaskGroup_8c2288f8', $group);
        $this->shouldCall('_TaskInitGroup_8c014a9c')->with($group, 1);

        // init_passengerVoiceVariant_8c04c4dc[2] == 0x02 -> 0x02 + 0x32 = 0x34.
        $rand1 = 1000;
        $rand2 = 2000;
        $expectRandX = $this->f32(((float)$rand1 / 32768.0) * 0.2);
        $expectRandZ = $this->f32(((float)$rand2 / 32768.0) * 0.2);

        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with($group, $this->addressOf('_PassengerBoardTask_8c02d21c'))
            ->do(function () use ($task, $state) {
                $this->memory->writeUInt32($this->getRegister(6)->value, U32::of($task));
                $this->memory->writeUInt32($this->getRegister(7)->value, U32::of($state));
            })
            ->andReturn(1);
        $this->shouldWriteLong($state + 0x00, $spot);
        $this->shouldWriteLong($state + 0x04, 1);
        // pos_0x08..0x14 = var_waitingPassengers_8c228798[0].pos_0x04 (NJS_POINT3 copy), via
        // the SHC runtime struct-copy helper: dest in R1, src in R2, byte
        // count in R0 (see docs/lessons_learned.md).
        $this->shouldCall('__quick_odd_mvn')->do(function () use ($state, $slotBase) {
            $got = sprintf('dest=%08x src=%08x n=%d',
                $this->getRegister(1)->value, $this->getRegister(2)->value, $this->getRegister(0)->value);
            $want = sprintf('dest=%08x src=%08x n=12', $state + 0x08, $slotBase + 0x04);
            if ($got !== $want) {
                throw new \RuntimeException("_quick_odd_mvn: expected $want, got $got");
            }
        });
        $this->shouldCall('_rand')->andReturn($rand1);
        $this->shouldWriteFloat($state + 0x18, $expectRandX);
        $this->shouldCall('_rand')->andReturn($rand2);
        $this->shouldWriteFloat($state + 0x1c, $expectRandZ);
        $this->shouldWriteLong($state + 0x20, 0); // index
        $this->shouldWriteLong($state + 0x28, 0);
        $this->shouldWriteLong($state + 0x2c, 0); // 0 % 4
        $this->shouldWriteLong($state + 0x30, 0x34);
        $this->shouldWriteLong($state + 0x34, 0x38);
        $this->shouldWriteLongTo('_var_passengerCount_8c1bb8e4', 6);

        $this->shouldCall('_syMalloc')->with(0xf8)->andReturn($matchedBuf);
        $this->shouldCall('_syFree')->with($matchedBuf);
    }

    private function commonSpawnLoopSymbols(): void {
        $this->setSize('_TaskSpawn_8c014ae8', 4);
        $this->setSize('_PassengerStopSceneTask_8c02d644', 4);
        $this->setSize('_PassengerSeatedTask_8c02d5ca', 4);
        $this->setSize('_PassengerBoardTask_8c02d21c', 4);
        $this->setSize('_PassengerExitTask_8c02d46c', 4);
        $this->setSize('_var_tasks_8c1ba5e8', 4);
        $this->setSize('_var_playMode_8c1bb8d0', 4);
        $this->setSize('_var_practiceRules_8c226410', 4);
        $this->setSize('_njSetTexture', 4);
        $this->setSize('_njLoadCacheTexture', 4);
        $this->setSize('_CinematicBoardingCamera_8c025870', 4);
        $this->setSize('_var_interiorTexlist_8c1bc438', 4);
        $this->setSize('_var_route_8c18ad1c', 4);
        $this->setSize('_var_busState_8c1bb9d0', 0x3cc);
        $this->setSize('_njCalcPoint', 4);
        $this->setSize('_var_boardSpot1_8c228928', 12);
        $this->setSize('_var_boardSpot2_8c228910', 12);
        $this->setSize('_var_boardSpot3_8c22891c', 12);
        $this->setSize('_var_exitSpot1_8c228934', 12);
        $this->setSize('_var_exitSpot2_8c228940', 12);
        $this->setSize('_var_exitSpot3_8c22894c', 12);
        $this->setSize('_var_passengerFadeColor_8c228960', 20);
        $this->setSize('_var_passengersFadedOut_8c22895c', 4);
        $this->setSize('_var_seats_8c228718', 31 * 4);
        $this->setSize('_var_waitingPassengerCount_8c228794', 4);
        $this->setSize('_var_waitingPassengers_8c228798', 4);
        $this->setSize('_var_passengerCount_8c1bb8e4', 4);
        $this->setSize('_syMalloc', 4);
        $this->setSize('_TaskInitGroup_8c014a9c', 4);
        $this->setSize('_var_stopTaskGroup_8c2288f8', 4);
        $this->setSize('_syFree', 4);
        $this->setSize('_rand', 4);
        $this->setSize('_AsqGetRandomInRangeA_8c012178', 4);
        $this->setSize('__quick_evn_mvn', 4);
        $this->setSize('_init_seatPositions_8c04c3e4', 8 * 31);
        $this->setSize('_var_currentSegment_8c228708', 4);
    }

    // A scripted-schedule slot whose stop segment does NOT match the bus's
    // current segment is spawned immediately via PassengerSeatedTask_8c02d5ca (never
    // collected into the shuffle buffer). Exercises the previously
    // UNVERIFIED "no match" branch: field write order is ref_0x00,
    // field_0x08, field_0x0c (= var_exitSpot1_8c228934.y), field_0x10, field_0x14
    // (dispatched on the schedule index), a conditional re-write of
    // field_0x0c (-= 0.18f, only for index < 0x14), then field_0x28 = 1.
    // No field_0x18/0x1c/0x20/0x24/0x2c/0x30/0x34 writes and no rand()
    // calls happen on this path (those belong to the other two spawns).
    public function test_scriptedScheduleUnmatchedSlot(): void {
        $this->commonSpawnLoopSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 3); // skip anchor points
        $this->initUint32($this->addressOf('_var_waitingPassengerCount_8c228794'), 0); // no waiting passengers
        $this->initUint32($this->addressOf('_var_currentSegment_8c228708'), 5);

        $scheduleBase = $this->addressOf('_var_seats_8c228718');
        for ($i = 0; $i < 31; $i++) {
            $this->initUint32($scheduleBase + $i * 4, 0xffffffff);
        }

        // Slot 5 (< 0x14): stop segment 7 != current segment 5 -> no match.
        $entry = $this->alloc(2);
        $this->initUint8($entry + 0, 9); // stop-type byte; unused on this path
        $this->initUint8($entry + 1, 7); // segment byte
        $this->initUint32($scheduleBase + 5 * 4, $entry);

        $this->initFloat($this->addressOf('_var_exitSpot1_8c228934') + 4, 1.25); // .y

        $init3e4Base = $this->addressOf('_init_seatPositions_8c04c3e4');
        $this->initUint32($init3e4Base + 5 * 8 + 0, 0x1234abcd); // field_0x00 (int, raw-copied)
        $this->initFloat($init3e4Base + 5 * 8 + 4, 0.5);          // field_0x04

        $interiorTexlist = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_interiorTexlist_8c1bc438'), $interiorTexlist);

        $mainState = $this->alloc(8);
        $mainTask = $this->alloc(0x20);
        $group = $this->alloc(0x20);
        $matchedBuf = $this->alloc(0xf8);
        $state = $this->alloc(0x38);
        $task = $this->alloc(0x20);

        $this->call('_StopSpawnInit_8c02d968');

        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with($this->addressOf('_var_tasks_8c1ba5e8'), $this->addressOf('_PassengerStopSceneTask_8c02d644'))
            ->do(function () use ($mainTask, $mainState) {
                $this->memory->writeUInt32($this->getRegister(6)->value, U32::of($mainTask));
                $this->memory->writeUInt32($this->getRegister(7)->value, U32::of($mainState));
            })
            ->andReturn(1);
        $this->shouldWriteLong($mainState + 0, 0);
        $this->shouldWriteLong($mainState + 4, 0);

        $this->shouldCall('_njSetTexture')->with($interiorTexlist);
        $this->shouldCall('_njLoadCacheTexture')->with($interiorTexlist);
        $this->shouldCall('_CinematicBoardingCamera_8c025870');

        $p960 = $this->addressOf('_var_passengerFadeColor_8c228960');
        $this->shouldWriteLongTo('_var_passengersFadedOut_8c22895c', 0);
        $this->shouldWriteFloat($p960 + 0, $this->f32(0.0));
        $this->shouldWriteFloat($p960 + 4, $this->f32(1.0));
        $this->shouldWriteFloat($p960 + 8, $this->f32(1.0));
        $this->shouldWriteFloat($p960 + 0xc, $this->f32(1.0));

        // n = 1 used schedule slot -> group sized for (0 + 1 + 1) * 0x20.
        $this->shouldCall('_syMalloc')->with(0x40)->andReturn($group);
        $this->shouldWriteLongTo('_var_stopTaskGroup_8c2288f8', $group);
        $this->shouldCall('_TaskInitGroup_8c014a9c')->with($group, 1);

        $this->shouldCall('_syMalloc')->with(0xf8)->andReturn($matchedBuf);

        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with($group, $this->addressOf('_PassengerSeatedTask_8c02d5ca'))
            ->do(function () use ($task, $state) {
                $this->memory->writeUInt32($this->getRegister(6)->value, U32::of($task));
                $this->memory->writeUInt32($this->getRegister(7)->value, U32::of($state));
            })
            ->andReturn(1);
        $this->shouldWriteLong($state + 0x00, $entry);
        $this->shouldWriteLong($state + 0x08, 0x1234abcd);
        $this->shouldWriteFloat($state + 0x0c, $this->f32(1.25));
        $this->shouldWriteFloat($state + 0x10, $this->f32(0.5));
        $this->shouldWriteLong($state + 0x14, 0x20);
        $this->shouldWriteFloat($state + 0x0c, $this->f32($this->f32(1.25) - $this->f32(0.18)));
        $this->shouldWriteLong($state + 0x28, 1);

        // Shuffle/spawn loops are both empty (count == 0): no
        // AsqGetRandomInRangeA/_quick_evn_mvn/PassengerExitTask_8c02d46c calls.
        $this->shouldCall('_syFree')->with($matchedBuf);
    }

    // Simulates the __quick_evn_mvn 3-word-struct-copy runtime helper
    // (dest R1, src R2, byte count R0 -- see docs/lessons_learned.md), so
    // the Fisher-Yates shuffle below actually reorders the matched buffer
    // and the final PassengerExitTask_8c02d46c spawns can be asserted against the real
    // post-shuffle contents.
    private function simulateQuickEvnMvn(): void {
        $this->shouldCall('__quick_evn_mvn')->do(function () {
            $dst = $this->getRegister(1)->value;
            $src = $this->getRegister(2)->value;
            $len = $this->getRegister(0)->value;
            for ($i = 0; $i < $len; $i++) {
                $this->memory->writeUInt8($dst + $i, $this->memory->readUInt8($src + $i));
            }
        });
    }

    // Two scripted-schedule slots whose stop segment MATCHES the bus's
    // current segment are collected into the shuffle buffer instead of
    // being spawned directly, Fisher-Yates shuffled (AsqGetRandomInRangeA
    // + three _quick_evn_mvn struct copies per iteration: tmp = matched[i];
    // matched[i] = matched[randIdx]; matched[randIdx] = tmp), then spawned
    // via PassengerExitTask_8c02d46c in the shuffled order. randIdx is forced to 0 both
    // iterations, which reverses the two-element buffer -- so the spawn
    // loop's position-0 spawn must carry the ORIGINALLY-second slot and
    // vice versa, proving the shuffle (not just the field writes) is
    // correct. Exercises the previously UNVERIFIED shuffle and
    // shuffled-spawn field write order: ref_0x00, field_0x04 (= 0),
    // field_0x08, field_0x0c (= var_exitSpot1_8c228934.y), field_0x10, field_0x14
    // (+ conditional field_0x0c -= 0.18f), field_0x18/0x1c (rand()-derived),
    // field_0x20 (= spawn position), field_0x24 (= original schedule
    // index), field_0x28 (= 1), field_0x2c (= position % 4), field_0x30,
    // field_0x34 (= field_0x30 - 8).
    public function test_scriptedScheduleShuffleAndSpawn(): void {
        $this->commonSpawnLoopSymbols();

        $this->initUint32($this->addressOf('_var_playMode_8c1bb8d0'), 0);
        $this->initUint32($this->addressOf('_var_practiceRules_8c226410'), 0);
        $this->initUint32($this->addressOf('_var_route_8c18ad1c'), 3); // skip anchor points
        $this->initUint32($this->addressOf('_var_waitingPassengerCount_8c228794'), 0); // no waiting passengers
        $this->initUint32($this->addressOf('_var_currentSegment_8c228708'), 5);

        $scheduleBase = $this->addressOf('_var_seats_8c228718');
        for ($i = 0; $i < 31; $i++) {
            $this->initUint32($scheduleBase + $i * 4, 0xffffffff);
        }

        // Slot 2 and slot 7, both segment 5 -> both match current segment.
        // matched[0] = {entryA, index=2}; matched[1] = {entryB, index=7}.
        $entryA = $this->alloc(2);
        $this->initUint8($entryA + 0, 2); // init_passengerVoiceVariant_8c04c4dc[2] == 0x02
        $this->initUint8($entryA + 1, 5);
        $this->initUint32($scheduleBase + 2 * 4, $entryA);

        $entryB = $this->alloc(2);
        $this->initUint8($entryB + 0, 10); // init_passengerVoiceVariant_8c04c4dc[10] == 0x00
        $this->initUint8($entryB + 1, 5);
        $this->initUint32($scheduleBase + 7 * 4, $entryB);

        $this->initFloat($this->addressOf('_var_exitSpot1_8c228934') + 4, 1.25); // .y

        $init3e4Base = $this->addressOf('_init_seatPositions_8c04c3e4');
        // Indexed by SPAWN POSITION (0, 1), not the original schedule
        // index (2, 7) -- verified against the asm, not a Ghidra artifact.
        $this->initUint32($init3e4Base + 0 * 8 + 0, 0x1111);
        $this->initFloat($init3e4Base + 0 * 8 + 4, 0.25);
        $this->initUint32($init3e4Base + 1 * 8 + 0, 0x2222);
        $this->initFloat($init3e4Base + 1 * 8 + 4, 0.75);

        $interiorTexlist = $this->alloc(4);
        $this->initUint32($this->addressOf('_var_interiorTexlist_8c1bc438'), $interiorTexlist);

        $mainState = $this->alloc(8);
        $mainTask = $this->alloc(0x20);
        $group = $this->alloc(0x20);
        $matchedBuf = $this->alloc(0xf8);
        $state0 = $this->alloc(0x38);
        $task0 = $this->alloc(0x20);
        $state1 = $this->alloc(0x38);
        $task1 = $this->alloc(0x20);

        $this->call('_StopSpawnInit_8c02d968');

        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with($this->addressOf('_var_tasks_8c1ba5e8'), $this->addressOf('_PassengerStopSceneTask_8c02d644'))
            ->do(function () use ($mainTask, $mainState) {
                $this->memory->writeUInt32($this->getRegister(6)->value, U32::of($mainTask));
                $this->memory->writeUInt32($this->getRegister(7)->value, U32::of($mainState));
            })
            ->andReturn(1);
        $this->shouldWriteLong($mainState + 0, 0);
        $this->shouldWriteLong($mainState + 4, 0);

        $this->shouldCall('_njSetTexture')->with($interiorTexlist);
        $this->shouldCall('_njLoadCacheTexture')->with($interiorTexlist);
        $this->shouldCall('_CinematicBoardingCamera_8c025870');

        $p960 = $this->addressOf('_var_passengerFadeColor_8c228960');
        $this->shouldWriteLongTo('_var_passengersFadedOut_8c22895c', 0);
        $this->shouldWriteFloat($p960 + 0, $this->f32(0.0));
        $this->shouldWriteFloat($p960 + 4, $this->f32(1.0));
        $this->shouldWriteFloat($p960 + 8, $this->f32(1.0));
        $this->shouldWriteFloat($p960 + 0xc, $this->f32(1.0));

        // n = 2 used schedule slots -> group sized for (0 + 2 + 1) * 0x20.
        $this->shouldCall('_syMalloc')->with(0x60)->andReturn($group);
        $this->shouldWriteLongTo('_var_stopTaskGroup_8c2288f8', $group);
        $this->shouldCall('_TaskInitGroup_8c014a9c')->with($group, 2);

        $this->shouldCall('_syMalloc')->with(0xf8)->andReturn($matchedBuf);

        // The classify walk collects both matched slots into matchedBuf,
        // in schedule order (index 2 before index 7), before the shuffle.
        $this->shouldWriteLong($matchedBuf + 0 * 8 + 0, $entryA);
        $this->shouldWriteLong($matchedBuf + 0 * 8 + 4, 2);
        $this->shouldWriteLong($matchedBuf + 1 * 8 + 0, $entryB);
        $this->shouldWriteLong($matchedBuf + 1 * 8 + 4, 7);

        // Fisher-Yates shuffle, count == 2. randIdx forced to 0 on both
        // iterations: i=0 swaps matched[0] with itself (no-op); i=1 swaps
        // matched[1] with matched[0], reversing the pair.
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(2)->andReturn(0);
        $this->simulateQuickEvnMvn();
        $this->simulateQuickEvnMvn();
        $this->simulateQuickEvnMvn();
        $this->shouldCall('_AsqGetRandomInRangeA_8c012178')->with(2)->andReturn(0);
        $this->simulateQuickEvnMvn();
        $this->simulateQuickEvnMvn();
        $this->simulateQuickEvnMvn();

        // Spawn position 0 now carries the originally-second slot
        // (entryB / schedule index 7); position 1 carries the first
        // (entryA / schedule index 2) -- proving the reversal actually
        // happened, not just that the field writes look plausible.
        $rand0x = 100;
        $rand0z = 200;
        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with($group, $this->addressOf('_PassengerExitTask_8c02d46c'))
            ->do(function () use ($task0, $state0) {
                $this->memory->writeUInt32($this->getRegister(6)->value, U32::of($task0));
                $this->memory->writeUInt32($this->getRegister(7)->value, U32::of($state0));
            })
            ->andReturn(1);
        $this->shouldWriteLong($state0 + 0x00, $entryB);
        $this->shouldWriteLong($state0 + 0x04, 0);
        $this->shouldWriteLong($state0 + 0x08, 0x1111);
        $this->shouldWriteFloat($state0 + 0x0c, $this->f32(1.25));
        $this->shouldWriteFloat($state0 + 0x10, $this->f32(0.25));
        $this->shouldWriteLong($state0 + 0x14, 0x20);
        $this->shouldWriteFloat($state0 + 0x0c, $this->f32($this->f32(1.25) - $this->f32(0.18)));
        $this->shouldCall('_rand')->andReturn($rand0x);
        $this->shouldWriteFloat($state0 + 0x18, $this->f32(((float)$rand0x / 32768.0) * 0.2));
        $this->shouldCall('_rand')->andReturn($rand0z);
        $this->shouldWriteFloat($state0 + 0x1c, $this->f32(((float)$rand0z / 32768.0) * 0.2));
        $this->shouldWriteLong($state0 + 0x20, 0);
        $this->shouldWriteLong($state0 + 0x24, 7);
        $this->shouldWriteLong($state0 + 0x28, 1);
        $this->shouldWriteLong($state0 + 0x2c, 0);
        $this->shouldWriteLong($state0 + 0x30, 0x36); // init_passengerVoiceVariant_8c04c4dc[10] (0x00) + 0x36
        $this->shouldWriteLong($state0 + 0x34, 0x2e);

        $rand1x = 300;
        $rand1z = 400;
        $this->shouldCall('_TaskSpawn_8c014ae8')
            ->with($group, $this->addressOf('_PassengerExitTask_8c02d46c'))
            ->do(function () use ($task1, $state1) {
                $this->memory->writeUInt32($this->getRegister(6)->value, U32::of($task1));
                $this->memory->writeUInt32($this->getRegister(7)->value, U32::of($state1));
            })
            ->andReturn(1);
        $this->shouldWriteLong($state1 + 0x00, $entryA);
        $this->shouldWriteLong($state1 + 0x04, 0);
        $this->shouldWriteLong($state1 + 0x08, 0x2222);
        $this->shouldWriteFloat($state1 + 0x0c, $this->f32(1.25));
        $this->shouldWriteFloat($state1 + 0x10, $this->f32(0.75));
        $this->shouldWriteLong($state1 + 0x14, 0x20);
        $this->shouldWriteFloat($state1 + 0x0c, $this->f32($this->f32(1.25) - $this->f32(0.18)));
        $this->shouldCall('_rand')->andReturn($rand1x);
        $this->shouldWriteFloat($state1 + 0x18, $this->f32(((float)$rand1x / 32768.0) * 0.2));
        $this->shouldCall('_rand')->andReturn($rand1z);
        $this->shouldWriteFloat($state1 + 0x1c, $this->f32(((float)$rand1z / 32768.0) * 0.2));
        $this->shouldWriteLong($state1 + 0x20, 1);
        $this->shouldWriteLong($state1 + 0x24, 2);
        $this->shouldWriteLong($state1 + 0x28, 1);
        $this->shouldWriteLong($state1 + 0x2c, 1);
        $this->shouldWriteLong($state1 + 0x30, 0x38); // init_passengerVoiceVariant_8c04c4dc[2] (0x02) + 0x36
        $this->shouldWriteLong($state1 + 0x34, 0x30);

        $this->shouldCall('_syFree')->with($matchedBuf);
    }
};
