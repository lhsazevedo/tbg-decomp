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

        $this->call('_inputTask_8c012504');

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

        $this->call('_inputTask_8c012504');

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')->with(0)->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);
        $this->shouldCall('__quick_odd_mvn')->do($this->oddMvn());
        $this->shouldWriteLongTo('_var_vibport_8c1ba354', -1);
        $this->shouldWriteLongTo('_var_activeCtrlType_8c157a70', -1);
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

        $this->call('_inputTask_8c012504');

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')->with(0)->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);
        $this->shouldWriteWord($buf + 0x18, 0);
        $this->shouldWriteWord($buf + 0x1a, 0);
        $this->shouldWriteWord($buf + 0x1c, 0);
        $this->shouldWriteLong($buf + 0x08, 0);
        $this->shouldWriteLong($buf + 0x10, 0);
        $this->shouldWriteLongTo('_var_activeCtrlType_8c157a70', 0xf06fe);

        // TODO: test full behavior instead of stopping early
        $this->forceStop();
    }

    public function test_standard_controller_translates_inputs()
    {
        $this->resolveSymbols();
        $this->setupInitTable();

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1);
        $this->initUint32($per + 0x04, 0xf06fe);
        $this->initUint16($per + 0x18, 0);
        $this->initUint16($per + 0x1a, 0);
        $this->initUint16($per + 0x1c, 0);
        $this->initUint32($per + 0x30, $info);

        // Two pairs in init_btnRemap_8c03be80; rest zeroed.
        $table = $this->addressOf('_init_btnRemap_8c03be80');
        $this->initUint32($table + 0x00, 0x0001); // pair 0 mask
        $this->initUint32($table + 0x04, 0x0100); // pair 0 button
        $this->initUint32($table + 0x08, 0x0002); // pair 1 mask
        $this->initUint32($table + 0x0c, 0x0200); // pair 1 button

        $this->initUint32($per + 0x08, 0x0003); // on: matches both pairs
        $this->initUint32($per + 0x10, 0x0001); // press: matches only pair 0

        $buf = $this->addressOf('_var_peripherals_8c1ba35c');

        $this->call('_inputTask_8c012504');

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')->with(0)->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);
        $this->shouldWriteWord($buf + 0x18, 0);
        $this->shouldWriteWord($buf + 0x1a, 0);
        $this->shouldWriteWord($buf + 0x1c, 0);
        $this->shouldWriteLong($buf + 0x08, 0);
        $this->shouldWriteLong($buf + 0x10, 0);
        $this->shouldWriteLongTo('_var_activeCtrlType_8c157a70', 0xf06fe);

        // on-loop: pair 0 matches -> .on |= 0x100
        $this->shouldWriteLong($buf + 0x08, 0x0100);
        // on-loop: pair 1 matches -> .on |= 0x200
        $this->shouldWriteLong($buf + 0x08, 0x0300);
        // press-loop: only pair 0 matches -> .press |= 0x100
        $this->shouldWriteLong($buf + 0x10, 0x0100);

        // press & 8 = 0, reset combo skipped.
        $this->forceStop();
    }

    public function test_standard_controller_reset_combo_triggers()
    {
        $this->resolveSymbols();
        $this->setupInitTable();

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1);
        $this->initUint32($per + 0x04, 0xf06fe);
        $this->initUint16($per + 0x18, 0);
        $this->initUint16($per + 0x1a, 0);
        $this->initUint16($per + 0x1c, 0);
        $this->initUint32($per + 0x30, $info);

        // init_btnRemap_8c03be80 all zeros -> loops produce no writes.
        // Sega mandatory reset combo: Start + A + B + X + Y.
        $this->initUint32($per + 0x08, 0x0606); // on: A|B|X|Y held
        $this->initUint32($per + 0x10, 0x0008); // press: Start pressed

        $buf = $this->addressOf('_var_peripherals_8c1ba35c');

        $this->call('_inputTask_8c012504');

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
        $this->setupInitTable();

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1);
        $this->initUint32($per + 0x04, 0xf06fe);
        $this->initUint16($per + 0x18, 0);
        $this->initUint16($per + 0x1a, 0);
        $this->initUint16($per + 0x1c, 0);
        $this->initUint32($per + 0x30, $info);

        // Start pressed but X|Y missing -> combo fails.
        $this->initUint32($per + 0x08, 0x0006); // on: A|B held only
        $this->initUint32($per + 0x10, 0x0008); // press: Start pressed

        $buf = $this->addressOf('_var_peripherals_8c1ba35c');

        $this->call('_inputTask_8c012504');

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

        $this->call('_inputTask_8c012504');

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')->with(0)->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);
        $this->shouldWriteWord($buf + 0x18, 0);
        $this->shouldWriteWord($buf + 0x1a, 0);
        $this->shouldWriteWord($buf + 0x1c, 0);
        $this->shouldWriteLong($buf + 0x08, 0);
        $this->shouldWriteLong($buf + 0x10, 0);
        $this->shouldWriteLongTo('_var_activeCtrlType_8c157a70', 0x700fe);

        // TODO: test full behavior instead of stopping early
        $this->forceStop();
    }

    public function test_racing_controller_translates_inputs()
    {
        $this->resolveSymbols();
        $this->setupRacingInitTable();
        $this->setupRacingState(timerRaw: 0, mode: 0);

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1);
        $this->initUint32($per + 0x04, 0x700fe);
        $this->initUint16($per + 0x18, 0);
        // l = 0 -> peripherals[0].l < 0x81, special-XOR path gated off.
        $this->initUint16($per + 0x1a, 0);
        $this->initUint16($per + 0x1c, 0);
        $this->initUint32($per + 0x30, $info);

        // Two (mask, button) pairs at start of init_btnRemapWheel_8c03bef0; rest stay 0.
        $table = $this->addressOf('_init_btnRemapWheel_8c03bef0');
        $this->initUint32($table + 0x00, 0x0001); // pair 0 mask
        $this->initUint32($table + 0x04, 0x0100); // pair 0 button
        $this->initUint32($table + 0x08, 0x0002); // pair 1 mask
        $this->initUint32($table + 0x0c, 0x0200); // pair 1 button

        $this->initUint32($per + 0x08, 0x0003); // on: matches both pairs
        $this->initUint32($per + 0x10, 0x0001); // press: matches only pair 0

        $buf = $this->addressOf('_var_peripherals_8c1ba35c');

        $this->call('_inputTask_8c012504');

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

        // press & 0x200 == 0 -> reset-combo branch; press & 8 == 0 -> no reset.
        $this->forceStop();
    }

    public function test_racing_controller_reset_combo_triggers()
    {
        $this->resolveSymbols();
        $this->setupRacingInitTable();
        $this->setupRacingState(timerRaw: 0, mode: 0);

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

        $this->call('_inputTask_8c012504');

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

    public function test_racing_paddle_shift_remaps_to_dpad_up()
    {
        // mode 5: press ^ 0x210 remaps TY -> D-pad Up.
        $this->racingPaddleShift(mode: 5, finalPress: 0x0010);
    }

    public function test_racing_paddle_shift_remaps_to_dpad_down()
    {
        // mode 0: press ^ 0x220 remaps TY -> D-pad Down.
        $this->racingPaddleShift(mode: 0, finalPress: 0x0020);
    }

    public function test_racing_paddle_shift_no_remap_for_other_mode()
    {
        // Mode 3: gate passes but no remap -> press stays 0x200.
        $this->racingPaddleShift(mode: 3, finalPress: null);
    }

    // Exercises paddle-shift remap with brake at threshold (0x81) and timer=0.
    // finalPress=null means mode is neither 5 nor 0 (no remap).
    private function racingPaddleShift(int $mode, ?int $finalPress): void
    {
        $this->resolveSymbols();
        $this->setupRacingInitTable();
        $this->setupRacingState(timerRaw: 0, mode: $mode);

        $per = $this->alloc(0x34);
        $info = $this->alloc(0x4);

        $this->initUint32($info, 1);
        $this->initUint32($per + 0x04, 0x700fe);
        $this->initUint16($per + 0x18, 0);
        $this->initUint16($per + 0x1a, 0x81); // l threshold
        $this->initUint16($per + 0x1c, 0);
        $this->initUint32($per + 0x30, $info);

        // One pair mapping mask 0x1 -> logical TY (0x200).
        $table = $this->addressOf('_init_btnRemapWheel_8c03bef0');
        $this->initUint32($table + 0x00, 0x0001);
        $this->initUint32($table + 0x04, 0x0200);

        $this->initUint32($per + 0x08, 0x0000);
        $this->initUint32($per + 0x10, 0x0001); // press fires pair 0 -> .press |= TY

        $buf = $this->addressOf('_var_peripherals_8c1ba35c');

        $this->call('_inputTask_8c012504');

        $this->shouldWriteLongTo('_var_resetRequested_8c157a78', 0);
        $this->shouldCall('_pdGetPeripheral')->with(0)->andReturn($per);
        $this->shouldWriteLongTo('_var_peripheral_8c1ba358', $per);
        $this->shouldWriteWord($buf + 0x18, 0);
        $this->shouldWriteWord($buf + 0x1a, 0x81);
        $this->shouldWriteWord($buf + 0x1c, 0);
        $this->shouldWriteLong($buf + 0x08, 0);
        $this->shouldWriteLong($buf + 0x10, 0);
        $this->shouldWriteLongTo('_var_activeCtrlType_8c157a70', 0x700fe);

        // press-loop translates physical bit 0x1 to logical TY (0x200).
        $this->shouldWriteLong($buf + 0x10, 0x0200);
        if ($finalPress !== null) {
            // XOR remap rewrites press to the D-pad bit.
            $this->shouldWriteLong($buf + 0x10, $finalPress);
        }

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
        // Port 1 has non-vibration bits -> must not match; guards against truthy type check.
        $this->vibportScenario(port1Type: 0x0ff, port2Type: 0x100, expected: 2);
    }

    // port2Type=null means port 1 matched (port 2 never queried).
    private function vibportScenario(int $port1Type, ?int $port2Type, int $expected): void
    {
        $this->resolveSymbols();
        $this->setupInitTable();

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

        $this->call('_inputTask_8c012504');

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

    // Pin _init_btnRemapAlt_8c03beb8 right after _init_btnRemap_8c03be80 (asm loop bound); zero the table.
    private function setupInitTable(): void
    {
        $this->setSize('_init_btnRemap_8c03be80', 0x38);
        $base = $this->addressOf('_init_btnRemap_8c03be80');
        $this->rellocate('_init_btnRemapAlt_8c03beb8', $base + 0x38);
        $this->initUint32Array($base, array_fill(0, 14, 0));
    }

    // Pin _init_btnRemapWheelAlt_8c03bf18 right after _init_btnRemapWheel_8c03bef0 (asm loop bound); zero the table.
    private function setupRacingInitTable(): void
    {
        $this->setSize('_init_btnRemapWheel_8c03bef0', 0x28);
        $base = $this->addressOf('_init_btnRemapWheel_8c03bef0');
        $this->rellocate('_init_btnRemapWheelAlt_8c03bf18', $base + 0x28);
        $this->initUint32Array($base, array_fill(0, 10, 0));
    }

    // timerRaw: raw bits of float _var_8c1bbc4c (0 == 0.0f).
    private function setupRacingState(int $timerRaw, int $mode): void
    {
        $this->setSize('_var_8c1bbc4c', 4);
        $this->setSize('_var_8c1bbcc4', 4);
        $this->initUint32($this->addressOf('_var_8c1bbc4c'), $timerRaw);
        $this->initUint32($this->addressOf('_var_8c1bbcc4'), $mode);
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
