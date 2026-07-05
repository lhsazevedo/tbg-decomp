<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_happyPath()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x00, 0x40);
        $this->initUint32($this->addressOf('_init_adxVolTargets_8c03bd88') + 0x04, 660);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0b0101_0101);

        $this->shouldWriteLong($this->addressOf('_var_adxFade_8c157a34') + 0x00, 0x60);
        // step1 = vol1 / 90, not a power of two.
        $this->shouldCall('__divls');
        $this->shouldWriteLong($this->addressOf('_var_adxFade_8c157a34') + 0x08, 7);
        $this->shouldWriteLong($this->addressOf('_var_adxFade_8c157a34') + 0x10, -990);
        $this->shouldWriteLong($this->addressOf('_init_8c03bd80'), 0b0100_0101);

        $this->singleCall('_startAdxCh1FadeIn_8c010c2c')->with(1)->run();
    }

    public function test_skipsWhenParamIsZero()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x00, 0x40);
        $this->initUint32($this->addressOf('_init_adxVolTargets_8c03bd88') + 0x04, 660);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0b0101_0101);

        $this->singleCall('_startAdxCh1FadeIn_8c010c2c')->with(0)->run();
    }

    public function test_skipsWhenParamIsTwo()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x00, 0x40);
        $this->initUint32($this->addressOf('_init_adxVolTargets_8c03bd88') + 0x04, 660);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0b0101_0101);

        $this->singleCall('_startAdxCh1FadeIn_8c010c2c')->with(2)->run();
    }

    public function test_skipsWhenLowerNibbleIsNotZero()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x00, 0x41);
        $this->initUint32($this->addressOf('_init_adxVolTargets_8c03bd88') + 0x04, 660);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0b0101_0101);

        $this->singleCall('_startAdxCh1FadeIn_8c010c2c')->with(1)->run();
    }

    public function test_skipsWhenUpperNibbleIs2()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x00, 0x20);
        $this->initUint32($this->addressOf('_init_adxVolTargets_8c03bd88') + 0x04, 660);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0b0101_0101);

        $this->singleCall('_startAdxCh1FadeIn_8c010c2c')->with(1)->run();
    }

    private function resolveSymbols(): void
    {
        // Functions
        $this->setSize('__divls', 4);

        $this->onCall('__divls', function () {
            $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
        });
    }
};
