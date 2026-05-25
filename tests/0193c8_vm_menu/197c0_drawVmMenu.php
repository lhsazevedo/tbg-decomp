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
    public function test_A()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x20, fdec(290.0));
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x24, fdec(194.0));

        $this->initUint32Array(
            $this->addressOf('_var_vmuStatus_8c226048'),
            [0,1,2,4,5,6,0,1,2,3]
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0x10,
            290.0, 194.0, -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0x09,
            0.0, 0.0, -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0x0a,
            0.0, 0.0, -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0x0b,
            0.0, 0.0, -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0x0c,
            0.0, 0.0, -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0x0d,
            0.0, 0.0, -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0x0f,
            0.0, 0.0, -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0x07,
            0.0, 0.0, -6.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00, 0x01,
            0.0, 0.0, -4.3
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00, 0x00,
            0.0, 0.0, -7.0
        );

        $this->singleCall('_drawVmMenu_197c0')->run();
    }

    public function test_B()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x20, fdec(290.0));
        $this->initUint32($this->addressOf('_menuState_8c1bc7a8') + 0x24, fdec(194.0));

        $this->initUint32Array(
            $this->addressOf('_var_vmuStatus_8c226048'),
            [2,1,0,6,5,4,2,1,0,3]
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0x10,
            290.0, 194.0, -4.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0x08,
            0.0, 0.0, -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0x09,
            0.0, 0.0, -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0x0b,
            0.0, 0.0, -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0x0c,
            0.0, 0.0, -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0x0d,
            0.0, 0.0, -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0x0e,
            0.0, 0.0, -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0x0f,
            0.0, 0.0, -5.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x0c, 0x07,
            0.0, 0.0, -6.0
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00, 0x01,
            0.0, 0.0, -4.3
        );

        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8') + 0x00, 0x00,
            0.0, 0.0, -7.0
        );

        $this->singleCall('_drawVmMenu_197c0')->run();
    }

    private function resolveSymbols(): void
    {
        $this->setSize('_menuState_8c1bc7a8', 0x6c);

        // Functions
        $this->setSize('_drawSprite_8c014f54', 0x4);
    }

    private function initUint32Array(int $address, array $values): void
    {
        foreach ($values as $i => $value) {
            $this->initUint32($address + $i * 4, $value);
        }
    }
};
