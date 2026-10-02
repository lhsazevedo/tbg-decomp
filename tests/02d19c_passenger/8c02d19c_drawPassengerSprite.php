<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    private function resolveSymbols(): void
    {
        $this->setSize('_njDrawSprite3D', 4);
        $this->setSize('__quick_odd_mvn', 4);
        $this->setSize('_var_passengerSprite_8c2288d8', 0x20); // NJS_SPRITE
        $this->setSize('_var_pedestrianAssets_8c1bbfdc', 0x41 * 0x10); // ModelSlot[65]
    }

    /** The struct-copy helper SHC emits for `p = ...` (dest R1, src R2, len R0). */
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

    /** Allocates a PassengerState pointing ref_0x00 at a fresh pedestrian-type byte. */
    private function makeState(int $pedType, int $field14, int $field28): int
    {
        $state = $this->alloc(0x38);
        $entry = $this->alloc(4);
        $this->initUint8($entry, $pedType);
        $this->initUint32($state + 0x00, $entry); // ref_0x00
        $this->initUint32($state + 0x14, $field14);
        $this->initUint32($state + 0x28, $field28);
        return $state;
    }

    private function f(float $v): int
    {
        return unpack('L', pack('f', $v))[1];
    }

    public function test_draws_sprite_field28_zero_uses_size_0x32(): void
    {
        $this->resolveSymbols();

        $assets = $this->addressOf('_var_pedestrianAssets_8c1bbfdc');
        $tlist = 0xdeadbeef;
        $this->initUint32($assets + 5 * 0x10 + 0x08, $tlist);

        $state = $this->makeState(5, 0x20, 0);
        $this->initUint32($state + 0x08, $this->f(1.0));
        $this->initUint32($state + 0x0c, $this->f(2.0));
        $this->initUint32($state + 0x10, $this->f(3.0));

        $this->call('_drawPassengerSprite_8c02d19c')->with($state);

        $sprite = $this->addressOf('_var_passengerSprite_8c2288d8');
        $this->shouldWriteLong($sprite + 0x18, $tlist);
        $this->mockStructCopy($sprite + 0x00, $state + 0x08);
        $this->shouldCall('_njDrawSprite3D')->with($sprite, 0x20, 0x32);
    }

    public function test_draws_sprite_field28_nonzero_uses_size_0x30(): void
    {
        $this->resolveSymbols();

        $assets = $this->addressOf('_var_pedestrianAssets_8c1bbfdc');
        $tlist = 0x12345678;
        $this->initUint32($assets + 7 * 0x10 + 0x08, $tlist);

        $state = $this->makeState(7, 0x23, 1);
        $this->initUint32($state + 0x08, $this->f(4.0));
        $this->initUint32($state + 0x0c, $this->f(5.0));
        $this->initUint32($state + 0x10, $this->f(6.0));

        $this->call('_drawPassengerSprite_8c02d19c')->with($state);

        $sprite = $this->addressOf('_var_passengerSprite_8c2288d8');
        $this->shouldWriteLong($sprite + 0x18, $tlist);
        $this->mockStructCopy($sprite + 0x00, $state + 0x08);
        $this->shouldCall('_njDrawSprite3D')->with($sprite, 0x23, 0x30);
    }

    public function test_ped_type_out_of_range_is_skipped(): void
    {
        $this->resolveSymbols();

        $state = $this->makeState(0x41, 0x20, 0);

        $this->call('_drawPassengerSprite_8c02d19c')->with($state);
    }

    public function test_unloaded_texlist_is_skipped(): void
    {
        $this->resolveSymbols();

        $assets = $this->addressOf('_var_pedestrianAssets_8c1bbfdc');
        $this->initUint32($assets + 2 * 0x10 + 0x08, 0xffffffff);

        $state = $this->makeState(2, 0x20, 0);

        $this->call('_drawPassengerSprite_8c02d19c')->with($state);
    }
};
