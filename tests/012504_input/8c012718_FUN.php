<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;

/* Twin of inputTask_8c012504: alt tables, no paddle-shift remap. */
return new class extends TestCase {
    public function test_not_controller_type()
    {
        $this->resolveSymbols();

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 0);               // *info & 1 == 0 -> not a controller
        $this->initUint32($per + 0x04, 0xf06fe);   // support bits (irrelevant for this path)
        $this->initUint32($per + 0x30, $info);

        $this->call('_inputTaskAlt_8c012718');

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')->with(0)->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);
        $this->shouldCall('__quick_odd_mvn')->do($this->oddMvn());
        $this->shouldWriteLongTo('_var_vibport_8c1ba354', -1);
        $this->shouldWriteLongTo('_var_activeCtrlType_8c157a70', -1);
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

        $this->call('_inputTaskAlt_8c012718');

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')->with(0)->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);
        $this->shouldCall('__quick_odd_mvn')->do($this->oddMvn());
        $this->shouldWriteLongTo('_var_vibport_8c1ba354', -1);
        $this->shouldWriteLongTo('_var_activeCtrlType_8c157a70', -1);
        $this->shouldCall('_vmsLcd_8c01c910');
    }

    public function test_standard_controller_translates_inputs()
    {
        $this->resolveSymbols();
        $this->setupStandardTable();

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1);
        $this->initUint32($per + 0x04, 0xf06fe);
        $this->initUint16($per + 0x18, 0);
        $this->initUint16($per + 0x1a, 0);
        $this->initUint16($per + 0x1c, 0);
        $this->initUint32($per + 0x30, $info);

        // Two pairs in init_8c03beb8; rest zeroed.
        $table = $this->addressOf('_init_8c03beb8');
        $this->initUint32($table + 0x00, 0x0001); // pair 0 mask
        $this->initUint32($table + 0x04, 0x0100); // pair 0 button
        $this->initUint32($table + 0x08, 0x0002); // pair 1 mask
        $this->initUint32($table + 0x0c, 0x0200); // pair 1 button

        $this->initUint32($per + 0x08, 0x0003); // on: matches both pairs
        $this->initUint32($per + 0x10, 0x0001); // press: matches only pair 0

        $buf = $this->addressOf('_var_peripherals_8c1ba35c');

        $this->call('_inputTaskAlt_8c012718');

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')->with(0)->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);
        $this->shouldWriteWord($buf + 0x18, 0);
        $this->shouldWriteWord($buf + 0x1a, 0);
        $this->shouldWriteWord($buf + 0x1c, 0);
        $this->shouldWriteLong($buf + 0x08, 0);
        $this->shouldWriteLong($buf + 0x10, 0);
        $this->shouldWriteLongTo('_var_activeCtrlType_8c157a70', 0xf06fe);

        // on-loop: pair 0 matches -> .on |= 0x100, then pair 1 -> .on |= 0x200
        $this->shouldWriteLong($buf + 0x08, 0x0100);
        $this->shouldWriteLong($buf + 0x08, 0x0300);
        // press-loop: only pair 0 matches -> .press |= 0x100
        $this->shouldWriteLong($buf + 0x10, 0x0100);

        // press & 8 = 0, reset combo skipped.
        $this->forceStop();
    }

    public function test_standard_controller_reset_combo_triggers()
    {
        $this->resolveSymbols();
        $this->setupStandardTable();

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1);
        $this->initUint32($per + 0x04, 0xf06fe);
        $this->initUint16($per + 0x18, 0);
        $this->initUint16($per + 0x1a, 0);
        $this->initUint16($per + 0x1c, 0);
        $this->initUint32($per + 0x30, $info);

        // init_8c03beb8 all zeros -> loops produce no writes.
        // Sega mandatory reset combo: Start + A + B + X + Y.
        $this->initUint32($per + 0x08, 0x0606); // on: A|B|X|Y held
        $this->initUint32($per + 0x10, 0x0008); // press: Start pressed

        $buf = $this->addressOf('_var_peripherals_8c1ba35c');

        $this->call('_inputTaskAlt_8c012718');

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')->with(0)->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);
        $this->shouldWriteWord($buf + 0x18, 0);
        $this->shouldWriteWord($buf + 0x1a, 0);
        $this->shouldWriteWord($buf + 0x1c, 0);
        $this->shouldWriteLong($buf + 0x08, 0);
        $this->shouldWriteLong($buf + 0x10, 0);
        $this->shouldWriteLongTo('_var_activeCtrlType_8c157a70', 0xf06fe);

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 1);

        $this->forceStop();
    }

    public function test_standard_controller_reset_combo_partial_hold()
    {
        $this->resolveSymbols();
        $this->setupStandardTable();

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1);
        $this->initUint32($per + 0x04, 0xf06fe);
        $this->initUint16($per + 0x18, 0);
        $this->initUint16($per + 0x1a, 0);
        $this->initUint16($per + 0x1c, 0);
        $this->initUint32($per + 0x30, $info);

        // Start is pressed but only A|B held (X|Y = 0x600 missing) -> no reset.
        $this->initUint32($per + 0x08, 0x0006);
        $this->initUint32($per + 0x10, 0x0008);

        $buf = $this->addressOf('_var_peripherals_8c1ba35c');

        $this->call('_inputTaskAlt_8c012718');

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')->with(0)->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);
        $this->shouldWriteWord($buf + 0x18, 0);
        $this->shouldWriteWord($buf + 0x1a, 0);
        $this->shouldWriteWord($buf + 0x1c, 0);
        $this->shouldWriteLong($buf + 0x08, 0);
        $this->shouldWriteLong($buf + 0x10, 0);
        $this->shouldWriteLongTo('_var_activeCtrlType_8c157a70', 0xf06fe);

        // No second write to _var_resetRequested_8c157a78 (combo incomplete).

        $this->forceStop();
    }

    public function test_racing_controller_translates_inputs()
    {
        $this->resolveSymbols();
        $this->setupRacingTable();

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1);
        $this->initUint32($per + 0x04, 0x700fe);
        $this->initUint16($per + 0x18, 0);
        $this->initUint16($per + 0x1a, 0);
        $this->initUint16($per + 0x1c, 0);
        $this->initUint32($per + 0x30, $info);

        // Two (mask, button) pairs at start of init_8c03bf18; rest stay 0.
        $table = $this->addressOf('_init_8c03bf18');
        $this->initUint32($table + 0x00, 0x0001); // pair 0 mask
        $this->initUint32($table + 0x04, 0x0100); // pair 0 button
        $this->initUint32($table + 0x08, 0x0002); // pair 1 mask
        $this->initUint32($table + 0x0c, 0x0200); // pair 1 button

        $this->initUint32($per + 0x08, 0x0003); // on: matches both pairs
        $this->initUint32($per + 0x10, 0x0001); // press: matches only pair 0

        $buf = $this->addressOf('_var_peripherals_8c1ba35c');

        $this->call('_inputTaskAlt_8c012718');

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')->with(0)->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);
        $this->shouldWriteWord($buf + 0x18, 0);
        $this->shouldWriteWord($buf + 0x1a, 0);
        $this->shouldWriteWord($buf + 0x1c, 0);
        $this->shouldWriteLong($buf + 0x08, 0);
        $this->shouldWriteLong($buf + 0x10, 0);
        $this->shouldWriteLongTo('_var_activeCtrlType_8c157a70', 0x700fe);

        // on-loop: pair 0 then pair 1 match.
        $this->shouldWriteLong($buf + 0x08, 0x0100);
        $this->shouldWriteLong($buf + 0x08, 0x0300);
        // press-loop: only pair 0 matches.
        $this->shouldWriteLong($buf + 0x10, 0x0100);

        // press & 8 == 0 -> reset combo skipped.
        $this->forceStop();
    }

    public function test_racing_controller_reset_combo_triggers()
    {
        $this->resolveSymbols();
        $this->setupRacingTable();

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1);
        $this->initUint32($per + 0x04, 0x700fe);
        $this->initUint16($per + 0x18, 0);
        $this->initUint16($per + 0x1a, 0);
        $this->initUint16($per + 0x1c, 0);
        $this->initUint32($per + 0x30, $info);

        // Racing reset combo: Start + A + B (no X/Y on a wheel).
        $this->initUint32($per + 0x08, 0x0006); // on: A|B held
        $this->initUint32($per + 0x10, 0x0008); // press: Start pressed

        $buf = $this->addressOf('_var_peripherals_8c1ba35c');

        $this->call('_inputTaskAlt_8c012718');

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')->with(0)->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);
        $this->shouldWriteWord($buf + 0x18, 0);
        $this->shouldWriteWord($buf + 0x1a, 0);
        $this->shouldWriteWord($buf + 0x1c, 0);
        $this->shouldWriteLong($buf + 0x08, 0);
        $this->shouldWriteLong($buf + 0x10, 0);
        $this->shouldWriteLongTo('_var_activeCtrlType_8c157a70', 0x700fe);

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 1);

        $this->forceStop();
    }

    public function test_vibration_pack_on_port1()
    {
        // Port 1 has vibration -> vibport = 1.
        $this->vibportScenario(port1Type: 0x100, port2Type: null, expected: 1);
    }

    public function test_vibration_pack_on_port2()
    {
        // Port 1 lacks it, port 2 has it -> vibport = 2.
        $this->vibportScenario(port1Type: 0, port2Type: 0x100, expected: 2);
    }

    public function test_no_vibration_pack()
    {
        // Neither port reports vibration -> vibport = -1.
        $this->vibportScenario(port1Type: 0, port2Type: 0, expected: -1);
    }

    public function test_vibration_detection_masks_other_capability_bits()
    {
        // Port 1 has non-vibration bits -> must not match.
        $this->vibportScenario(port1Type: 0x0ff, port2Type: 0x100, expected: 2);
    }

    // port2Type=null means port 1 matched (port 2 never queried).
    private function vibportScenario(int $port1Type, ?int $port2Type, int $expected): void
    {
        $this->resolveSymbols();
        $this->setupStandardTable();

        $per  = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1);             // controller type bit set
        $this->initUint32($per + 0x04, 0xf06fe); // standard controller support bits
        $this->initUint16($per + 0x18, 0);
        $this->initUint16($per + 0x1a, 0);
        $this->initUint16($per + 0x1c, 0);
        $this->initUint32($per + 0x08, 0);        // on: no inputs -> reset combo off
        $this->initUint32($per + 0x10, 0);        // press: no inputs
        $this->initUint32($per + 0x30, $info);

        $per1  = $this->alloc(0x34);
        $info1 = $this->alloc(0x4);
        $this->initUint32($info1, $port1Type);
        $this->initUint32($per1 + 0x30, $info1);

        $per2 = null;
        if ($port2Type !== null) {
            $per2  = $this->alloc(0x34);
            $info2 = $this->alloc(0x4);
            $this->initUint32($info2, $port2Type);
            $this->initUint32($per2 + 0x30, $info2);
        }

        $buf = $this->addressOf('_var_peripherals_8c1ba35c');

        $this->call('_inputTaskAlt_8c012718');

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')->with(0)->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);
        $this->shouldWriteWord($buf + 0x18, 0);
        $this->shouldWriteWord($buf + 0x1a, 0);
        $this->shouldWriteWord($buf + 0x1c, 0);
        $this->shouldWriteLong($buf + 0x08, 0);
        $this->shouldWriteLong($buf + 0x10, 0);
        $this->shouldWriteLongTo('_var_activeCtrlType_8c157a70', 0xf06fe);

        $this->shouldCall('_pdGetPeripheral')->with(1)->andReturn($per1);
        if ($per2 !== null) {
            $this->shouldCall('_pdGetPeripheral')->with(2)->andReturn($per2);
        }
        $this->shouldWriteLongTo('_var_vibport_8c1ba354', $expected);
        $this->shouldCall('_vmsLcd_8c01c910');
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_var_vibport_8c1ba354', 4);
        $this->setSize('_var_peripherals_8c1ba35c', 0x34 * 2);
        $this->setSize('_var_activeCtrlType_8c157a70', 4);
        $this->setSize('_vmsLcd_8c01c910', 4);
        $this->initUint32Array(
            $this->addressOf('_const_peripheral_8c033318'),
            array_fill(0, 0x34 / 4, 0),
        );
    }

    // Pin _init_8c03bef0 right after _init_8c03beb8 (asm loop bound); zero the table.
    private function setupStandardTable(): void
    {
        $this->setSize('_init_8c03beb8', 0x38);
        $base = $this->addressOf('_init_8c03beb8');
        $this->rellocate('_init_8c03bef0', $base + 0x38);
        $this->initUint32Array($base, array_fill(0, 14, 0));
    }

    // Loop bound (base+0x28) is computed in asm; no symbol to pin.
    private function setupRacingTable(): void
    {
        $this->setSize('_init_8c03bf18', 0x28);
        $base = $this->addressOf('_init_8c03bf18');
        $this->initUint32Array($base, array_fill(0, 10, 0));
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
