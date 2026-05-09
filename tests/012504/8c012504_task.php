<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

return new class extends TestCase {
    public function test_not_controller_type()
    {
        $this->resolveSymbols();

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 0);               // *info & 1 == 0 -> not a controller
        $this->initUint32($per + 0x04, 0xf06fe);   // support bits (irrelevant for this path)
        $this->initUint32($per + 0x30, $info);

        $this->call('_task_8c012504');

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')->with(0)->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);
        $this->shouldCall('__quick_odd_mvn')->do($this->oddMvn());
        $this->shouldWriteLongTo('_var_vibport_8c1ba354', -1);
        $this->shouldWriteLongTo('_var_8c157a70', -1);
        $this->shouldCall('_vmsLcd_8c01c910');
    }

    public function test_unsupported_controller()
    {
        $this->resolveSymbols();

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1);    // *info & 1 == 1 -> controller bit set
        $this->initUint32($per + 0x04, 0); // support & 0xF06FE = 0 -> matches neither type
        $this->initUint32($per + 0x30, $info);

        $this->call('_task_8c012504');

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')->with(0)->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);
        $this->shouldCall('__quick_odd_mvn')->do($this->oddMvn());
        $this->shouldWriteLongTo('_var_vibport_8c1ba354', -1);
        $this->shouldWriteLongTo('_var_8c157a70', -1);
        $this->shouldCall('_vmsLcd_8c01c910');
    }

    public function test_standard_controller()
    {
        $this->resolveSymbols();

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1);             // controller type bit set
        $this->initUint32($per + 0x04, 0xf06fe); // standard controller support bits
        $this->initUint16($per + 0x18, 0);
        $this->initUint16($per + 0x1a, 0);
        $this->initUint16($per + 0x1c, 0);
        $this->initUint32($per + 0x30, $info);

        $buf = $this->addressOf('_var_peripherals_8c1ba35c');

        $this->call('_task_8c012504');

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')->with(0)->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);
        $this->shouldWriteWord($buf + 0x18, 0);
        $this->shouldWriteWord($buf + 0x1a, 0);
        $this->shouldWriteWord($buf + 0x1c, 0);
        $this->shouldWriteLong($buf + 0x08, 0);
        $this->shouldWriteLong($buf + 0x10, 0);
        $this->shouldWriteLongTo('_var_8c157a70', 0xf06fe);

        // TODO: test full behavior instead of stopping early
        $this->forceStop();
    }

    public function test_racing_controller()
    {
        $this->resolveSymbols();

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1);             // controller type bit set
        $this->initUint32($per + 0x04, 0x700fe); // racing controller support bits
        $this->initUint16($per + 0x18, 0);
        $this->initUint16($per + 0x1a, 0);
        $this->initUint16($per + 0x1c, 0);
        $this->initUint32($per + 0x30, $info);

        $buf = $this->addressOf('_var_peripherals_8c1ba35c');

        $this->call('_task_8c012504');

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')->with(0)->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);
        $this->shouldWriteWord($buf + 0x18, 0);
        $this->shouldWriteWord($buf + 0x1a, 0);
        $this->shouldWriteWord($buf + 0x1c, 0);
        $this->shouldWriteLong($buf + 0x08, 0);
        $this->shouldWriteLong($buf + 0x10, 0);
        $this->shouldWriteLongTo('_var_8c157a70', 0x700fe);

        // TODO: test full behavior instead of stopping early
        $this->forceStop();
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_vibport_8c1ba354', 4);
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_8c157a70', 4);
        $this->setSize('_vmsLcd_8c01c910', 4);
        $this->initUint32Array(
            $this->addressOf('_const_peripheral_8c033318'),
            array_fill(0, 0x34 / 4, 0),
        );
    }

    private function oddMvn(): Closure
    {
        return function () {
            $src = $this->registers[2];
            $dst = $this->registers[1];
            $len = $this->registers[0];

            for ($i = 0; $i < $len->value; $i++) {
                $this->memory->writeUInt8(
                    $dst->value + $i, $this->readUInt8($src->value + $i)
                );
            }
        };
    }

    private function initUint32Array(int $address, array $values): void
    {
        foreach ($values as $i => $value) {
            $this->initUint32($address + $i * 4, $value);
        }
    }
};
