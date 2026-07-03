<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_pathNone()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x00, 1);

        $this->singleCall('_startAdxFadeOut_8c010bae')->with(0)->run();
    }

    public function test_pathNone2()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x00, 0xf0);

        $this->singleCall('_startAdxFadeOut_8c010bae')->with(0)->run();
    }

    public function test_pathNone3()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x00, 0xf0);

        $this->singleCall('_startAdxFadeOut_8c010bae')->with(3)->run();
    }

    public function test_pathA()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x00, 0b100);
        $this->initUint32($this->addressOf('_init_adxVolTargets_8c03bd88') + 0x00, 660);

        $this->shouldWrite($this->addressOf('_var_adxFade_8c157a34') + 0x00, 0b101);
        $this->shouldWrite($this->addressOf('_var_adxFade_8c157a34') + 0x04, 4);
        $this->shouldWrite($this->addressOf('_var_adxFade_8c157a34') + 0x0c, -330);

        $this->singleCall('_startAdxFadeOut_8c010bae')->with(0)->run();
    }

    public function test_pathB()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x00, 0b100);
        $this->initUint32($this->addressOf('_init_adxVolTargets_8c03bd88') + 0x04, 660);

        $this->shouldWrite($this->addressOf('_var_adxFade_8c157a34') + 0x00, 0b110);
        $this->shouldWrite($this->addressOf('_var_adxFade_8c157a34') + 0x08, 4);
        $this->shouldWrite($this->addressOf('_var_adxFade_8c157a34') + 0x10, -330);

        $this->singleCall('_startAdxFadeOut_8c010bae')->with(1)->run();
    }

    private function resolveSymbols(): void
    {
        // Functions
        $this->setSize('__divls', 4);
    }
};
