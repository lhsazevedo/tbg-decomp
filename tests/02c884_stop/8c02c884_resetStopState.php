<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/*
 * _resetStopState_8c02c884(void): clears the 31-slot scripted
 * waiting-passenger schedule (var_seats_8c228718, -1 = unused) and resets the
 * shared waiting-passenger sprite's scale/angle/animation ahead of a new
 * stop.
 */
return new class extends TestCase {
    const SLOT_COUNT = 31;
    const SPRITE_SX = 0x0c;
    const SPRITE_SY = 0x10;
    const SPRITE_ANG = 0x14;
    const SPRITE_TANIM = 0x1c;

    private function resolveSymbols(): void
    {
        $this->setSize('_init_pedestrianTexAnims_8c04623c', 8);
    }

    public function test_resets_slots_and_sprite(): void
    {
        $this->resolveSymbols();

        $slots = $this->addressOf('_var_seats_8c228718');
        $sprite = $this->addressOf('_var_passengerSprite_8c2288d8');
        $tanim = $this->addressOf('_init_pedestrianTexAnims_8c04623c');

        $this->call('_resetStopState_8c02c884')->with();

        for ($i = 0; $i < self::SLOT_COUNT; $i++) {
            $this->shouldWriteLong($slots + $i * 4, -1);
        }

        $this->shouldWriteFloat($sprite + self::SPRITE_SX, 0.014);
        $this->shouldWriteFloat($sprite + self::SPRITE_SY, 0.014);
        $this->shouldWriteLong($sprite + self::SPRITE_ANG, 0);
        $this->shouldWriteLong($sprite + self::SPRITE_TANIM, $tanim);
    }
};
