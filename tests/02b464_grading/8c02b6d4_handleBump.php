<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_CollisionQueueTest_8c02e4ac', 4);
        $this->setSize('_BusCollisionFindHit_8c02e2dc', 4);
        $this->setSize('_BusDriveEnterKnockback_8c023bce', 4);
        $this->setSize('_VibStart_8c010f7a', 4);
        $this->setSize('_sdMidiPlay', 4);
        $this->setSize('_var_midiHandles_8c0fcd28', 0x20);
        $this->setSize('_var_playerBus_8c1bbd9c', 4); // BusState*, allocated via alloc()
        $this->setSize('_njSqrt', 4);
    }


    // Rounds to float32 precision, so expected sums/quotients can be computed
    // the same way the SH4 FPU does (round each operand to float32 first,
    // then round the float32 result) rather than PHP's double precision.
    private function f32(float $value): float
    {
        return unpack('f', pack('f', $value))[1];
    }

    private function initFloat(int $address, float $value): void
    {
        $raw = unpack('L', pack('f', $value))[1];
        $this->initUint32($address, $raw);
    }

    public function test_queued_collision_applies_flat_penalty_and_locks_out(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_playerBus_8c1bbd9c'), $this->alloc(0x2b8));

        $this->call('_handleBump_8c02b6d4');

        $this->shouldCall('_CollisionQueueTest_8c02e4ac')->andReturn(0x1234);
        $this->shouldCall('_adjust_8c02b464')->with(0x1f, 0xffffff38); // -200
        $this->shouldWriteLongTo('_var_cooldownCollision_8c228690', 0x7fff);
    }

    public function test_no_op_when_bus_not_driving(): void
    {
        $this->resolveSymbols();

        $busState = $this->alloc(0x2b8);
        $this->initUint32($busState + 0x2b4, 2); // driveState_0x2b4 != 1
        $this->initUint32($this->addressOf('_var_playerBus_8c1bbd9c'), $busState);

        $this->call('_handleBump_8c02b6d4');

        $this->shouldCall('_CollisionQueueTest_8c02e4ac')->andReturn(0);
    }

    public function test_no_op_when_nothing_found_to_bump(): void
    {
        $this->resolveSymbols();

        $busState = $this->alloc(0x2b8);
        $this->initUint32($busState + 0x2b4, 1); // driveState_0x2b4 == 1 (driving)
        $this->initUint32($this->addressOf('_var_playerBus_8c1bbd9c'), $busState);

        $this->call('_handleBump_8c02b6d4');

        $this->shouldCall('_CollisionQueueTest_8c02e4ac')->andReturn(0);
        $this->shouldCall('_BusCollisionFindHit_8c02e2dc')->andReturn(0);
        $this->shouldWriteLongTo('_var_hitCandidate_8c228664', 0);
    }

    public function test_bump_knocks_both_apart_and_grades_penalty_by_speed(): void
    {
        $this->resolveSymbols();

        $me = $this->alloc(0x2b8);
        $this->initUint32($me + 0x2b4, 1); // driveState_0x2b4 == 1 (driving)
        $this->initFloat($me + 0xf4, 0.0); // posX
        $this->initFloat($me + 0xfc, 0.0); // posZ
        $this->initUint32($this->addressOf('_var_playerBus_8c1bbd9c'), $me);

        $other = $this->alloc(0x2b8);
        $this->initFloat($other + 0xf4, 3.0); // posX
        $this->initFloat($other + 0xfc, 4.0); // posZ
        $this->initFloat($other + 0x27c, 2.5); // speed, saved before overwrite
        $this->initUint32($other + 0x2e4, 0); // rpmRampAngle_0x2e4 == 0 branch

        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 0.05); // player speed: lowest tier
        $this->initUint32($this->addressOf('_var_cooldownCollision_8c228690'), 0xfffffffb); // -5: cooldown expired
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x1234);

        $this->call('_handleBump_8c02b6d4');

        $this->shouldCall('_CollisionQueueTest_8c02e4ac')->andReturn(0);
        $this->shouldCall('_BusCollisionFindHit_8c02e2dc')->andReturn($other);
        $this->shouldWriteLongTo('_var_hitCandidate_8c228664', $other);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0x13, 0);
        $this->shouldWriteLongTo('_var_hitVehicle_8c228668', $other);
        $this->shouldWriteFloat($this->addressOf('_var_bumpSpeed_8c228670'), 2.5);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0); // dx*dx+dz*dz = 3^2+4^2
        $this->shouldWriteLong($other + 0x2b4, 1);
        $this->shouldWriteFloat($other + 0x27c, $this->f32($this->f32(0.05) + $this->f32(0.3))); // var_frameSpeed_8c22866c + 0.3
        $this->shouldWriteFloat($other + 0x29c, $this->f32(3.0 / 5.0)); // dx/dist
        $this->shouldWriteFloat($other + 0x2a0, $this->f32(4.0 / 5.0)); // dz/dist
        $this->shouldWriteLong($other + 0x19c, 0);
        $this->shouldWriteLong($other + 0x1ac, 0);
        $this->shouldWriteLong($other + 0x1bc, 0);
        $this->shouldCall('_BusDriveEnterKnockback_8c023bce');
        $this->shouldWriteFloat($me + 0x27c, $this->f32($this->f32(2.5) + $this->f32(0.3))); // var_bumpSpeed_8c228670 + 0.3
        $this->shouldWriteFloat($me + 0x29c, -$this->f32(3.0 / 5.0));
        $this->shouldWriteFloat($me + 0x2a0, -$this->f32(4.0 / 5.0));
        $this->shouldWriteFloat($me + 0x2ac, -$this->f32(3.0 / 5.0));
        $this->shouldWriteFloat($me + 0x2b0, -$this->f32(4.0 / 5.0));
        $this->shouldCall('_VibStart_8c010f7a')->with(4);
        $this->shouldCall('_adjust_8c02b464')->with(1, 0xffffffe2); // -30
        $this->shouldCall('_armCooldowns_8c02b578')->with(1);
    }

    public function test_high_speed_bump_with_field_0x2e4_set_skips_extra_zeroing(): void
    {
        $this->resolveSymbols();

        $me = $this->alloc(0x2b8);
        $this->initUint32($me + 0x2b4, 1);
        $this->initFloat($me + 0xf4, 0.0);
        $this->initFloat($me + 0xfc, 0.0);
        $this->initUint32($this->addressOf('_var_playerBus_8c1bbd9c'), $me);

        $other = $this->alloc(0x2b8);
        $this->initFloat($other + 0xf4, 3.0);
        $this->initFloat($other + 0xfc, 4.0);
        $this->initFloat($other + 0x27c, 2.5);
        $this->initUint32($other + 0x2e4, 1); // rpmRampAngle_0x2e4 != 0 branch

        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 0.5); // highest tier
        $this->initUint32($this->addressOf('_var_cooldownCollision_8c228690'), 0); // cooldown still active
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x1234);

        $this->call('_handleBump_8c02b6d4');

        $this->shouldCall('_CollisionQueueTest_8c02e4ac')->andReturn(0);
        $this->shouldCall('_BusCollisionFindHit_8c02e2dc')->andReturn($other);
        $this->shouldWriteLongTo('_var_hitCandidate_8c228664', $other);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0x13, 0);
        $this->shouldWriteLongTo('_var_hitVehicle_8c228668', $other);
        $this->shouldWriteFloat($this->addressOf('_var_bumpSpeed_8c228670'), 2.5);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldWriteLong($other + 0x2b4, 1);
        $this->shouldWriteFloat($other + 0x27c, $this->f32($this->f32(0.5) + $this->f32(0.3)));
        $this->shouldWriteFloat($other + 0x29c, $this->f32(3.0 / 5.0));
        $this->shouldWriteFloat($other + 0x2a0, $this->f32(4.0 / 5.0));
        // No field_0x19c/0x1ac/0x1bc zeroing on this branch.
        $this->shouldCall('_BusDriveEnterKnockback_8c023bce');
        $this->shouldWriteFloat($me + 0x27c, $this->f32($this->f32(2.5) + $this->f32(0.3)));
        $this->shouldWriteFloat($me + 0x29c, -$this->f32(3.0 / 5.0));
        $this->shouldWriteFloat($me + 0x2a0, -$this->f32(4.0 / 5.0));
        $this->shouldWriteFloat($me + 0x2ac, -$this->f32(3.0 / 5.0));
        $this->shouldWriteFloat($me + 0x2b0, -$this->f32(4.0 / 5.0));
        $this->shouldCall('_VibStart_8c010f7a')->with(6);
        // Cooldown still active (var_cooldownCollision_8c228690 >= 0): no penalty, no re-arm.
    }

    public function test_mid_speed_bump_applies_mid_tier_penalty(): void
    {
        $this->resolveSymbols();

        $me = $this->alloc(0x2b8);
        $this->initUint32($me + 0x2b4, 1);
        $this->initFloat($me + 0xf4, 0.0);
        $this->initFloat($me + 0xfc, 0.0);
        $this->initUint32($this->addressOf('_var_playerBus_8c1bbd9c'), $me);

        $other = $this->alloc(0x2b8);
        $this->initFloat($other + 0xf4, 3.0);
        $this->initFloat($other + 0xfc, 4.0);
        $this->initFloat($other + 0x27c, 2.5);
        $this->initUint32($other + 0x2e4, 0);

        $this->initFloat($this->addressOf('_var_frameSpeed_8c22866c'), 0.2); // mid tier
        $this->initUint32($this->addressOf('_var_cooldownCollision_8c228690'), 0xffffffff); // -1: cooldown expired
        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x1234);

        $this->call('_handleBump_8c02b6d4');

        $this->shouldCall('_CollisionQueueTest_8c02e4ac')->andReturn(0);
        $this->shouldCall('_BusCollisionFindHit_8c02e2dc')->andReturn($other);
        $this->shouldWriteLongTo('_var_hitCandidate_8c228664', $other);
        $this->shouldCall('_sdMidiPlay')->with(0x1234, 1, 0x13, 0);
        $this->shouldWriteLongTo('_var_hitVehicle_8c228668', $other);
        $this->shouldWriteFloat($this->addressOf('_var_bumpSpeed_8c228670'), 2.5);
        $this->shouldCall('_njSqrt')->with(25.0)->andReturn(5.0);
        $this->shouldWriteLong($other + 0x2b4, 1);
        $this->shouldWriteFloat($other + 0x27c, $this->f32($this->f32(0.2) + $this->f32(0.3)));
        $this->shouldWriteFloat($other + 0x29c, $this->f32(3.0 / 5.0));
        $this->shouldWriteFloat($other + 0x2a0, $this->f32(4.0 / 5.0));
        $this->shouldWriteLong($other + 0x19c, 0);
        $this->shouldWriteLong($other + 0x1ac, 0);
        $this->shouldWriteLong($other + 0x1bc, 0);
        $this->shouldCall('_BusDriveEnterKnockback_8c023bce');
        $this->shouldWriteFloat($me + 0x27c, $this->f32($this->f32(2.5) + $this->f32(0.3)));
        $this->shouldWriteFloat($me + 0x29c, -$this->f32(3.0 / 5.0));
        $this->shouldWriteFloat($me + 0x2a0, -$this->f32(4.0 / 5.0));
        $this->shouldWriteFloat($me + 0x2ac, -$this->f32(3.0 / 5.0));
        $this->shouldWriteFloat($me + 0x2b0, -$this->f32(4.0 / 5.0));
        $this->shouldCall('_VibStart_8c010f7a')->with(5);
        $this->shouldCall('_adjust_8c02b464')->with(2, 0xffffffc4); // -60
        $this->shouldCall('_armCooldowns_8c02b578')->with(1);
    }
};
