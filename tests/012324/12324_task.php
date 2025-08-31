<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_controller_0xf06fe_no_stick()
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_8c157a70', 4);

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1); // PDD_DEVTYPE_CONTROLLER
        $this->initUint32($per + 0x04, 0xf06fe);
        $this->initUint32($per + 0x08, 0); // on, nothing
        $this->initUint32($per + 0x10, 0); // press, nothing
        $this->initUint16($per + 0x1c, 0); // x1
        $this->initUint16($per + 0x1e, 0); // y1
        $this->initUint32($per + 0x30, $info);

        $this->initUint32($this->addressOf('_var_8c157ae4'), 0);
        $this->initUint32($this->addressOf('_var_8c157ae8'), 0);

        // Avoid entering the second part of the function
        $this->initUint32($this->addressOf('_var_8c157ad4'), 2);

        $this->call('_PspTask_8c012324');

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')
            ->with(0)
            ->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);

        // TODO: Move implementation to Simulator
        $oddMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8(
                    $dst->value + $i, $this->readUInt8($src->value + $i)
                );
            }
        };

        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);

        $this->shouldWriteLongTo('_var_8c157a70', 0xf06fe);
        $this->shouldWriteLongTo('_var_8c157ae4', 0);
        $this->shouldWriteLongTo('_var_8c157ae8', 0);

        $this->shouldCall('_vmsLcd_8c01c910');
        $this->shouldCall('_FUN_adxVol_8c010a40');
    }

    public function test_controller_0xf06fe_stick_left()
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_8c157a70', 4);

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1); // PDD_DEVTYPE_CONTROLLER
        $this->initUint32($per + 0x04, 0xf06fe);
        $this->initUint32($per + 0x08, 0); // on, nothing
        $this->initUint32($per + 0x10, 0); // press, nothing
        $this->initUint16($per + 0x1c, -65); // x1
        $this->initUint16($per + 0x1e, 0); // y1
        $this->initUint32($per + 0x30, $info);

        $this->initUint32($this->addressOf('_var_8c157ae4'), 0);
        $this->initUint32($this->addressOf('_var_8c157ae8'), 0);

        // Avoid entering the second part of the function
        $this->initUint32($this->addressOf('_var_8c157ad4'), 2);

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')
            ->with(0)
            ->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);

        // TODO: Move implementation to Simulator
        $oddMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8(
                    $dst->value + $i, $this->readUInt8($src->value + $i)
                );
            }
        };

        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);

        $this->shouldWriteLongTo('_var_8c157a70', 0xf06fe);

        $this->shouldWriteLong($this->addressOf('_var_peripherals_8c1ba35c') + 0x08, 0x40);
        $this->shouldWriteLong($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x40);
        $this->shouldWriteLongTo('_var_8c157ae4', 0x40);

        $this->shouldWriteLongTo('_var_8c157ae8', 0);

        $this->shouldCall('_vmsLcd_8c01c910');
        $this->shouldCall('_FUN_adxVol_8c010a40');

        $this->singleCall('_PspTask_8c012324')
            ->run();
    }

    public function test_controller_0xf06fe_stick_left_hold()
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_8c157a70', 4);

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1); // PDD_DEVTYPE_CONTROLLER
        $this->initUint32($per + 0x04, 0xf06fe);
        $this->initUint32($per + 0x08, 0); // on, nothing
        $this->initUint32($per + 0x10, 0); // press, nothing
        $this->initUint16($per + 0x1c, -65); // x1
        $this->initUint16($per + 0x1e, 0); // y1
        $this->initUint32($per + 0x30, $info);

        $this->initUint32($this->addressOf('_var_8c157ae4'), 0x40);
        $this->initUint32($this->addressOf('_var_8c157ae8'), 0);

        // Avoid entering the second part of the function
        $this->initUint32($this->addressOf('_var_8c157ad4'), 2);

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')
            ->with(0)
            ->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);

        // TODO: Move implementation to Simulator
        $oddMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8(
                    $dst->value + $i, $this->readUInt8($src->value + $i)
                );
            }
        };

        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);

        $this->shouldWriteLongTo('_var_8c157a70', 0xf06fe);

        $this->shouldWriteLong($this->addressOf('_var_peripherals_8c1ba35c') + 0x08, 0x40);
        $this->shouldWriteLongTo('_var_8c157ae4', 0x40);

        $this->shouldWriteLongTo('_var_8c157ae8', 0);

        $this->shouldCall('_vmsLcd_8c01c910');
        $this->shouldCall('_FUN_adxVol_8c010a40');

        $this->singleCall('_PspTask_8c012324')
            ->run();
    }

    public function test_controller_0xf06fe_stick_right()
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_8c157a70', 4);

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1); // PDD_DEVTYPE_CONTROLLER
        $this->initUint32($per + 0x04, 0xf06fe);
        $this->initUint32($per + 0x08, 0); // on, nothing
        $this->initUint32($per + 0x10, 0); // press, nothing
        $this->initUint16($per + 0x1c, 65); // x1
        $this->initUint16($per + 0x1e, 0); // y1
        $this->initUint32($per + 0x30, $info);

        $this->initUint32($this->addressOf('_var_8c157ae4'), 0);
        $this->initUint32($this->addressOf('_var_8c157ae8'), 0);

        // Avoid entering the second part of the function
        $this->initUint32($this->addressOf('_var_8c157ad4'), 2);

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')
            ->with(0)
            ->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);

        // TODO: Move implementation to Simulator
        $oddMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8(
                    $dst->value + $i, $this->readUInt8($src->value + $i)
                );
            }
        };

        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);

        $this->shouldWriteLongTo('_var_8c157a70', 0xf06fe);

        $this->shouldWriteLong($this->addressOf('_var_peripherals_8c1ba35c') + 0x08, 0x80);
        $this->shouldWriteLong($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x80);
        $this->shouldWriteLongTo('_var_8c157ae4', 0x80);

        $this->shouldWriteLongTo('_var_8c157ae8', 0);

        $this->shouldCall('_vmsLcd_8c01c910');
        $this->shouldCall('_FUN_adxVol_8c010a40');

        $this->singleCall('_PspTask_8c012324')
            ->run();
    }

    public function test_controller_0xf06fe_stick_up()
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_8c157a70', 4);

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1); // PDD_DEVTYPE_CONTROLLER
        $this->initUint32($per + 0x04, 0xf06fe);
        $this->initUint32($per + 0x08, 0); // on, nothing
        $this->initUint32($per + 0x10, 0); // press, nothing
        $this->initUint16($per + 0x1c, 0); // x1
        $this->initUint16($per + 0x1e, -65); // y1
        $this->initUint32($per + 0x30, $info);

        $this->initUint32($this->addressOf('_var_8c157ae4'), 0);
        $this->initUint32($this->addressOf('_var_8c157ae8'), 0);

        // Avoid entering the second part of the function
        $this->initUint32($this->addressOf('_var_8c157ad4'), 2);

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')
            ->with(0)
            ->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);

        // TODO: Move implementation to Simulator
        $oddMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8(
                    $dst->value + $i, $this->readUInt8($src->value + $i)
                );
            }
        };

        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);

        $this->shouldWriteLongTo('_var_8c157a70', 0xf06fe);

        $this->shouldWriteLongTo('_var_8c157ae4', 0);

        $this->shouldWriteLong($this->addressOf('_var_peripherals_8c1ba35c') + 0x08, 0x10);
        $this->shouldWriteLong($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x10);
        $this->shouldWriteLongTo('_var_8c157ae8', 0x10);

        $this->shouldCall('_vmsLcd_8c01c910');
        $this->shouldCall('_FUN_adxVol_8c010a40');

        $this->singleCall('_PspTask_8c012324')
            ->run();
    }

    public function test_controller_0xf06fe_stick_up_hold()
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_8c157a70', 4);

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1); // PDD_DEVTYPE_CONTROLLER
        $this->initUint32($per + 0x04, 0xf06fe);
        $this->initUint32($per + 0x08, 0); // on, nothing
        $this->initUint32($per + 0x10, 0); // press, nothing
        $this->initUint16($per + 0x1c, 0); // x1
        $this->initUint16($per + 0x1e, -65); // y1
        $this->initUint32($per + 0x30, $info);

        $this->initUint32($this->addressOf('_var_8c157ae4'), 0);
        $this->initUint32($this->addressOf('_var_8c157ae8'), 0x10);

        // Avoid entering the second part of the function
        $this->initUint32($this->addressOf('_var_8c157ad4'), 2);

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')
            ->with(0)
            ->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);

        // TODO: Move implementation to Simulator
        $oddMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8(
                    $dst->value + $i, $this->readUInt8($src->value + $i)
                );
            }
        };

        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);

        $this->shouldWriteLongTo('_var_8c157a70', 0xf06fe);

        $this->shouldWriteLongTo('_var_8c157ae4', 0);

        $this->shouldWriteLong($this->addressOf('_var_peripherals_8c1ba35c') + 0x08, 0x10);
        $this->shouldWriteLongTo('_var_8c157ae8', 0x10);

        $this->shouldCall('_vmsLcd_8c01c910');
        $this->shouldCall('_FUN_adxVol_8c010a40');

        $this->singleCall('_PspTask_8c012324')
            ->run();
    }

    public function test_controller_0xf06fe_stick_down()
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_8c157a70', 4);

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1); // PDD_DEVTYPE_CONTROLLER
        $this->initUint32($per + 0x04, 0xf06fe);
        $this->initUint32($per + 0x08, 0); // on, nothing
        $this->initUint32($per + 0x10, 0); // press, nothing
        $this->initUint16($per + 0x1c, 0); // x1
        $this->initUint16($per + 0x1e, 65); // y1
        $this->initUint32($per + 0x30, $info);

        $this->initUint32($this->addressOf('_var_8c157ae4'), 0);
        $this->initUint32($this->addressOf('_var_8c157ae8'), 0);

        // Avoid entering the second part of the function
        $this->initUint32($this->addressOf('_var_8c157ad4'), 2);

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')
            ->with(0)
            ->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);

        // TODO: Move implementation to Simulator
        $oddMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8(
                    $dst->value + $i, $this->readUInt8($src->value + $i)
                );
            }
        };

        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);

        $this->shouldWriteLongTo('_var_8c157a70', 0xf06fe);

        $this->shouldWriteLongTo('_var_8c157ae4', 0);

        $this->shouldWriteLong($this->addressOf('_var_peripherals_8c1ba35c') + 0x08, 0x20);
        $this->shouldWriteLong($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0x20);
        $this->shouldWriteLongTo('_var_8c157ae8', 0x20);

        $this->shouldCall('_vmsLcd_8c01c910');
        $this->shouldCall('_FUN_adxVol_8c010a40');

        $this->singleCall('_PspTask_8c012324')
            ->run();
    }

    public function test_controller_0xf06fe_start_abxy()
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_8c157a70', 4);

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1); // PDD_DEVTYPE_CONTROLLER
        $this->initUint32($per + 0x04, 0xf06fe);
        $this->initUint32($per + 0x08, 0x606); // on, ABXY
        $this->initUint32($per + 0x10, 0x8); // press, Start
        $this->initUint16($per + 0x1c, 0); // x1
        $this->initUint16($per + 0x1e, 0); // y1
        $this->initUint32($per + 0x30, $info);

        $this->initUint32($this->addressOf('_var_8c157ae4'), 0);
        $this->initUint32($this->addressOf('_var_8c157ae8'), 0);

        // Avoid entering the second part of the function
        $this->initUint32($this->addressOf('_var_8c157ad4'), 2);

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')
            ->with(0)
            ->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);

        // TODO: Move implementation to Simulator
        $oddMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8(
                    $dst->value + $i, $this->readUInt8($src->value + $i)
                );
            }
        };

        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);

        $this->shouldWriteLongTo('_var_8c157a70', 0xf06fe);
        $this->shouldWriteLongTo('_var_8c157ae4', 0);
        $this->shouldWriteLongTo('_var_8c157ae8', 0);

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 1);

        $this->shouldCall('_vmsLcd_8c01c910');
        $this->shouldCall('_FUN_adxVol_8c010a40');

        $this->singleCall('_PspTask_8c012324')
            ->run();
    }

    public function test_controller_0x700fe_nothing()
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_8c157a70', 4);

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1); // PDD_DEVTYPE_CONTROLLER
        $this->initUint32($per + 0x04, 0x700fe);
        $this->initUint32($per + 0x08, 0); // on, nothing
        $this->initUint32($per + 0x10, 0); // press, nothing
        $this->initUint16($per + 0x1c, 0); // x1
        $this->initUint16($per + 0x1e, 0); // y1
        $this->initUint32($per + 0x30, $info);

        $this->initUint32($this->addressOf('_var_8c157ae4'), 0);
        $this->initUint32($this->addressOf('_var_8c157ae8'), 0);

        // Avoid entering the second part of the function
        $this->initUint32($this->addressOf('_var_8c157ad4'), 2);

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')
            ->with(0)
            ->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);

        // TODO: Move implementation to Simulator
        $oddMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8(
                    $dst->value + $i, $this->readUInt8($src->value + $i)
                );
            }
        };

        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);

        $this->shouldWriteLongTo('_var_8c157a70', 0x700fe);

        $this->shouldCall('_vmsLcd_8c01c910');
        $this->shouldCall('_FUN_adxVol_8c010a40');

        $this->singleCall('_PspTask_8c012324')
            ->run();
    }

    public function test_controller_0x700fe_pressed()
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_8c157a70', 4);

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1); // PDD_DEVTYPE_CONTROLLER
        $this->initUint32($per + 0x04, 0x700fe);
        $this->initUint32($per + 0x08, 0x606); // on, ABXY
        $this->initUint32($per + 0x10, 8); // press, Start
        $this->initUint16($per + 0x1c, 0); // x1
        $this->initUint16($per + 0x1e, 0); // y1
        $this->initUint32($per + 0x30, $info);

        $this->initUint32($this->addressOf('_var_8c157ae4'), 0);
        $this->initUint32($this->addressOf('_var_8c157ae8'), 0);

        // Avoid entering the second part of the function
        $this->initUint32($this->addressOf('_var_8c157ad4'), 2);

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')
            ->with(0)
            ->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);

        // TODO: Move implementation to Simulator
        $oddMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8(
                    $dst->value + $i, $this->readUInt8($src->value + $i)
                );
            }
        };

        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);

        $this->shouldWriteLongTo('_var_8c157a70', 0x700fe);
        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 1);

        $this->shouldCall('_vmsLcd_8c01c910');
        $this->shouldCall('_FUN_adxVol_8c010a40');

        $this->singleCall('_PspTask_8c012324')->run();
    }

    public function test_unsuported_controller()
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_const_peripheral_8c033318', 0x34);
        $this->setSize('_var_8c157a70', 4);

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1); // PDD_DEVTYPE_CONTROLLER
        $this->initUint32($per + 0x04, 0x01337);
        $this->initUint32($per + 0x08, 0); // on, nothing
        $this->initUint32($per + 0x10, 0); // press, nothing
        $this->initUint16($per + 0x1c, 0); // x1
        $this->initUint16($per + 0x1e, 0); // y1
        $this->initUint32($per + 0x30, $info);

        $this->initUint32($this->addressOf('_var_8c157ae4'), 0);
        $this->initUint32($this->addressOf('_var_8c157ae8'), 0);

        // Avoid entering the second part of the function
        $this->initUint32($this->addressOf('_var_8c157ad4'), 2);

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')
            ->with(0)
            ->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);

        // TODO: Move implementation to Simulator
        $oddMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8(
                    $dst->value + $i, $this->readUInt8($src->value + $i)
                );
            }
        };

        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);

        $this->shouldWriteLongTo('_var_vibport_8c1ba354', -1);
        $this->shouldWriteLongTo('_var_8c157a70', -1);

        $this->shouldCall('_vmsLcd_8c01c910');
        $this->shouldCall('_FUN_adxVol_8c010a40');

        $this->singleCall('_PspTask_8c012324')->run();
    }

    public function test_second_part_zero_nothing()
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_const_peripheral_8c033318', 0x34);
        $this->initUint32Array(
            $this->addressOf('_const_peripheral_8c033318'),
            array_fill(0, 0x34 / 4, 0),
        );
        $this->setSize('_var_8c157a70', 4);

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1); // PDD_DEVTYPE_CONTROLLER
        $this->initUint32($per + 0x04, 0x01337);
        $this->initUint32($per + 0x08, 0); // on, nothing
        $this->initUint32($per + 0x10, 0); // press, nothing
        $this->initUint16($per + 0x1c, 0); // x1
        $this->initUint16($per + 0x1e, 0); // y1
        $this->initUint32($per + 0x30, $info);

        $this->initUint32($this->addressOf('_var_8c157ae4'), 0);
        $this->initUint32($this->addressOf('_var_8c157ae8'), 0);

        $this->initUint32($this->addressOf('_var_8c157ad4'), 0);

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')
            ->with(0)
            ->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);

        // TODO: Move implementation to Simulator
        $oddMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8(
                    $dst->value + $i, $this->readUInt8($src->value + $i)
                );
            }
        };

        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);

        $this->shouldWriteLongTo('_var_vibport_8c1ba354', -1);
        $this->shouldWriteLongTo('_var_8c157a70', -1);

        $this->shouldCall('_vmsLcd_8c01c910');
        $this->shouldCall('_FUN_adxVol_8c010a40');

        $this->singleCall('_PspTask_8c012324')->run();
    }

    public function test_second_part_zero_up()
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_const_peripheral_8c033318', 0x34);
        $this->initUint32Array(
            $this->addressOf('_const_peripheral_8c033318'),
            array_fill(0, 0x34 / 4, 0),
        );
        $this->setSize('_var_8c157a70', 4);

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1); // PDD_DEVTYPE_CONTROLLER
        $this->initUint32($per + 0x04, 0xf06fe);
        $this->initUint32($per + 0x08, 0x10); // on, up
        $this->initUint32($per + 0x10, 0); // press, nothing
        $this->initUint16($per + 0x1c, 0); // x1
        $this->initUint16($per + 0x1e, 0); // y1
        $this->initUint32($per + 0x30, $info);

        $this->initUint32($this->addressOf('_var_8c157ae4'), 0);
        $this->initUint32($this->addressOf('_var_8c157ae8'), 0);

        $this->initUint32($this->addressOf('_var_8c157ad4'), 0);

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')
            ->with(0)
            ->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);

        // TODO: Move implementation to Simulator
        $oddMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8(
                    $dst->value + $i, $this->readUInt8($src->value + $i)
                );
            }
        };

        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);

        $this->shouldWriteLongTo('_var_8c157a70', 0xf06fe);
        $this->shouldWriteLongTo('_var_8c157ae4', 0);
        $this->shouldWriteLongTo('_var_8c157ae8', 0);

        $this->shouldWriteLong($this->addressOf('_var_8c157ad4') + 0 * 0x4, 1);
        $this->shouldWriteLong($this->addressOf('_var_8c157ad4') + 1 * 0x4, 0);
        $this->shouldWriteLong($this->addressOf('_var_8c157ad4') + 2 * 0x4, 15);
        $this->shouldWriteLong($this->addressOf('_var_8c157ad4') + 3 * 0x4, 0);

        $this->shouldCall('_vmsLcd_8c01c910');
        $this->shouldCall('_FUN_adxVol_8c010a40');

        $this->singleCall('_PspTask_8c012324')->run();
    }

    public function test_second_part_one_nothing()
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_const_peripheral_8c033318', 0x34);
        $this->initUint32Array(
            $this->addressOf('_const_peripheral_8c033318'),
            array_fill(0, 0x34 / 4, 0),
        );
        $this->setSize('_var_8c157a70', 4);

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1); // PDD_DEVTYPE_CONTROLLER
        $this->initUint32($per + 0x04, 0xf06fe);
        $this->initUint32($per + 0x08, 0); // on, nothing
        $this->initUint32($per + 0x10, 0); // press, nothing
        $this->initUint16($per + 0x1c, 0); // x1
        $this->initUint16($per + 0x1e, 0); // y1
        $this->initUint32($per + 0x30, $info);

        $this->initUint32($this->addressOf('_var_8c157ae4'), 0);
        $this->initUint32($this->addressOf('_var_8c157ae8'), 0);

        $this->initUint32($this->addressOf('_var_8c157ad4'), 1);
        // $this->initUint32($this->addressOf('_var_8c157ad4') + 1 * 0x04, 0);
        // $this->initUint32($this->addressOf('_var_8c157ad4') + 2 * 0x04, 42);
        // $this->initUint32($this->addressOf('_var_8c157ad4') + 3 * 0x04, 0);

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')
            ->with(0)
            ->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);

        // TODO: Move implementation to Simulator
        $oddMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8(
                    $dst->value + $i, $this->readUInt8($src->value + $i)
                );
            }
        };

        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);

        $this->shouldWriteLongTo('_var_8c157a70', 0xf06fe);
        $this->shouldWriteLongTo('_var_8c157ae4', 0);
        $this->shouldWriteLongTo('_var_8c157ae8', 0);

        $this->shouldWriteLong($this->addressOf('_var_8c157ad4'), 0);

        $this->shouldCall('_vmsLcd_8c01c910');
        $this->shouldCall('_FUN_adxVol_8c010a40');

        $this->singleCall('_PspTask_8c012324')->run();
    }

    public function test_second_part_one_up()
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_const_peripheral_8c033318', 0x34);
        $this->initUint32Array(
            $this->addressOf('_const_peripheral_8c033318'),
            array_fill(0, 0x34 / 4, 0),
        );
        $this->setSize('_var_8c157a70', 4);

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1); // PDD_DEVTYPE_CONTROLLER
        $this->initUint32($per + 0x04, 0xf06fe);
        $this->initUint32($per + 0x08, 0x10); // on, up
        $this->initUint32($per + 0x10, 0); // press, nothing
        $this->initUint16($per + 0x1c, 0); // x1
        $this->initUint16($per + 0x1e, 0); // y1
        $this->initUint32($per + 0x30, $info);

        $this->initUint32($this->addressOf('_var_8c157ae4'), 0);
        $this->initUint32($this->addressOf('_var_8c157ae8'), 0);

        $this->initUint32($this->addressOf('_var_8c157ad4') + 0 * 0x4, 1);
        $this->initUint32($this->addressOf('_var_8c157ad4') + 1 * 0x4, 0);
        $this->initUint32($this->addressOf('_var_8c157ad4') + 2 * 0x4, 42);
        $this->initUint32($this->addressOf('_var_8c157ad4') + 3 * 0x4, 5);

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')
            ->with(0)
            ->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);

        // TODO: Move implementation to Simulator
        $oddMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8(
                    $dst->value + $i, $this->readUInt8($src->value + $i)
                );
            }
        };

        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);

        $this->shouldWriteLongTo('_var_8c157a70', 0xf06fe);
        $this->shouldWriteLongTo('_var_8c157ae4', 0);
        $this->shouldWriteLongTo('_var_8c157ae8', 0);

        $this->shouldWriteLong($this->addressOf('_var_8c157ad4') + 1 * 0x4, 1);
        $this->shouldWriteLong($this->addressOf('_var_8c157ad4') + 3 * 0x4, 6);

        $this->shouldCall('_vmsLcd_8c01c910');
        $this->shouldCall('_FUN_adxVol_8c010a40');

        $this->singleCall('_PspTask_8c012324')->run();
    }

    public function test_second_part_one_path_a()
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_const_peripheral_8c033318', 0x34);
        $this->initUint32Array(
            $this->addressOf('_const_peripheral_8c033318'),
            array_fill(0, 0x34 / 4, 0),
        );
        $this->setSize('_var_8c157a70', 4);

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1); // PDD_DEVTYPE_CONTROLLER
        $this->initUint32($per + 0x04, 0xf06fe);
        $this->initUint32($per + 0x08, 0xff); // on, up
        $this->initUint32($per + 0x10, 0x01); // press, nothing
        $this->initUint16($per + 0x1c, 0); // x1
        $this->initUint16($per + 0x1e, 0); // y1
        $this->initUint32($per + 0x30, $info);

        $this->initUint32($this->addressOf('_var_8c157ae4'), 0);
        $this->initUint32($this->addressOf('_var_8c157ae8'), 0);

        $this->initUint32($this->addressOf('_var_8c157ad4') + 0 * 0x4, 1);
        $this->initUint32($this->addressOf('_var_8c157ad4') + 1 * 0x4, 42);
        $this->initUint32($this->addressOf('_var_8c157ad4') + 2 * 0x4, 42);
        $this->initUint32($this->addressOf('_var_8c157ad4') + 3 * 0x4, 5);

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')
            ->with(0)
            ->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);

        // TODO: Move implementation to Simulator
        $oddMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8(
                    $dst->value + $i, $this->readUInt8($src->value + $i)
                );
            }
        };

        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);

        $this->shouldWriteLongTo('_var_8c157a70', 0xf06fe);
        $this->shouldWriteLongTo('_var_8c157ae4', 0);
        $this->shouldWriteLongTo('_var_8c157ae8', 0);

        $this->shouldWriteLong($this->addressOf('_var_8c157ad4') + 1 * 0x4, 43);
        $this->shouldWriteLong($this->addressOf('_var_8c157ad4') + 1 * 0x4, 0);
        $this->shouldWriteLong($this->addressOf('_var_8c157ad4') + 2 * 0x4, 6);
        $this->shouldWriteLong($this->addressOf('_var_peripherals_8c1ba35c') + 0x10, 0xf1);
        $this->shouldWriteLong($this->addressOf('_var_8c157ad4') + 3 * 0x4, 6);

        $this->shouldCall('_vmsLcd_8c01c910');
        $this->shouldCall('_FUN_adxVol_8c010a40');

        $this->singleCall('_PspTask_8c012324')->run();
    }

    public function test_second_part_one_path_b()
    {
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_const_peripheral_8c033318', 0x34);
        $this->initUint32Array(
            $this->addressOf('_const_peripheral_8c033318'),
            array_fill(0, 0x34 / 4, 0),
        );
        $this->setSize('_var_8c157a70', 4);

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1); // PDD_DEVTYPE_CONTROLLER
        $this->initUint32($per + 0x04, 0xf06fe);
        $this->initUint32($per + 0x08, 0xff); // on, up
        $this->initUint32($per + 0x10, 0x01); // press, nothing
        $this->initUint16($per + 0x1c, 0); // x1
        $this->initUint16($per + 0x1e, 0); // y1
        $this->initUint32($per + 0x30, $info);

        $this->initUint32($this->addressOf('_var_8c157ae4'), 0);
        $this->initUint32($this->addressOf('_var_8c157ae8'), 0);

        $this->initUint32($this->addressOf('_var_8c157ad4') + 0 * 0x4, 1);
        $this->initUint32($this->addressOf('_var_8c157ad4') + 1 * 0x4, 0);
        $this->initUint32($this->addressOf('_var_8c157ad4') + 2 * 0x4, 42);
        $this->initUint32($this->addressOf('_var_8c157ad4') + 3 * 0x4, 30);

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')
            ->with(0)
            ->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);

        // TODO: Move implementation to Simulator
        $oddMvn = function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8(
                    $dst->value + $i, $this->readUInt8($src->value + $i)
                );
            }
        };

        $this->shouldCall('__quick_odd_mvn')->do($oddMvn);

        $this->shouldWriteLongTo('_var_8c157a70', 0xf06fe);
        $this->shouldWriteLongTo('_var_8c157ae4', 0);
        $this->shouldWriteLongTo('_var_8c157ae8', 0);

        $this->shouldWriteLong($this->addressOf('_var_8c157ad4') + 1 * 0x4, 1);
        $this->shouldWriteLong($this->addressOf('_var_8c157ad4') + 3 * 0x4, 31);
        $this->shouldWriteLong($this->addressOf('_var_8c157ad4') + 2 * 0x4, 1);

        $this->shouldCall('_vmsLcd_8c01c910');
        $this->shouldCall('_FUN_adxVol_8c010a40');

        $this->singleCall('_PspTask_8c012324')->run();
    }

    private function initUint32Array(int $address, array $values): void
    {
        foreach ($values as $i => $value) {
            $this->initUint32($address + $i * 4, $value);
        }
    }
};