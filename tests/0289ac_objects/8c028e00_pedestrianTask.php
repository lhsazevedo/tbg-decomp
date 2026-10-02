<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    // PedestrianState offsets
    const OFF_SPRITE_X = 0x00;
    const OFF_SPRITE_Y = 0x04;
    const OFF_SPRITE_Z = 0x08;
    const OFF_BASE_X = 0x20;
    const OFF_BASE_Z = 0x24;
    const OFF_GROUND = 0x28;
    const OFF_GROUND_COUNT = 0x28 + 0x0c;
    const OFF_KIND = 0x3c;
    const OFF_REVERSE = 0x40;
    const OFF_PATH_NODE = 0x4c;
    const OFF_PATH_POS = 0x54;
    const OFF_SPEED = 0x58;
    const OFF_ANIM_PHASE = 0x5c;
    const OFF_NODE_FLAGS = 0x60;
    const OFF_STATE = 0x64;
    const OFF_SIGNAL_A = 0x68;
    const OFF_SIGNAL_B = 0x6c;

    private function makePed(): int
    {
        $ped = $this->alloc(0x70);
        $this->initUint32($ped + self::OFF_STATE, 0);
        $this->initUint32($ped + self::OFF_NODE_FLAGS, 0);
        $this->initUint32($ped + self::OFF_REVERSE, 0);
        $this->initUint32($ped + self::OFF_PATH_POS, fdec(0.0));
        $this->initUint32($ped + self::OFF_SPEED, fdec(0.0));
        $this->initUint32($ped + self::OFF_ANIM_PHASE, 0);
        $this->initUint32($ped + self::OFF_KIND, 0);
        return $ped;
    }

    private function resolveSymbols(): void
    {
        // The asm loads its address into a register on entry, called or not.
        $this->setSize('_SignalGetFrame_8c028900', 4);
    }

    /**
     * Checks _IntersectSegments_8c0206f0's R4/R5/R7/stack-arg operands (a0, a1, b1, out);
     * arg3 (pos, a stack local) is intentionally not checked, see caller.
     */
    private function assertCrossingArgs(int $expectedB1): Closure
    {
        $wantA0 = $this->addressOf('_var_busAxisA_8c228268');
        $wantA1 = $this->addressOf('_var_busAxisB_8c228270');
        $wantOut = $this->addressOf('_var_crossingIntersectPoint_8c1bc458');

        return function () use ($expectedB1, $wantA0, $wantA1, $wantOut) {
            $got = sprintf(
                'a0=%08x a1=%08x b1=%08x out=%08x',
                $this->registers[4]->value,
                $this->registers[5]->value,
                $this->registers[7]->value,
                $this->memory->readUInt32($this->registers[15]->value)->value,
            );
            $want = sprintf('a0=%08x a1=%08x b1=%08x out=%08x', $wantA0, $wantA1, $expectedB1, $wantOut);
            if ($got !== $want) {
                throw new RuntimeException("_IntersectSegments_8c0206f0: expected $want, got $got");
            }
        };
    }

    public function test_walking_forward_no_ground_snap()
    {
        $this->resolveSymbols();

        $ped = $this->makePed();
        $this->initUint32($ped + self::OFF_PATH_POS, fdec(10.0));
        $this->initUint32($ped + self::OFF_SPEED, fdec(1.5));
        $this->initUint32($ped + self::OFF_ANIM_PHASE, 3);

        $this->call('_pedestrianTask_8c028e00')->with(0, $ped);

        $this->shouldWriteFloat($ped + self::OFF_PATH_POS, 11.5);
        $this->shouldWriteLong($ped + self::OFF_ANIM_PHASE, 4);
        $this->shouldCall('_advancePedPathPos_8c0289ac')->with($ped)->andReturn(0);
    }

    public function test_walking_backward_no_ground_snap()
    {
        $this->resolveSymbols();

        $ped = $this->makePed();
        $this->initUint32($ped + self::OFF_REVERSE, 1);
        $this->initUint32($ped + self::OFF_PATH_POS, fdec(10.0));
        $this->initUint32($ped + self::OFF_SPEED, fdec(1.5));
        $this->initUint32($ped + self::OFF_ANIM_PHASE, 3);

        $this->call('_pedestrianTask_8c028e00')->with(0, $ped);

        $this->shouldWriteFloat($ped + self::OFF_PATH_POS, 8.5);
        $this->shouldWriteLong($ped + self::OFF_ANIM_PHASE, 4);
        $this->shouldCall('_advancePedPathPos_8c0289ac')->with($ped)->andReturn(0);
    }

    public function test_segment_change_triggers_ground_snap()
    {
        $this->resolveSymbols();

        $this->setSize('_GroundProbeTrackPolygon_8c020b6c', 4);
        $this->setSize('_GroundProbeInterpolateHeight_8c020f7e', 4);

        $ped = $this->makePed();
        $this->initUint32($ped + self::OFF_SPRITE_X, fdec(12.0));
        $this->initUint32($ped + self::OFF_SPRITE_Y, fdec(99.0));
        $this->initUint32($ped + self::OFF_SPRITE_Z, fdec(34.0));
        $this->initUint32($ped + self::OFF_BASE_X, fdec(2.0));
        $this->initUint32($ped + self::OFF_BASE_Z, fdec(4.0));

        $this->call('_pedestrianTask_8c028e00')->with(0, $ped);

        $this->shouldWriteFloat($ped + self::OFF_PATH_POS, 0.0);
        $this->shouldWriteLong($ped + self::OFF_ANIM_PHASE, 1);
        $this->shouldCall('_advancePedPathPos_8c0289ac')->with($ped)->andReturn(1);
        $this->shouldCall('_GroundProbeTrackPolygon_8c020b6c')
            ->with(10.0, 99.0, 30.0, $ped + self::OFF_GROUND)
            ->do(function () use ($ped) {
                $this->writeUInt32($ped + 0x28 + 0x0c, 0, U32::of(1));
            });
        $this->shouldCall('_GroundProbeInterpolateHeight_8c020f7e')->with($ped + self::OFF_GROUND, $ped);
    }

    public function test_pedestrian_kind_without_crossing_still_snaps_to_ground()
    {
        $this->resolveSymbols();

        $this->setSize('_GroundProbeTrackPolygon_8c020b6c', 4);
        $this->setSize('_GroundProbeInterpolateHeight_8c020f7e', 4);

        $ped = $this->makePed();
        $this->initUint32($ped + self::OFF_KIND, 1);
        $this->initUint32($ped + self::OFF_SPRITE_X, fdec(0.0));
        $this->initUint32($ped + self::OFF_SPRITE_Y, fdec(0.0));
        $this->initUint32($ped + self::OFF_SPRITE_Z, fdec(0.0));
        $this->initUint32($ped + self::OFF_BASE_X, fdec(0.0));
        $this->initUint32($ped + self::OFF_BASE_Z, fdec(0.0));

        $this->call('_pedestrianTask_8c028e00')->with(0, $ped);

        $this->shouldWriteFloat($ped + self::OFF_PATH_POS, 0.0);
        $this->shouldWriteLong($ped + self::OFF_ANIM_PHASE, 1);
        $this->shouldCall('_advancePedPathPos_8c0289ac')->with($ped)->andReturn(0);
        $this->shouldCall('_GroundProbeTrackPolygon_8c020b6c')
            ->with(0.0, 0.0, 0.0, $ped + self::OFF_GROUND)
            ->do(function () use ($ped) {
                $this->writeUInt32($ped + 0x28 + 0x0c, 0, U32::of(0));
            });
    }

    public function test_entering_signalled_node_computes_signal_ids_and_stays_waiting()
    {
        $this->resolveSymbols();

        $ped = $this->makePed();
        // nNodeFlags: high 12 bits = signal id A, low 12 bits = signal id B.
        $this->initUint32($ped + self::OFF_NODE_FLAGS, (3 << 16) | 5);

        $this->call('_pedestrianTask_8c028e00')->with(0, $ped);

        $this->shouldWriteLong($ped + self::OFF_STATE, 1);
        // The nSignalIdA/B split (nNodeFlags >> 16 and & 0xfff) compiles to a
        // SHLR16 the sh4objtest simulator doesn't implement yet, so this test
        // can't drive past that point; stop here rather than crash the run.
        $this->forceStop();
    }

    public function test_state1_rechecks_signal()
    {
        $this->resolveSymbols();

        $ped = $this->makePed();
        $this->initUint32($ped + self::OFF_STATE, 1);
        $this->initUint32($ped + self::OFF_SIGNAL_A, 7);

        $this->call('_pedestrianTask_8c028e00')->with(0, $ped);

        $this->shouldCall('_SignalGetFrame_8c028900')->with(7)->andReturn(1);
        // No further state write: it stays 1.
    }

    public function test_state2_waits_while_signal_pending()
    {
        $this->resolveSymbols();

        $ped = $this->makePed();
        $this->initUint32($ped + self::OFF_STATE, 2);
        $this->initUint32($ped + self::OFF_SIGNAL_A, 7);
        $this->initUint32($ped + self::OFF_SIGNAL_B, 9);

        $this->call('_pedestrianTask_8c028e00')->with(0, $ped);

        $this->shouldCall('_SignalGetFrame_8c028900')->with(7)->andReturn(0);
    }

    public function test_state2_starts_crossing_when_signal_allows()
    {
        $this->resolveSymbols();

        $ped = $this->makePed();
        $this->initUint32($ped + self::OFF_STATE, 2);
        $this->initUint32($ped + self::OFF_SIGNAL_A, 7);
        $this->initUint32($ped + self::OFF_SIGNAL_B, 9);
        $this->initUint32($ped + self::OFF_NODE_FLAGS, 0); // no crossing table entry
        $this->initUint32($ped + self::OFF_PATH_POS, fdec(0.0));
        $this->initUint32($ped + self::OFF_SPEED, fdec(1.0));

        $this->call('_pedestrianTask_8c028e00')->with(0, $ped);

        $this->shouldCall('_SignalGetFrame_8c028900')->with(7)->andReturn(1);
        $this->shouldCall('_SignalIsCrossingOccupied_8c02898e')->with(9)->andReturn(0);
        $this->shouldWriteLong($ped + self::OFF_STATE, 4);
        $this->shouldWriteLong($ped + self::OFF_STATE, 0); // no active crossing node left
        $this->shouldWriteFloat($ped + self::OFF_PATH_POS, 1.0);
        $this->shouldWriteLong($ped + self::OFF_ANIM_PHASE, 1);
        $this->shouldCall('_advancePedPathPos_8c0289ac')->with($ped)->andReturn(0);
    }

    public function test_state4_crosswalk_match_forward_blocks_movement()
    {
        $this->resolveSymbols();

        $this->setSize('_IntersectSegments_8c0206f0', 4);
        $this->setSize('_var_crossingIntersectPoint_8c1bc458', 4);

        $ped = $this->makePed();
        $this->initUint32($ped + self::OFF_STATE, 4);
        $this->initUint32($ped + self::OFF_SIGNAL_A, 7);
        $this->initUint32($ped + self::OFF_SIGNAL_B, 9);
        $this->initUint32($ped + self::OFF_NODE_FLAGS, 1);
        $this->initUint32($ped + self::OFF_REVERSE, 0);
        $node = $this->alloc(4);
        $other = $this->alloc(4);
        $this->initUint32($ped + self::OFF_PATH_NODE, $node);
        $this->initUint32($ped + self::OFF_SPRITE_X, fdec(1.0));
        $this->initUint32($ped + self::OFF_SPRITE_Z, fdec(2.0));

        $table = $this->addressOf('_var_blockedCrosswalks_8c228248');
        $this->initUint32($table + 0x00, $node);
        $this->initUint32($table + 0x04, $other);
        $this->initUint32($this->addressOf('_var_blockedCrosswalksEnd_8c228244'), $table + 2 * 4);

        $this->call('_pedestrianTask_8c028e00')->with(0, $ped);

        // forward (nReverse == 0) passes entry[1] + 4, not entry[0] + 4. Arg 3
        // (pos) is a stack local computed just before the call, so it can't
        // be pinned to a known address; check the other four by hand instead
        // of wildcarding it.
        $this->shouldCall('_IntersectSegments_8c0206f0')
            ->do($this->assertCrossingArgs($other + 4))
            ->andReturn(1);
        $this->shouldCall('_SignalMarkPedCrossing_8c02897a')->with(9);
        $this->shouldCall('_CollisionQueueAdd_8c02e48e')->with($ped);
        // shouldMove was cleared: no flPathPos/nAnimPhase write, no advancePedPathPos call.
    }

    public function test_state4_crosswalk_match_reverse_keeps_moving()
    {
        $this->resolveSymbols();

        $this->setSize('_IntersectSegments_8c0206f0', 4);
        $this->setSize('_var_crossingIntersectPoint_8c1bc458', 4);

        $ped = $this->makePed();
        $this->initUint32($ped + self::OFF_STATE, 4);
        $this->initUint32($ped + self::OFF_SIGNAL_A, 7);
        $this->initUint32($ped + self::OFF_SIGNAL_B, 9);
        $this->initUint32($ped + self::OFF_NODE_FLAGS, 1);
        $this->initUint32($ped + self::OFF_REVERSE, 1);
        $this->initUint32($ped + self::OFF_PATH_POS, fdec(10.0));
        $this->initUint32($ped + self::OFF_SPEED, fdec(2.0));
        $node = $this->alloc(4);
        $other = $this->alloc(4);
        $this->initUint32($ped + self::OFF_PATH_NODE, $node);
        $this->initUint32($ped + self::OFF_SPRITE_X, fdec(1.0));
        $this->initUint32($ped + self::OFF_SPRITE_Z, fdec(2.0));

        $table = $this->addressOf('_var_blockedCrosswalks_8c228248');
        $this->initUint32($table + 0x00, $node);
        $this->initUint32($table + 0x04, $other);
        $this->initUint32($this->addressOf('_var_blockedCrosswalksEnd_8c228244'), $table + 2 * 4);

        $this->call('_pedestrianTask_8c028e00')->with(0, $ped);

        // reverse (nReverse == 1) passes entry[0] + 4 (== node + 4), not entry[1] + 4.
        $this->shouldCall('_IntersectSegments_8c0206f0')
            ->do($this->assertCrossingArgs($node + 4))
            ->andReturn(0);
        $this->shouldCall('_SignalMarkPedCrossing_8c02897a')->with(9);
        $this->shouldCall('_CollisionQueueAdd_8c02e48e')->with($ped);
        $this->shouldCall('_SignalGetFrame_8c028900')->with(7)->andReturn(1);
        $this->shouldWriteFloat($ped + self::OFF_PATH_POS, 6.0); // -speed*2
        $this->shouldWriteLong($ped + self::OFF_ANIM_PHASE, 1);
        $this->shouldCall('_advancePedPathPos_8c0289ac')->with($ped)->andReturn(0);
    }

    public function test_state1_transitions_to_state2_when_signal_not_1()
    {
        $this->resolveSymbols();

        $ped = $this->makePed();
        $this->initUint32($ped + self::OFF_STATE, 1);
        $this->initUint32($ped + self::OFF_SIGNAL_A, 7);

        $this->call('_pedestrianTask_8c028e00')->with(0, $ped);

        $this->shouldCall('_SignalGetFrame_8c028900')->with(7)->andReturn(0);
        $this->shouldWriteLong($ped + self::OFF_STATE, 2);
        // shouldMove stays false: no flPathPos write.
    }

    public function test_state2_frame1_and_occupied_keeps_waiting()
    {
        $this->resolveSymbols();

        $ped = $this->makePed();
        $this->initUint32($ped + self::OFF_STATE, 2);
        $this->initUint32($ped + self::OFF_SIGNAL_A, 7);
        $this->initUint32($ped + self::OFF_SIGNAL_B, 9);

        $this->call('_pedestrianTask_8c028e00')->with(0, $ped);

        $this->shouldCall('_SignalGetFrame_8c028900')->with(7)->andReturn(1);
        $this->shouldCall('_SignalIsCrossingOccupied_8c02898e')->with(9)->andReturn(1);
        // No nState write and no flPathPos write: falls to the plain "state==2" shouldMove=FALSE case.
    }

    public function test_state4_crossing_continues_fast_when_signal_green()
    {
        $this->resolveSymbols();

        $ped = $this->makePed();
        $this->initUint32($ped + self::OFF_STATE, 4);
        $this->initUint32($ped + self::OFF_SIGNAL_A, 7);
        $this->initUint32($ped + self::OFF_SIGNAL_B, 9);
        $this->initUint32($ped + self::OFF_NODE_FLAGS, 1); // keeps crossing (state stays 4)
        $this->initUint32($ped + self::OFF_PATH_NODE, 0); // never matches table entries
        $this->initUint32($ped + self::OFF_PATH_POS, fdec(10.0));
        $this->initUint32($ped + self::OFF_SPEED, fdec(2.0));

        // Empty crossing table: entry range is empty (end <= start).
        $tableStart = $this->addressOf('_var_blockedCrosswalks_8c228248');
        $this->initUint32($this->addressOf('_var_blockedCrosswalksEnd_8c228244'), $tableStart);

        $this->call('_pedestrianTask_8c028e00')->with(0, $ped);

        $this->shouldCall('_SignalMarkPedCrossing_8c02897a')->with(9);
        $this->shouldCall('_CollisionQueueAdd_8c02e48e')->with($ped);
        $this->shouldCall('_SignalGetFrame_8c028900')->with(7)->andReturn(1);
        $this->shouldWriteFloat($ped + self::OFF_PATH_POS, 14.0); // +speed*2
        $this->shouldWriteLong($ped + self::OFF_ANIM_PHASE, 1);
        $this->shouldCall('_advancePedPathPos_8c0289ac')->with($ped)->andReturn(0);
    }

    public function test_state4_crossing_continues_slow_when_signal_not_green()
    {
        $this->resolveSymbols();

        $ped = $this->makePed();
        $this->initUint32($ped + self::OFF_STATE, 4);
        $this->initUint32($ped + self::OFF_SIGNAL_A, 7);
        $this->initUint32($ped + self::OFF_SIGNAL_B, 9);
        $this->initUint32($ped + self::OFF_NODE_FLAGS, 1);
        $this->initUint32($ped + self::OFF_PATH_NODE, 0);
        $this->initUint32($ped + self::OFF_PATH_POS, fdec(10.0));
        $this->initUint32($ped + self::OFF_SPEED, fdec(2.0));

        $tableStart = $this->addressOf('_var_blockedCrosswalks_8c228248');
        $this->initUint32($this->addressOf('_var_blockedCrosswalksEnd_8c228244'), $tableStart);

        $this->call('_pedestrianTask_8c028e00')->with(0, $ped);

        $this->shouldCall('_SignalMarkPedCrossing_8c02897a')->with(9);
        $this->shouldCall('_CollisionQueueAdd_8c02e48e')->with($ped);
        $this->shouldCall('_SignalGetFrame_8c028900')->with(7)->andReturn(0);
        $this->shouldWriteFloat($ped + self::OFF_PATH_POS, 16.0); // +speed*3
        $this->shouldWriteLong($ped + self::OFF_ANIM_PHASE, 1); // extra bump for the slow case
        $this->shouldWriteLong($ped + self::OFF_ANIM_PHASE, 2);
        $this->shouldCall('_advancePedPathPos_8c0289ac')->with($ped)->andReturn(0);
    }
};
