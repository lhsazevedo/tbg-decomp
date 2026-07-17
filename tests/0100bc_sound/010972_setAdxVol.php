<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_setField0x00Vol1()
    {
        $this->doTestWithVol(1);
    }

    public function test_setField0x00Vol2()
    {
        $this->doTestWithVol(2);
    }

    public function test_setField0x00Vol3()
    {
        $this->doTestWithVol(3);
    }

    public function test_setField0x00Vol4()
    {
        $this->resolveSymbols();

        // TODO: Move implementation to Simulator
        $mvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8($dst->value + $i, $this->readUInt8($src->value + $i));
            }
        };

        $this->shouldCall('__quick_evn_mvn')->do($mvn);

        $this->shouldWriteLong($this->addressOf('_init_adxVolumes_8c03bd88') + 0, 440);
        $this->shouldCall('_ADXT_SetOutVol')->with(0xcafe0000, 440 - 990);

        $this->singleCall('_SndSetAdxVol_8c010972')
            ->with(4, 0)
            ->run();
    }

    public function test_setField0x00Vol5()
    {
        $this->doTestWithVol(5);
    }

    public function test_setField0x00Vol6()
    {
        $this->doTestWithVol(6);
    }

    public function test_setField0x00Vol7()
    {
        $this->doTestWithVol(7);
    }

    public function test_setField0x00Vol8()
    {
        $this->doTestWithVol(8);
    }

    public function test_setField0x00Vol9()
    {
        $this->doTestWithVol(9);
    }

    protected function doTestWithVol(int $volNo)
    {
        $this->resolveSymbols();

        // Initialized data
        $vols = [
            0,
            110,
            220,
            330,
            440,
            550,
            660,
            770,
            880,
            990
        ];

        // TODO: Move implementation to Simulator
        $mvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8($dst->value + $i, $this->readUInt8($src->value + $i));
            }
        };

        $this->shouldCall('__quick_evn_mvn')->do($mvn);

        $this->shouldWriteLong($this->addressOf('_init_adxVolumes_8c03bd88') + 0, $vols[$volNo]);
        $this->shouldCall('_ADXT_SetOutVol')->with(0xcafe0000, $vols[$volNo] - 990);

        $this->singleCall('_SndSetAdxVol_8c010972')
            ->with($volNo, 0)
            ->run();
    }

    public function test_setField0x04Vol2()
    {
        $this->resolveSymbols();

        // TODO: Move implementation to Simulator
        $mvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8($dst->value + $i, $this->readUInt8($src->value + $i));
            }
        };

        $this->shouldCall('__quick_evn_mvn')->do($mvn);

        $this->shouldWriteLong($this->addressOf('_init_adxVolumes_8c03bd88') + 4, 220);
        $this->shouldCall('_ADXT_SetOutVol')->with(0xcafe0001, 220 - 990);

        $this->singleCall('_SndSetAdxVol_8c010972')
            ->with(2, 1)
            ->run();
    }

    public function test_setField0x04Vol4()
    {
        $this->resolveSymbols();

        // TODO: Move implementation to Simulator
        $mvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8($dst->value + $i, $this->readUInt8($src->value + $i));
            }
        };

        $this->shouldCall('__quick_evn_mvn')->do($mvn);

        $this->shouldWriteLong($this->addressOf('_init_adxVolumes_8c03bd88') + 4, 440);
        $this->shouldCall('_ADXT_SetOutVol')->with(0xcafe0001, 440 - 990);

        $this->singleCall('_SndSetAdxVol_8c010972')
            ->with(4, 1)
            ->run();
    }

    private function resolveSymbols(): void
    {
        // Functions

        // Basic inits
        $this->initUint32($this->addressOf('_var_adxtHandles_8c0fcd20') + 0 * 4, 0xcafe0000);
        $this->initUint32($this->addressOf('_var_adxtHandles_8c0fcd20') + 1 * 4, 0xcafe0001);
    }
};
