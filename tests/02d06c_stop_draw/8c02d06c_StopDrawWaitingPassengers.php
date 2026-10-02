<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_njDrawSprite3D', 4);
        $this->setSize('__quick_odd_mvn', 4);
        $this->setSize('_var_waitingPassengerCount_8c228794', 4);
        $this->setSize('_var_waitingPassengers_8c228798', 16 * 0x14); // WaitingPassengerSlot[16]
        $this->setSize('_var_passengerSprite_8c2288d8', 0x20); // NJS_SPRITE
        $this->setSize('_var_pedestrianAssets_8c1bbfdc', 0x41 * 0x10); // ModelSlot[65]

    }

    /**
     * The struct-copy helper SHC emits for the p = pos_0x04 assignment
     * (dest in R1, src in R2, byte count in R0 -- see docs/lessons_learned.md).
     * Performs the copy and asserts dst/src/len so the expected NJS_VECTOR
     * is what actually lands in var_passengerSprite_8c2288d8.p; the writes it performs go
     * straight to memory and are not separately tracked expectations, so
     * register it in chronological position (right after the tlist write,
     * before the njDrawSprite3D call it precedes).
     */
    private function mockStructCopy(int $expectedDst, int $expectedSrc): void
    {
        $this->shouldCall('__quick_odd_mvn')->do(function () use ($expectedDst, $expectedSrc) {
            $dst = $this->getRegister(1)->value;
            $src = $this->getRegister(2)->value;
            $len = $this->getRegister(0)->value;

            $got = sprintf('dst=%08x src=%08x len=%d', $dst, $src, $len);
            $want = sprintf('dst=%08x src=%08x len=12', $expectedDst, $expectedSrc);
            if ($got !== $want) {
                throw new \RuntimeException("__quick_odd_mvn: expected $want, got $got");
            }

            for ($i = 0; $i < $len; $i += 4) {
                $this->memory->writeUInt32($dst + $i, $this->memory->readUInt32($src + $i));
            }
        });
    }

    /** Sets up one WaitingPassengerSlot at var_waitingPassengers_8c228798[i]. */
    private function initSlot(int $base, int $i, int $pedType, float $x, float $y, float $z): void
    {
        $slot = $base + $i * 0x14;
        $typeByte = $this->alloc(1);
        $this->initUint8($typeByte, $pedType);
        $this->initUint32($slot + 0x00, $typeByte); // spot_0x00
        $this->initUint32($slot + 0x04, $this->f($x));
        $this->initUint32($slot + 0x08, $this->f($y));
        $this->initUint32($slot + 0x0c, $this->f($z));
    }

    /** Sets one ModelSlot's texlist_0x08 field. */
    private function initTexlist(int $base, int $pedType, int $tlist): void
    {
        $this->initUint32($base + $pedType * 0x10 + 0x08, $tlist);
    }

    public function test_single_valid_passenger_draws_sprite_facing0(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_waitingPassengerCount_8c228794'), 1);

        $slots = $this->addressOf('_var_waitingPassengers_8c228798');
        $this->initSlot($slots, 0, 5, 1.0, 2.0, 3.0);

        $assets = $this->addressOf('_var_pedestrianAssets_8c1bbfdc');
        $tlist = 0xdeadbeef;
        $this->initTexlist($assets, 5, $tlist);

        $this->call('_StopDrawWaitingPassengers_8c02d06c')->with(0);

        $sprite = $this->addressOf('_var_passengerSprite_8c2288d8');
        $this->shouldWriteLong($sprite + 0x18, $tlist); // tlist

        $this->mockStructCopy($sprite + 0x00, $slots + 0x04); // p = pos_0x04

        $this->shouldCall('_njDrawSprite3D')->with($sprite, 0x23, 0x30);
    }

    public function test_single_valid_passenger_draws_sprite_facing1(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_waitingPassengerCount_8c228794'), 1);

        $slots = $this->addressOf('_var_waitingPassengers_8c228798');
        $this->initSlot($slots, 0, 5, 1.0, 2.0, 3.0);

        $assets = $this->addressOf('_var_pedestrianAssets_8c1bbfdc');
        $tlist = 0xdeadbeef;
        $this->initTexlist($assets, 5, $tlist);

        $this->call('_StopDrawWaitingPassengers_8c02d06c')->with(1);

        $sprite = $this->addressOf('_var_passengerSprite_8c2288d8');
        $this->shouldWriteLong($sprite + 0x18, $tlist);

        $this->mockStructCopy($sprite + 0x00, $slots + 0x04);

        $this->shouldCall('_njDrawSprite3D')->with($sprite, 0x22, 0x30);
    }

    public function test_ped_type_at_bound_is_skipped(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_waitingPassengerCount_8c228794'), 1);

        $slots = $this->addressOf('_var_waitingPassengers_8c228798');
        // 0x41 (65) is out of range (< 0x41 required)
        $this->initSlot($slots, 0, 0x41, 1.0, 2.0, 3.0);

        $this->call('_StopDrawWaitingPassengers_8c02d06c')->with(0);
    }

    public function test_unloaded_texlist_is_skipped(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_waitingPassengerCount_8c228794'), 1);

        $slots = $this->addressOf('_var_waitingPassengers_8c228798');
        $this->initSlot($slots, 0, 3, 1.0, 2.0, 3.0);

        $assets = $this->addressOf('_var_pedestrianAssets_8c1bbfdc');
        $this->initTexlist($assets, 3, 0xffffffff); // -1 = unloaded

        $this->call('_StopDrawWaitingPassengers_8c02d06c')->with(0);
    }

    public function test_zero_count_draws_nothing(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_waitingPassengerCount_8c228794'), 0);

        $this->call('_StopDrawWaitingPassengers_8c02d06c')->with(0);
    }

    public function test_multiple_passengers_draw_in_order(): void
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_waitingPassengerCount_8c228794'), 2);

        $slots = $this->addressOf('_var_waitingPassengers_8c228798');
        $this->initSlot($slots, 0, 1, 10.0, 20.0, 30.0);
        $this->initSlot($slots, 1, 2, 40.0, 50.0, 60.0);

        $assets = $this->addressOf('_var_pedestrianAssets_8c1bbfdc');
        $this->initTexlist($assets, 1, 0x1111);
        $this->initTexlist($assets, 2, 0x2222);

        $this->call('_StopDrawWaitingPassengers_8c02d06c')->with(0);

        $sprite = $this->addressOf('_var_passengerSprite_8c2288d8');

        $this->shouldWriteLong($sprite + 0x18, 0x1111);
        $this->mockStructCopy($sprite + 0x00, $slots + 0x00 + 0x04);
        $this->shouldCall('_njDrawSprite3D')->with($sprite, 0x23, 0x30);

        $this->shouldWriteLong($sprite + 0x18, 0x2222);
        $this->mockStructCopy($sprite + 0x00, $slots + 0x14 + 0x04);
        $this->shouldCall('_njDrawSprite3D')->with($sprite, 0x23, 0x30);
    }

    /** Raw uint32 bits of a float, for initUint32. */
    private function f(float $v): int
    {
        return unpack('L', pack('f', $v))[1];
    }
};
