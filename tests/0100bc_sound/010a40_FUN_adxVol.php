<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_pathA()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x00, 1);
        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x0c, 42);
        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x04, 2);

        $this->shouldWrite(
            $this->addressOf('_var_adxFade_8c157a34') + 0x0c,
            40
        );
        $this->shouldCall('_ADXT_SetOutVol')->with(0xcafe0000, 40);

        $this->singleCall('_SndUpdateAdxVolFade_8c010a40')->run();
    }

    public function test_pathB()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x00, 2);
        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x10, 42);
        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x08, 2);

        $this->shouldWrite(
            $this->addressOf('_var_adxFade_8c157a34') + 0x10,
            40
        );
        $this->shouldCall('_ADXT_SetOutVol')->with(0xcafe0001, 40);

        $this->singleCall('_SndUpdateAdxVolFade_8c010a40')->run();
    }

    public function test_pathC()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x00, 0b00001101);
        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x0c, 10);
        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x04, 311);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0b11111111);

        $this->shouldWrite(
            $this->addressOf('_var_adxFade_8c157a34') + 0x0c,
            -301
        );
        $this->shouldCall('_ADXT_SetOutVol')->with(0xcafe0000, -301);

        $this->shouldCall('_ADXT_Stop')->with(0xcafe0000);
        $this->shouldCall('_ADXT_SetOutVol')->with(0xcafe0000, 0);

        $this->shouldWrite(
            $this->addressOf('_var_adxFade_8c157a34') + 0x00,
            0b00001100
        );

        $this->shouldWrite($this->addressOf('_init_8c03bd80'), 0b11111110);

        $this->singleCall('_SndUpdateAdxVolFade_8c010a40')->run();
    }

    public function test_pathD()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x00, 0b00001110);
        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x10, 10);
        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x08, 311);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0b11111111);

        $this->shouldWrite(
            $this->addressOf('_var_adxFade_8c157a34') + 0x10,
            -301
        );
        $this->shouldCall('_ADXT_SetOutVol')->with(0xcafe0001, -301);

        $this->shouldCall('_ADXT_Stop')->with(0xcafe0001);
        $this->shouldCall('_ADXT_SetOutVol')->with(0xcafe0001, 0);

        $this->shouldWrite(
            $this->addressOf('_var_adxFade_8c157a34') + 0x00,
            0b00001100
        );

        $this->shouldWrite($this->addressOf('_init_8c03bd80'), 0b11101111);

        $this->singleCall('_SndUpdateAdxVolFade_8c010a40')->run();
    }

    public function test_pathE()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x00, 0b0001_0000);
        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x0c, 390);
        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x04, 600);

        $this->shouldWrite(
            $this->addressOf('_var_adxFade_8c157a34') + 0x0c,
            990
        );
        $this->shouldCall('_ADXT_SetOutVol')->with(0xcafe0000, 990);

        $this->singleCall('_SndUpdateAdxVolFade_8c010a40')->run();
    }

    public function test_pathF()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x00, 0b0010_0000);
        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x10, 390);
        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x08, 600);

        $this->shouldWrite(
            $this->addressOf('_var_adxFade_8c157a34') + 0x10,
            990
        );
        $this->shouldCall('_ADXT_SetOutVol')->with(0xcafe0001, 990);

        $this->singleCall('_SndUpdateAdxVolFade_8c010a40')->run();
    }

    public function test_pathG()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x00, 0b1101_0000);
        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x0c, 390);
        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x04, 601);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0b11111111);

        $this->shouldWrite(
            $this->addressOf('_var_adxFade_8c157a34') + 0x0c,
            991
        );
        $this->shouldCall('_ADXT_SetOutVol')->with(0xcafe0000, 991);
        
        $this->shouldCall('_ADXT_SetOutVol')->with(0xcafe0000, 990);

        $this->shouldWrite(
            $this->addressOf('_var_adxFade_8c157a34') + 0x00,
            0b1100_0000
        );

        $this->singleCall('_SndUpdateAdxVolFade_8c010a40')->run();
    }

    public function test_pathH()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x00, 0b1110_0000);
        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x10, 390);
        $this->initUint32($this->addressOf('_var_adxFade_8c157a34') + 0x08, 601);
        $this->initUint32($this->addressOf('_init_8c03bd80'), 0b11111111);

        $this->shouldWrite(
            $this->addressOf('_var_adxFade_8c157a34') + 0x10,
            991
        );
        $this->shouldCall('_ADXT_SetOutVol')->with(0xcafe0001, 991);

        $this->shouldCall('_ADXT_SetOutVol')->with(0xcafe0001, 990);

        $this->shouldWrite(
            $this->addressOf('_var_adxFade_8c157a34') + 0x00,
            0b1100_0000
        );

        $this->singleCall('_SndUpdateAdxVolFade_8c010a40')->run();
    }

    private function resolveSymbols(): void
    {

        // Functions
        $this->setSize('_ADXT_SetOutVol', 0x04);
        $this->setSize('_ADXT_Stop', 0x04);

        // Basic inits
        $this->initUint32($this->addressOf('_var_adxtHandles_8c0fcd20') + 0 * 4, 0xcafe0000);
        $this->initUint32($this->addressOf('_var_adxtHandles_8c0fcd20') + 1 * 4, 0xcafe0001);
    }
};
