<?php

declare(strict_types=1);

use Lhsazevedo\Sh4ObjTest\TestCase;
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

        $this->call('_FUN_8c017126');
    }

    public function resolveSymbols()
    {
        $this->setSize('_var_peripheral_8c1ba35c', 52 * 2);
        $this->setSize('_init_runStruct_8c04442c', 0x1c * 15);
        $this->setSize('_menuState_8c1bc7a8', 0x6c);
    }
};
