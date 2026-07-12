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
    public function test_basic()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_menuState_8c1bc7a8') + 0x00, 0xcafe0001);

        $this->call('_drawInteger_8c016e6c')->with(
            100,
            42.0,
            69.0,
        );

        $this->shouldCall('__modls');
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_var_menuState_8c1bc7a8') + 0,
            15,
            42.0,
            69.0,
            -4.0
        );

        $this->shouldCall('__divls');
        $this->shouldCall('__modls');
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_var_menuState_8c1bc7a8') + 0,
            15,
            32.0,
            69.0,
            -4.0
        );

        $this->shouldCall('__divls');
        $this->shouldCall('__modls');
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_var_menuState_8c1bc7a8') + 0,
            16,
            22.0,
            69.0,
            -4.0
        );

        $this->shouldCall('__divls');
    }

    public function resolveSymbols()
    {
        $this->setSize('_var_menuState_8c1bc7a8', 0x6c);

        // Functions
        $this->setSize('__modls', 4);
        $this->setSize('__divls', 4);

        $this->onCall('__modls', function () {
            $this->setRegister(0, $this->getRegister(1)->mod($this->getRegister(0)));
        });
        $this->onCall('__divls', function () {
            $this->setRegister(0, $this->getRegister(1)->div($this->getRegister(0)));
        });
    }
};
