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
    public function test_1()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_8c1ba1cc'), 9);
        $this->initUint32($this->addressOf('_var_8c1ba25c'), 1337);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0x90, 0xcafe0001);

        $this->call('_FUN_8c016ee6');

        $this->shouldCall('_FUN_8c016e6c')->with(9, 84.0, 82.0);
        $this->shouldCall('_FUN_8c016ed2')->andReturn(2);
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8'),
            8,
            112.0,
            82.0,
            -4.0,
        );
        $this->shouldCall('_FUN_8c016e6c')->with(
            0xcafe0001, 534.0, 82.0
        );
    }

    public function test_2()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_8c1ba1cc'), 10);
        $this->initUint32($this->addressOf('_var_8c1ba25c'), 1337);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0x90, 0xcafe0001);

        $this->call('_FUN_8c016ee6');

        $this->shouldCall('_FUN_8c016e6c')->with(10, 95.0, 82.0);
        $this->shouldCall('_FUN_8c016ed2')->andReturn(4);
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8'),
            6 + 4,
            112.0,
            82.0,
            -4.0,
        );
        $this->shouldCall('_FUN_8c016e6c')->with(
            0xcafe0001, 534.0, 82.0
        );
    }

    public function test_3()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_8c1ba1cc'), 15);
        $this->initUint32($this->addressOf('_var_8c1ba25c'), 1337);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0x90, 0xcafe0001);

        $this->call('_FUN_8c016ee6');

        $this->shouldCall('_FUN_8c016e6c')->with(15, 95.0, 82.0);
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8'),
            13,
            112.0,
            82.0,
            -4.0,
        );
        $this->shouldCall('_FUN_8c016e6c')->with(
            0xcafe0001, 534.0, 82.0
        );
    }

    public function test_4()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_8c1ba1cc'), 23);
        $this->initUint32($this->addressOf('_var_8c1ba25c'), 1337);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0x90, 0xcafe0001);

        $this->call('_FUN_8c016ee6');

        $this->shouldCall('_FUN_8c016e6c')->with(23, 95.0, 82.0);
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8'),
            14,
            112.0,
            82.0,
            -4.0,
        );
        $this->shouldCall('_FUN_8c016e6c')->with(
            0xcafe0001, 534.0, 82.0
        );
    }

    public function test_5()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_8c1ba1cc'), 24);
        $this->initUint32($this->addressOf('_var_8c1ba25c'), 1337);
        $this->initUint32($this->addressOf('_var_8c1ba1cc') + 0x90, 0xcafe0001);

        $this->call('_FUN_8c016ee6');

        $this->shouldCall('_FUN_8c016e6c')->with(24, 95.0, 82.0);
        $this->shouldCall('_FUN_8c016ed2')->andReturn(3);
        $this->shouldCall('_drawSprite_8c014f54')->with(
            $this->addressOf('_menuState_8c1bc7a8'),
            9,
            112.0,
            82.0,
            -4.0,
        );
        $this->shouldCall('_FUN_8c016e6c')->with(0xcafe0001, 534.0, 82.0);
    }

    public function resolveSymbols()
    {
        $this->setSize('_var_8c1ba1cc', 4);
        $this->setSize('_var_8c1ba25c', 4);
        // Functions
        $this->setSize('__modlu', 4);
    }
};

