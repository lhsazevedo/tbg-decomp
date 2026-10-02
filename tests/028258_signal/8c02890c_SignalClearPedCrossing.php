<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_clears_all_entries()
    {
        $base = $this->addressOf('_var_pedCrossingFlags_8c227e2c');
        for ($i = 0; $i < 64; $i++) {
            $this->initUint32($base + $i * 8, 0x11111111);
            $this->initUint32($base + $i * 8 + 4, 0x22222222);
        }

        $this->call('_SignalClearPedCrossing_8c02890c');

        for ($i = 0; $i < 64; $i++) {
            $this->shouldWriteLong($base + $i * 8, 0);
            $this->shouldWriteLong($base + $i * 8 + 4, 0);
        }
    }
};
