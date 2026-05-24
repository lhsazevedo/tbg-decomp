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
        $this->call('_swapDialogMessageBox_8c017108')->with(65);

        $this->shouldCall('_swapMessageBoxFor_8c02aefc')
            ->with("さぁ、コースを選んでくれ")
            ->andReturn(42);

        $this->shouldWrite($this->addressOf('_var_menuTextboxCharLimit_8c225fb8'), 42);
    }
};
