<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

// applyLighting_8c02756a is a hidden (never directly called) fade-
// command callback -- its address is only taken via a .DATA.L pool entry
// and passed to RenderQueueDraw_8c0223ea. It selects one of two fixed
// float[3] light-direction vectors by flag and applies it as both the
// Chunk "simple" and "easy" light direction.

return new class extends TestCase {
    private function resolveSymbols(): void {
        $this->setSize('_njCnkSetSimpleLight', 4);
        $this->setSize('_njCnkSetEasyLight', 4);
        $this->setSize('_var_busSimpleLightDir_8c227db8', 12);
        $this->setSize('_var_mirrorLightDir_8c227dc4', 12);
    }

    private function initFloat(int $addr, float $value): void {
        $this->initUint32($addr, unpack('L', pack('f', $value))[1]);
    }

    private function seedDir(string $symbol, float $x, float $y, float $z): void {
        $base = $this->addressOf($symbol);
        $this->initFloat($base + 0, $x);
        $this->initFloat($base + 4, $y);
        $this->initFloat($base + 8, $z);
    }

    public function test_flag_zero_uses_simple_light_dir(): void {
        $this->resolveSymbols();

        $this->seedDir('_var_busSimpleLightDir_8c227db8', 1.0, 2.0, 3.0);
        $this->seedDir('_var_mirrorLightDir_8c227dc4', 4.0, 5.0, 6.0);

        $this->call('_applyLighting_8c02756a')->with(0);

        $this->shouldCall('_njCnkSetSimpleLight')->with(1.0, 2.0, 3.0);
        $this->shouldCall('_njCnkSetEasyLight')->with(1.0, 2.0, 3.0);
    }

    public function test_flag_nonzero_uses_other_dir(): void {
        $this->resolveSymbols();

        $this->seedDir('_var_busSimpleLightDir_8c227db8', 1.0, 2.0, 3.0);
        $this->seedDir('_var_mirrorLightDir_8c227dc4', 4.0, 5.0, 6.0);

        $this->call('_applyLighting_8c02756a')->with(1);

        $this->shouldCall('_njCnkSetSimpleLight')->with(4.0, 5.0, 6.0);
        $this->shouldCall('_njCnkSetEasyLight')->with(4.0, 5.0, 6.0);
    }
};
