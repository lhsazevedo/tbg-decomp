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

        $this->initUint32($this->addressOf('_var_8c1ba1cc'), 264);
        $this->call('_FUN_8c016ed2');
        $this->shouldReturn(6);
    }

    public function test_2()
    {
        $this->resolveSymbols();

        $this->initUint32($this->addressOf('_var_8c1ba1cc'), 265);
        $this->call('_FUN_8c016ed2');
        $this->shouldReturn(0);
    }

    public function resolveSymbols()
    {
        $this->setSize('_var_8c1ba1cc', 4);
        // Functions
        $this->setSize('__modlu', 4);
    }
};

