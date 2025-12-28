<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
use Lhsazevedo\Sh4ObjTest\Simulator\Arguments\WildcardArgument;
use Lhsazevedo\Sh4ObjTest\Simulator\Types\U32;

if (!function_exists('fdec')) {
    function fdec(float $value) {
        return unpack('L', pack('f', $value))[1];
    }
}

return new class extends TestCase {
    public function test_handles_already_selected()
    {
        $this->resolveSymbols();

        // selected
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x3c, 2);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x40, 1);

        // cursor
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x20, fdec(42.0));
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x24, fdec(69.0));

        // target
        $this->initUint32($this->addressOf('_init_courseMenuButtons_8c04442c') + 0x1c * 7 + 0x08, fdec(42.0));
        $this->initUint32($this->addressOf('_init_courseMenuButtons_8c04442c') + 0x1c * 7 + 0x0c, fdec(69.0));

        $this->call('_cursorOffTarget_8c016dc6');
        $this->shouldReturn(0);
    }

    public function test_selects()
    {
        $this->resolveSymbols();

        // selected
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x3c, 2);
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x40, 1);

        // cursor
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x20, fdec(12.0));
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x24, fdec(9.0));

        // target
        $this->initUint32($this->addressOf('_init_courseMenuButtons_8c04442c') + 0x1c * 7 + 0x08, fdec(42.0));
        $this->initUint32($this->addressOf('_init_courseMenuButtons_8c04442c') + 0x1c * 7 + 0x0c, fdec(69.0));

        $this->call('_cursorOffTarget_8c016dc6');

        $this->shouldWriteFloat($this->addressOf('_menuState_8c1bc7a8') + 0x28, 42.0);
        $this->shouldWriteFloat($this->addressOf('_menuState_8c1bc7a8') + 0x2c, 69.0);
        $this->shouldWriteFloat($this->addressOf('_menuState_8c1bc7a8') + 0x30, 5.0);
        $this->shouldWriteFloat($this->addressOf('_menuState_8c1bc7a8') + 0x34, 10.0);

        $this->shouldCall('_sdMidiPlay')->with(0x31d131d1);
        $this->shouldCall('_swapMessageBoxFor_8c02aefc')->with($this->addressOf('_const_8c03628c'));
        $this->shouldReturn(1);
    }

    public function resolveSymbols()
    {
        $this->setSize('_menuState_8c1bc7a8', 0x6c);
        $this->setSize('_init_courseMenuButtons_8c04442c', 0x1c * 15);

        // Functions
        $this->setSize('__quick_evn_mvn', 4);

        $this->initUint32($this->addressOf('_var_midiHandles_8c0fcd28'), 0x31d131d1);
    }
};
